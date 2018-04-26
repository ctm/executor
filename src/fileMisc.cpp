/* Copyright 1986-1997 by Abacus Research and
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
#include "StdFilePkg.h"

#include "rsys/hfs.h"
#include "rsys/file.h"
#include "rsys/notmac.h"
#include "rsys/stdfile.h"
#include "rsys/ini.h"
#include "rsys/string.h"
#include "rsys/segment.h"
#include "rsys/suffix_maps.h"

#if defined(MSDOS) || defined(CYGWIN32)
#include "rsys/checkpoint.h"
#endif

#if !defined(WIN32)
#include <pwd.h>
#else
#include "winfs.h"
//#include "dosdisk.h"
#endif

#include <ctype.h>
#include <algorithm>

using namespace Executor;

/* NOTE:  calling most of the routines here is a sign that the user may
	  be depending on the internal layout of things a bit too much */

void Executor::FInitQueue() /* IMIV-128 */
{
}

QHdrPtr Executor::GetFSQHdr() /* IMIV-175 */
{
    return (&LM(FSQHdr)); /* in UNIX domain, everything is synchronous */
}

QHdrPtr Executor::GetVCBQHdr() /* IMIV-178 */
{
    return (&LM(VCBQHdr));
}

QHdrPtr Executor::GetDrvQHdr() /* IMIV-182 */
{
    return (&LM(DrvQHdr));
}

OSErr Executor::ufsPBGetFCBInfo(FCBPBPtr pb, BOOLEAN a) /* INTERNAL */
{
    int rn;
    OSErr err;
    fcbrec *fp;
    int i, count;

#if !defined(LETGCCWAIL)
    rn = -1;
    fp = 0;
#endif /* LETGCCWAIL */

    err = noErr;
    if(pb->ioFCBIndx == CWC(0))
    {
        rn = Cx(pb->ioRefNum);
        fp = PRNTOFPERR(rn, &err);
    }
    else if(Cx(pb->ioFCBIndx) > 0)
    {
        for(count = 0, i = 0; i < NFCB && count < Cx(pb->ioFCBIndx); i++)
            if(ROMlib_fcblocks[i].fdfnum && (!pb->ioVRefNum || MR(ROMlib_fcblocks[i].fcvptr)->vcbVRefNum == pb->ioVRefNum))
                count++;
        if(count == Cx(pb->ioFCBIndx))
        {
            fp = ROMlib_fcblocks + i - 1;
            rn = (Ptr)fp - MR(LM(FCBSPtr));
        }
        else
            err = paramErr;
    }
    else
        err = paramErr;
    if(err == noErr)
    {
        if(pb->ioNamePtr)
            str255assign(MR(pb->ioNamePtr), fp->fcname);
        pb->ioVRefNum = MR(fp->fcvptr)->vcbVRefNum;
        pb->ioRefNum = CW(rn);
        pb->ioFCBFlNm = fp->fdfnum;
        pb->ioFCBFlags = CW((fp->fcflags << 8) | (unsigned char)fp->fcbTypByt);
        pb->ioFCBStBlk = CW(0);
        pb->ioFCBEOF = fp->fcleof;
        pb->ioFCBPLen = fp->fcleof;
        pb->ioFCBCrPs = CL(lseek(fp->fcfd, 0, SEEK_CUR) - FORKOFFSET(fp));
        pb->ioFCBVRefNum = MR(fp->fcvptr)->vcbVRefNum; /* what's this? */
        pb->ioFCBClpSiz = MR(fp->fcvptr)->vcbClpSiz;
        pb->ioFCBParID = fp->fcparid;
    }
    return err;
}

static bool
charcmp(char c1, char c2)
{
    bool retval;

    if(c1 == c2)
        retval = true;
    else if(c1 == '/')
        retval = c2 == '\\';
    else if(c1 == '\\')
        retval = c2 == '/';
    else
        retval = tolower(c1) == tolower(c2);
    return retval;
}

