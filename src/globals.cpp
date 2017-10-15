/* Copyright 1990, 1996 by Abacus Research and
 * Development, Inc. All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_globals[] =
 "$Id: globals.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#if 0

#include "MemoryMgr.h"
#include "DeviceMgr.h"
#include "SoundDvr.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "FontMgr.h"
#include "WindowMgr.h"
#include "ControlMgr.h"
#include "MenuMgr.h"
#include "AppleEvents.h"

using namespace Executor;

#define DATA(type, name, suffix, address, supported, manager, citation) \
GUEST<type> name suffix

DATA(Ptr,     nilhandle,,	 0x00, TRUE-b, rsys/misc,      MADEUP);
/*
 * NOTE: MacWrite starts writing longwords at location 0x80 for TRAPs
 */
DATA(LONGINT,        trapvectors,[10],	 0x80, TRUE-b, rsys/misc,    WHOKNOWS);
DATA(Ptr,     dodusesit,, 	 0xE4, TRUE-b, rsys/misc,    WHOKNOWS);
DATA(INTEGER,        monkeylives,, 	0x100, TRUE-b, OSEvent,      SysEqu.a);
DATA(INTEGER,        ScrVRes,, 		0x102, TRUE  , QuickDraw,     IMI-473);
DATA(INTEGER,        ScrHRes,, 		0x104, TRUE  , QuickDraw,     IMI-473);
DATA(INTEGER,        ScreenRow,, 	0x106, TRUE  , QuickDraw,      ThinkC);
DATA(Ptr,     MemTop,, 	0x108, TRUE  , MemoryMgr,     IMII-19);
DATA(Ptr,     BufPtr,,		0x10C, TRUE-b, MemoryMgr,     IMII-19);
DATA(Ptr,     HeapEnd,,	0x114, TRUE  , MemoryMgr,     IMII-19);
DATA(THz,     TheZone,,	0x118, TRUE  , MemoryMgr,     IMII-31);
DATA(DCtlHandlePtr, UTableBase,,0x11C, FALSE, DeviceMgr,    IMII-192);
DATA(Byte,           loadtrap,, 	0x12D, TRUE-b, SegmentLdr,   SysEqu.a);
DATA(Byte,           CPUFlag,, 		0x12F, TRUE-b, StartMgr,      IMV-348);
DATA(Ptr,     ApplLimit,, 	0x130, TRUE  , MemoryMgr,     IMII-19);
DATA(INTEGER,        SysEvtMask,, 	0x144, TRUE  , OSEvent,       IMII-70);
DATA(QHdr,           EventQueue,, 	0x14A, TRUE  , OSEvent,       IMII-71);
DATA(LONGINT, RndSeed,, 	0x156, TRUE  , QuickDraw,     IMI-195);
DATA(INTEGER,        SysVersion,, 	0x15A, TRUE  , OSUtil,         ThinkC);
DATA(Byte, 	     SEvtEnb,, 		0x15C, FALSE , DeskMgr,       IMI-443);
DATA(QHdr,           VBLQueue,, 	0x160, TRUE  , VRetraceMgr,  IMII-352);
DATA(ULONGINT,Ticks,, 	0x16A, TRUE  , OSEvent,       IMI-260);
DATA(Byte, 	     MBState,, 		0x172, True-b, EventMgr,       PegLeg);
DATA(unsigned char,  KeyMap,[16], 	0x174, TRUE-b, EventMgr,     SysEqu.a);
/* was LONGINT KeypadMap[2]; */
DATA(INTEGER,        KeyThresh,, 	0x18E, TRUE  , ToolboxEvent,  IMI-246);
DATA(INTEGER,        KeyRepThresh,, 	0x190, TRUE  , ToolboxEvent,  IMI-246);
DATA(ProcPtr, Lvl1DT,[8], 	0x192, FALSE , DeviceMgr,    IMII-197);
/*
 * Hypercard does a movel to this location.
 */
