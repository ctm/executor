#if defined(OUTDATEDCODE)
#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "myhfs.h"
#include <string.h>

PUBLIC INTEGER flnumtorefnum(ulong flnum)
{
    INTEGER length;
    filecontrolblock *fcbp, *efcbp;

    length = *(short *)FCBSPtr;
    fcbp = (filecontrolblock *)((short *)FCBSPtr + 1);
    efcbp = (filecontrolblock *)((char *)FCBSPtr + length);
    for(; fcbp < efcbp && fcbp->fcbFlNum != flnum;
        fcbp = (filecontrolblock *)((char *)fcbp + FSFCBLen))
        ;
    return fcbp < efcbp ? (char *)fcbp - (char *)FCBSPtr : 0;
}

typedef enum { Get,
               Set,
               Lock,
               Unlock } changeop;

PRIVATE OSErr PBFInfoHelper(changeop op, fileParam *pb, long dirid,
                            BOOLEAN async)
{
    OSErr err, err1;
    HVCB *vcbp;
    filerec *frp;
    catkey *catkeyp;
    btparam btparamrec;
    filekind kind;

    if(op == Get && pb->ioFDirIndex > 0)
        err = btpbindex((ioParam *)pb, dirid, &vcbp, &frp, &catkeyp, true);
    else
    {
        kind = regular;
        err = findvcbandfile((ioParam *)pb, dirid, &btparamrec, &kind, false);
        if(err == noErr)
        {
            if(btparamrec.success)
            {
                vcbp = btparamrec.vcbp;
                frp = (filerec *)DATAPFROMKEY(btparamrec.foundp);
                catkeyp = (catkey *)btparamrec.foundp;
            }
            else
                err = fnfErr;
        }
    }
    if(err == noErr)
    {
        switch(op)
        {
            case Get:
                if(pb->ioNamePtr)
                    str255assign(pb->ioNamePtr, catkeyp->ckrCName);
                pb->ioFRefNum = flnumtorefnum(frp->filFlNum);
                pb->ioFlAttrib = frp->filFlags;
                pb->ioFlVersNum = 0;
                memcpy(&pb->ioFlFndrInfo, &frp->filUsrWds,
                       (size_t)sizeof(pb->ioFlFndrInfo));
                pb->ioFlNum = frp->filFlNum;
                pb->ioFlStBlk = frp->filStBlk;
                pb->ioFlLgLen = frp->filLgLen;
                pb->ioFlPyLen = frp->filPyLen;
                pb->ioFlRStBlk = frp->filRStBlk;
                pb->ioFlRLgLen = frp->filRLgLen;
                pb->ioFlRPyLen = frp->filRPyLen;
                pb->ioFlCrDat = frp->filCrDat;
                pb->ioFlMdDat = frp->filMdDat;
                break;
            case Set:
                memcpy(&frp->filUsrWds, &pb->ioFlFndrInfo,
                       (size_t)sizeof(frp->filUsrWds));
                frp->filCrDat = pb->ioFlCrDat;
                frp->filMdDat = pb->ioFlMdDat;
                dirtyleaf(frp, vcbp);
                break;
            case Lock:
                frp->filFlags |= FSOFTLOCKBIT;
                dirtyleaf(frp, vcbp);
                break;
            case Unlock:
                frp->filFlags &= ~FSOFTLOCKBIT;
                dirtyleaf(frp, vcbp);
                break;
        }
    }
    err1 = cleancache(vcbp);
    if(err == noErr)
        err = err1;
    PBRETURN(pb, err);
}

PUBLIC OSErr myPBGetFInfo(fileParam *pb, BOOLEAN async)
{
    return PBFInfoHelper(Get, pb, 1L, async);
}

PUBLIC OSErr myPBHGetFInfo(HFileParam *pb, BOOLEAN async)
{
    return PBFInfoHelper(Get, (fileParam *)pb, pb->ioDirID, async);
}

PUBLIC OSErr myPBSetFInfo(HFileParam *pb, BOOLEAN async)
{
    return PBFInfoHelper(Set, (fileParam *)pb, 1L, async);
}

PUBLIC OSErr myPBHSetFInfo(HFileParam *pb, BOOLEAN async)
{
    return PBFInfoHelper(Set, (fileParam *)pb, pb->ioDirID, async);
}

PUBLIC OSErr myPBSetFLock(HFileParam *pb, BOOLEAN async)
{
    return PBFInfoHelper(Lock, (fileParam *)pb, 1L, async);
}

PUBLIC OSErr myPBHSetFLock(HFileParam *pb, BOOLEAN async)
{
    return PBFInfoHelper(Lock, (fileParam *)pb, pb->ioDirID, async);
}

PUBLIC OSErr myPBRstFlock(HFileParam *pb, BOOLEAN async)
{
    return PBFInfoHelper(Unlock, (fileParam *)pb, 1L, async);
}

PUBLIC OSErr myPBHRstFlock(HFileParam *pb, BOOLEAN async)
{
    return PBFInfoHelper(Unlock, (fileParam *)pb, 1L, async);
}

PUBLIC OSErr myPBSetFVers(ioParam *pb, BOOLEAN async)
{
    PBRETURN(pb, wrgVolTypErr);
}

PRIVATE OSErr renamehelper(ioParam *pb, BOOLEAN async, LONGINT dirid, filekind kind)
{
    OSErr err, err1;
    btparam btparamrec, btparamrec2;
    ioParam npb;

    if(!pb->ioNamePtr || indexn((char *)pb->ioNamePtr + 1, ':', pb->ioNamePtr[0]) == (char *)pb->ioNamePtr + pb->ioNamePtr[0])
        err = pbvolrename(pb, (StringPtr)pb->ioMisc);
    else
    {
        err = findvcbandfile(pb, dirid, &btparamrec, &kind, false);
        if(err == noErr)
        {
            npb = *pb;
            npb.ioNamePtr = (StringPtr)pb->ioMisc;
            err = findvcbandfile(&npb, dirid, &btparamrec2, &kind, false);
            if(err != fnfErr)
                err = dupFNErr;
            else
            {
                err = writevcbp(btparamrec.vcbp);
                if(err == noErr)
                    err = btrename(&btparamrec, (StringPtr)pb->ioMisc);
                err1 = cleancache(btparamrec.vcbp);
                if(err == noErr)
                    err = err1;
            }
        }
    }
    PBRETURN(pb, err);
}

PUBLIC OSErr myPBRename(HFileParam *pb, BOOLEAN async)
{
    return renamehelper((ioParam *)pb, async, (LONGINT)1, regular);
}

PUBLIC OSErr myPBHRename(HFileParam *pb, BOOLEAN async)
{
    return renamehelper((ioParam *)pb, async, pb->ioDirID, regular | directory);
}
#endif
