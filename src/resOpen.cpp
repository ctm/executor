/* Copyright 1986, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_resOpen[] =
	    "$Id: resOpen.c 88 2005-05-25 03:59:37Z ctm $";
#endif

/* Forward declarations in ResourceMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "MemoryMgr.h"
#include "FileMgr.h"

#include "rsys/resource.h"
#include "rsys/glue.h"
#include "rsys/mman.h"
#include "rsys/file.h"
#include "rsys/pstuff.h"
#include "rsys/osevent.h"
#include "rsys/prefs.h"

using namespace Executor;

PUBLIC void
Executor::HCreateResFile_helper (INTEGER vrefnum, LONGINT parid, Str255 name,
		       OSType creator, OSType type, ScriptCode script)
{
    INTEGER f;
    LONGINT leof, lc;
    empty_resource_template_t buf;

    ROMlib_setreserr(HCreate(vrefnum, parid, name, creator, type)); /* ????
								       might
								    be wrong */
    if (ResErr != noErr && Cx(ResErr) != dupFNErr)
        return;
    ROMlib_setreserr(HOpenRF(vrefnum, parid, name, fsRdWrPerm, &f));
    if (ResErr != noErr)
        return;
    ROMlib_setreserr(GetEOF(f, &leof));
    if (ResErr != noErr) {
        FSClose(f);
        return;
    }
    if (leof) {
        ROMlib_setreserr(dupFNErr);
        FSClose(f);
        return;
    }
    buf.bhead.rdatoff = buf.bhead.rmapoff = CL(sizeof(reshead) +
							      sizeof(rsrvrec));
    buf.bhead.datlen = CLC(0);   /* No data */
    buf.bhead.maplen = CLC(sizeof(resmap) + sizeof(INTEGER));
    buf.bmap.namoff  = CWC(sizeof(resmap) + sizeof(INTEGER));
    buf.bmap.resfatr = CWC(0);   /* No special attributes */
    buf.bmap.typoff  = CWC(sizeof(resmap));
    buf.negone = -1;        /* zero types (0 - 1) */
    lc = sizeof(buf);
    ROMlib_setreserr(FSWriteAll(f, &lc, (Ptr)&buf));
    if (ResErr != noErr)
        return;
    ROMlib_setreserr(FSClose(f));
}

P1(PUBLIC pascal trap, void, CreateResFile, StringPtr, fn)
{
  HCreateResFile_helper (0, 0, fn, TICK("????"), TICK("????"), 0);
}

P3(PUBLIC pascal trap, void, HCreateResFile, INTEGER, vrefnum, LONGINT, parid,
   Str255, name)
{
  HCreateResFile_helper (vrefnum, parid, name, TICK("????"), TICK("????"), 0);
}

#if defined(BINCOMPAT)

Handle Executor::ROMlib_mgetres(resmaphand map, resref *rr)
{
    Handle retval;

    if (ostraptable[0xFC] == osstuff[0xFC].orig)
	retval = ROMlib_mgetres2(map, rr);
    else {
	LONGINT saved0, saved1, saved2, savea0, savea1, savea2, savea3, savea4;

	saved0 = EM_D0;
	saved1 = EM_D1;
	saved2 = EM_D2;
	savea0 = EM_A0;
	savea1 = EM_A1;
	savea2 = EM_A2;
	savea3 = EM_A3;
	savea4 = EM_A4;
	EM_A4 = (LONGINT) (long) US_TO_SYN68K(map);
	EM_A3 = (LONGINT) (long) US_TO_SYN68K(rr);
	EM_A2 = (LONGINT) (long) US_TO_SYN68K(rr);
#define TEMPORARYHACKUNTILWESWAPTABLES
#if !defined(TEMPORARYHACKUNTILWESWAPTABLES)	
	EM_A0 = (LONGINT) CL(ostraptable[0xFC]);
#else /* defined(TEMPORARYHACKUNTILWESWAPTABLES) */
	EM_A0 = (LONGINT) (long) ostraptable[0xFC];
#endif
	CALL_EMULATOR(EM_A0);
	retval = (Handle) (long) SYN68K_TO_US(EM_A0);
	EM_D0 = saved0;
	EM_D1 = saved1;
	EM_D2 = saved2;
	EM_A0 = savea0;
	EM_A1 = savea1;
	EM_A2 = savea2;
	EM_A3 = savea3;
	EM_A4 = savea4;
    }
    return retval;
}

