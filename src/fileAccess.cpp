/* Copyright 1986-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in FileMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "FileMgr.h"
#include "OSEvent.h"
#include "VRetraceMgr.h"
#include "OSUtil.h"
#include "DeviceMgr.h"
#include "MemoryMgr.h"

#include "rsys/hfs.h"

#include "rsys/file.h"
#include "rsys/glue.h"
#include "rsys/filedouble.h"
#include "rsys/segment.h"
#include "rsys/string.h"
#include "rsys/file.h"
#include "rsys/suffix_maps.h"
#include "rsys/lockunlock.h"
#include "rsys/prefs.h"

#include <ctype.h>

#if defined(CYGWIN32)
#include "winfs.h"
#endif

using namespace Executor;

namespace Executor
{
int ROMlib_nosync = 0; /* if non-zero, we don't call sync () or fsync () */
}

static OSErr PBLockUnlockRange(ParmBlkPtr, BOOLEAN, lockunlock_t);
static VCB *vlookupbydrive(INTEGER);
static char *dirindex(char *, LONGINT, BOOLEAN, struct stat *);
static BOOLEAN myscandir(char *, char, char *, INTEGER);
static LONGINT weasel(char *, struct stat *);
static OSErr macopen(char *, short, LONGINT *, BOOLEAN, Byte *);
static void freeprn(fcbrec *);
static OSErr getprn(INTEGER *);
static OSErr PBOpenForkD(ParmBlkPtr, BOOLEAN, ForkType, LONGINT);
static OSErr PBLockUnlockRange(ParmBlkPtr, BOOLEAN a, lockunlock_t op);

#if !defined(NDEBUG)
void Executor::fs_err_hook(OSErr err)
{
}
#endif

int ROMlib_lasterrnomapped;

#define MAX_ERRNO 50

#define install_errno(uerr, merr)         \
    do                                    \
    {                                     \
        gui_assert(uerr < NELEM(xtable)); \
        xtable[uerr] = merr;              \
    } while(false);

OSErr Executor::ROMlib_maperrno() /* INTERNAL */
{
    OSErr retval;
    static OSErr xtable[MAX_ERRNO + 1];
    static char been_here = false;
    int errno_save;

    if(!been_here)
    {
        int i;

        for(i = 0; i < (int)NELEM(xtable); ++i)
            xtable[i] = fsDSIntErr;

        install_errno(0, noErr);
        install_errno(EPERM, permErr);
        install_errno(ENOENT, fnfErr);
        install_errno(EIO, ioErr);
        install_errno(ENXIO, paramErr);
        install_errno(EBADF, fnOpnErr);
        install_errno(EAGAIN, fLckdErr);
        install_errno(ENOMEM, memFullErr);
        install_errno(EACCES, permErr);
        install_errno(EFAULT, paramErr);
        install_errno(EBUSY, fBsyErr);
        install_errno(EEXIST, dupFNErr);
        install_errno(EXDEV, fsRnErr);
        install_errno(ENODEV, nsvErr);
        install_errno(ENOTDIR, dirNFErr);
        install_errno(EINVAL, paramErr);
        install_errno(ENFILE, tmfoErr);
        install_errno(EMFILE, tmfoErr);
        install_errno(EFBIG, dskFulErr);
        install_errno(ENOSPC, dskFulErr);
        install_errno(ESPIPE, posErr);
        install_errno(EROFS, wPrErr);
        install_errno(EMLINK, dirFulErr);
#if !defined(WIN32)
        install_errno(ETXTBSY, fBsyErr);
        install_errno(EWOULDBLOCK, permErr);
#endif

        been_here = true;
    }

    errno_save = errno;
    ROMlib_lasterrnomapped = errno_save;

    if(errno_save < 0 || errno_save >= (int)NELEM(xtable))
        retval = fsDSIntErr;
    else
        retval = xtable[errno_save];

    if(retval == fsDSIntErr)
        warning_unexpected("fsDSIntErr errno = %d", errno_save);

    if(retval == dirNFErr)
        warning_trace_info("dirNFErr errno = %d", errno_save);

    fs_err_hook(retval);
    return retval;
}

OSErr Executor::FSOpen(StringPtr filen, INTEGER vrn, INTEGER *rn) /* IMIV-109 */
{
    ParamBlockRec pbr;
    OSErr temp;

    pbr.ioParam.ioNamePtr = RM(filen);
    pbr.ioParam.ioVRefNum = CW(vrn);
    pbr.ioParam.ioVersNum = 0;
    pbr.ioParam.ioPermssn = fsCurPerm;
    pbr.ioParam.ioMisc = 0;
    temp = PBOpen(&pbr, 0);
    *rn = CW(pbr.ioParam.ioRefNum);
    fs_err_hook(temp);
    return (temp);
}

OSErr Executor::OpenRF(StringPtr filen, INTEGER vrn, INTEGER *rn) /* IMIV-109 */
{
    ParamBlockRec pbr;
    OSErr temp;

    pbr.ioParam.ioNamePtr = RM(filen);
    pbr.ioParam.ioVRefNum = CW(vrn);
    pbr.ioParam.ioVersNum = 0;
    pbr.ioParam.ioPermssn = fsCurPerm;
    pbr.ioParam.ioMisc = 0;
    temp = PBOpenRF(&pbr, 0);
    *rn = CW(pbr.ioParam.ioRefNum);
    fs_err_hook(temp);
    return (temp);
}

OSErr Executor::FSRead(INTEGER rn, LONGINT *count, Ptr buffp) /* IMIV-109 */
{
    ParamBlockRec pbr;
    OSErr temp;

    pbr.ioParam.ioRefNum = CW(rn);
    pbr.ioParam.ioBuffer = RM(buffp);
    pbr.ioParam.ioReqCount = CL(*count);
    pbr.ioParam.ioPosMode = CWC(fsAtMark);
    temp = PBRead(&pbr, 0);
    *count = CL(pbr.ioParam.ioActCount);
    fs_err_hook(temp);
    return (temp);
}

OSErr
Executor::FSReadAll(INTEGER rn, LONGINT *countp, Ptr buffp)
{
    LONGINT orig_count;
    OSErr retval;

    orig_count = *countp;
    retval = FSRead(rn, countp, buffp);
    if(retval == noErr && *countp != orig_count)
        retval = eofErr;
    return retval;
}

