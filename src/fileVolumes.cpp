/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in FileMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "FileMgr.h"
#include "OSEvent.h"
#include "VRetraceMgr.h"
#include "OSUtil.h"
#include "MemoryMgr.h"

#include "rsys/hfs.h"

#include "rsys/file.h"
#include "rsys/glue.h"
#include "rsys/notmac.h"
#include "rsys/segment.h"
#include "rsys/string.h"

#if defined(CYGWIN32) || defined(MSDOS) || defined(WIN32)
#include "rsys/fauxdbm.h"
#endif

#if defined(WIN32)
#include "winfs.h"
#endif

#define MAKEVOLUME "/makevolume"

#define RETURN(x)  \
    {              \
        err = (x); \
        goto DONE; \
    }

using namespace Executor;

/*
 * For now we just use 101 entries in our hash table, no matter what.
 * The hash function should be pretty damn uniform, so even if we have
 * 10,000 entries (pretty unlikely), the linked list would only grow
 * to 100 entries each, which shouldn't lead to very LONGINT searches.
 *
 * We still keep the number of hash entries in the data structure, because
 * we want the flexibility to be able to change if anyone ever has the
 * time to care.
 */

/*
 * TODO: put an override option into the function that writes into
 *	 the hash table.  Use it when the host is known good.
 */

/*
 * NOTE: The only two directories that are guaranteed to be in the database
 *	 are "ExecutorVolume" and "ExecutorVolume/System Folder".  If the
 *	 program is started with any file arguments, their directory IDs
 *	 and the directories/filesystems that are between them and /
 *	 are also put into the database.  All other entries come from
 *	 the special hooks in Stdfile (or other places we put hooks).
 */

#define HASHSIZE 101

PRIVATE hashlink_t ** hashloc(VCBExtra * vcbp, LONGINT dir)
{
    hashlink_t **hlpp;
    INTEGER index;

    gui_assert(vcbp);
    index = dir % vcbp->u.ufs.nhashentries;
    for(hlpp = &vcbp->u.ufs.hashtable[index]; *hlpp && (*hlpp)->dirid != dir;
        hlpp = &(*hlpp)->next)
        ;
    return hlpp;
}

typedef struct _chain_str
{
    hashlink_t *hashlinkp;
    struct _chain_str *next;
} chain_t;

/*
 * one of the following enum values is placed at the beginning of the
 * directory name to indicate how confident we are that we have the
 * right value.  The order *is* important in the code:  they are
 * listed from least confident to most confident (actually checked_val
 * and new_val represent the same confidence, but checked_val doesn't
 * get placed into the database when we shut down).
 */

typedef enum {
    old_generic_val,
    old_specific_val,
    checked_val,
    new_val,
} newness_t;

static int
chaincount(chain_t *therest)
{
    if(therest == 0)
        /* the destination string already has accounted
	 for the nul terminator */
        return 0;
    else
        return (strlen(therest->hashlinkp->dirname + 1)
                + chaincount(therest->next)
                /* for the prepended `/' */
                + 1);
}

static void
chainfill(char *p, chain_t *therest)
{
    if(therest)
    {
        *p++ = '/';
        strcpy(p, therest->hashlinkp->dirname + 1);
        chainfill(p + strlen(p), therest->next);
    }
}

static bool
on_chain_p(hashlink_t *hlp, chain_t *therest, bool dump_link)
{
    if(dump_link)
        warning_trace_info("dirid = %d, parid = %d, dirname = '%s'", hlp->dirid,
                           hlp->parid, hlp->dirname);
    if(!therest)
        return false;
    else if(therest->hashlinkp == hlp)
        return true;
    else
        return on_chain_p(hlp, therest->next, dump_link);
}

void recsetdp(VCBExtra *vcbp, datum *dp, hashlink_t *hlp, chain_t *therest)
{
    if(hlp->parid <= 2 || hlp->parid == vcbp->u.ufs.ino)
    {
        int ourlen;

        ourlen = strlen(hlp->dirname + 1); /* don't count flag byte */
        dp->dsize = ourlen + chaincount(therest) + 1;
        dp->dptr = (char *)malloc(dp->dsize);
        strcpy((char *)dp->dptr, hlp->dirname + 1);
        chainfill((char *)((uintptr_t)dp->dptr + ourlen), therest);
    }
    else
    {
        chain_t ourlink;
        hashlink_t *newhlp;

        ourlink.hashlinkp = hlp;
        ourlink.next = therest;
        newhlp = *hashloc(vcbp, hlp->parid);
        if(newhlp)
        {
            if(on_chain_p(newhlp, therest, false))
            {
                warning_unexpected("recset loop unixname = '%s'"
                                   " dirid = %d, parid = %d, dirname = '%s'",
                                   vcbp->unixname, newhlp->dirid,
                                   newhlp->parid, newhlp->dirname);
                on_chain_p(newhlp, therest, true);
                dp->dsize = 0;
                dp->dptr = 0;
            }
            else
                recsetdp(vcbp, dp, newhlp, &ourlink);
        }
        else
        {
            warning_unexpected("newhlp = 0, unixname = %s, parid = %d",
                               vcbp->unixname, hlp->parid);
            dp->dsize = 0;
            dp->dptr = 0;
        }
    }
}