#else

void Executor::ROMlib_mgetres2( resmaphand unused1, resref *unused2 )
{
    gui_abort();
}

#define ROMlib_mgetres2	ROMlib_mgetres

#endif

namespace Executor {
  PUBLIC pascal trap void C_dcmp_template(Ptr,Ptr,Ptr, Size);
}

P4 (PUBLIC pascal trap, void, dcmp_template, Ptr, source, Ptr, dest, 
    Ptr, working, Size, len)
{
}

/* TODO: decompress_setup also has to pass back the decompressed size
   we need to adjust down the size of "dlen" down below where we read
   the compressed info. */

PRIVATE boolean_t
decompress_setup (INTEGER rn, int32 *dlenp, int32 *final_sizep, int32 *offsetp,
		  Handle *dcmp_handlep, Ptr *workspacep)
{
  boolean_t retval;
  OSErr err;
  LONGINT len;
  dcomp_info_t info;
  LONGINT master_save_pos;

  *final_sizep = *dlenp;
  *offsetp = 0;
  *dcmp_handlep = NULL;
  *workspacep = NULL;

  GetFPos (rn, &master_save_pos);
  len = sizeof info;
  err = FSReadAll (rn, &len, (Ptr) &info);

  /*
   * If we can't read the entire header in or if we don't get the correct tag
   * then we'll return false, but clear ResErr.  This is a sign that the
   * resource is to be treated as a non-compressed resource.
   */

  if (err != noErr || info.compressedResourceTag != CLC (COMPRESSED_TAG))
    {
      SetFPos (rn, fsFromStart, master_save_pos);
      ROMlib_setreserr (noErr);
/*->*/return FALSE;
    }

  if (info.typeFlags != CLC (COMPRESSED_FLAGS))
    retval = FALSE;
  else
    {
      LONGINT save_pos;

      GetFPos (rn, &save_pos);
      *dcmp_handlep = GetResource (TICK("dcmp"), CW (info.dcmpID));
      SetFPos (rn, fsFromStart, save_pos);

      if (!*dcmp_handlep)
	retval = FALSE;
      else
	{
	  int32 final_size;
	  int32 working_size;

	  LoadResource (*dcmp_handlep);
	  final_size = CL (info.uncompressedSize);

	  /* 
	   * The MacTech article says that the workingBufferFractionalSize
	   * byte is a fixed point value, but it doesn't give enough
	   * information to be sure how to interpret it.  I tried  using it
	   * in a variety of ways and found I was not allocating enough bytes.
	   * This use seems to work, but I'm sufficiently nervous to merit
	   * possibly allocating more room than we needed.
	   */

	  working_size = (*dlenp + (double) *dlenp
			  * CB (info.workingBufferFractionalRatio) / (1 << 8));

#define DONT_TRUST_FRACTIONAL_RATIO
#if defined (DONT_TRUST_FRACTIONAL_RATIO)
	  working_size = MAX (final_size, working_size);
#endif

	  *workspacep = NewPtr (working_size);
	  if (!*workspacep)
	    retval = FALSE;
	  else
	    {
	      *dlenp -= sizeof info;
	      *final_sizep = final_size;
	      *offsetp = CB (info.expansionBufferSize);
	      retval = TRUE;
	    }
	}
    }

  if (!retval)
    ROMlib_setreserr (CantDecompress);
  return retval;
}

/* ROMlib_mgetres: given a resource map handle and a
	    resource reference pointer, ROMlib_mgetres returns a handle to
	    the appropriate resource */

/*
 * TODO: see whether or not lock bits and whatnot get reset on a
 *       mgetres when the handle's already there.
 */


