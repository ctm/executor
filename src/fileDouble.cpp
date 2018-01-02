/* Copyright 1992-2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "FileMgr.h"
#include "rsys/file.h"

#ifdef MACOSX
#include <sys/xattr.h>
#endif

#if defined(CYGWIN32)
#include "winfs.h"
#endif

using namespace Executor;

namespace Executor
{
int afpd_conventions_p;
int netatalk_conventions_p;
char apple_double_quote_char;
const char *apple_double_fork_prefix;
int apple_double_fork_prefix_length;
const char *resfork_suffix = "";
int resfork_suffix_length = 0;

bool native_resfork_p = true;
}

static unsigned char tohex(unsigned char);
static INTEGER Mac_to_UNIX7(unsigned char *, INTEGER, unsigned char *);

/*
 * Coded more or less up to the spec:  APDA M0908LL/A
 *				       AppleSingle/AppleDouble Formats
 *				       For Foreign Files Developer's Note
 *
 * I don't like this spec for a variety of reasons:
 *
 *	The idea that you should ever have to look inside a file to determine
 *	whether %father is really 0xfather or the header file for "father"
 *	is messed.  The way we get around this is we always quote the first
 *	if that's really what they want, so the header file for "father"
 *	will be %%66ather (actually we only do that if the two characters
 *	are hex for something that we'd expand normally (i.e. '%', '/', NUL
 *	or something with the high bit on, like 0xFA.
 *
 *	The spec says you can put the descriptors in any order you want, but
 *	it would be much nicer if they had to be sorted by offset.  This is
 *	what we do/assume.
 */

#include "rsys/filedouble.h"
#include "rsys/suffix_maps.h"
#include "rsys/osutil.h"

static defaulthead_t ourdefault;

/*
 * Fill it in by hand, writing a proper constructor would be as much work
 */

#define LEN0 (sizeof(struct defaulthead))
#define LEN1 (LEN0 + sizeof(Single_attribs))
#define LEN2 (LEN1 + sizeof(Single_dates))

static void initialize_ourdefault(void)
{
    static int beenhere = 0;

    if(!beenhere)
    {
        beenhere = 1;
        ourdefault.head.magic = CLC(DOUBLEMAGIC);
        ourdefault.head.version = CLC(SINGLEVERSION);
        ourdefault.head.nentries = CWC(4);

        ourdefault.desc[0].id = CLC(Macintosh_File_Info_ID);
        ourdefault.desc[0].offset = CLC(LEN0);
        ourdefault.desc[0].length = CLC(sizeof(Single_attribs));

        ourdefault.desc[1].id = CLC(File_Dates_Info_ID);
        ourdefault.desc[1].offset = CLC(LEN1);
        ourdefault.desc[1].length = CLC(sizeof(Single_dates));

        ourdefault.desc[2].id = CLC(Finder_Info_ID);
        ourdefault.desc[2].offset = CLC(LEN2);
        ourdefault.desc[2].length = CLC(sizeof(Single_finfo));

        ourdefault.desc[3].id = CLC(Resource_Fork_ID);
        ourdefault.desc[3].offset = CLC(512);
        ourdefault.desc[3].length = CLC(0L);
    }
}

static defaultentries_t ourentries;

void
Executor::double_dir_op(char *name, double_dir_op_t op)
{
    if(!netatalk_conventions_p)
        return;
    char *last_slash;

    last_slash = strrchr(name, '/');
    if(last_slash)
    {
        char save;

        save = *last_slash;
        *last_slash = 0;
        switch(op)
        {
            case mkdir_op:
                Umkdir(name, 0777);
                break;
            case rmdir_op:
            {
                char *to_del, *to_unlink;
                int len;

#define PARENT_FILE ".Parent"

                *last_slash = save;
                len = strlen(name) + 1 + apple_double_fork_prefix_length + 1;
                to_del = (char *)alloca(len);
                sprintf(to_del, "%s/%s", name, apple_double_fork_prefix);
                to_unlink = (char *)alloca(len + strlen(PARENT_FILE));
                sprintf(to_unlink, "%s%s", to_del, PARENT_FILE);
                unlink(to_unlink);
                to_del[len - 2] = 0; /* remove trailing slash */
                Urmdir(to_del);
            }
            break;
            default:
                warning_unexpected("op = %d", op);
                break;
        }
        *last_slash = save;
    }
}