PUBLIC datum Executor::ROMlib_dbm_fetch(VCBExtra * vcbp, LONGINT dir) /* INTERNAL */
{
    datum foo;
    hashlink_t *hlp;

    hlp = *hashloc(vcbp, dir);

    if(hlp)
        recsetdp(vcbp, &foo, hlp, (chain_t *)0);
    else
    {
        foo.dptr = 0;
        foo.dsize = 0;
    }
    return foo;
}

PRIVATE char *copystring(const char *orig, newness_t new1)
{
    size_t length;
    char *retval;

    length = strlen(orig);
    retval = (char *)malloc(length + 2); /* +1 for NULL, +1 for new */
    retval[0] = new1;
    strcpy(retval + 1, orig);
    return retval;
}

PRIVATE const char *
filename_from_pathname(const char *path)
{
    const char *last_slash;
    const char *last_back_slash;

    last_slash = strrchr(path, '/');
    last_back_slash = strrchr(path, '\\');
    path = MAX(path, last_slash + 1);
    path = MAX(path, last_back_slash + 1);
    return path;
}

#define REMOVE_TRAILING_SLASHES(path)                            \
    ({                                                           \
        char *__retval;                                          \
        const char *__path;                                      \
        int __len;                                               \
        const char *__lastcp;                                    \
                                                                 \
        __path = (path);                                         \
        __len = strlen(__path);                                  \
        __lastcp = __path + __len - 1;                           \
        while(__len > SLASH_CHAR_OFFSET + 1 && *__lastcp == '/') \
        {                                                        \
            --__len;                                             \
            --__lastcp;                                          \
        }                                                        \
                                                                 \
        __retval = (char *)alloca(__len + 1);                    \
        memcpy(__retval, __path, __len);                         \
        __retval[__len] = 0;                                     \
                                                                 \
        __retval;                                                \
    })

PRIVATE bool
filename_match(const char *prefix, const char *path1, const char *path2)
{
    bool retval;
    int prefix_length;

    retval = true;
    if(path2[0] == '/' && !path2[1]) /* top-level dir of this volume */
        prefix_length = 1;
    else
    {
        prefix_length = strlen(prefix);
        if(strncasecmp(prefix, path2, prefix_length) != 0)
            retval = false;
    }

    if(retval)
    {
        const char *file1;
        const char *file2;

        path2 += prefix_length;
        file1 = filename_from_pathname(path1);
        file2 = REMOVE_TRAILING_SLASHES(path2);
        file2 = filename_from_pathname(file2);
        retval = strcasecmp(file1, file2) == 0;
    }
    return retval;
}

/*
 * hashinsert will do the stat for you if the dirid is 0
 * hashinsert returns true if the entry is already there
 */

PRIVATE BOOLEAN hashinsert(VCBExtra *vcbp, char *pathname, LONGINT *diridp,
                           LONGINT parid, newness_t new1)
{
    BOOLEAN retval;
    hashlink_t **hlpp, *newlink;
    struct stat sbuf;
    char *savep, save;
    char *newpathname;
    int len;

    len = strlen(pathname) + 1;
    newpathname = (char *)alloca(len);
    memcpy(newpathname, pathname, len);
    ROMlib_undotdot(newpathname);
    pathname = newpathname;
    if(*diridp == 0)
    {
        if(Ustat(pathname, &sbuf) == 0 && S_ISDIR(sbuf.st_mode))
            *diridp = ST_INO(sbuf);
    }
    if(!diridp)
        retval = false;
    else
    {
        hlpp = hashloc(vcbp, *diridp);
        if(*hlpp && (*hlpp)->dirname[0] >= checked_val)
        {
            if(!filename_match(vcbp->unixname,
                               (*hlpp)->dirname + 1, pathname)
               || (parid && (*hlpp)->parid != parid))
            {
                warning_unexpected("%s %s %d %d %d %d %s %d %d",
                                   vcbp->unixname, pathname, *diridp, parid,
                                   new1, (*hlpp)->dirname[0],
                                   (*hlpp)->dirname + 1, (*hlpp)->dirid,
                                   (*hlpp)->parid);
                goto use_new_value_to_be_safe;
            }
            retval = true;
        }
        else
        {
        use_new_value_to_be_safe:
            savep = 0;
            retval = false;
            if(parid == 0)
            {
                if(*diridp == 2) /* root of a filesystem */
                    parid = 1; /* that's the way it is on the mac */
                else
                {
                    savep = strrchr(pathname, '/');
                    if(savep == pathname + SLASH_CHAR_OFFSET)
                        ++savep;
                    save = *savep;
                    *savep = 0;
                    if(Ustat(pathname, &sbuf) == 0)
                        parid = ST_INO(sbuf);
                    *savep = save;
                    if(savep == pathname + 1 + SLASH_CHAR_OFFSET)
                        --savep; /* restore for later */
                }
            }
            if(parid)
            {
                if(strlen(pathname) > SLASH_CHAR_OFFSET
                   && pathname[SLASH_CHAR_OFFSET] == '/')
                {
                    if(!savep)
                        savep = strrchr(pathname, '/');
                    ++savep;
                }
                else
                    savep = pathname;
                if(*hlpp)
                {
                    if((*hlpp)->dirid == *diridp && (*hlpp)->parid == parid && strcmp(savep, (*hlpp)->dirname + 1) == 0)
                    {
                        if(new1 >= checked_val && (*hlpp)->dirname[0] < checked_val)
                            (*hlpp)->dirname[0] = checked_val;
                    }
                    else if(new1 >= (unsigned char)(*hlpp)->dirname[0])
                    {
                        free((*hlpp)->dirname);
                        (*hlpp)->dirname = 0;
                    }
                }
                else
                {
                    newlink = (hashlink_t *)malloc(sizeof(hashlink_t));
                    newlink->next = 0;
                    newlink->dirid = *diridp;
                    newlink->dirname = 0;
                    *hlpp = newlink;
                }
                (*hlpp)->parid = parid;
                if(!(*hlpp)->dirname)
                    (*hlpp)->dirname = copystring(savep, new1);
            }
        }
    }
    return retval;
}

