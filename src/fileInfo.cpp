/* Copyright 1986-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in FileMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "FileMgr.h"
#include "OSEvent.h"
#include "VRetraceMgr.h"
#include "OSUtil.h"
#include "MemoryMgr.h"

#include "rsys/file.h"
#include "rsys/glue.h"
#include "rsys/osutil.h"
#include "rsys/filedouble.h"
#include "rsys/suffix_maps.h"

#include <ctype.h>

using namespace Executor;

#if 0
#if defined(CYGWIN32)

PRIVATE char line_no[] = "123\r\n";

#define LOG()                                 \
    do                                        \
    {                                         \
        int line = __LINE__;                  \
                                              \
        line_no[0] = (line / 100) % 10 + '0'; \
        line_no[1] = (line / 10) % 10 + '0';  \
        line_no[2] = (line / 1) % 10 + '0';   \
        win32_log_data(line_no, 5);           \
    } while(0)

#else
#define LOG()
#endif
#endif

A3(PUBLIC, OSErr, GetFInfo, StringPtr, filen, INTEGER, vrn, /* IMIV-113 */
   FInfo *, fndrinfo)
{
    ParamBlockRec pbr;
    OSErr temp;

    pbr.fileParam.ioNamePtr = RM(filen);
    pbr.fileParam.ioVRefNum = CW(vrn);
    pbr.fileParam.ioFVersNum = 0;
    pbr.fileParam.ioFDirIndex = CWC(0);
    temp = PBGetFInfo(&pbr, 0);
    if(temp == noErr)
    {
        OSASSIGN(fndrinfo->fdType, pbr.fileParam.ioFlFndrInfo.fdType);
        OSASSIGN(fndrinfo->fdCreator, pbr.fileParam.ioFlFndrInfo.fdCreator);
        fndrinfo->fdFlags = pbr.fileParam.ioFlFndrInfo.fdFlags;
        fndrinfo->fdLocation = pbr.fileParam.ioFlFndrInfo.fdLocation;
        fndrinfo->fdFldr = pbr.fileParam.ioFlFndrInfo.fdFldr;
    }
    return temp;
}

PUBLIC OSErr HGetFInfo(INTEGER vref, LONGINT dirid, Str255 name,
                       FInfo *fndrinfo)
{
    HParamBlockRec pbr;
    OSErr retval;

    memset(&pbr, 0, sizeof pbr);
    pbr.fileParam.ioNamePtr = RM(name);
    pbr.fileParam.ioVRefNum = CW(vref);
    pbr.fileParam.ioDirID = CL(dirid);
    retval = PBHGetFInfo(&pbr, false);
    if(retval == noErr)
    {
        OSASSIGN(fndrinfo->fdType, pbr.fileParam.ioFlFndrInfo.fdType);
        OSASSIGN(fndrinfo->fdCreator, pbr.fileParam.ioFlFndrInfo.fdCreator);
        fndrinfo->fdFlags = pbr.fileParam.ioFlFndrInfo.fdFlags;
        fndrinfo->fdLocation = pbr.fileParam.ioFlFndrInfo.fdLocation;
        fndrinfo->fdFldr = pbr.fileParam.ioFlFndrInfo.fdFldr;
    }

    return retval;
}

A3(PUBLIC, OSErr, SetFInfo, StringPtr, filen, INTEGER, vrn, /* IMIV-114 */
   FInfo *, fndrinfo)
{
    ParamBlockRec pbr;
    OSErr temp;
    GUEST<ULONGINT> t;

    pbr.fileParam.ioNamePtr = RM(filen);
    pbr.fileParam.ioVRefNum = CW(vrn);
    pbr.fileParam.ioFVersNum = 0;
    pbr.fileParam.ioFDirIndex = CWC(0);
    temp = PBGetFInfo(&pbr, 0);
    if(temp != noErr)
        return (temp);
    OSASSIGN(pbr.fileParam.ioFlFndrInfo.fdType, fndrinfo->fdType);
    OSASSIGN(pbr.fileParam.ioFlFndrInfo.fdCreator, fndrinfo->fdCreator);
    pbr.fileParam.ioFlFndrInfo.fdFlags = fndrinfo->fdFlags;
    pbr.fileParam.ioFlFndrInfo.fdLocation = fndrinfo->fdLocation;
    pbr.fileParam.ioFlFndrInfo.fdFldr = fndrinfo->fdFldr = pbr.fileParam.ioFlFndrInfo.fdFldr;

    GetDateTime(&t);
    pbr.fileParam.ioFlMdDat = t;

    return (PBSetFInfo(&pbr, 0));
}