OSErr Executor::ROMlib_newresfork(char *name, LONGINT *fdp, bool unix_p)
{
    LONGINT fd;
    OSErr retval;

    if(netatalk_conventions_p)
        double_dir_op(name, mkdir_op);
    if(unix_p)
    {
        ourentries.finfo.finfo.fdType = TICKX("TEXT");
        ourentries.finfo.finfo.fdCreator = TICKX("UNIX");
    }
    else
    {
        ourentries.finfo.finfo.fdType = 0;
        ourentries.finfo.finfo.fdCreator = 0;
    }
    initialize_ourdefault();
    if((fd = Uopen(name, (O_BINARY | O_RDWR | O_CREAT), 0666L)) < 0
       || (!native_resfork_p && (write(fd, (char *)&ourdefault, sizeof(ourdefault)) != sizeof(ourdefault)
                                 || write(fd, (char *)&ourentries, sizeof(ourentries)) != sizeof(ourentries))))

    {
        retval = ROMlib_maperrno();
        Uclose(fd);
    }
    else
    {
        retval = noErr;
        *fdp = fd;
    }
    fs_err_hook(retval);
    return retval;
}

/*
 * Modifications to hiddenbyname:
 *
 * haven't thought it through yet.
 */

#define OURBSIZE 512

static BOOLEAN getsetentry(GetOrSetType gors, LONGINT fd, Single_ID sid,
                           Single_descriptor *savesdp, ULONGINT *lengthp);
static void writebyteat(LONGINT fd, LONGINT loc);
static BOOLEAN getsetpiece(GetOrSetType gors, LONGINT fd,
                           Single_descriptor *sdp, char *bufp, LONGINT length);

static BOOLEAN getsetentry(GetOrSetType gors, LONGINT fd, Single_ID sid,
                           Single_descriptor *savesdp, ULONGINT *lengthp)
{
    off_t saveloc;
    struct defaulthead *dfp;
    INTEGER nread;
    Single_descriptor *sdp;
    char buf[OURBSIZE];
    BOOLEAN retval;
    INTEGER n;

    retval = false;
    saveloc = lseek(fd, 0L, SEEK_CUR);
    lseek(fd, 0L, L_SET);
    nread = read(fd, buf, sizeof(buf));
    dfp = (struct defaulthead *)buf;
    if(nread >= (int)sizeof(struct defaulthead)
       && (dfp->head.magic == CLC(SINGLEMAGIC)
           || dfp->head.magic == CLC(DOUBLEMAGIC)
           || dfp->head.magic.raw() == SINGLEMAGIC
           || dfp->head.magic.raw() == DOUBLEMAGIC))
    {
        n = MIN(Cx(dfp->head.nentries),
                ((nread - (int)sizeof(Single_header)) / (int)sizeof(Single_descriptor)));
        for(sdp = dfp->desc; --n >= 0 && Cx(sdp->id) != sid; ++sdp)
            ;
        if(n >= 0)
        {
            switch(gors)
            {
                case Get:
                    *savesdp = *sdp;
                    if(n == 0)
                    {
                        if(lengthp)
                            *lengthp = 0x7FFFFFFF; /* unlimited */
                    }
                    else if(lengthp)
                        *lengthp = CL(sdp[1].offset) - CL(sdp[0].offset);
                    break;
                case Set:
                    *sdp = *savesdp;
                    lseek(fd, 0L, L_SET);
                    write(fd, buf, nread);
                    break;
                default:
                    gui_assert(0);
                    break;
            }
            retval = true;
        }
    }
    lseek(fd, saveloc, L_SET);
    return retval;
}

#define IDWANTED(fp) ((fp->fcflags & fcfisres) ? Resource_Fork_ID : Data_Fork_ID)

LONGINT Executor::ROMlib_FORKOFFSET(fcbrec *fp) /* INTERNAL */
{
    Single_descriptor d;
    Single_ID idwanted;

    if(native_resfork_p)
        return 0;

    if(fp->fcfd != fp->hiddenfd)
        /*-->*/ return 0L;
    idwanted = IDWANTED(fp);
    if(getsetentry(Get, fp->fcfd, IDWANTED(fp), &d, NULL))
        return CL(d.offset);
    else
        return RESOURCEPREAMBLE;
}

