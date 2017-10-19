/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if defined (powerpc) || defined (__ppc__)

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_interfacelib[] =
	    "$Id: interfacelib.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>

#include "rsys/common.h"
#include "rsys/interfacelib.h"

#include "rsys/cfm.h"
#include "rsys/pef.h"
#include "rsys/file.h"

#include "ppc_stubs.h"


#include "SegmentLdr.h"
#include "DialogMgr.h"
#include "FontMgr.h"
#include "MenuMgr.h"
#include "TextEdit.h"
#include "OSUtil.h"
#include "ToolboxEvent.h"
#include "CQuickDraw.h"
#include "AliasMgr.h"
#include "MemoryMgr.h"
#include "Gestalt.h"
#include "ToolboxUtil.h"
#include "BinaryDecimal.h"
#include "AppleEvents.h"
#include "QuickDraw.h"
#include "ListMgr.h"
#include "DeskMgr.h"
#include "StdFilePkg.h"
#include "PrintMgr.h"
#include "FileMgr.h"
#include "ControlMgr.h"
#include "MenuMgr.h"
#include "ScrapMgr.h"
#include "SoundMgr.h"
#include "TimeMgr.h"
#include "Iconutil.h"
#include "ResourceMgr.h"
#include "HelpMgr.h"
#include "ScriptMgr.h"
#include "OSEvent.h"
#include "ProcessMgr.h"
#include "EditionMgr.h"
#include "Finder.h"
#include "ADB.h"
#include "Disk.h"
#include "DiskInit.h"
#include "Package.h"
#include "ThinkC.h"
#include "ShutDown.h"

#include "rsys/time.h"
#include "rsys/mixed_mode.h"
#include "rsys/executor.h"
#include "rsys/interfacelib.h"
#include "rsys/mathlib.h"
#include "rsys/launch.h"
#include "rsys/emustubs.h"
#include "rsys/mixed_mode.h"

using namespace Executor;

#if 0 /* set of routines that are known to have point trouble */

Routines that actually return points:

LMGetMouseTemp
LMGetRawMouseLocation
LMGetMouseLocation

Routines that have points as params:

LMSetMouseTemp
LMSetRawMouseLocation
LMSetMouseLocation
C_PtInIconSuite (Point test_pt,
C_PtInIconID (Point test_pt, const Rect *rect,
C_PtInIconMethod (Point test_pt, const Rect *rect,
C_PrLine( Point p );
C_PrTxMeas( INTEGER n, Ptr p, Point *nump,
C_PrText( INTEGER n, Ptr textbufp, Point num,
C_SndRecord(ProcPtr filterp, Point corner,
C_SndRecordToFile(ProcPtr filterp, Point corner,
C_DeltaPoint( Point a, Point b );
C_PinRect( Rect *r, Point p );
C_DragTheRgn( RgnHandle rgn, Point startp, 
#endif

/* here are some low-memory global accessors */

PRIVATE int32
LMGetLastSPExtra (void)
{
  warning_trace_info (NULL_STRING);
  return CW (LastSPExtra);
}

PRIVATE GDHandle
LMGetTheGDevice (void)
{
  warning_trace_info (NULL_STRING);
  return MR (TheGDevice);
}

PRIVATE Ptr
LMGetROMBase (void)
{
  warning_trace_info (NULL_STRING);
  return MR (ROMBase);
}

PRIVATE Handle
LMGetMenuList (void)
{
  warning_trace_info (NULL_STRING);
  return MR (MenuList);
}

PRIVATE INTEGER
LMGetResErr (void)
{
  warning_trace_info (NULL_STRING);
  return CW (ResErr);
}

PRIVATE INTEGER
LMGetPrintErr (void)
{
  warning_trace_info (NULL_STRING);
  return CW (PrintErr);
}

PRIVATE Ptr
LMGetWidthPtr (void)
{
  warning_trace_info (NULL_STRING);
  return (Ptr) MR (WidthPtr);
}

PRIVATE uint32
LMGetCaretTime (void)
{
  warning_trace_info (NULL_STRING);
  return CL (CaretTime);
}

#if 0
PRIVATE Handle
LMGetQDColors (void)
{
  warning_trace_info (NULL_STRING);
  return MR (QDColors);
}
#endif

PRIVATE Ptr
LMGetDefVCBPtr (void)
{
  warning_trace_info (NULL_STRING);
  return (Ptr) MR (DefVCBPtr);
}

PRIVATE INTEGER
LMGetAtMenuBottom (void)
{
  warning_trace_info (NULL_STRING);
  return CW (AtMenuBottom);
}

PRIVATE uint8
LMGetSdVolume (void)
{
  warning_trace_info (NULL_STRING);
  return CB (SdVolume);
}

#if 1
typedef void *DragGrayRgnUPP;
#endif

PRIVATE DragGrayRgnUPP
LMGetDragHook (void)
{
  warning_trace_info (NULL_STRING);
  return MR (DragHook);
}

PRIVATE Handle
LMGetWidthListHand (void)
{
  warning_trace_info (NULL_STRING);
  return MR (WidthListHand);
}

PRIVATE INTEGER
LMGetTopMenuItem (void)
{
  warning_trace_info (NULL_STRING);
  return CW (TopMenuItem);
}

PRIVATE uint32
LMGetDoubleTime (void)
{
  warning_trace_info (NULL_STRING);
  return CL (DoubleTime);
}

PRIVATE StringHandle
LMGetDAStrings (short which)
{
  warning_trace_info (NULL_STRING);
  return (StringHandle) MR (DAStrings_H[which].p);
}

PRIVATE GDHandle
LMGetMainDevice (void)
{
  warning_trace_info (NULL_STRING);
  return MR (MainDevice);
}

PRIVATE Handle
LMGetWidthTabHandle (void)
{
  warning_trace_info (NULL_STRING);
  return (Handle) MR (WidthTabHandle);
}

PRIVATE int16
LMGetROM85 (void)
{
  warning_trace_info (NULL_STRING);
  return CW (ROM85);
}

PRIVATE uint8
LMGetKbdType (void)
{
  warning_trace_info (NULL_STRING);
  return CB (KbdType);
}

PRIVATE INTEGER
LMGetScrHRes (void)
{
  warning_trace_info (NULL_STRING);
  return CW (ScrHRes);
}

PRIVATE StringPtr
LMGetCurApName (void)
{
  warning_trace_info (NULL_STRING);
  return CurApName; /* don't swap: is an array */
}

PRIVATE INTEGER
LMGetSysMap (void)
{
  warning_trace_info (NULL_STRING);
  return CW (SysMap);
}

PRIVATE THz
LMGetTheZone (void)
{
  warning_trace_info (NULL_STRING);
  return MR (TheZone);
}

PRIVATE RgnHandle
LMGetGrayRgn (void)
{
  warning_trace_info (NULL_STRING);
  return MR (GrayRgn);
}

PRIVATE QHdrPtr
LMGetEventQueue (void)
{
  warning_trace_info (NULL_STRING);
  return &EventQueue;
}

PRIVATE THz
LMGetApplZone (void)
{
  warning_trace_info (NULL_STRING);
  return MR (ApplZone);
}

PRIVATE uint32
LMGetTicks (void)
{
  warning_trace_info (NULL_STRING);
  return CL (Ticks);
}

PRIVATE uint8
LMGetResLoad (void)
{
  warning_trace_info (NULL_STRING);
  return CB (ResLoad);
}

PRIVATE StringPtr
LMGetFinderName (void)
{
  warning_trace_info (NULL_STRING);
  return FinderName;
}

PRIVATE Ptr
LMGetApplLimit (void)
{
  warning_trace_info (NULL_STRING);
  return MR (ApplLimit);
}

PRIVATE Ptr
LMGetHeapEnd (void)
{
  warning_trace_info (NULL_STRING);
  return MR (HeapEnd);
}

PRIVATE INTEGER
LMGetMBarHeight (void)
{
  warning_trace_info (NULL_STRING);
  return CW (MBarHeight);
}

PRIVATE Handle
LMGetTopMapHndl (void)
{
  warning_trace_info (NULL_STRING);
  return MR (TopMapHndl);
}

PRIVATE INTEGER
LMGetScrVRes (void)
{
  warning_trace_info (NULL_STRING);
  return CW (ScrVRes);
}

PRIVATE void
LMSetMenuDisable (int32 val)
{
  warning_trace_info (NULL_STRING);
  MenuDisable = CL (val);
}

PRIVATE void
LMSetAtMenuBottom (INTEGER val)
{
  warning_trace_info (NULL_STRING);
  AtMenuBottom = CW (val);
}

PRIVATE void
LMSetTopMenuItem (INTEGER val)
{
  warning_trace_info (NULL_STRING);
  TopMenuItem = CW (val);
}

PRIVATE void
LMSetSFSaveDisk (INTEGER val)
{
  warning_trace_info (NULL_STRING);
  SFSaveDisk = CW (val);
}

PRIVATE void
LMSetTheZone (THz val)
{
  warning_trace_info (NULL_STRING);
  TheZone = RM (val);
}

PRIVATE void
LMSetApplZone (THz val)
{
  warning_trace_info (NULL_STRING);
  ApplZone = RM (val);
}

PRIVATE void
LMSetResLoad (uint8 val)
{
  warning_trace_info (NULL_STRING);
  ResLoad = CB (val);
}

PRIVATE void
LMSetApplLimit (Ptr val)
{
  warning_trace_info (NULL_STRING);
  ApplLimit = RM (val);
}

PRIVATE void
LMSetResErr (INTEGER val)
{
  warning_trace_info (NULL_STRING);
  ResErr = CW (val);
}

PRIVATE void
LMSetHeapEnd (Ptr val)
{
  warning_trace_info (NULL_STRING);
  HeapEnd = RM (val);
}

PRIVATE void
LMSetCurDirStore (INTEGER val)
{
  warning_trace_info (NULL_STRING);
  CurDirStore = CW (val);
}

PRIVATE INTEGER
LMGetCurApRefNum (void)
{
  warning_trace_info (NULL_STRING);
  return CW (CurApRefNum);
}

PRIVATE LONGINT
LMGetCurDirStore (void)
{
  warning_trace_info (NULL_STRING);
  return CL (CurDirStore);
}

PRIVATE uint8
LMGetCrsrBusy (void)
{
  warning_trace_info (NULL_STRING);
  return CB (CrsrBusy);
}

PRIVATE INTEGER
LMGetSysFontSize (void)
{
  warning_trace_info (NULL_STRING);
  return CW (SysFontSiz);
}

PRIVATE INTEGER
GetDefFontSize (void)
{
  INTEGER retval;

  retval = LMGetSysFontSize ();
  if (!retval)
    retval = 12;
  warning_trace_info ("%d", retval);
  return retval;
}

PRIVATE INTEGER
LMGetSFSaveDisk (void)
{
  warning_trace_info (NULL_STRING);
  return CW (SFSaveDisk);
}

PRIVATE INTEGER
LMGetSysFontFam (void)
{
  warning_trace_info (NULL_STRING);
  return CW (SysFontFam);
}

PRIVATE Handle
LMGetGZRootHnd (void)
{
  warning_trace_info (NULL_STRING);
  return MR (GZRootHnd);
}

PRIVATE void 
LMSetSysFontFam (INTEGER val)
{
  warning_trace_info (NULL_STRING);
  SysFontFam = CW (val);
}

PRIVATE void
LMSetSysFontSize (INTEGER val)
{
  warning_trace_info (NULL_STRING);
  SysFontSiz = CW (val);
}

PRIVATE WindowPtr
LMGetCurActivate (void)
{
  warning_trace_info (NULL_STRING);
  return MR (CurActivate);
}

PRIVATE INTEGER
LMGetCurJTOffset (void)
{
  warning_trace_info (NULL_STRING);
  return CW (CurJTOffset);
}

PRIVATE INTEGER
LMGetCurMap (void)
{
  warning_trace_info (NULL_STRING);
  return CW (CurMap);
}

PRIVATE Ptr
LMGetCurStackBase (void)
{
  warning_trace_info (NULL_STRING);
  return MR (CurStackBase);
}

PRIVATE INTEGER
LMGetFSFCBLen (void)
{
  warning_trace_info (NULL_STRING);
  return CW (FSFCBLen);
}

PRIVATE Handle
LMGetGZMoveHnd (void)
{
  warning_trace_info (NULL_STRING);
  warning_unimplemented (NULL_STRING);
  return (Handle) CLC (-1);
}

PRIVATE WindowPtr
LMGetGhostWindow (void)
{
  warning_trace_info (NULL_STRING);
  return MR (GhostWindow);
}

PRIVATE INTEGER
LMGetHWCfgFlags (void)
{
  warning_trace_info (NULL_STRING);
  return CW (SCSIFlags);
}

PRIVATE Byte
LMGetHiliteMode (void)
{
  warning_trace_info (NULL_STRING);
  return CB (HiliteMode);
}

PRIVATE void
LMGetHiliteRGB (RGBColor *rgbp)
{
  warning_trace_info (NULL_STRING);
  *rgbp = HiliteRGB;
}

PRIVATE INTEGER
LMGetKeyThresh (void)
{
  warning_trace_info (NULL_STRING);
  return CW (KeyThresh);
}

PRIVATE INTEGER
LMGetSysEvtMask (void)
{
  warning_trace_info (NULL_STRING);
  return CW (SysEvtMask);
}

PRIVATE INTEGER
LMGetTEScrpLength (void)
{
  warning_trace_info (NULL_STRING);
  return CW (TEScrpLength);
}

PRIVATE uint32
LMGetTime (void)
{
  warning_trace_info (NULL_STRING);
  GetDateTime (&Time);
  return  CL (Time);
}

PRIVATE WindowPeek
LMGetWindowList (void)
{
  warning_trace_info (NULL_STRING);
  return MR (WindowList);
}

PRIVATE INTEGER
LMGetTESysJust (void)
{
  warning_trace_info (NULL_STRING);
  return CW (TESysJust);
}

PRIVATE INTEGER
LMGetBootDrive (void)
{
  warning_trace_info (NULL_STRING);
  return CW (BootDrive);
}

PRIVATE Byte
LMGetFractEnable (void)
{
  warning_trace_info (NULL_STRING);
  return CB (FractEnable);
}

PRIVATE Ptr
LMGetRAMBase (void)
{
  warning_trace_info (NULL_STRING);
  warning_unimplemented (NULL_STRING);
  return (Ptr) MR (SysZone);
}

PRIVATE Ptr
LMGetBufPtr (void)
{
  warning_trace_info (NULL_STRING);
  return MR (BufPtr);
}

PRIVATE void
LMSetTESysJust (INTEGER val)
{
  warning_trace_info (NULL_STRING);
  TESysJust = CW (val);
}

PRIVATE void
LMSetCurActivate (WindowPtr val)
{
  warning_trace_info (NULL_STRING);
  CurActivate = RM (val);
}
  
PRIVATE void
LMSetCurDeactive (WindowPtr val)
{
  warning_trace_info (NULL_STRING);
  CurDeactive = RM (val);
}

PRIVATE void
LMSetCurMap (INTEGER val)
{
  warning_trace_info (NULL_STRING);
  CurMap = CW (val);
}

PRIVATE void
LMSetGrayRgn (RgnHandle val)
{
  warning_trace_info (NULL_STRING);
  GrayRgn = RM (val);
}

PRIVATE void
LMSetHiliteMode (Byte val)
{
  warning_trace_info (NULL_STRING);
  HiliteMode = CB (val);
}

PRIVATE void
LMSetLastSPExtra (LONGINT val)
{
  warning_trace_info (NULL_STRING);
  warning_unimplemented ("val = 0x%x", val);
  LastSPExtra = CL (val);
}

PRIVATE void
LMSetMBarHeight (INTEGER val)
{
  warning_trace_info (NULL_STRING);
  MBarHeight = CW (val);
}

PRIVATE void
LMSetROMMapInsert (Byte val)
{
  warning_trace_info (NULL_STRING);
  warning_unimplemented (NULL_STRING);
  RomMapInsert = CB (val);
}

PRIVATE void
LMSetTEScrpLength (INTEGER val)
{
  warning_trace_info (NULL_STRING);
  TEScrpLength = CW (val);
}

PRIVATE void
LMSetTopMapHndl (Handle val)
{
  warning_trace_info (NULL_STRING);
  TopMapHndl = RM (val);
}

PRIVATE void
LMSetFractEnable (Byte value)
{
  warning_trace_info ("%d", value);
  FractEnable = CB (value);
}

/* provide some routines we don't normally use in ROMlib */

PUBLIC char *
ROMlib_p2cstr (StringPtr str)
{
  int len;

  len = str[0];
  memmove (str, str+1, len);
  str[len] = 0;
  warning_trace_info ("%s", str);
  return str;
}

PRIVATE StringPtr
c2pstr (char *str)
{
  int len;

  warning_trace_info (NULL_STRING);
  len = strlen (str);
  memmove (str+1, str, len);
  str[0] = len;
  return str;
}

/* wrap FrontWindow because we know Illustrator patches it out */
PRIVATE WindowPtr
FrontWindow_wrapper (void)
{
  WindowPtr retval;

  warning_trace_info (NULL_STRING);
  retval = FrontWindow ();
  return retval;
}
 
/* wrap functions around #defines */

PRIVATE Size
MaxMemSys_wrapper (Size *grow)
{
  Size retval;

  retval = MaxMemSys (grow);
  warning_trace_info ("%d", retval);
  return retval;
}

PRIVATE Size
MaxMem_wrapper (Size *grow)
{
  Size retval;

  retval = MaxMem (grow);
  warning_trace_info ("%d", retval);
  return retval;
}

PRIVATE Ptr
NewPtr_wrapper (Size s)
{
  Ptr retval;

  retval = NewPtr (s);
  warning_trace_info ("%p(%d)", retval, s);
  return retval;
}

PRIVATE Size
CompactMem_wrapper (Size s)
{
  warning_trace_info (NULL_STRING);
  return CompactMem (s);
}

PRIVATE Ptr
NewPtrClear_wrapper (Size s)
{
  warning_trace_info (NULL_STRING);
  return NewPtrClear (s);
}

PRIVATE Ptr
NewPtrSys_wrapper (Size s)
{
  warning_trace_info (NULL_STRING);
  return NewPtrSys (s);
}

PRIVATE Ptr
NewPtrSysClear_wrapper (Size s)
{
  warning_trace_info (NULL_STRING);
  return NewPtrSysClear (s);
}

PRIVATE Handle
NewHandleClear_wrapper (Size s)
{
  warning_trace_info (NULL_STRING);
  return NewHandleClear (s);
}

PRIVATE Handle
NewHandleClearSys_wrapper (Size s)
{
  warning_trace_info (NULL_STRING);
  return NewHandleSysClear (s);
}

PRIVATE Handle
NewHandle_wrapper (Size s)
{
  Handle retval;

  retval = NewHandle (s);
  warning_trace_info ("%p", retval);
  return retval;
}

PRIVATE Handle
NewHandleSys_wrapper (Size s)
{
  warning_trace_info (NULL_STRING);
  return NewHandleSys (s);
}

PRIVATE void
PurgeMem_wrapper (Size needed)
{
  warning_trace_info ("%d", needed);
  PurgeMem (needed);
}

PRIVATE OSErr
PBFlushVolSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBFlushVol (pb, FALSE);
}

PRIVATE OSErr
PBHSetVolSync (WDPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHSetVol (pb, FALSE);
}

PRIVATE OSErr
PBFlushFileSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBFlushFile (pb, FALSE);
}

PRIVATE OSErr
PBCatSearchAsync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBCatSearch (pb, TRUE);
}

PRIVATE OSErr
PBLockRangeSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBLockRange (pb, FALSE);
}

PRIVATE OSErr
PBUnlockRangeSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBUnlockRange (pb, FALSE);
}

PRIVATE OSErr
PBDTGetIconSync (DTPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBDTGetIcon (pb, FALSE);
}

PRIVATE OSErr
PBDTGetIconInfoSync (DTPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBDTGetIconInfo (pb, FALSE);
}

PRIVATE OSErr
PBHRstFLockSync (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHRstFLock (pb, FALSE);
}

PRIVATE OSErr
PBHSetFLockSync (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHSetFLock (pb, FALSE);
}

PRIVATE OSErr
PBCloseWDSync (WDPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBCloseWD (pb, FALSE);
}

PRIVATE OSErr
CloseWD (INTEGER wdref)
{
  OSErr retval;
  WDPBRec pb;

  memset (&pb, 0, sizeof pb);
  pb.ioVRefNum = CW (wdref);
  retval = PBCloseWD (&pb, FALSE);
  warning_trace_info ("retval = %d", retval);
  return retval;
}

PRIVATE OSErr
PBHGetVInfoSync (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHGetVInfo (pb, FALSE);
}

PRIVATE OSErr
PBGetWDInfoSync (WDPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBGetWDInfo (pb, FALSE);
}

PRIVATE OSErr
PBAllocateSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBAllocate (pb, FALSE);
}

PRIVATE OSErr
PBAllocContigSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBAllocContig (pb, FALSE);
}

PRIVATE OSErr
PBGetEOFSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBGetEOF (pb, FALSE);
}