static int
slashstrcmp(const char *p1, const char *p2)
{
    int retval;

    retval = 0;

    while(*p1 || *p2)
    {
        if(!charcmp(*p1, *p2))
        {
            retval = -1;
            break;
        }
        ++p1;
        ++p2;
    }
    return retval;
}

static INTEGER ROMlib_driveno = 3;
static INTEGER ROMlib_ejdriveno = 2;

/*
 * NOTE: The way we handle drive information is pretty messed up right now.
 * In general the correct information is in the VCBExtra; we only recently
 * began putting it in the DriveExtra and right now we only use the info
 * in the DriveExtra to allow us to format floppies -- no other formatting
 * is currently permitted.  The problem is there's no easy way to map drive
 * characteristics from the non-Mac host into Mac type information unless
 * we can pull the information out of the Mac filesystem.
 */

DrvQExtra *
Executor::ROMlib_addtodq(ULONGINT drvsize, const char *devicename, INTEGER partition,
                         INTEGER drefnum, drive_flags_t flags, hfs_access_t *hfsp)
{
    INTEGER dno;
    DrvQExtra *dqp;
    DrvQEl *dp;
    int strl;
    GUEST<THz> saveZone;
    static bool seen_floppy = false;

    saveZone = LM(TheZone);
    LM(TheZone) = LM(SysZone);
#if !defined(LETGCCWAIL)
    dqp = (DrvQExtra *)0;
#endif
    dno = 0;
    for(dp = (DrvQEl *)MR(LM(DrvQHdr).qHead); dp; dp = (DrvQEl *)MR(dp->qLink))
    {
        dqp = (DrvQExtra *)((char *)dp - sizeof(LONGINT));
        if(dqp->partition == CW(partition) && slashstrcmp((char *)dqp->devicename, devicename) == 0)
        {
            dno = Cx(dqp->dq.dQDrive);
            /*-->*/ break;
        }
    }
    if(!dno)
    {
        if((flags & DRIVE_FLAGS_FLOPPY) && !seen_floppy)
        {
            dno = 1;
            seen_floppy = true;
        }
        else
        {
            if((flags & DRIVE_FLAGS_FIXED) || ROMlib_ejdriveno == 3)
                dno = ROMlib_driveno++;
            else
                dno = ROMlib_ejdriveno++;
        }
        dqp = (DrvQExtra *)NewPtr(sizeof(DrvQExtra));
        dqp->flags = CL(1 << 7); /* is not single sided */
        if(flags & DRIVE_FLAGS_LOCKED)
            dqp->flags = CL(CL(dqp->flags) | 1L << 31);
        if(flags & DRIVE_FLAGS_FIXED)
            dqp->flags = CL(CL(dqp->flags) | 8L << 16);
        else
            dqp->flags = CL(CL(dqp->flags) | 2); /* IMIV-181 says
							   it can be 1 or 2 */

        /*	dqp->dq.qLink will be set up when we Enqueue this baby */
        dqp->dq.dQDrvSz = CW(drvsize);
        dqp->dq.dQDrvSz2 = CW(drvsize >> 16);
        dqp->dq.qType = CWC(1);
        dqp->dq.dQDrive = CW(dno);
        dqp->dq.dQRefNum = CW(drefnum);
        dqp->dq.dQFSID = 0;
        if(!devicename)
            dqp->devicename = 0;
        else
        {
            strl = strlen(devicename);
            dqp->devicename = NewPtr(strl + 1);
            strcpy((char *)dqp->devicename, devicename);
        }
        dqp->partition = CW(partition);
        if(hfsp)
            dqp->hfs = *hfsp;
        else
        {
            memset(&dqp->hfs, 0, sizeof(dqp->hfs));
            dqp->hfs.fd = -1;
        }
        Enqueue((QElemPtr)&dqp->dq, &LM(DrvQHdr));
    }
    LM(TheZone) = saveZone;
    return dqp;
}

