/* Copyright 1992 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_hfsWorkingdir[] =
	    "$Id: hfsWorkingdir.c 86 2005-05-25 00:47:12Z ctm $";
#endif

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "rsys/hfs.h"
#include "rsys/file.h"
#include "MemoryMgr.h"

using namespace Executor;
using namespace ByteSwap;

/*
 * TODO: use this working directory stuff in ROMlib
 */

PUBLIC OSErr Executor::ROMlib_dirbusy(LONGINT dirid, HVCB *vcbp)
{
#if defined(MAC)
    wdentry *wdp, *ewdp;
    
    for (wdp = (wdentry *) (BigEndianValue(WDCBsPtr) + sizeof(INTEGER)),
	    ewdp = (wdentry *) (BigEndianValue(WDCBsPtr) + BigEndianValue(*(INTEGER *)BigEndianValue(WDCBsPtr)));
							    wdp != ewdp; wdp++)
	;
    return wdp == ewdp ? noErr : fBsyErr;
#else
    return noErr;
#endif
}

PUBLIC OSErr Executor::ROMlib_mkwd(WDPBPtr pb, HVCB *vcbp, LONGINT dirid, LONGINT procid)
{
    wdentry *wdp, *ewdp, *firstfreep;
    OSErr retval;
    INTEGER n_wd_bytes, new_n_wd_bytes;
    Ptr newptr;
    THz saveZone;

    firstfreep = 0;
    for (wdp = (wdentry *) (MR(WDCBsPtr) + sizeof(INTEGER)),
	       ewdp = (wdentry *) (MR(WDCBsPtr) +BigEndianValue(*(INTEGER *)MR(WDCBsPtr)));
							  wdp != ewdp; wdp++) {
	if (!firstfreep && !wdp->vcbp)
	    firstfreep = wdp;
	if (MR(wdp->vcbp) == vcbp && BigEndianValue(wdp->dirid) == dirid &&
						   BigEndianValue(wdp->procid) == procid) {
	    pb->ioVRefNum = BigEndianValue(WDPTOWDNUM(wdp));
/*-->*/	    return noErr;
	}
    }
    if (!firstfreep) {
	n_wd_bytes = BigEndianValue(*(INTEGER *) MR(WDCBsPtr));
	new_n_wd_bytes = (n_wd_bytes - sizeof(INTEGER)) * 2 + sizeof(INTEGER);
	saveZone = TheZone;
	TheZone = SysZone;
	newptr = NewPtr(new_n_wd_bytes);
	SysZone = TheZone;
	if (!newptr)
	    retval = tmwdoErr;
	else {
	    BlockMove( MR(WDCBsPtr), newptr, n_wd_bytes);
	    DisposPtr( MR(WDCBsPtr) );
	    WDCBsPtr = RM(newptr);
	    *(INTEGER *) newptr = BigEndianValue(new_n_wd_bytes);
	    firstfreep = (wdentry *) (newptr + n_wd_bytes);
	    retval = noErr;
	}
    } else
	retval = noErr;
    if (retval == noErr) {
	firstfreep->vcbp = RM(vcbp);
	firstfreep->dirid = BigEndianValue(dirid);
	firstfreep->procid = BigEndianValue(procid);
	pb->ioVRefNum = BigEndianValue(WDPTOWDNUM(firstfreep));
	retval = noErr;
    }
    return retval;
}

PUBLIC OSErr Executor::hfsPBOpenWD(WDPBPtr pb, BOOLEAN async)
{
    LONGINT dirid;
    OSErr retval;
    filekind kind;
    btparam btparamrec;
    HVCB *vcbp;
    StringPtr namep;
    
    kind = (filekind)(regular|directory);
    retval = ROMlib_findvcbandfile((ioParam *)pb, Cx(pb->ioWDDirID),
						    &btparamrec, &kind, FALSE);
    if (retval != noErr)
	PBRETURN(pb, retval);
    vcbp = btparamrec.vcbp;
    retval = ROMlib_cleancache(vcbp);
    if (retval != noErr)
	PBRETURN(pb, retval);
    namep = MR (pb->ioNamePtr);
    if (kind == directory && namep && namep[0])
	dirid =
	      BigEndianValue(((directoryrec *) DATAPFROMKEY(btparamrec.foundp))->dirDirID);
    else
	dirid = BigEndianValue(pb->ioWDDirID);
    retval = ROMlib_mkwd(pb, vcbp, dirid, BigEndianValue(pb->ioWDProcID));

    PBRETURN(pb, retval);
}