PUBLIC BOOLEAN Executor::ROMlib_dbm_store(VCBExtra * vcbp, char * pathname, /* INTERNAL */
   LONGINT * diridp, BOOLEAN verify_p)
{
    BOOLEAN retval;

    retval = hashinsert(vcbp, pathname, diridp, 0, new_val);
    if(retval && verify_p)
    {
        datum verify;

        verify = ROMlib_dbm_fetch(vcbp, *diridp);
        if(!verify.dptr)
        {
            warning_unexpected("vcbp->unixname = '%s', pathname = '%s', "
                               "*diridp = %d",
                               vcbp->unixname, pathname,
                               *diridp);
            ROMlib_automount(pathname);
        }
    }
    return retval;
}

PUBLIC void Executor::ROMlib_dbm_delete_inode(VCBExtra *vcbp, LONGINT inode)
{
    hashlink_t **hlpp, *todie;

    hlpp = hashloc(vcbp, inode);
    if((todie = *hlpp))
    {
        *hlpp = todie->next;
        free(todie->dirname);
        free(todie);
    }
}

PRIVATE BOOLEAN filesystems_match(rkey_t *keyp, VCBExtra *vcbp)
{
    int namlen;
    BOOLEAN retval;

    namlen = strlen(vcbp->unixname);
    if(namlen == CW(keyp->filesystemlen))
        retval = strncmp(vcbp->unixname,
                         keyp->hostnameandroot + keyp->hostnamelen, namlen)
            == 0;
    else
        retval = false;
    return retval;
}

PRIVATE BOOLEAN no_hostname(rkey_t *keyp)
{
    return !!keyp->hostnamelen;
}

#define OURMAXHOSTNAMELEN 64

#if !defined(MSDOS) && !defined(CYGWIN32)
PUBLIC char ROMlib_hostname[OURMAXHOSTNAMELEN + 1];
PUBLIC INTEGER ROMlib_hostnamelen = -1;
#else /* defined(MSDOS) */
PUBLIC char ROMlib_hostname[] = "MSDOS";
PUBLIC INTEGER ROMlib_hostnamelen = sizeof(ROMlib_hostname) - 1;
#endif /* defined(MSDOS) */

PRIVATE void ourgethostname(void)
{
    if(ROMlib_hostnamelen == -1)
    {
#if !defined(MSDOS) && !defined(CYGWIN32) && !defined(WIN32)
        gethostname(ROMlib_hostname, OURMAXHOSTNAMELEN);
        ROMlib_hostnamelen = strlen(ROMlib_hostname);
#else /* defined(MSDOS) */
        gui_abort();
#endif /* defined(MSDOS) */
    }
}

PRIVATE BOOLEAN hostnames_match(rkey_t *keyp, VCBExtra *vcbp)
{
    BOOLEAN retval;

    ourgethostname();
    if(ROMlib_hostnamelen == keyp->hostnamelen)
    {
        retval = strncmp(ROMlib_hostname, keyp->hostnameandroot,
                         ROMlib_hostnamelen)
            == 0;
    }
    else
        retval = false;
    return retval;
}

PRIVATE BOOLEAN isamatch(rkey_t *keyp, VCBExtra *vcbp, newness_t *newp)
{
    BOOLEAN retval;

    retval = false;
    if(filesystems_match(keyp, vcbp))
    {
        if(no_hostname(keyp))
        {
            *newp = old_generic_val;
            retval = true;
        }
        else if(hostnames_match(keyp, vcbp))
        {
            *newp = old_specific_val;
            retval = true;
        }
    }
    return retval;

    return filesystems_match(keyp, vcbp) && (no_hostname(keyp) || hostnames_match(keyp, vcbp));
}