static bool
root_directory_p(const char *path, dev_t our_dev)
{
    const char *slash;
    bool retval;

    /* we used to just compare our_inode to 2, but that doesn't work with
     NFS mounted filesystems that aren't mounted at the root directory or
     with DOS filesystems mounted under Linux */

    slash = strrchr(path, '/');
    if(!slash || ((slash == path + SLASH_CHAR_OFFSET) && !slash[1]))
        retval = true;
    else
    {
        struct stat sbuf;

        if(slash == path + SLASH_CHAR_OFFSET)
            ++slash;
        std::string tmp(path, slash);
        if(Ustat(tmp.c_str(), &sbuf) != 0)
            retval = true;
        else
            retval = sbuf.st_dev != our_dev;
    }
    return retval;
}

/*
 * ROMlib_volumename is a magic global variable that tells MountVol
 * the name of the volume that you're mounting (since there is no
 * way to map the "drive number" into such a string)
 */

std::string Executor::ROMlib_volumename;

static void ROMlib_automount_helper(const char *cpath, char *aliasp)
{
    char *path = (char *)alloca(strlen(cpath) + 1);
    strcpy(path, cpath);

    struct stat sbuf;
    ParamBlockRec pb;
    int sret;
    int i;
    LONGINT dirid;
    INTEGER retval;
    HVCB *vcbp;
    DrvQExtra *dqp;
    char *oldsavep;
    char *savep;
    char save;

#if defined(WIN32)
    {
        char *temppath, *op, c;
        int len;

        len = strlen(path) + 1;

        /* If we don't have x:/ then we need to prepend the start drive */
        if(path[0] && (path[1] != ':' || path[2] != '/'))
            len += 2;
        temppath = (char*)alloca(len);
        if(path[0] && (path[1] != ':' || path[2] != '/'))
        {
            temppath[0] = ROMlib_start_drive;
            temppath[1] = ':';
            temppath += 2;
        }

        /* convert backslashes to slashes */
        op = temppath;
        while((c = *path++))
            *op++ = c == '\\' ? '/' : c;
        *op = 0;
        path = temppath;
    }
#endif

    retval = 0;
#if !defined(LETGCCWAIL)
    save = 0;
#endif
    if(path[0] == '/'
#if defined(WIN32)
       || (path[1] == ':' && path[2] == '/')
#endif
           )
    {
#if 1 /*!defined(WIN32)*/
        ROMlib_undotdot(path);
#else
        char *newpath = (char*)alloca(strlen(path) + 3); /* one for null, two for drive */
        _fixpath(path, newpath);
        path = newpath;
#endif
        /* Make two passes:  On the first pass (i == 0) we identify
	   filesystems and mount them.  On the second pass (i == 1) we
	   store away intermediate directory numbers */

        for(i = 0; i < 2; ++i)
        {
            bool done;
            sret = Ustat(path, &sbuf);
            savep = 0;
            oldsavep = 0;
            done = false;
            do
            {
                if(sret == 0 && S_ISDIR(sbuf.st_mode))
                {
                    if(root_directory_p(path, sbuf.st_dev) || aliasp)
                    {
                        if(i == 0)
                        {
                            ROMlib_volumename = path;
                            dqp = ROMlib_addtodq(2048L * 50,
                                                 ROMlib_volumename.c_str(), 0,
                                                 OURUFSDREF,
                                                 DRIVE_FLAGS_FIXED, 0);
                            pb.ioParam.ioVRefNum = dqp->dq.dQDrive;
                            ufsPBMountVol(&pb);
                            if(aliasp)
                            {
                                HVCB *vcbp;

                                vcbp = ROMlib_vcbbyvrn(CW(pb.ioParam.ioVRefNum));
                                str255_from_c_string(vcbp->vcbVN, aliasp);
                                /* hack in name */
                                /*-->*/ return;
                            }
                        }
                    }
                    else
                    {
                        if(i == 1)
                        {
                            vcbp = ROMlib_vcbbybiggestunixname(path);
                            gui_assert(vcbp);
                            dirid = ST_INO(sbuf);
                            ROMlib_dbm_store((VCBExtra *)vcbp, path, &dirid,
                                             false);
                        }
                    }
                }
                if(savep == path + SLASH_CHAR_OFFSET + 1)
                    done = true;
                else
                {
                    savep = strrchr(path, '/');
                    if(savep == path + SLASH_CHAR_OFFSET)
                        ++savep;
                    if(oldsavep)
                        *oldsavep = save;
                    save = *savep;
                    *savep = 0;
                    oldsavep = savep;
                    sret = Ustat(path, &sbuf);
                }
            } while(!done);
            if(oldsavep)
                *oldsavep = save;
        }
    }
}