DATA(LONGINT,        hyperlong,, 	0x1AA, TRUE-b, rsys/misc,    WHOKNOWS);
DATA(ProcPtr, Lvl2DT,[8], 	0x1B2, FALSE , DeviceMgr,    IMII-198);
DATA(INTEGER,        UnitNtryCnt,, 	0x1D2, TRUE-b, DeviceMgr,      ThinkC);
DATA(Ptr,     VIA,, 		0x1D4, TRUE-b, DeviceMgr,    IMIII-39);
DATA(Ptr,     SCCRd,, 		0x1D8, FALSE , DeviceMgr,    IMII-199);
DATA(Ptr,     SCCWr,, 		0x1DC, FALSE , DeviceMgr,    IMII-199);
DATA(Ptr,     IWM,, 		0x1E0, FALSE , DeviceMgr,      ThinkC);
DATA(Byte,           Scratch20,[20], 	0x1E4, TRUE  , MemoryMgr,      IMI-85);
DATA(Byte,           SPValid,, 		0x1F8, TRUE  , OSUtil,       IMII-392);
DATA(Byte,           SPATalkA,,		0x1F9, TRUE  , OSUtil,       IMII-392);
DATA(Byte,           SPATalkB,,		0x1FA, TRUE  , OSUtil,       IMII-392);
DATA(Byte,           SPConfig,,		0x1FB, TRUE  , OSUtil,       IMII-392);
DATA(INTEGER,        SPPortA,, 		0x1FC, TRUE  , OSUtil,       IMII-392);
DATA(INTEGER,        SPPortB,, 		0x1FE, TRUE  , OSUtil,       IMII-392);
DATA(LONGINT,        SPAlarm,, 		0x200, TRUE  , OSUtil,       IMII-392);
DATA(INTEGER,        SPFont,, 		0x204, TRUE  , OSUtil,       IMII-392);
DATA(Byte,           SPKbd,, 		0x206, TRUE  , OSUtil,       IMII-369);
DATA(Byte,           SPPrint,, 		0x207, TRUE  , OSUtil,       IMII-392);
DATA(Byte,           SPVolCtl,, 	0x208, TRUE  , OSUtil,       IMII-392);
DATA(Byte,           SPClikCaret,, 	0x209, TRUE  , OSUtil,       IMII-392);
DATA(Byte,           SPMisc2,, 		0x20B, TRUE  , OSUtil,       IMII-392);
DATA(ULONGINT,       Time,, 		0x20C, TRUE  , OSUtil,        IMI-260);
DATA(INTEGER,        BootDrive,, 	0x210, TRUE  , FileMgr,      IMIV-212);
DATA(INTEGER,        SFSaveDisk,, 	0x214, TRUE  , StdFilePkg,    IMIV-72);
DATA(Byte,           KbdLast,, 		0x218, FALSE , QuickDraw,     IMV-367);
DATA(Byte,           KbdType,, 		0x21E, FALSE , QuickDraw,     IMV-367);
DATA(INTEGER,        MemErr,, 		0x220, TRUE  , MemoryMgr,     IMIV-80);
DATA(Byte,           SdVolume,, 	0x260, TRUE-b, SoundDvr,     IMII-232);
DATA(FTSndRecPtr,SoundPtr,, 	0x262, FALSE , SoundDvr,     IMII-227);
DATA(Ptr,     SoundBase,, 	0x266, TRUE-b, SoundDvr,     IMIII-21);
DATA(Byte,           SoundActive,, 	0x27E, TRUE  , SoundDvr,          MPW);
DATA(Byte,           SoundLevel,, 	0x27F, FALSE , SoundDvr,     IMII-234);
DATA(INTEGER,        CurPitch,, 	0x280, TRUE-b, SoundDvr,     IMII-226);
/*
 * NOTE: mathones is a LONGINT that Mathematica looks at that contains -1
 * on a Mac+
 */
