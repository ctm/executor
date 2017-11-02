#if defined(OUTDATEDCODE)

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "myhfs.h"
#include <string.h>

typedef enum { create, delete } createop;

PRIVATE OSErr freeallblocks(HVCB *vcbp, filerec *frp)
{
    OSErr retval;
    filecontrolblock *fcbp;
    ParamBlockRec pbr;
    
    fcbp = getfreefcbp();
    if (!fcbp)
	retval = tmfoErr;
    else {
	fcbp->fcbVPtr = vcbp;
	fcbp->fcbFlNum = frp->filFlNum;
	fcbp->fcbPLen = frp->filPyLen;
	memcpy((char *) fcbp->fcbExtRec, (char *) frp->filExtRec,
					       (size_t) sizeof(frp->filExtRec));
	fcbp->fcbMdRByt = WRITEBIT;
	pbr.ioParam.ioMisc = 0;
	pbr.ioParam.ioRefNum = (char *) fcbp - (char *) FCBSPtr;
	retval = AllocHelper((ioParam *) &pbr, false, seteof, false);
	if (retval == noErr) {
	    fcbp->fcbPLen = frp->filRPyLen;
	    memcpy((char *) fcbp->fcbExtRec, (char *) frp->filRExtRec,
					      (size_t) sizeof(frp->filRExtRec));
	    fcbp->fcbMdRByt = WRITEBIT|RESOURCEBIT;
	    retval = AllocHelper((ioParam *) &pbr, false, seteof, false);
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
    extern ulong blockchecksum();
    
    curkind = regular | directory;
    err = findvcbandfile(pb, dirid, &btparamrec, &curkind, false);
    vcbp = btparamrec.vcbp;
    switch (err) {
    case noErr:
	if (op == create)
	    err = dupFNErr;
	else {
	    if (curkind == directory) {
		drp = (directoryrec *) DATAPFROMKEY(btparamrec.foundp);
		err = dirbusy(drp->dirDirID, vcbp);
		if (err == noErr)
		    if (drp->dirVal != 0)
			err = fBsyErr;
		    else
			err = writevcbp(vcbp);
		if (err == noErr)
		    err = dirdelete(&btparamrec);
	    } else {
		frp = (filerec *) DATAPFROMKEY(btparamrec.foundp);
		if (alreadyopen(vcbp, frp->filFlNum,
				       (SignedByte *) 0, (short *) 0) != noErr)
		    err = fBsyErr;
#if 0
		else
		    err = dirtyleaf(frp, vcbp);
#endif
		if (err == noErr)
		    err = writevcbp(vcbp);
		if (err == noErr) {
		    err = freeallblocks(vcbp, frp);
		    if (err == noErr)
			err = filedelete(&btparamrec, regular);
		}
	    }
	}
	break;
    case fnfErr:
	if (op == delete)
	    err = fnfErr;
	else {
	    err = writevcbp(vcbp);
	    if (err == noErr) {
		if (kind == directory)
		    err = btcreateemptydir(&btparamrec,
		    				  &((HFileParam *)pb)->ioDirID);
		else
		    err = btcreateemptyfile(&btparamrec);
	    }
	}
	break;
    }
    err1 = cleancache(vcbp);
    if (err == noErr)
	err = err1;
    PBRETURN(pb, err);
}

PUBLIC OSErr myPBCreate(ioParam *pb, BOOLEAN async)
{
    return createhelper(pb, async, create, (LONGINT) 1, regular);
}

PUBLIC OSErr myPBHCreate(HFileParam *pb, BOOLEAN async)
{
    return createhelper((ioParam *)pb, async, create, pb->ioDirID, regular);
}

PUBLIC OSErr myPBDirCreate(HFileParam *pb, BOOLEAN async)
{
    return createhelper((ioParam *)pb, async, create, pb->ioDirID, directory);
}

PUBLIC OSErr myPBDelete(ioParam *pb, BOOLEAN async)
{
    return createhelper(pb, async, delete, (LONGINT) 1, regular|directory);
}

PUBLIC OSErr myPBHDelete(HFileParam *pb, BOOLEAN async)
{
    return createhelper((ioParam *)pb, async, delete, pb->ioDirID,
							     regular|directory);
}

#endif