PUBLIC OSErr Executor::hfsPBCloseWD(WDPBPtr pb, BOOLEAN async)
{
    wdentry *wdp;
    OSErr retval;
    
    retval = noErr;
    if (ISWDNUM(Cx(pb->ioVRefNum))) {
	wdp = WDNUMTOWDP(Cx(pb->ioVRefNum));
	if (wdp)
	    wdp->vcbp = 0;
	else
	    retval = nsvErr;
    }
    PBRETURN(pb, retval);
}

PUBLIC OSErr Executor::hfsPBGetWDInfo(WDPBPtr pb, BOOLEAN async)
{
    OSErr retval;
    wdentry *wdp, *ewdp;
    INTEGER i;
    BOOLEAN foundelsewhere;
    HVCB *vcbp;
    
    foundelsewhere = FALSE;
    retval = noErr;
    wdp = 0;
    if (Cx(pb->ioWDIndex) > 0) {
	i = Cx(pb->ioWDIndex);
	wdp = (wdentry *) (MR(WDCBsPtr) + sizeof(INTEGER));
	ewdp = (wdentry *) (MR(WDCBsPtr) + BigEndianValue(*(INTEGER *)MR(WDCBsPtr)));
	if (Cx(pb->ioVRefNum) < 0) {
	    for (;wdp != ewdp; wdp++)
		if (wdp->vcbp && MR(wdp->vcbp)->vcbVRefNum == pb->ioVRefNum && --i <= 0)
		    break;
	} else if (pb->ioVRefNum == 0) {
	    for (;wdp != ewdp && i > 1; ++wdp)
	      if (wdp->vcbp)
		--i;
	} else /* if (Cx(pb->ioVRefNum) > 0 */ {
	    for (;wdp != ewdp; wdp++)
		if (MR(wdp->vcbp)->vcbDrvNum == pb->ioVRefNum && --i <= 0)
		    break;
	}
	if (wdp == ewdp || !wdp->vcbp)
	    wdp = 0;
    } else if (ISWDNUM(Cx(pb->ioVRefNum)))
	wdp = WDNUMTOWDP(Cx(pb->ioVRefNum));
    else {
	vcbp = ROMlib_findvcb(Cx(pb->ioVRefNum), (StringPtr) 0, (LONGINT *) 0,
									 TRUE);
	if (vcbp) {
	    if (pb->ioNamePtr)
		str255assign(MR(pb->ioNamePtr), (StringPtr) vcbp->vcbVN);
	    pb->ioWDProcID  = 0;
	    pb->ioVRefNum   = pb->ioWDVRefNum = vcbp->vcbVRefNum;
	    pb->ioWDDirID   = BigEndianValue((vcbp == MR(DefVCBPtr)) ? DefDirID : 2);
	    foundelsewhere = TRUE;
	}
    }
	
    if (!foundelsewhere) {
	if (wdp) {
	    if (pb->ioNamePtr)
		str255assign(MR(pb->ioNamePtr),
					     (StringPtr) MR(wdp->vcbp)->vcbVN);
	    if (Cx(pb->ioWDIndex) > 0)
		pb->ioVRefNum = MR(wdp->vcbp)->vcbVRefNum;
	    pb->ioWDProcID = wdp->procid;
	    pb->ioWDVRefNum = MR(wdp->vcbp)->vcbVRefNum;
	    pb->ioWDDirID   = wdp->dirid;
	} else
	    retval = nsvErr;
    }
	
    PBRETURN(pb, retval);
}

PUBLIC OSErr
Executor::GetWDInfo (INTEGER wd, INTEGER *vrefp, LONGINT *dirp, LONGINT *procp)
{
  OSErr retval;

  WDPBRec wdp;
  memset (&wdp, 0, sizeof wdp);
  wdp.ioVRefNum = BigEndianValue (wd);
  retval = PBGetWDInfo (&wdp, FALSE);
  if (retval == noErr)
    {
      *vrefp = wdp.ioVRefNum;
      *dirp = wdp.ioWDDirID;
      *procp = wdp.ioWDProcID;
    }
  return retval;
}

PUBLIC void Executor::ROMlib_adjustdirid(LONGINT *diridp, HVCB *vcbp, INTEGER vrefnum)
{
    wdentry *wdp;
    
    if (*(ULONGINT *) diridp <= 1 && ISWDNUM(vrefnum)) {
	wdp = WDNUMTOWDP(vrefnum);
	if (MR(wdp->vcbp) == vcbp)
	    *diridp = BigEndianValue(wdp->dirid);
    } else if (*diridp == 0 && !vrefnum /* vcbp == BigEndianValue(DefVCBPtr) */)
	*diridp = BigEndianValue(DefDirID);
    if (*diridp == 0)
	*diridp = 2;
}