PRIVATE char *extractpathname(rcontent_t *contentp)
{
    return contentp->path;
}

/*
 * cleanvcbhash cleans up the hash table by freeing all the entries
 * except the new ones, which are kept on a list so that they can be
 * added to the database when things shut down.
 */

PRIVATE hashlink_t *cleanvcbhash(VCBExtra *vcbp)
{
    hashlink_t *retval, *hlp, *next;
    INTEGER i;

    retval = 0;
    for(i = 0; i < vcbp->u.ufs.nhashentries; ++i)
        for(hlp = vcbp->u.ufs.hashtable[i]; hlp; hlp = next)
        {
            next = hlp->next;
            if(hlp->dirname[0] == new_val)
            {
                hlp->next = retval;
                retval = hlp;
            }
            else
                free(hlp);
        }
    free(vcbp->u.ufs.hashtable);
    return retval;
}

PRIVATE void freedeletelist(hashlink_t *deletep)
{
    hashlink_t *nextlink;

    while(deletep)
    {
        nextlink = deletep->next;
        free(deletep);
        deletep = nextlink;
    }
}

PRIVATE void readadbm(const char *dbmname, VCBExtra *vcbp)
{
    DBM *db;
    datum key, content;
    rkey_t *keyp;
    rcontent_t *contentp;
    newness_t new1;
    LONGINT dirid;

    /*
 * The cast to (char *) of dbmname is necessary since dbm_open doesn't
 * explicitly require a (const char *)
 */
    if(!(db = dbm_open((char *)dbmname, O_RDONLY, (LONGINT)0)))
        ; /* fprintf(stderr, "Problems opening dbm \"%s\"\n", dbmname); */
    else
    {
        for(key = dbm_firstkey(db); key.dptr; key = dbm_nextkey(db))
        {
            keyp = (rkey_t *)key.dptr;
            if(isamatch(keyp, vcbp, &new1))
            {
                content = dbm_fetch(db, key);
                if(content.dsize)
                {
                    contentp = (rcontent_t *)content.dptr;
                    dirid = CL(keyp->dirid);
                    hashinsert(vcbp, extractpathname(contentp), &dirid,
                               CL(contentp->parid), new1);
                }
            }
        }
        dbm_close(db);
    }
}

PRIVATE void writeadbm(const char *dbmname, VCBExtra *vcbp,
                       hashlink_t *deletep)
{
    DBM *db;
    datum key, content;
    INTEGER unamelen, dirnamelen;
    char buf[1024], *dst, buf2[1024];
    rkey_t *keyp;
    rcontent_t *contentp;

    /*
 * The cast to (char *) of dbmname is necessary since dbm_open doesn't
 * explicitly require a (const char *)
 */
    if(deletep && (db = dbm_open((char *)dbmname, O_CREAT | O_RDWR,
                                 (LONGINT)0666)))
    {
        ourgethostname();
        unamelen = strlen(vcbp->unixname);
        keyp = (rkey_t *)buf;
        key.dptr = buf;
        contentp = (rcontent_t *)buf2;
        content.dptr = buf2;
        do
        {
            dirnamelen = strlen(deletep->dirname + 1);
            key.dsize = sizeof(rkey_t) + ROMlib_hostnamelen + unamelen - 1;
            content.dsize = sizeof(rcontent_t) + dirnamelen - 1;
            keyp->dirid = CL(deletep->dirid);
            keyp->filesystemlen = CW(unamelen);
            keyp->hostnamelen = ROMlib_hostnamelen;
            dst = keyp->hostnameandroot;
            memcpy(dst, ROMlib_hostname, ROMlib_hostnamelen);
            dst += ROMlib_hostnamelen;
            memcpy(dst, vcbp->unixname, unamelen);
            contentp->parid = CL(deletep->parid);
            strcpy(contentp->path, deletep->dirname + 1);
            dbm_store(db, key, content, DBM_REPLACE);
            keyp->hostnamelen = 0;
            key.dsize -= ROMlib_hostnamelen;
            /* NOTE: we memmove more bytes than we need, but with no ill effect */
            memmove(keyp->hostnameandroot,
                    keyp->hostnameandroot + ROMlib_hostnamelen,
                    key.dsize);
            dbm_store(db, key, content, DBM_REPLACE);
        } while((deletep = deletep->next));
        dbm_close(db);
    }
}

PUBLIC void Executor::ROMlib_dbm_open(VCBExtra * vcbp) /* INTERNAL */
{
    LONGINT size;
    LONGINT dirid;

    size = (LONGINT)sizeof(hashlink_t) * HASHSIZE;
    vcbp->u.ufs.hashtable = (hashlink_t **)malloc(size);
    vcbp->u.ufs.nhashentries = HASHSIZE;
    memset(vcbp->u.ufs.hashtable, 0, size);
    readadbm(ROMlib_PublicDirectoryMap.c_str(), vcbp);
#if !defined(CYGWIN32) && !defined(MSDOS)
    readadbm(ROMlib_PrivateDirectoryMap.c_str(), vcbp);
#endif
    dirid = 2;
    hashinsert(vcbp, "", &dirid, 1, checked_val);
}