static void writebyteat(LONGINT fd, LONGINT loc)
{
    off_t saveloc;

    saveloc = lseek(fd, 0L, SEEK_CUR);
    lseek(fd, loc, L_SET);
    write(fd, "", 1);
    lseek(fd, saveloc, L_SET);
}

/* TODO: better error checking */

OSErr Executor::ROMlib_seteof(fcbrec *fp) /* INTERNAL */
{
    ULONGINT leof, peof;
    off_t curloc;
    LONGINT fd;
    Single_descriptor d;
    Single_ID idwanted;
    INTEGER i;
    OSErr err;

    fd = fp->fcfd;
    leof = Cx(fp->fcleof);
    err = noErr;
    if(!native_resfork_p && fd == fp->hiddenfd)
    { /* mixed file */
        idwanted = IDWANTED(fp);
        if(getsetentry(Get, fd, idwanted, &d, &peof))
        {
            if(leof > peof)
            {
                gui_assert(0); /* TODO: We need to move stuff around.
			 	   There are many things we could do */
            }
            else if(leof < peof)
            {
                d.length = CL(leof);
                getsetentry(Set, fd, idwanted, &d, NULL);
            }
        }
        else
        {
            err = fsDSIntErr;
            warning_unexpected("getsetentry (Get, %d, %d) failed", fd,
                               idwanted);
        }
    }
    else
    { /* pure file */
        peof = Cx(fp->fcPLen);
        if(leof > peof)
            writebyteat(fd, leof - 1);
        else if(leof < peof)
        {
            curloc = lseek(fd, 0L, SEEK_CUR);
            ftruncate(fd, leof);
            if(curloc > leof)
                lseek(fd, leof, L_SET);
        }
        peof = leof;
    }
    if(err == noErr)
    {
        for(i = 0; i < NFCB; i++)
        {
            if(ROMlib_fcblocks[i].fdfnum == fp->fdfnum && ROMlib_fcblocks[i].fcvptr == fp->fcvptr && (ROMlib_fcblocks[i].fcflags & fcfisres) == (fp->fcflags & fcfisres))
            {
                ROMlib_fcblocks[i].fcleof = CL(leof);
                ROMlib_fcblocks[i].fcPLen = CL(peof);
            }
        }
    }
    fs_err_hook(err);
    return err;
}

static BOOLEAN getsetpiece(GetOrSetType gors, LONGINT fd,
                           Single_descriptor *sdp, char *bufp, LONGINT length)
{
    off_t saveloc;
    BOOLEAN retval;

    saveloc = lseek(fd, 0L, SEEK_CUR);
    lseek(fd, Cx(sdp->offset), L_SET);
    switch(gors)
    {
        case Get:
            retval = read(fd, bufp, length) == length;
            break;
        case Set:
            retval = write(fd, bufp, length) == length;
            break;
        default:
            retval = false;
            gui_assert(0);
            break;
    }
    lseek(fd, saveloc, L_SET);
    return retval;
}

/*
 * ROMlib_geteofostype
 *
 * Fill fp->fcPLen (physical length?)
 * 	    fp->fcleof (logical eof, same as above)
 *		fp->fcbFType (file type)
 */
OSErr Executor::ROMlib_geteofostype(fcbrec *fp) /* INTERNAL */
{
    LONGINT fd;
    Single_descriptor d;
    Single_ID idwanted;
    Single_finfo finfo;
    OSErr err;
    struct stat sbuf;

    fd = fp->hiddenfd;
    if(fstat(fp->fcfd, &sbuf) < 0)
        err = ROMlib_maperrno();
    else
    {
        err = noErr;
        if(!native_resfork_p && fd == fp->fcfd)
        { /* mixed file */
            idwanted = IDWANTED(fp);
            if(!getsetentry(Get, fd, idwanted, &d, NULL))
            {
                err = 0;
                fp->fcPLen = 0;
            }
            else
                fp->fcPLen = fp->fcleof = d.length;
        }
        else
            fp->fcleof = fp->fcPLen = CL((int)sbuf.st_size);
        if(err == noErr)
        {
#ifdef MACOSX
            if(native_resfork_p)
            {
                /* struct {
                    FInfo finfo;
                    FXInfo fxinfo;
                } buffer;
                if(getxattr(pathname, XATTR_FINDERINFO_NAME, &buffer, 32, 0, 0) < 0)
                {
                    uint32_t type;

                    if(ROMlib_creator_and_type_from_filename(fp->fcname[0], (char *)fp->fcname + 1, NULL, &type))
                        fp->fcbFType = CL(type);
                    else
                        fp->fcbFType = TICKX("TEXT");
                }
                else
                    fp->fcbFType = buffer.finfo.fdType;*/
                fp->fcbFType = 0; /* who reads this, anyway? */
            }
            else
#endif
                if(!getsetentry(Get, fd, Finder_Info_ID, &d, NULL) || (!getsetpiece(Get, fd, &d, (char *)&finfo, sizeof(finfo))))
            {
                uint32_t type;

                if(ROMlib_creator_and_type_from_filename(fp->fcname[0], (char *)fp->fcname + 1, NULL, &type))
                    fp->fcbFType = CL(type);
                else
                    fp->fcbFType = TICKX("TEXT");
            }
            else
                fp->fcbFType = finfo.finfo.fdType;
        }
    }
    fs_err_hook(err);
    return err;
}