OSErr Executor::FSWrite(INTEGER rn, LONGINT *count, Ptr buffp) /* IMIV-110 */
{
    ParamBlockRec pbr;
    OSErr temp;

    pbr.ioParam.ioRefNum = CW(rn);
    pbr.ioParam.ioBuffer = RM(buffp);
    pbr.ioParam.ioReqCount = CL(*count);
    pbr.ioParam.ioPosMode = CWC(fsAtMark);
    temp = PBWrite(&pbr, 0);
    *count = CL(pbr.ioParam.ioActCount);
    fs_err_hook(temp);
    return (temp);
}

OSErr
Executor::FSWriteAll(INTEGER rn, LONGINT *countp, Ptr buffp)
{
    LONGINT orig_count;
    OSErr retval;

    orig_count = *countp;
    retval = FSWrite(rn, countp, buffp);
    if(retval == noErr && *countp != orig_count)
        retval = ioErr;
    return retval;
}

OSErr Executor::GetFPos(INTEGER rn, LONGINT *filep) /* IMIV-110 */
{
    ParamBlockRec pbr;
    OSErr temp;

    pbr.ioParam.ioRefNum = CW(rn);
    temp = PBGetFPos(&pbr, 0);
    *filep = CL(pbr.ioParam.ioPosOffset);
    fs_err_hook(temp);
    return (temp);
}

OSErr Executor::SetFPos(INTEGER rn, INTEGER posmode,
                        LONGINT possoff) /* IMIV-110 */
{
    ParamBlockRec pbr;
    OSErr err;

    pbr.ioParam.ioRefNum = CW(rn);
    pbr.ioParam.ioPosMode = CW(posmode);
    pbr.ioParam.ioPosOffset = Cx(possoff);
    err = PBSetFPos(&pbr, 0);
    fs_err_hook(err);
    return err;
}

OSErr Executor::GetEOF(INTEGER rn, LONGINT *eof) /* IMIV-111 */
{
    ParamBlockRec pbr;
    OSErr temp;

    pbr.ioParam.ioRefNum = Cx(rn);
    temp = PBGetEOF(&pbr, 0);
    *eof = CL(pbr.ioParam.ioMisc);
    fs_err_hook(temp);
    return (temp);
}

OSErr Executor::SetEOF(INTEGER rn, LONGINT eof) /* IMIV-111 */
{
    ParamBlockRec pbr;
    OSErr err;

    pbr.ioParam.ioRefNum = Cx(rn);
    pbr.ioParam.ioMisc = Cx(eof);
    err = PBSetEOF(&pbr, 0);
    fs_err_hook(err);
    return (err);
}

OSErr Executor::Allocate(INTEGER rn, GUEST<LONGINT> *count) /* IMIV-112 */
{
    ParamBlockRec pbr;
    OSErr temp;

    pbr.ioParam.ioRefNum = Cx(rn);
    pbr.ioParam.ioReqCount = *count;
    temp = PBAllocate(&pbr, 0);
    *count = pbr.ioParam.ioActCount;
    fs_err_hook(temp);
    return (temp);
}

OSErr Executor::AllocContig(INTEGER rn, GUEST<LONGINT> *count)
{
    ParamBlockRec pbr;
    OSErr temp;

    pbr.ioParam.ioRefNum = Cx(rn);
    pbr.ioParam.ioReqCount = *count;
    temp = PBAllocContig(&pbr, 0);
    *count = pbr.ioParam.ioActCount;
    fs_err_hook(temp);
    return (temp);
}

OSErr Executor::FSClose(INTEGER rn) /* IMIV-112 */
{
    ParamBlockRec pbr;
    OSErr err;

    pbr.ioParam.ioRefNum = Cx(rn);
    err = PBClose(&pbr, 0);
    fs_err_hook(err);
    return err;
}

VCB *Executor::vlookupbyname(const char *namep, const char *endp)
{
    VCB *retval;
    int n;

    n = endp - namep;
    for(retval = (VCB *)MR(LM(VCBQHdr).qHead); retval;
        retval = (VCB *)MR(retval->qLink))
        if(n == retval->vcbVN[0] && strncmp(namep, (char *)retval->vcbVN + 1, n) == 0)
            return retval;

#if defined(MSDOS) || defined(CYGWIN32)
    for(retval = (VCB *)MR(LM(VCBQHdr).qHead); retval;
        retval = (VCB *)MR(retval->qLink))
        if(n == retval->vcbVN[0] && strncasecmp(namep, (char *)retval->vcbVN + 1, n) == 0)
            return retval;
#endif
    return 0;
}

static VCB *vlookupbydrive(INTEGER drive)
{
    VCB *retval;

    for(retval = (VCB *)MR(LM(VCBQHdr).qHead); retval;
        retval = (VCB *)MR(retval->qLink))
        if(drive == Cx(retval->vcbDrvNum))
            return retval;
    return 0;
}

static LONGINT cacheindex = 0x7fffffff;

void Executor::ROMlib_rewinddir()
{
    cacheindex = 0x7fffffff;
}

int Executor::ROMlib_no_dot_files = false;

static bool
dislike_name(char *namep)
{
    bool retval = false;

    if(strcmp(namep, ".") == 0
       || strcmp(namep, "..") == 0
       || strcmp(namep, ".AppleDouble") == 0
       || strcmp(namep, ".AppleDesktop") == 0
       || (ROMlib_no_dot_files && namep[0] == '.'))
        retval = true;
    else
    {
        char *buf;
        int namelen;

        namelen = strlen(namep);
        buf = (char *)alloca(namelen);
        memcpy(buf, namep, namelen);
        namelen -= ROMlib_UNIX7_to_Mac(buf, namelen);
        retval = namelen > 31;
    }
    return retval;
}

static char *dirindex(char *dir, LONGINT index, BOOLEAN nodirectories,
                      struct stat *sbufp)
{
    GUEST<THz> saveZone;
    static char *cachedir = 0, *cachedirend;
    static DIR *dirp = 0;
#if defined(USE_STRUCT_DIRECT)
    struct direct *dp;
#else
    struct dirent *dp;
#endif
    int dirnamelen;
    struct stat sbuf;

#if defined(MSDOS) || defined(CYGWIN32)
    while(dir[0] == '/' && dir[1] == '/')
        ++dir;
#endif