DATA(LONGINT,        mathones,,		0x282, TRUE-b, rsys/misc,    WHOKNOWS);
/*
 * NOTE: Theoretically ROM85 is mentioned in IMV, but I don't know where.
 * On a Mac+ the value 0x7FFF is stored there.
 * tim: It is at least on page IMV-328.
 */
DATA(INTEGER,        ROM85,, 		0x28E, TRUE-b, MacTypes,      IMV-328);
DATA(Byte,           PortBUse,, 	0x291, TRUE-b, AppleTalk,    IMII-305);
DATA(Byte,           ScreenVars,[8], 	0x292, FALSE , QuickDraw,         MPW);
/*
 * NOTE: Key1Trans in the keyboard translator procedure, and Key2Trans in the
 * numeric keypad translator procedure (MPW).
 */
DATA(Ptr,     Key1Trans,, 	0x29E, FALSE , QuickDraw,         MPW);
DATA(Ptr,     Key2Trans,, 	0x2A2, FALSE , QuickDraw,         MPW);
DATA(THz,     SysZone,, 	0x2A6, TRUE  , MemoryMgr,     IMII-19);
DATA(THz,     ApplZone,, 	0x2AA, TRUE  , MemoryMgr,     IMII-19);
DATA(Ptr,     ROMBase,, 	0x2AE, TRUE-b, MemoryMgr,    IMIV-236);
DATA(Ptr,     RAMBase,, 	0x2B2, FALSE , MemoryMgr,      IMI-87);
DATA(AE_info_ptr,AE_info,,	0x2B6, TRUE,   AppleEvents,   AEGizmo);
DATA(Ptr,     DSAlertTab,, 	0x2BA, TRUE  , SysErr,       IMII-359);
DATA(ProcPtr, ExtStsDT,[4], 	0x2BE, FALSE , DeviceMgr,    IMII-199);
DATA(Ptr,     ABusVars,, 	0x2D8, FALSE , AppleTalk,    IMII-328);
DATA(Ptr,     ABusDCE,,        0x2DC, FALSE , AppleTalk,         MPW);
DATA(Byte,           FinderName,[16],	0x2E0, TRUE  , SegmentLdr,    IMII-59);
DATA(LONGINT,        DoubleTime,, 	0x2F0, TRUE  , ToolboxEvent,  IMI-260);
DATA(LONGINT,        CaretTime,, 	0x2F4, TRUE  , ToolboxEvent,  IMI-260);
DATA(Byte,           ScrDmpEnb,, 	0x2F8, TRUE  , ToolboxEvent,  IMI-258);
DATA(LONGINT,        BufTgFNum,, 	0x2FC, FALSE , DiskDvr,      IMII-212);
DATA(INTEGER,        BufTgFFlg,, 	0x300, FALSE , DiskDvr,      IMII-212);
DATA(INTEGER,        BufTgFBkNum,, 	0x302, FALSE , DiskDvr,      IMII-212);
DATA(LONGINT,        BufTgDate,, 	0x304, FALSE , DiskDvr,      IMII-212);
DATA(QHdr,           DrvQHdr,, 		0x308, TRUE  , FileMgr,      IMIV-182);
DATA(Ptr,     heapcheck,, 	0x316, TRUE-b, MemoryMgr,    SysEqu.a);
DATA(LONGINT,        Lo3Bytes,, 	0x31A, TRUE  , MemoryMgr,      IMI-85);
DATA(LONGINT,        MinStack,, 	0x31E, TRUE-b, MemoryMgr,     IMII-17);
DATA(LONGINT,        DefltStack,, 	0x322, TRUE-b, MemoryMgr,     IMII-17);
DATA(Handle,  GZRootHnd,, 	0x328, TRUE  , MemoryMgr,      IMI-43);
DATA(Handle,  GZMoveHnd,, 	0x330, FALSE , MemoryMgr,    LowMem.h);
DATA(ProcPtr, EjectNotify,, 	0x338, FALSE , FileMgr,        ThinkC);
DATA(ProcPtr, IAZNotify,, 	0x33C, TRUE-b, MemoryMgr,      ThinkC);
DATA(Ptr,     FCBSPtr,, 	0x34E, TRUE  , FileMgr,      IMIV-179);
DATA(VCBPtr,  DefVCBPtr,, 	0x352, TRUE  , FileMgr,      IMIV-178);
DATA(QHdr,           VCBQHdr,, 		0x356, TRUE  , FileMgr,      IMIV-178);
DATA(QHdr,           FSQHdr,, 		0x360, TRUE  , FileMgr,      IMIV-176);
DATA(Ptr,     WDCBsPtr,, 	0x372, TRUE  , FileMgr,        idunno);
DATA(INTEGER,        DefVRefNum,, 	0x384, TRUE  , FileMgr,           MPW);
DATA(LONGINT,        CurDirStore,, 	0x398, TRUE  , StdFilePkg,    IMIV-72);
/*
 * Note: MacLinkPC+ loads 0x358 into a register (i.e. the address of the
 * pointer to the first element on the VCB queue) and then uses
 * 72 off of it (0x3A0) and 78 off of it (0x3A6). As LONGINT as
 * there are zeros there, that doesn't hurt us, but normally,
 * we'd have negative ones in there. Hence we describe them
 * here and set them to zero in executor.
 */