A2(PUBLIC, OSErr, SetFLock, StringPtr, filen, INTEGER, vrn) /* IMIV-114 */
{
    ParamBlockRec pbr;

    pbr.fileParam.ioNamePtr = RM(filen);
    pbr.fileParam.ioVRefNum = CW(vrn);
    pbr.fileParam.ioFVersNum = 0;
    return (PBSetFLock(&pbr, 0));
}

A2(PUBLIC, OSErr, RstFLock, StringPtr, filen, INTEGER, vrn) /* IMIV-114 */
{
    ParamBlockRec pbr;

    pbr.fileParam.ioNamePtr = RM(filen);
    pbr.fileParam.ioVRefNum = CW(vrn);
    pbr.fileParam.ioFVersNum = 0;
    return (PBRstFLock(&pbr, 0));
}

A3(PUBLIC, OSErr, Rename, StringPtr, filen, INTEGER, vrn, /* IMIV-114 */
   StringPtr, newf)
{
    ParamBlockRec pbr;

    pbr.ioParam.ioNamePtr = RM(filen);
    pbr.ioParam.ioVRefNum = CW(vrn);
    pbr.ioParam.ioVersNum = 0;
    pbr.ioParam.ioMisc = guest_cast<LONGINT>(RM(newf));
    return (PBRename(&pbr, 0));
}

A1(PUBLIC, unsigned char, ROMlib_fromhex, unsigned char, c)
{
    switch(c)
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return c - '0';
            break;
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            return 10 + c - 'a';
            break;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            return 10 + c - 'A';
            break;
    }
    return 0;
}

/*
 * NOTE: Before netatalk conventions, it was safe to convert slashes to colons
 * before calling UNIX7_to_Mac, because Mac names would never have colons in
 * them and unix names would never have slashes in them.  But since the
 * netatalk naming convention uses a colon as a special character, this is
 * no longer valid, because it uses a colon to tag special characters.
 *
 * Now we actually break complete paths into individual components,
 * then convert each component, then concatenate the components,
 * adding colons as the last step.
 *
 * The code below trys to sanity check things so that if there are places
 * where we're still trying to convert an entire pathname, we will get an
 * error message.
 */

PRIVATE bool
islowerxdigit(unsigned char c)
{
    bool retval;

    retval = ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    return retval;
}

PRIVATE bool
iscandidate_for_quoting(unsigned char c1, unsigned char c2)
{
    bool retval;

    if(!islowerxdigit(c1) || !islowerxdigit(c2))
        retval = false;
    else
    {
        unsigned char c;

        c = (ROMlib_fromhex(c1) << 4) | ROMlib_fromhex(c2);
        retval = !isalnum(c);
    }
    return retval;
}

PRIVATE bool
quoted_character(unsigned char *name, int length)
{
    bool retval;

    if(!netatalk_conventions_p)
        retval = (name[0] == apple_double_quote_char && length > 2 && isxdigit(name[1]) && isxdigit(name[2]));
    else
    {
        if(name[0] != apple_double_quote_char)
            retval = false;
        else
        {
            retval = (length > 2 && iscandidate_for_quoting(name[1], name[2]));
            if(!retval)
                fprintf(stderr, "*** suspicious colon in '%.*s' ***\n", length,
                        name);
        }
    }
    return retval;
}