    if(!sbufp)
        sbufp = &sbuf;
    if(index <= cacheindex || !cachedir || strcmp(dir, cachedir) != 0)
    {
        if(cachedir)
            free(cachedir);
        if(dirp)
            closedir(dirp);
        dirp = Uopendir(dir);
        if(!dirp)
        {
            dp = 0;
            goto DONE;
        }
        cacheindex = 0;
        dirnamelen = strlen(dir);

#if !defined(MAXNAMLEN)
#define MAXNAMLEN 1024
#endif /* !defined(MAXNAMLEN) */

        saveZone = LM(TheZone);
        LM(TheZone) = LM(SysZone);
        cachedir = (char *)malloc(dirnamelen + 1 + MAXNAMLEN + 1);
        LM(TheZone) = saveZone;
        if(!cachedir)
            /*-->*/ return 0;
        strcpy(cachedir, dir);
        cachedirend = cachedir + dirnamelen;
    }
    *cachedirend = '/';
    do
    {
        if((dp = readdir(dirp)))
        {
            strcpy(cachedirend + 1, dp->d_name);
            if(!dislike_name(dp->d_name)
               && !ROMlib_isresourcefork(cachedir)
               && Ustat(cachedir, sbufp) == 0)
            {
                if(!nodirectories || !S_ISDIR(sbufp->st_mode))
                    ++cacheindex;
            }
        }
    } while(dp && cacheindex != index);
    *cachedirend = 0;
DONE:
    return dp ? dp->d_name : 0;
}

/*
 * ROMlib_breakoutioname's role is to examine ioNamePtr and vRefNum
 * to determine the vcbp, but since it has to look at ioNamePtr
 * anyway it makes sense to make a unix partial pathname for the
 * rest.  While we're at it, if we get passed in a working-director
 * instead of a true vrn we might as well note the directory id
 * that is associated with it.
 */

/*
 * BUG Fix TODO: newunixfrommac needs to be more complex; specifically
 *		 it needs to handle :: properly (which is to go up
 *		 a directory, UNLESS it's already at the top).
 */

VCB *Executor::ROMlib_breakoutioname(ParmBlkPtr pb, /* INTERNAL */
                                            LONGINT *diridp, char **therestp, BOOLEAN *fullpathp, BOOLEAN usedefault)
{
    VCB *retval;
    char *colon;
    unsigned char *p;
    INTEGER v;
    wdentry *wdp;

    retval = 0;
    *diridp = 0;
    *therestp = 0;
    if(fullpathp)
        *fullpathp = false;
    if((p = (unsigned char *)MR(pb->ioParam.ioNamePtr)))
    {
        if(p[0] > 1 && p[1] != ':')
        {
            if((colon = pstr_index_after(p, ':', SLASH_CHAR_OFFSET)))
            {
                retval = vlookupbyname((char *)p + 1, colon);
                if(colon - ((char *)p + 1) < p[0])
                    *therestp = ROMlib_newunixfrommac(colon + 1,
                                                      p[0] + (char *)p - colon);
                if(fullpathp)
                    *fullpathp = true;
            }
            else
                *therestp = ROMlib_newunixfrommac((char *)p + 1, p[0]);
        }
        else if(p[0] > 0)
        {
            if(p[1] == ':' || (p[0] == 1 && p[1] == '/'))
                *therestp = ROMlib_newunixfrommac((char *)p + 2, p[0] - 1);
            else
                *therestp = ROMlib_newunixfrommac((char *)p + 1, p[0]);
        }
    }
    if(!retval)
    {
        if((v = Cx(pb->ioParam.ioVRefNum)) > 0) /* drive number */
            retval = vlookupbydrive(v);
        else if(v < 0)
        {
            if(ISWDNUM(v))
            {
                wdp = WDNUMTOWDP(v);
                *diridp = CL(wdp->dirid);
                retval = MR(wdp->vcbp);
            }
            else
                retval = ROMlib_vcbbyvrn(v);
        }
        if(!retval && (usedefault || (!pb->ioParam.ioNamePtr && !pb->ioParam.ioVRefNum)))
        {
            retval = MR(LM(DefVCBPtr));
            *diridp = CL(DefDirID);
        }
    }
    return retval;
}

/*
 * myscandir looks through a directory for a filename match with case not
 *	     significant.  It returns true if a match was found; false
 *	     otherwise.  Note, the filename specified in "rest" starts
 *	     out smashed and needs the character stored in "save" to
 *	     be filled in before it can be used, but this has to be
 *	     done after "dirname" has been used.
 */

static BOOLEAN myscandir(char *dirname, char save, char *rest, INTEGER restlen)
{
    DIR *dirp;
#if defined(USE_STRUCT_DIRECT)
    struct direct *directp;
#else
    struct dirent *directp;
#endif
    int retval;
    LONGINT d0;

#if defined(MSDOS) || defined(CYGWIN32)
    while(dirname[0] == '/' && dirname[1] == '/')
        ++dirname;
#endif

    retval = false;
    if((dirp = Uopendir(dirname)))
    {
        *rest = save;
        d0 = ((LONGINT)restlen << 16) | (unsigned short)restlen;
        while((directp = readdir(dirp)))
            if(restlen == (INTEGER)strlen(directp->d_name) && ROMlib_RelString((unsigned char *)rest, (unsigned char *)directp->d_name, false, true, d0) == 0)
            {
                retval = true;
                memcpy(rest, directp->d_name, restlen);
                /*-->*/ break;
            }
        closedir(dirp);
    }
    else
        *rest = save;
    return retval;
}

static LONGINT weasel(char *fullpathname, struct stat *sbufp)
{
    char *start, *nextslash, save;
    INTEGER length;
    LONGINT retval;

#if 1
    gui_assert(fullpathname[SLASH_CHAR_OFFSET] == '/');
#else
    if(fullpathname[SLASH_CHAR_OFFSET] != '/')
    {
        DEBUG_OUT("weasel surprise %s", fullpathname);
        *(long *)-1 = -1;
    }
#endif

    if(Ustat(fullpathname, sbufp) < 0)
    {
        switch(errno)
        {
#if !defined(CYGWIN32)
            case ENOENT:
#if 0
	  /* this will only work on systems that distinguish
	     between ENOENT and ENOTDIR ... maybe it won't work
	     at all.  It doesn't work under Linux, so omit it
	     for now */
	    start = strrchr(fullpathname, '/') + 1;
	    break;
#endif
            case ENOTDIR:
                start = fullpathname + SLASH_CHAR_OFFSET + 1;
                break;
#endif
            default:
                return errno;
                break;
        }
    }
    else
        return 0;
    do
    {
        nextslash = strchr(start, '/');
        if(nextslash)
            length = nextslash - start;
        else
            length = strlen(start);
        save = *start;
        *start = 0;
        if(!myscandir(fullpathname, save, start, length))
        {
            return nextslash ? ENOTDIR : ENOENT;
        }
        start = nextslash + 1;
    } while(nextslash);
    if(Ustat(fullpathname, sbufp) < 0)
    {
        retval = errno;
        return retval;
    }
    else
        return 0;
}

