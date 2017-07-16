/* Copyright 1992-1993 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_hfsChanging[] =
	    "$Id: hfsChanging.c 86 2005-05-25 00:47:12Z ctm $";
#endif

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "MemoryMgr.h"
#include "rsys/hfs.h"

using namespace Executor;
using namespace ByteSwap;

typedef enum {
	GetOp,
	SetOp,
	LockOp,
	UnlockOp
} changeop;

PRIVATE OSErr PBFInfoHelper(changeop op, fileParam *pb, LONGINT dirid,
								  BOOLEAN async)
{
    OSErr err, err1;
    HVCB *vcbp;
    filerec *frp = NULL;
    catkey *catkeyp = NULL;
    btparam btparamrec;
    filekind kind;
    
    vcbp = 0;
    if (op == GetOp && (BigEndianValue(pb->ioFDirIndex) > 0))
	err = ROMlib_btpbindex((ioParam *) pb, dirid, &vcbp, &frp, &catkeyp,
									 TRUE);
    else {
	kind = regular;
	err = ROMlib_findvcbandfile((ioParam *) pb, dirid, &btparamrec, &kind,
									FALSE);
	if (err == noErr) {
	    if (btparamrec.success) {
		vcbp = btparamrec.vcbp;
		frp  = (filerec *) DATAPFROMKEY(btparamrec.foundp);
		catkeyp = (catkey *) btparamrec.foundp;
	    } else
		err = fnfErr;
	}
    }
    if (err == noErr) {
	switch (op) {
	case GetOp:
	    if (BigEndianValue(pb->ioFDirIndex) > 0 && pb->ioNamePtr)
		str255assign(MR(pb->ioNamePtr), catkeyp->ckrCName);
	    pb->ioFlAttrib = CB (open_attrib_bits (BigEndianValue (frp->filFlNum), vcbp,
						   &pb->ioFRefNum));
	    pb->ioFlAttrib |= frp->filFlags & CB (INHERITED_FLAG_BITS);
	    pb->ioFlVersNum = 0;
	    memmove(&pb->ioFlFndrInfo, &frp->filUsrWds,
		    (LONGINT) sizeof(pb->ioFlFndrInfo));
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
	case SetOp:
	    memmove(&frp->filUsrWds, &pb->ioFlFndrInfo,
		    (LONGINT) sizeof(frp->filUsrWds));
	    frp->filCrDat = pb->ioFlCrDat;
	    frp->filMdDat = pb->ioFlMdDat;
	    ROMlib_dirtyleaf(frp, vcbp);
	    ROMlib_flushvcbp (vcbp);
	    break;
	case LockOp:
	    frp->filFlags |= FSOFTLOCKBIT;
	    ROMlib_dirtyleaf(frp, vcbp);
	    break;
	case UnlockOp:
	    frp->filFlags &= ~FSOFTLOCKBIT;
	    ROMlib_dirtyleaf(frp, vcbp);
	    break;
	}
    }
    if (vcbp) {
	err1 = ROMlib_cleancache(vcbp);
	if (err == noErr)
	    err = err1;
    }
    PBRETURN(pb, err);
}

PUBLIC OSErr Executor::hfsPBGetFInfo(ParmBlkPtr pb, BOOLEAN async)
{
    return PBFInfoHelper(GetOp, (fileParam *) pb, 0L, async);
}

PUBLIC OSErr Executor::hfsPBHGetFInfo(HParmBlkPtr pb, BOOLEAN async)
{
    return PBFInfoHelper(GetOp, (fileParam *) pb, BigEndianValue(pb->fileParam.ioDirID), async);
}

PUBLIC OSErr Executor::hfsPBSetFInfo(ParmBlkPtr pb, BOOLEAN async)
{
    return PBFInfoHelper(SetOp, (fileParam *) pb, 0L, async);
}

PUBLIC OSErr Executor::hfsPBHSetFInfo(HParmBlkPtr pb, BOOLEAN async)
{
    return PBFInfoHelper(SetOp, (fileParam *) pb, BigEndianValue(pb->fileParam.ioDirID), async);
}

PUBLIC OSErr Executor::hfsPBSetFLock(ParmBlkPtr pb, BOOLEAN async)
{
    return PBFInfoHelper(LockOp, (fileParam *) pb, 0L, async);
}

PUBLIC OSErr Executor::hfsPBHSetFLock(HParmBlkPtr pb, BOOLEAN async)
{
    return PBFInfoHelper(LockOp, (fileParam *) pb, BigEndianValue(pb->fileParam.ioDirID), async);
}

PUBLIC OSErr Executor::hfsPBRstFLock(ParmBlkPtr pb, BOOLEAN async)
{
    return PBFInfoHelper(UnlockOp, (fileParam *) pb, 0L, async);
}

PUBLIC OSErr Executor::hfsPBHRstFLock(HParmBlkPtr pb, BOOLEAN async)
{
    return PBFInfoHelper(UnlockOp, (fileParam *) pb,
			 BigEndianValue (pb->fileParam.ioDirID), async);
}

PUBLIC OSErr Executor::hfsPBSetFVers(ParmBlkPtr pb, BOOLEAN async)
{
    PBRETURN((ioParam *)pb, wrgVolTypErr);
}

PUBLIC void
ROMlib_fcbrename (HVCB *vcbp, LONGINT swapped_parid, StringPtr oldnamep,
		  StringPtr newnamep)
{
  short length;
  filecontrolblock *fcbp, *efcbp;
  HVCB *swapped_vcbp;

  swapped_vcbp = RM (vcbp);
  length = BigEndianValue(*(short *)MR(FCBSPtr));
  fcbp = (filecontrolblock *) ((short *)MR(FCBSPtr)+1);
  efcbp = (filecontrolblock *) ((char *)MR(FCBSPtr) + length);
  for (;fcbp < efcbp;
       fcbp = (filecontrolblock *) ((char *)fcbp + BigEndianValue(FSFCBLen)))
    {
      if (fcbp->fcbDirID == swapped_parid
	  && fcbp->fcbVPtr == swapped_vcbp
	  && RelString (fcbp->fcbCName, oldnamep, FALSE, TRUE) == 0)
	str255assign (fcbp->fcbCName, newnamep);
    }
}
    
PRIVATE OSErr
renamehelper(ioParam *pb, BOOLEAN async, LONGINT dirid, filekind kind)
{
  OSErr err, err1;
  btparam btparamrec, btparamrec2;
  ioParam npb;

  err = ROMlib_findvcbandfile(pb, dirid, &btparamrec, &kind, FALSE);
  if (err == noErr)
    {
      npb = *pb;
      npb.ioNamePtr = (StringPtr) (long) pb->ioMisc;
      err = ROMlib_findvcbandfile(&npb, dirid, &btparamrec2, &kind, FALSE);
      if (err != fnfErr)
	{
	  if (err != bdNamErr)
	    err = dupFNErr;
	}
      else
	{
	  err = ROMlib_writevcbp(btparamrec.vcbp);
	  if (err == noErr)
	    {
	      err = ROMlib_btrename(&btparamrec,
				    (StringPtr) (long) MR(pb->ioMisc));
	      if (err == noErr)
		ROMlib_fcbrename (btparamrec.vcbp,
				  btparamrec.tofind.catk.ckrParID,
				  (StringPtr)
				  &btparamrec.tofind.catk.ckrCName[0],
				  (StringPtr) BigEndianValue (pb->ioMisc));
	    }
	  err1 = ROMlib_cleancache(btparamrec.vcbp);
	  if (err1 == noErr)
	    err1 = ROMlib_flushvcbp(btparamrec.vcbp);
	  if (err == noErr)
	    err = err1;
	}
    }
/*
 * This first test of !pb->ioNamePtr makes me nervous.  Perhaps we should
 * use thread information to locate the directory "dirid" instead of assuming
 * a volumerename is needed.  TODO -- FIXME -- test on Mac.
 */

  if (err == noErr)
    {
      StringPtr nameptr;

      nameptr = MR(pb->ioNamePtr);
      if (!pb->ioNamePtr
	  || (ROMlib_indexn((char *)nameptr+1, ':', nameptr[0])
	      == (char *) nameptr + nameptr[0]))
	{
	  err = ROMlib_pbvolrename(pb, (StringPtr) (long) MR(pb->ioMisc));
	  dirid = 1;
	}
    }
  PBRETURN(pb, err);
}

PUBLIC OSErr Executor::hfsPBRename(ParmBlkPtr pb, BOOLEAN async)
{
    return renamehelper((ioParam *) pb, async, 0L, regular);
}

PUBLIC OSErr Executor::hfsPBHRename(HParmBlkPtr pb, BOOLEAN async)
{
    return renamehelper((ioParam *) pb, async, BigEndianValue(pb->fileParam.ioDirID),
							    (filekind)(regular|directory));
}