/*
 * ROMlib_hiddenbyname
 *
 * Gets or sets file metadata.
 * I (autc04) have no idea why the function is named as it is.
 *
 * 		datep	file dates
 *		finfop
 *		fxinfop
 *		lenp	data fork length
 *		rlenp	resource fork length
 */

OSErr Executor::ROMlib_hiddenbyname(GetOrSetType gors, char *pathname,
                                    char *rpathname, Single_dates *datep,
                                    FInfo *finfop, FXInfo *fxinfop,
                                    GUEST<LONGINT> *lenp,
                                    GUEST<LONGINT> *rlenp) /* INTERNAL */
{
    LONGINT rfd;
    struct stat sbuf;
    Single_finfo sfinfo;
    Single_descriptor d;
    OSErr retval;
    BOOLEAN done;

    retval = noErr;
    if(Ustat(pathname, &sbuf) < 0)
        retval = ROMlib_maperrno();
    else
    {
#ifdef MACOSX
        if(native_resfork_p)
        {
            struct
            {
                FInfo finfo;
                FXInfo fxinfo;
            } buffer;
            if(gors == Get)
            {
                if(getxattr(pathname, XATTR_FINDERINFO_NAME, &buffer, 32, 0, 0) < 0)
                {
                    memset(&buffer, 0, sizeof(buffer));
                }
                if(finfop)
                    *finfop = buffer.finfo;
                if(fxinfop)
                    *fxinfop = buffer.fxinfo;

                if(datep)
                {
                    datep->crdat = CL(UNIXTIMETOMACTIME(sbuf.st_birthtime));
                    datep->moddat = CL(UNIXTIMETOMACTIME(sbuf.st_mtime));
                    datep->accessdat = CL(UNIXTIMETOMACTIME(sbuf.st_atime));
                    datep->backupdat = 0;
                }
                if(lenp)
                    *lenp = CL((int)sbuf.st_size);
                if(rlenp)
                {
                    if(Ustat(rpathname, &sbuf) < 0)
                        *rlenp = 0;
                    else
                        *rlenp = CL((int)sbuf.st_size);
                }
                fs_err_hook(retval);
                return retval;
            }
            else
            {
                if(getxattr(pathname, XATTR_FINDERINFO_NAME, &buffer, 32, 0, 0) < 0)
                {
                    memset(&buffer, 0, sizeof(buffer));
                }

                if(finfop)
                    buffer.finfo = *finfop;
                if(fxinfop)
                    buffer.fxinfo = *fxinfop;

                if(setxattr(pathname, XATTR_FINDERINFO_NAME, &buffer, 32, 0, 0) < 0)
                    retval = ROMlib_maperrno();
                fs_err_hook(retval);
                return retval;
            }
        }
#endif
        done = false;
        rfd = Uopen(rpathname, O_BINARY | (gors == Set ? O_RDWR : O_RDONLY), 0);
        /*
if (rfd == -1)
fprintf(stderr, "%s(%d): open '%s' fails\n", __FILE__, __LINE__, rpathname);
*/
        if(rfd < 0)
        {
            done = true;
            if(errno == ENOENT)
            {
                /* no resource fork (or AppleSingle) */
                /* right now we ignore that it could be AppleSingle */
                switch(gors)
                {
                    case Get:
                        memset(datep, 0, sizeof(*datep));
                        memset(finfop, 0, sizeof(*finfop));
                        memset(fxinfop, 0, sizeof(*fxinfop));
                        *lenp = CL((int)sbuf.st_size);
                        *rlenp = 0;
                        break;
                    case Set:
                        retval = ROMlib_newresfork(rpathname, &rfd, false);
                        done = false;
                        break;
                    default:
                        gui_assert(0);
                        break;
                }
            }
            else
                retval = ROMlib_maperrno();
        }
        if(!done && retval == noErr)
        {
            switch(gors)
            {
                case Get:
                    if(getsetentry(Get, rfd, File_Dates_Info_ID, &d, NULL))
                        getsetpiece(Get, rfd, &d, (char *)datep, sizeof(*datep));
                    else
                    {
                        datep->crdat = CL(UNIXTIMETOMACTIME(MIN(sbuf.st_ctime, sbuf.st_mtime)));
                        datep->moddat = CL(UNIXTIMETOMACTIME(sbuf.st_mtime));
                        datep->accessdat = CL(UNIXTIMETOMACTIME(sbuf.st_atime));
                        datep->backupdat = 0;
                    }

                    if(!getsetentry(Get, rfd, Finder_Info_ID, &d, NULL))
                        warning_unexpected("no finder info");
                    else
                    {
                        getsetpiece(Get, rfd, &d, (char *)&sfinfo, sizeof(sfinfo));

                        if(finfop)
                            *finfop = sfinfo.finfo;
                        if(fxinfop)
                            *fxinfop = sfinfo.fxinfo;

                        *lenp = CL((int)sbuf.st_size);
                    }
                    if(getsetentry(Get, rfd, Resource_Fork_ID, &d, NULL))
                        *rlenp = d.length;
                    else
                        *rlenp = 0;
                    break;
                case Set:
                    if(getsetentry(Get, rfd, File_Dates_Info_ID, &d, NULL))
                        getsetpiece(Set, rfd, &d, (char *)datep, sizeof(*datep));

                    if(!getsetentry(Get, rfd, Finder_Info_ID, &d, NULL))
                        warning_unexpected("no finfo");
                    else
                    {
                        if(!finfop || !fxinfop)
                        {
                            sfinfo.finfo.fdCreator = TICKX("UNIX");
                            sfinfo.finfo.fdType = TICKX("TEXT");
                            getsetpiece(Get, rfd, &d, (char *)&sfinfo,
                                        sizeof(sfinfo));
                        }

                        if(finfop)
                            sfinfo.finfo = *finfop;
                        if(fxinfop)
                            sfinfo.fxinfo = *fxinfop;
                        getsetpiece(Set, rfd, &d, (char *)&sfinfo, sizeof(sfinfo));
                    }
                    break;
                default:
                    gui_assert(0);
                    break;
            }
        }
        Uclose(rfd);
    }
    fs_err_hook(retval);
    return retval;
}

