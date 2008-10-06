/*
 * This file is used by both Go and HFS_Xfer.  Changes should be tested with both.
 */

#include "xfer.h"

#include "sharedtransfer.proto.h"
#include "alerts.proto.h"
#include "delete.proto.h"

long sizetocopy, sizecopied;
short BreakCopy;
DialogPtr piechartdp;

void
mystr255copy (Str255 dest, Str255 src)
{
  memmove (dest, src, src[0] + 1);
}

long
ischild (long potential_parent_dir, long potential_child_dir, short volume)
{
  CInfoPBRec pb;
  OSErr err;

  if (potential_parent_dir == potential_child_dir)
    return true;		/* for our purposes, any directory is a child of itself */
  pb.dirInfo.ioCompletion = 0;
  pb.dirInfo.ioNamePtr = 0;
  pb.dirInfo.ioVRefNum = volume;
  pb.dirInfo.ioFDirIndex = -1;
  pb.dirInfo.ioDrDirID = potential_child_dir;
  while (pb.dirInfo.ioDrDirID > 1)
    {
      err = PBGetCatInfo (&pb, false);
      if (err != noErr)
	{
	  doerror (err, (StringPtr) "\pPBGetCatInfo");
	  return true;
	}
      if (pb.dirInfo.ioDrParID == potential_parent_dir)
/*-->*/ return true;
      pb.dirInfo.ioDrDirID = pb.dirInfo.ioDrParID;
    }
  return false;
}

#define BUFSIZE (8L * 1024)

static char buf[BUFSIZE];

OSErr
CopyFork (forktype fork, StringPtr srcname, StringPtr destname, long fromid,
	  long toid, short srcvrn, short dstvrn)
{
  HParamBlockRec rdio, wrio;
  OSErr err;
  EventRecord event;

  if (abortflag)
/*-->*/ return noErr;

  rdio.ioParam.ioVRefNum = srcvrn;
  rdio.ioParam.ioNamePtr = srcname;
  rdio.fileParam.ioDirID = fromid;
  rdio.fileParam.ioFDirIndex = 0;
  err = PBHGetFInfo (&rdio, false);
  if (err != noErr)
    {
      doerror (err, (StringPtr) "\pPBGetFInfo");
/*-->*/ return err;
    }
  rdio.fileParam.ioDirID = fromid;
  rdio.ioParam.ioPermssn = fsRdPerm;
  rdio.ioParam.ioMisc = 0;

  wrio.ioParam.ioVRefNum = dstvrn;
  wrio.ioParam.ioNamePtr = destname;
  wrio.ioParam.ioPermssn = fsWrPerm;
  wrio.ioParam.ioMisc = 0;
  wrio.fileParam.ioDirID = toid;

  switch (fork)
    {
    case datafork:
      err = PBHOpen (&rdio, false);
      if (err != noErr)
	return err;
      err = PBHOpen (&wrio, false);
      if (err != noErr)
	{
	  PBClose ((ParmBlkPtr) & rdio, false);
	  return err;
	}
      wrio.ioParam.ioReqCount = rdio.fileParam.ioFlLgLen;
      wrio.ioParam.ioMisc = (LONGORPTR) rdio.fileParam.ioFlLgLen;
      break;
    case resourcefork:
      err = PBHOpenRF (&rdio, false);
      if (err != noErr)
	return err;
      err = PBHOpenRF (&wrio, false);
      if (err != noErr)
	{
	  PBClose ((ParmBlkPtr) & rdio, false);
	  return err;
	}
      wrio.ioParam.ioReqCount = rdio.fileParam.ioFlRLgLen;
      wrio.ioParam.ioMisc = (LONGORPTR) rdio.fileParam.ioFlRLgLen;
      break;
    default:
#if 0
/*-->*/ return fsDSIntErr;
#else
/*-->*/ return -127;
#endif
    }

  err = PBAllocContig ((ParmBlkPtr) & wrio, false);
  if (err != noErr)
    {
      if (err == dskFulErr)
	{
	  err = PBAllocate ((ParmBlkPtr) & wrio, false);
	  if (err != noErr)
	    {
	      doerror (err, (StringPtr) "\pPBAllocate");
/*-->*/ goto DONE;
	    }
	}
      else
	{
	  doerror (err, (StringPtr) "\pPBAllocContig");
/*-->*/ goto DONE;
	}
    }
  err = PBSetEOF ((ParmBlkPtr) & wrio, false);
  if (err != noErr)
    {
      doerror (err, (StringPtr) "\pPBSetEOF");
/*-->*/ goto DONE;
    }

  do
    {
      rdio.ioParam.ioBuffer = (Ptr) buf;
      rdio.ioParam.ioReqCount = BUFSIZE;
      rdio.ioParam.ioPosMode = fsFromMark;
      rdio.ioParam.ioPosOffset = 0;
      err = PBRead ((ParmBlkPtr) & rdio, false);

      if (err == noErr || (err == eofErr && rdio.ioParam.ioActCount > 0))
	{
	  wrio.ioParam.ioBuffer = (Ptr) buf;
	  wrio.ioParam.ioReqCount = rdio.ioParam.ioActCount;
	  wrio.ioParam.ioPosMode = fsFromMark;
	  wrio.ioParam.ioPosOffset = 0;
	  err = PBWrite ((ParmBlkPtr) & wrio, false);
	}
      sizecopied++;
      if (piechartdp)
	updatepiechart ();
      while (GetNextEvent (keyDownMask, &event))
	if ((event.modifiers & cmdKey) &&
	    ((event.message & charCodeMask) == '.'))
	  {
	    err = PBClose ((ParmBlkPtr) & wrio, false);
	    if (err != noErr)
	      doerror (err, (StringPtr) "\pPBClose");
	    wrio.fileParam.ioDirID = toid;
	    err = PBHDelete (&wrio, false);
	    if (err != noErr)
	      doerror (err, (StringPtr) "\pPBHDelete");
	    BreakCopy = true;
	    return eofErr;
	  }
    }
  while (err == noErr);

  sizecopied--;
DONE:
  PBClose ((ParmBlkPtr) & rdio, false);
  PBClose ((ParmBlkPtr) & wrio, false);
  return err;
}