/*
 * NOTE: As of today, ROMlib_nami always either returns an error, or leaves
 *	 a valid result in *sbufp (if sbufp is non-zero).  That means that
 *	 you can call ROMlib_nami and not have to do a stat afterward.
 */

OSErr Executor::ROMlib_nami(ParmBlkPtr pb, LONGINT dir, IndexType indextype,
                            char **pathname, char **filename, char **endname,
                            BOOLEAN nodirs, VCBExtra **vcbpp,
                            struct stat *sbufp) /* INTERNAL */
{
    char *temp1, *temp2, *temp3;
    INTEGER index;
    StringPtr fnamep;
    Str255 fname;
    LONGINT dirid;
    char *therest;
    char *fn;
    VCBExtra *vcbp;
    datum content;
    char *vcbanddir;
    int vcbstrlen, therestlen;
    StringPtr savenamep;
    BOOLEAN fullp;
    struct stat sbuf;
    INTEGER badness;
    OSErr err;
    ALLOCABEGIN

    if(!pathname)
        pathname = &temp1;
    if(!filename)
        filename = &temp2;
    if(!endname)
        endname = &temp3;
    fnamep = 0;
    if(indextype != NoIndex)
    {
        savenamep = MR(pb->ioParam.ioNamePtr);
        pb->ioParam.ioNamePtr = 0;
    }
#if !defined(LETGCCWAIL)
    else
        savenamep = (StringPtr)-1;
#endif /* LETGCCWAIL */
    vcbp = (VCBExtra *)ROMlib_breakoutioname(pb, &dirid, &therest, &fullp,
                                             true);
    if(vcbpp)
        *vcbpp = vcbp;
    if(fullp)
        dir = 0;
    else
    {
        if(dir <= 0)
            dir = dirid;
    }
    content.dsize = 0;
    /*
 * Retry strategy:
 *	first time things don't work we try closing and reopening
 *	the database (badness == 1); second time we try closing,
 *	rebuilding and reopening the database (badness == 2).
 */
    badness = 0;
/* do { */
loop:
    if(dir > 2)
    {
        content = ROMlib_dbm_fetch(vcbp, dir);
        if(!content.dptr)
        {
            content.dsize = 0;
        }
    }
    vcbstrlen = strlen(vcbp->unixname);
    vcbanddir = (char *)ALLOCA(vcbstrlen + content.dsize + 2);
    memcpy(vcbanddir, vcbp->unixname, vcbstrlen + 1);
    if(content.dsize)
    {
        if(vcbanddir[vcbstrlen - 1] != '/')
        {
            vcbanddir[vcbstrlen] = '/';
            ++vcbstrlen;
        }
        memcpy(vcbanddir + vcbstrlen, content.dptr, content.dsize);
        vcbanddir[vcbstrlen + content.dsize] = 0;
        if(Ustat(vcbanddir, &sbuf) < 0)
        {
            warning_unexpected("vcbanddir = '%s', dir = 0x%x",
                               vcbanddir, dir);
            ROMlib_dbm_delete_inode(vcbp, dir);
            /*-->*/ goto loop;
        }
        else if(ST_INO(sbuf) != dir)
        {
#if 1
            warning_unexpected("ST_INO(sbuf) = 0x%lx, dir = 0x%x",
                               (unsigned long)ST_INO(sbuf), dir);
            ROMlib_dbm_delete_inode(vcbp, dir);
            {
                LONGINT templong;

                templong = ST_INO(sbuf);
                ROMlib_dbm_store(vcbp, vcbanddir, &templong, true);
            }
            /*-->*/ goto loop;
#else
            fprintf(stderr, "ino mismatch %s %ld %d\n", vcbanddir,
                    (long)ST_INO(sbuf), dir);
            ++badness;
#endif
        }
        else
            badness = 0;
    }
    else if(dir > 2)
        ++badness;
    if(badness == 1 && dirid)
    {
        dir = dirid;
        badness = 2;
        goto loop;
    }
    if(badness && badness <= 1)
        goto loop;
    /* } while (badness && badness <= 1); */
    if(badness)
    {
        err = dirNFErr;
        warning_trace_info("badness = %d, dirid = %d, dir = %d, "
                           "vcbp->unixname = '%s', therest = '%s', "
                           "fullp = %d, vcbanddir = '%s'",
                           badness, dirid, dir,
                           vcbp->unixname, therest, fullp, vcbanddir);
        fs_err_hook(err);
        free(therest);
        /*-->*/ return err;
    }
    if(indextype != NoIndex)
        pb->ioParam.ioNamePtr = RM(savenamep);
    if(indextype == FDirIndex && (index = Cx(pb->fileParam.ioFDirIndex)) > 0)
    {
        free(therest);
        therest = 0;
        if(!(fn = dirindex(vcbanddir, index, nodirs, sbufp)))
        {
            ALLOCAEND
            err = fnfErr;
            fs_err_hook(err);
            /*-->*/ return err;
        }
        if(pb->ioParam.ioNamePtr)
            fnamep = MR(pb->ioParam.ioNamePtr);
        else
            fnamep = fname;
        fnamep[0] = strlen(fn);
        strcpy((char *)fnamep + 1, fn);
#if 0
	therest = ROMlib_newunixfrommac((char *) fnamep+1, fnamep[0]);
#else
        therest = (char *)malloc((Size)fnamep[0] + 1);
        memcpy(therest, fnamep + 1, fnamep[0]);
        therest[fnamep[0]] = 0;
#endif
    }
    therestlen = therest ? strlen(therest) : 0;
    *pathname = (char *)malloc(therestlen + strlen(vcbanddir) + 2);
    *filename = *pathname + strlen(vcbanddir) + 1;
    *endname = *filename + therestlen + 1;
    memcpy(*pathname, vcbanddir, strlen(vcbanddir));
    if(therest)
    {
        if(vcbanddir[1 + SLASH_CHAR_OFFSET]) /* i.e. test for "/" */
            (*filename)[-1] = '/';
        else
        {
            --*filename;
            --*endname;
        }
        memcpy(*filename, therest, therestlen);
        (*endname)[-1] = 0;
    }
    else
    {
        *endname = *filename;
        (*endname)[-1] = 0;
    }
    if(strcmp(*pathname + SLASH_CHAR_OFFSET, "/") != 0)
    {
        while((*endname)[-2] == '/')
        { /* remove trailing slashes */
            (*endname)[-2] = 0;
            --*endname;
        }
        if((*filename = strrchr(*pathname, '/')))
            ++*filename;
        else
            *filename = *pathname;
    }
    else
        *filename = *pathname;
    if(therest)
        free(therest);
    ALLOCAEND
    if(indextype == NoIndex && sbufp)
    {
        errno = weasel(*pathname, sbufp);
        err = ROMlib_maperrno();
    }
    else if(indextype == IGNORENAME && sbufp)
    {
        if(Ustat(*pathname, sbufp) < 0)
        {
            err = ROMlib_maperrno();
        }
        else
            err = noErr;
    }
    else
        err = noErr;
    fs_err_hook(err);
    if(err == noErr && fullp)
        ROMlib_automount(*pathname);
    return err;
}