void Executor::ROMlib_automount(const char *path)
{
    ROMlib_automount_helper(path, NULL);
}

void ROMlib_volume_alias(const char *path, const char *alias_name)
{
    ROMlib_automount_helper((char *)path, (char *)alias_name);
}

std::string
Executor::expandPath(std::string name)
{
    if(name.empty())
        return "";

    switch(name[0])
    {
        case '+':
            name = ROMlib_startdir + name.substr(1);
            break;
        case '~':
        {
            auto home = getenv("HOME");
            if(home)
            {
                name = home + name.substr(1);
                break;
            }
        }
        break;
    }

#if defined(WIN32)
    std::replace(name.begin(), name.end(), '/', '\\');
#endif

    return name;
}

#if defined(MSDOS) || defined(CYGWIN32)
bool cd_mounted_by_trickery_p = false;

#define MACCDROM \
    (ROMlib_mac_cdromp ? (char *)ROMlib_mac_cdromp->chars : "DOS/EXTRA/LIBRARY/MACCDROM.HFV")

#if defined(MSDOS) || defined(CYGWIN32)
static char *cd_big_hfv = 0;

static void
check_for_executor_cd(const char *drive)
{
    if(!cd_big_hfv)
    {
        struct stat sbuf;

        cd_big_hfv = malloc(strlen(drive) + strlen(MACCDROM) + 1);
        sprintf(cd_big_hfv, "%s%s", drive, MACCDROM);
        if(stat(cd_big_hfv, &sbuf) != 0)
        {
            free(cd_big_hfv);
            cd_big_hfv = 0;
        }
    }
}

static bool
e2_is_mounted(void)
{
    bool retval;
    const char e2_name[] = "Executor2";

    retval = !!vlookupbyname(e2_name, e2_name + strlen(e2_name));
    return retval;
}
#endif

#endif

StringPtr Executor::ROMlib_exefname;
char *Executor::ROMlib_exeuname;

std::string Executor::ROMlib_ConfigurationFolder;
std::string Executor::ROMlib_SystemFolder;
std::string Executor::ROMlib_DefaultFolder;
std::string Executor::ROMlib_PublicDirectoryMap;
std::string Executor::ROMlib_PrivateDirectoryMap;
static std::string ROMlib_MacVolumes;
std::string Executor::ROMlib_ScreenDumpFile;
static std::string ROMlib_OffsetFile;

LONGINT Executor::ROMlib_magic_offset = -1;

static void
skip_comments(FILE *fp)
{
    int c;

    while((c = getc(fp)) == '#')
    {
        while(c != '\n')
            c = getc(fp);
    }
    ungetc(c, fp);
}

static void
parse_offset_file(void)
{
    FILE *fp;

    fp = Ufopen(ROMlib_OffsetFile.c_str(), "r");
    if(!fp)
    {
#if 0
      warning_unexpected ("Couldn't open \"%s\"", ROMlib_OffsetFile);
#endif
    }
    else
    {
        int n_found;

        skip_comments(fp);
        n_found = fscanf(fp, "0x%08x", &ROMlib_magic_offset);
        if(n_found != 1)
            warning_unexpected("n_found = %d", n_found);
        fclose(fp);
    }
}

