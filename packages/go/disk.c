#include "go.h"
#include "xfer.h"

#include "disk.proto.h"
#include "inithotband.proto.h"
#include "alerts.proto.h"

/* TODO: move to .h file */
#define FLOPPYDRIVE	1
#define NAMEVOLDIALOG	203
#define VOLNAMEITEM	2
#define FORMATTEXTITEM	3

void
checkfordisk (void)
{
  ParamBlockRec pb;
  OSErr e;
  EventRecord ev;
  short i;
  EvQEl *qp;

/* Tell Executor/DOS to check for disks. */

  if (executor_p ())
    unixmount (0);

  pb.ioParam.ioVRefNum = FLOPPYDRIVE;
  e = PBMountVol (&pb);
  resetvolumes ();
}

void
format (void)
{
  ParamBlockRec pb;
  OSErr e;
  short vrefnum, doit, type, itemhit;
  Rect r;
  long free;
  Str255 s;
  Handle h;
  DialogPtr dp;

  pb.ioParam.ioVRefNum = FLOPPYDRIVE;
  e = PBMountVol (&pb);
  switch (e)
    {
    case volOnLinErr:
    case noErr:
      e = GetVInfo (FLOPPYDRIVE, s, &vrefnum, &free);
      pb.ioParam.ioVRefNum = vrefnum;
      pb.ioParam.ioNamePtr = 0;
      PBUnmountVol (&pb);
#ifdef THINK_C
      doit = (ask ((StringPtr) "\pErase the floppy disk", s) == OK);
#else
      doit = (ask ("\pErase the floppy disk", s) == ok);
#endif
      if (!doit)
	{
	  pb.ioParam.ioVRefNum = FLOPPYDRIVE;
	  PBMountVol (&pb);
	}
      break;
    case badMDBErr:
    case extFSErr:
    case noMacDskErr:
    case ioErr:
#ifdef THINK_C
      doit = (ask ((StringPtr) "\pFormat the floppy disk", (StringPtr) "\p") == OK);
#else
      doit = (ask ("\pFormat the floppy disk", (StringPtr) "\p") == ok);
#endif
      break;
    case paramErr:
    case memFullErr:
    case nsDrvErr:
    case tmfoErr:
    default:
      ParamText ((StringPtr)
      "\pThe floppy disk could not be formatted because an error occurred.",
		 0, 0, 0);
#ifdef THINK_C
      StopAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
#else
      StopAlert (FOUR_PARAM_ALERT, (ModalFilterUPP) 0);
#endif
      doit = false;
      break;
    }
  if (doit)
    {
      dp = GetNewDialog (NAMEVOLDIALOG, 0, (WindowPtr) - 1);
      MoveWindow (dp, (qd.screenBits.bounds.right - qd.screenBits.bounds.left -
		     dp->portRect.right + dp->portRect.left) / 2, 80, true);
      ShowWindow (dp);
      SelectWindow (dp);
#ifdef THINK_C
      ModalDialog ((ProcPtr) 0, &itemhit);
#else
      ModalDialog ((ModalFilterUPP) 0, &itemhit);
#endif
      GetDItem (dp, VOLNAMEITEM, &type, &h, &r);
      GetIText (h, s);
      ParamText (s, 0, 0, 0);
      EraseRect (&r);
      OffsetRect (&r, -1000, -1000);
      SetDItem (dp, VOLNAMEITEM, type, h, &r);
      GetDItem (dp, FORMATTEXTITEM, &type, &h, &r);
      SetIText (h, (StringPtr) "\pFormatting Disk ^0.");
      e = DIFormat (FLOPPYDRIVE);
      if (e != noErr)
	{
	  ParamText ((StringPtr) "\pThe format failed.", 0, 0, 0);
#ifdef THINK_C
	  StopAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
#else
	  StopAlert (FOUR_PARAM_ALERT, (ModalFilterUPP) 0);
#endif
	  DisposeDialog (dp);
/*-->*/ return;
	}
      SetIText (h, (StringPtr) "\pVerifying Disk ^0.");
      e = DIVerify (FLOPPYDRIVE);
      if (e != noErr)
	{
	  ParamText ((StringPtr) "\pThe format could not be verified.", 0, 0, 0);
#ifdef THINK_C
	  StopAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
#else
	  StopAlert (FOUR_PARAM_ALERT, (ModalFilterUPP) 0);
#endif
	  DisposeDialog (dp);
/*-->*/ return;
	}
      SetIText (h, (StringPtr) "\pInitializing Disk ^0.");
      e = DIZero (FLOPPYDRIVE, s);
      if (e != noErr)
	{
	  ParamText ((StringPtr) "\pZeroing the disk failed.", 0, 0, 0);
#ifdef THINK_C
	  StopAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
#else
	  StopAlert (FOUR_PARAM_ALERT, (ModalFilterUPP) 0);
#endif
	}
      DisposeDialog (dp);
      pb.ioParam.ioVRefNum = FLOPPYDRIVE;
      e = PBMountVol (&pb);
    }
  resetvolumes ();
}

OSErr
get_HParamBlockRec_from_ControlHandle (HParamBlockRec * hpbp, ControlHandle c)
{
  Str255 s;

  mystr255copy (s, (*c)->contrlTitle);
  hpbp->volumeParam.ioNamePtr = s;
  hpbp->volumeParam.ioVRefNum = 0;
  hpbp->volumeParam.ioVolIndex = -1;
  return PBHGetVInfo (hpbp, false);
}

void
goeject ()
{
  ParamBlockRec pb;
  OSErr e;
  short vrefnum;
  WindowPeek wp, nextwp;
  ControlHandle c;
  HParamBlockRec hpb;

  e = get_HParamBlockRec_from_ControlHandle (&hpb, (**g_selection)[0]);
  if (e == noErr)
    {
      vrefnum = hpb.volumeParam.ioVRefNum;
#ifdef THINK_C
      for (wp = WindowList; wp != 0; wp = nextwp)
	{
#else
      for (wp = LMGetWindowList (); wp != 0; wp = nextwp)
	{
#endif
	  nextwp = wp->nextWindow;
	  if ((WindowPtr) wp != g_hotband && wp->windowKind == userKind &&
	    (*(opendirinfo **) wp->refCon)->vrefnum == vrefnum)
	    {
	      disposedirwindow (wp);
	    }
	}
      pb.ioParam.ioVRefNum = vrefnum;
      pb.ioParam.ioNamePtr = 0;
      e = PBEject (&pb);
/* FIXME - shouldn't we complain if this fails? */
      if (e == noErr)
	e = UnmountVol (0, vrefnum);
      if (e == noErr)
	disable_menu_item (eject_menuid);
      resetvolumes ();
    }
}