/*
 * macopen opens the file using mac permissions, returning an OSErr
 * the file descriptor of the opened file is filled in where fdp points
 */

static OSErr macopen(char *file, short perm, LONGINT *fdp, BOOLEAN isres,
                     Byte *flagp)
{
    int newperm;
    int flockret;
    OSErr err;

    switch(perm)
    {
        default:
            warning_unexpected("perm = %d", perm);
        /* FALL THROUGH */
        case fsCurPerm:
            newperm = O_RDWR; /* start out here, downgrade if necessary */
            *flagp |= fcwriteperm;
            break;
        case fsRdPerm:
            newperm = O_RDONLY;
            break;
        case fsWrPerm:
            *flagp |= fcwriteperm;
            newperm = isres ? O_RDWR : O_WRONLY;
            break;
        case fsRdWrPerm:
            *flagp |= fcwriteperm;
            newperm = O_RDWR;
            break;
        case fsRdWrShPerm:
            *flagp |= fcwriteperm;
            newperm = O_RDWR;
            break;
    }

    *fdp = Uopen(file, O_BINARY | newperm, 0);
    if(*fdp == -1 && perm == fsCurPerm)
    {
        *fdp = Uopen(file, O_BINARY | O_RDONLY, 0);
        *flagp &= ~fcwriteperm;
    }
    flockret = 0;
#if !defined(AVIION) && !defined(MACBLITZ) && 0
    if(*fdp != -1)
    {
        if(perm == fsCurPerm || perm == fsRdWrPerm)
            flockret = flock(*fdp, LOCK_EX | LOCK_NB);
        if(perm == fsRdWrShPerm || (perm == fsCurPerm && flockret == -1))
            flockret = flock(*fdp, LOCK_SH | LOCK_NB);
        if(flockret == -1 && perm != fsCurPerm)
        {
            Uclose(*fdp);
            *fdp = flockret;
        }
    }
#endif /* !defined(AVIION) */
    err = *fdp == -1 ? ROMlib_maperrno() : noErr;
    fs_err_hook(err);
    return err;
}

static void freeprn(fcbrec *fp)
{
    fp->fdfnum = 0;
}

static OSErr getprn(INTEGER *pprn)
{
    INTEGER length;
    fcbrec *fcbp, *efcbp;
    OSErr err;

/* #define	NASTYTEMPHAXK */
#if defined(NASTYTEMPHAXK) && defined(MSDOS)
    static int n = 0;
    *pprn = 2 + 94 * n++;
    return noErr;
#endif
    length = CW(*(GUEST<int16_t> *)MR(LM(FCBSPtr)));
    fcbp = (fcbrec *)((GUEST<int16_t> *)MR(LM(FCBSPtr)) + 1);
    efcbp = (fcbrec *)((char *)MR(LM(FCBSPtr)) + length);
    for(; fcbp < efcbp && fcbp->fdfnum;
        fcbp = (fcbrec *)((char *)fcbp + Cx(LM(FSFCBLen))))
        ;
    if(fcbp < efcbp)
    {
        *pprn = (char *)fcbp - (char *)MR(LM(FCBSPtr));
        err = noErr;
    }
    else
        err = tmfoErr;
    fs_err_hook(err);
    return err;
}

static OSErr PBOpenForkD(ParmBlkPtr pb, BOOLEAN a, ForkType fork, LONGINT dir)
{
    OSErr err;
    int temp;
    INTEGER prn;
    char *pathname, *filename, *endname, *newname;
    struct stat sbuf;
    VCBExtra *vcbp;
    int namelen;
    fcbrec *fp;
    GUEST<THz> savezone;
    int save_index;
    char save_char;
    char *need_to_free;

    need_to_free = 0;
    pb->ioParam.ioRefNum = CWC(0); /* in case some goofy program,
					   like StuffIt Lite decides to
					   ignore error codes */
    savezone = LM(TheZone);
    LM(TheZone) = LM(SysZone);
    pathname = 0;
    /*
 * We have to pass in sbuf below or we won't be weaseled like we should be.
 */
    if((err = ROMlib_nami(pb, dir, NoIndex, &pathname, &filename, &endname,
                          true, &vcbp, (struct stat *)&sbuf))
       == noErr)
    {
        err = getprn(&prn);
        if(err == noErr)
        {
            fp = (fcbrec *)(MR(LM(FCBSPtr)) + prn);
            fp->fdfnum = CL((LONGINT)ST_INO(sbuf));
            fp->fcfd = -1;
            fp->fcflags = 0;
            fp->fcbTypByt = 0;
            fp->fcbCrPs = 0;
            if((filename == pathname + 1) || (filename == pathname + SLASH_CHAR_OFFSET + 1))
                save_index = 0; /* root ... don't scorch leading "/" */
            else
                save_index = -1;
            save_char = filename[save_index];
            filename[save_index] = 0;
            if(Ustat(pathname, &sbuf) < 0) /* NOTE: this is a stat of */
                err = ROMlib_maperrno(); /* the parent's directory, */
            else
            { /* so ROMlib_nami won't do the stat for us */
                if(ST_INO(sbuf) == vcbp->u.ufs.ino)
                    fp->fcparid = CLC(2);
                else
                    fp->fcparid = CL((LONGINT)ST_INO(sbuf));
                filename[save_index] = save_char;
                if(fork == ResourceFork)
                {
                    newname = ROMlib_resname(pathname, filename, endname);
                    temp = Ustat(newname, &sbuf);
                    if(temp < 0 && errno != ENOENT)
                        err = ioErr;
                    need_to_free = pathname; /* can't free it now, because
						filename is drawn from
						here */
                    pathname = newname;
                }
            }
            if(err == noErr)
            {
                err = macopen(pathname, pb->ioParam.ioPermssn,
                              &fp->fcfd, fork == ResourceFork, &fp->fcflags);
                if(err != noErr
                   && (fork != ResourceFork || (err = ROMlib_newresfork(pathname, &fp->fcfd, true))))
                    freeprn(fp);
                else
                {
                    if(fork == ResourceFork)
                    {
                        fp->fcflags |= fcfisres;
                        fp->hiddenfd = fp->fcfd;
                        lseek(fp->fcfd, ROMlib_FORKOFFSET(fp), SEEK_SET);
                    }
                    else
                    {
                        newname = ROMlib_resname(pathname, filename,
                                                 endname);
                        fp->hiddenfd = Uopen(newname, O_BINARY | O_RDWR, 0666L);
                        if(fp->hiddenfd < 0)
                        {
                            Byte dummy_byte = -1;

                            if(errno == ENOENT)
                            {
                                if((fp->fcflags & fcwriteperm) && !ROMlib_creator_and_type_from_filename(strlen(filename), filename, NULL, NULL))
                                    err = ROMlib_newresfork(newname,
                                                            &fp->hiddenfd, true);
                                else
                                {
                                    err = noErr;
                                    fp->hiddenfd = -1;
                                }
                            }
                            else
                                macopen(newname, pb->ioParam.ioPermssn,
                                        &fp->hiddenfd, true, &dummy_byte);
                        }
                        free(newname);
                    }
                    if(err == noErr)
                        err = ROMlib_geteofostype(fp);
                    fp->fcvptr = RM((VCB *)vcbp);
                    pb->ioParam.ioRefNum = CW(prn);
                    namelen = strlen(filename);
                    namelen -= ROMlib_UNIX7_to_Mac(filename, namelen);
                    namelen = MIN(namelen, 31);
                    fp->fcname[0] = namelen;
                    memcpy(fp->fcname + 1, filename, namelen);
                }
            }
        }
    }

    if(need_to_free)
        free(need_to_free);

    if(pathname)
        free(pathname);
    LM(TheZone) = savezone;
    fs_err_hook(err);
    return err;
}