PUBLIC void Executor::ROMlib_dbm_close(VCBExtra * vcbp) /* INTERNAL */
{
    hashlink_t *deletep;
    LONGINT oumask;

    deletep = cleanvcbhash(vcbp);
#if !defined(CYGWIN32) && !defined(MSDOS)
    writeadbm(ROMlib_PrivateDirectoryMap.c_str(), vcbp, deletep);
#endif
    oumask = umask(000);
    writeadbm(ROMlib_PublicDirectoryMap.c_str(), vcbp, deletep);
    umask(oumask);
    freedeletelist(deletep);
}

PUBLIC OSErr Executor::ufsPBMountVol(ParmBlkPtr pb) /* INTERNAL */
{
    VCBExtra *vp;
    const char *name;
    int macvolumenamelen, namelen;
    struct stat sbuf;
    OSErr err;
    GUEST<THz> savezone;
    ALLOCABEGIN

    err = noErr;
    savezone = TheZone;
    TheZone = SysZone;
    if(Ustat(ROMlib_volumename.c_str(), &sbuf) < 0)
        /*-->*/ RETURN(badMDBErr) if(ROMlib_vcbbydrive(Cx(pb->volumeParam.ioVRefNum)) || ROMlib_vcbbyunixname(ROMlib_volumename.c_str()))
            RETURN(volOnLinErr);
    vp = (VCBExtra *)NewPtr((Size)sizeof(VCBExtra));

    macvolumenamelen = ROMlib_volumename.size();
    if(vp)
        memset(vp, 0, (LONGINT)sizeof(VCBExtra));
    if(!vp || !(vp->unixname = (char *)NewPtr(macvolumenamelen + 1)))
        /*-->*/ RETURN(memFullErr)
            strcpy(vp->unixname, ROMlib_volumename.c_str());
    vp->vcb.vcbDrvNum = pb->ioParam.ioVRefNum;
    --ROMlib_nextvrn;
    vp->vcb.vcbVRefNum = CW(ROMlib_nextvrn);
    vp->u.ufs.ino = ST_INO(sbuf);
    if(strcmp("/", ROMlib_volumename.c_str() + SLASH_CHAR_OFFSET) == 0)
    {
        name = ROMlib_volumename.c_str();
        if(SLASH_CHAR_OFFSET == 2) /* C:/ --> C/ */
        {
            char *new_name;

            new_name = (char *)alloca(3);
            new_name[0] = name[0];
            new_name[1] = '/';
            new_name[2] = 0;
            name = new_name;
        }
    }
    else
    {
        if(!(name = strrchr(ROMlib_volumename.c_str(), '/')))
            name = ROMlib_volumename.c_str();
        else
            ++name;
    }
    namelen = strlen(name);
    if(namelen >= (int)sizeof(vp->vcb.vcbVN))
        namelen = sizeof(vp->vcb.vcbVN) - 1;
    vp->vcb.vcbVN[0] = namelen;
    strncpy((char *)vp->vcb.vcbVN + 1, name, namelen);
    /*
 * TODO: we have to set up the hash table for this entry ...
 */
    ROMlib_dbm_open(vp);
    vp->vcb.vcbSigWord = CWC(0x4244); /* IMIV-188 */
    vp->vcb.vcbFreeBks = CWC(20480); /* arbitrary */
    vp->vcb.vcbCrDate = 0; /* I'm lying here */
    vp->vcb.vcbVolBkUp = 0;
    vp->vcb.vcbAtrb = CWC(VNONEJECTABLEBIT);
    vp->vcb.vcbNmFls = CWC(100);
    vp->vcb.vcbNmAlBlks = CWC(300);
    vp->vcb.vcbAlBlkSiz = CLC(512);
    vp->vcb.vcbClpSiz = CLC(1);
    vp->vcb.vcbAlBlSt = CWC(10);
    vp->vcb.vcbNxtCNID = CLC(1000);
    if(!DefVCBPtr)
    {
        DefVCBPtr = RM((VCB *)vp);
        DefVRefNum = vp->vcb.vcbVRefNum;
        DefDirID = CLC(2); /* top level */
    }

    pb->ioParam.ioVRefNum = vp->vcb.vcbVRefNum;
    Enqueue((QElemPtr)vp, &VCBQHdr);
DONE:
    TheZone = savezone;
    ALLOCAEND
    return err;
}
#undef RETURN

PUBLIC OSErr Executor::GetVInfo(INTEGER drv, StringPtr voln, /* IMIV-107 */
   GUEST<INTEGER> * vrn, GUEST<LONGINT> * freeb)
{
    ParamBlockRec pbr;
    OSErr temp;

    pbr.volumeParam.ioVolIndex = 0;
    pbr.volumeParam.ioVRefNum = CW(drv);
    pbr.volumeParam.ioNamePtr = RM(voln);
    temp = PBGetVInfo(&pbr, 0);
    *vrn = pbr.volumeParam.ioVRefNum;
    *freeb = CL(Cx(pbr.volumeParam.ioVFrBlk) * Cx(pbr.volumeParam.ioVAlBlkSiz));
    return (temp);
}

