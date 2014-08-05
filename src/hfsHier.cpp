/* Copyright 1992-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_hfsHier[] =
	    "$Id: hfsHier.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "MemoryMgr.h"
#include "rsys/hfs.h"
#include "rsys/file.h"

using namespace Executor;
using namespace ByteSwap;

typedef enum { catGet, catSet } catop;

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
    
    vcbp = 0;
    if (BigEndianValue(pb->hFileInfo.ioFDirIndex) > 0 && op == catGet) {
	err = ROMlib_btpbindex((ioParam *) pb, BigEndianValue(pb->hFileInfo.ioDirID), &vcbp, &frp,
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
	    warning_unexpected ("unknown cdrtype in cathelper");
	    err = fsDSIntErr;
	    goto done;
	    break;
	}
    } else {
    	if (BigEndianValue(pb->hFileInfo.ioFDirIndex) < 0) {
    	    kind = directory;
    	    ignorename = TRUE;
    	} else {
	    kind = filekind(regular | directory);
	    ignorename = FALSE;
	}
	err = ROMlib_findvcbandfile((ioParam *) pb, BigEndianValue(pb->hFileInfo.ioDirID),
						&btparamrec, &kind, ignorename);
	if (err != noErr)
	    goto done;
	vcbp = btparamrec.vcbp;
	if (op == catSet)
	  {
	    err = ROMlib_writevcbp (vcbp);
	    if (err != noErr)
	      goto done;
	  }
	voidp = DATAPFROMKEY(btparamrec.foundp);
	catkeyp = (catkey *) btparamrec.foundp;
    }
    
    switch (kind) {
    case regular:
	frp = (filerec*)voidp;
	pbf = (HFileInfo *) pb;
	if (op == catGet) {
	    if (UPDATE_IONAMEPTR_P(*pbf))
		str255assign(MR(pbf->ioNamePtr), catkeyp->ckrCName);
	    pbf->ioACUser = 0;
	    pbf->ioFlAttrib = CB (open_attrib_bits (BigEndianValue (frp->filFlNum), vcbp,
						   &pbf->ioFRefNum));
	    pbf->ioFlAttrib |= frp->filFlags & CB (INHERITED_FLAG_BITS);
	    memmove(&pbf->ioFlFndrInfo, &frp->filUsrWds,
		    (LONGINT) sizeof(pbf->ioFlFndrInfo));
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
	    memmove(&pbf->ioFlXFndrInfo, frp->filFndrInfo,
		    (LONGINT) sizeof(pbf->ioFlXFndrInfo));
	    pbf->ioFlParID = catkeyp->ckrParID;
	    pbf->ioFlClpSiz = frp->filClpSize;
	} else {
	    frp->filFlags &= ~FILEFLAGSUSERSETTABLEMASK;
	    frp->filFlags |=  FILEFLAGSUSERSETTABLEMASK &pbf->ioFlAttrib;
	    memmove(&frp->filUsrWds, &pbf->ioFlFndrInfo,
		    (LONGINT) sizeof(frp->filUsrWds));
	    frp->filCrDat = pbf->ioFlCrDat;
	    frp->filMdDat = pbf->ioFlMdDat;
	    frp->filBkDat = pbf->ioFlBkDat;
	    memmove(&frp->filFndrInfo, &pbf->ioFlXFndrInfo,
		    (LONGINT) sizeof(frp->filFndrInfo));
	    frp->filClpSize = pbf->ioFlClpSiz;
	    ROMlib_dirtyleaf(frp, vcbp);
	    err = ROMlib_flushvcbp (vcbp);
	}
	break;
    case directory:
	drp = (directoryrec*)voidp;
	pbd = (DirInfo *) pb;
	if (op == catGet) {
	    if (UPDATE_IONAMEPTR_P(*pbd))
		str255assign(MR(pbd->ioNamePtr), catkeyp->ckrCName);

	    pbd->ioACUser = 0;
		
    /* NOTE: IMIV-155 claims that pbd->ioFRefNum is updated, but the Mac+
	 doesn't do the updating */
	 
	    pbd->ioFlAttrib = drp->dirFlags >> 8;
	    pbd->ioFlAttrib |= ATTRIB_ISADIR;
	    memmove(&pbd->ioDrUsrWds, drp->dirUsrInfo,
		    (LONGINT) sizeof(pbd->ioDrUsrWds));
	    pbd->ioDrDirID = drp->dirDirID;
	    pbd->ioDrNmFls = drp->dirVal;
	    pbd->ioDrCrDat = drp->dirCrDat;
	    pbd->ioDrMdDat = drp->dirMdDat;
	    pbd->ioDrBkDat = drp->dirBkDat;
	    memmove(&pbd->ioDrFndrInfo, drp->dirFndrInfo,
		    (LONGINT) sizeof(pbd->ioDrFndrInfo));
	    pbd->ioDrParID = catkeyp->ckrParID;
	} else {
#if 0
/* I don't think you can change any directory flags */
	    drp->dirFlags = pbd->ioFlAttrib;
#endif
	    memmove(&drp->dirUsrInfo, &pbd->ioDrUsrWds,
		    (LONGINT) sizeof(drp->dirUsrInfo));
	    drp->dirCrDat = pbd->ioDrCrDat;
	    drp->dirMdDat = pbd->ioDrMdDat;
	    drp->dirBkDat = pbd->ioDrBkDat;
	    memmove(&drp->dirFndrInfo, &pbd->ioDrCrDat,
		    (LONGINT) sizeof(drp->dirFndrInfo));
	    ROMlib_dirtyleaf(drp, vcbp);
	    err = ROMlib_flushvcbp (vcbp);
	}
	break;
    default:
        warning_unexpected ("unknown kind in cathelper");
	err = fsDSIntErr;
	goto done;
	break;
    }