PRIVATE OSErr
HRstFLock (INTEGER vref, LONGINT dirid, Str255 file)
{
  OSErr retval;
  HParamBlockRec pb;

  memset (&pb, 0, sizeof pb);
  pb.ioParam.ioVRefNum = CW (vref);
  pb.fileParam.ioDirID = CL (dirid);
  pb.ioParam.ioNamePtr = file;
  warning_trace_info (NULL_STRING);
  retval = PBHRstFLock (&pb, FALSE);
  return retval;
}

PRIVATE OSErr
HDelete (INTEGER vrefnum, LONGINT dirid, Str255 filename)
{
  OSErr retval;
  HParamBlockRec pb;

  memset (&pb, 0, sizeof pb);
  pb.ioParam.ioVRefNum = CW (vrefnum);
  pb.fileParam.ioDirID = CL (dirid);
  pb.ioParam.ioNamePtr = filename;
  retval = PBHDelete (&pb, FALSE);
  warning_trace_info ("HDelete(%d, %d, %.*s) = %d", vrefnum, dirid,
		      filename[0], filename+1, retval);
  return retval;
}

PRIVATE OSErr
HOpenDF (INTEGER vref, LONGINT dirid, Str255 file, int8 perm, INTEGER *refp)
{
  OSErr retval;
  HParamBlockRec pb;

  memset (&pb, 0, sizeof pb);
  pb.ioParam.ioVRefNum = CW (vref);
  pb.fileParam.ioDirID = CL (dirid);
  pb.ioParam.ioNamePtr = file;
  pb.ioParam.ioPermssn = perm;
  warning_trace_info (NULL_STRING);
  retval = PBOpenDF (&pb, FALSE);
  if (retval == noErr)
    *refp = pb.ioParam.ioRefNum;
  return retval;
}

/* nops */

PRIVATE void
DisableIdle_nop (void)
{
  warning_trace_info (NULL_STRING);
}

PRIVATE void
EnableIdle_nop (void)
{
  warning_trace_info (NULL_STRING);
}

/* routines that pass points by value */

PRIVATE LONGINT
PinRect_PC (Rect *r, PointAsLong pal)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  return PinRect (r, p);
}

PRIVATE void
AddPt_PC (PointAsLong src, Point *dst)
{
  Point p;

  p.h = src;
  p.v = src >> 16;
  warning_trace_info (NULL_STRING);
  AddPt (p, dst);
}

PRIVATE void
SubPt_PC (PointAsLong src, Point *dst)
{
  Point p;

  p.h = src;
  p.v = src >> 16;
  warning_trace_info (NULL_STRING);
  SubPt (p, dst);
}

PRIVATE Boolean
IsOutline_PC (PointAsLong numerAL, PointAsLong denomAL)
{
  Point numer;
  Point denom;

  numer.h = numerAL;
  numer.v = numerAL >> 16;
  denom.h = denomAL;
  denom.v = denomAL >> 16;
  warning_trace_info (NULL_STRING);
  return IsOutline (numer, denom);
}

PRIVATE INTEGER
DIBadMount_PC (PointAsLong pal, LONGINT evtmess)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  return C_DIBadMount (p, evtmess);
}

PRIVATE INTEGER
TrackControl_PC (ControlHandle c, PointAsLong pal, ProcPtr a)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  return TrackControl (c, p, a);
}

PRIVATE void
DragWindow_PC (WindowPtr wp, PointAsLong pal, Rect *rp)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  DragWindow (wp, p, rp);
}

PRIVATE INTEGER
FindWindow_PC (PointAsLong pal, GUEST<WindowPtr> *wpp)
{
  Point p;
  INTEGER retval;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info ("%d %d", p.h, p.v);
  retval = FindWindow (p, wpp);
  return retval;
}

PRIVATE LONGINT
MenuSelect_PC (PointAsLong pal)
{
  Point p;
  LONGINT retval;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info ("%d %d", p.h, p.v);
  retval = MenuSelect (p);
  warning_trace_info ("%d", retval);
  return retval;
}

PRIVATE INTEGER
FindControl_PC (PointAsLong pal, WindowPtr w, GUEST<ControlHandle> *cp)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  return FindControl (p, w, cp);
}

PRIVATE BOOLEAN
PtInRect_PC (PointAsLong pal, Rect *r)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info ("%d %d", p.h, p.v);
  return PtInRect (p, r);
} 

PRIVATE void
SFPPutFile_PC (PointAsLong pal, StringPtr prompt, StringPtr name, ProcPtr dh,
	       SFReply *rep, INTEGER dig, ProcPtr fp)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  SFPPutFile (p, prompt, name, dh, rep, dig, fp);
}

PRIVATE void
SFGetFile_PC (PointAsLong pal, StringPtr prompt, ProcPtr filef, INTEGER numt,
	      SFTypeList tl, ProcPtr dh, SFReply *rep)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  SFGetFile (p, prompt, filef, numt, tl, dh, rep);
}

PRIVATE void
SFPutFile_PC (PointAsLong pal, StringPtr prompt, StringPtr name, ProcPtr dh,
	      SFReply *rep)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  SFPutFile (p, prompt, name, dh, rep);
}

PUBLIC void
SFPGetFile_SYSV4 (PointAsLong pal, StringPtr prompt, ProcPtr filef,
		  INTEGER numt, SFTypeList tl, ProcPtr dh, SFReply *rep,
		  const sfpgetfile_aixtosysv4 *pbp)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  SFPGetFile (p, prompt, filef, numt, tl, dh, rep, pbp->dig, pbp->fp);
}

PUBLIC OSErr
HMShowMenuBalloon_SYSV4 (INTEGER item, INTEGER menuid, LONGINT flags,
			 LONGINT itemreserved, PointAsLong pal,
			 RectPtr alternaterectp, Ptr tipproc,
			 const hmshowmenuballoon_aixtosysv4 *argp)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  return C_HMShowMenuBalloon (item, menuid, flags, itemreserved, p,
			      alternaterectp, tipproc, argp->proc,
			      argp->variant);
}

PUBLIC OSErr
HMGetIndHelpMsg_SYSV4 (ResType type, INTEGER resid, INTEGER msg,
		       INTEGER state, LONGINT *options, PointAsLong pal,
		       Rect *altrectp, const hmgetindhelpmsg_aixtosysv4 *pbp)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  return HMGetIndHelpMsg (type, resid, msg, state, options, p, altrectp,
			  pbp->theprocp, pbp->variantp, pbp->helpmsgp,
			  pbp->count);
}

PUBLIC ControlHandle
NewControl_SYSV4 (WindowPtr wst, Rect *r, StringPtr title, BOOLEAN vis,
		  INTEGER value, INTEGER min, INTEGER max,
		  const newcontrol_aixtosysv4 *pbp)
{
  return NewControl (wst, r, title, vis, value, min, max, pbp->procid,
		     pbp->rc);
}

PUBLIC CDialogPtr
NewCDialog_SYSV4 (Ptr p, Rect *rp, StringPtr sp, BOOLEAN b1, INTEGER i,
		  WindowPtr wp, BOOLEAN b2, const newcdialog_aixtosysv4 *pbp)
{
  return NewCDialog (p, rp, sp, b1, i, wp, b2, pbp->l, pbp->h);
}

PUBLIC DialogPtr
NewDialog_SYSV4 (Ptr dst, Rect *r, StringPtr tit, BOOLEAN vis, INTEGER procid,
		 WindowPtr behind, BOOLEAN gaflag,
		 const newdialog_aixtosysv4 *pbp)
{
  return NewDialog (dst, r, tit, vis, procid, behind, gaflag, pbp->rc,
		    pbp->items);
}

PUBLIC OSErr
OutlineMetrics_SYSV4 (int16 byte_count, Ptr text, PointAsLong numerAL,
		      PointAsLong denomAL, int16 *y_max, int16 *y_min,
		      Fixed *aw_array, const outlinemetrics_aixtosysv4 *pbp)
{
  Point numer;
  Point denom;

  numer.h = numerAL;
  numer.v = numerAL >> 16;
  denom.h = denomAL;
  denom.v = denomAL >> 16;

  return OutlineMetrics (byte_count, text, numer, denom, y_max, y_min,
			 aw_array, pbp->lsb_array, pbp->bounds_array);
}

PUBLIC ListHandle
LNew_SYSV4 (Rect *rview, Rect *bounds, PointAsLong pal, INTEGER proc,
	    WindowPtr wind, BOOLEAN draw, BOOLEAN grow,
	    const lnew_aixtosysv4 *pbp)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  return LNew (rview, bounds, p, proc, wind, draw, grow, pbp->scrollh, 
	       pbp->scrollv);
}

PUBLIC INTEGER
PixelToChar_SYSV4 (Ptr textBuf, LONGINT textLen, Fixed slop,
		   Fixed pixelWidth, BOOLEAN *leadingEdgep,
		   Fixed *widthRemainingp, JustStyleCode styleRunPosition,
		   const pixeltochar_aixtosysv4 *pbp)
{
  return C_PixelToChar (textBuf, textLen, slop, pixelWidth, leadingEdgep,
			widthRemainingp, styleRunPosition, pbp->numer,
			pbp->denom);
}

PUBLIC void
CustomPutFile_SYSV4 (Str255 prompt, Str255 defaultName,
		     StandardFileReply *replyp, INTEGER dlgid, PointAsLong pal,
		     DlgHookYDProcPtr dlghook,
		     ModalFilterYDProcPtr filterproc,
		     const customputfile_aixtosysv4 *pbp)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  C_CustomPutFile (prompt, defaultName, replyp, dlgid, p, dlghook, filterproc,
		   pbp->activeList, pbp->activateproc, pbp->yourdatap);
}

PUBLIC void
CustomGetFile_SYSV4 (FileFilterYDProcPtr filefilter, INTEGER numtypes,
		     SFTypeList typelist, StandardFileReply *replyp,
		     INTEGER dlgid, PointAsLong pal, DlgHookYDProcPtr dlghook,
		     const customgetfile_aixtosysv4 *pbp)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  C_CustomGetFile (filefilter, numtypes, typelist, replyp, dlgid, p, dlghook,
		   pbp->filterproc, pbp->activeList, pbp->activateproc,
		   pbp->yourdatap);
}

PRIVATE LONGINT
GrowWindow_PC (WindowPtr w, PointAsLong pal, Rect *rp)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  return GrowWindow (w, p, rp);
}

PRIVATE BOOLEAN
TrackGoAway_PC (WindowPtr w, PointAsLong pal)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  return TrackGoAway (w, p);
}

PRIVATE BOOLEAN
LClick_PC (PointAsLong pal, INTEGER mods, ListHandle list)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  return LClick (p, mods, list);
}

PRIVATE BOOLEAN
GetColor_PC (PointAsLong pal, Str255 prompt, RGBColor *in, RGBColor *out)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  return GetColor (p, prompt, in, out);
}

PRIVATE BOOLEAN
EqualPt_PC (PointAsLong pal1, PointAsLong pal2)
{
  Point p1, p2;

  p1.h = pal1;
  p1.v = pal1 >> 16;
  p2.h = pal2;
  p2.v = pal2 >> 16;
  warning_trace_info (NULL_STRING);
  return EqualPt (p1, p2);
}

PRIVATE void
ShieldCursor_PC (Rect *rp, PointAsLong pal)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  ShieldCursor (rp, p);
}

PRIVATE void
PtToAngle_PC (Rect *rp, PointAsLong pal, INTEGER *angle)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  PtToAngle (rp, p, angle);
}

PRIVATE void
TEClick_PC (PointAsLong pal, BOOLEAN ext, TEHandle teh)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  TEClick (p, ext, teh);
}

PRIVATE void
Pt2Rect_PC (PointAsLong pal1, PointAsLong pal2, Rect *dest)
{
  Point p1, p2;

  p1.h = pal1;
  p1.v = pal1 >> 16;
  p2.h = pal2;
  p2.v = pal2 >> 16;
  warning_trace_info (NULL_STRING);
  Pt2Rect (p1, p2, dest);
}

PRIVATE LONGINT
DragGrayRgn_PC (RgnHandle rgn, PointAsLong pal, Rect *limit, Rect *slop,
		INTEGER axis, ProcPtr proc)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  return DragGrayRgn (rgn, p, limit, slop, axis, proc);
}

PRIVATE BOOLEAN
PtInRgn_PC (PointAsLong pal, RgnHandle rh)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  return PtInRgn (p, rh);
}

PRIVATE OSErr
HMShowBalloon_PC (HMMessageRecord *msgp, PointAsLong pal,
		  RectPtr alternaterectp, Ptr tipprocptr, INTEGER proc,
		  INTEGER variant, INTEGER method)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  return C_HMShowBalloon (msgp, p, alternaterectp, tipprocptr, proc,
			  variant, method);
}

PRIVATE INTEGER
TestControl_PC (ControlHandle c, PointAsLong pal)
{
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  warning_trace_info (NULL_STRING);
  return TestControl (c, p);
}

PRIVATE void
TEGetPoint_PC (Point *retvalp, INTEGER offset, TEHandle teh)
{
  PointAsLong pal;
  Point p;

  warning_trace_info ("returning Point");
  pal = TEGetPoint (offset, teh);
  p.h = pal;
  p.v = pal >> 16;
  *retvalp = p;
}


PRIVATE INTEGER
TEGetOffset_PC (PointAsLong pal, TEHandle te)
{
  INTEGER retval;
  Point p;

  p.h = pal;
  p.v = pal >> 16;
  retval = TEGetOffset (p, te);
  return retval;
}

PRIVATE void
StdLine_PC (PointAsLong pal)
{
  Point p;

  warning_trace_info (NULL_STRING);
  p.h = pal;
  p.v = pal >> 16;
  StdLine (p);
}

PRIVATE void
StdText_PC (INTEGER n, Ptr textbufp, PointAsLong numAL, PointAsLong denAL)
{
  Point num;
  Point den;

  warning_trace_info (NULL_STRING);
  num.h = numAL;
  num.v = numAL >> 16;
  den.h = denAL;
  den.v = denAL >> 16;
  StdText (n, textbufp, num, den);
}