OSErr Executor::ufsPBOpen(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    OSErr err;

    err = PBOpenForkD(pb, a, DataFork, 0);
    fs_err_hook(err);
    return err;
}

OSErr Executor::ufsPBHOpen(HParmBlkPtr pb,
                           BOOLEAN a) /* INTERNAL */ /* IMIV-136 */
{
    OSErr err;

    err = PBOpenForkD((ParmBlkPtr)pb, a, DataFork, CL(pb->fileParam.ioDirID));
    fs_err_hook(err);
    return err;
}

OSErr Executor::OpenDeny(HParmBlkPtr pb, BOOLEAN a) /* IMV-397 */
{ /* HACK */
    HParamBlockRec block;
    OSErr retval;

    block.ioParam.ioCompletion = pb->ioParam.ioCompletion;
    block.ioParam.ioNamePtr = pb->ioParam.ioNamePtr;
    block.ioParam.ioVRefNum = pb->ioParam.ioVRefNum;
    block.ioParam.ioVersNum = 0;
    switch(pb->ioParam.ioPermssn & 3)
    {
        case 0:
            block.ioParam.ioPermssn = fsCurPerm;
            break;
        case 1:
            block.ioParam.ioPermssn = fsRdPerm;
            break;
        case 2:
            block.ioParam.ioPermssn = fsWrPerm;
            break;
        case 3:
            block.ioParam.ioPermssn = fsRdWrPerm;
            break;
    }
    block.ioParam.ioMisc = 0;
    block.fileParam.ioDirID = pb->fileParam.ioDirID;

    retval = PBHOpen(&block, a);
    pb->ioParam.ioResult = block.ioParam.ioResult;
    pb->ioParam.ioRefNum = block.ioParam.ioRefNum;
    fs_err_hook(retval);
    return retval;
}

OSErr Executor::ufsPBOpenRF(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    OSErr err;

    err = PBOpenForkD(pb, a, ResourceFork, 0);
    fs_err_hook(err);
    return err;
}

OSErr Executor::ufsPBHOpenRF(HParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    OSErr err;

    err = PBOpenForkD((ParmBlkPtr)pb, a, ResourceFork,
                      CL(pb->fileParam.ioDirID));
    fs_err_hook(err);
    return err;
}

#define POSMASK 0x3 /* IMIV-121 */

static OSErr
pbfpos(ParmBlkPtr pb, LONGINT *toseekp, bool can_go_past_eof)
{
    OSErr err;
    LONGINT leof;
    INTEGER forkoffset;
    LONGINT pos;
    int fd;
    fcbrec *fp;

    fp = PRNTOFPERR(Cx(pb->ioParam.ioRefNum), &err);
    if(err == noErr)
    {
        forkoffset = FORKOFFSET(fp);
        leof = Cx(fp->fcleof);
        fd = fp->fcfd;

        switch(Cx(pb->ioParam.ioPosMode) & POSMASK)
        {
            case fsAtMark:
                if((pos = lseek(fd, 0L, SEEK_CUR)) == -1)
                {
                    switch(errno)
                    {
                        case EBADF:
                            err = fnOpnErr;
                            break;
                        case EINVAL:
                            err = paramErr;
                            break;
                        default:
                            err = ioErr;
                    }
                }
                else
                    *toseekp = pos - forkoffset;
                break;
            case fsFromStart:
                *toseekp = Cx(pb->ioParam.ioPosOffset);
                break;
            case fsFromLEOF:
                *toseekp = leof + Cx(pb->ioParam.ioPosOffset);
                break;
            case fsFromMark:
                if((pos = lseek(fd, 0L, L_INCR)) == -1)
                {
                    switch(errno)
                    {
                        case EBADF:
                            err = fnOpnErr;
                            break;
                        case EINVAL:
                            err = paramErr;
                            break;
                        default:
                            err = ioErr;
                    }
                }
                else
                    *toseekp = pos + Cx(pb->ioParam.ioPosOffset) - forkoffset;
                break;
        }
        if(*toseekp + forkoffset < 0)
        {
            err = posErr;
            *toseekp = 0;
        }
        else if((*toseekp > leof) && !can_go_past_eof)
        {
            err = eofErr;
            *toseekp = leof;
        }
        *toseekp += forkoffset;
    }
    fs_err_hook(err);
    return err;
}