PUBLIC OSErr Executor::GetVRefNum(INTEGER prn, GUEST<INTEGER> * vrn) /* IMIV-107 */
{
    OSErr err;
    fcbrec *fp;

    fp = PRNTOFPERR(prn, &err);

    if(err == noErr)
        *vrn = MR(fp->fcvptr)->vcbVRefNum;
    return (err);
}

PUBLIC OSErr Executor::GetVol(StringPtr voln, GUEST<INTEGER> * vrn) /* IMIV-107 */
{
    ParamBlockRec pbr;
    OSErr temp;

    pbr.volumeParam.ioNamePtr = RM(voln);
    temp = PBGetVol(&pbr, 0);
    *vrn = pbr.volumeParam.ioVRefNum;
    return (temp);
}

PUBLIC OSErr Executor::SetVol(StringPtr voln, INTEGER vrn) /* IMIV-107 */
{
    ParamBlockRec pbr;

    pbr.volumeParam.ioNamePtr = RM(voln);
    pbr.volumeParam.ioVRefNum = CW(vrn);
    return (PBSetVol(&pbr, 0));
}

PUBLIC OSErr Executor::FlushVol(StringPtr voln, INTEGER vrn) /* IMIV-108 */
{
    ParamBlockRec pbr;

    pbr.ioParam.ioNamePtr = RM(voln);
    pbr.ioParam.ioVRefNum = CW(vrn);
    return (PBFlushVol(&pbr, 0));
}

PUBLIC OSErr Executor::UnmountVol(StringPtr voln, INTEGER vrn) /* IMIV-108 */
{
    ParamBlockRec pbr;

    pbr.ioParam.ioNamePtr = RM(voln);
    pbr.ioParam.ioVRefNum = CW(vrn);
    return (PBUnmountVol(&pbr));
}

PUBLIC OSErr Executor::Eject(StringPtr voln, INTEGER vrn) /* IMIV-108 */
{
    ParamBlockRec pbr;

    pbr.ioParam.ioNamePtr = RM(voln);
    pbr.ioParam.ioVRefNum = CW(vrn);
    return (PBEject(&pbr));
}

static VCB *findvcb(StringPtr, INTEGER, BOOLEAN *, GUEST<INTEGER> *);
static VCB *grabvcb(ParmBlkPtr, GUEST<INTEGER> *);

PRIVATE VCB * findvcb(StringPtr sp, INTEGER vrn, BOOLEAN * iswd, GUEST<INTEGER> * vrnp)
{
    VCB *vcbptr;

    *iswd = false;
    if(sp && sp[U(sp[0])] == VOLCHAR)
    {
        for(vcbptr = (VCB *)MR(VCBQHdr.qHead);
            vcbptr && !EqualString(sp, (StringPtr)vcbptr->vcbVN, false, true);
            vcbptr = (VCB *)MR(vcbptr->qLink))
            ;
        *vrnp = vcbptr->vcbVRefNum;
    }
    else
    {
        if(vrn == 0)
        {
            if(DefVCBPtr)
                *vrnp = MR(DefVCBPtr)->vcbVRefNum;
            else
                *vrnp = 0;
            /*-->*/ return MR(DefVCBPtr);
        }
        if(vrn < 0)
        {
            *vrnp = CW(vrn);
            if(ISWDNUM(vrn))
            {
                *iswd = true;
// FIXME: #warning autc04 ### This is a guess. The original was missing the MR.
                vcbptr = MR(WDNUMTOWDP(vrn)->vcbp);
            }
            else
                vcbptr = ROMlib_vcbbyvrn(vrn);
        }
        else
        {
            for(vcbptr = (VCB *)MR(VCBQHdr.qHead);
                vcbptr && Cx(vcbptr->vcbDrvNum) != vrn;
                vcbptr = (VCB *)MR(vcbptr->qLink))
                ;
            if(vcbptr)
                *vrnp = vcbptr->vcbVRefNum;
            else
                *vrnp = 0;
        }
    }
    return vcbptr;
}