typedef uint32 CellAsLong;

PRIVATE void
LFind_PC (INTEGER *offsetp, INTEGER *lenp, CellAsLong cellAL, ListHandle list)
{
  Cell cell;

  warning_trace_info (NULL_STRING);
  cell.h = cellAL;
  cell.v = cellAL >> 16;
  LFind (offsetp, lenp, cell, list);
}

PRIVATE void
LRect_PC (Rect *cellrect, CellAsLong cellAL, ListHandle list)
{
  Cell cell;

  warning_trace_info (NULL_STRING);
  cell.h = cellAL;
  cell.v = cellAL >> 16;
  LRect (cellrect, cell, list);
}

PRIVATE void
LDraw_PC (CellAsLong cellAL, ListHandle list)
{
  Cell cell;

  warning_trace_info (NULL_STRING);
  cell.h = cellAL;
  cell.v = cellAL >> 16;
  LDraw (cell, list);
}

PRIVATE void
LSetSelect_PC (BOOLEAN setit, CellAsLong cellAL, ListHandle list)
{
  Cell cell;

  warning_trace_info (NULL_STRING);
  cell.h = cellAL;
  cell.v = cellAL >> 16;
  LSetSelect (setit, cell, list);
}

PRIVATE void
LAddToCell_PC (Ptr dp, INTEGER dl, CellAsLong cellAL, ListHandle list)
{
  Cell cell;

  warning_trace_info (NULL_STRING);
  cell.h = cellAL;
  cell.v = cellAL >> 16;
  LAddToCell (dp, dl, cell, list);
}

PRIVATE void
LClrCell_PC (CellAsLong cellAL, ListHandle list)
{
  Cell cell;

  warning_trace_info (NULL_STRING);
  cell.h = cellAL;
  cell.v = cellAL >> 16;
  LClrCell (cell, list);
}

PRIVATE void
LGetCell_PC (Ptr dp, INTEGER *dlp, CellAsLong cellAL, ListHandle list)
{
  Cell cell;

  warning_trace_info (NULL_STRING);
  cell.h = cellAL;
  cell.v = cellAL >> 16;
  LGetCell (dp, dlp, cell, list);
}

PRIVATE void
LSetCell_PC (Ptr dp, INTEGER dl, CellAsLong cellAL, ListHandle list)
{
  Cell cell;

  warning_trace_info (NULL_STRING);
  cell.h = cellAL;
  cell.v = cellAL >> 16;
  LSetCell (dp, dl, cell, list);
}

PRIVATE void
LCellSize_PC (PointAsLong csizeAL, ListHandle list)
{
  Point p;

  warning_trace_info (NULL_STRING);
  p.h = csizeAL;
  p.v = csizeAL >> 16;
  LCellSize (p, list);
}

PRIVATE BOOLEAN
TrackBox_PC (WindowPtr wp, PointAsLong ptAL, INTEGER part)
{
  Point p;
  BOOLEAN retval;

  warning_trace_info (NULL_STRING);
  p.h = ptAL;
  p.v = ptAL >> 16;
  retval = TrackBox (wp, p, part);
  return retval;
}

/* Some stubs that are very likely to give us all sorts of trouble */

typedef int32 OSStatus;

PRIVATE OSStatus
RegisterAppearanceClient_stub (void)
{
  warning_trace_info (NULL_STRING);
  return noErr;
}

PRIVATE void
LowerText_stub (unsigned char *p, INTEGER len)
{
  while (len-- > 0)
    {
      *p = tolower (*p);
      ++p;
    }
  warning_trace_info (NULL_STRING);
}

PRIVATE void
UpperText_stub (unsigned char *p, INTEGER len)
{
  while (len-- > 0)
    {
      *p = toupper (*p);
      ++p;
    }
  warning_trace_info (NULL_STRING);
}

typedef SignedByte TrapType;

enum
{
  kOSTrapType,
  kToolboxTrapType,
};

// #warning we should really merge this with what is in emustubs.c

PRIVATE void
NSetTrapAddress_stub (UniversalProcPtr addr, uint16 trapnum, TrapType typ)
{
  switch (typ)
    {
    case kOSTrapType:
      trapnum &= 0xff;
      ostraptable [trapnum] = RM (addr);
      break;
    case kToolboxTrapType:
      trapnum &= 0x3ff;
      tooltraptable [trapnum] = RM (addr); 
      break;
    default:
      warning_unexpected ("%d", typ);
      break;
    }
  warning_trace_info (NULL_STRING);
}

PRIVATE void
SetOSTrapAddress (UniversalProcPtr addr, uint16 trapnum)
{
  warning_trace_info (NULL_STRING);
  NSetTrapAddress_stub (addr, trapnum, kOSTrapType);
}

PRIVATE UniversalProcPtr
GetToolTrapAddress (uint16 trap_no)
{
  UniversalProcPtr retval;
  uint32 d0, a0;

  d0 = trap_no;
  ROMlib_GetTrapAddress_helper (&d0, 0xA746, &a0);
  retval = (UniversalProcPtr) a0;
  warning_trace_info ("trap = 0x%x, retval = %p", trap_no, retval);
  return retval;
}

PRIVATE UniversalProcPtr
GetOSTrapAddress (uint16 trap_no)
{
  UniversalProcPtr retval;
  uint32 d0, a0;

  d0 = trap_no;
  ROMlib_GetTrapAddress_helper (&d0, 0xA346, &a0);
  retval = (UniversalProcPtr) a0;
  warning_trace_info ("trap = 0x%x, retval = %p", trap_no, retval);
  return retval;
}

PRIVATE long
SetA5 (long val)
{
  long retval;

  retval = (long) SYN68K_TO_US_CHECK0 (EM_A5);
  EM_A5 = US_TO_SYN68K_CHECK0 (val);
  warning_trace_info ("old = 0x%lx new = 0x%lx", retval, val);
  return retval;
}

PRIVATE int
count_and_reverse_args (uint32 *infop)
{
  uint32 info_in;
  uint32 info_out;
  int retval;

  info_in = *infop;
  info_out = 0;
  for (retval = 0; info_in; ++retval, info_in >>= kStackParameterWidth)
    {
      info_out <<= kStackParameterWidth;
      info_out |= info_in & ((1<<kStackParameterWidth)-1);
    }
  *infop = info_out;
  return retval;
}

PRIVATE int
extract_stack_parameters (ProcInfoType info, va_list ap,
			  uint32 params[13], int widths[13],
			  where_args_t where)
{
  int n_params;
  int convention;
  uint32 *argp;
  int *widp;
  
  int incr;

  n_params = 0;
  convention = info & ((1 << kCallingConventionWidth)-1);
  info >>= (kCallingConventionWidth + kResultSizeWidth);
  argp = params;
  incr = 1;
  switch (convention)
    {
    default:
      warning_unexpected ("%d", convention);
      /* FALL THROUGH */
    case kCStackBased:
    case kThinkCStackBased:
    case kD0DispatchedCStackBased:
      break;
    case kPascalStackBased:
    case kD0DispatchedPascalStackBased:
    case kD1DispatchedPascalStackBased:
    case kStackDispatchedPascalStackBased:
      if (where == args_via_68k_stack)
	{
	  argp = params + count_and_reverse_args (&info) - 1;
	  incr = -1;
	}
      break;
    }
  widp = widths + (argp - params);

  while (info)  
    {
      int width;

      width = info & ((1 <<kStackParameterWidth) -1);
      switch (width)
	{
	case kOneByteCode:
	  if (where == args_via_stdarg)
	    *argp = (uint8) va_arg (ap, uint32);
	  else
	    *argp = POPUB ();
	  *widp = 1;
	  break;
	case kTwoByteCode:
	  if (where == args_via_stdarg)
	    *argp = (uint16) va_arg (ap, uint32);
	  else
	    *argp = POPUW ();
	  *widp = 2;
	  break;
	default:
	  warning_unexpected ("%d", width);
	  /* FALL THROUGH */
	case kFourByteCode:
	  if (where == args_via_stdarg)
	    *argp = (uint32) va_arg (ap, uint32);
	  else
	    *argp = POPUL ();
	  *widp = 4;
	  break;
	}
      info >>= kStackParameterWidth;
      argp += incr;
      widp += incr;
      ++n_params;
    }    

  return n_params;
}

PRIVATE int
extract_68k_reg_parameters (ProcInfoType orig_info,  uint32 params[13],
			    int widths[13])
{
  int retval;
  uint32 info;
  int i;
  boolean_t seen_a_zero_p;
  boolean_t suspicious_info_p;

  retval = 0;
  seen_a_zero_p = FALSE;
  suspicious_info_p = FALSE;
  info = orig_info >> 11;
  for (i = 0; i < 4; ++i)
    {
      int width;
      int reg;

      width = info & 3;
      info >>= 2;
      reg = info & 7;
      info >>= 3;
      widths[i] = width;
      if (width == 0)
	{
	  seen_a_zero_p = TRUE;
	  if (reg)
	    suspicious_info_p = TRUE;
	  params[i] = 0;
	}
      else
	{
	  uint32 reg_contents;
	  static int map[] =
	  {    0, /* kRegisterD0,  0 */
	       1, /* kRegisterD1,  1 */
	       2, /* kRegisterD2,  2 */
	       3, /* kRegisterD3,  3 */
	       8, /* kRegisterA0,  4 */
	       9, /* kRegisterA1,  5 */
	      10, /* kRegisterA2,  6 */
	      11, /* kRegisterA3,  7 */
	  };

	  reg_contents = cpu_state.regs[map[reg]].ul.n;
	  retval = i + 1;
	  if (seen_a_zero_p)
	    suspicious_info_p = TRUE;
	  switch (width)
	    {
	    case 1:
	      params[i] = reg_contents & 0xFF;
	      break;
	    case 2:
	      params[i] = reg_contents & 0xFFFF;
	      break;
	    case 3:
	      params[i] = reg_contents;
	      break;
	    }
	}
    }
  if (suspicious_info_p)
    warning_unexpected ("info = 0x%x", orig_info);
  return retval;
}

PRIVATE long
Call68KProc_from_native (void *addr, ProcInfoType info, va_list ap)
{
  long retval;
  uint32 params[13];
  uint32 *argp;
  int incr;
  int widths[13];
  int n_params;
  int convention;
  int return_width;
  enum { on_stack, reg_d0 } return_location;
  
  n_params = extract_stack_parameters (info, ap, params, widths,
				       args_via_stdarg);

  retval = 0;
  convention = info & 0xf;
  info >>= 4;
  return_width = info & 0x3;
  if (return_width)
    return_width = (1 << (return_width-1));
  info >>= 2;

  if (convention == kPascalStackBased ||
      convention == kD0DispatchedPascalStackBased ||
      convention == kD1DispatchedPascalStackBased ||
      convention == kStackDispatchedPascalStackBased)
    {
      argp = params;
      incr = 1;
      return_location = on_stack;
    }
  else
    {
      int count;

      count = count_and_reverse_args (&info);
      if (count != n_params)
	warning_unexpected ("%d %d", count, n_params);
      argp = params + count - 1;
      incr = -1;
      return_location = reg_d0;
    }

  if (return_location == on_stack)
    {
      switch (return_width)
	{
	case 0:
	  break;
	case 1:
	case 2:
	  EM_A7 -= 2;
	  break;
	case 4:
	  EM_A7 -= 4;
	  break;
	}
    }

  while (info)
    {
      uint32 param_width;

      param_width = (info & 0x3);
      switch (param_width)
	{
	case kNoByteCode:
	  warning_unexpected ("0x%x", info);
	  break;
	case kOneByteCode:
	  {
	    uint8 b;
	    
	    b = *argp;
	    PUSHUB (b);
	  }
	  break;
	case kTwoByteCode:
	  {
	    uint16 w;

	    w = *argp;
	    PUSHUW (w);
	  }
	  break;
	case kFourByteCode:
	  {
	    uint32 l;
		
	    l = *argp;
	    PUSHUL (l);
	  }
	  break;
	}
      info >>= 2;
      argp += incr;
    }
  CALL_EMULATOR (US_TO_SYN68K (addr));
  if (return_location == reg_d0)
    {
      switch (return_width)
	{
	case 0:
	  retval = 0;
	  break;
	case 1:
	  retval = (uint8) EM_D0;
	  break;
	case 2:
	  retval = (uint16) EM_D0;
	  break;
	case 4:
	  retval = EM_D0;
	  break;
	}
    }
  else if (return_location == on_stack)
    {
      switch (return_width)
	{
	case 0:
	  retval = 0;
	  break;
	case 1:
	  retval = POPUB ();
	  break;
	case 2:
	  retval = POPUW ();
	  break;
	case 4:
	  retval = POPUL ();
	  break;
	}
    }
  else
    warning_unexpected ("%d", return_location);
  return retval;
}

PUBLIC long
CallUniversalProc_from_native_common (va_list ap, where_args_t where,
				      ProcPtr proc, ProcInfoType info)
{
  long retval;
  uint32 params[13];
  int widths[13];
  int n_params;

// #warning really need to test to see that we have a matching routine record
  //  warning_unimplemented ("need to look more carefully here"); 

  if (where == args_via_68k_regs)
    n_params = extract_68k_reg_parameters (info, params, widths);
  else
    n_params = extract_stack_parameters (info, ap, params, widths, where);
  {
    uint32 *transition_vectorp;
    register uint32 toc asm ("r2");

    transition_vectorp = (uint32 *) proc;
    toc = transition_vectorp[1];
    asm volatile ("" : : "r" (toc));
    switch (n_params)
      {
      case 0:
	{
	  long (*funcp)(void);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp ();
	}
	break;
      case 1:
	{
	  long (*funcp)(uint32);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp (params[0]);
	}
	break;
      case 2:
	{
	  long (*funcp)(uint32, uint32);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp (params[0], params[1]);
	}
	break;
      case 3:
	{
	  long (*funcp)(uint32, uint32, uint32);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp (params[0], params[1], params[2]);
	}
	break;
      case 4:
	{
	  long (*funcp)(uint32, uint32, uint32, uint32);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp (params[0], params[1], params[2], params[3]);
	}
	break;
      case 5:
	{
	  long (*funcp)(uint32, uint32, uint32, uint32, uint32);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp (params[0], params[1], params [2], params [3],
			  params[4]);
	}
	break;
      case 6:
	{
	  long (*funcp)(uint32, uint32, uint32, uint32, uint32, uint32);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp (params[0], params[1], params [2], params [3],
			  params[4], params[5]);
	}
	break;
      case 7:
	{
	  long (*funcp)(uint32, uint32, uint32, uint32, uint32, uint32,
			uint32);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp (params[0], params[1], params [2], params [3],
			  params[4], params[5], params [6]);
	}
	break;
      case 8:
	{
	  long (*funcp)(uint32, uint32, uint32, uint32, uint32, uint32,
			uint32, uint32);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp (params[0], params[1], params [2], params [3],
			  params[4], params[5], params [6], params [7]);
	}
	break;
      case 9:
	{
	  long (*funcp)(uint32, uint32, uint32, uint32, uint32, uint32,
			uint32, uint32, uint32);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp (params[0], params[1], params [2], params [3],
			  params[4], params[5], params [6], params [7],
			  params[8]);
	}
	break;
      case 10:
	{
	  long (*funcp)(uint32, uint32, uint32, uint32, uint32, uint32,
			uint32, uint32, uint32, uint32);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp (params[0], params[1], params [2], params [3],
			  params[4], params[5], params [6], params [7],
			  params[8], params[9]);
	}
	break;
      case 11:
	{
	  long (*funcp)(uint32, uint32, uint32, uint32, uint32, uint32,
			uint32, uint32, uint32, uint32, uint32);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp (params[0], params[1], params [2], params [3],
			  params[4], params[5], params [6], params [7],
			  params[8], params[9], params[10]);
	}
	break;
      case 12:
	{
	  long (*funcp)(uint32, uint32, uint32, uint32, uint32, uint32,
			uint32, uint32, uint32, uint32, uint32, uint32);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp (params[0], params[1], params [2], params [3],
			  params[4], params[5], params [6], params [7],
			  params[8], params[9], params[10], params[11]);
	}
	break;
      case 13:
	{
	  long (*funcp)(uint32, uint32, uint32, uint32, uint32, uint32,
			uint32, uint32, uint32, uint32, uint32, uint32,
			uint32);

	  funcp = (typeof (funcp)) transition_vectorp[0];
	  retval = funcp (params[0], params[1], params [2], params [3],
			  params[4], params[5], params [6], params [7],
			  params[8], params[9], params[10], params[11],
			  params[12]);
	}
	break;
      default:
	fprintf (stderr, "blowing off native call (%d)\n", n_params);
	retval = 0;
	break;
      }
    }
  return retval;
}

PUBLIC long
CallUniversalProc_from_native (UniversalProcPtr proc, ProcInfoType info, ...)
{
  va_list ap;
  long retval;

  va_start (ap, info);

  warning_trace_info ("proc = %p, *proc = 0x%04x", proc, *(uint16 *)proc);
  if (proc->goMixedModeTrap != (uint16) CWC (MIXED_MODE_TRAP))
    retval = Call68KProc_from_native (proc, info, ap);
  else if (proc->routineRecords[0].ISA == CBC (kM68kISA))
    retval = Call68KProc_from_native (proc->routineRecords[0].procDescriptor,
				      info, ap);
  else
    retval = CallUniversalProc_from_native_common
                                (ap, args_via_stdarg,
			         MR (proc->routineRecords[0].procDescriptor),
				 info);
  va_end (ap);
  return retval;
  return retval;
}

