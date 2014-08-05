/* Copyright 1992 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_hfsMisc[] =
	    "$Id: hfsMisc.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "MemoryMgr.h"
#include "rsys/hfs.h"
#include "rsys/file.h"

using namespace Executor;
using namespace ByteSwap;

#if defined (TESTFCB)
PUBLIC void testfcb()
{
    short length;
    filecontrolblock *fp;
    INTEGER i;
    
    length =  BigEndianValue(*(short *)BigEndianValue(FCBSPtr));
    fp = (filecontrolblock *)((short *)BigEndianValue(FCBSPtr) + 1);
    printf("length = %d, length / 94 = %d, length mod 94 = %d\n",
	length, length / 94, length % 94);
    for (i = 0; i < 40 && i < length / 94; i++, fp++) {
	printf("# %ld flags 0x%x vers %d sblk %d EOF %ld PLEN %ld mark %ld\n"
	       "vptr 0x%lx pbuffer 0x%lx FlPos %d clmpsiz %ld BTCBPtr 0x%lx\n"
	       "ext (%d %d) (%d %d) (%d %d) FNDR '%c%c%c%c' CatPos 0x%lx\n"
	       "parid %ld name %s\n", BigEndianValue(fp->fcbFlNum), fp->fcbMdRByt,
	       fp->fcbTypByt, BigEndianValue(fp->fcbSBlk), BigEndianValue(fp->fcbEOF), BigEndianValue(fp->fcbPLen), BigEndianValue(fp->fcbCrPs),
	       BigEndianValue(fp->fcbVPtr), BigEndianValue(fp->fcbBfAdr), BigEndianValue(fp->fcbFlPos), BigEndianValue(fp->fcbClmpSize),
	       BigEndianValue(fp->fcbBTCBPtr),
	       BigEndianValue(fp->fcbExtRec[0].blockstart), BigEndianValue(fp->fcbExtRec[0].blockcount),
	       BigEndianValue(fp->fcbExtRec[1].blockstart), BigEndianValue(fp->fcbExtRec[1].blockcount),
	       BigEndianValue(fp->fcbExtRec[2].blockstart), BigEndianValue(fp->fcbExtRec[2].blockcount),
	       (short) (BigEndianValue(fp->fcbFType) >> 24), (short) (BigEndianValue(fp->fcbFType) >> 16),
	       (short) BigEndianValue(fp->fcbFType) >> 8,  (short) BigEndianValue(fp->fcbFType),
	       BigEndianValue(fp->fcbCatPos), BigEndianValue(fp->fcbDirID), fp->fcbCName+1);
    }
}
#endif /* TESTFCB */

#if 0
PUBLIC void myFInitQueue( void )    /* IMIV-128 */
{
    /* When we support asynchronous stuff we'll have to do this */
}

PUBLIC QHdrPtr myGetFSQHdr( void )
{
    return &FSQHdr;
}

PUBLIC QHdrPtr myGetVCBQHdr( void )
{
    return &VCBQHdr;
}
#endif

/*
 * NOTE:  This is *the* PCBGetFCBInfo that we use whether
 *	  we're looking at hfs or ufs stuff.
 */

A2(PUBLIC trap, OSErrRET, PBGetFCBInfo, FCBPBPtr, pb, BOOLEAN, async)
{
    filecontrolblock *fcbp, *efcbp;
    INTEGER i;
    
    if ((i = BigEndianValue(pb->ioFCBIndx)) > 0) {
	fcbp  = (filecontrolblock *) (MR(FCBSPtr) + sizeof(INTEGER));
	efcbp = (filecontrolblock *) (MR(FCBSPtr) + BigEndianValue(*(INTEGER *)MR(FCBSPtr)));
	if (BigEndianValue(pb->ioVRefNum) < 0) {
	    for (;fcbp != efcbp; fcbp++)
		if (fcbp->fcbFlNum &&
		        MR(fcbp->fcbVPtr)->vcbVRefNum == pb->ioVRefNum && --i <= 0)
		    break;
	} else if (pb->ioVRefNum == 0) {
	    for (;fcbp != efcbp && (fcbp->fcbFlNum == 0 || --i > 0); fcbp++)
		;
	} else /* if (BigEndianValue(pb->ioVRefNum) > 0 */ {
	    for (;fcbp != efcbp; fcbp++)
		if (fcbp->fcbFlNum &&
		         MR(fcbp->fcbVPtr)->vcbDrvNum == pb->ioVRefNum && --i <= 0)
		    break;
	}
	if (fcbp == efcbp)
	    PBRETURN(pb, fnOpnErr);
	pb->ioRefNum = BigEndianValue((char *) fcbp - (char *) MR(FCBSPtr));
    } else {
	fcbp = ROMlib_refnumtofcbp(BigEndianValue(pb->ioRefNum));
	if (!fcbp)
	    PBRETURN(pb, rfNumErr);
    }
    if (pb->ioNamePtr)
	str255assign(MR(pb->ioNamePtr), fcbp->fcbCName);
    pb->ioFCBFlNm = fcbp->fcbFlNum;
    pb->ioFCBFlags = BigEndianValue((fcbp->fcbMdRByt <<8) | (unsigned char) fcbp->fcbTypByt);
    pb->ioFCBStBlk = fcbp->fcbSBlk;
    pb->ioFCBEOF = fcbp->fcbEOF;
    if (MR(fcbp->fcbVPtr)->vcbCTRef) {
	pb->ioFCBCrPs = fcbp->fcbCrPs;	/* HFS */
	pb->ioFCBPLen = fcbp->fcbPLen;
    } else {
	pb->ioFCBCrPs = BigEndianValue((ULONGINT)(lseek(((fcbrec *)fcbp)->fcfd, 0, L_INCR) -    /* UFS */
						   FORKOFFSET((fcbrec *) fcbp)));
	pb->ioFCBPLen = fcbp->fcbEOF;
    }
    pb->ioFCBVRefNum = MR(fcbp->fcbVPtr)->vcbVRefNum;
    if (BigEndianValue(pb->ioFCBIndx) <= 0 || pb->ioVRefNum == 0)
	pb->ioVRefNum = pb->ioFCBVRefNum;
    pb->ioFCBClpSiz = fcbp->fcbClmpSize;
    pb->ioFCBParID = fcbp->fcbDirID;
    PBRETURN(pb, noErr);
}

#if 0
PUBLIC QHdrPtr myGetDrvQHdr( void )
{
    return &DrvQHdr;
}
#endif

#if defined(MAC)
#define LINKHACK
#endif

#if defined (LINKHACK)
PUBLIC void bcopy(void *srcp, void *dstp, LONGINT length)
{
    BlockMove(srcp, dstp, length);
}

PUBLIC void bzero(void *dstp, LONGINT ntozero)
{
    char *dstcp;
    
    dstcp = dstp;
    while (--ntozero >= 0)
	*dstcp++ = 0;
}
#endif /* LINKHACK */
