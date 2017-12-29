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

#if 0
#if !defined(WindowList_H)
extern GUEST<WindowPeek> 	WindowList_H;
extern GUEST<GrafPtr> 		WMgrPort_H;
extern GUEST<CGrafPtr> 		WMgrCPort_H;
extern GUEST<RgnHandle> 	OldStructure_H;
extern GUEST<RgnHandle> 	OldContent_H;
extern GUEST<RgnHandle> 	GrayRgn_H;
extern GUEST<RgnHandle> 	SaveVisRgn_H;
extern GUEST<ProcPtr> 		DragHook_H;
extern GUEST<WindowPtr> 	CurActivate_H;
extern GUEST<WindowPtr> 	CurDeactive_H;
extern GUEST<ProcPtr> 		DeskHook_H;
extern GUEST<WindowPtr> 	GhostWindow_H;
extern GUEST<AuxWinHandle>	AuxWinHead_H;
extern GUEST<PixPatHandle> DeskCPat_H;
extern INTEGER 	SaveUpdate;
extern INTEGER 	PaintWhite;
extern Pattern 	DragPattern;
extern Pattern 	DeskPattern;
#endif

enum
{
    WindowList = (WindowList_H.p),
    WMgrPort = (WMgrPort_H.p),
    WMgrCPort = (WMgrCPort_H.p),
    OldStructure = (OldStructure_H.p),
    OldContent = (OldContent_H.p),
    GrayRgn = (GrayRgn_H.p),
    SaveVisRgn = (SaveVisRgn_H.p),
    DragHook = (DragHook_H.p),
    CurActivate = (CurActivate_H.p),
    CurDeactive = (CurDeactive_H.p),
    DeskHook = (DeskHook_H.p),
    GhostWindow = (GhostWindow_H.p),
    AuxWinHead = (AuxWinHead_H.p),
    DeskCPat = (DeskCPat_H.p),
};
#endif

extern pascal trap void C_SetWTitle(WindowPtr w, StringPtr t);
PASCAL_TRAP(SetWTitle, 0xA91A);

extern pascal trap void C_GetWTitle(WindowPtr w, StringPtr t);
PASCAL_TRAP(GetWTitle, 0xA919);

extern pascal trap WindowPtr C_FrontWindow(void);
PASCAL_TRAP(FrontWindow, 0xA924);

extern pascal trap void C_HiliteWindow(WindowPtr w, BOOLEAN flag);
PASCAL_TRAP(HiliteWindow, 0xA91C);

extern pascal trap void C_BringToFront(WindowPtr w);
PASCAL_TRAP(BringToFront, 0xA920);

extern pascal trap void C_SelectWindow(WindowPtr w);
PASCAL_TRAP(SelectWindow, 0xA91F);

extern pascal trap void C_ShowHide(WindowPtr w, BOOLEAN flag);
PASCAL_TRAP(ShowHide, 0xA908);

extern pascal trap void C_HideWindow(WindowPtr w);
PASCAL_TRAP(HideWindow, 0xA916);

extern pascal trap void C_ShowWindow(WindowPtr w);
PASCAL_TRAP(ShowWindow, 0xA915);

extern pascal trap void C_SendBehind(WindowPtr w, WindowPtr behind);
PASCAL_TRAP(SendBehind, 0xA921);

extern pascal trap void C_DrawGrowIcon(WindowPtr w);
PASCAL_TRAP(DrawGrowIcon, 0xA904);

extern pascal trap void C_InitWindows(void);
PASCAL_TRAP(InitWindows, 0xA912);

extern pascal trap void C_GetWMgrPort(GUEST<GrafPtr> *wp);
PASCAL_TRAP(GetWMgrPort, 0xA910);

extern pascal trap WindowPtr C_NewWindow(Ptr wst, Rect *r,
                                         StringPtr title, BOOLEAN vis, INTEGER procid, WindowPtr behind,
                                         BOOLEAN gaflag, LONGINT rc);
PASCAL_TRAP(NewWindow, 0xA913);
extern pascal trap WindowPtr C_GetNewWindow(INTEGER wid, Ptr wst,
                                            WindowPtr behind);
PASCAL_TRAP(GetNewWindow, 0xA9BD);
extern pascal trap void C_CloseWindow(WindowPtr w);
PASCAL_TRAP(CloseWindow, 0xA92D);

extern pascal trap void C_DisposeWindow(WindowPtr w);
PASCAL_TRAP(DisposeWindow, 0xA914);

extern pascal trap void C_SetWRefCon(WindowPtr w, LONGINT data);
PASCAL_TRAP(SetWRefCon, 0xA918);

extern pascal trap LONGINT C_GetWRefCon(WindowPtr w);
PASCAL_TRAP(GetWRefCon, 0xA917);

extern pascal trap void C_SetWindowPic(WindowPtr w, PicHandle p);
PASCAL_TRAP(SetWindowPic, 0xA92E);

