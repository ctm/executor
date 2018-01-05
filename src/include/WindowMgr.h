#if !defined(__WINDOW__)
#define __WINDOW__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "QuickDraw.h"
#include "EventMgr.h"

namespace Executor
{
enum
{
    documentProc = 0,
    dBoxProc = 1,
    plainDBox = 2,
    altDBoxProc = 3,
    noGrowDocProc = 4,
    movableDBoxProc = 5,
    rDocProc = 16,
};

enum
{
    dialogKind = 2,
    userKind = 8,
};

enum
{
    inDesk = 0,
    inMenuBar = 1,
    inSysWindow = 2,
    inContent = 3,
    inDrag = 4,
    inGrow = 5,
    inGoAway = 6,
};

enum
{
    noConstraint = 0,
    hAxisOnly = 1,
    vAxisOnly = 2,
};

enum
{
    wDraw = 0,
    wHit = 1,
    wCalcRgns = 2,
    wNew = 3,
    wDispose = 4,
    wGrow = 5,
    wDrawGIcon = 6,
};

enum
{
    wNoHit = 0,
    wInContent = 1,
    wInDrag = 2,
    wInGrow = 3,
    wInGoAway = 4,
};

/* color table entries */

enum
{
    wContentColor = 0,
    wFrameColor = 1,
    wTextColor = 2,
    wHiliteColor = 3,
    wTitleBarColor = 4,
    wHiliteColorLight = 5,
    wHiliteColorDark = 6,
    wTitleBarLight = 7,
    wTitleBarDark = 8,
    wDialogLight = 9,
    wDialogDark = 10,
    wTingeLight = 11,
    wTingeDark = 12,
};

enum
{
    deskPatID = 16,
};

typedef GrafPtr WindowPtr;

typedef CGrafPtr CWindowPtr;
}
#include "ControlMgr.h"
namespace Executor
{
struct WindowRecord
{
    GUEST_STRUCT;
    GUEST<GrafPort> port;
    GUEST<INTEGER> windowKind;
    GUEST<BOOLEAN> visible;
    GUEST<BOOLEAN> hilited;
    GUEST<BOOLEAN> goAwayFlag;
    GUEST<BOOLEAN> spareFlag;
    GUEST<RgnHandle> strucRgn;
    GUEST<RgnHandle> contRgn;
    GUEST<RgnHandle> updateRgn;
    GUEST<Handle> windowDefProc;
    GUEST<Handle> dataHandle;
    GUEST<StringHandle> titleHandle;
    GUEST<INTEGER> titleWidth;
    GUEST<ControlHandle> controlList;
    GUEST<WindowRecord *> nextWindow;
    GUEST<PicHandle> windowPic;
    GUEST<LONGINT> refCon;
};
typedef WindowRecord *WindowPeek;

struct WStateData
{
    GUEST_STRUCT;
    GUEST<Rect> userState;
    GUEST<Rect> stdState;
};

enum
{
    inZoomIn = 7,
    inZoomOut = 8,
};

enum
{
    wInZoomIn = 5,
    wInZoomOut = 6,
};

typedef struct AuxWinRec *AuxWinPtr;

typedef GUEST<AuxWinPtr> *AuxWinHandle;

struct AuxWinRec
{
    GUEST_STRUCT;
    GUEST<AuxWinHandle> awNext;
    GUEST<WindowPtr> awOwner;
    GUEST<CTabHandle> awCTable;
    GUEST<Handle> dialogCItem;
    GUEST<LONGINT> awFlags;
    GUEST<CTabHandle> awReserved;
    GUEST<LONGINT> awRefCon;
};

const LowMemGlobal<WindowPeek> WindowList { 0x9D6 }; // WindowMgr IMI-274 (true);
const LowMemGlobal<INTEGER> SaveUpdate { 0x9DA }; // WindowMgr IMI-297 (true);
const LowMemGlobal<INTEGER> PaintWhite { 0x9DC }; // WindowMgr IMI-297 (true);
const LowMemGlobal<GrafPtr> WMgrPort { 0x9DE }; // WindowMgr IMI-282 (true);
const LowMemGlobal<CGrafPtr> WMgrCPort { 0xD2C }; // QuickDraw IMV-205 (false);
const LowMemGlobal<RgnHandle> OldStructure { 0x9E6 }; // WindowMgr IMI-296 (true);
const LowMemGlobal<RgnHandle> OldContent { 0x9EA }; // WindowMgr IMI-296 (true);
const LowMemGlobal<RgnHandle> GrayRgn { 0x9EE }; // WindowMgr IMI-282 (true);
const LowMemGlobal<RgnHandle> SaveVisRgn { 0x9F2 }; // WindowMgr IMI-293 (true);
const LowMemGlobal<ProcPtr> DragHook { 0x9F6 }; // WindowMgr IMI-324 (true);
const LowMemGlobal<Pattern> DragPattern { 0xA34 }; // WindowMgr IMI-324 (true);
const LowMemGlobal<Pattern> DeskPattern { 0xA3C }; // WindowMgr IMI-282 (true);
const LowMemGlobal<WindowPtr> CurActivate { 0xA64 }; // WindowMgr IMI-280 (true);
const LowMemGlobal<WindowPtr> CurDeactive { 0xA68 }; // WindowMgr IMI-280 (true);
const LowMemGlobal<ProcPtr> DeskHook { 0xA6C }; // WindowMgr IMI-282 (true);
const LowMemGlobal<WindowPtr> GhostWindow { 0xA84 }; // WindowMgr IMI-287 (true);
const LowMemGlobal<AuxWinHandle> AuxWinHead { 0xCD0 }; // WindowMgr IMV-200 (true);
const LowMemGlobal<PixPatHandle> DeskCPat { 0xCD8 }; // WindowMgr SysEqua.a (true);

extern void C_SetWTitle(WindowPtr w, StringPtr t);
PASCAL_TRAP(SetWTitle, 0xA91A);

extern void C_GetWTitle(WindowPtr w, StringPtr t);
PASCAL_TRAP(GetWTitle, 0xA919);

extern WindowPtr C_FrontWindow(void);
PASCAL_TRAP(FrontWindow, 0xA924);

extern void C_HiliteWindow(WindowPtr w, BOOLEAN flag);
PASCAL_TRAP(HiliteWindow, 0xA91C);

extern void C_BringToFront(WindowPtr w);
PASCAL_TRAP(BringToFront, 0xA920);

extern void C_SelectWindow(WindowPtr w);
PASCAL_TRAP(SelectWindow, 0xA91F);

extern void C_ShowHide(WindowPtr w, BOOLEAN flag);
PASCAL_TRAP(ShowHide, 0xA908);

extern void C_HideWindow(WindowPtr w);
PASCAL_TRAP(HideWindow, 0xA916);

extern void C_ShowWindow(WindowPtr w);
PASCAL_TRAP(ShowWindow, 0xA915);

extern void C_SendBehind(WindowPtr w, WindowPtr behind);
PASCAL_TRAP(SendBehind, 0xA921);

extern void C_DrawGrowIcon(WindowPtr w);
PASCAL_TRAP(DrawGrowIcon, 0xA904);

extern void C_InitWindows(void);
PASCAL_TRAP(InitWindows, 0xA912);

extern void C_GetWMgrPort(GUEST<GrafPtr> *wp);
PASCAL_TRAP(GetWMgrPort, 0xA910);

extern WindowPtr C_NewWindow(Ptr wst, Rect *r,
                                         StringPtr title, BOOLEAN vis, INTEGER procid, WindowPtr behind,
                                         BOOLEAN gaflag, LONGINT rc);
PASCAL_TRAP(NewWindow, 0xA913);
extern WindowPtr C_GetNewWindow(INTEGER wid, Ptr wst,
                                            WindowPtr behind);
PASCAL_TRAP(GetNewWindow, 0xA9BD);
extern void C_CloseWindow(WindowPtr w);
PASCAL_TRAP(CloseWindow, 0xA92D);

extern void C_DisposeWindow(WindowPtr w);
PASCAL_TRAP(DisposeWindow, 0xA914);

extern void C_SetWRefCon(WindowPtr w, LONGINT data);
PASCAL_TRAP(SetWRefCon, 0xA918);

extern LONGINT C_GetWRefCon(WindowPtr w);
PASCAL_TRAP(GetWRefCon, 0xA917);

extern void C_SetWindowPic(WindowPtr w, PicHandle p);
PASCAL_TRAP(SetWindowPic, 0xA92E);

extern PicHandle C_GetWindowPic(WindowPtr w);
PASCAL_TRAP(GetWindowPic, 0xA92F);

extern LONGINT C_PinRect(Rect *r, Point p);
PASCAL_TRAP(PinRect, 0xA94E);

extern LONGINT C_DragTheRgn(RgnHandle rgn, Point startp,
                                        Rect *limit, Rect *slop, INTEGER axis, ProcPtr proc);
PASCAL_TRAP(DragTheRgn, 0xA926);
extern LONGINT C_DragGrayRgn(RgnHandle rgn, Point startp,
                                         Rect *limit, Rect *slop, INTEGER axis, ProcPtr proc);
PASCAL_TRAP(DragGrayRgn, 0xA905);
extern void C_ClipAbove(WindowPeek w);
PASCAL_TRAP(ClipAbove, 0xA90B);

extern BOOLEAN C_CheckUpdate(EventRecord *ev);
PASCAL_TRAP(CheckUpdate, 0xA911);

extern void C_SaveOld(WindowPeek w);
PASCAL_TRAP(SaveOld, 0xA90E);

extern void C_PaintOne(WindowPeek w, RgnHandle clobbered);
PASCAL_TRAP(PaintOne, 0xA90C);

extern void C_PaintBehind(WindowPeek w, RgnHandle clobbered);
PASCAL_TRAP(PaintBehind, 0xA90D);

extern void C_CalcVis(WindowPeek w);
PASCAL_TRAP(CalcVis, 0xA909);

extern void C_CalcVisBehind(WindowPeek w,
                                        RgnHandle clobbered);
PASCAL_TRAP(CalcVisBehind, 0xA90A);
extern void C_DrawNew(WindowPeek w, BOOLEAN flag);
PASCAL_TRAP(DrawNew, 0xA90F);

extern INTEGER C_GetWVariant(WindowPtr w);
PASCAL_TRAP(GetWVariant, 0xA80A);

extern INTEGER C_FindWindow(Point p, GUEST<WindowPtr> *wpp);
PASCAL_TRAP(FindWindow, 0xA92C);

extern BOOLEAN C_TrackBox(WindowPtr wp,
                                      Point pt, INTEGER part);
PASCAL_TRAP(TrackBox, 0xA83B);
extern BOOLEAN C_TrackGoAway(WindowPtr w, Point p);
PASCAL_TRAP(TrackGoAway, 0xA91E);

extern void C_ZoomWindow(WindowPtr wp,
                                     INTEGER part, BOOLEAN front);
PASCAL_TRAP(ZoomWindow, 0xA83A);
extern void C_MoveWindow(WindowPtr wp, INTEGER h, INTEGER v,
                                     BOOLEAN front);
PASCAL_TRAP(MoveWindow, 0xA91B);
extern void C_DragWindow(WindowPtr wp, Point p, Rect *rp);
PASCAL_TRAP(DragWindow, 0xA925);

extern LONGINT C_GrowWindow(WindowPtr w, Point startp,
                                        Rect *rp);
PASCAL_TRAP(GrowWindow, 0xA92B);
extern void C_SizeWindow(WindowPtr w, INTEGER width,
                                     INTEGER height, BOOLEAN flag);
PASCAL_TRAP(SizeWindow, 0xA91D);
extern void C_InvalRect(Rect *r);
PASCAL_TRAP(InvalRect, 0xA928);

extern void C_InvalRgn(RgnHandle r);
PASCAL_TRAP(InvalRgn, 0xA927);

extern void C_ValidRect(Rect *r);
PASCAL_TRAP(ValidRect, 0xA92A);

extern void C_ValidRgn(RgnHandle r);
PASCAL_TRAP(ValidRgn, 0xA929);

extern void C_BeginUpdate(WindowPtr w);
PASCAL_TRAP(BeginUpdate, 0xA922);

extern void C_EndUpdate(WindowPtr w);
PASCAL_TRAP(EndUpdate, 0xA923);

extern void C_SetWinColor(WindowPtr w, CTabHandle new_w_ctab);
PASCAL_TRAP(SetWinColor, 0xAA41);
extern void C_SetDeskCPat(PixPatHandle);
PASCAL_TRAP(SetDeskCPat, 0xAA47);

extern BOOLEAN C_GetAuxWin(WindowPtr, GUEST<AuxWinHandle> *);
PASCAL_TRAP(GetAuxWin, 0xAA42);
}
#endif /* __WINDOW__ */
