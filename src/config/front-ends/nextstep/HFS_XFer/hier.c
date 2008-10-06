#if defined(OUTDATEDCODE)
#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "myhfs.h"
#include <string.h>

typedef enum { Get, Set } catop;

PRIVATE OSErr cathelper(CInfoPBPtr pb, BOOLEAN async, catop op)
{
    filekind kind;
    filerec *frp;
    void *voidp;
    directoryrec *drp;
    HFileInfo *pbf;
    DirInfo *pbd;
    OSErr err, err1;
    btparam btparamrec;
    HVCB *vcbp;
    catkey *catkeyp;
    BOOLEAN ignorename;
    
    if (pb->hFileInfo.ioFDirIndex > 0 && op == Get) {
	err = btpbindex((ioParam *) pb, pb->hFileInfo.ioDirID, &vcbp, &frp,
								      &catkeyp, FALSE);
	if (err != noErr)
	    goto done;
	voidp = DATAPFROMKEY(catkeyp);
	switch (((filerec *)voidp)->cdrType) {
	case FILETYPE:
	    kind = regular;
	    break;
	case DIRTYPE:
	    kind = directory;
	    break;
	default:
	    DebugStr((StringPtr) "\punknown cdrtype in cathelper");
	    err = fsDSIntErr;
	    goto done;
	    break;
	}
    } else {
    	if (pb->hFileInfo.ioFDirIndex < 0) {
    	    kind = directory;
    	    ignorename = TRUE;
    	} else {
	    kind = regular | directory;
	    ignorename = FALSE;
	}
	err = findvcbandfile((ioParam *) pb, pb->hFileInfo.ioDirID,
						&btparamrec, &kind, ignorename);
	if (err != noErr)
	    goto done;
	vcbp = btparamrec.vcbp;
	voidp = DATAPFROMKEY(btparamrec.foundp);
	catkeyp = (catkey *) btparamrec.foundp;
    }
    
    switch (kind) {
    case regular:
	frp = voidp;
	pbf = (HFileInfo *) pb;
	if (op == Get) {
	    if (pbf->ioNamePtr)
		str255assign(pbf->ioNamePtr, catkeyp->ckrCName);
	    pbf->ioFRefNum = flnumtorefnum(frp->filFlNum);
    
	    pbf->ioFlAttrib = frp->filFlags;
	    memcpy(&pbf->ioFlFndrInfo, &frp->filUsrWds,
					    (size_t) sizeof(pbf->ioFlFndrInfo));
	    pbf->ioDirID = frp->filFlNum;
	    pbf->ioFlStBlk = frp->filStBlk;
	    pbf->ioFlLgLen = frp->filLgLen;
	    pbf->ioFlPyLen = frp->filPyLen;
	    pbf->ioFlRStBlk = frp->filRStBlk;
	    pbf->ioFlRLgLen = frp->filRLgLen;
	    pbf->ioFlRPyLen = frp->filRPyLen;
	    pbf->ioFlCrDat = frp->filCrDat;
	    pbf->ioFlMdDat = frp->filMdDat;
	    pbf->ioFlBkDat = frp->filBkDat;
	    memcpy(&pbf->ioFlXFndrInfo, frp->filFndrInfo,
					   (size_t) sizeof(pbf->ioFlXFndrInfo));
	    pbf->ioFlParID = catkeyp->ckrParID;
	    pbf->ioFlClpSiz = frp->filClpSize;
	} else {
	    frp->filFlags &= ~FILEFLAGSUSERSETTABLEMASK;
	    frp->filFlags |=  FILEFLAGSUSERSETTABLEMASK &pbf->ioFlAttrib;
	    memcpy(&frp->filUsrWds, &pbf->ioFlFndrInfo,
					       (size_t) sizeof(frp->filUsrWds));
	    frp->filCrDat = pbf->ioFlCrDat;
	    frp->filMdDat = pbf->ioFlMdDat;
	    frp->filBkDat = pbf->ioFlBkDat;
	    memcpy(&frp->filFndrInfo, &pbf->ioFlXFndrInfo,
					     (size_t) sizeof(frp->filFndrInfo));
	    frp->filClpSize = pbf->ioFlAttrib;
	    dirtyleaf(frp, vcbp);
	}
	break;
    case directory:
	drp = voidp;
	pbd = (DirInfo *) pb;
	if (op == Get) {
	    if (pbd->ioNamePtr)
		str255assign(pbd->ioNamePtr, catkeyp->ckrCName);
		
    /* NOTE: IMIV-155 claims that pbd->ioFRefNum is updated, but the Mac+
	 doesn't do the updating */
	 
	    pbd->ioFlAttrib = drp->dirFlags >> 8;
	    pbd->ioFlAttrib |= (1 << 4);	/* TODO: better name */
	    memcpy(&pbd->ioDrUsrWds, drp->dirUsrInfo,
					      (size_t) sizeof(pbd->ioDrUsrWds));
	    pbd->ioDrDirID = drp->dirDirID;
	    pbd->ioDrNmFls = drp->dirVal;
	    pbd->ioDrCrDat = drp->dirCrDat;
	    pbd->ioDrMdDat = drp->dirMdDat;
	    pbd->ioDrBkDat = drp->dirBkDat;
	    memcpy(&pbd->ioDrFndrInfo, drp->dirFndrInfo,
					    (size_t) sizeof(pbd->ioDrFndrInfo));
	    pbd->ioDrParID = catkeyp->ckrParID;
	} else {
#if 0
/* I don't think you can change any directory flags */
	    drp->dirFlags = pbd->ioFlAttrib;
#endif
	    memcpy(&drp->dirUsrInfo, &pbd->ioDrUsrWds,
					      (size_t) sizeof(drp->dirUsrInfo));
	    drp->dirCrDat = pbd->ioDrCrDat;
	    drp->dirMdDat = pbd->ioDrMdDat;
	    drp->dirBkDat = pbd->ioDrBkDat;
	    memcpy(&drp->dirFndrInfo, &pbd->ioDrCrDat,
					     (size_t) sizeof(drp->dirFndrInfo));
	    dirtyleaf(drp, vcbp);
	}
	break;
    default:
        DebugStr((StringPtr) "\punknown kind in cathelper");
	err = fsDSIntErr;
	goto done;
	break;
    }
done:
    err1 = vcbp ? cleancache(vcbp) : noErr;
    if (err == noErr)
        err = err1;
    PBRETURN((ioParam *) pb, err);
}