static OSErr PBLockUnlockRange(ParmBlkPtr pb, BOOLEAN a, lockunlock_t op)
{
    OSErr err;
    LONGINT toseek;
    int fd;
    LONGINT curseek;
    fcbrec *fp;

    err = noErr;
    fp = PRNTOFPERR(Cx(pb->ioParam.ioRefNum), &err);
    if(err == noErr)
    {
        if((err = pbfpos(pb, &toseek, true)) == noErr)
        {
            OSErr (*verify)(int fd, uint32_t start, uint32_t count);
            OSErr (*cleanup)(int fd, uint32_t start, uint32_t count);

            fd = fp->fcfd;
            curseek = lseek(fd, 0, SEEK_SET);

            if(op == lock)
            {
                verify = ROMlib_fd_range_overlap;
                cleanup = ROMlib_fd_add_range;
            }
            else
            {
                verify = ROMlib_find_fd_start_count;
                cleanup = ROMlib_fd_remove_range;
            }

            err = verify(fd, toseek, CL(pb->ioParam.ioReqCount));
            if(err == noErr && lseek(fd, toseek, SEEK_SET) == -1)
                err = ROMlib_maperrno();
            if(err == noErr)
                err = ROMlib_lockunlockrange(fd, toseek,
                                             CL(pb->ioParam.ioReqCount), op);
            if(err == noErr)
                err = cleanup(fd, toseek, CL(pb->ioParam.ioReqCount));
            lseek(fd, curseek, SEEK_SET);
        }
    }
    fs_err_hook(err);
    return err;
}

OSErr Executor::ufsPBLockRange(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    OSErr err;

    err = PBLockUnlockRange(pb, a, lock);
    fs_err_hook(err);
    return err;
}

OSErr Executor::ufsPBUnlockRange(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    OSErr err;

    err = PBLockUnlockRange(pb, a, unlock);
    fs_err_hook(err);
    return err;
}

static OSErr
pbsetfpos(ParmBlkPtr pb, bool can_go_past_eof)
{
    OSErr err;
    INTEGER forkoffset;
    LONGINT toseek;
    int fd;
    fcbrec *fp;

    fp = PRNTOFPERR(Cx(pb->ioParam.ioRefNum), &err);
    if(err == noErr)
    {
        forkoffset = FORKOFFSET(fp);
        err = pbfpos(pb, &toseek, can_go_past_eof);
        fd = fp->fcfd;
        pb->ioParam.ioPosOffset = CL((int)(lseek(fd, toseek, SEEK_SET) - forkoffset));
    }
    fs_err_hook(err);
    return err;
}

int Executor::ROMlib_newlinetocr = true;

/*
 * NOTE: ROMlib_destroy_blocks is a wrapper routine that either destroys
 *	 a limited range or flushes the entire cache.  Apple's semantics
 *	 say that you must flush the entire cache in certain circumstances.
 *	 However, 99% of all programs need to destroy only a range of
 *	 addresses.
 */

int Executor::ROMlib_flushoften = 0;

unsigned long Executor::ROMlib_destroy_blocks(
    syn68k_addr_t start, uint32_t count, BOOLEAN flush_only_faulty_checksums)
{
    unsigned long num_blocks_destroyed;

    if(ROMlib_flushoften)
    {
        start = 0;
        count = ~0;
    }
    if(count)
    {
        if(flush_only_faulty_checksums)
            num_blocks_destroyed = destroy_blocks_with_checksum_mismatch(start, count);
        else
            num_blocks_destroyed = destroy_blocks(start, count);
    }
    else
        num_blocks_destroyed = 0;

    return num_blocks_destroyed;
}

OSErr Executor::ufsPBRead(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    OSErr err;
    LONGINT nread;
    INTEGER forkoffset;
    fcbrec *fp;

#if 0
    if (pb->ioParam.ioRefNum == 0x5003)
      {
	printf ("read IN  mode = %d, offset = %5d, req = %5d\n",
		CW (pb->ioParam.ioPosMode),
		CL (pb->ioParam.ioPosOffset), CL (pb->ioParam.ioReqCount));
      }
#endif

    if(Cx(pb->ioParam.ioRefNum) < 0)
    {
        err = ROMlib_dispatch(pb, a, Prime, aRdCmd);
        fs_err_hook(err);
        return err;
    }
    fp = PRNTOFPERR(Cx(pb->ioParam.ioRefNum), &err);
    if(err == noErr && ((err = pbsetfpos(pb, false)) == noErr || err == eofErr))
    {
        forkoffset = FORKOFFSET(fp);
        if(Cx(pb->ioParam.ioReqCount) < 0)
            err = paramErr;
        else
        {
            int fd = fp->fcfd;
            LONGINT rc = Cx(pb->ioParam.ioReqCount);

            if(Cx(pb->ioParam.ioPosOffset) + rc > Cx(fp->fcleof))
                rc = Cx(fp->fcleof) - Cx(pb->ioParam.ioPosOffset);
            nread = read(fd, (char *)(MR(pb->ioParam.ioBuffer)), rc);
            ROMlib_destroy_blocks(US_TO_SYN68K(MR(pb->ioParam.ioBuffer)),
                                  rc, true);
            if(nread == -1)
            {
                pb->ioParam.ioActCount = 0;
                err = ioErr;
            }
            else
            {
                pb->ioParam.ioActCount = CL(nread);
                pb->ioParam.ioPosOffset = CL((int)(lseek(fd, 0L, L_INCR) - forkoffset));
                if(rc != Cx(pb->ioParam.ioReqCount))
                    err = eofErr;
            }
        }
    }
    fs_err_hook(err);
#if 0
    if (pb->ioParam.ioRefNum == 0x5003)
      {
	printf ("read OUT mode = %d, offset = %5d, req = %5d, act = %d, err = %d\n",
		CW (pb->ioParam.ioPosMode),
		CL (pb->ioParam.ioPosOffset), CL (pb->ioParam.ioReqCount),
		CL (pb->ioParam.ioActCount),
		err);
      }
#endif
    return err;
}

