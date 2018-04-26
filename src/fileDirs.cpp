/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
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

#include "rsys/hfs.h"
#include "rsys/file.h"
#include "rsys/filedouble.h"

using namespace Executor;

/*
 * Remember, vrn and wdn are negative numbers
 */

OSErr Executor::ufsPBGetCatInfo(CInfoPBPtr pb, BOOLEAN a) /* INTERNAL */
{
    GUEST<LONGINT> swapped_dir;
    OSErr retval;

    swapped_dir = pb->hFileInfo.ioDirID;
    retval = ROMlib_PBGetSetFInfoD((ParmBlkPtr)pb, a, Get, &swapped_dir, true);
#if 0

  /* Although this looks right, it's wrong.  Swapped_dir gets filled with
     the parent directory id.  ROMlib_PBGetSetFInfoD fills in the correct
     information based on dodirs being true already, so nothing is needed
     here.

  pb->hFileInfo.ioDirID = swapped_dir;

  */
#endif

    pb->hFileInfo.ioACUser = 0;

    return retval;
}

OSErr Executor::ufsPBSetCatInfo(CInfoPBPtr pb, BOOLEAN a) /* INTERNAL */
{
    GUEST<LONGINT> swapped_dir;
    OSErr retval;

    swapped_dir = pb->hFileInfo.ioDirID;
    retval = ROMlib_PBGetSetFInfoD((ParmBlkPtr)pb, a, Set, &swapped_dir, true);
    return retval;
}

OSErr Executor::ROMlib_PBMoveOrRename(ParmBlkPtr pb, BOOLEAN a, LONGINT dir,
                                      LONGINT newdir, char *newname,
                                      MoveOrRenameType op) /* INTERNAL */
{
    ParamBlockRec npb;
    OSErr err;
    char *oldpathname, *oldfilename, *oldendname;
    char *newpathname, *newfilename, *newendname, *fullnewname;
    char *rname, *rnew;
    int renamefailed;
    VCBExtra *vcbp;
    struct stat sbuf;
    LONGINT dirid;
    ALLOCABEGIN

    oldpathname = 0;
    newpathname = 0;
    rname = 0;
    rnew = 0;

    if((err = ROMlib_nami(pb, dir, NoIndex, &oldpathname, &oldfilename,
                          &oldendname, true, &vcbp, &sbuf))
       == noErr)
    {
        npb.ioParam.ioNamePtr = RM((StringPtr)newname);
        npb.ioParam.ioVRefNum = pb->ioParam.ioVRefNum;
        if((err = ROMlib_nami(&npb, newdir, NoIndex, &newpathname,
                              &newfilename, &newendname, true, (VCBExtra **)0,
                              (struct stat *)0))
           == noErr)
        {
            if(op == CatMove)
            {
                fullnewname = (char *)ALLOCA(newendname - newpathname + oldendname - oldfilename + 2); /* + '/' and null */
                sprintf(fullnewname, "%s/%s", newpathname, oldfilename);
            }
            else
                fullnewname = newpathname;
            if(strcmp(oldpathname, fullnewname) != 0 && Ustat(fullnewname, &sbuf) != -1)
                err = dupFNErr;
            else
            {
                if(stat(oldpathname, &sbuf) != 0)
                {
                    sbuf.st_ino = 0;
#if defined(CYGWIN32)
                    sbuf.st_rdev = 0;
#endif
                }

                if((renamefailed = Urename(oldpathname, fullnewname)) == 0)
                {
                    ROMlib_dbm_delete_inode(vcbp, ST_INO(sbuf));
                    dirid = 0;
                    ROMlib_dbm_store(vcbp, fullnewname, &dirid, true);
                    rname = ROMlib_resname(oldpathname, oldfilename,
                                           oldendname);
                    if(Ustat(rname, &sbuf) != -1)
                    {
                        if(op == CatMove)
                        {
                            newfilename = strrchr(fullnewname, '/') + 1;
                            newendname = fullnewname + strlen(fullnewname) + 1;
                        }
                        rnew = ROMlib_resname(fullnewname, newfilename,
                                              newendname);
                        double_dir_op(rnew, mkdir_op);
                        renamefailed = Urename(rname, rnew);
                        double_dir_op(rname, rmdir_op);
                    }
                    /* TODO: eventually put a call to ROMlib_fcbrename here, but that requires
   us to know the parent id number, something we don't know right now. */
                }
                if(renamefailed != 0)
                    err = ROMlib_maperrno();
            }
        }
    }
    if(oldpathname)
        free(oldpathname);
    if(newpathname)
        free(newpathname);
    if(rname)
        free(rname);
    if(rnew)
        free(rnew);
    return err;
    ALLOCAEND
}

OSErr Executor::ufsPBCatMove(CMovePBPtr pb, BOOLEAN a) /* INTERNAL */
{
    return ROMlib_PBMoveOrRename((ParmBlkPtr)pb, a, Cx(pb->ioDirID),
                                 Cx(pb->ioNewDirID), (char *)MR(pb->ioNewName), CatMove);
}

OSErr Executor::ufsPBOpenWD(WDPBPtr pb, BOOLEAN a) /* INTERNAL */
{
    OSErr err;
    ParamBlockRec newpb;
    struct stat sbuf;
    char *pathname, *filename, *endname;
    VCBExtra *vcbp;
    LONGINT dirid;

    newpb.ioParam.ioNamePtr = pb->ioNamePtr;
    newpb.ioParam.ioVRefNum = pb->ioVRefNum;

    err = ROMlib_nami(&newpb, Cx(pb->ioWDDirID), NoIndex, &pathname, &filename,
                      &endname, true, &vcbp, &sbuf);
    if(err == noErr)
    {
        LONGINT l;

        ROMlib_automount(pathname);
        vcbp = (VCBExtra *)ROMlib_vcbbybiggestunixname(pathname);
        if(S_ISDIR(sbuf.st_mode))
            dirid = ST_INO(sbuf);
        else
            dirid = Cx(pb->ioWDDirID);
        if(dirid == vcbp->u.ufs.ino)
            dirid = 2;
        l = dirid;
        if(S_ISDIR(sbuf.st_mode))
            ROMlib_dbm_store(vcbp, pathname, &l, true);
        free(pathname);
        err = ROMlib_mkwd(pb, (VCB *)vcbp, dirid, Cx(pb->ioWDProcID));
    }
    return err;
}