PRIVATE Handle mgetres_helper (resmaphand map, resref *rr, int32 dlen,
			       Handle retval)
{
  int32 dcmp_offset = 0;
  Handle dcmp_handle = NULL;
  Ptr dcmp_workspace = NULL;
  int32 uncompressed_size = 0;
  Ptr xxx;
  OSErr err;
  bool compressed_p;
  bool done_p = false;

  compressed_p = (rr->ratr & resCompressed) && system_version >= 0x700;

  if (compressed_p)
    {
      if (!decompress_setup (Hx (map, resfn), &dlen, &uncompressed_size,
			     &dcmp_offset, &dcmp_handle, &dcmp_workspace))
	{
	  if (ResErr == noErr)
	    compressed_p = FALSE;
	  else
	    {
	      retval = NULL;
	      done_p = TRUE;
	    }
	}
    }

  if (!done_p)
    {
      if (!compressed_p)
	{
	  dcmp_offset = 0;
	  dcmp_handle = NULL;
	  dcmp_workspace = NULL;
	  uncompressed_size = dlen;
	}

      if (!rr->rhand)
	{
	  TheZone = ((rr->ratr & resSysHeap)
		     ? SysZone
		     : (GUEST<THz>) RM (HandleZone ((Handle) map)));
	  retval = NewHandle (uncompressed_size + dcmp_offset);
	  rr->rhand = RM (retval);
	}
      else
	{
	  retval = MR (rr->rhand);
	  ReallocHandle (retval, uncompressed_size + dcmp_offset);
	}
      err = MemError ();
      xxx = STARH (retval) + uncompressed_size + dcmp_offset - dlen;
      if ((ROMlib_setreserr (err)) ||
	  (ROMlib_setreserr (err = FSReadAll (Hx (map, resfn), &dlen, xxx))))
	{
	  if (dcmp_workspace)
	    DisposPtr (dcmp_workspace);
	  DisposHandle (MR (rr->rhand));
	  rr->rhand = NULL;
	  retval = NULL;
	}
      else
	{
	  if (dcmp_handle)
	    {
	      ProcPtr dcmp;
	      SignedByte state;

	      state = hlock_return_orig_state (dcmp_handle);
	      dcmp = (typeof (dcmp)) STARH (dcmp_handle);
	      HLock (retval);
	      CToPascalCall((void*)dcmp, CTOP_dcmp_template,
			     xxx,
			     STARH (retval), dcmp_workspace, dlen);
	      HUnlock (retval);
	      SetHandleSize (retval, uncompressed_size);
	      HSetState (dcmp_handle, state);
	      if (dcmp_workspace)
		DisposPtr (dcmp_workspace);
	    }
	}
    }

  return retval;
}

/* ROMlib_mgetres: given a resource map handle and a
	    resource reference pointer, ROMlib_mgetres returns a handle to
	    the appropriate resource */

/*
 * TODO: see whether or not lock bits and whatnot get reset on a
 *       mgetres when the handle's already there.
 */

Handle
Executor::ROMlib_mgetres2 (resmaphand map, resref *rr)
{
  Handle retval;

  retval = MR (rr->rhand);
  if (retval && *retval)
    ROMlib_setreserr (noErr);
  else
    {
      THz savezone;
      SignedByte state;
      int32 loc;

      savezone = TheZone;
      state = hlock_return_orig_state ((Handle) map);
      loc = Hx (map, rh.rdatoff) + B3TOLONG (rr->doff);
      ROMlib_setreserr (SetFPos (Hx (map, resfn), fsFromStart, loc));
      if (ResErr != noErr)
	retval = NULL;
      else
	{
	  int32 lc;
	  OSErr err;
	  int32 dlen; /* length on disk (remaining) */

	  lc = sizeof (Size);
	  err = FSReadAll (Hx (map, resfn), &lc, (Ptr) &dlen);
	  ROMlib_setreserr (err);
	  if (ResErr != noErr)
	    retval = NULL;
	  else
	    {
	      dlen = CL (dlen);
	      if (ResLoad)
		retval = mgetres_helper (map, rr, dlen, retval);
	      else if (!rr->rhand)
		{
		  TheZone = ((rr->ratr & resSysHeap)
			     ? SysZone
			     : (GUEST<THz>) RM (HandleZone ((Handle) map)));
		  retval = NewEmptyHandle ();
		  rr->rhand = RM (retval);
		}
  
	      /* we can only set the state bits if the block pointer
		 is non-nil */

	      if (retval && *retval)
		HSetState (retval,
			   (RSRCBIT
			    | ((rr->ratr & resLocked)    ? LOCKBIT  : 0)
			    | ((rr->ratr & resPurgeable) ? PURGEBIT : 0)));
  
	    }
	}
      HSetState ((Handle) map, state);
      TheZone = savezone;
    }
  return retval;
}

/* ROMlib_rntohandl:  ROMlib_rntohandl returns the resmap handle
	       of the resource file
               with the reference number rn, *pph is filled in with the
               Handle to the previous (on the linked list of files)
               file.  Note pph is undefined if rn is at the top, nor
               is it filled in if pph is nil */
    