done:
    err1 = vcbp ? ROMlib_cleancache(vcbp) : noErr;
    if (err == noErr)
        err = err1;
    fs_err_hook (err);
    PBRETURN((ioParam *) pb, err);
}


PUBLIC OSErr Executor::hfsPBGetCatInfo(CInfoPBPtr pb, BOOLEAN async)
{
  OSErr err;
  err = cathelper(pb, async, catGet);
  fs_err_hook (err);
  return err;
}

PUBLIC OSErr Executor::hfsPBSetCatInfo(CInfoPBPtr pb, BOOLEAN async)
{
  OSErr err;
  err = cathelper(pb, async, catSet);
  fs_err_hook (err);
  return err;
}

PRIVATE OSErr parentchild(HVCB *vcbp, catkey *parentcatp,
	   directoryrec *parentdirp, catkey *childcatp, directoryrec *childdirp)
{
    OSErr err;
    LONGINT parid, newid;
    INTEGER ctref;
    btparam btparamrec;
    
    err = noErr;
    parid = BigEndianValue(parentdirp->dirDirID);
    if (parid == BigEndianValue(childdirp->dirDirID))
	err = badMovErr;    /* can't move into oneself */
    else if (parentdirp->dirVal != 0) { /* no need to check if no children */
	if (parid <= 2)     /* automatic disqualification; can't move */
	    err = badMovErr;            /* root directory */
	else {
	    newid = BigEndianValue(childcatp->ckrParID);
	    err = ROMlib_makecatparam(&btparamrec, vcbp, (LONGINT) 0, 0, (Ptr) 0);
	    ctref = BigEndianValue(vcbp->vcbCTRef);
	    while (err == noErr && newid > 2 && newid != parid) {
		btparamrec.tofind.catk.ckrParID = BigEndianValue(newid);
		err = ROMlib_keyfind(&btparamrec);
		if (err == noErr && !btparamrec.success) {
		    err = fsDSIntErr;
		    warning_unexpected ("no success in parentchild");
		}
		newid = BigEndianValue(
		       ((threadrec *)DATAPFROMKEY(btparamrec.foundp))->thdParID);
	    }
	    if (err == noErr)
		err = newid <= 2 ? noErr : badMovErr;
	}
    }
    fs_err_hook (err);
    return err;
}