PRIVATE VCB * grabvcb(ParmBlkPtr pb, GUEST<INTEGER> * vrefnump)
{
    INTEGER i;
    VCB *vcbp;
    StringPtr sp;
    LONGINT dir;
    char *therest;

    therest = 0;
    dir = 1;
    if((i = Cx(pb->volumeParam.ioVolIndex)) > 0)
        for(vcbp = (VCB *)MR(VCBQHdr.qHead); i > 1 && vcbp;
            vcbp = (VCB *)MR(vcbp->qLink), i--)
            ;
    else if(Cx(pb->volumeParam.ioVolIndex) == 0)
    {
        sp = MR(pb->volumeParam.ioNamePtr);
        pb->volumeParam.ioNamePtr = 0;
        vcbp = ROMlib_breakoutioname(pb, &dir, &therest, (BOOLEAN *)0, false);
        pb->volumeParam.ioNamePtr = RM(sp);
    }
    else
    {
        BOOLEAN use_defaults;
        unsigned char *p;

        p = (unsigned char *)MR(pb->volumeParam.ioNamePtr);
        if(!p || p[0] == 0 || p[1] == ':')
            use_defaults = true;
        else
            use_defaults = !pstr_index_after(p, ':', 1);
        vcbp = ROMlib_breakoutioname(pb, &dir, &therest, (BOOLEAN *)0,
                                     use_defaults);
    }
    free(therest);
    if(vcbp)
    {
        if(dir <= 2)
            *vrefnump = vcbp->vcbVRefNum;
        else
        { /* working dir id */
            if(pb->volumeParam.ioVRefNum != CWC(0))
                *vrefnump = pb->volumeParam.ioVRefNum;
            else
                *vrefnump = DefVRefNum;
        }
    }

    return vcbp;
}

PRIVATE unsigned short
find_pseudo_block_size(long n_blocks, long block_size)
{
    unsigned short retval;
    long long total_blockage;

    if(!block_size) /* can happen when dealing with stale NFS handles */
        block_size = 1024;

    total_blockage = (long long)n_blocks * block_size;

    if(total_blockage >= (unsigned long)1 << 31)
        total_blockage = ((unsigned long)1 << 31) - 512;

    if(n_blocks < (1 << 16))
        retval = block_size;
    else
    {
        retval = (block_size + 511) / 512 * 512;
        while(total_blockage / retval >= (1 << 16))
            retval += 512;
    }

    if(retval == 0)
    {
        /* shouldn't be possible to get here */
        warning_unexpected("n_blocks = %ld, block_size = %ld", n_blocks,
                           block_size);
        retval = 1024;
    }
    return retval;
}

PRIVATE VCB * common(ParmBlkPtr pb)
{
    VCB *vcbp;
    struct statfs sbuf;

    vcbp = grabvcb(pb, &pb->volumeParam.ioVRefNum);
    if(vcbp)
    {
        if(Ustatfs(((VCBExtra *)vcbp)->unixname, &sbuf) == 0)
        {
            LONGINT effective_free_blocks;
            unsigned long pseudo_block_size;
            long nm_al_blks;
            long free_bks;
            unsigned short short_nm_al_blks;
            unsigned short short_free_bks;

            pseudo_block_size = find_pseudo_block_size(sbuf.f_blocks,
                                                       sbuf.f_bsize);
            vcbp->vcbAlBlkSiz = CL(pseudo_block_size);
            nm_al_blks = ((long long)sbuf.f_blocks * sbuf.f_bsize
                          / pseudo_block_size);
            short_nm_al_blks = MIN(nm_al_blks, 65535);
            vcbp->vcbNmAlBlks = CW(short_nm_al_blks);
            effective_free_blocks = geteuid() ? sbuf.f_bavail : sbuf.f_bfree;
            free_bks = ((long long)effective_free_blocks * sbuf.f_bsize
                        / pseudo_block_size);

            short_free_bks = MIN(free_bks, 65535);
            vcbp->vcbFreeBks = CW(short_free_bks);
            vcbp->vcbFilCnt = CL(sbuf.f_files);
        }
        if(pb->volumeParam.ioNamePtr)
            str255assign(MR(pb->volumeParam.ioNamePtr), vcbp->vcbVN);
        pb->volumeParam.ioVCrDate = vcbp->vcbCrDate;
        pb->volumeParam.ioVLsBkUp = vcbp->vcbVolBkUp;
        pb->volumeParam.ioVAtrb = vcbp->vcbAtrb;
        pb->volumeParam.ioVNmFls = vcbp->vcbNmFls;
        pb->volumeParam.ioVNmAlBlks = vcbp->vcbNmAlBlks;
        pb->volumeParam.ioVAlBlkSiz = vcbp->vcbAlBlkSiz;
        pb->volumeParam.ioVClpSiz = vcbp->vcbClpSiz;
        pb->volumeParam.ioAlBlSt = vcbp->vcbAlBlSt;
        pb->volumeParam.ioVNxtFNum = vcbp->vcbNxtCNID;
        pb->volumeParam.ioVFrBlk = vcbp->vcbFreeBks;
    }
    return vcbp;
}

PUBLIC OSErr Executor::ufsPBGetVInfo(ParmBlkPtr pb, /* INTERNAL */
   BOOLEAN a)
{
    OSErr err = noErr;
    VCB *vcbp;

    vcbp = common(pb);
    if(!vcbp)
        err = nsvErr;
    return err;
}

