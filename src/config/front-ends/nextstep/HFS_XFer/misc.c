#if defined(OUTDATEDCODE)
#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "myhfs.h"
#include <string.h>

#define TESTFCB
#if defined(TESTFCB)
PUBLIC void testfcb()
{
    short length;
    filecontrolblock *fp;
    INTEGER i;

    length = *(short *)FCBSPtr;
    fp = (filecontrolblock *)((short *)FCBSPtr + 1);
    printf("length = %d, length / 94 = %d, length mod 94 = %d\n",
           length, length / 94, length % 94);
    for(i = 0; i < 40 && i < length / 94; i++, fp++)
    {
        printf("# %ld flags 0x%x vers %d sblk %d EOF %ld PLEN %ld mark %ld\n"
               "vptr 0x%lx pbuffer 0x%lx FlPos %d clmpsiz %ld BTCBPtr 0x%lx\n"
               "ext (%d %d) (%d %d) (%d %d) FNDR '%c%c%c%c' CatPos 0x%lx\n"
               "parid %ld name %s\n",
               fp->fcbFlNum, fp->fcbMdRByt,
               fp->fcbTypByt, fp->fcbSBlk, fp->fcbEOF, fp->fcbPLen, fp->fcbCrPs,
               fp->fcbVPtr, fp->fcbBfAdr, fp->fcbFlPos, fp->fcbClmpSize,
               fp->fcbBTCBPtr,
               fp->fcbExtRec[0].blockstart, fp->fcbExtRec[0].blockcount,
               fp->fcbExtRec[1].blockstart, fp->fcbExtRec[1].blockcount,
               fp->fcbExtRec[2].blockstart, fp->fcbExtRec[2].blockcount,
               (short)(fp->fcbFType >> 24), (short)(fp->fcbFType >> 16),
               (short)fp->fcbFType >> 8, (short)fp->fcbFType,
               fp->fcbCatPos, fp->fcbDirID, fp->fcbCName + 1);
    }
}
#endif /* TESTFCB */

#if defined(FOO)
PRIVATE void foo(short drivenumber)
{
    CInfoPBRec pb;
    OSErr err;
    StringPtr table[] = {
        (StringPtr) "\pMyVol:",
        (StringPtr) "\pMyVol:DeskTop",
        (StringPtr) "\pMyVol:IRS",
        (StringPtr) "\pMyVol:FBI",
        (StringPtr) "\pMyVol:Mail:",
        (StringPtr) "\pMyVol:Mail:Jody",
        (StringPtr) "\pMyVol:Mail:Bob",
        0
    },
              *strpp;

    pb.hFileInfo.ioFDirIndex = 0;
    for(strpp = table; *strpp; strpp++)
    {
        pb.hFileInfo.ioNamePtr = *strpp;
        err = xPBGetCatInfo(&pb, false);
        if(err)
        {
            printf("getcatinfo failed %d\n", err);
            exit(1);
        }
        printf("%ld\n", pb.hFileInfo.ioDirID);
    }
}
#endif /* FOO */

PUBLIC void myFInitQueue(void) /* IMIV-128 */
{
    /* When we support asynchronous stuff we'll have to do this */
}

PUBLIC QHdrPtr myGetFSQHdr(void)
{
    return &FSQHdr;
}

PUBLIC QHdrPtr myGetVCBQHdr(void)
{
    return &VCBQHdr;
}

#if defined(UNIX)
PUBLIC Ptr WDCBsPtr;
#endif

PUBLIC OSErr myPBGetFCBInfo(FCBPBPtr pb, BOOLEAN async)
{
    filecontrolblock *fcbp, *efcbp;
    INTEGER i;

    if((i = pb->ioFCBIndx) > 0)
    {
        fcbp = (filecontrolblock *)(FCBSPtr + sizeof(INTEGER));
        efcbp = (filecontrolblock *)(FCBSPtr + *(INTEGER *)WDCBsPtr);
        if(pb->ioVRefNum < 0)
        {
            for(; fcbp != efcbp; fcbp++)
                if(fcbp->fcbVPtr->vcbVRefNum == pb->ioVRefNum && --i <= 0)
                    break;
        }
        else if(pb->ioVRefNum == 0)
        {
            for(; fcbp != efcbp && --i > 0; fcbp++)
                ;
        }
        else /* if (pb->ioVRefNum > 0 */
        {
            for(; fcbp != efcbp; fcbp++)
                if(fcbp->fcbVPtr->vcbDrvNum == pb->ioVRefNum && --i <= 0)
                    break;
        }
        if(fcbp == efcbp)
            PBRETURN(pb, fnOpnErr);
        pb->ioRefNum = (char *)fcbp - (char *)FCBSPtr;
    }
    else
    {
        fcbp = refnumtofcbp(pb->ioRefNum);
        if(!fcbp)
            PBRETURN(pb, rfNumErr);
    }
    if(pb->ioNamePtr)
        str255assign(pb->ioNamePtr, fcbp->fcbCName);
    pb->ioFCBFlNm = fcbp->fcbFlNum;
    pb->ioFCBFlags = (fcbp->fcbMdRByt << 8) | (unsigned char)fcbp->fcbTypByt;
    pb->ioFCBStBlk = fcbp->fcbSBlk;
    pb->ioFCBEOF = fcbp->fcbEOF;
    pb->ioFCBPLen = fcbp->fcbPLen;
    pb->ioFCBCrPs = fcbp->fcbCrPs;
    pb->ioFCBVRefNum = fcbp->fcbVPtr->vcbVRefNum;
    if(pb->ioFCBIndx <= 0 || pb->ioVRefNum == 0)
        pb->ioVRefNum = pb->ioFCBVRefNum;
    pb->ioFCBClpSiz = fcbp->fcbClmpSize;
    pb->ioFCBParID = fcbp->fcbDirID;
    PBRETURN(pb, noErr);
}

PUBLIC QHdrPtr myGetDrvQHdr(void)
{
    return &DrvQHdr;
}

#if !defined(UNIX)
#define LINKHACK
#endif

#if defined(LINKHACK)
PUBLIC void bcopy(void *srcp, void *dstp, LONGINT length)
{
    BlockMove(srcp, dstp, length);
}

PUBLIC void bzero(void *dstp, LONGINT ntozero)
{
    char *dstcp;

    dstcp = dstp;
    while(--ntozero >= 0)
        *dstcp++ = 0;
}
#endif /* LINKHACK */
#endif