PRIVATE void
microseconds (uint64_t *retp)
{
  warning_trace_info (NULL_STRING);
  *retp = (uint64_t) (uint32) msecs_elapsed () * 1000;
}

PRIVATE void
PurgeSpace_wrapper (uint32 *totalp, uint32 *contigp)
{
  PurgeSpace (totalp, contigp);
  warning_trace_info ("%d %d", *totalp, *contigp);
}

PRIVATE void
HLockHi_wrapper (Handle h)
{
  warning_trace_info (NULL_STRING);
  MoveHHi (h);
  HLock (h);
}

PRIVATE LONGINT
MaxBlock_wrapper (void)
{
  warning_trace_info (NULL_STRING);
  return MaxBlock ();
}

PRIVATE Handle
RecoverHandle_wrapper (Ptr p)
{
  warning_trace_info (NULL_STRING);
  return RecoverHandle (p);
}

PRIVATE uint32
FreeMem_wrapper (void)
{
  uint32 retval;

  retval = FreeMem ();
  warning_trace_info ("%d", (int32) retval);
  return retval;
}

PRIVATE uint32
FreeMemSys_wrapper (void)
{
  uint32 retval;

  retval = FreeMemSys ();
  warning_trace_info ("%d", (int32) retval);
  return retval;
}

PRIVATE void
ReserveMem_wrapper (Size size)
{
  warning_trace_info (NULL_STRING);
  ResrvMem (size);
}

PRIVATE uint32
SetCurrentA5 (void)
{
  uint32 retval;

  retval = (uint32) SYN68K_TO_US (EM_A5);
  EM_A5 = (uint32) CL (CurrentA5);
  warning_trace_info ("old = 0x%x, new = 0x%x", retval, EM_A5);
  return retval;
}

PRIVATE OSErr
PBCloseSync_wrapper (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBClose (pb, FALSE);
}

PRIVATE OSErr
PBDeleteSync_wrapper (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBDelete (pb, FALSE);
}

PRIVATE OSErr
PBExchangeFilesSync_wrapper (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBExchangeFiles (pb, FALSE);
}

PRIVATE OSErr
PBGetCatInfoSync_wrapper (CInfoPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBGetCatInfo (pb, FALSE);
}

PRIVATE OSErr
PBGetFCBInfoSync_wrapper (FCBPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBGetFCBInfo (pb, FALSE);
}

PRIVATE OSErr
PBGetFCBInfoAsync_wrapper (FCBPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBGetFCBInfo (pb, TRUE);
}

PRIVATE OSErr
PBGetVInfoSync_wrapper (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBGetVInfo (pb, FALSE);
}

PRIVATE OSErr
PBHCreateSync_wrapper (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHCreate (pb, FALSE);
}

PRIVATE OSErr
PBGetFInfoSync_wrapper (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBGetFInfo (pb, FALSE);
}

PRIVATE OSErr
PBHGetFInfoSync_wrapper (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHGetFInfo (pb, FALSE);
}

PRIVATE OSErr
PBHGetFInfoAsync_wrapper (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHGetFInfo (pb, TRUE);
}

PRIVATE OSErr
PBHGetVolParmsSync_wrapper (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHGetVolParms (pb, FALSE);
}

PRIVATE OSErr
PBHGetVolSync_wrapper (WDPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHGetVol (pb, FALSE);
}

PRIVATE OSErr
PBHOpenSync_wrapper (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHOpen (pb, FALSE);
}

PRIVATE OSErr
PBHSetFInfoSync_wrapper (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHSetFInfo (pb, FALSE);
}

PRIVATE OSErr
PBSetFInfoSync_wrapper (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBSetFInfo (pb, FALSE);
}

PRIVATE OSErr
OpenWD (INTEGER vref, LONGINT dirid, LONGINT procid, INTEGER *wdrefp)
{
  WDPBRec wdpb;
  OSErr retval;

  memset (&wdpb, 0, sizeof wdpb);
  wdpb.ioVRefNum = CW (vref);
  wdpb.ioWDDirID = CL (dirid);
  wdpb.ioWDProcID = CL (procid);
  retval = PBOpenWD (&wdpb, FALSE);
  if (retval == noErr)
    *wdrefp = wdpb.ioVRefNum;
  warning_trace_info (NULL_STRING);
  return retval;
}

PRIVATE OSErr
DirCreate (INTEGER vref, LONGINT parid, Str255 dirname, LONGINT *outdir)
{
  OSErr retval;
  HParamBlockRec hpb;

  memset (&hpb, 0, sizeof hpb);
  hpb.ioParam.ioVRefNum = CW (vref);
  hpb.fileParam.ioDirID = CL (parid);
  hpb.ioParam.ioNamePtr = RM ((Ptr) dirname);
  retval = PBDirCreate (&hpb, FALSE);
  if (retval == noErr)
    *outdir = hpb.fileParam.ioDirID;
  warning_trace_info (NULL_STRING);
  return retval;
}

PRIVATE OSErr
PBOpenWDSync_wrapper (WDPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBOpenWD (pb, FALSE);
}

PRIVATE OSErr
PBReadSync_wrapper (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBRead (pb, FALSE);
}

PRIVATE OSErr
PBReadAsync_wrapper (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBRead (pb, TRUE);
}

PRIVATE OSErr
PBRenameSync_wrapper (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBRename (pb, FALSE);
}

PRIVATE OSErr
PBSetCatInfoSync_wrapper (CInfoPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBSetCatInfo (pb, FALSE);
}

PRIVATE OSErr
PBSetEOFSync_wrapper (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBSetEOF (pb, FALSE);
}

PRIVATE OSErr
PBStatusSync_wrapper (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBStatus (pb, FALSE);
}

PRIVATE OSErr
PBWriteSync_wrapper (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBWrite (pb, FALSE);
}

PRIVATE OSErr
PBWriteAsync_wrapper (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBWrite (pb, TRUE);
}

PRIVATE OSErr
PBDTAddAPPLSync (DTPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBDTAddAPPL (pb, FALSE);
}

PRIVATE OSErr
PBDTGetAPPLSync (DTPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBDTGetAPPL (pb, FALSE);
}

PRIVATE OSErr
PBDirCreateSync (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBDirCreate (pb, FALSE);
}

PRIVATE OSErr
PBGetFPosSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBGetFPos (pb, FALSE);
}

PRIVATE OSErr
PBSetFPosSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBSetFPos (pb, FALSE);
}

PRIVATE OSErr
PBDTAddIconSync (DTPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBDTAddIcon (pb, FALSE);
}

PRIVATE OSErr
PBDTSetCommentSync (DTPBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBDTSetComment (pb, FALSE);
}

PRIVATE OSErr
PBSetVolSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBSetVol (pb, FALSE);
}

PRIVATE OSErr
PBSetVInfoSync (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBSetVInfo (pb, FALSE);
}

PRIVATE OSErr
PBHDeleteSync (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHDelete (pb, FALSE);
}

PRIVATE OSErr
PBCatSearchSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBCatSearch (pb, FALSE);
}

PRIVATE OSErr
PBCatMoveSync (CMovePBPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBCatMove (pb, FALSE);
}

PRIVATE OSErr
PBHRenameSync (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHRename (pb, FALSE);
}

PRIVATE OSErr
PBOpenSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBOpen (pb, FALSE);
}

PRIVATE OSErr
PBHOpenDFSync (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBOpenDF (pb, FALSE);
}

PRIVATE OSErr
PBHOpenRFSync (HParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBHOpenRF (pb, FALSE);
}

PRIVATE OSErr
PBOpenRFSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBOpenRF (pb, FALSE);
}

PRIVATE OSErr
PBControlSync (ParmBlkPtr pb)
{
  warning_trace_info (NULL_STRING);
  return PBControl (pb, FALSE);
}

PRIVATE OSErr
PBHOpenDenySync (HParmBlkPtr pb)
{
  OSErr retval;

  retval = OpenDeny (pb, FALSE);
  warning_trace_info ("%d", retval);
  return retval;
}

PRIVATE OSErr
HGetVol (StringPtr volName, INTEGER *vrefp, LONGINT *diridp)
{
  OSErr retval;
  WDPBRec wdp;

  wdp.ioNamePtr = RM (volName);
  retval = PBHGetVol (&wdp, FALSE);
  if (retval == noErr)
    {
      *vrefp = wdp.ioVRefNum;
      *diridp = wdp.ioWDDirID;
    }
  warning_trace_info ("retval = %d", retval);
  return retval;
}

PRIVATE OSErr
HSetVol (StringPtr volName, INTEGER vref, LONGINT dirid)
{
  OSErr retval;
  WDPBRec wdp;

  wdp.ioNamePtr = RM (volName);
  wdp.ioVRefNum = CW (vref);
  wdp.ioWDDirID = CL (dirid);
  retval = PBHSetVol (&wdp, FALSE);
  warning_trace_info ("retval = %d", retval);
  return retval;
}

PRIVATE OSErr
HOpen (INTEGER vref, LONGINT dirid, Str255 filename, SignedByte perm,
       INTEGER *refp)
{
  OSErr retval;
  HParamBlockRec hpb;

  hpb.ioParam.ioVRefNum = CW (vref);
  hpb.fileParam.ioDirID = CL (dirid);
  hpb.ioParam.ioNamePtr = RM (filename);
  hpb.ioParam.ioPermssn = CB (perm);
  retval = PBHOpen (&hpb, FALSE);
  if (retval == noErr)
    *refp = hpb.ioParam.ioRefNum;

  warning_trace_info ("retval = %d, filename = '%.*s'", retval, filename[0],
		      filename+1);
  return retval;
}

PRIVATE OSErr
HSetFInfo (INTEGER vref, LONGINT dirid, Str255 filename, FInfo *finfop)
{
  OSErr retval;
  HParamBlockRec hpb;

  hpb.ioParam.ioVRefNum = CW (vref);
  hpb.fileParam.ioDirID = CL (dirid);
  hpb.ioParam.ioNamePtr = RM (filename);
  hpb.fileParam.ioFlFndrInfo = *finfop;
  retval = PBHSetFInfo (&hpb, FALSE);
  warning_trace_info ("retval = %d", retval);
  return retval;
}

/* AppleTalk stubs -- ick */

PRIVATE void
NBPSetEntity_stub (Ptr bufp, Str32 obj, Str32 typ, Str32 zone)
{
  *bufp = 0;
  warning_trace_info (NULL_STRING);
}

PRIVATE Boolean
IsMPPOpen (void)
{
  warning_trace_info (NULL_STRING);
  return FALSE;
}

PRIVATE Boolean
IsATPOpen (void)
{
  warning_trace_info (NULL_STRING);
  return FALSE;
}

PRIVATE OSErr
MPPOpen_stub (void)
{
  warning_trace_info (NULL_STRING);
  return paramErr;
}

/* Misc. PPC-only */

PRIVATE void
SetDialogFont (INTEGER font)
{
  DlgFont = CW (font);
  warning_trace_info (NULL_STRING);
}

/* Fixed point Math */

PRIVATE Fixed
X2Fix_wrapper (double x)
{
  Fixed retval;

  retval = X2Fix (&x);
  warning_trace_info ("x = %f, retval = 0x%x", x, retval);
  return retval;
}

PRIVATE void
MakeDataExecutable (void *p, uint32 len)
{
  warning_trace_info ("%p %d", p, len);
  warning_unimplemented ("need to call mprotect here");
#warning need to call mprotect here
}