PUBLIC OSErr Executor::ufsPBHGetVInfo(HParmBlkPtr pb, /* INTERNAL */
   BOOLEAN a)
{
    OSErr err = noErr;
    VCB *vcbp;
    wdentry *wdp;

    vcbp = common((ParmBlkPtr)pb);
    if(!vcbp)
        err = nsvErr;
    else
    {
        pb->volumeParam.ioVRefNum = vcbp->vcbVRefNum;
        pb->volumeParam.ioVSigWord = vcbp->vcbSigWord;
        pb->volumeParam.ioVDrvInfo = vcbp->vcbDrvNum;
        pb->volumeParam.ioVDRefNum = vcbp->vcbDRefNum;
        pb->volumeParam.ioVFSID = vcbp->vcbFSID;
        pb->volumeParam.ioVBkUp = vcbp->vcbVolBkUp;
        pb->volumeParam.ioVSeqNum = vcbp->vcbVSeqNum;
        pb->volumeParam.ioVWrCnt = vcbp->vcbWrCnt;
        pb->volumeParam.ioVFilCnt = vcbp->vcbFilCnt;
        pb->volumeParam.ioVDirCnt = vcbp->vcbDirCnt;
        BlockMoveData((Ptr)vcbp->vcbFndrInfo, (Ptr)pb->volumeParam.ioVFndrInfo,
                      (Size)sizeof(vcbp->vcbFndrInfo));
        if(ISWDNUM(Cx(BootDrive)))
        {
            wdp = WDNUMTOWDP(Cx(BootDrive));
            if(MR(wdp->vcbp) == vcbp)
                pb->volumeParam.ioVFndrInfo[1] = wdp->dirid;
        }
    }
    return err;
}

PUBLIC OSErr Executor::ufsPBSetVInfo(HParmBlkPtr pb, /* INTERNAL */
   BOOLEAN a)
{
    VCB *vcbp;
    int ntocopy;
    OSErr err;
    BOOLEAN iswd;
    GUEST<INTEGER> temp;

    err = noErr;

    if((vcbp = findvcb(MR(pb->volumeParam.ioNamePtr), Cx(pb->volumeParam.ioVRefNum),
                       &iswd, &temp))
       && !iswd)
    {
        if(pb->volumeParam.ioNamePtr)
        {
            ntocopy = MIN(MR(pb->volumeParam.ioNamePtr)[0],
                          sizeof(vcbp->vcbVN) - 1);
            BlockMoveData((Ptr)MR(pb->volumeParam.ioNamePtr) + 1,
                          (Ptr)vcbp->vcbVN + 1, (Size)ntocopy);
            vcbp->vcbVN[0] = ntocopy;
        }
        vcbp->vcbCrDate = pb->volumeParam.ioVCrDate;
        vcbp->vcbLsMod = pb->volumeParam.ioVLsMod;
        vcbp->vcbAtrb = pb->volumeParam.ioVAtrb;
        vcbp->vcbClpSiz = pb->volumeParam.ioVClpSiz;
        vcbp->vcbVolBkUp = pb->volumeParam.ioVBkUp;
        vcbp->vcbVSeqNum = pb->volumeParam.ioVSeqNum;
        BlockMoveData((Ptr)pb->volumeParam.ioVFndrInfo, (Ptr)vcbp->vcbFndrInfo,
                      (Size)sizeof(vcbp->vcbFndrInfo));
    }
    else
        err = nsvErr;
    return err;
}

PUBLIC OSErr Executor::ufsPBFlushVol(ParmBlkPtr pb, /* INTERNAL */
   BOOLEAN a)
{
    OSErr err = noErr;

#ifndef WIN32
    if(!ROMlib_nosync)
        sync();
#endif
    return err;
}

/*
 * TODO: think about what it really means to unmount a UNIX volume...
 *	 we could probably free up some more memory if it's worth it.
 */

PUBLIC OSErr Executor::ufsPBUnmountVol(ParmBlkPtr pb) /* INTERNAL */
{
    OSErr temp;
    int i;
    VCBExtra *vcbp;

    temp = PBFlushVol(pb, false);
    for(i = 0; i < NFCB; i++)
        if(ROMlib_fcblocks[i].fdfnum && MR(ROMlib_fcblocks[i].fcvptr)->vcbVRefNum == pb->ioParam.ioVRefNum)
            FSClose(i * 94 + 2);
    if((vcbp = (VCBExtra *)ROMlib_vcbbyvrn(Cx(pb->ioParam.ioVRefNum))))
        ROMlib_dbm_close(vcbp);
    if(temp != noErr)
        return (temp);
    else
        return (nsDrvErr);
}

PUBLIC OSErr Executor::ufsPBOffLine(ParmBlkPtr pb) /* INTERNAL */
{
    OSErr temp;

    temp = PBFlushVol(pb, false);
    if(temp != noErr)
        return (temp);
    else
        return (noErr);
}

PUBLIC OSErr Executor::ufsPBEject(ParmBlkPtr pb) /* INTERNAL */
{
    OSErr temp;

    temp = PBFlushFile(pb, false);
    if(temp != noErr)
        return (temp);
    temp = PBOffLine(pb);
    if(temp != noErr)
        return (temp);
    return (nsDrvErr);
}