PUBLIC OSErr myPBGetCatInfo(CInfoPBPtr pb, BOOLEAN async)
{
    return cathelper(pb, async, Get);
}

PUBLIC OSErr myPBSetCatInfo(CInfoPBPtr pb, BOOLEAN async)
{
    return cathelper(pb, async, Set);
}

PRIVATE OSErr parentchild(HVCB *vcbp, catkey *parentcatp,
	   directoryrec *parentdirp, catkey *childcatp, directoryrec *childdirp)
{
    OSErr err;
    unsigned long parid, newid;
    catkey key;
    INTEGER ctref;
    btparam btparamrec;
    
    err = noErr;
    parid = (unsigned long) parentdirp->dirDirID;
    if (parid == childdirp->dirDirID)
	err = badMovErr;    /* can't move into oneself */
    else if (parentdirp->dirVal != 0) { /* no need to check if no children */
	if (parid <= 2)     /* automatic disqualification; can't move */
	    err = badMovErr;            /* root directory */
	else {
	    newid = (unsigned long) childcatp->ckrParID;
	    makecatparam(&btparamrec, vcbp, (LONGINT) 0, 0, (Ptr) 0);
	    ctref = vcbp->vcbCTRef;
	    while (err == noErr && newid > 2 && newid != parid) {
		key.ckrParID = newid;
		err = keyfind(&btparamrec);
		if (err == noErr && !btparamrec.success) {
		    err = fsDSIntErr;
		    DebugStr((StringPtr) "\pno success in parentchild");
		}
		newid =
		       ((threadrec *)DATAPFROMKEY(btparamrec.foundp))->thdParID;
	    }
	    if (err == noErr)
		err = newid <= 2 ? noErr : badMovErr;
	}
    }
    return err;
}

PUBLIC OSErr myPBCatMove(CMovePBPtr pb, BOOLEAN async)
{
    OSErr err, err1;
    filekind srccurkind, dstcurkind;
    ioParam iop;
    btparam srcbtparam, dstdirbtparam, dstbtparam;
    directoryrec *dstdirdrp;
    directoryrec srcdrec;
    filerec srcfrec;
    BOOLEAN ignorename;
    
    srccurkind = regular | directory;
    err = findvcbandfile((ioParam *) pb, pb->ioDirID, &srcbtparam,
							    &srccurkind, FALSE);
    if (err == noErr) {
	err = writevcbp(srcbtparam.vcbp);
	iop = *(ioParam *)pb;
	iop.ioNamePtr = pb->ioNewName;
	dstcurkind = directory;
	ignorename = iop.ioNamePtr == 0;
	err = findvcbandfile(&iop, pb->ioNewDirID, &dstdirbtparam, &dstcurkind,
								    ignorename);
	if (err == noErr) {
	    if (srcbtparam.vcbp != dstdirbtparam.vcbp)
		err = badMovErr;
	    else {
		dstdirdrp = (directoryrec *) DATAPFROMKEY(dstdirbtparam.foundp);
		dstbtparam = dstdirbtparam;
    		makecatkey((catkey *) &dstbtparam.tofind, dstdirdrp->dirDirID,
    					    srcbtparam.foundp->catk.ckrCName[0],
    				      (Ptr) srcbtparam.foundp->catk.ckrCName+1);
		dstbtparam.leafindex = -1;
		if (srccurkind == directory) {
		    srcdrec = *(directoryrec *) DATAPFROMKEY(srcbtparam.foundp);
		    err = parentchild(srcbtparam.vcbp,
				    (catkey *) srcbtparam.foundp, &srcdrec,
				    (catkey *) dstdirbtparam.foundp, dstdirdrp);
		    if (err == noErr)
			err = dircreate(&dstbtparam, &srcdrec);
		    if (err == noErr)
			err = dirdelete(&srcbtparam);
		} else {
		    srcfrec = *(filerec *) DATAPFROMKEY(srcbtparam.foundp);
		    err = filecreate(&dstbtparam, &srcfrec, regular);
		    if (err == noErr) {
		        srcbtparam.leafindex = -1;
			err = filedelete(&srcbtparam, regular);
		    }
		}
	    }
	}
    }
    err1 = cleancache(srcbtparam.vcbp);
    if (err == noErr)
	err = err1;
    PBRETURN(pb, err);
}
#endif