#if defined(MSDOS)

static uint32_t
drive_char_to_bit(char c)
{
    uint32_t retval;

    if(c >= 'a' && c <= 'z')
        retval = 1 << (c - 'a');
    else if(c >= 'A' && c <= 'Z')
        retval = 1 << (c - 'A');
    else
        retval = 0;
    return retval;
}
#endif

static bool
is_unix_path(const char *pathname)
{
    bool retval;

#if defined(MSDOS) || defined(CYGWIN32)
    if(pathname[0] && pathname[1] == ':' && (pathname[2] == '/' || pathname[2] == '\\'))
        pathname += 3;
#endif
    retval = strchr(pathname, ':') == 0;
    return retval;
}

void Executor::ROMlib_fileinit() /* INTERNAL */
{
    INTEGER i;
    CInfoPBRec cpb;
    WDPBRec wpb;
    INTEGER wdlen;
    HVCB *vcbp;
    GUEST<LONGINT> m;
    GUEST<THz> savezone;
    struct stat sbuf;
    char *sysname;
    int sysnamelen;
    char *p, *ep;

    LM(CurDirStore) = CLC(2);

    savezone = LM(TheZone);
    LM(TheZone) = LM(SysZone);
    LM(FCBSPtr) = RM(NewPtr((Size)sizeof(fcbhidden)));
    ((fcbhidden *)MR(LM(FCBSPtr)))->nbytes = CW(sizeof(fcbhidden));

    for(i = 0; i < NFCB; i++)
    {
        ROMlib_fcblocks[i].fdfnum = 0;
        ROMlib_fcblocks[i].fcleof = CL(i + 1);
        ROMlib_fcblocks[i].fcbTypByt = 0;
        ROMlib_fcblocks[i].fcbSBlk = 0;
        ROMlib_fcblocks[i].fcPLen = 0;
        ROMlib_fcblocks[i].fcbCrPs = 0;
        ROMlib_fcblocks[i].fcbBfAdr = 0;
        ROMlib_fcblocks[i].fcbFlPos = 0;
        ROMlib_fcblocks[i].fcbClmpSize = CLC(1);
        ROMlib_fcblocks[i].fcbFType = 0;
        ROMlib_fcblocks[i].zero[0] = 0;
        ROMlib_fcblocks[i].zero[1] = 0;
        ROMlib_fcblocks[i].zero[2] = 0;
        ROMlib_fcblocks[i].fcname[0] = 0;
    }
    ROMlib_fcblocks[NFCB - 1].fcleof = CLC(-1);

#define NWDENTRIES 40
    wdlen = NWDENTRIES * sizeof(wdentry) + sizeof(INTEGER);
    LM(WDCBsPtr) = RM(NewPtr((Size)wdlen));
    LM(TheZone) = savezone;
    memset(MR(LM(WDCBsPtr)), 0, wdlen);
    *(GUEST<INTEGER> *)MR(LM(WDCBsPtr)) = CW(wdlen);

    auto initpath = [](const char *varname, const char *defval) {
        if(auto v = getenv(varname))
            return expandPath(v);
        else
            return expandPath(defval);
    };

    ROMlib_ConfigurationFolder = initpath("Configuration", "+/Configuration");
    ROMlib_SystemFolder = initpath("SystemFolder", "+/ExecutorVolume/System Folder");
    ROMlib_PublicDirectoryMap = initpath("PublicDirectoryMap", "+/DirectoryMap");
    ROMlib_PrivateDirectoryMap = initpath("PrivateDirectoryMap", "~/.ExecutorDirectoryMap");
    ROMlib_DefaultFolder = initpath("DefaultFolder", "+/ExecutorVolume");
    ROMlib_MacVolumes = initpath("MacVolumes", "+/exsystem.hfv;+"); // this is wrong: only first + is replaced
    ROMlib_ScreenDumpFile = initpath("ScreenDumpFile", "/tmp/excscrn*.tif");
    ROMlib_OffsetFile = initpath("OffsetFile", "+/offset_file");
    ROMlib_PrintersIni = initpath("PrintersIni", "+/printers.ini");
    ROMlib_PrintDef = initpath("PrintDef", "+/printdef.ini");

    parse_offset_file();

/*
 * NOTE: The following is a hack that will remain in place until we have
 *     a replacement for using the ndbm routines which apparently can't
 *     share files between machines of different endianness.
 */

#if defined(LITTLEENDIAN)
    ROMlib_PublicDirectoryMap += "-le";
    ROMlib_PrivateDirectoryMap += "-le";
#endif /* defined(LITTLEENDIAN) */

    ROMlib_hfsinit();
    ROMlib_automount(ROMlib_SystemFolder.c_str());

#if 0
    m = 0;
    if (Ustat(ROMlib_DefaultFolder, &sbuf) == 0)
	if ((sbuf.st_mode & S_IFMT) == S_IFREG)
	    ROMlib_openharddisk(ROMlib_DefaultFolder, &m);
#else
    m = 0;
    p = (char *)alloca(ROMlib_MacVolumes.size() + 1);
    strcpy(p, ROMlib_MacVolumes.c_str());
    while(p && *p)
    {
        ep = strchr(p, ';');
        if(ep)
            *ep = 0;
        std::string newp = expandPath(p);
        if(Ustat(newp.c_str(), &sbuf) == 0)
        {
            if(!S_ISDIR(sbuf.st_mode))
                ROMlib_openharddisk(newp.c_str(), &m);
            else
            {
                DIR *dirp;

                dirp = Uopendir(newp.c_str());
                if(dirp)
                {
#if defined(USE_STRUCT_DIRECT)
                    struct direct *direntp;
#else
                    struct dirent *direntp;
#endif

                    while((direntp = readdir(dirp)))
                    {
                        int namelen;

                        namelen = strlen(direntp->d_name);
                        if(namelen >= 4 && (strcasecmp(direntp->d_name + namelen - 4, ".hfv")
                                                == 0
                                            || strcasecmp(direntp->d_name + namelen - 4, ".ima")
                                                == 0))
                        {
                            ROMlib_openharddisk((newp + "/" + direntp->d_name).c_str(), &m);
                        }
                    }
                    closedir(dirp);
                }
            }
        }
        if(ep)
        {
            *ep = ';';
            p = ep + 1;
        }
        else
            p = 0;
    }
#endif

    ROMlib_automount(ROMlib_startdir);
    ROMlib_automount(ROMlib_DefaultFolder.c_str());
    if(is_unix_path(ROMlib_DefaultFolder.c_str())
       && Ustat(ROMlib_DefaultFolder.c_str(), &sbuf) == 0)
    {
        LM(CurDirStore) = CL((LONGINT)ST_INO(sbuf));
        vcbp = ROMlib_vcbbybiggestunixname(ROMlib_DefaultFolder.c_str());
        LM(SFSaveDisk) = CW(-CW(vcbp->vcbVRefNum));
    }
    if(is_unix_path(ROMlib_SystemFolder.c_str()))
    {
        if(Ustat(ROMlib_SystemFolder.c_str(), &sbuf) < 0)
        {
            fprintf(stderr, "Couldn't find '%s'\n", ROMlib_SystemFolder.c_str());
            exit(1);
        }
        cpb.hFileInfo.ioNamePtr = RM((StringPtr)SYSMACNAME);
        cpb.hFileInfo.ioVRefNum = CWC(-1);
        cpb.hFileInfo.ioDirID = CL((LONGINT)ST_INO(sbuf));
    }
    else
    {
        sysnamelen = 1 + ROMlib_SystemFolder.size() + 1 + strlen(SYSMACNAME + 1) + 1;
        sysname = (char *)alloca(sysnamelen);
        *sysname = sysnamelen - 2; /* don't count first byte or nul */
        sprintf(sysname + 1, "%s:%s", ROMlib_SystemFolder.c_str(), SYSMACNAME + 1);
        cpb.hFileInfo.ioNamePtr = RM((StringPtr)sysname);
        cpb.hFileInfo.ioVRefNum = 0;
        cpb.hFileInfo.ioDirID = 0;
    }
    cpb.hFileInfo.ioFDirIndex = CWC(0);
    if(PBGetCatInfo(&cpb, false) == noErr)
    {
        wpb.ioNamePtr = 0;
        wpb.ioVRefNum = cpb.hFileInfo.ioVRefNum;
        wpb.ioWDProcID = TICKX("unix");
        wpb.ioWDDirID = cpb.hFileInfo.ioFlParID;
        if(PBOpenWD(&wpb, false) == noErr)
            LM(BootDrive) = wpb.ioVRefNum;
    }
    else
    {
        fprintf(stderr, "Couldn't open System: '%s'\n", ROMlib_SystemFolder.c_str());
        exit(1);
    }
#if defined(MSDOS) || defined(CYGWIN32)
    {
        static char drive_to_mount[4] = "x:/";

#if defined(MSDOS)
        if(ROMlib_dosdrives == ~0)
        {
            struct mntent *mp;
            FILE *mnt_fp;

            mnt_fp = setmntent("", "");
            if(mnt_fp)
            {
                while((mp = getmntent(mnt_fp)))
                {
                    drive_to_mount[0] = mp->mnt_dir[0];
                    {
                        struct statfs sbuf;
                        static char stat_test[] = "x:";

                        stat_test[0] = mp->mnt_dir[0];
                        if(statfs(stat_test, &sbuf) == 0)
                        {
                            uint32_t bit;

                            bit = drive_char_to_bit(stat_test[0]);
                            checkpoint_dosdrives(checkpointp, begin, bit);
                            ROMlib_automount(drive_to_mount);
                            check_for_executor_cd(drive_to_mount);
                            checkpoint_dosdrives(checkpointp, end, bit);
                        }
                    }
                }
                endmntent(mnt_fp);
            }
        }
        else
#endif
        {
            int i;

            for(i = 0; i <= 31; ++i)
            {
                uint32_t bit;

                bit = 1 << i;
                if(ROMlib_dosdrives & bit)
                {
                    drive_to_mount[0] = 'a' + i;
                    checkpoint_dosdrives(checkpointp, begin, bit);
#if defined(CYGWIN32)
                    drive_to_mount[0] += 'A' - 'a';
                    if(win_access(drive_to_mount))
                    {
#endif
                        ROMlib_automount(drive_to_mount);
#if defined(MSDOS) || defined(CYGWIN32)
                        check_for_executor_cd(drive_to_mount);
#endif
#if defined(CYGWIN32)
                    }
#endif
                    checkpoint_dosdrives(checkpointp, end, bit);
                }
            }
        }
    }
#endif

#if defined(MSDOS) || defined(CYGWIN32)
    if(ROMlib_dosdrives)
#endif
        futzwithdosdisks();

#if defined(MSDOS) || defined(CYGWIN32)
    if(!e2_is_mounted() && cd_big_hfv)
    {
        LONGINT m;

        ROMlib_openharddisk(cd_big_hfv, &m);
        if(m)
            cd_mounted_by_trickery_p = true;
    }
#endif
}

fcbrec *
Executor::PRNTOFPERR(INTEGER prn, OSErr *errp)
{
    fcbrec *retval;
    OSErr err;

    if(prn < 0 || prn >= CW(*(GUEST<INTEGER> *)MR(LM(FCBSPtr))) || (prn % 94) != 2)
    {
        retval = 0;
        err = rfNumErr;
    }
    else
    {
        retval = (fcbrec *)((char *)MR(LM(FCBSPtr)) + prn);
        if(!retval->fdfnum)
        {
            retval = 0;
            err = rfNumErr;
        }
        else
            err = noErr;
    }
    *errp = err;
    return retval;
}
