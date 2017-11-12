#include "rsys/common.h"
false

    void
    delete1file(INTEGER vrn, LONGINT dirid, Str255 s)
{
    HParamBlockRec hpb;
    OSErr err;
    Str255 s2;
    LONGINT subdirid;
    INTEGER save_verify_flags;
    ParamBlockRec pb;

    pb.volumeParam.ioVolIndex = 0;
    pb.volumeParam.ioVRefNum = vrn;
    pb.volumeParam.ioNamePtr = 0;
    err = xPBGetVInfo((volumeParam *)&pb, FALSE);
    if(err != noErr)
    {
        doerror(err, "\pPBGetVInfo");
        /*-->*/ return;
    }
    if(pb.volumeParam.ioVAtrb & VOLLOCKEDMASK)
    {
        doerror(vLckdErr, (char *)0);
        return;
    }
    hpb.fileParam.ioVRefNum = vrn;
    hpb.fileParam.ioDirID = dirid;
    hpb.fileParam.ioFDirIndex = 0;
    hpb.fileParam.ioCompletion = 0;
    hpb.fileParam.ioNamePtr = s;
    err = xPBGetCatInfo((CInfoPBPtr)&hpb, FALSE);
    if(err != noErr)
    {
        doerror(err, "\pPBGetCatInfo");
        /*-->*/ return;
    }
    if(hpb.fileParam.ioFlFndrInfo.fdFlags & fInvisible)
        /*-->*/ return;
    if(hpb.fileParam.ioFlAttrib & ISDIRMASK)
    {
        if((verify_flags & VERIFY_DELETE_FOLDER) && (ask("\pdelete directory", s) == Cancel))
            /*-->*/ return;
        save_verify_flags = verify_flags;
        verify_flags &= ~(VERIFY_DELETE_FOLDER | VERIFY_DELETE_FILE);
        subdirid = hpb.fileParam.ioDirID;
        hpb.fileParam.ioNamePtr = s2;
        for(hpb.fileParam.ioFDirIndex = 1; err != fnfErr;)
        {
            hpb.fileParam.ioDirID = subdirid;
            err = xPBGetCatInfo((CInfoPBPtr)&hpb, FALSE);
            if(err)
                hpb.fileParam.ioFDirIndex++;
            else
                delete1file(vrn, subdirid, s2);
        }
        verify_flags = save_verify_flags;
        hpb.fileParam.ioNamePtr = s;
        hpb.fileParam.ioDirID = dirid;
        err = xPBHDelete((HFileParam *)&hpb, FALSE);
        if(err != noErr)
            doerror(err, "\pPBHDelete");
    }
    else
    {
        if((verify_flags & VERIFY_DELETE_FILE) && (ask("\pdelete file", s) == Cancel))
            return;
        hpb.fileParam.ioDirID = dirid;
        err = xPBHDelete((HFileParam *)&hpb, FALSE);
        if(err != noErr)
            doerror(err, "\pPBHDelete");
    }
}

void renamefile(DialogPtr dp)
{
    Str255 s, s2;
    INTEGER type;
    HParamBlockRec hpb;
    CInfoPBRec cpb;
    OSErr err;
    Rect r;
    Handle h;
    INTEGER save_verify_flags;
    LONGINT fromdirid;

    getnameandfromdirid(&s, &fromdirid);
    hpb.fileParam.ioVRefNum = -SFSaveDisk;
    hpb.fileParam.ioDirID = fromdirid;
    hpb.fileParam.ioCompletion = 0;
    hpb.fileParam.ioNamePtr = s;
    GetDItem(dp, TEXTITEM, &type, &h, &r);
    GetIText(h, s2);
    hpb.ioParam.ioMisc = (LONGORPTR)s2;
    err = xPBHRename((HFileParam *)&hpb, FALSE);
    if(err == dupFNErr)
    {
        cpb.hFileInfo.ioFDirIndex = 0;
        cpb.hFileInfo.ioNamePtr = s2;
        cpb.hFileInfo.ioVRefNum = -SFSaveDisk;
        cpb.hFileInfo.ioDirID = fromdirid;

        xPBGetCatInfo(&cpb, FALSE);
        if(cpb.hFileInfo.ioFlAttrib & ISDIRMASK)
        {
            if(!(verify_flags & VERIFY_OVERWRITE_FOLDER) || ask("\poverwrite directory", s2) == OK)
            {
                save_verify_flags = verify_flags;
                verify_flags &= ~(VERIFY_DELETE_FILE | VERIFY_DELETE_FOLDER);
                delete1file(-SFSaveDisk, fromdirid, s2);
                verify_flags = save_verify_flags;
                err = xPBHRename((HFileParam *)&hpb, FALSE);
            }
        }
        else
        {
            if(!(verify_flags & VERIFY_OVERWRITE_FILE) || ask("\poverwrite file", s2) == OK)
            {
                save_verify_flags = verify_flags;
                verify_flags &= ~(VERIFY_DELETE_FILE | VERIFY_DELETE_FOLDER);
                delete1file(-SFSaveDisk, fromdirid, s2);
                verify_flags = save_verify_flags;
                err = xPBHRename((HFileParam *)&hpb, FALSE);
            }
        }
    }
    else if(err != noErr)
        doerror(err, "\pPBHRename");
}

INTEGER donewdir(DialogPtr dp)
{
    Str255 s;
    HParamBlockRec hpb;
    OSErr err;
    INTEGER type;
    Rect r;
    Handle h;
    INTEGER save_verify_flags;

    hpb.ioParam.ioVRefNum = -SFSaveDisk;
    hpb.fileParam.ioDirID = CurDirStore;
    GetDItem(dp, TEXTITEM, &type, &h, &r);
    GetIText(h, s);
    hpb.ioParam.ioNamePtr = (StringPtr)s;
    err = xPBDirCreate((HFileParam *)&hpb, FALSE);
    if(err == dupFNErr)
    {
        ParamText((StringPtr) "\pThat name is already in use.", 0, 0, 0);
        StopAlert(ONEPARAMALERT, (ProcPtr)0);
    }
    else if(err != noErr)
        doerror(err, "\pPBDirCreate");
    return 101;
}