/*
 * Old algorithm:
 *
 * The new code is a little bit tricky; it would be just prepend a %, except
 * if someone has a name that begins with two hex digits ("feed me") you would
 * get something that could be a data fork, so we make sure that if the first
 * char is a % and the next two are hex and the hex would make something that
 * we would have wanted to map then we map the second character even though
 * we normally wouldn't map it.
 *
 * New algorithm:
 *
 * We just prepend the bloody "%" -- We get into trouble with other providers
 * of Apple Double if we do it differently.  What a crock.
 */

#define ROOTS_PERCENT_FILE "%%2F"

char *Executor::ROMlib_resname(char *pathname, /* INTERNAL */
                                      char *filename, char *endname)
{
    int pathnamesize, filenamesize, newsize;
    char *newname;

    pathnamesize = filename - pathname;
    filenamesize = endname - filename;
    if(pathnamesize)
    {
        newsize = pathnamesize + filenamesize
            + apple_double_fork_prefix_length
            + resfork_suffix_length;
        newname = (char *)malloc(newsize);
        memcpy(newname, pathname, pathnamesize);
        strcpy(&newname[pathnamesize], apple_double_fork_prefix);
        memcpy(newname + pathnamesize + apple_double_fork_prefix_length,
               filename, filenamesize);
        memcpy(newname + pathnamesize + apple_double_fork_prefix_length + filenamesize - 1,
               resfork_suffix, resfork_suffix_length);
        newname[newsize - 1] = 0;
    }
    else
    {
        /* "E:/" --> "E:/%%2F" */
        newsize = filenamesize + sizeof(ROOTS_PERCENT_FILE);
        newname = (char *)malloc(newsize);
        sprintf(newname, "%s%s", filename, ROOTS_PERCENT_FILE);
    }
    return newname;
}