PUBLIC OSErr Executor::hfsPBCatMove(CMovePBPtr pb, BOOLEAN async)
{
    OSErr err, err1;
    filekind srccurkind, dstcurkind;
    ioParam iop;
    btparam srcbtparam, dstdirbtparam, dstbtparam;
    directoryrec *dstdirdrp;
    directoryrec srcdrec;
    filerec srcfrec;
    BOOLEAN ignorename;
    
    srccurkind = (filekind)(regular | directory);
    err = ROMlib_findvcbandfile((ioParam *) pb, BigEndianValue(pb->ioDirID), &srcbtparam,
							    &srccurkind, FALSE);
    if (err == noErr) {
	err = ROMlib_writevcbp(srcbtparam.vcbp);
	iop = *(ioParam *)pb;
	iop.ioNamePtr = pb->ioNewName;
	dstcurkind = directory;
	ignorename = iop.ioNamePtr == 0;
	err = ROMlib_findvcbandfile(&iop, BigEndianValue(pb->ioNewDirID), &dstdirbtparam, &dstcurkind,
								    ignorename);
	if (err == noErr) {
	    if (srcbtparam.vcbp != dstdirbtparam.vcbp)
		err = badMovErr;
	    else {
		dstdirdrp = (directoryrec *) DATAPFROMKEY(dstdirbtparam.foundp);
		dstbtparam = dstdirbtparam;
    		err = ROMlib_makecatkey((catkey *) &dstbtparam.tofind, BigEndianValue(dstdirdrp->dirDirID),
    					    srcbtparam.foundp->catk.ckrCName[0],
    				      (Ptr) srcbtparam.foundp->catk.ckrCName+1);
		dstbtparam.leafindex = -1;
		err = ROMlib_keyfind (&dstbtparam);
		if (err != noErr)
		  ;
		else if (dstbtparam.success)
		  err = dupFNErr;
		else
		  {
		    dstbtparam.leafindex = -1;
		    if (srccurkind == directory) {
		      srcdrec = *(directoryrec *) DATAPFROMKEY(srcbtparam.foundp);
		      if (err == noErr)
			err = parentchild(srcbtparam.vcbp,
					  (catkey *) srcbtparam.foundp, &srcdrec,
					  (catkey *) dstdirbtparam.foundp, dstdirdrp);
/*
 * NOTE: Even though it is less safe to delete and then create (an
 *	 untimely power hit loses the file), we need to do it in this
 *	 order or greatly rework things.  The problem is that if we
 *	 do a create followed by a delete, we wind up putting in another
 *	 thread record WITH A DUPLICATE KEY, which means that the dirdelete
 *	 may will the wrong thread record when moving directories up.
 */
		      if (err == noErr)
			err = ROMlib_dirdelete(&srcbtparam);
		      if (err == noErr)
			err = ROMlib_dircreate(&dstbtparam, &srcdrec);
		    } else {
		      srcfrec = *(filerec *) DATAPFROMKEY(srcbtparam.foundp);
		      if (err == noErr)
			err = ROMlib_filecreate(&dstbtparam, &srcfrec, regular);
		      if (err == noErr) {
		        srcbtparam.leafindex = -1;
			err = ROMlib_filedelete(&srcbtparam, regular);
		      }
		    }
		  }
	    }
	}
    }
    err1 = ROMlib_cleancache(srcbtparam.vcbp);
    if (err1 == noErr)
	err1 = ROMlib_flushvcbp(srcbtparam.vcbp);
    if (err == noErr)
	err = err1;
    fs_err_hook (err);
    PBRETURN(pb, err);
}