/* DATA(Ptr, FmtDefaults,, 	0x39E, FALSE , FileMgr, ThinkC); */
DATA(INTEGER,        MCLKPCmiss1,, 	0x3A0, TRUE-b, MacLinkPC,   badaccess);
DATA(INTEGER,        MCLKPCmiss2,, 	0x3A6, TRUE-b, MacLinkPC,   badaccess);
DATA(Ptr,     ToExtFS,, 	0x3F2, FALSE , FileMgr,      IMIV-212);
DATA(INTEGER,        FSFCBLen,, 	0x3F6, TRUE  , FileMgr,       IMIV-97);
DATA(Rect,           DSAlertRect,, 	0x3F8, TRUE  , SysErr,       IMII-362);
DATA(ProcPtr, JUnknown574,, 	0x574, TRUE-b, QuickDraw,         IMV);
DATA(ProcPtr, JADBProc,, 	0x6B8, FALSE , QuickDraw,         IMV);
/*
 * JFLUSH is a guess from disassembling some of Excel 3.0
 */
DATA(ProcPtr, JFLUSH,, 	0x6F4, TRUE-b, idunno,          guess);
DATA(ProcPtr, JResUnknown1,, 	0x700, TRUE-b, idunno,        resedit);
DATA(ProcPtr, JResUnknown2,, 	0x714, TRUE-b, idunno,        resedit);
DATA(ProcPtr, JHideCursor,, 	0x800, TRUE-b, QuickDraw,   Private.a);
DATA(ProcPtr, JShowCursor,, 	0x804, TRUE-b, QuickDraw,   Private.a);
DATA(ProcPtr, JShieldCursor,, 	0x808, TRUE-b, QuickDraw,   Private.a);
DATA(ProcPtr, JScrnAddr,, 	0x80C, FALSE , QuickDraw,   Private.a);
DATA(ProcPtr, JScrnSize,, 	0x810, FALSE , QuickDraw,   Private.a);
DATA(ProcPtr, JInitCrsr,, 	0x814, TRUE-b, QuickDraw,   Private.a);
DATA(ProcPtr, JSetCrsr,, 	0x818, TRUE-b, QuickDraw,   Private.a);
DATA(ProcPtr, JCrsrObscure,, 	0x81C, TRUE-b, QuickDraw,   Private.a);
DATA(ProcPtr, JUpdateProc,, 	0x820, FALSE , QuickDraw,   Private.a);
DATA(Ptr,     ScrnBase,, 	0x824, TRUE  , QuickDraw,     IMII-19);
/*
 * MouseLocation used to be 0x830, but that doesn't jibe with what I've
 * seen of Crystal Quest --ctm
 */