/*
 * NOTE: we must use capital letters in tohex below.  Code depends on it.
 */

static unsigned char tohex(unsigned char c)
{
    unsigned char retval;

    retval = (unsigned char)(netatalk_conventions_p ? "0123456789abcdef"
                                                    : "0123456789ABCDEF")[c & 0xF];
    return retval;
}

static INTEGER Mac_to_UNIX7(unsigned char *name, INTEGER length,
                            unsigned char *out)
{
    unsigned char c;
    INTEGER retval;
    bool last_character_was_colon;

    retval = length;
    last_character_was_colon = true;
    while(--length >= 0)
    {
        c = *name++;
        if(((c != '\r' && c != '?') || !netatalk_conventions_p)
           && (c == '\\' || c == '%' || c == '*' || c == '?' || c == '"'
               || c == '<' || c == '>' || c == '|' || c == '/'
               || c < ' ' || c >= 0x7f))
        {
            *out++ = apple_double_quote_char;
            *out++ = tohex(c >> 4);
            *out++ = tohex(c);
            retval += 2;
        }
        else if(c == ':')
        {
            if(last_character_was_colon)
            {
                *out++ = '.';
                *out++ = '.';
                retval += 2;
            }
            *out++ = '/';
        }
        else
            *out++ = c;
        last_character_was_colon = c == ':';
    }
    return retval;
}

char *Executor::ROMlib_newunixfrommac(char *ip, INTEGER n)
{
    char *retval;

    if((retval = (char *)malloc(3 * n + 1))) /* worst case numbers */
        retval[Mac_to_UNIX7((unsigned char *)ip, n,
                            (unsigned char *)retval)]
            = 0;
    return retval;
}

void Executor::setup_resfork_format(ResForkFormat rf)
{
    afpd_conventions_p = netatalk_conventions_p = native_resfork_p = false;
    switch(rf)
    {
        case ResForkFormat::standard:
            apple_double_quote_char = '%';
            apple_double_fork_prefix = "%";
            break;
        case ResForkFormat::afpd:
            apple_double_quote_char = '%';
            apple_double_fork_prefix = "%";
            afpd_conventions_p = true;
            break;
        case ResForkFormat::netatalk:
            afpd_conventions_p = netatalk_conventions_p = true;
            apple_double_quote_char = ':';
            apple_double_fork_prefix = ".AppleDouble/";
            break;
        case ResForkFormat::native:
            apple_double_quote_char = '%';
            apple_double_fork_prefix = "";
            resfork_suffix = "/..namedfork/rsrc";
            native_resfork_p = true;
            break;
    }
    apple_double_fork_prefix_length = strlen(apple_double_fork_prefix);
    resfork_suffix_length = strlen(resfork_suffix);
}

void Executor::report_resfork_problem()
{
    if(afpd_conventions_p)
        fprintf(stderr,
                "Try omitting \"-afpd\" from the command line\n");
    else if(netatalk_conventions_p)
        fprintf(stderr,
                "Try omitting \"-netatalk\" from the command line\n");
}

/*
 * Blech!  I've finally caved in and am going to look for the magic
 * number in the obnoxious case of a file that starts with %XX where XX
 * are two upper case hex digits.
 */

BOOLEAN Executor::ROMlib_isresourcefork(const char *fullname)
{
    LONGINT fd;
    LONGINT magic;
    BOOLEAN retval;
    const char *filename;

    /* netatalk_conventions_p means we never see resource forks, because
	 they're stored in a subdirectory */

    if(netatalk_conventions_p)
        retval = false;
    else
    {
        filename = strrchr(fullname, '/');
        if(!filename)
            filename = fullname;
        else
            ++filename;
        if(filename[0] != '%')
            retval = false;
        else if(!isxdigit(filename[1]) || !isxdigit(filename[2]))
            retval = true;
        else
        {
            fd = -1;
            retval = (fd = Uopen(fullname, O_BINARY | O_RDONLY, 0)) >= 0
                && read(fd, (void *)&magic, sizeof(magic)) == sizeof(magic)
                && (magic == CLC(DOUBLEMAGIC).raw() || magic == DOUBLEMAGIC);
            if(fd >= 0)
                Uclose(fd);
        }
    }
    return retval;
}