OSErr Executor::ufsPBWrite(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    OSErr err;
    LONGINT nwrite, rc;
    INTEGER forkoffset;
    fcbrec *fp;

    if(Cx(pb->ioParam.ioRefNum) < 0)
    {
        err = ROMlib_dispatch(pb, a, Prime, aWrCmd);
        fs_err_hook(err);
        return err;
    }
    fp = PRNTOFPERR(Cx(pb->ioParam.ioRefNum), &err);
    if(err == noErr && ((err = pbsetfpos(pb, true)) == noErr || err == eofErr))
    {
        forkoffset = FORKOFFSET(fp);
        if((rc = Cx(pb->ioParam.ioReqCount)) < 0)
            err = paramErr;
        else
        {
            int fd = fp->fcfd;
#if defined(WEHAVEENOUGHSWAPSPACETORUNSUNSGOOFYLITTLERPCTHINGYS)
#if defined(SUN)
            if(lockf(fd, F_TLOCK, rc))
                err = fLckdErr;
#endif /* SUN */
#endif /* WEHAVEENOUGHSWAPSPACETORUNSUNSGOOFYLITTLERPCTHINGYS */
            if(err == noErr && Cx(pb->ioParam.ioPosOffset) + rc > Cx(fp->fcleof))
            {
                fp->fcleof = CL(CL(pb->ioParam.ioPosOffset) + rc);
                err = ROMlib_seteof(fp);
            }
            if(err == noErr)
            {
                nwrite = write(fd, (char *)(MR(pb->ioParam.ioBuffer)), rc);
                if(nwrite == -1)
                {
                    pb->ioParam.ioActCount = 0;
                    err = ioErr;
                }
                else
                {
                    err = noErr;
                    pb->ioParam.ioActCount = CL(nwrite);
                    pb->ioParam.ioPosOffset = CL((int)(lseek(fd, 0L, L_INCR) - forkoffset));
                    if(Cx(pb->ioParam.ioPosOffset) > Cx(fp->fcleof))
                    {
                        fp->fcleof = pb->ioParam.ioPosOffset;
                        err = ROMlib_seteof(fp);
                    }
                }
            }
#if defined(WEHAVEENOUGHSWAPSPACETORUNSUNSGOOFYLITTLERPCTHINGYS)
#if defined(SUN)
            lockf(fd, F_ULOCK, Cx(pb->ioParam.ioReqCount));
#endif /* SUN */
#endif /* WEHAVEENOUGHSWAPSPACETORUNSUNSGOOFYLITTLERPCTHINGYS */
        }
    }
    fs_err_hook(err);
    return err;
}

OSErr Executor::ufsPBGetFPos(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    OSErr err;
    INTEGER forkoffset;
    fcbrec *fp;

    fp = PRNTOFPERR(Cx(pb->ioParam.ioRefNum), &err);
    if(err == noErr)
    {
        forkoffset = FORKOFFSET(fp);
        pb->ioParam.ioPosOffset = CL((int)(lseek(fp->fcfd, 0L, L_INCR) - forkoffset));
        pb->ioParam.ioReqCount = pb->ioParam.ioActCount = CL(0);
        pb->ioParam.ioPosMode = CW(0);
    }
    fs_err_hook(err);
    return err;
}

OSErr Executor::ufsPBSetFPos(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    OSErr err = pbsetfpos(pb, false);

#if 0
    if (pb->ioParam.ioRefNum == 0x5003)
      {
	printf ("seek IN  mode = %d, offset = %5d\n",
		CW (pb->ioParam.ioPosMode),
		CL (pb->ioParam.ioPosOffset));
      }
#endif

    if(err == noErr)
        pb->ioParam.ioReqCount = pb->ioParam.ioActCount = 0;
    fs_err_hook(err);
#if 0
    if (pb->ioParam.ioRefNum == 0x5003)
      {
	printf ("seek OUT mode = %d, offset = %5d, err = %d\n",
		CW (pb->ioParam.ioPosMode),
		CL (pb->ioParam.ioPosOffset), err);
      }
#endif
    return err;
}

OSErr Executor::ufsPBGetEOF(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    fcbrec *fp;
    OSErr err;

    fp = PRNTOFPERR(Cx(pb->ioParam.ioRefNum), &err);
    if(err == noErr)
        pb->ioParam.ioMisc = fp->fcleof;
    fs_err_hook(err);
    return err;
}

OSErr Executor::ufsPBSetEOF(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    fcbrec *fp;
    OSErr err;

    fp = PRNTOFPERR(Cx(pb->ioParam.ioRefNum), &err);
    if(err == noErr)
    {
        fp->fcleof = pb->ioParam.ioMisc;
        err = ROMlib_seteof(fp);
    }
    fs_err_hook(err);
    return err;
}

OSErr Executor::ufsPBAllocate(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    fcbrec *fp;
    OSErr err;

    fp = PRNTOFPERR(Cx(pb->ioParam.ioRefNum), &err);
    if(err == noErr)
    {
        fp->fcleof = CL(CL(fp->fcleof) + CL(pb->ioParam.ioReqCount));
        err = ROMlib_seteof(fp);
        pb->ioParam.ioActCount = pb->ioParam.ioReqCount;
    }
    fs_err_hook(err);
    return err;
}

OSErr Executor::ufsPBAllocContig(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    OSErr err;

    /* WARN that calling this might be a sign of a non-portable program */
    err = PBAllocate(pb, a); /* o.k. a lie, but wtf */
    fs_err_hook(err);
    return err;
}

OSErr Executor::ufsPBFlushFile(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    fcbrec *fp;
    OSErr err;

    fp = PRNTOFPERR(Cx(pb->ioParam.ioRefNum), &err);
#ifndef WIN32
    if(!ROMlib_nosync)
        sync();
#endif
    fs_err_hook(err);
    return err;
}

OSErr Executor::ufsPBClose(ParmBlkPtr pb, BOOLEAN a) /* INTERNAL */
{
    OSErr err;
    fcbrec *fp;

    if(Cx(pb->ioParam.ioRefNum) < 0)
    {
        err = ROMlib_dispatch(pb, a, Close, 0);
        fs_err_hook(err);
        return err;
    }
    fp = PRNTOFPERR(Cx(pb->ioParam.ioRefNum), &err);

    if(err == noErr)
    {
        if(Uclose(fp->fcfd) < 0)
        {
            err = ioErr;
        }
        Uclose(fp->hiddenfd);
        freeprn(fp);
    }
    fs_err_hook(err);
    return err;
}

OSErr Executor::PBHGetLogInInfo(HParmBlkPtr pb, BOOLEAN a)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}

OSErr Executor::PBHGetDirAccess(HParmBlkPtr pb, BOOLEAN a)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}

OSErr Executor::PBHCopyFile(HParmBlkPtr pb, BOOLEAN a)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}

OSErr Executor::PBHMapName(HParmBlkPtr pb, BOOLEAN a)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}

OSErr Executor::PBHMapID(HParmBlkPtr pb, BOOLEAN a)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}

OSErr Executor::PBHSetDirAccess(HParmBlkPtr pb, BOOLEAN a)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}

OSErr Executor::PBHMoveRename(HParmBlkPtr pb, BOOLEAN a)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}