DATA(Point,          MTemp,,	 	0x828, True-b, QuickDraw,      PegLeg);
DATA(Point,          MouseLocation,, 	0x82C, TRUE  , QuickDraw,        Vamp);
DATA(Point,          MouseLocation2,, 	0x830, TRUE  , QuickDraw, MacAttack);
DATA(Rect,           CrsrPin,, 		0x834, FALSE , QuickDraw,      ThinkC);
DATA(GDHandle,MainDevice,, 	0x8A4, TRUE  , QuickDraw,         IMV);
DATA(GDHandle,DeviceList,, 	0x8A8, TRUE  , QuickDraw,         IMV);
DATA(Byte,           QDColors,, 	0x8B0, FALSE , QuickDraw,         IMV);
DATA(BOOLEAN,        CrsrVis,, 		0x8CC, TRUE  , QuickDraw,    SysEqu.a);
DATA(Byte,           CrsrBusy,, 	0x8CD, TRUE  , QuickDraw,    SysEqu.a);
DATA(INTEGER,        CrsrState,, 	0x8D0, TRUE  , QuickDraw,    SysEqu.a);
DATA(LONGINT,        mousemask,, 	0x8D6, TRUE-b, QuickDraw,          .a);
DATA(LONGINT,        mouseoffset,, 	0x8DA, TRUE-b, QuickDraw,    SysEqu.a);
DATA(INTEGER,        JournalFlag,, 	0x8DE, FALSE , ToolboxEvent,  IMI-261);
DATA(ProcPtr, JSwapFont,, 	0x8E0, TRUE-b, FontMgr,     Private.a);
DATA(Handle,  WidthListHand,, 	0x8E4, TRUE  , FontMgr,       IMIV-42);
DATA(INTEGER,        JournalRef,, 	0x8E8, FALSE , ToolboxEvent,  IMI-261);
DATA(INTEGER,        CrsrThresh,, 	0x8EC, FALSE , OSUtil,       IMII-372);
DATA(ProcPtr, JCrsrTask,,	0x8EE, TRUE  , ,                     );
DATA(Byte,           WWExist,, 		0x8F2, TRUE  , SysError,     SysEqu.a);
DATA(Byte,           QDExist,, 		0x8F3, TRUE  , SysError,     SysEqu.a);
DATA(Ptr,     JFetch,, 	0x8F4, FALSE , DeviceMgr,    IMII-194);
DATA(Ptr,     JStash,, 	0x8F8, FALSE , DeviceMgr,    IMII-195);
DATA(Ptr,     JIODone,, 	0x8FC, FALSE , DeviceMgr,    IMII-195);
DATA(INTEGER,        CurApRefNum,, 	0x900, TRUE  , SegmentLdr,    IMII-58);
DATA(Ptr,     CurrentA5,, 	0x904, TRUE  , MemoryMgr,      IMI-95);
DATA(Ptr,     CurStackBase,, 	0x908, TRUE-b, MemoryMgr,     IMII-19);
/*
 * NOTE: IMIII says CurApName is 32 bytes LONGINT, but it looks to me like
 * it is really 34 bytes LONGINT.
 */
DATA(Byte,           CurApName,[34], 	0x910, TRUE  , SegmentLdr,    IMII-58);
DATA(INTEGER,        CurJTOffset,, 	0x934, TRUE-b, SegmentLdr,    IMII-62);
DATA(INTEGER,        CurPageOption,, 	0x936, TRUE  , SegmentLdr,    IMII-60);
DATA(Byte,           HiliteMode,, 	0x938, TRUE-b, QuickDraw,         IMV);
/*
 * NOTE: Since we don't support printing yet, starting PrintErr at 0
 * is what we mean by supporting it.
 */
