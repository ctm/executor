/* Copyright 1992 - 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_hfsXbar[] =
	    "$Id: hfsXbar.c 87 2005-05-25 01:57:33Z ctm $";
#endif

/* Forward declarations in FileMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "FileMgr.h"
#include "rsys/hfs.h"
#include "rsys/file.h"
#include "rsys/stdfile.h"
#include "rsys/flags.h"
#include "rsys/prefs.h"

using namespace Executor;
using namespace ByteSwap;

/*
 * TODO: pass the information gleaned by hfsvol and hfsfil into the
 *       various hfsPB and ufsPB routines.
 */

#if defined(CACHECHECK)
void cachecheck(HVCB *vcbp)
{
    cacheentry *cachep;
    cachehead *headp;
    INTEGER i;
    
    headp = (cachehead *) BigEndianValue(vcbp->vcbCtlBuf);
    for (i = Cx(headp->nitems), cachep = Cx(headp->flink); --i >= 0;
						    cachep = Cx(cachep->flink))
	if (Cx(cachep->flags) & CACHEBUSY)
	    warning_unexpected ("busy");
}
#endif /* defined(CACHECHECK) */

PRIVATE BOOLEAN hfsvol(ioParam *pb)
{
    HVCB *vcbp;
    LONGINT dir;
    
    vcbp = ROMlib_findvcb(Cx(pb->ioVRefNum), MR(pb->ioNamePtr), &dir, TRUE);
    if (!vcbp) {
	vcbp = ROMlib_findvcb(Cx(pb->ioVRefNum), (StringPtr) 0, &dir, TRUE);
	if (!vcbp)
	    return TRUE;	/* hopefully is a messed up working dir
				    reference */
    }
    if (vcbp->vcbCTRef) {
#if defined(CACHECHECK)
	cachecheck(vcbp);
#endif /* defined(CACHECHECK) */
	return TRUE;
    } else
	return FALSE;
}

PRIVATE BOOLEAN hfsIvol(volumeParam *pb)	/* potentially Indexed vol */
{
    BOOLEAN retval;
    HVCB *vcbp;

    retval = FALSE;
    if (Cx(pb->ioVolIndex) > 0) {
	vcbp = (HVCB *) ROMlib_indexqueue(&VCBQHdr, Cx(pb->ioVolIndex));
	if (vcbp && vcbp->vcbCTRef)
	    retval = TRUE;
    } else
	retval = hfsvol((ioParam *) pb);
    return retval;
}

PRIVATE BOOLEAN hfsfil(ioParam *pb)
{
    filecontrolblock *fcbp;
    HVCB *vcbp;

    fcbp = ROMlib_refnumtofcbp(Cx(pb->ioRefNum));
    if (fcbp) {
	vcbp = MR(fcbp->fcbVPtr);
	if (vcbp->vcbCTRef) {
#if defined(CACHECHECK)
	    cachecheck(vcbp);
#endif /* defined(CACHECHECK) */
	    return TRUE;
	} else
	    return FALSE;
    } else
	return FALSE;
}