A2(PUBLIC, INTEGER, ROMlib_UNIX7_to_Mac, char *, cname, INTEGER, length)
{
    INTEGER retval;
    unsigned char *name;
    unsigned char c, *out;

    name = (unsigned char *)cname;
    retval = 0;
    out = name;
    while(--length >= 0)
    {
        c = *name;
        if(quoted_character(name, length + 1))
        {
            *out++ = ROMlib_fromhex(name[1]) << 4 | ROMlib_fromhex(name[2]);
            retval += 2;
            name += 3;
            /* we are sucking up three digits, but the `-- length'
 	       only accounts for one of them */
            length -= 2;
        }
        else
        {
            *out++ = c;
            ++name;
        }
    }
    return retval;
}

PUBLIC Byte
Executor::open_attrib_bits(LONGINT file_id, VCB *vcbp, GUEST<INTEGER> *refnump)
{
    Byte retval;
    int i;

    retval = 0;
    *refnump = 0;
    for(i = 0; i < NFCB; i++)
    {
        if(CL(ROMlib_fcblocks[i].fdfnum) == file_id
           && MR(ROMlib_fcblocks[i].fcvptr) == vcbp)
        {
            if(*refnump == CW(0))
                *refnump = CW(i * 94 + 2);
            if(ROMlib_fcblocks[i].fcflags & fcfisres)
                retval |= ATTRIB_RESOPEN;
            else
                retval |= ATTRIB_DATAOPEN;
        }
    }
    if(retval & (ATTRIB_RESOPEN | ATTRIB_DATAOPEN))
        retval |= ATTRIB_ISOPEN;
    return retval;
}