extern pascal trap PicHandle C_GetWindowPic(WindowPtr w);
PASCAL_TRAP(GetWindowPic, 0xA92F);

extern pascal trap LONGINT C_PinRect(Rect *r, Point p);
PASCAL_TRAP(PinRect, 0xA94E);

extern pascal trap LONGINT C_DragTheRgn(RgnHandle rgn, Point startp,
                                        Rect *limit, Rect *slop, INTEGER axis, ProcPtr proc);
PASCAL_TRAP(DragTheRgn, 0xA926);
extern pascal trap LONGINT C_DragGrayRgn(RgnHandle rgn, Point startp,
                                         Rect *limit, Rect *slop, INTEGER axis, ProcPtr proc);
PASCAL_TRAP(DragGrayRgn, 0xA905);
extern pascal trap void C_ClipAbove(WindowPeek w);
PASCAL_TRAP(ClipAbove, 0xA90B);

extern pascal trap BOOLEAN C_CheckUpdate(EventRecord *ev);
PASCAL_TRAP(CheckUpdate, 0xA911);

extern pascal trap void C_SaveOld(WindowPeek w);
PASCAL_TRAP(SaveOld, 0xA90E);

extern pascal trap void C_PaintOne(WindowPeek w, RgnHandle clobbered);
PASCAL_TRAP(PaintOne, 0xA90C);

extern pascal trap void C_PaintBehind(WindowPeek w, RgnHandle clobbered);
PASCAL_TRAP(PaintBehind, 0xA90D);

extern pascal trap void C_CalcVis(WindowPeek w);
PASCAL_TRAP(CalcVis, 0xA909);

extern pascal trap void C_CalcVisBehind(WindowPeek w,
                                        RgnHandle clobbered);
PASCAL_TRAP(CalcVisBehind, 0xA90A);
extern pascal trap void C_DrawNew(WindowPeek w, BOOLEAN flag);
PASCAL_TRAP(DrawNew, 0xA90F);

extern pascal trap INTEGER C_GetWVariant(WindowPtr w);
PASCAL_TRAP(GetWVariant, 0xA80A);

extern pascal trap INTEGER C_FindWindow(Point p, GUEST<WindowPtr> *wpp);
PASCAL_TRAP(FindWindow, 0xA92C);

extern pascal trap BOOLEAN C_TrackBox(WindowPtr wp,
                                      Point pt, INTEGER part);
PASCAL_TRAP(TrackBox, 0xA83B);
extern pascal trap BOOLEAN C_TrackGoAway(WindowPtr w, Point p);
PASCAL_TRAP(TrackGoAway, 0xA91E);

extern pascal trap void C_ZoomWindow(WindowPtr wp,
                                     INTEGER part, BOOLEAN front);
PASCAL_TRAP(ZoomWindow, 0xA83A);
extern pascal trap void C_MoveWindow(WindowPtr wp, INTEGER h, INTEGER v,
                                     BOOLEAN front);
PASCAL_TRAP(MoveWindow, 0xA91B);
extern pascal trap void C_DragWindow(WindowPtr wp, Point p, Rect *rp);
PASCAL_TRAP(DragWindow, 0xA925);

extern pascal trap LONGINT C_GrowWindow(WindowPtr w, Point startp,
                                        Rect *rp);
PASCAL_TRAP(GrowWindow, 0xA92B);
extern pascal trap void C_SizeWindow(WindowPtr w, INTEGER width,
                                     INTEGER height, BOOLEAN flag);
PASCAL_TRAP(SizeWindow, 0xA91D);
extern pascal trap void C_InvalRect(Rect *r);
PASCAL_TRAP(InvalRect, 0xA928);

extern pascal trap void C_InvalRgn(RgnHandle r);
PASCAL_TRAP(InvalRgn, 0xA927);

extern pascal trap void C_ValidRect(Rect *r);
PASCAL_TRAP(ValidRect, 0xA92A);

extern pascal trap void C_ValidRgn(RgnHandle r);
PASCAL_TRAP(ValidRgn, 0xA929);

extern pascal trap void C_BeginUpdate(WindowPtr w);
PASCAL_TRAP(BeginUpdate, 0xA922);

extern pascal trap void C_EndUpdate(WindowPtr w);
PASCAL_TRAP(EndUpdate, 0xA923);

extern pascal trap void C_SetWinColor(WindowPtr w, CTabHandle new_w_ctab);
PASCAL_TRAP(SetWinColor, 0xAA41);
extern pascal trap void C_SetDeskCPat(PixPatHandle);
PASCAL_TRAP(SetDeskCPat, 0xAA47);

extern pascal trap BOOLEAN C_GetAuxWin(WindowPtr, GUEST<AuxWinHandle> *);
PASCAL_TRAP(GetAuxWin, 0xAA42);
}
#endif /* __WINDOW__ */