PRIVATE map_entry_t
interfacelib_map[] =
{
  { "ExitToShell", C_ExitToShell, }, /* hello_world */
  { "InitCursor",  C_InitCursor, },
  { "InitDialogs", C_InitDialogs, },
  { "InitFonts",   C_InitFonts, },
  { "InitGraf",    C_InitGraf, },
  { "InitMenus",   C_InitMenus, },
  { "InitWindows", C_InitWindows, },
  { "NoteAlert",   C_NoteAlert, },
  { "TEInit",      C_TEInit, },
  { "InsetRect", C_InsetRect, },    /* silly_balls */
  { "SysBeep", C_SysBeep, },
  { "TextSize", C_TextSize, },
  { "MoveTo", C_MoveTo, },
  { "Button", C_Button, },
  { "GetDateTime", GetDateTime, },
  { "InvertColor", C_InvertColor, },
  { "SetPort", C_SetPort, },
  { "Random", C_Random, },
  { "NewCWindow", C_NewCWindow, },
  { "SetRect", C_SetRect, },
  { "SysEnvirons", SysEnvirons, },
  { "DrawString", C_DrawString, },
  { "RGBForeColor", C_RGBForeColor, },
  { "PaintOval", C_PaintOval, },
  { "PenNormal", C_PenNormal, }, /* StuffIt Expander */
  { "GetPort", C_GetPort, },
  { "FSClose", FSClose, },
  { "DisposeRoutineDescriptor", C_DisposeRoutineDescriptor, },
  { "StopAlert", C_StopAlert, },
  { "WritePartialResource", C_WritePartialResource, },
  //  { "CallComponentUPP", C_CallComponentUPP, },
  { "NMRemove", NMRemove, },
  { "CountTypes", C_CountTypes, },
  { "NGetTrapAddress", NGetTrapAddress, },
  { "MoveWindow", C_MoveWindow, },
  { "DragWindow", DragWindow_PC, },
  { "SetResLoad", C_SetResLoad, },
  { "NMInstall", NMInstall, },
  { "CautionAlert", C_CautionAlert, },
  { "FindFolder", C_FindFolder, },
  { "PBGetEOFSync", PBGetEOFSync, },
  { "DisposeHandle", DisposHandle, },
  { "UpdateDialog", C_UpdtDialog, },
  { "TestDeviceAttribute", C_TestDeviceAttribute, },
#if !defined (CFM_PROBLEMS)
  { "CloseConnection", C_CloseConnection, },
#endif
  { "GetControlReference", C_GetCRefCon, },
  { "RemoveResource", C_RmveResource, },
  { "EndUpdate", C_EndUpdate, },
  { "LMGetCurApName", LMGetCurApName, },
  { "ModalDialog", C_ModalDialog, },
  { "HiliteWindow", C_HiliteWindow, },
  { "HideControl", C_HideControl, },
  { "SetControlTitle", C_SetCTitle, },
  { "BeginUpdate", C_BeginUpdate, },
  { "SectRect", C_SectRect, },
  { "PBDTAddAPPLSync", PBDTAddAPPLSync, },
  { "AppendDITL", AppendDITL, },
  { "PBDTGetAPPLSync", PBDTGetAPPLSync, },
  { "LMSetSFSaveDisk", LMSetSFSaveDisk, },
  { "HOpenResFile", C_HOpenResFile, },
  { "UpdateResFile", C_UpdateResFile, },
  { "GetControlTitle", C_GetCTitle, },
  { "FindWindow", FindWindow_PC, },
  { "MenuSelect", MenuSelect_PC, },
  //  { "CloseComponent", C_CloseComponent, },
  { "PenSize", C_PenSize, },
  { "InvalRect", C_InvalRect, },
  { "GetIndType", C_GetIndType, },
  { "Create", Create, },
  { "p2cstr", ROMlib_p2cstr, },
  { "P2CStr", ROMlib_p2cstr, },
  { "HandleZone", HandleZone, },
  { "GetFPos", GetFPos, },
  { "ShowControl", C_ShowControl, },
  { "EventAvail", C_EventAvail, },
  { "WaitNextEvent", C_WaitNextEvent, },
  { "GetZone", GetZone, },
  { "PBDTGetPath", PBDTGetPath, },
  { "FlushEvents", FlushEvents, },
  { "GetDialogItem", C_GetDItem, },
  { "CountResources", C_CountResources, },
  { "LMGetSysMap", LMGetSysMap, },
  { "TempHLock", C_TempHLock, },
  { "TempHUnlock", C_TempHUnlock, },
  { "ResError", C_ResError, },
  { "TruncString", C_TruncString, },
  { "FreeMem", FreeMem_wrapper, },
  { "GetClip", C_GetClip, },
  { "SetDialogItem", C_SetDItem, },
  ///////  { "DebugStr", C_DebugStr, },
  { "PBDirCreateSync", PBDirCreateSync, },
  { "PaintBehind", C_PaintBehind, },
  { "NumToString", NumToString, },
  { "InitZone", InitZone, },
  { "HLockHi", HLockHi_wrapper, },
  { "Draw1Control", C_Draw1Control, },
  { "GetIndString", GetIndString, },
  { "TempDisposeHandle", C_TempDisposeHandle, },
  { "TempNewHandle", C_TempNewHandle, },
  { "ReleaseResource", C_ReleaseResource, },
  { "NewHandle", NewHandle_wrapper, },
  { "MoveHHi", MoveHHi, },
  { "DrawPicture", C_DrawPicture, },
  { "UseResFile", C_UseResFile, },
  //  { "AESend", C_AESend, },
  { "NewRgn", C_NewRgn, },
  { "PtrAndHand", PtrAndHand, },
  { "GetMainDevice", C_GetMainDevice, },
  { "GetNextDevice", C_GetNextDevice, },
  //  { "SameProcess", C_SameProcess, },
  { "PBGetFPosSync", PBGetFPosSync, },
  //  { "AERemoveSpecialHandler", C_AERemoveSpecialHandler, },
  { "GetResourceSizeOnDisk", C_SizeResource, },
  //  { "HoldMemory", C_HoldMemory, },
  { "CloseWD", CloseWD, },
  { "FindControl", FindControl_PC, },
  { "MemError", MemError, },
  { "NewAlias", C_NewAlias, },
  //  { "Delay", C_Delay, },
  { "GetKeys", C_GetKeys, },
  { "LMSetTheZone", LMSetTheZone, },
  { "FrontWindow", FrontWindow_wrapper, },
  //  { "DisposeZone", C_DisposeZone, },
  { "NewPtrSys", NewPtrSys_wrapper, },
  { "NewPtrSysClear", NewPtrSysClear_wrapper, },
  { "HRstFLock", HRstFLock, },
  //  { "AEInstallSpecialHandler", C_AEInstallSpecialHandler, },
  { "HCreate", HCreate, },
  { "SetControlMaximum", C_SetCtlMax, },
  { "NewPtr", NewPtr_wrapper, },
  { "LMGetTheZone", LMGetTheZone, },
  { "Alert", C_Alert, },
  { "NewControl", NewControl_AIX, },
  //  { "Allocate", C_Allocate, },
  { "PBSetFPosSync", PBSetFPosSync, },
  { "GetControlMaximum", C_GetCtlMax, },
  { "EqualString", EqualString, },
  { "LMGetGrayRgn", LMGetGrayRgn, },
  { "LMGetEventQueue", LMGetEventQueue, },
  //  { "GetScrap", C_GetScrap, },
  { "ShowWindow", C_ShowWindow, },
  { "LMGetApplZone", LMGetApplZone, },
  { "ClipRect", C_ClipRect, },
  { "LMGetTicks", LMGetTicks, },
  //  { "AEDisposeDesc", C_AEDisposeDesc, },
  { "NewRoutineDescriptor", C_NewRoutineDescriptor, },
  { "HOpenRF", HOpenRF, },
  //  { "FlushVol", C_FlushVol, },
  { "LMGetResLoad", LMGetResLoad, },
  { "HDelete", HDelete, },
  { "LMSetApplZone", LMSetApplZone, },
  //  { "GetCurrentProcess", C_GetCurrentProcess, },
  //  { "AECreateAppleEvent", C_AECreateAppleEvent, },
  { "LMGetFinderName", LMGetFinderName, },
  { "StringWidth", C_StringWidth, },
  { "HOpenDF", HOpenDF, },
  { "LMSetResLoad", LMSetResLoad, },
  { "DetachResource", C_DetachResource, },
  { "Get1Resource", C_Get1Resource, },
  //  { "OpenDefaultComponent", C_OpenDefaultComponent, },
  { "LMGetApplLimit", LMGetApplLimit, },
  { "GetWRefCon", C_GetWRefCon, },
  { "GetDialogItemText", C_GetIText, },
  { "RGBBackColor", C_RGBBackColor, },
  { "SetResourceSize", C_SetResourceSize, },
  { "SetCurrentA5", SetCurrentA5, },
  { "BlockMoveData", BlockMoveData, },
  //  { "AECreateDesc", C_AECreateDesc, },
  { "GetNewDialog", C_GetNewDialog, },
  { "ForeColor", C_ForeColor, },
  { "PBReadAsync", PBReadAsync_wrapper, },
  { "PBDTAddIconSync", PBDTAddIconSync, },
  { "PaintRect", C_PaintRect, },
  //  { "ReadLocation", C_ReadLocation, },
  { "GetToolTrapAddress", GetToolTrapAddress, },
  { "GetToolboxTrapAddress", GetToolTrapAddress, },
  { "CurResFile", C_CurResFile, },
  { "Gestalt", Gestalt, },
  //  { "FSRead", C_FSRead, },
  { "SelectDialogItemText", C_SelIText, },
  { "GetIndResource", C_GetIndResource, },
#if !defined (CFM_PROBLEMS)
  { "GetSharedLibrary", C_GetSharedLibrary, },
#endif
  //  { "SetEOF", C_SetEOF, },
  { "SystemZone", SystemZone, },
  { "GetWMgrPort", C_GetWMgrPort, },
  { "HGetState", HGetState, },
  { "HiliteControl", C_HiliteControl, },
  { "LMSetApplLimit", LMSetApplLimit, },
  { "GetBackColor", C_GetBackColor, },
  { "DisposeDialog", C_DisposDialog, },
  { "LMSetResErr", LMSetResErr, },
  { "PtInRect", PtInRect_PC, },
  { "PBDTSetCommentSync", PBDTSetCommentSync, },
  { "LocalToGlobal", C_LocalToGlobal, },
  //  { "CatMove", C_CatMove, },
  { "EraseRect", C_EraseRect, },
  { "HLock", HLock, },
  { "SetFPos", SetFPos, },
  { "GetScriptManagerVariable", C_GetEnvirons, },
  //  { "FSDelete", C_FSDelete, },
  //  { "AEGetSpecialHandler", C_AEGetSpecialHandler, },
  { "GetVRefNum", GetVRefNum, },
  //  { "GetFrontProcess", C_GetFrontProcess, },
  { "SetZone", SetZone, },
  { "GetOSTrapAddress", GetOSTrapAddress, },
  { "GetPicture", C_GetPicture, },
  //  { "UnholdMemory", C_UnholdMemory, },
  { "FrameRoundRect", C_FrameRoundRect, },
  { "GetMBarHeight", LMGetMBarHeight, },
  { "GetDeviceList", C_GetDeviceList, },
  { "EraseRoundRect", C_EraseRoundRect, },
  { "MaxBlock", MaxBlock_wrapper, },
  { "ShowDialogItem", C_ShowDItem, },
  { "FSMakeFSSpec", C_FSMakeFSSpec, },
  { "NewHandleSys", NewHandleSys_wrapper, },
  { "Count1Resources", C_Count1Resources, },
  { "Munger", C_Munger, },
  { "SFPPutFile", SFPPutFile_PC, },
  { "SetClip", C_SetClip, },
  //  { "FSWrite", C_FSWrite, },
  { "CloseResFile", C_CloseResFile, },
  { "PBSetVolSync", PBSetVolSync, },
  { "TickCount", C_TickCount, },
  { "FrameRect", C_FrameRect, },
  { "Get1IndResource", C_Get1IndResource, },
  //  { "AEProcessAppleEvent", C_AEProcessAppleEvent, },
  { "HNoPurge", HNoPurge, },
  { "GetProcessInformation", C_GetProcessInformation, },
  { "CustomPutFile", CustomPutFile_AIX, },
  { "SetWTitle", C_SetWTitle, },
  { "HideDialogItem", C_HideDItem, },
  { "HUnlock", HUnlock, },
  { "TempFreeMem", C_TempFreeMem, },
  //  { "GetWDInfo", C_GetWDInfo, },
  { "BackColor", C_BackColor, },
  { "DisposePtr", DisposPtr, },
  { "SetControlValue", C_SetCtlValue, },
  { "LMSetHeapEnd", LMSetHeapEnd, },
  { "HPurge", HPurge, },
  { "PenPat", C_PenPat, },
  { "DirCreate", DirCreate, },
  { "PBWriteAsync", PBWriteAsync_wrapper, },
  { "RecoverHandle", RecoverHandle_wrapper, },
  { "LMSetCurDirStore", LMSetCurDirStore, },
  { "GetHandleSize", GetHandleSize, },
  //  { "GetEOF", C_GetEOF, },
  { "AddResource", C_AddResource, },
  { "PBSetVInfoSync", PBSetVInfoSync, },
  { "GetResource", C_GetResource, },
  { "LMGetHeapEnd", LMGetHeapEnd, },
  { "HCreateResFile", C_HCreateResFile, },
  { "ChangedResource", C_ChangedResource, },
  { "GetForeColor", C_GetForeColor, },
  //  { "GetNextProcess", C_GetNextProcess, },
  { "TrackControl", TrackControl_PC, },
  { "NewPtrClear", NewPtrClear_wrapper, },
  //  { "HSetFLock", C_HSetFLock, },
  //  { "FSOpen", C_FSOpen, },
  { "ValidRect", C_ValidRect, },
  { "SelectWindow", C_SelectWindow, },
  { "SetDialogItemText", C_SetIText, },
  { "InlineGetHandleSize", GetHandleSize, },
  { "GetCursor", C_GetCursor, },
  { "LMGetMBarHeight", LMGetMBarHeight, },
  { "CallUniversalProc", CallUniversalProc_from_native, },
  { "LMGetTopMapHndl", LMGetTopMapHndl, },
  { "DisposeRgn", C_DisposeRgn, },
  { "SetHandleSize", SetHandleSize, },
  { "GlobalToLocal", C_GlobalToLocal, },
  { "SetCursor", C_SetCursor, },
  { "ParamText", C_ParamText, },
  { "SetA5", SetA5, },
  //  { "ActivateControl", C_ActivateControl, },
  //  { "SetKeyboardFocus", C_SetKeyboardFocus, },
  //  { "DeactivateControl", C_DeactivateControl, },
  //  { "GetDialogItemAsControl", C_GetDialogItemAsControl, },
  //  { "GetControlDataSize", C_GetControlDataSize, },
  //  { "HandleControlKey", C_HandleControlKey, },
  //  { "GetControlData", C_GetControlData, },
  //  { "SetControlData", C_SetControlData, },
  //  { "IdleControls", C_IdleControls, },
  //  { "GetRootControl", C_GetRootControl, },
  //  { "ConvertFromUnicodeToText", C_ConvertFromUnicodeToText, },
  //  { "ConvertFromTextToUnicode", C_ConvertFromTextToUnicode, },
  //  { "DisposeTextToUnicodeInfo", C_DisposeTextToUnicodeInfo, },
  //  { "DisposeUnicodeToTextInfo", C_DisposeUnicodeToTextInfo, },
  //  { "CreateTextToUnicodeInfo", C_CreateTextToUnicodeInfo, },
  //  { "CreateUnicodeToTextInfo", C_CreateUnicodeToTextInfo, },
  //  { "GetTextEncodingBase", C_GetTextEncodingBase, },
  //  { "CreateTextEncoding", C_CreateTextEncoding, },
  //  { "YieldToThread", C_YieldToThread, },
  { "MaxApplZone", MaxApplZone, }, /* Speedometer 4.02 */
  { "MoreMasters", MoreMasters, },
  { "GetGDevice", C_GetGDevice, },
  { "GetNewCWindow", C_GetNewCWindow, },
  { "ZoomWindow", C_ZoomWindow, },
  { "PmForeColor", C_PmForeColor, },
  { "DeleteMenu", C_DeleteMenu, },
  { "InsTime", InsTime, },
  { "VInstall", VInstall, },
  { "OpenRgn", C_OpenRgn, },
  { "AEInstallEventHandler", C_AEInstallEventHandler, },
  { "SetPt", C_SetPt, },
  { "HiliteMenu", C_HiliteMenu, },
  { "TextFont", C_TextFont, },
  { "SystemEdit", C_SystemEdit, },
  { "TEGetHeight", C_TEGetHeight, },
  { "DisposePixPat", C_DisposPixPat, },
  { "TEStyleNew", C_TEStylNew, },
  { "InvertRect", C_InvertRect, },
  { "PenMode", C_PenMode, },
  { "DrawMenuBar", C_DrawMenuBar, },
  { "DisableIdle", DisableIdle_nop, },
  { "TEInsert", C_TEInsert, },
  { "BlockMove", BlockMove, },
  { "EnableItem", C_EnableItem, },
  { "MenuKey", C_MenuKey, },
  { "MoveControl", C_MoveControl, },
  { "DrawControls", C_DrawControls, },
  { "CountMItems", C_CountMItems, },
  { "TextFace", C_TextFace, },
  { "PrOpen", C_PrOpen, },
  { "StandardGetFile", C_StandardGetFile, },
  { "PrClose", C_PrClose, },
  { "FSWrite", FSWrite, },
  { "SetDeviceAttribute", C_SetDeviceAttribute, },
  { "PrCloseDoc", C_PrCloseDoc, },
  { "TEActivate", C_TEActivate, },
  { "GetCIcon", C_GetCIcon, },
  { "HasDepth", C_HasDepth, },
  { "PrOpenDoc", C_PrOpenDoc, },
  { "LineTo", C_LineTo, },
  { "TEDispose", C_TEDispose, },
  { "TextMode", C_TextMode, },
  { "FSpOpenDF", C_FSpOpenDF, },
  { "SetGDevice", C_SetGDevice, },
  { "InsertMenu", C_InsertMenu, },
  { "MakeITable", C_MakeITable, },
  { "PBHGetVInfoSync", PBHGetVInfoSync, },
  { "GetString", C_GetString, },
  { "GetMenuItemText", C_GetItem, },
  { "OffsetRect", C_OffsetRect, },
  { "PrPicFile", C_PrPicFile, },
  { "GetIcon", C_GetIcon, },
  { "InsetRgn", C_InsetRgn, },
  { "DisposePixMap", C_DisposPixMap, },
  { "AppendMenu", C_AppendMenu, },
  { "GetControlValue", C_GetCtlValue, },
  { "SetDepth", C_SetDepth, },
  { "AECountItems", C_AECountItems, },
  { "FSDelete", FSDelete, },
  { "GetNewWindow", C_GetNewWindow, },
  { "NewHandleClear", NewHandleClear_wrapper, },
  { "NewHandleClearSys", NewHandleClearSys_wrapper, },
  { "SizeControl", C_SizeControl, },
  { "LMGetScrVRes", LMGetScrVRes, },
  { "TrackBox", TrackBox_PC, },
  { "SetOrigin", C_SetOrigin, },
  { "RelString", RelString, },
  { "FSRead", FSRead, },
  { "KillPicture", C_KillPicture, },
  { "SystemClick", C_SystemClick, },
  { "OpenPort", C_OpenPort, },
  { "TESetSelect", C_TESetSelect, },
  { "HideCursor", C_HideCursor, },
  { "SFGetFile", SFGetFile_PC, },
  { "PrintDefault", C_PrintDefault, },
  { "InsertMenuItem", C_InsMenuItem, },
  { "DisposeControl", C_DisposeControl, },
  { "Delay", Delay, },
  { "StillDown", C_StillDown, },
  { "CompactMem", CompactMem_wrapper, },
  { "ShowCursor", C_ShowCursor, },
  { "LLastClick", C_LLastClick, },
  { "GetMenu", C_GetMenu, },
  { "Get1NamedResource", C_Get1NamedResource, },
  { "PrimeTime", PrimeTime, },
  { "PlotCIcon", C_PlotCIcon, },
  { "AEProcessAppleEvent", C_AEProcessAppleEvent, },
  { "EnableIdle", EnableIdle_nop, },
  { "SetMenuItemText", C_SetItem, },
  { "AEGetNthPtr", C_AEGetNthPtr, },
  { "OpenRFPerm", C_OpenRFPerm, },
  { "OpenPicture", C_OpenPicture, },
  { "Unique1ID", C_Unique1ID, },
  { "OpenCPort", C_OpenCPort, },
  { "QDError", C_QDError, },
  { "TECalText", C_TECalText, },
  { "DisableItem", C_DisableItem, },
  { "AEGetParamDesc", C_AEGetKeyDesc, },
  { "GetTime", GetTime, },
  { "DisposeCTable", C_DisposCTable, },
  { "FrameRgn", C_FrameRgn, },
  { "FillRect", C_FillRect, },
  { "NewPixPat", C_NewPixPat, },
  { "DrawDialog", C_DrawDialog, },
  //  { "NewPtrClear", C_NewPtrClear, },
  { "GetMouse", C_GetMouse, },
  { "TEStyleInsert", C_TEStylInsert, },
  { "GetResInfo", C_GetResInfo, },
  { "PmBackColor", C_PmBackColor, },
  { "DrawGrowIcon", C_DrawGrowIcon, },
  { "PrOpenPage", C_PrOpenPage, },
  { "FSpGetFInfo", C_FSpGetFInfo, },
  { "LDispose", C_LDispose, },
  { "StringToNum", StringToNum, },
  { "LUpdate", C_LUpdate, },
  { "TEDeactivate", C_TEDeactivate, },
  { "DisposeCIcon", C_DisposeCIcon, },
  { "GetCWMgrPort", C_GetCWMgrPort, },
  { "DisposeWindow", C_DisposeWindow, },
  { "AppendResMenu", C_AddResMenu, },
  { "c2pstr", c2pstr, },
  { "C2PStr", c2pstr, },
  { "SizeWindow", C_SizeWindow, },
  { "PrClosePage", C_PrClosePage, },
  { "GrowWindow", GrowWindow_PC, },
  { "CloseRgn", C_CloseRgn, },
  { "HideWindow", C_HideWindow, },
  { "SetResInfo", C_SetResInfo, },
  { "SFPutFile", SFPutFile_PC, },
  { "LMGetScrHRes", LMGetScrHRes, },
  { "HSetState", HSetState, },
  { "SetItemStyle", C_SetItemStyle, },
  { "SystemTask", C_SystemTask, },
  { "PutScrap", C_PutScrap, },
  { "ClosePicture", C_ClosePicture, },
  { "CloseCPort", C_CloseCPort, },
  { "PBGetWDInfoSync", PBGetWDInfoSync, },
  { "PopUpMenuSelect", C_PopUpMenuSelect, },
  { "HMGetHelpMenuHandle", C_HMGetHelpMenuHandle, },
  { "TEScroll", C_TEScroll, },
  { "BitTst", C_BitTst, },
  { "SetWRefCon", C_SetWRefCon, },
  { "TEAutoView", C_TEAutoView, },
  { "TETextBox", C_TextBox, },
  { "CheckItem", C_CheckItem, },
  { "PrStlDialog", C_PrStlDialog, },
  { "ZeroScrap", C_ZeroScrap, },
  { "GetMaxDevice", C_GetMaxDevice, },
  { "MakeRGBPat", C_MakeRGBPat, },
  { "TrackGoAway", TrackGoAway_PC, },
  { "DisposeGDevice", C_DisposGDevice, },
  { "CopyBits", C_CopyBits, },
  { "FrameOval", C_FrameOval, },
  { "VRemove", VRemove, },
  { "LClick", LClick_PC, },
  { "SndPlay", C_SndPlay, },
  { "SFPGetFile", SFPGetFile_AIX, },
  { "HandToHand", HandToHand, },
  { "PrJobDialog", C_PrJobDialog, },
  { "TEDelete", C_TEDelete, },
  { "TECopy", C_TECopy, },
  { "FSOpen", FSOpen, },
  { "PBAllocateSync", PBAllocateSync, },
  { "FillCRect", C_FillCRect, },
  { "RmvTime", RmvTime, },
  { "LNew", LNew_AIX, },
  { "TENew", C_TENew, },
  { "FSpOpenResFile", C_FSpOpenResFile, },
  { "PlotIcon", C_PlotIcon, },
  { "WriteResource", C_WriteResource, },
  { "GetVol", GetVol, },
  { "PrError", C_PrError, },
  { "NewPixMap", C_NewPixMap, },
  { "GetCTable", C_GetCTable, },
  { "PBAllocContigSync", PBAllocContigSync, },
  { "Microseconds", microseconds, },
  { "TEUpdate", C_TEUpdate, },
  { "GetNextEvent", C_GetNextEvent, },
  { "OpenDeskAcc", C_OpenDeskAcc, },
  { "DrawText", C_DrawText, }, /* mathtest */
  { "GetItemMark", C_GetItemMark, }, /* Illustrator 5.5 */
  { "AllocContig", AllocContig, },
  { "FlushVol", FlushVol, },
  { "GetEOF", GetEOF, },
  { "GetWDInfo", GetWDInfo, },
  { "HGetFInfo", HGetFInfo, },
  { "OpenWD", OpenWD, },
  { "PBCloseSync", PBCloseSync_wrapper, },
  { "PBDeleteSync", PBDeleteSync_wrapper, },
  { "PBExchangeFilesSync", PBExchangeFilesSync_wrapper, },
  { "PBGetCatInfoSync", PBGetCatInfoSync_wrapper, },
  { "PBGetFCBInfoSync", PBGetFCBInfoSync_wrapper, },
  { "PBGetFCBInfoAsync", PBGetFCBInfoAsync_wrapper, },
  { "PBGetVInfoSync", PBGetVInfoSync_wrapper, },
  { "PBHCreateSync", PBHCreateSync_wrapper, },
  { "PBHGetFInfoSync", PBHGetFInfoSync_wrapper, },
  { "PBHGetFInfoAsync", PBHGetFInfoAsync_wrapper, },
  { "PBGetFInfoSync", PBGetFInfoSync_wrapper, },
  { "PBHGetVolParmsSync", PBHGetVolParmsSync_wrapper, },
  { "PBHGetVolSync", PBHGetVolSync_wrapper, },
  { "PBHOpenSync", PBHOpenSync_wrapper, },
  { "PBHSetFInfoSync", PBHSetFInfoSync_wrapper, },
  { "PBSetFInfoSync", PBSetFInfoSync_wrapper, },
  { "PBOpenWDSync", PBOpenWDSync_wrapper, },
  { "PBReadSync", PBReadSync_wrapper, },
  { "PBRenameSync", PBRenameSync_wrapper, },
  { "PBSetCatInfoSync", PBSetCatInfoSync_wrapper, },
  { "PBSetEOFSync", PBSetEOFSync_wrapper, },
  { "PBStatusSync", PBStatusSync_wrapper, },
  { "PBWriteSync", PBWriteSync_wrapper, },
  { "SetEOF", SetEOF, },
  { "SetFInfo", SetFInfo, },
  { "SetVol", SetVol, },
  { "EditionHasFormat", C_EditionHasFormat, },
  { "SectRgn", C_SectRgn, },
  { "HomeResFile", C_HomeResFile, },
  { "DisposeGWorld", C_DisposeGWorld, },
  { "GetNextProcess", C_GetNextProcess, },
  { "ClosePoly", C_ClosePoly, },
  { "LGetSelect", C_LGetSelect, },
  { "GetItemStyle", C_GetItemStyle, },
  { "PurgeSpace", PurgeSpace_wrapper, },
  { "InvertPoly", C_InvertPoly, },
  { "IUMagString", C_IUMagString, },
  { "ApplicationZone", LMGetApplZone, },
  { "PrSetError", C_PrSetError, },
  { "AEGetParamPtr", C_AEGetKeyPtr, },
  { "GetLastEditionContainerUsed", C_GetLastEditionContainerUsed, },
  { "NewWindow", C_NewWindow, },
  { "GetScript", C_GetScript, },
  { "CreateResFile", C_CreateResFile, },
  { "CopyRgn", C_CopyRgn, },
  { "LowerText", LowerText_stub, },
  { "NewMenu", C_NewMenu, },
  { "LMGetCurDirStore", LMGetCurDirStore, },
  { "GetScrap", C_GetScrap, },
  { "LMGetCrsrBusy", LMGetCrsrBusy, },
  { "InvertRgn", C_InvertRgn, },
  { "FracSin", C_FracSin, },
  { "AEInteractWithUser", C_AEInteractWithUser, },
  { "SetPortBits", C_SetPortBits, },
  { "GetColor", GetColor_PC, },
  { "GetAliasInfo", C_GetAliasInfo, },
  { "FixDiv", C_FixDiv, },
  //  { "GetNodeAddress", C_GetNodeAddress, },
  { "KillPoly", C_KillPoly, },
  { "SetEventMask", SetEventMask, },
  { "AEPutDesc", C_AEPutDesc, },
  { "ReadEdition", C_ReadEdition, },
  { "FixMul", C_FixMul, },
  { "FontToScript", C_Font2Script, },
  { "GetApplLimit", LMGetApplLimit, },
  //  { "NBPSetNTE", C_NBPSetNTE, },
  { "FixRatio", C_FixRatio, },
  { "ObscureCursor", C_ObscureCursor, },
  { "ClosePort", C_ClosePort, },
  { "GetSysFont", LMGetSysFontFam, },
  { "SetDialogFont", SetDialogFont, },
  //  { "PLookupName", C_PLookupName, },
  { "IUDateString", C_IUDateString, },
  { "IsRegisteredSection", C_IsRegisteredSection, },
  { "DialogSelect", C_DialogSelect, },
  //  { "PRemoveName", C_PRemoveName, },
  { "DisposeMenu", C_DisposeMenu, },
  { "GetFInfo", GetFInfo, },
  { "GetMenuHandle", C_GetMHandle, },
  { "LNextCell", C_LNextCell, },
  { "GetControlMinimum", C_GetCtlMin, },
  { "OpenDriver", OpenDriver, },
  { "GetNextFOND", C_GetNextFOND, },
  { "FramePoly", C_FramePoly, },
  { "AssociateSection", C_AssociateSection, },
  { "EmptyRect", C_EmptyRect, },
  { "MPPOpen", MPPOpen_stub, },
  //  { "LaunchApplication", C_LaunchApplication, },
  { "LSearch", C_LSearch, },
  { "NSetTrapAddress", NSetTrapAddress_stub, },
  { "EqualPt", EqualPt_PC, },
  { "GZSaveHnd", LMGetGZRootHnd, },
  { "SetItemIcon", C_SetItemIcon, },
  { "CloseDialog", C_CloseDialog, },
  { "LMGetSysFontSize", LMGetSysFontSize, },
  { "UpperText", UpperText_stub, },
  { "GetNamedResource", C_GetNamedResource, },
  { "CloseEdition", C_CloseEdition, },
  { "SetControlMinimum", C_SetCtlMin, },
  //  { "NBPExtract", C_NBPExtract, },
  { "FreeMemSys", FreeMemSys_wrapper, },
  { "GetNewPalette", C_GetNewPalette, },
  { "LDelRow", C_LDelRow, },
  { "SetItemMark", C_SetItemMark, },
  { "UnpackBits", C_UnpackBits, },
  { "LoadResource", C_LoadResource, },
  //  { "Frac2X", C_Frac2X, },
  { "GetIntlResource", C_IUGetIntl, },
  { "TextWidth", C_TextWidth, },
  { "TEToScrap", TEToScrap, },
  { "AECreateAppleEvent", C_AECreateAppleEvent, },
  { "Move", C_Move, },
  { "InfoScrap", C_InfoScrap, },
  { "GetWTitle", C_GetWTitle, },
  { "FracMul", C_FracMul, },
  { "SetEmptyRgn", C_SetEmptyRgn, },
  { "ShowHide", C_ShowHide, },
  { "GetEditionFormatMark", C_GetEditionFormatMark, },
  { "GetCTSeed", C_GetCTSeed, },
  { "AEDisposeDesc", C_AEDisposeDesc, },
  { "FracDiv", C_FracDiv, },
  { "LwrText", LowerText_stub, },
  { "FracSqrt", C_FracSqrt, },
  { "GetDefFontSize", GetDefFontSize, },
#if !defined(CFM_PROBLEMS)
  { "GetDiskFragment", C_GetDiskFragment, },
#endif
  { "InvalRgn", C_InvalRgn, },
  { "CloseWindow", C_CloseWindow, },
  { "ShieldCursor", ShieldCursor_PC, },
  { "OpenPoly", C_OpenPoly, },
  { "SetGWorld", C_SetGWorld, },
  { "UnionRgn", C_UnionRgn, },
  { "SetPalette", C_SetPalette, },
  { "GoToPublisherSection", C_GoToPublisherSection, },
  { "FracCos", C_FracCos, },
  //  { "X2Frac", C_X2Frac, },
  { "GetGray", C_GetGray, },
  { "OpenEdition", C_OpenEdition, },
  { "NewSubscriberDialog", C_NewSubscriberDialog, },
  { "IsDialogEvent", C_IsDialogEvent, },
  { "LMGetSFSaveDisk", LMGetSFSaveDisk, },
  { "GetCaretTime", GetCaretTime, },
  { "AEPutParamDesc", C_AEPutKeyDesc, },
  { "GetGrayRgn", LMGetGrayRgn, },
  { "TESetText", C_TESetText, },
  { "DisposeCCursor", C_DisposCCursor, },
  { "DialogCut", DlgCut, },
  { "GetGWorld", C_GetGWorld, },
  { "Line", C_Line, },
  { "ResolveAliasFile", C_ResolveAliasFile, },
  { "TEGetText", C_TEGetText, },
  { "DiffRgn", C_DiffRgn, },
  { "SetGrowZone", SetGrowZone, },
  { "GetFontInfo", C_GetFontInfo, },
  { "CopyMask", C_CopyMask, },
  { "GetPenState", C_GetPenState, },
  { "ErasePoly", C_ErasePoly, },
  { "GetItemCmd", C_GetItemCmd, },
  { "UnionRect", C_UnionRect, },
  //  { "POpenATPSkt", C_POpenATPSkt, },
  { "PackBits", C_PackBits, },
  { "TEFromScrap", TEFromScrap, },
  { "SectionOptionsExpDialog", C_SectionOptionsExpDialog, },
  { "AECreateList", C_AECreateList, },
  { "SecondsToDate", Secs2Date, },
  { "DeleteMenuItem", C_DelMenuItem, },
  { "SetApplLimit", SetApplLimit, },
  { "PrValidate", C_PrValidate, },
  { "EmptyRgn", C_EmptyRgn, },
  { "GetMaxResourceSize", C_MaxSizeRsrc, },
  { "GetGWorldPixMap", C_GetGWorldPixMap, },
  //  { "SetFrontProcess", C_SetFrontProcess, },
  //  { "SectionOptionsDialog", C_SectionOptionsDialog, },
  { "LMSetSysFontFam", LMSetSysFontFam, },
  { "OffsetRgn", C_OffsetRgn, },
  { "LActivate", C_LActivate, },
  { "WriteEdition", C_WriteEdition, },
  { "GetEditionInfo", C_GetEditionInfo, },
  { "DateToSeconds", Date2Secs, },
  { "AEGetAttributePtr", C_AEGetAttributePtr, },
  { "IUTimeString", C_IUTimeString, },
  { "KeyTranslate", C_KeyTrans, },
  //  { "PKillNBP", C_PKillNBP, },
  { "SetResFileAttrs", C_SetResFileAttrs, },
  { "LMGetCurApRefNum", LMGetCurApRefNum, },
  { "GetPixBaseAddr", C_GetPixBaseAddr, },
  { "X2Fix", X2Fix_wrapper, },
  { "FontMetrics", C_FontMetrics, },
  { "RegisterSection", C_RegisterSection, },
  { "UnlockPixels", C_UnlockPixels, },
  { "OpenResFile", C_OpenResFile, },
  { "DeleteEditionContainerFile", C_DeleteEditionContainerFile, },
  { "FixATan2", FixATan2, },
  { "SetItemCmd", C_SetItemCmd, },
  { "FillPoly", C_FillPoly, },
  { "PaintPoly", C_PaintPoly, },
  { "ReserveMem", ReserveMem_wrapper, },
  { "LSetDrawingMode", C_LDoDraw, },
  { "GetOSEvent", GetOSEvent, },
  { "NewGWorld", C_NewGWorld, },
  { "GetFNum", C_GetFNum, },
  { "IUMagIDString", C_IUMagIDString, },
  { "TEIdle", C_TEIdle, },
  { "RectInRgn", C_RectInRgn, },
  { "ReadDateTime", ReadDateTime, },
  { "NewSection", C_NewSection, },
  { "MaxMemSys", MaxMemSys_wrapper, },
  { "SetOutlinePreferred", C_SetOutlinePreferred, },
  { "PrGeneral", C_PrGeneral, },
  { "LAddRow", C_LAddRow, },
  { "GetPtrSize", GetPtrSize, },
  { "PicComment", C_PicComment, },
  { "DeviceLoop", C_DeviceLoop, },
  //  { "PRegisterName", C_PRegisterName, },
  { "CharByte", C_CharByte, },
  { "AESend", C_AESend, },
  { "GetCCursor", C_GetCCursor, },
  { "EqualRgn", C_EqualRgn, },
  { "GetNewControl", C_GetNewControl, },
  { "MapRect", C_MapRect, },
  { "LMSetSysFontSize", LMSetSysFontSize, },
  { "UprText", UpperText_stub, },
  { "SetFractEnable", C_SetFractEnable, },
  { "BringToFront", C_BringToFront, },
  { "SetPenState", C_SetPenState, },
  { "TestControl", TestControl_PC, },
  { "InitEditionPackVersion", C_InitEditionPack, },
  { "InvertRoundRect", C_InvertRoundRect, },
  { "PtToAngle", PtToAngle_PC, },
  { "UnRegisterSection", C_UnRegisterSection, },
  { "LMGetSysFontFam", LMGetSysFontFam, },
  { "CalcVis", C_CalcVis, },
  { "GetGWorldDevice", C_GetGWorldDevice, },
  { "BitClr", C_BitClr, },
  //  { "PCloseATPSkt", C_PCloseATPSkt, },
  { "SetEditionFormatMark", C_SetEditionFormatMark, },
  { "OpenNewEdition", C_OpenNewEdition, },
  { "CalcMenuSize", C_CalcMenuSize, },
  { "NewPublisherExpDialog", C_NewPublisherExpDialog, },
  { "LAutoScroll", C_LAutoScroll, },
  { "TEClick", TEClick_PC, },
  { "SetCCursor", C_SetCCursor, },
  { "ReallocateHandle", ReallocHandle, },
  { "DialogDelete", DlgDelete, },
  { "DIBadMount", DIBadMount_PC, },
  { "EraseRgn", C_EraseRgn, },
  { "AECreateDesc", C_AECreateDesc, },
  { "SetPtrSize", SetPtrSize, },
  { "UpdateControls", C_UpdtControl, },
  { "DialogPaste", DlgPaste, },
  { "DrawChar", C_DrawChar, },
  { "MovePortTo", C_MovePortTo, },
  { "DialogCopy", DlgCopy, },
  { "PtrToXHand", PtrToXHand, },
  { "FillRgn", C_FillRgn, },
  { "TEKey", C_TEKey, },
  { "PaintRgn", C_PaintRgn, },
  { "AESetInteractionAllowed", C_AESetInteractionAllowed, },
  { "Fix2X", Fix2X, },
  { "GetDblTime", GetDblTime, },
  { "IUCompString", IUCompString, },
  { "OSEventAvail", OSEventAvail, },
  { "TEPaste", C_TEPaste, },
  { "GetResFileAttrs", C_GetResFileAttrs, },
  { "PurgeMem", PurgeMem_wrapper, },
  { "PtrToHand", PtrToHand, },
  { "CreateEditionContainerFile", C_CreateEditionContainerFile, },
  { "NewGDevice", C_NewGDevice, },
  { "GetPen", C_GetPen, },
  { "NBPSetEntity", NBPSetEntity_stub, },
  { "TECut", C_TECut, },
  { "LockPixels", C_LockPixels, },
  { "PrJobMerge", C_PrJobMerge, },
  { "EqualRect", C_EqualRect, },
  /* Energy Scheming */
  //  { "setmenuitemtext", C_setmenuitemtext, },
  { "ScreenRes", C_ScreenRes, },
  { "LMGetHiliteMode", LMGetHiliteMode, },
  { "Pt2Rect", Pt2Rect_PC, },
  //  { "appendmenu", C_appendmenu, },
  { "ClipAbove", C_ClipAbove, },
  //  { "paramtext", C_paramtext, },
  { "HMGetBalloons", C_HMGetBalloons, },
  //  { "newmenu", C_newmenu, },
  //  { "findcontrol", C_findcontrol, },
  { "SetResAttrs", C_SetResAttrs, },
  //  { "C2PStr", C_C2PStr, },
  //  { "setdialogitemtext", C_setdialogitemtext, },
  { "HMRemoveBalloon", C_HMRemoveBalloon, },
  //  { "getdialogitemtext", C_getdialogitemtext, },
  //  { "newwindow", C_newwindow, },
  { "SndNewChannel", C_SndNewChannel, },
  { "KillControls", C_KillControls, },
  //  { "lsetcell", C_lsetcell, },
  { "NewDialog", NewDialog_AIX, },
  //  { "lgetcell", C_lgetcell, },
  { "FillOval", C_FillOval, },
  { "SndDoImmediate", C_SndDoImmediate, },
  { "HMSetBalloons", C_HMSetBalloons, },
  { "LMSetHiliteMode", LMSetHiliteMode, },
  { "GetNewMBar", C_GetNewMBar, },
  { "GetResAttrs", C_GetResAttrs, },
  { "CharWidth", C_CharWidth, },
  //  { "setwtitle", C_setwtitle, },
  { "FSpDelete", C_FSpDelete, },
  { "TESetAlignment", C_TESetJust, },
  //  { "ptinrect", C_ptinrect, },
  { "SetString", C_SetString, },
  { "TEScrapHandle", TEScrapHandle, },
  { "OffsetPoly", C_OffsetPoly, },
  { "LMGetTime", LMGetTime, },
  { "ShowPen", C_ShowPen, },
  //  { "AEPutParamPtr", C_AEPutParamPtr, },
  //  { "drawstring", C_drawstring, },
  //  { "trackcontrol", C_trackcontrol, },
  //  { "lsetselect", C_lsetselect, },
  { "GetPixel", C_GetPixel, },
  { "GetIndPattern", GetIndPattern, },
  { "NewString", C_NewString, },
  ////////////////  { "PBSetEOFAsync", PBSetEOFAsync, },
  { "DragGrayRgn", DragGrayRgn_PC, },
  //  { "SecondsToDate", C_SecondsToDate, },
  //  { "GetFrontProcess", C_GetFrontProcess, },
  { "PtInRgn", PtInRgn_PC, },
  { "FSpCreateResFile", C_FSpCreateResFile, },
  //  { "flushvol", C_flushvol, },
  { "FSpCreate", C_FSpCreate, },
  //  { "getindstring", C_getindstring, },
  { "SendBehind", C_SendBehind, },
  { "PBHDeleteSync", PBHDeleteSync, },
  { "HMShowBalloon", HMShowBalloon_PC, },
  //  { "LSetDrawingMode", C_LSetDrawingMode, },
  ///////////////////  { "PBSetFPosAsync", PBSetFPosAsync, },
  { "HidePen", C_HidePen, },
  //  { "stringwidth", C_stringwidth, },
  //  { "GetPtrSize", C_GetPtrSize, },
  { "CloseDeskAcc", C_CloseDeskAcc, },
  { "MapPt", C_MapPt, },
  { "SndChannelStatus", C_SndChannelStatus, },
  { "GetSysDirection", LMGetTESysJust, },
  { "LMSetTEScrpLength", LMSetTEScrpLength, },
  { "MapRgn", C_MapRgn, },
  { "LMGetTEScrpLength", LMGetTEScrpLength, },
  { "GetMenuBar", C_GetMenuBar, },
  ///////////////////  { "PBGetEOFAsync", PBGetEOFAsync, },
  //  { "setcontroltitle", C_setcontroltitle, },
  { "LMGetWindowList", LMGetWindowList, },
  { "NewColorDialog", NewCDialog_AIX, },
  { "MapPoly", C_MapPoly, },
  //  { "SameProcess", C_SameProcess, },
  //  { "setvol", C_setvol, },
  { "ResetAlertStage", ResetAlrtStage, },
  { "GetWVariant", C_GetWVariant, },
  { "WaitMouseUp", C_WaitMouseUp, },
  //  { "GetDblTime", C_GetDblTime, },
  { "SetMenuBar", C_SetMenuBar, },
  { "StandardPutFile", C_StandardPutFile, },
  //  { "GetCurrentProcess", C_GetCurrentProcess, },
  //  { "SpeechBusy", C_SpeechBusy, },
  //  { "SpeakBuffer", C_SpeakBuffer, },
  //  { "GetIndVoice", C_GetIndVoice, },
  //  { "StopSpeech", C_StopSpeech, },
  //  { "NewSpeechChannel", C_NewSpeechChannel, },
  //  { "CountVoices", C_CountVoices, },

  //  { "DirCreate", C_DirCreate, }, /* photoshop 5.5 demo */
  { "SetControlColor", C_SetCtlColor, },
  { "LMGetGZRootHnd", LMGetGZRootHnd, },
  { "RGB2HSL", C_RGB2HSL, },
  //  { "Control", C_Control, },
  { "LMSetGrayRgn", LMSetGrayRgn, },
  { "GDeviceChanged", C_GDeviceChanged, },
  { "HSV2RGB", C_HSV2RGB, },
#if !defined (CFM_PROBLEMS)
  { "GetMemFragment", C_GetMemFragment, },
#endif
  //  { "GetComponentVersion", C_GetComponentVersion, },
  { "EraseOval", C_EraseOval, },
  { "GetPattern", C_GetPattern, },
  { "LMGetHWCfgFlags", LMGetHWCfgFlags, },
  //  { "CharacterByteType", C_CharacterByteType, },
  { "Color2Index", C_Color2Index, },
  { "GetItemIcon", C_GetItemIcon, },
  { "RGB2HSV", C_RGB2HSV, },
  { "AEResumeTheCurrentEvent", C_AEResumeTheCurrentEvent, },
  { "HandAndHand", HandAndHand, },
  //  { "Dequeue", C_Dequeue, },
  { "ActivatePalette", C_ActivatePalette, },
  { "Eject", Eject, },
  { "DisposeIconSuite", C_DisposeIconSuite, },
  //  { "OpenComponent", C_OpenComponent, },
  { "AESuspendTheCurrentEvent", C_AESuspendTheCurrentEvent, },
  { "Index2Color", C_Index2Color, },
  { "LMGetGhostWindow", LMGetGhostWindow, },
  { "IULDateString", C_IULDateString, },
  //  { "GetNodeAddress", C_GetNodeAddress, },
  { "AEGetAttributeDesc", C_AEGetAttributeDesc, },
  //  { "OpenDefaultComponent", C_OpenDefaultComponent, },
  //  { "NBPSetNTE", C_NBPSetNTE, },
  { "ClearMenuBar", C_ClearMenuBar, },
  { "Fix2Long", C_Fix2Long, },
  { "CalcVisBehind", C_CalcVisBehind, },
  { "LMGetHiliteRGB", LMGetHiliteRGB, },
  { "LMGetKeyThresh", LMGetKeyThresh, },
  { "GetOutlinePreferred", C_GetOutlinePreferred, },
  //  { "PLookupName", C_PLookupName, },
  //  { "Enqueue", C_Enqueue, },
  { "PBCatSearchSync", PBCatSearchSync, },
  { "UpdateGWorld", C_UpdateGWorld, },
  //  { "PRemoveName", C_PRemoveName, },
  { "InvalMenuBar", C_InvalMenuBar, },
  { "GetPixPat", C_GetPixPat, },
  //  { "LaunchApplication", C_LaunchApplication, },
  { "FixRound", C_FixRound, },
  ////////////////  { "HRename", HRename, },
  //  { "UnholdMemory", C_UnholdMemory, },
  { "QDDone", C_QDDone, },
  { "UpperString", UprString, },
  //  { "NBPExtract", C_NBPExtract, },
  { "TESetWordBreak", SetWordBreak, },
  { "LMGetCurActivate", LMGetCurActivate, },
  { "LMGetGZMoveHnd", LMGetGZMoveHnd, },
  { "SetStdProcs", C_SetStdProcs, },
  //  { "Frac2X", C_Frac2X, },
  { "TEGetStyle", C_TEGetStyle, },
  { "GetPalette", C_GetPalette, },
  { "KeyScript", C_KeyScript, },
  { "SetMCEntries", C_SetMCEntries, },
  { "IsMetric", C_IUMetric, },
  //  { "GetDCtlEntry", C_GetDCtlEntry, },
  { "SetStdCProcs", C_SetStdCProcs, },
  { "AESetTheCurrentEvent", C_AESetTheCurrentEvent, },
  //  { "HoldMemory", C_HoldMemory, },
  { "Long2Fix", C_Long2Fix, },
  { "AEGetNthDesc", C_AEGetNthDesc, },
  { "SetDialogCancelItem", C_SetDialogCancelItem, },
  //  { "AESizeOfParam", C_AESizeOfParam, },
  { "PrJobInit", C_PrJobInit, },
  { "FindSymbol", C_FindSymbol, },
  { "LMGetCurJTOffset", LMGetCurJTOffset, },
  { "HSetVol", HSetVol, },
  { "LMGetCurMap", LMGetCurMap, },
  { "AEPutAttributePtr", C_AEPutAttributePtr, },
  { "GetScriptVariable", C_GetScript, },
  { "FSpOpenRF", C_FSpOpenRF, },
  { "Status", Status, },
  //  { "AEPutParamPtr", C_AEPutParamPtr, },
  { "CharType", C_CharType, },
  //  { "X2Frac", C_X2Frac, },
  { "FindWord", C_FindWord, },
  { "MatchAlias", C_MatchAlias, },
  { "CustomGetFile", CustomGetFile_AIX, },
  //  { "StringToDate", C_StringToDate, },
  //  { "numtostring", C_numtostring, },
  { "LMSetCurMap", LMSetCurMap, },
  //  { "CountComponents", C_CountComponents, },
  { "GetADBInfo", GetADBInfo, },
  { "CTabChanged", C_CTabChanged, },
  ///////  { "Debugger", C_DebugStr, },
  //  { "GetIndPattern", C_GetIndPattern, },
  { "IULTimeString", C_IULTimeString, },
  //  { "FindNextComponent", C_FindNextComponent, },
  { "GetControlVariant", C_GetCVariant, },
  //  { "POpenATPSkt", C_POpenATPSkt, },
  { "XorRgn", C_XorRgn, },
  { "PBCatMoveSync", PBCatMoveSync, },
  { "LMSetLastSPExtra", LMSetLastSPExtra, },
  { "GetFrontProcess", C_GetFrontProcess, },
  { "LMSetROMMapInsert", LMSetROMMapInsert, },
  { "TECustomHook", C_TECustomHook, },
  { "InitDateCache", C_InitDateCache, },
  //  { "ReadLocation", C_ReadLocation, },
  { "SetFrontProcess", C_SetFrontProcess, },
  { "VisibleLength", C_VisibleLength, },
  { "FSpDirCreate", C_FSpDirCreate, },
  { "LMGetFSFCBLen", LMGetFSFCBLen, },
  { "EmptyHandle", EmptyHandle, },
  { "LMSetMBarHeight", LMSetMBarHeight, },
  { "PaintOne", C_PaintOne, },
  { "LMSetTopMapHndl", LMSetTopMapHndl, },
  { "SetWinColor", C_SetWinColor, },
  { "TEFeatureFlag", C_TEFeatureFlag, },
  { "PrStlInit", C_PrStlInit, },
  { "PBHRenameSync", PBHRenameSync, },
  { "ValidRgn", C_ValidRgn, },
  { "FSpRename", C_FSpRename, },
  { "ReadPartialResource", C_ReadPartialResource, },
  { "HSL2RGB", C_HSL2RGB, },
  { "SetPreserveGlyph", C_SetPreserveGlyph, },
  { "UnmountVol", UnmountVol, },
  { "LMSetCurDeactive", LMSetCurDeactive, },
  { "GetPictInfo", C_GetPictInfo, },
  { "AEPutAttributeDesc", C_AEPutAttributeDesc, },
  //  { "PBControlAsync", C_PBControlAsync, },
  { "LongDateToSeconds", C_LongDate2Secs, },
  { "LongSecondsToDate", C_LongSecs2Date, },
  { "GetAppFont", GetAppFont, },
  { "SetControlReference", C_SetCRefCon, },
  //  { "IUEqualString", C_IUEqualString, },
  { "NewAliasMinimal", C_NewAliasMinimal, },
  //  { "StdFilterProc", C_StdFilterProc, },
  //  { "PPCBrowser", C_PPCBrowser, },
  //  { "CloseComponent", C_CloseComponent, },
  { "SetRectRgn", C_SetRectRgn, },
  { "CountDITL", CountDITL, },
  { "DisposePalette", C_DisposePalette, },
  //  { "GetComponentInfo", C_GetComponentInfo, },
  { "PixPatChanged", C_PixPatChanged, },
  //  { "PRegisterName", C_PRegisterName, },
  { "NewPalette", C_NewPalette, },
  { "PrDlgMain", C_PrDlgMain, },
  { "UnloadScrap", C_UnloadScrap, },
  { "PBOpenSync", PBOpenSync, },
  { "AEPutPtr", C_AEPutPtr, },
  { "LMGetCurStackBase", LMGetCurStackBase, },
  { "SystemEvent", C_SystemEvent, },
  { "AEDeleteParam", C_AEDeleteKeyDesc, },
  //  { "PCloseATPSkt", C_PCloseATPSkt, },
  { "StuffHex", C_StuffHex, },
  { "LMSetCurActivate", LMSetCurActivate, },
  { "PBHOpenDFSync", PBHOpenDFSync, },
  { "PBHOpenRFSync", PBHOpenRFSync, },
  { "PBOpenRFSync", PBOpenRFSync, },
  //  { "CloseDriver", C_CloseDriver, },
  { "TESelView", C_TESelView, },
  { "SetDialogDefaultItem", C_SetDialogDefaultItem, },
  { "LMGetSysEvtMask", LMGetSysEvtMask, },
  { "HGetVol", HGetVol, },
  { "PBControlSync", PBControlSync, },
  { "SameProcess", C_SameProcess, },
  { "TEGetPoint", TEGetPoint_PC, },
  { "PBHOpenDenySync", PBHOpenDenySync, },
  //  { "CallComponentUPP", C_CallComponentUPP, },
  { "GetCurrentProcess", C_GetCurrentProcess, },
  { "GetDrvQHdr", GetDrvQHdr, },
  { "TEGetOffset", TEGetOffset_PC, },

  { "BitAnd", C_BitAnd, }, /* 3D-filmstrip */
  { "BitOr", C_BitOr, }, /* 3D-filmstrip */
  { "BitXor", C_BitXor, }, /* 3D-filmstrip */
  { "BitNot", C_BitNot, }, /* 3D-filmstrip */
  //  { "SetControlColor", C_SetControlColor, },
  //  { "IdleUpdate", C_IdleUpdate, },
  { "TESetScrapLength", TESetScrapLen, },
  { "BackPat", C_BackPat, },
  //  { "OpenDefaultComponent", C_OpenDefaultComponent, },
  { "GetEvQHdr", GetEvQHdr, },
  //  { "NMRemove", C_NMRemove, },
  //  { "LMSetPaintWhite", C_LMSetPaintWhite, },
  { "NSetPalette", C_NSetPalette, },
  { "Fix2SmallFract", C_Fix2SmallFract, },
  { "PortSize", C_PortSize, },
  { "TESetStyle", C_TESetStyle, },
  //  { "LMSetSaveUpdate", C_LMSetSaveUpdate, },
  //  { "TEScrapHandle", C_TEScrapHandle, },
  { "TEPinScroll", C_TEPinScroll, },
  { "LSize", C_LSize, },
  { "PostEvent", PostEvent, },
  { "BitMapToRegion", C_BitMapToRegion, },
  //  { "Debugger", C_Debugger, },
  { "GetMCEntry", C_GetMCEntry, },
  { "FSpSetFInfo", C_FSpSetFInfo, },
  { "TempMaxMem", C_TempMaxMem, },
  //  { "PBSetEOFAsync", C_PBSetEOFAsync, },
  //  { "LMSetLastSPExtra", C_LMSetLastSPExtra, },
  { "PaintRoundRect", C_PaintRoundRect, },
  { "OpColor", C_OpColor, },
  { "SndGetSysBeepState", C_SndGetSysBeepState, },
  //  { "DeleteMCEntries", C_DeleteMCEntries, },
  { "SndSetSysBeepState", C_SndSetSysBeepState, },
  { "TESetClickLoop", SetClikLoop, },
  { "GetAuxWin", C_GetAuxWin, },
  //  { "PBSetFPosAsync", C_PBSetFPosAsync, },
  { "TEGetScrapLength", TEGetScrapLen, },
  //  { "CloseComponent", C_CloseComponent, },
  { "RealFont", C_RealFont, },
  //  { "LMGetDAStrings", C_LMGetDAStrings, },
  { "FillRoundRect", C_FillRoundRect, },
  { "FillCPoly", C_FillCPoly, },
  { "MeasureText", C_MeasureText, },
  { "PenPixPat", C_PenPixPat, },
  //  { "DIBadMount", C_DIBadMount, },
  { "MaxMem", MaxMem_wrapper, },
  { "GetAuxiliaryControlRecord", C_GetAuxCtl, },
  { "SetEntryColor", C_SetEntryColor, },
  { "SmallFract2Fix", C_SmallFract2Fix, },
  //  { "SetMovieGWorld", C_SetMovieGWorld, },
  //  { "MCGetControllerBoundsRect", C_MCGetControllerBoundsRect, },
  //  { "BeginMediaEdits", C_BeginMediaEdits, },
  //  { "NewMovieTrack", C_NewMovieTrack, },
  //  { "GoToBeginningOfMovie", C_GoToBeginningOfMovie, },
  //  { "DisposeMovieController", C_DisposeMovieController, },
  //  { "GetMovieTime", C_GetMovieTime, },
  //  { "GetMovieUserData", C_GetMovieUserData, },
  //  { "NewTrackMedia", C_NewTrackMedia, },
  //  { "GetMoviePict", C_GetMoviePict, },
  //  { "StandardGetFilePreview", C_StandardGetFilePreview, },
  //  { "AddMovieResource", C_AddMovieResource, },
  //  { "MCIsPlayerEvent", C_MCIsPlayerEvent, },
  //  { "NewMovieFromFile", C_NewMovieFromFile, },
  //  { "GetMovieBox", C_GetMovieBox, },
  //  { "MCDoAction", C_MCDoAction, },
  //  { "DisposeMovie", C_DisposeMovie, },
  //  { "AddUserData", C_AddUserData, },
  //  { "EnterMovies", C_EnterMovies, },
  //  { "GetMaxCompressionSize", C_GetMaxCompressionSize, }
  //  { "AddMediaSample", C_AddMediaSample, },
  //  { "CloseMovieFile", C_CloseMovieFile, },
  //  { "CreateMovieFile", C_CreateMovieFile, },
  //  { "MCEnableEditing", C_MCEnableEditing, },
  //  { "InsertMediaIntoTrack", C_InsertMediaIntoTrack, },
  //  { "CompressImage", C_CompressImage, },
  //  { "GetMediaDuration", C_GetMediaDuration, },
  //  { "EndMediaEdits", C_EndMediaEdits, },
  //  { "OpenMovieFile", C_OpenMovieFile, },
  //  { "NewMovieController", C_NewMovieController, },
  //  { "CallComponentUPP", C_CallComponentUPP, },
  { "RegisterAppearanceClient", RegisterAppearanceClient_stub, },
  { "HOpen", HOpen, },
  { "HSetFInfo", HSetFInfo, },
  { "LMGetBootDrive", LMGetBootDrive, },
  { "LMGetFractEnable", LMGetFractEnable, },
  { "LMSetFractEnable", LMSetFractEnable, },

  { "HMShowMenuBalloon", HMShowMenuBalloon_AIX, },
  { "HMGetIndHelpMsg", HMGetIndHelpMsg_AIX, },
  { "NewCDialog", NewCDialog_AIX, },
  { "OutlineMetrics", OutlineMetrics_AIX, },
  { "PixelToChar", PixelToChar_AIX, },
  { "StdLine", StdLine_PC, },
  { "PBHRstFLockSync", PBHRstFLockSync, },
  { "GetVCBQHdr",  GetVCBQHdr, },
  { "PBCloseWDSync", PBCloseWDSync, },
  { "PBUnmountVol", PBUnmountVol, },
  { "PBHSetFLockSync", PBHSetFLockSync, },
  { "DiskEject", DiskEject, },
  { "UniqueID", C_UniqueID, },
  { "AddPt", AddPt_PC, },
  { "IsOutline", IsOutline_PC, },
  { "SubPt", SubPt_PC, },
  { "PixMap32Bit", C_PixMap32Bit, },
  { "RGetResource", C_RGetResource, },
  { "Get1IndType", C_Get1IndType, },
  { "SndDisposeChannel", C_SndDisposeChannel, },
  { "StdBits", C_StdBits, },
  { "LMGetWidthTabHandle", LMGetWidthTabHandle, },
  { "Count1Types", C_Count1Types, },
  { "HMIsBalloon", C_HMIsBalloon, },
  { "PBFlushVolSync", PBFlushVolSync, },
  { "SetPortPix", C_SetPortPix, },
  { "LoadScrap", C_LoadScrap, },
  { "FSpExchangeFiles", C_FSpExchangeFiles, },
  { "EraseArc", C_EraseArc, },
  { "LMGetROM85", LMGetROM85, },
  { "GetFontName", C_GetFontName, },
  { "LMGetKbdType", LMGetKbdType, },
  { "FillArc", C_FillArc, },
  { "CharExtra", C_CharExtra, },
  { "StdText", StdText_PC, },
  { "PBHSetVolSync", PBHSetVolSync, },
  { "FMSwapFont", C_FMSwapFont, },
  { "FindDialogItem", C_FindDItem, },
  { "GetDefaultOutputVolume", C_GetDefaultOutputVolume, },
  { "SpaceExtra", C_SpaceExtra, },
  { "SndDoCommand", C_SndDoCommand, },
  ///////////////////////////  { "PBGetVolSync", PBGetVolSync, },
  { "AngleFromSlope", C_AngleFromSlope, },
  { "InitPack", C_InitPack, },
  { "PinRect", PinRect_PC, },
  { "OpenCPicture", C_OpenCPicture, },
  { "BitSet", C_BitSet, },
  { "InvertOval", C_InvertOval, },
  { "BitShift", C_BitShift, },
  { "GetEntryColor", C_GetEntryColor, },
  { "LScroll", C_LScroll, },
  { "FrameArc", C_FrameArc, },
  { "SetScriptManagerVariable", C_SetEnvirons, },
  { "SetScriptVariable", C_SetScript, },
  { "SetIntlResource", C_IUSetIntl, },
  { "ClearIntlResourceCache", C_IUClearCache, },
  { "GetIntlResourceTable", C_IUGetItlTable, },
  { "SetSysDirection", LMSetTESysJust, },

  { "PBFlushFileSync", PBFlushFileSync, },
  { "PBMountVol", PBMountVol, },
  { "PBEject", PBEject, },
  { "PBCatSearchAsync", PBCatSearchAsync, },
  { "PBLockRangeSync", PBLockRangeSync, },
  { "PBUnlockRangeSync", PBUnlockRangeSync, }, 
  { "PBDTGetIconSync", PBDTGetIconSync, },
  { "PBDTGetIconInfoSync", PBDTGetIconInfoSync, },
  { "PBDTOpenInform", PBDTOpenInform, },

  { "GetCPixel", C_GetCPixel, },
  { "InitCPort", C_InitCPort , },
  { "BackPixPat", C_BackPixPat , },
  { "HiliteColor", C_HiliteColor , },
  { "CopyPixMap", C_CopyPixMap , },
  { "CopyPixPat", C_CopyPixPat , },
  { "InitGDevice", C_InitGDevice , },
  { "GetSubTable", C_GetSubTable , },
  { "FillCRoundRect", C_FillCRoundRect , },
  { "FillCOval", C_FillCOval , },
  { "FillCArc", C_FillCArc , },
  { "FillCRgn", C_FillCRgn , },
  { "RealColor", C_RealColor , },
  { "ProtectEntry", C_ProtectEntry , },
  { "ReserveEntry", C_ReserveEntry , },
  { "SetEntries", C_SetEntries , },
  { "AddSearch", C_AddSearch , },
  { "AddComp", C_AddComp , },
  { "DelSearch", C_DelSearch , },
  { "DelComp", C_DelComp , },
  { "SetClientID", C_SetClientID , },
  { "CMY2RGB", C_CMY2RGB , },
  { "RGB2CMY", C_RGB2CMY , },
  { "DisposCTable", C_DisposCTable , },
  { "InitPalettes", C_InitPalettes , },
  { "AnimateEntry", C_AnimateEntry , },
  { "AnimatePalette", C_AnimatePalette , },
  { "GetEntryUsage", C_GetEntryUsage , },
  { "SetEntryUsage", C_SetEntryUsage , },
  { "CTab2Palette", C_CTab2Palette , },
  { "Palette2CTab", C_Palette2CTab , },
  { "DisposCCursor", C_DisposCCursor , },
  { "AllocCursor", C_AllocCursor , },
  { "RestoreClutDevice", C_RestoreClutDevice , },
  { "ResizePalette", C_ResizePalette , },
  { "PMgrVersion", C_PMgrVersion , },
  { "SaveFore", C_SaveFore , },
  { "RestoreFore", C_RestoreFore , },
  { "SaveBack", C_SaveBack , },
  { "RestoreBack", C_RestoreBack , },
  { "SetPaletteUpdates", C_SetPaletteUpdates , },
  { "GetPaletteUpdates", C_GetPaletteUpdates , },
  { "CopyPalette", C_CopyPalette , },
  { "AllowPurgePixels", C_AllowPurgePixels , },
  { "NoPurgePixels", C_NoPurgePixels , },
  { "GetPixelsState", C_GetPixelsState , },
  { "SetPixelsState", C_SetPixelsState , },
  { "NewScreenBuffer", C_NewScreenBuffer , },
  { "DisposeScreenBuffer", C_DisposeScreenBuffer , },
  { "NewTempScreenBuffer", C_NewTempScreenBuffer , },
  { "PortChanged", C_PortChanged , },
  { "OffscreenVersion", C_OffscreenVersion , },
  { "Entry2Index", C_Entry2Index, },
  { "SaveEntries", C_SaveEntries , },
  { "RestoreEntries", C_RestoreEntries , },
  { "DisposGDevice", C_DisposGDevice, },
  { "DisposePictInfo", C_DisposePictInfo , },
  { "RecordPictInfo", C_RecordPictInfo , },
  { "RecordPixMapInfo", C_RecordPixMapInfo , },
  { "RetrievePictInfo", C_RetrievePictInfo , },
  { "NewPictInfo", C_NewPictInfo , },
  { "GetPixMapInfo", C_GetPixMapInfo , },
  { "ADBReInit", ADBReInit, },
  { "ADBOp", ADBOp, },
  { "CountADBs", CountADBs, },
  { "GetIndADB", GetIndADB, },
  { "SetADBInfo", SetADBInfo, },
  { "IsMPPOpen", IsMPPOpen, },
  { "IsATPOpen", IsATPOpen, },
  { "LMGetLastSPExtra", LMGetLastSPExtra, },
  { "LMGetTheGDevice", LMGetTheGDevice, },
  { "LMGetROMBase", LMGetROMBase, },
  { "LMGetMenuList", LMGetMenuList, },
  { "LMGetResErr", LMGetResErr, },
  { "LMGetPrintErr", LMGetPrintErr, },
  { "LMGetWidthPtr", LMGetWidthPtr, },
  { "LMGetCaretTime", LMGetCaretTime, },
  //////  { "LMGetQDColors", LMGetQDColors, },
  { "LMGetDefVCBPtr", LMGetDefVCBPtr, },
  { "LMGetAtMenuBottom", LMGetAtMenuBottom, },
  { "LMGetSdVolume", LMGetSdVolume, },
  { "LMGetDragHook", LMGetDragHook, },
  { "LMGetWidthListHand", LMGetWidthListHand, },
  { "LMGetTopMenuItem", LMGetTopMenuItem, },
  { "LMGetDoubleTime", LMGetDoubleTime, },
  { "LMGetDAStrings", LMGetDAStrings, },
  { "LMGetMainDevice", LMGetMainDevice, },
  { "LMSetMenuDisable", LMSetMenuDisable, },
  { "LMSetAtMenuBottom", LMSetAtMenuBottom, },
  { "LMSetTopMenuItem", LMSetTopMenuItem, },
  { "MakeDataExecutable", MakeDataExecutable, },
  { "Rename", Rename, },
  { "RstFLock", RstFLock, },
  { "DILoad", C_DILoad, },
  { "DIUnload", C_DIUnload, },
  { "DIFormat", C_DIFormat, },
  { "DIVerify", C_DIVerify, },
  { "DIZero", C_DIZero, },
  { "OpenRF", OpenRF, },
  { "NewAliasMinimalFromFullPath", C_NewAliasMinimalFromFullPath, },
  { "UpdateAlias", C_UpdateAlias, },
  { "ResolveAlias", C_ResolveAlias, },
  { "TempTopMem", C_TempTopMem, },
  { "InsertResMenu", C_InsertResMenu, },
  { "FlashMenuBar", C_FlashMenuBar, },
  { "SetMenuFlash", C_SetMenuFlash, },
  { "InitProcMenu", C_InitProcMenu, },
  { "MenuChoice", C_MenuChoice, },
  { "DeleteMCEntries", C_DelMCEntries, },
  { "GetMCInfo", C_GetMCInfo, },
  { "SetMCInfo", C_SetMCInfo, },
  { "DisposeMCInfo", C_DispMCInfo, },

  { "ScrollRect", C_ScrollRect, },
  { "ColorBit", C_ColorBit, },
  { "InitPort", C_InitPort, },
  { "GrafDevice", C_GrafDevice, },
  { "SeedFill", C_SeedFill, },
  { "CalcMask", C_CalcMask, },
  { "DisposPixPat", C_DisposPixPat, },
  { "ScalePt", C_ScalePt, },
  { "ReadComment", C_ReadComment, },
  { "RectRgn", C_RectRgn, },
  { "PaintArc", C_PaintArc, },
  { "InvertArc", C_InvertArc, },
  { "StdArc", C_StdArc, },
  { "StdOval", C_StdOval, },
  { "StdComment", C_StdComment, },
  { "StdGetPic", C_StdGetPic, },
  { "StdPutPic", C_StdPutPic, },
  { "StdPoly", C_StdPoly, },
  { "StdRRect", C_StdRRect, },
  { "StdRect", C_StdRect, },
  { "StdRgn", C_StdRgn, },
  { "StdTxMeas", C_StdTxMeas, },
  { "SetCPixel", C_SetCPixel, },
  { "SeedCFill", C_SeedCFill, },
  { "CalcCMask", C_CalcCMask, },
  { "CopyDeepMask", C_IMVI_CopyDeepMask, },
  { "ShutDwnPower", C_ShutDwnPower, },
  { "ShutDwnStart", C_ShutDwnStart, },
  { "ShutDwnInstall", C_ShutDwnInstall, },
  { "ShutDwnRemove", C_ShutDwnRemove, },
  { "AECoerceDesc", C_AECoerceDesc, },
  { "AEDuplicateDesc", C_AEDuplicateDesc, },
  { "AEInstallCoercionHandler", C_AEInstallCoercionHandler, },
  { "AEPutParamPtr", C_AEPutKeyPtr, },
  { "AERemoveCoercionHandler", C_AERemoveCoercionHandler, },
  { "AEGetCoercionHandler", C_AEGetCoercionHandler, },
  { "LFind", LFind_PC, },
  { "LRect", LRect_PC, },
  { "LDraw", LDraw_PC, },
  { "LSetSelect", LSetSelect_PC, },
  { "LAddToCell", LAddToCell_PC, },
  { "LClrCell", LClrCell_PC, },
  { "LGetCell", LGetCell_PC, },
  { "LSetCell", LSetCell_PC, },
  { "LCellSize", LCellSize_PC, },
  { "LAddColumn", C_LAddColumn, },
  { "LGetCellDataLocation", LFind_PC, },

  { "StringToTime", C_String2Time, },
  { "StringToDate", C_String2Date, },
  { "MacReplaceText", C_ReplaceText, },
  { "ReplaceText", C_ReplaceText, },

  { "Transliterate", C_Transliterate, },
  { "FontScript", C_FontScript, },
  { "IntlScript", C_IntlScript, },
  { "MeasureJust", C_MeasureJust, },
  { "HiliteText", C_HiliteText, },
  { "DrawJust", C_DrawJust, },
  { "StyledLineBreak", C_StyledLineBreak, },
  { "ParseTable", C_ParseTable, },
  { "FillParseTable", C_FillParseTable, },
  { "StringToFormatRec", C_StringToFormatRec, },
  { "ToggleDate", C_ToggleDate, },
  { "FindScriptRun", C_FindScriptRun, },
  { "LowercaseText", C_LowercaseText, },
  { "UppercaseText", C_UppercaseText, },
  { "StripDiacritics", C_StripDiacritics, },
  { "UppercaseStripDiacritics", C_UppercaseStripDiacritics, },
  { "Pixel2Char", C_Pixel2Char, },
  { "Char2Pixel", C_Char2Pixel, },

  //  NMeasureJust_PC
  //  PortionLine_PC
  //  DrawJustified_PC
  //  CharToPixel_PC

  { "LMGetRAMBase", LMGetRAMBase, },
  { "LMGetBufPtr", LMGetBufPtr, },
  { "SetControlAction", C_SetCtlAction, },
  { "GetControlAction", C_GetCtlAction, },
  { "SetOSTrapAddress", SetOSTrapAddress, },
  { "NewIconSuite", C_NewIconSuite, },
  { "PlotIconSuite", C_PlotIconSuite, },
  { "AddIconToSuite", C_AddIconToSuite, },

  { "CharacterByteType", C_CharacterByteType, },
  { "CharacterType", C_CharacterType, },
  { "TransliterateText", C_TransliterateText, },
};

PUBLIC OSErr
ROMlib_GetInterfaceLib (Str63 library, OSType arch, LoadFlags loadflags,
			ConnectionID *cidp, Ptr *mainaddrp, Str255 errName)
{
  static ConnectionID cid;
  OSErr retval;

  if (cid)
    {
      *cidp = cid;
      retval = noErr;
    }
  else
    {
#if !defined (CFM_PROBLEMS)
      cid = ROMlib_new_connection (1);
#else
#warning "Can't do the right thing without CFM"
      cid = 0;
#endif
      if (!cid)
	retval = fragNoMem;
      else
	{
	  cid->lihp = ROMlib_build_pef_hash (interfacelib_map,
					     NELEM (interfacelib_map));
	  cid->ref_count = 1;
	  retval = noErr;
	  *cidp = cid;
	}
    }
  if (retval == noErr)
    *mainaddrp = 0;
  return retval;
}

#endif