A2(PUBLIC trap, OSErrRET, PBHRename, HParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBHRename(pb, async);
    else
	retval = ufsPBHRename(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBHCreate, HParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBHCreate(pb, async);
    else
	retval = ufsPBHCreate(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBDirCreate, HParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBDirCreate(pb, async);
    else
	retval = ufsPBDirCreate(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBHDelete, HParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBHDelete(pb, async);
    else
	retval = ufsPBHDelete(pb, async);
    FAKEASYNC (pb, async, retval);
}

PRIVATE void
try_to_reopen (DrvQExtra *dqp)
{
#if defined(LINUX)
  drive_flags_t flags;
  dqp->hfs.fd = linuxfloppy_open (0, &dqp->hfs.bsize, &flags,
				  (char *) dqp->devicename);
#elif defined(MSDOS)
#else
/* #warning need to be able to reopen a drive */
  warning_unimplemented ("Unable to reopen a drive because the code has not "
			 "yet\nbeen written for this platform.");
#endif
}

PUBLIC int Executor::ROMlib_directdiskaccess = FALSE;

A2(PUBLIC trap, OSErrRET, PBRead, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;
    DrvQExtra *dqp;

    switch (Cx(pb->ioParam.ioRefNum)) {
    case OURHFSDREF:
	if (ROMlib_directdiskaccess) {
	    dqp = ROMlib_dqbydrive(Cx(pb->ioParam.ioVRefNum));
	    if (!dqp) {
		pb->ioParam.ioResult = BigEndianValue(nsvErr);
		pb->ioParam.ioActCount = 0;
	    } else {
	        if (dqp->hfs.fd == -1)
		  try_to_reopen (dqp);
		if (dqp->hfs.fd == -1)
		  {
		    pb->ioParam.ioResult = CWC (nsvErr);
		    pb->ioParam.ioActCount = 0;
		  }
		else if (Cx(pb->ioParam.ioPosMode) != fsFromStart) {
		    pb->ioParam.ioResult = BigEndianValue(paramErr);	/* for now */
		    pb->ioParam.ioActCount = 0;
		} else
		    pb->ioParam.ioResult = BigEndianValue(ROMlib_transphysblk(&dqp->hfs,
						   Cx(pb->ioParam.ioPosOffset),
					Cx(pb->ioParam.ioReqCount) / PHYSBSIZE,
					     MR(pb->ioParam.ioBuffer), reading,
						  &pb->ioParam.ioActCount));

	    }
	} else {
	    pb->ioParam.ioResult = BigEndianValue(vLckdErr);
	    pb->ioParam.ioActCount = 0;
	}
	retval = Cx(pb->ioParam.ioResult);
	break;

    default:
	if (pb->ioParam.ioPosMode & CWC (NEWLINEMODE))
	  {
	    char *buf, *p_to_find, *p_alternate, *p;
	    unsigned char to_find;
	    long act_count;
	    ParamBlockRec pbr;

	    pbr = *pb;
	    to_find = BigEndianValue (pb->ioParam.ioPosMode) >> 8;
	    pbr.ioParam.ioPosMode &= CWC (0x7F);

	    buf = (char*)alloca (BigEndianValue (pb->ioParam.ioReqCount));

	    pbr.ioParam.ioBuffer = (Ptr) RM (buf);
	    retval = PBRead (&pbr, FALSE);
	    pb->ioParam.ioActCount  = pbr.ioParam.ioActCount;
	    pb->ioParam.ioPosOffset = pbr.ioParam.ioPosOffset;

	    act_count = BigEndianValue (pb->ioParam.ioActCount);
	    p_to_find = (char*)memchr (buf, to_find, act_count);

	    if (to_find == '\r' && ROMlib_newlinetocr)
	      p_alternate = (char*)memchr (buf, '\n', act_count);
	    else
	      p_alternate = 0;

	    if (p_alternate && (!p_to_find || p_alternate < p_to_find))
	      p = p_alternate;
	    else
	      p = p_to_find;

	    if (p)
	      {
		long to_backup;

		retval = noErr; /* we couldn't have gotten EOF yet */
		to_backup = act_count - (p+1 - buf);
		SWAPPED_OPL (pb->ioParam.ioActCount , -, to_backup);
#if 0
		SWAPPED_OPL (pb->ioParam.ioPosOffset, -, to_backup);
#else
		{
		  ParamBlockRec newpb;
		  OSErr newerr;

		  newpb.ioParam.ioRefNum = pb->ioParam.ioRefNum;
		  newpb.ioParam.ioPosMode = CWC (fsFromMark);
		  newpb.ioParam.ioPosOffset = BigEndianValue (- to_backup);
		  newerr = PBSetFPos (&newpb, FALSE);
		  if (newerr != noErr)
		    warning_unexpected ("err = %d", newerr);
		}
#endif
		if (ROMlib_newlinetocr && to_find == '\r')
		  *p = '\r';
	      }
	    memcpy (MR (pb->ioParam.ioBuffer), MR (pbr.ioParam.ioBuffer),
		    BigEndianValue (pb->ioParam.ioActCount));
	    ROMlib_destroy_blocks ((syn68k_addr_t) (long)
				   US_TO_SYN68K(MR (pb->ioParam.ioBuffer)),
				   BigEndianValue (pb->ioParam.ioActCount), TRUE);
	  }
	else
	  {
	    if (hfsfil((ioParam *) pb))
	      retval = hfsPBRead(pb, async);
	    else
	      retval = ufsPBRead(pb, async);
	  }
	break;
    }
    FAKEASYNC (pb, async, retval);
}

#define SOUND_DRIVER_REF	(-4)
#define ffMode			0

A2(PUBLIC trap, OSErrRET, PBWrite, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;
    HVCB *vcbp;
    DrvQExtra *dqp;

    switch (Cx(pb->ioParam.ioRefNum)) {
    case OURHFSDREF:
	if (!ROMlib_directdiskaccess)
	    pb->ioParam.ioResult = BigEndianValue(vLckdErr);
	else {
	    dqp = ROMlib_dqbydrive(Cx(pb->ioParam.ioVRefNum));
	    if (dqp && dqp->hfs.fd == -1)
	      try_to_reopen (dqp);
	    vcbp = ROMlib_vcbbydrive (Cx(pb->ioParam.ioVRefNum));
	    if (!dqp)
		pb->ioParam.ioResult = BigEndianValue(nsvErr);
	    else if (vcbp && (Cx(vcbp->vcbAtrb) & VSOFTLOCKBIT))
		pb->ioParam.ioResult = BigEndianValue(vLckdErr);
	    else if (vcbp && (Cx(vcbp->vcbAtrb) & VHARDLOCKBIT))
		pb->ioParam.ioResult = BigEndianValue(wPrErr);
	    else if (Cx(pb->ioParam.ioPosMode) != fsFromStart)
		    pb->ioParam.ioResult = BigEndianValue(paramErr);	/* for now */
	    else
		pb->ioParam.ioResult = BigEndianValue(ROMlib_transphysblk(&dqp->hfs,
						   Cx(pb->ioParam.ioPosOffset),
					Cx(pb->ioParam.ioReqCount) / PHYSBSIZE,
					     MR(pb->ioParam.ioBuffer), writing,
						  &pb->ioParam.ioActCount));

	}
	if (pb->ioParam.ioResult != noErr)
	    pb->ioParam.ioActCount = 0;
	retval = Cx(pb->ioParam.ioResult);
	break;

#if 0
    case SOUND_DRIVER_REF:
	p = (char *) Cx(pb->ioParam.ioBuffer);
	if (BigEndianValue(*(short *)p) == ffMode) {
	    n = Cx(pb->ioParam.ioReqCount);
	    ROMlib_dosound(p + 4, n - 4, (void (*)(void)) 0);
	}
	retval = noErr;
	break;
#endif
    default:
	if (hfsfil((ioParam *) pb))
	    retval = hfsPBWrite(pb, async);
	else
	    retval = ufsPBWrite(pb, async);
	break;
    }
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBClose, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsfil((ioParam *) pb))
	retval = hfsPBClose(pb, async);
    else
	retval = ufsPBClose(pb, async);
    FAKEASYNC (pb, async, retval);
}

/*
 * The test for ioBuffer being 0 was determined necessary after MacRoads
 * couldn't open the serial drivers.  When I first tested PBHOpen to see
 * if it could open drivers, it couldn't, but that's because my test was
 * coded according to Inside Macintosh and I was assuming that ioBuffer
 * wasn't examined.  It took a few runs of binary searches stuffing fields
 * with zeros to find out that ioBuffer had this magic property.
 *
 * PBOpen doesn't require ioBuffer to be 0 in order to open a driver.
 */

A2(PUBLIC trap, OSErrRET, PBHOpen, HParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (pb->ioParam.ioBuffer == 0 &&
	pb->ioParam.ioNamePtr && MR(pb->ioParam.ioNamePtr)[0]
			      && MR(pb->ioParam.ioNamePtr)[1] == '.')
        retval = ROMlib_driveropen((ParmBlkPtr) pb, async);
    else if (hfsvol((ioParam *) pb))
	retval = hfsPBHOpen(pb, async);
    else
	retval = ufsPBHOpen(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBOpenDF, HParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBHOpen(pb, async);
    else
	retval = ufsPBHOpen(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBHOpenRF, HParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBHOpenRF(pb, async);
    else
	retval = ufsPBHOpenRF(pb, async);
    FAKEASYNC (pb, async, retval);
}

#if 0
PRIVATE void
swappedstr255print (const char *prefix, Str255 sp)
{
  printf ("%s", prefix);
  if (sp)
    {
      unsigned char *cp;
      int n;

      cp = (unsigned char *) MR(sp);
      n = *cp++;
      while (n-- > 0)
	putchar (*cp++);
    }
  else
    printf ("<empty>");
  printf ("\n");
}
#endif

A2(PUBLIC trap, OSErrRET, PBGetCatInfo, CInfoPBPtr, pb, BOOLEAN, async)
{
    OSErr retval;
    BOOLEAN ishfs;
    StringPtr savep;

    if (BigEndianValue(pb->dirInfo.ioFDirIndex) < 0 && pb->hFileInfo.ioDirID == CLC (1))
	retval = -43; /* perhaps we should check for a valid volume
			 first */
    else
      {
	savep = pb->dirInfo.ioNamePtr;
	if (pb->dirInfo.ioFDirIndex != CWC (0))	/* IMIV-155, 156 */
	  pb->dirInfo.ioNamePtr = 0;
	ishfs = hfsvol((ioParam *) pb);
	pb->dirInfo.ioNamePtr = savep;
	
	if (ishfs)
	  retval = hfsPBGetCatInfo(pb, async);
	else
	  retval = ufsPBGetCatInfo(pb, async);
      }
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBSetCatInfo, CInfoPBPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBSetCatInfo(pb, async);
    else
	retval = ufsPBSetCatInfo(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBCatMove, CMovePBPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBCatMove(pb, async);
    else
	retval = ufsPBCatMove(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBGetVInfo, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsIvol((volumeParam *) pb))
	retval = hfsPBGetVInfo(pb, async);
    else
	retval = ufsPBGetVInfo(pb, async);
    FAKEASYNC (pb, async, retval);
}

A1(PUBLIC trap, OSErrRET, PBUnmountVol, ParmBlkPtr, pb)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBUnmountVol(pb);
    else
	retval = ufsPBUnmountVol(pb);
    PBRETURN (pb, retval);
}

A1(PUBLIC trap, OSErrRET, PBEject, ParmBlkPtr, pb)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBEject(pb);
    else
	retval = ufsPBEject(pb);
    PBRETURN (pb, retval);
}

A2(PUBLIC trap, OSErrRET, PBAllocate, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsfil((ioParam *) pb))
	retval = hfsPBAllocate(pb, async);
    else
	retval = ufsPBAllocate(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBAllocContig, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsfil((ioParam *) pb))
	retval = hfsPBAllocContig(pb, async);
    else
	retval = ufsPBAllocContig(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBHGetFInfo, HParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;
    BOOLEAN ishfs;
    StringPtr savep;

    savep = pb->ioParam.ioNamePtr;
    if (BigEndianValue(pb->fileParam.ioFDirIndex) > 0)	/* IMIV-155, 156 */
	pb->ioParam.ioNamePtr = 0;
    ishfs = hfsvol((ioParam *) pb);
    pb->ioParam.ioNamePtr = savep;

    if (ishfs)
	retval = hfsPBHGetFInfo(pb, async);
    else
	retval = ufsPBHGetFInfo(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBSetEOF, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsfil((ioParam *) pb))
	retval = hfsPBSetEOF(pb, async);
    else
	retval = ufsPBSetEOF(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBOpen, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (pb->ioParam.ioNamePtr && MR(pb->ioParam.ioNamePtr)[0]
			      && MR(pb->ioParam.ioNamePtr)[1] == '.')
	retval = ROMlib_driveropen(pb, async);
    else if (hfsvol((ioParam *) pb))
	retval = hfsPBOpen(pb, async);
    else
	retval = ufsPBOpen(pb, async);
    FAKEASYNC (pb, async, retval);
}

#if !defined (NDEBUG)
PUBLIC void test_serial(void)
{
  OSErr open_in_val, open_out_val, close_in_val, close_out_val;
  ParamBlockRec pb_in, pb_out;
  static int count = 0;

  memset (&pb_in, 0, sizeof pb_in);
  memset (&pb_out, 0, sizeof pb_out);
  pb_in.ioParam.ioNamePtr = RM ((StringPtr) "\004.AIn");
  pb_out.ioParam.ioNamePtr = RM ((StringPtr) "\005.AOut");
  open_in_val = PBOpen (&pb_in, FALSE);
  open_out_val = PBOpen (&pb_out, FALSE);
  close_in_val = PBClose (&pb_in, FALSE);
  close_out_val  = PBClose (&pb_out, FALSE);
  ++count;
}
#endif

A2(PUBLIC trap, OSErrRET, PBOpenRF, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBOpenRF(pb, async);
    else
	retval = ufsPBOpenRF(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBLockRange, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsfil((ioParam *) pb))
	retval = hfsPBLockRange(pb, async);
    else
	retval = ufsPBLockRange(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBUnlockRange, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsfil((ioParam *) pb))
	retval = hfsPBUnlockRange(pb, async);
    else
	retval = ufsPBUnlockRange(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBGetFPos, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsfil((ioParam *) pb))
	retval = hfsPBGetFPos(pb, async);
    else
	retval = ufsPBGetFPos(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBSetFPos, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsfil((ioParam *) pb))
	retval = hfsPBSetFPos(pb, async);
    else
	retval = ufsPBSetFPos(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBGetEOF, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsfil((ioParam *) pb))
	retval = hfsPBGetEOF(pb, async);
    else
	retval = ufsPBGetEOF(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBFlushFile, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsfil((ioParam *) pb))
	retval = hfsPBFlushFile(pb, async);
    else
	retval = ufsPBFlushFile(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBCreate, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBCreate(pb, async);
    else
	retval = ufsPBCreate(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBDelete, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBDelete(pb, async);
    else
	retval = ufsPBDelete(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBOpenWD, WDPBPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBOpenWD(pb, async);
    else
	retval = ufsPBOpenWD(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBCloseWD, WDPBPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    retval = hfsPBCloseWD(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBGetWDInfo, WDPBPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    retval = hfsPBGetWDInfo(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBGetFInfo, ParmBlkPtr, pb, BOOLEAN, async)
{
    BOOLEAN ishfs;
    StringPtr savep;
    OSErr retval;

    savep = pb->ioParam.ioNamePtr;
    if (BigEndianValue(pb->fileParam.ioFDirIndex) > 0)	/* IMIV-155, 156 */
	pb->ioParam.ioNamePtr = 0;
    ishfs = hfsvol((ioParam *) pb);
    pb->ioParam.ioNamePtr = savep;

    if (ishfs)
	retval = hfsPBGetFInfo(pb, async);
    else
	retval = ufsPBGetFInfo(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBSetFInfo, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBSetFInfo(pb, async);
    else
	retval = ufsPBSetFInfo(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBHSetFInfo, HParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBHSetFInfo(pb, async);
    else
	retval = ufsPBHSetFInfo(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBSetFLock, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBSetFLock(pb, async);
    else
	retval = ufsPBSetFLock(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBHSetFLock, HParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBHSetFLock(pb, async);
    else
	retval = ufsPBHSetFLock(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBRstFLock, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBRstFLock(pb, async);
    else
	retval = ufsPBRstFLock(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBHRstFLock, HParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBHRstFLock(pb, async);
    else
	retval = ufsPBHRstFLock(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBSetFVers, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBSetFVers(pb, async);
    else
	retval = ufsPBSetFVers(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBRename, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBRename(pb, async);
    else
	retval = ufsPBRename(pb, async);
    FAKEASYNC (pb, async, retval);
}

/*
 * The code below has a hack in it that is due to the non-standard way in
 * which we handle floppy drives.  Currently, under Executor, Cmd-Shift-2
 * causes all potentially removable media drives to be scanned for new volumes
 * and any that are found are immediately mounted.  This means that programs
 * that use GetOSEvent to pick up disk inserted events *before* mounting that
 * then do their own mounting will lose, since the volume will already be
 * mounted and hence their own mount will fail.  BodyWorks 3.0's Installer
 * does this.  So currently if drive 1 or drive 2 is explictly being mounted
 * we return noErr.  This can get us into trouble later, but to fix it will
 * require a major rewrite of how we handle removable media.
 *
 * As a matter of fact, Browser used to have problems with this because it
 * would call PBMountVol to determine whether or not the floppy drive had
 * a disk in it that either was mounted or could be mounted, and it would
 * then call PBGetVInfo and not look at the return value and unconditionally
 * use the volume name it expected to be returned.  Ugh.  
 */

/* #warning hacked PBMountVol */

A1(PUBLIC trap, OSErr, PBMountVol, ParmBlkPtr, pb)
{
#if 0
    if (hfsfil((ioParam *) pb))
	return hfsPBMountVol(pb);
    else
	return ufsPBMountVol(pb);
#else
    INTEGER vref;
    OSErr retval;

    vref = BigEndianValue(pb->ioParam.ioVRefNum);
    if (vref == 1 || vref == 2)
      retval = noErr;
    else
      retval = ufsPBMountVol(pb);
    PBRETURN (pb, retval);
#endif
}

A2(PUBLIC trap, OSErrRET, PBHGetVInfo, HParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsIvol((volumeParam *) pb))
	retval = hfsPBHGetVInfo(pb, async);
    else
	retval = ufsPBHGetVInfo(pb, async);
    FAKEASYNC (pb, async, retval);
}

/* Smaller Installer wants to see bit 17 set.  We're guessing the other
 * bits will keep programs from messing with us */

enum { VOL_BITS = ((1L << bHasExtFSVol)
		   | (1L << bNoSysDir)
		   | (1L << bNoBootBlks)
		   | (1L << bNoDeskItems)
		   | (1L << bNoSwitchTo)
		   | (1L << bNoLclSync)
		   | (1L << bNoVNEdit)
		   | (1L << bNoMiniFndr)) };

A2(PUBLIC trap, OSErrRET, PBHGetVolParms, HParmBlkPtr, pb, BOOLEAN, async)
{
  LONGINT dir;
  HVCB *vcbp;
  getvolparams_info_t *infop;
  OSErr err;
  INTEGER rc, nused;

#define roomfor(ptr, field, byte_count) \
  ((byte_count) \
   >= (int) offsetof (typeof (*(ptr)), field) + (int) sizeof ((ptr)->field))
    
  vcbp = ROMlib_findvcb(Cx(pb->ioParam.ioVRefNum),
			MR(pb->ioParam.ioNamePtr), &dir, FALSE);
  if (vcbp)
    {
      infop = (getvolparams_info_t *) MR (pb->ioParam.ioBuffer);
      rc     = BigEndianValue (pb->ioParam.ioReqCount);
      nused  = 0;
      if (roomfor (infop, vMVersion, rc))
	{
	  infop->vMVersion = CWC (2);
	  nused += sizeof (infop->vMVersion);
	}
      if (roomfor (infop, vMAttrib, rc))
	{
	  infop->vMAttrib = CLC(VOL_BITS);
	  nused += sizeof (infop->vMAttrib);
	}
      if (roomfor (infop, vMLocalHand, rc))
	{
	  infop->vMLocalHand = CLC (0);
	  nused += sizeof (infop->vMLocalHand);
	}
      if (roomfor (infop, vMServerAdr, rc))
	{
	  infop->vMServerAdr = CLC (0);
	  nused += sizeof (infop->vMServerAdr);
	}
      if (roomfor (infop, vMForeignPrivID, rc))
	{
	  infop->vMForeignPrivID = CWC (2); /* fsUnixPriv + 1 */
	  nused += sizeof (infop->vMForeignPrivID);
	}
      pb->ioParam.ioActCount = BigEndianValue((LONGINT) nused);
      err = noErr;
    }
  else
    {
      err = nsvErr;
      pb->ioParam.ioActCount = CLC (0);
    }
  FAKEASYNC (pb, async, err);
}

A2(PUBLIC trap, OSErrRET, PBSetVInfo, HParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBSetVInfo(pb, async);
    else
	retval = ufsPBSetVInfo(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBGetVol, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    retval = hfsPBGetVol(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBHGetVol, WDPBPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    retval = hfsPBHGetVol(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBSetVol, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    retval = hfsPBSetVol(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBHSetVol, WDPBPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    retval = hfsPBHSetVol(pb, async);
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBFlushVol, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBFlushVol(pb, async);
    else
	retval = ufsPBFlushVol(pb, async);
    FAKEASYNC (pb, async, retval);
}

A1(PUBLIC trap, OSErrRET, PBOffLine, ParmBlkPtr, pb)
{
    OSErr retval;

    if (hfsvol((ioParam *) pb))
	retval = hfsPBOffLine(pb);
    else
	retval = ufsPBOffLine(pb);
    PBRETURN (pb, retval);
}

A2(PUBLIC trap, OSErrRET, PBExchangeFiles, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    warning_unimplemented (NULL_STRING);
    retval = paramErr;
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBCatSearch, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    warning_unimplemented (NULL_STRING);
    retval = paramErr;
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBCreateFileIDRef, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    warning_unimplemented (NULL_STRING);
    retval = paramErr;
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBDeleteFileIDRef, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    warning_unimplemented (NULL_STRING);
    retval = paramErr;
    FAKEASYNC (pb, async, retval);
}

A2(PUBLIC trap, OSErrRET, PBResolveFileIDRef, ParmBlkPtr, pb, BOOLEAN, async)
{
    OSErr retval;

    warning_unimplemented (NULL_STRING);
    retval = paramErr;
    FAKEASYNC (pb, async, retval);
}