A5(PUBLIC, OSErr, ROMlib_PBGetSetFInfoD, ParmBlkPtr, pb, /* INTERNAL */
   BOOLEAN, a, GetOrSetType, op, GUEST<LONGINT> *, dir, BOOLEAN, dodirs)
{
    OSErr err;
    char *pathname, *filename, *endname, *rpathname;
    char savechar;
    struct stat datasbuf, resourcesbuf, parentsbuf;
    GUEST<LONGINT> longzero = 0;
    VCBExtra *vcbp;
    GUEST<THz> savezone;
    struct timeval accessupdatetimes[2];
    IndexType indext;
    FInfo finfo;
    FXInfo fxinfo;
    Single_dates dateinfo;
    LONGINT errval;
    INTEGER tozeroout;

    savezone = TheZone;
    TheZone = SysZone;
    pathname = 0;
    rpathname = 0;

    if(!dir)
    {
        dir = &longzero;
        longzero = dodirs ? pb->fileParam.ioFlNum : GUEST<LONGINT>(0);
    }

    if(op == Get)
    {
        if(dodirs && (CW(pb->fileParam.ioFDirIndex) < 0))
            indext = IGNORENAME; /* IMIV-156 */
        else if(CW(pb->fileParam.ioFDirIndex) > 0)
            indext = FDirIndex;
        else
            indext = NoIndex;
    }
    else
        indext = NoIndex;
    if((err = ROMlib_nami(pb, CL(*dir), indext, &pathname, &filename, &endname,
                          !dodirs, &vcbp, &datasbuf))
       != noErr)
        /*-->*/ goto theend;
    rpathname = ROMlib_resname(pathname, filename, endname);
    if(op == Get)
    {
        LONGINT file_num;

        file_num = ST_INO(datasbuf);
        pb->fileParam.ioFlNum = CL(file_num);

        if(S_ISDIR(datasbuf.st_mode))
        {
            LONGINT dirid;

            dirid = ST_INO(datasbuf);
            ROMlib_dbm_store(vcbp, pathname, &dirid, true);
            if(!dodirs)
            {
                err = fnfErr;
                warning_trace_info("'%s' '%s' '%s' returning fnfErr",
                                   pathname, filename, endname);
                /*-->*/ goto theend;
            }
        }
        /*
 * NOTE: Changing filename[-1] may smash things so it's important that it
 *       gets restored.
 */
        tozeroout = filename == pathname + 1 ? 0 : -1;

        if(filename + tozeroout < pathname)
            savechar = 0;
        else
        {
            savechar = filename[tozeroout];
            filename[tozeroout] = 0;
        }

        /* don't eat the only '/' */

        if(savechar == '/' && strchr(pathname, '/') == 0)
        {
            if(filename + tozeroout >= pathname)
                filename[tozeroout] = '/';
            ++tozeroout;
            if(filename + tozeroout >= pathname)
            {
                savechar = filename[tozeroout];
                filename[tozeroout] = 0;
            }
        }

        errval = Ustat(pathname, &parentsbuf);
        if(errval == 0)
        {
            LONGINT dirid;

            dirid = ST_INO(parentsbuf);
            ROMlib_dbm_store(vcbp, pathname, &dirid, true);
        }
        if(filename + tozeroout >= pathname)
            filename[tozeroout] = savechar;

        if(errval < 0)
        {
            err = ioErr;
            /*-->*/ goto theend;
        }

        if(UPDATE_IONAMEPTR_P(pb->fileParam))
        {
            Str255 temp_name;
            StringPtr name_ptr;

            temp_name[0] = MIN(255, endname - filename - 1);
            BlockMoveData((Ptr)filename, (Ptr)temp_name + 1,
                          (Size)temp_name[0]);
            temp_name[0] -= ROMlib_UNIX7_to_Mac((char *)temp_name + 1,
                                                temp_name[0]);

#if defined(MSDOS) || defined(CYGWIN32)
            if(temp_name[0] == 3 && temp_name[2] == ':')
            {
                temp_name[2] = temp_name[3];
                --temp_name[0];
            }
#endif
            name_ptr = (StringPtr)MR(pb->fileParam.ioNamePtr);
            name_ptr[0] = MIN(31, temp_name[0]);
            BlockMoveData((Ptr)temp_name + 1, (Ptr)name_ptr + 1, name_ptr[0]);
        }
        pb->fileParam.ioFRefNum = 0;
        pb->fileParam.ioFlAttrib
            = CB(open_attrib_bits(ST_INO(datasbuf), (VCB *)vcbp,
                                  &pb->fileParam.ioFRefNum));
        if(!(datasbuf.st_mode & 0200))
            pb->fileParam.ioFlAttrib |= ATTRIB_ISLOCKED;
        if(S_ISDIR(datasbuf.st_mode))
        {
            pb->fileParam.ioFlAttrib |= ATTRIB_ISADIR;
            if(dodirs)
            {
                if(ST_INO(datasbuf) == vcbp->u.ufs.ino)
                {
                    *dir = ((CInfoPBPtr)pb)->dirInfo.ioDrDirID = CLC(2);
                    ((CInfoPBPtr)pb)->dirInfo.ioDrParID = CLC(1);
                }
                else
                {
                    *dir = ((CInfoPBPtr)pb)->dirInfo.ioDrDirID = CL((LONGINT)ST_INO(datasbuf));
                    if(ST_INO(parentsbuf) == vcbp->u.ufs.ino)
                        ((CInfoPBPtr)pb)->dirInfo.ioDrParID = CLC(2);
                    else
                        ((CInfoPBPtr)pb)->dirInfo.ioDrParID = CL((LONGINT)ST_INO(parentsbuf));
                }
            }
            else
            {
                /* actually what happens here doesn't really matter, since
		   we are doing a lookup on a directory, but we don't allo
		   lookups on directories */
                if(ST_INO(datasbuf) == vcbp->u.ufs.ino)
                {
                    *dir = CLC(2); /* root directory number */
                }
                else
                    *dir = CL((LONGINT)ST_INO(datasbuf));
                pb->fileParam.ioFlNum = CL((LONGINT)ST_INO(datasbuf));
            }
        }
        else
        {
            if(ST_INO(parentsbuf) == vcbp->u.ufs.ino)
                *dir = CLC(2); /* root directory number */
            else
                *dir = CL((LONGINT)ST_INO(parentsbuf));
            pb->fileParam.ioFlNum = CL((LONGINT)ST_INO(datasbuf));
            if(dodirs)
            {
                ((CInfoPBPtr)pb)->hFileInfo.ioFlParID = *dir;
                ((CInfoPBPtr)pb)->hFileInfo.ioFlClpSiz = CLC(512);
            }
        }
        if(Ustat(rpathname, &resourcesbuf) < 0)
        {
            pb->fileParam.ioFlRPyLen = 0;
            resourcesbuf.st_mtime = 0;
            memset(&pb->fileParam.ioFlFndrInfo, 0,
                   sizeof(pb->fileParam.ioFlFndrInfo));
            pb->fileParam.ioFlFndrInfo.fdCreator = TICKX("UNIX");
            pb->fileParam.ioFlFndrInfo.fdType = TICKX("TEXT");
            {
                uint32 creator, type;

                if(ROMlib_creator_and_type_from_filename(strlen(pathname),
                                                         pathname, &creator,
                                                         &type))
                {
                    pb->fileParam.ioFlFndrInfo.fdCreator = CL(creator);
                    pb->fileParam.ioFlFndrInfo.fdType = CL(type);
                }
            }
            pb->fileParam.ioFlLgLen = CL(datasbuf.st_size);
            pb->fileParam.ioFlRLgLen = 0;
            pb->fileParam.ioFlCrDat = CL(UNIXTIMETOMACTIME(datasbuf.st_ctime));
            if(dodirs)
                memset(&((CInfoPBPtr)pb)->hFileInfo.ioFlXFndrInfo, 0,
                       sizeof(((CInfoPBPtr)pb)->hFileInfo.ioFlXFndrInfo));
        }
        else
        {
            if((err = ROMlib_hiddenbyname(Get, pathname, rpathname, &dateinfo,
                                          &finfo, &fxinfo, &pb->fileParam.ioFlLgLen,
                                          &pb->fileParam.ioFlRLgLen))
               != noErr)
                goto theend;
            pb->fileParam.ioFlFndrInfo = finfo;
            pb->fileParam.ioFlCrDat = dateinfo.crdat;
            if(dodirs)
                ((CInfoPBPtr)pb)->hFileInfo.ioFlXFndrInfo = fxinfo;
        }
        pb->fileParam.ioFlPyLen = pb->fileParam.ioFlLgLen;
        pb->fileParam.ioFlRPyLen = pb->fileParam.ioFlRLgLen;
        pb->fileParam.ioFlVersNum = 0;
        pb->fileParam.ioFlStBlk = CW(-1); /* NOT SUPPORTED */
        pb->fileParam.ioFlRStBlk = CW(-1); /* NOT SUPPORTED */
        pb->fileParam.ioFlMdDat = CL(UNIXTIMETOMACTIME(
            MAX(resourcesbuf.st_mtime, datasbuf.st_mtime)));
        if(dodirs)
            ((CInfoPBPtr)pb)->hFileInfo.ioFlBkDat = 0;
    }
    else
    {
        finfo = pb->fileParam.ioFlFndrInfo;
        dateinfo.crdat = pb->fileParam.ioFlCrDat;
        if(dodirs)
            fxinfo = ((CInfoPBPtr)pb)->hFileInfo.ioFlXFndrInfo;
        {
            uint32 creator, type;

            if(ROMlib_creator_and_type_from_filename(strlen(filename), filename, &creator, &type)
               && finfo.fdType == CL(type) && finfo.fdCreator == CL(creator))
                err = noErr;
            else if((err = ROMlib_hiddenbyname(Set, pathname, rpathname,
                                               &dateinfo, &finfo,
                                               dodirs ? &fxinfo : (FXInfo *)0,
                                               (GUEST<LONGINT> *)0, (GUEST<LONGINT> *)0))
                    != noErr)
                goto theend;
        }
        Ustat(pathname, &datasbuf);
#if !defined(MACBLITZ)
        accessupdatetimes[0].tv_sec = datasbuf.st_atime;
        accessupdatetimes[0].tv_usec = 0;
        accessupdatetimes[1].tv_sec = MACTIMETOGUNIXTIME(CL(pb->fileParam.ioFlMdDat));
        accessupdatetimes[1].tv_usec = 0;
        Uutimes(pathname, accessupdatetimes);
#endif /* MACBLITZ */
    }

theend:
    if(pathname)
        free(pathname);
    if(rpathname)
        free(rpathname);
    TheZone = savezone;
    return err;
}

