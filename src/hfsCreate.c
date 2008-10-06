/* Copyright 1992 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_hfsCreate[] =
	    "$Id: hfsCreate.c 86 2005-05-25 00:47:12Z ctm $";
#endif

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "rsys/hfs.h"

typedef enum { create, delete } createop;

PRIVATE OSErr freeallblocks(HVCB *vcbp, filerec *frp)
{
    OSErr retval;
    filecontrolblock *fcbp;
    ParamBlockRec pbr;
    
    fcbp = ROMlib_getfreefcbp();
    if (!fcbp)
	retval = tmfoErr;
    else {
	fcbp->fcbVPtr = RM(vcbp);
	fcbp->fcbFlNum = frp->filFlNum;
	fcbp->fcbPLen = frp->filPyLen;
	memmove((char *) fcbp->fcbExtRec, (char *) frp->filExtRec,
		(LONGINT) sizeof(frp->filExtRec));
	fcbp->fcbMdRByt = WRITEBIT;
	pbr.ioParam.ioMisc = 0;
	pbr.ioParam.ioRefNum = CW((char *) fcbp - (char *) MR(FCBSPtr));
	retval = ROMlib_allochelper((ioParam *) &pbr, FALSE, seteof, FALSE);
	if (retval == noErr) {
	    fcbp->fcbPLen = frp->filRPyLen;
	    memmove((char *) fcbp->fcbExtRec, (char *) frp->filRExtRec,
		    (LONGINT) sizeof(frp->filRExtRec));
	    fcbp->fcbMdRByt = WRITEBIT|RESOURCEBIT;
	    retval = ROMlib_allochelper((ioParam *) &pbr, FALSE, seteof, FALSE);
	}
    }
    fcbp->fcbFlNum = 0;
    return retval;
}

PRIVATE OSErr createhelper(ioParam *pb, BOOLEAN async, createop op,
						   LONGINT dirid, filekind kind)
{
    OSErr err, err1;
    filekind curkind;
    btparam btparamrec;
    directoryrec *drp;
    filerec *frp;
    HVCB *vcbp;
    
    curkind = regular | directory;
    err = ROMlib_findvcbandfile(pb, dirid, &btparamrec, &curkind, FALSE);
    vcbp = btparamrec.vcbp;
    switch (err) {
    case noErr:
	if (op == create)
	    err = dupFNErr;
	else {
	    if (curkind == directory) {
		drp = (directoryrec *) DATAPFROMKEY(btparamrec.foundp);
		err = ROMlib_dirbusy(CL(drp->dirDirID), vcbp);
		if (err == noErr)
		  {
		    if (drp->dirVal != 0)
			err = fBsyErr;
		    else
			err = ROMlib_writevcbp(vcbp);
		  }
		if (err == noErr)
		    err = ROMlib_dirdelete(&btparamrec);
	    } else {
		frp = (filerec *) DATAPFROMKEY(btparamrec.foundp);
		if (ROMlib_alreadyopen(vcbp, CL(frp->filFlNum),
			   (SignedByte *) 0, 0, eitherbusy) != noErr)
		    err = fBsyErr;
#if 0
		else
		    err = ROMlib_dirtyleaf(frp, vcbp);
#endif
		if (err == noErr)
		    err = ROMlib_writevcbp(vcbp);
		if (err == noErr) {
		    err = freeallblocks(vcbp, frp);
		    if (err == noErr)
			err = ROMlib_filedelete(&btparamrec, regular);
		}
	    }
	}
	break;
    case fnfErr:
	if (op == delete)
	    err = fnfErr;
	else {
	    err = ROMlib_writevcbp(vcbp);
	    if (err == noErr) {
		if (kind == directory)
		    err = ROMlib_btcreateemptydir(&btparamrec,
		    				  &((HFileParam *)pb)->ioDirID);
		else
		    err = ROMlib_btcreateemptyfile(&btparamrec);
	    }
	}
	break;
    }
    if (vcbp)
      err1 = ROMlib_cleancache(vcbp);
    else
      err1 = noErr;
    if (err1 == noErr && vcbp)
	err1 = ROMlib_flushvcbp(vcbp);
    if (err == noErr)
	err = err1;
    PBRETURN(pb, err);
}

PUBLIC OSErr hfsPBCreate(ParmBlkPtr pb, BOOLEAN async)
{
    return createhelper((ioParam *) pb, async, create, (LONGINT) 0, regular);
}

PUBLIC OSErr hfsPBHCreate(HParmBlkPtr pb, BOOLEAN async)
{
    return createhelper((ioParam *)pb, async, create, CL(pb->fileParam.ioDirID),
								      regular);
}

PUBLIC OSErr hfsPBDirCreate(HParmBlkPtr pb, BOOLEAN async)
{
    return createhelper((ioParam *)pb, async, create, CL(pb->fileParam.ioDirID),
								    directory);
}

PUBLIC OSErr hfsPBDelete(ParmBlkPtr pb, BOOLEAN async)
{
    return createhelper((ioParam *) pb, async, delete, (LONGINT) 0,
							    regular|directory);
}

PUBLIC OSErr hfsPBHDelete(HParmBlkPtr pb, BOOLEAN async)
{
    return createhelper((ioParam *)pb, async, delete, CL(pb->fileParam.ioDirID),
							    regular|directory);
}