A2(PUBLIC, resmaphand, ROMlib_rntohandl, INTEGER, rn,		/* INTERNAL */
								Handle *, pph)
{
    resmaphand map, ph;

    ph = 0;
    WALKMAPTOP(map)
        if (Hx(map, resfn) == rn)
            break;
        ph = map;
    EWALKMAP()
    
    if (pph)
        *pph = (Handle) ph;
    return(map);
}

P3(PUBLIC pascal trap, INTEGER, OpenRFPerm, StringPtr, fn, 	/* IMIV-17 */
						   INTEGER, vref, Byte, perm)
{
  INTEGER retval;
  
  retval = HOpenResFile (vref, 0, fn, perm);
  return retval;
}

P1(PUBLIC pascal trap, INTEGER, OpenResFile, StringPtr, fn)
{
    return OpenRFPerm(fn, 0, fsCurPerm);
}

P1(PUBLIC pascal trap, void, CloseResFile, INTEGER, rn)
{
    resmaphand map, ph, nextmap;
    INTEGER i, j;
    typref *tr;
    resref *rr;

    invalidate_kchr_ptr ();

    ROMlib_invalar();
    if (rn == REF0) {
	for (map = (resmaphand) MR(TopMapHndl); map; map = nextmap) {
	    nextmap = (resmaphand) HxP(map, nextmap);
	    CloseResFile(Hx(map, resfn));
        }
/*-->*/	return;
    } else {
        Handle temph;

	map = ROMlib_rntohandl(rn, &temph);
	ph = (resmaphand) temph;
    }
    if (map) {
        OSErr save_ResErr;

        UpdateResFile(rn);
	save_ResErr = ResErr;
        
        /* update linked list */
        
        if (map == (resmaphand)MR(TopMapHndl))
            TopMapHndl = HxX(map, nextmap);
        else
            HxX(ph, nextmap) = HxX(map, nextmap);

	if (Cx(CurMap) == rn)
	  {
//                printf("curmap %02x topmaphndl %08x\n", (int) CurMap.raw(), (int)TopMapHndl.raw());
	    if (TopMapHndl)
		CurMap = STARH((resmaphand)MR(TopMapHndl))->resfn;
	    else
		CurMap = 0;
	  }

        /* release individual resource memory */

        WALKTANDR(map, i, tr, j, rr)
	  {
	    Handle h;

	    h = rr->rhand;
            if (h)
	      {
		h = MR (h);
		if (*h)
		  HClrRBit (h);
                DisposHandle (h);
	      }
	  }
        EWALKTANDR(tr, rr)
        
        DisposHandle((Handle) map);
        FSClose(rn);
        ROMlib_setreserr(save_ResErr);
    } else
        ROMlib_setreserr(resFNotFound);
}

PRIVATE INTEGER
already_open_res_file (INTEGER swapped_vref, LONGINT swapped_file_num)
{
  resmaphand map;
  fcbrec *fcbp;
  OSErr err;
  INTEGER retval;

  retval = -1;
  WALKMAPTOP (map)
    fcbp = PRNTOFPERR (Hx(map, resfn), &err);
    if (err == noErr && fcbp->fdfnum == swapped_file_num)
      {
	VCB *vptr;
	vptr = MR(fcbp->fcvptr);
	if (vptr->vcbVRefNum == swapped_vref && (fcbp->fcflags & fcfisres))
	  retval = Hx (map, resfn);
      }
  EWALKMAP ()
  return retval;
}