A2(PUBLIC, OSErr, ufsPBGetFInfo, ParmBlkPtr, pb, /* INTERNAL */
   BOOLEAN, a)
{
    return ROMlib_PBGetSetFInfoD(pb, a, Get, (GUEST<LONGINT> *)0, false);
}

A2(PUBLIC, OSErr, ufsPBHGetFInfo, HParmBlkPtr, pb, /* INTERNAL */
   BOOLEAN, a)
{
    GUEST<LONGINT> d;
    OSErr err;

    d = pb->fileParam.ioDirID;
    auto compsave = pb->ioParam.ioCompletion;
    pb->ioParam.ioCompletion = 0;
    err = ROMlib_PBGetSetFInfoD((ParmBlkPtr)pb, a, Get, &d, false);
    pb->fileParam.ioDirID = d;
    pb->ioParam.ioCompletion = compsave;
    return err;
}

A2(PUBLIC, OSErr, ufsPBSetFInfo, ParmBlkPtr, pb, /* INTERNAL */
   BOOLEAN, a)
{
    return ROMlib_PBGetSetFInfoD(pb, a, Set, (GUEST<LONGINT> *)0, false);
}

A2(PUBLIC, OSErr, ufsPBHSetFInfo, HParmBlkPtr, pb, /* INTERNAL */
   BOOLEAN, a)
{
    GUEST<LONGINT> d;

    d = pb->fileParam.ioDirID;
    return ROMlib_PBGetSetFInfoD((ParmBlkPtr)pb, a, Set, &d, false);
}