DATA(INTEGER,        PrintErr,, 	0x944, TRUE-b, PrintMgr,     IMII-161);
/*
 * NOTE: The graphing program looks for a -1 in 0x952
 */
DATA(INTEGER,        graphlooksat,, 	0x952, TRUE-b, rsys/misc,    WHOKNOWS);
/*
 * NOTE: MacWrite stores a copy of the trap address for LoadSeg in 954
 */
DATA(LONGINT,        macwritespace,, 	0x954, TRUE-b, rsys/misc,    WHOKNOWS);
DATA(LONGINT,        ScrapSize,, 	0x960, TRUE  , ScrapMgr,      IMI-457);
DATA(Handle,  ScrapHandle,, 	0x964, TRUE  , ScrapMgr,      IMI-457);
DATA(INTEGER,        ScrapCount,, 	0x968, TRUE  , ScrapMgr,      IMI-457);
DATA(INTEGER,        ScrapState,, 	0x96A, TRUE  , ScrapMgr,      IMI-457);
DATA(StringPtr, ScrapName,, 	0x96C, TRUE  , ScrapMgr,      IMI-457);
DATA(Handle,  ROMFont0,, 	0x980, TRUE  , FontMgr,       IMI-233);
DATA(INTEGER,        ApFontID,, 	0x984, TRUE  , FontMgr,       IMIV-31);
DATA(FMInput,        ROMlib_myfmi,, 	0x988, TRUE  , FontMgr,     ToolEqu.a);
DATA(FMOutput,       ROMlib_fmo,, 	0x998, TRUE  , FontMgr,     Private.a);
DATA(Byte,           ToolScratch,[8],	0x9CE, TRUE  , MemoryMgr,      IMI-85);
DATA(WindowPeek,WindowList,, 	0x9D6, TRUE  , WindowMgr,     IMI-274);
DATA(INTEGER,        SaveUpdate,, 	0x9DA, TRUE  , WindowMgr,     IMI-297);
DATA(INTEGER,        PaintWhite,, 	0x9DC, TRUE  , WindowMgr,     IMI-297);
DATA(GrafPtr, WMgrPort,, 	0x9DE, TRUE  , WindowMgr,     IMI-282);
DATA(RgnHandle,OldStructure,, 	0x9E6, TRUE  , WindowMgr,     IMI-296);
DATA(RgnHandle,OldContent,, 	0x9EA, TRUE  , WindowMgr,     IMI-296);
DATA(RgnHandle,GrayRgn,, 	0x9EE, TRUE  , WindowMgr,     IMI-282);
DATA(RgnHandle,SaveVisRgn,, 	0x9F2, TRUE  , WindowMgr,     IMI-293);
DATA(ProcPtr, DragHook,, 	0x9F6, TRUE  , WindowMgr,     IMI-324);
DATA(Byte,           Scratch8,[8], 	0x9FA, TRUE  , MemoryMgr,      IMI-85);
DATA(LONGINT,        OneOne,, 		0xA02, TRUE  , MemoryMgr,      IMI-85);
DATA(LONGINT,        MinusOne,, 	0xA06, TRUE  , MemoryMgr,      IMI-85);
DATA(INTEGER,        TopMenuItem,, 	0xA0A, TRUE  , MenuMgr,       IMV-249);
DATA(INTEGER,        AtMenuBottom,, 	0xA0C, TRUE  , MenuMgr,       IMV-249);
DATA(Handle,  MenuList,, 	0xA1C, TRUE  , MenuMgr,       IMI-346);
DATA(INTEGER,        MBarEnable,, 	0xA20, TRUE  , MenuMgr,       IMI-356);
DATA(INTEGER,        MenuFlash,, 	0xA24, TRUE  , MenuMgr,       IMI-361);
DATA(INTEGER,        TheMenu,, 		0xA26, TRUE  , MenuMgr,       IMI-357);
DATA(ProcPtr, MBarHook,, 	0xA2C, TRUE  , MenuMgr,       IMI-356);
DATA(ProcPtr, MenuHook,, 	0xA30, TRUE  , MenuMgr,       IMI-356);
DATA(Pattern,        DragPattern,, 	0xA34, TRUE  , WindowMgr,     IMI-324);
DATA(Pattern,        DeskPattern,, 	0xA3C, TRUE  , WindowMgr,     IMI-282);
DATA(Byte,           macfpstate,[6], 	0xA4A, TRUE-b, unknown,     ToolEqu.a);
DATA(Handle,  TopMapHndl,, 	0xA50, TRUE  , ResourceMgr,   IMI-115);
DATA(Handle,  SysMapHndl,, 	0xA54, TRUE  , ResourceMgr,   IMI-114);
DATA(INTEGER,        SysMap,, 		0xA58, TRUE  , ResourceMgr,   IMI-114);
DATA(INTEGER,        CurMap,, 		0xA5A, TRUE  , ResourceMgr,   IMI-117);
DATA(INTEGER,        resreadonly,, 	0xA5C, FALSE , ResourceMgr, ToolEqu.a);
DATA(BOOLEAN,        ResLoad,, 		0xA5E, TRUE  , ResourceMgr,   IMI-118);
DATA(INTEGER,        ResErr,, 		0xA60, TRUE  , ResourceMgr,   IMI-118);
DATA(Byte,           FScaleDisable,, 	0xA63, TRUE  , FontMgr,       IMI-222);
DATA(WindowPtr,CurActivate,, 	0xA64, TRUE  , WindowMgr,     IMI-280);
DATA(WindowPtr,CurDeactive,, 	0xA68, TRUE  , WindowMgr,     IMI-280);
DATA(ProcPtr, DeskHook,, 	0xA6C, TRUE  , WindowMgr,     IMI-282);
DATA(ProcPtr, TEDoText,,	0xA70, TRUE  , TextEdit,      IMI-391);
DATA(ProcPtr, TERecal,, 	0xA74, FALSE , TextEdit,      IMI-391);
DATA(Byte,           ApplScratch,[12],	0xA78, TRUE  , MemoryMgr,      IMI-85);
DATA(WindowPtr,GhostWindow,, 	0xA84, TRUE  , WindowMgr,     IMI-287);
DATA(ProcPtr, ResumeProc,, 	0xA8C, TRUE  , DialogMgr,     IMI-411);
DATA(INTEGER,        ANumber,, 		0xA98, TRUE  , DialogMgr,     IMI-423);
DATA(INTEGER,        ACount,, 		0xA9A, TRUE  , DialogMgr,     IMI-423);
DATA(ProcPtr, DABeeper,, 	0xA9C, TRUE  , DialogMgr,     IMI-411);
DATA(Handle,  DAStrings,[4], 	0xAA0, TRUE  , DialogMgr,     IMI-421);
DATA(INTEGER,        TEScrpLength,, 	0xAB0, TRUE  , TextEdit,      IMI-389);
DATA(Handle,  TEScrpHandle,, 	0xAB4, TRUE  , TextEdit,      IMI-389);
DATA(Handle,  AppPacks,[8], 	0xAB8, TRUE-b , PackageMgr,     ThinkC);
DATA(Byte,           SysResName,[20],	0xAD8, TRUE  , ResourceMgr,   IMI-114);
DATA(Handle,  AppParmHandle,, 	0xAEC, TRUE  , SegmentLdr,    IMII-57);
DATA(INTEGER,        DSErrCode,, 	0xAF0, TRUE  , MacTypes,     IMII-362);
DATA(ProcPtr, ResErrProc,, 	0xAF2, TRUE  , ResourceMgr,   IMI-116);
DATA(INTEGER,        DlgFont,, 		0xAFA, TRUE  , DialogMgr,     IMI-412);
DATA(WidthTablePtr,WidthPtr,, 	0xB10, TRUE  , FontMgr,       IMIV-42);
DATA(INTEGER,        SCSIFlags,, 	0xB22, TRUE-b, uknown,      Private.a);
DATA(WidthTableHandle,WidthTabHandle,,0xB2A,TRUE, FontMgr,   IMIV-42);
DATA(LONGINT,        LastSPExtra,, 	0xB4C, TRUE-b, rsys/misc,    WHOKNOWS);
DATA(LONGINT,        MenuDisable,, 	0xB54, TRUE  , MenuMgr,       IMV-249);
DATA(Handle,  MBDFHndl,, 	0xB58, TRUE  , MenuMgr,     Private.a);
DATA(Handle,  MBSaveLoc,, 	0xB5C, TRUE  , MenuMgr,     Private.a);
DATA(Byte,           RomMapInsert,, 	0xB9E, FALSE , ResourceMgr,   IMIV-19);
DATA(Byte,           TmpResLoad,, 	0xB9F, FALSE , ResourceMgr,   IMIV-19);
DATA(LONGINT,        IntlSpec,, 	0xBA0, TRUE  , FontMgr,       IMIV-42);
DATA(INTEGER,        SysFontFam,, 	0xBA6, TRUE  , FontMgr,       IMIV-31);
DATA(INTEGER,        SysFontSiz,, 	0xBA8, TRUE  , FontMgr,       IMIV-31);
DATA(INTEGER,        MBarHeight,, 	0xBAA, TRUE  , MenuMgr,       IMV-253);
DATA(INTEGER,        TESysJust,, 	0xBAC, TRUE-b, ScriptMgr,   ToolEqu.a);
DATA(FamRecHandle, LastFOND,, 	0xBC2, TRUE  , FontMgr,       IMIV-36);
DATA(INTEGER,        fondid,, 		0xBC6, TRUE-b, FontMgr,     ToolEqu.a);
DATA(Byte,           FractEnable,, 	0xBF4, TRUE  , FontMgr,       IMIV-32);
DATA(Byte,           MMUType,,          0xCB1, FALSE , OSUtil,            MPW);
DATA(Byte,           MMU32Bit,, 	0xCB2, TRUE-b, OSUtil,        IMV-592);
DATA(GDHandle,TheGDevice,, 	0xCC8, TRUE  , QuickDraw,         IMV);
DATA(AuxWinHandle,AuxWinHead,,	0xCD0, TRUE  , WindowMgr,     IMV-200);
DATA(AuxCtlHandle,AuxCtlHead,,	0xCD4, TRUE  , ControlMgr,    IMV-216);
DATA(PixPatHandle, DeskCPat,,  0xCD8, TRUE  , WindowMgr,   SysEqua.a);
DATA(INTEGER,        TimeDBRA,, 	0xD00, FALSE , StartMgr,          IMV);
DATA(INTEGER,        TimeSCCDB,, 	0xD02, FALSE , StartMgr,          IMV);
DATA(ProcPtr, JVBLTask,, 	0xD28, FALSE , VRetraceMgr,       IMV);
DATA(CGrafPtr,WMgrCPort,, 	0xD2C, FALSE , QuickDraw,     IMV-205);
DATA(Handle,  SynListHandle,, 	0xD32, FALSE , FontMgr,       IMV-182);
DATA(MCTableHandle,MenuCInfo,,	0xD50, TRUE  , QuickDraw,     IMV-242);
DATA(QHdr,           DTQueue,,		0xD92, FALSE , OSUtil,        IMV-466);
DATA(ProcPtr, JDTInstall,, 	0xD9C, FALSE , OSUtil,            IMV);
DATA(RGBColor,       HiliteRGB,,	0xDA0, TRUE  , QuickDraw,      IMV-62);
DATA(INTEGER,        TimeSCSIDB,,	0xDA6, FALSE , StartMgr,          IMV);
DATA(LONGINT,        lastlowglobal,,   0x2000, TRUE-b, rsys/misc,      MadeUp);

#endif