P4 (PUBLIC pascal trap, INTEGER, HOpenResFile, INTEGER, vref, LONGINT, dirid,
    Str255, fn, SignedByte, perm)
{
    INTEGER f;
    reshead hd;
    LONGINT lc;
    resmaphand map;
    INTEGER i, j;
    typref *tr;
    resref *rr;
    HParamBlockRec pbr = {};
    OSErr err;

    invalidate_kchr_ptr ();

    ROMlib_setreserr(noErr);

    /* check for file already opened */

    {
      CInfoPBRec cpb = {};
      Str255 local_name;

      str255assign (local_name, fn);
      pbr.volumeParam.ioNamePtr = (StringPtr) RM ((Ptr) local_name);
      pbr.volumeParam.ioVRefNum = CW (vref);
      pbr.volumeParam.ioVolIndex = CLC (-1);
      err = PBHGetVInfo (&pbr, FALSE);
      if (err)
	{
	  ROMlib_setreserr (err);
	  return -1;
	}

      cpb.hFileInfo.ioNamePtr = RM (fn);
      cpb.hFileInfo.ioVRefNum = CW (vref);
      cpb.hFileInfo.ioFDirIndex = CWC (0);
      cpb.hFileInfo.ioDirID = CL (dirid);
      if ((ROMlib_setreserr(PBGetCatInfo(&cpb, 0))) == noErr
	  && perm > fsRdPerm)
	{
	  INTEGER fref;
	  
	  fref = already_open_res_file (pbr.volumeParam.ioVRefNum,
					cpb.hFileInfo.ioDirID);
	  if (fref != -1)
	    {
	      CurMap = CW (fref);
/*-->*/       return fref;
	    }
	}
    }

    if (ResErr != noErr)
/*-->*/	return -1;

    ROMlib_invalar();
    pbr.ioParam.ioNamePtr = RM (fn);
    pbr.ioParam.ioVRefNum = CW (vref);
    pbr.fileParam.ioFDirIndex = CWC (0);
    pbr.ioParam.ioPermssn = CB (perm);
    pbr.ioParam.ioMisc = CLC (0);
    pbr.fileParam.ioDirID = CL (dirid);
    ROMlib_setreserr(PBHOpenRF(&pbr, FALSE));
    if (ResErr != noErr)
        return(-1);
    f = CW(pbr.ioParam.ioRefNum);
    lc = sizeof(hd);
    ROMlib_setreserr(FSReadAll(f, &lc, (Ptr)&hd));
    if (ResErr != noErr) {
        FSClose(f);
        return(-1);
    }
    map = (resmaphand) NewHandle(CL(hd.maplen));
    err = MemError();
    if (ROMlib_setreserr(err)) {
        FSClose(f);
        return(-1);
    }

    ROMlib_setreserr(SetFPos(f, fsFromStart, Cx(hd.rmapoff)));
    if (ResErr != noErr) {
        DisposHandle((Handle) map);
        FSClose(f);
        return(-1);
    }
    lc = CL(hd.maplen);
    ROMlib_setreserr(FSReadAll(f, &lc, (Ptr) STARH(map)));
    if (ResErr != noErr) {
        DisposHandle((Handle) map);
        FSClose(f);
        return(-1);
    }
    
    HxX(map, rh) = hd;

    /* IMIV: consistency checks */
    
    if (
#if 0	/* See NOTE below */
	Hx(map, rh.rdatoff) != sizeof(reshead) + sizeof(rsrvrec) ||
        Hx(map, rh.rmapoff) < Hx(map, rh.rdatoff) + Hx(map, rh.datlen) ||
#else
	Hx(map, rh.rdatoff) < (int) sizeof(reshead) + (int) sizeof(rsrvrec) ||
#endif
        Hx(map, rh.datlen) < 0 ||
        Hx(map, rh.maplen) < (int) sizeof(resmap) + (int) sizeof(INTEGER) ||
        Hx(map, typoff) < (int) sizeof(resmap)
#if 0
/*
 * NOTE:  I used to have the following test in here, but when I ran
 *	  Disinfectant 1.5 from the BCS Mac PD-CD, we found 6 files
 *	  that were "corrupt" here, but not there.  The first file
 *	  that gave us trouble didn't pass the following test, so
 *	  presumably, the Mac doesn't make this test.
 */
	||
        Hx(map, namoff) < sizeof(resmap) +
                (NUMTMINUS1(map)+1) * (sizeof(resref) + sizeof(typref))
#endif
								       ) {
        ROMlib_setreserr(mapReadErr);
        DisposHandle((Handle) map);
        FSClose(f);
        return(-1);
    }
    
    HxX(map, nextmap) = TopMapHndl;
    HxX(map, resfn) = CW(f);
    TopMapHndl = RM((Handle) map);
    CurMap = CW(f);

    /* check for resprload bits */

    WALKTANDR(map, i, tr, j, rr)
        rr->rhand = 0;
        rr->ratr &= ~resChanged;
        if (rr->ratr & resPreload)
            ROMlib_mgetres(map, rr);
    EWALKTANDR(tr, rr)
    return(f);
}