namespace Executor
{
PRIVATE int flipwritebits(char *, LockOrUnlockType);
PRIVATE OSErr PBLockUnlock(ParmBlkPtr, BOOLEAN, LONGINT, LockOrUnlockType);
}

A2(PRIVATE, int, flipwritebits, char *, file, LockOrUnlockType, op)
{
    struct stat sbuf;
    int retval;
    int oldmask;

    if(!(retval = Ustat(file, &sbuf)))
    {
        if(op == Lock)
            retval = Uchmod(file, sbuf.st_mode & ~(0222));
        else
        {
            oldmask = umask(0);
            umask(oldmask);
            retval = Uchmod(file, sbuf.st_mode | (0222 & ~oldmask));
        }
    }
    return retval;
}

A4(PRIVATE, OSErr, PBLockUnlock, ParmBlkPtr, pb, BOOLEAN, a, LONGINT, dir,
   LockOrUnlockType, op)
{
    OSErr err;
    char *pathname, *filename, *endname, *rpathname;
    int fliperr;
    struct stat sbuf;

    if((err = ROMlib_nami(pb, dir, NoIndex, &pathname, &filename, &endname,
                          true, (VCBExtra **)0, &sbuf))
       == noErr)
    {
        rpathname = ROMlib_resname(pathname, filename, endname);
        if((fliperr = flipwritebits(pathname, op)))
        {
            switch(fliperr)
            {
                case ENOTDIR:
                    warning_trace_info("pathname = '%s', op = %d", pathname, op);
                    err = dirNFErr;
                    break;
                case EINVAL:
#if defined(ENAMETOOLONG)
                case ENAMETOOLONG:
#endif /* ENAMETOOLONG */
#if defined(ELOOP)
                case ELOOP:
#endif /* ELOOP */
#if defined(EFAULT)
                case EFAULT:
#endif /* defined(EFAULT) */
                    err = paramErr;
                    break;
                case ENOENT:
                    err = fnfErr;
                    break;
                case EACCES:
#if(EACCES != EPERM)
                case EPERM:
#endif /* (EACCES != EPERM) */
                    err = vLckdErr;
                    break;
#if defined(EROFS)
                case EROFS:
                    err = wPrErr;
                    break;
#endif /* defined(EROFS) */
                case EIO:
                default:
                    err = ioErr;
            }
        }
        if(err == noErr && (fliperr = flipwritebits(rpathname, op)))
        {
            switch(fliperr)
            {
                case ENOTDIR:
                    warning_trace_info("rpathname = '%s', op = %d", rpathname,
                                       op);
                    err = dirNFErr;
                    break;
                case EINVAL:
#if defined(ENAMETOOLONG)
                case ENAMETOOLONG:
#endif /* ENAMETOOLONG */
#if defined(ELOOP)
                case ELOOP:
#endif /* ELOOP */
#if defined(EFAULT)
                case EFAULT:
#endif /* defined(EFAULT) */
                    err = paramErr;
                    break;
                case ENOENT:
                    err = noErr; /* no resource fork, that's ok */
                    break;
                case EACCES:
#if(EACCES != EPERM)
                case EPERM:
#endif /* (EACCES != EPERM) */
                    err = vLckdErr;
                    break;
#if defined(EROFS)
                case EROFS:
                    err = wPrErr;
                    break;
#endif /* defined(EROFS) */
                case EIO:
                default:
                    err = ioErr;
            }
        }
        if(rpathname)
            free(rpathname);
    }
    if(pathname)
        free(pathname);
    return err;
}