short
copy1file (short srcvrn, short dstvrn, long srcdirid,
	   long dstdirid, Str255 s, BOOLEAN doit)
{
  return duplicate1file (srcvrn, dstvrn, srcdirid, dstdirid, s, s, doit);
}

short
duplicate1file (short srcvrn, short dstvrn, long srcdirid,
		long dstdirid, Str255 name, Str255 destnamep, BOOLEAN doit)

{
  HParamBlockRec hpb;
  OSErr err;
  short retval, type, filesize;
  Handle h;
  Rect r;
  short save_verify_flags;
  Str255 s2;
  Str255 destname;

  mystr255copy (destname, destnamep);
  if (destname[destname[0]] == ':')
    --destname[0];

  abortflag = 0;
  if (BreakCopy)
    return false;
  retval = true;
  hpb.fileParam.ioVRefNum = srcvrn;
  hpb.fileParam.ioDirID = srcdirid;
  hpb.fileParam.ioFDirIndex = 0;
  hpb.fileParam.ioNamePtr = name;
  err = PBGetCatInfo ((CInfoPBPtr) & hpb, false);
  if (err != noErr)
    {
      doerror (err, (StringPtr) "\pPBGetCatInfo");
/*-->*/ return false;
    }
  if (hpb.fileParam.ioFlFndrInfo.fdFlags & fInvisible)
/*-->*/ return false;

  if (hpb.fileParam.ioFlAttrib & ISDIRMASK)
    {
      srcdirid = hpb.fileParam.ioDirID;
      if (doit)
	{
	  hpb.fileParam.ioVRefNum = dstvrn;
	  hpb.fileParam.ioDirID = dstdirid;
	  hpb.fileParam.ioNamePtr = destname;
	  err = PBDirCreate (&hpb, false);
	  if (err == dupFNErr)
	    {
#ifdef THINK_C
	      if (!(verify_flags & VERIFY_OVERWRITE_FOLDER) ||
		  ask ((StringPtr) "\poverwrite folder", destname) == OK)
		{
#else
	      if (!(verify_flags & VERIFY_OVERWRITE_FOLDER) ||
		  ask ("\poverwrite folder", destname) == ok)
		{
#endif
		  save_verify_flags = verify_flags;
		  verify_flags &= ~(VERIFY_DELETE_FOLDER | VERIFY_DELETE_FILE);
		  delete1file (dstvrn, dstdirid, destname);
		  verify_flags = save_verify_flags;
		  err = PBDirCreate (&hpb, false);
		  if (err != noErr)
		    {
		      doerror (err, (StringPtr) "\pPBDirCreate");
/*-->*/ return false;
		    }
		}
	      else
/*-->*/ return false;
	    }
	  else if (err != noErr)
	    {
	      doerror (err, (StringPtr) "\pPBDirCreate");
/*-->*/ return false;
	    }
	  dstdirid = hpb.fileParam.ioDirID;
	}
      hpb.fileParam.ioVRefNum = srcvrn;
      hpb.fileParam.ioNamePtr = s2;
      for (hpb.fileParam.ioFDirIndex = 1; err == noErr && !abortflag;
	   hpb.fileParam.ioFDirIndex++)
	{
	  hpb.fileParam.ioDirID = srcdirid;
	  err = PBGetCatInfo ((CInfoPBPtr) & hpb, false);
	  if (err == noErr)
	    retval &= duplicate1file (srcvrn, dstvrn, srcdirid, dstdirid,
				      s2, s2, doit);
	}
    }
  else
    {
      filesize = (hpb.fileParam.ioFlLgLen + BUFSIZE - 1) / BUFSIZE
	+ (hpb.fileParam.ioFlRLgLen + BUFSIZE - 1) / BUFSIZE;
      if (!doit)
	{
	  sizetocopy += filesize;
	  return retval;
	}
      hpb.fileParam.ioVRefNum = dstvrn;
      hpb.fileParam.ioDirID = dstdirid;
      hpb.fileParam.ioNamePtr = destname;
      err = PBHCreate (&hpb, false);
      if (err == dupFNErr)
	{
#ifdef THINK_C
	  if (!(verify_flags & VERIFY_OVERWRITE_FILE) ||
	      ask ((StringPtr) "\poverwrite file", destname) == OK)
	    {
#else
	  if (!(verify_flags & VERIFY_OVERWRITE_FILE) ||
	      ask ("\poverwrite file", destname) == ok)
	    {
#endif
	      save_verify_flags = verify_flags;
	      verify_flags &= ~VERIFY_DELETE_FILE;
	      delete1file (dstvrn, dstdirid, destname);
	      verify_flags = save_verify_flags;
	      err = PBHCreate (&hpb, false);
	      if (err != noErr)
		{
		  doerror (err, (StringPtr) "\pPBHCreate");
/*-->*/ return false;
		}
	    }
	  else
	    {
	      sizecopied += (hpb.fileParam.ioFlLgLen + BUFSIZE - 1)
		/ BUFSIZE +
		(hpb.fileParam.ioFlRLgLen + BUFSIZE - 1) / BUFSIZE;
/*-->*/ return false;
	    }
	}
      else if (err != noErr)
	{
	  doerror (err, (StringPtr) "\pPBHCreate");
/*-->*/ return false;
	}
      if (piechartdp)
	{
	  GetDItem (piechartdp, FILENAMEITEM, &type, &h, &r);
	  SetIText (h, name);
	}
      err = CopyFork (datafork, name, destname, srcdirid, hpb.fileParam.ioDirID,
		      srcvrn, dstvrn);
      if (err != eofErr)
	{
	  doerror (err, (StringPtr) "\pCopyFork");
	  return false;
	}
      err = CopyFork (resourcefork, name, destname, srcdirid, hpb.fileParam.ioDirID,
		      srcvrn, dstvrn);
      if (err != eofErr)
	{
	  doerror (err, (StringPtr) "\pCopyFork");
	  return false;
	}
      err = PBSetCatInfo ((CInfoPBPtr) & hpb, false);
      if (err != noErr)
	{
	  doerror (err, (StringPtr) "\pPBSetCatInfo");
/*-->*/ return false;
	}
    }
  return retval;
}

short
move1file (short srcvrn, short dstvrn, long srcdirid,
	   long dstdirid, Str255 s, BOOLEAN doit)
{
  CMovePBRec cpb;
  CInfoPBRec hpb;
  OSErr err;
  short save_verify_flags, retval;
  ParamBlockRec pb;

  if (BreakCopy)
    return false;
  retval = true;
  if (dstvrn == srcvrn)
    {
      if (!doit)
/*-->*/ return false;
      cpb.ioCompletion = 0;
      cpb.ioNamePtr = s;
      cpb.ioVRefNum = srcvrn;
      cpb.ioNewName = 0;
      cpb.ioNewDirID = dstdirid;
      cpb.ioDirID = srcdirid;
      err = PBCatMove (&cpb, false);
      if (err == badMovErr)
	warnaboutincest ();
      else if (err == dupFNErr)
	{
	  hpb.hFileInfo.ioCompletion = 0;
	  hpb.hFileInfo.ioNamePtr = s;
	  hpb.hFileInfo.ioVRefNum = srcvrn;
	  hpb.hFileInfo.ioFDirIndex = 0;
	  hpb.hFileInfo.ioDirID = srcdirid;
	  CHECKR (PBGetCatInfo, &hpb);
	  if (hpb.hFileInfo.ioFlAttrib & ISDIRMASK)
	    {
#ifdef THINK_C
	      if (!(verify_flags & VERIFY_OVERWRITE_FOLDER) ||
		  ask ((StringPtr) "\poverwrite folder", s) == OK)
		{
#else
	      if (!(verify_flags & VERIFY_OVERWRITE_FOLDER) ||
		  ask ("\poverwrite folder", s) == ok)
		{
#endif
		  save_verify_flags = verify_flags;
		  verify_flags &= ~(VERIFY_DELETE_FOLDER | VERIFY_DELETE_FILE);
		  delete1file (dstvrn, dstdirid, s);
		  verify_flags = save_verify_flags;
		  err = PBCatMove (&cpb, false);
		  if (err != noErr)
		    doerror (err, (StringPtr) "\pPBCatMove");
		}
	    }
	  else
	    {
#ifdef THINK_C
	      if (!(verify_flags & VERIFY_OVERWRITE_FILE) ||
		  ask ((StringPtr) "\poverwrite file", s) == OK)
		{
#else
	      if (!(verify_flags & VERIFY_OVERWRITE_FILE) ||
		  ask ("\poverwrite file", s) == ok)
		{
#endif
		  save_verify_flags = verify_flags;
		  verify_flags &= ~VERIFY_DELETE_FILE;
		  delete1file (dstvrn, dstdirid, s);
		  verify_flags = save_verify_flags;
		  err = PBCatMove (&cpb, false);
		  if (err != noErr)
		    doerror (err, (StringPtr) "\pPBCatMove");
		}
	    }
	}
      else if (err != noErr)
	doerror (err, (StringPtr) "\pPBCatMove");
    }
  else
    {
      if (hpb.hFileInfo.ioFlFndrInfo.fdFlags & fInvisible)
/*-->*/ return false;
      pb.volumeParam.ioVolIndex = 0;
      pb.volumeParam.ioVRefNum = srcvrn;
      pb.volumeParam.ioNamePtr = 0;
      CHECKR (PBGetVInfo, &pb);
      if (pb.volumeParam.ioVAtrb & VOLLOCKEDMASK)
	{
	  if (doit)
	    {
	      ParamText ((StringPtr)
		   "\pFiles can not be moved from a locked disk.", 0, 0, 0);
#ifdef THINK_C
	      StopAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
#else
	      StopAlert (FOUR_PARAM_ALERT, (ModalFilterUPP) 0);
#endif
	    }
/*-->*/ return false;
	}

      if ((retval = copy1file (srcvrn, dstvrn, srcdirid, dstdirid, s, doit))
	  && doit)
	{
	  save_verify_flags = verify_flags;
	  verify_flags &= ~(VERIFY_DELETE_FOLDER | VERIFY_DELETE_FILE);
	  delete1file (srcvrn, srcdirid, s);
	  verify_flags = save_verify_flags;
	}
    }
  return retval;
}

short
commontrans (long fromd, long tod, short fromv, short tov, StringPtr s,
	     short (*fp) (short, short, long, long, Str255, char))
{
  OSErr err;
  ParamBlockRec pb;
  GrafPtr saveport;
  HParamBlockRec hpb;

  if (fromv == tov)
    {
      hpb.fileParam.ioVRefNum = fromv;
      hpb.fileParam.ioDirID = fromd;
      hpb.fileParam.ioFDirIndex = 0;
      hpb.fileParam.ioCompletion = 0;
      hpb.fileParam.ioNamePtr = s;
      PBGetCatInfo ((CInfoPBPtr) & hpb, false);
      if (hpb.fileParam.ioFlAttrib & ISDIRMASK && ischild (hpb.fileParam.ioDirID, tod, fromv))
	{
	  copyintochildrenwarning ();
/*-->*/ return false;
	}
    }
  piechartdp = (GrafPtr) 0;
  BreakCopy = false;
  sizetocopy = 0;
  sizecopied = 0;
  GetPort (&saveport);
  (*fp) (fromv, tov, fromd, tod, s, false);
  pb.volumeParam.ioCompletion = 0;
  pb.volumeParam.ioNamePtr = 0;
  pb.volumeParam.ioVRefNum = tov;
  pb.volumeParam.ioVolIndex = 0;
  CHECKR (PBGetVInfo, &pb);
  if (sizetocopy > pb.volumeParam.ioVFrBlk * pb.volumeParam.ioVAlBlkSiz / BUFSIZE)
    {
      noroom (sizetocopy * BUFSIZE / pb.volumeParam.ioVAlBlkSiz,
	      pb.volumeParam.ioVFrBlk);
      return false;
    }

  makepiechart ();
  updatepiechart ();
  (*fp) (fromv, tov, fromd, tod, s, true);
  updatepiechart ();
  SetPort (saveport);
  if (piechartdp)
    DisposDialog (piechartdp);
  piechartdp = 0;
  sizetocopy = 0;
  return true;
}