A2(PUBLIC, OSErr, ufsPBSetFLock, ParmBlkPtr, pb, /* INTERNAL */
   BOOLEAN, a)
{
    return PBLockUnlock(pb, a, (LONGINT)0, Lock);
}

A2(PUBLIC, OSErr, ufsPBHSetFLock, HParmBlkPtr, /* INTERNAL */
   pb, BOOLEAN, a)
{
    return PBLockUnlock((ParmBlkPtr)pb, a, CL(pb->fileParam.ioDirID), Lock);
}

A2(PUBLIC, OSErr, ufsPBRstFLock, ParmBlkPtr, pb, /* INTERNAL */
   BOOLEAN, a)
{
    return PBLockUnlock(pb, a, (LONGINT)0, Unlock);
}

A2(PUBLIC, OSErr, ufsPBHRstFLock, HParmBlkPtr, pb, /* INTERNAL */
   BOOLEAN, a)
{
    return PBLockUnlock((ParmBlkPtr)pb, a, CL(pb->fileParam.ioDirID), Unlock);
}

A2(PUBLIC, OSErr, ufsPBSetFVers, ParmBlkPtr, pb, /* INTERNAL */
   BOOLEAN, a)
{
    /* NOP */
    return noErr;
}

A2(PUBLIC, OSErr, ufsPBRename, ParmBlkPtr, pb, /* INTERNAL */
   BOOLEAN, a)
{
    return ROMlib_PBMoveOrRename(pb, a, (LONGINT)0, (LONGINT)0,
                                 MR(guest_cast<char *>(pb->ioParam.ioMisc)), FRename);
}

A2(PUBLIC, OSErr, ufsPBHRename, HParmBlkPtr, pb, /* INTERNAL */
   BOOLEAN, a)
{
    return ROMlib_PBMoveOrRename((ParmBlkPtr)pb, a, CL(pb->fileParam.ioDirID),
                                 CL(pb->fileParam.ioDirID), MR(guest_cast<char *>(pb->ioParam.ioMisc)),
                                 HRename);
}
