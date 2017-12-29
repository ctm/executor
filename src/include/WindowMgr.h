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
#define documentProc 0
#define dBoxProc 1
#define plainDBox 2
#define altDBoxProc 3
#define noGrowDocProc 4
#define movableDBoxProc 5
#define rDocProc 16

#define dialogKind 2
#define userKind 8

#define inDesk 0
#define inMenuBar 1
#define inSysWindow 2
#define inContent 3
#define inDrag 4
#define inGrow 5
#define inGoAway 6

#define noConstraint 0
#define hAxisOnly 1
#define vAxisOnly 2

#define wDraw 0
#define wHit 1
#define wCalcRgns 2
#define wNew 3
#define wDispose 4
#define wGrow 5
#define wDrawGIcon 6

#define wNoHit 0
#define wInContent 1
#define wInDrag 2
#define wInGrow 3
#define wInGoAway 4

/* color table entries */

#define wContentColor 0
#define wFrameColor 1
#define wTextColor 2
#define wHiliteColor 3
#define wTitleBarColor 4
#define wHiliteColorLight 5
#define wHiliteColorDark 6
#define wTitleBarLight 7
#define wTitleBarDark 8
#define wDialogLight 9
#define wDialogDark 10
#define wTingeLight 11
#define wTingeDark 12

#define deskPatID 16

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

#define inZoomIn 7
#define inZoomOut 8

#define wInZoomIn 5
#define wInZoomOut 6

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

#define WindowList (WindowList_H.p)
#define WMgrPort (WMgrPort_H.p)
#define WMgrCPort (WMgrCPort_H.p)
#define OldStructure (OldStructure_H.p)
#define OldContent (OldContent_H.p)
#define GrayRgn (GrayRgn_H.p)
#define SaveVisRgn (SaveVisRgn_H.p)
#define DragHook (DragHook_H.p)
#define CurActivate (CurActivate_H.p)
#define CurDeactive (CurDeactive_H.p)
#define DeskHook (DeskHook_H.p)
#define GhostWindow (GhostWindow_H.p)
#define AuxWinHead (AuxWinHead_H.p)
#define DeskCPat (DeskCPat_H.p)
#endif

extern pascal trap void C_SetWTitle(WindowPtr w, StringPtr t);

extern pascal trap void C_GetWTitle(WindowPtr w, StringPtr t);

extern pascal trap WindowPtr C_FrontWindow(void);

extern pascal trap void C_HiliteWindow(WindowPtr w, BOOLEAN flag);

extern pascal trap void C_BringToFront(WindowPtr w);

extern pascal trap void C_SelectWindow(WindowPtr w);

extern pascal trap void C_ShowHide(WindowPtr w, BOOLEAN flag);

extern pascal trap void C_HideWindow(WindowPtr w);

extern pascal trap void C_ShowWindow(WindowPtr w);

extern pascal trap void C_SendBehind(WindowPtr w, WindowPtr behind);

extern pascal trap void C_DrawGrowIcon(WindowPtr w);

extern pascal trap void C_InitWindows(void);

extern pascal trap void C_GetWMgrPort(GUEST<GrafPtr> *wp);

extern pascal trap WindowPtr C_NewWindow(Ptr wst, Rect *r,
                                         StringPtr title, BOOLEAN vis, INTEGER procid, WindowPtr behind,
                                         BOOLEAN gaflag, LONGINT rc);
extern pascal trap WindowPtr C_GetNewWindow(INTEGER wid, Ptr wst,
                                            WindowPtr behind);
extern pascal trap void C_CloseWindow(WindowPtr w);

extern pascal trap void C_DisposeWindow(WindowPtr w);

extern pascal trap void C_SetWRefCon(WindowPtr w, LONGINT data);

extern pascal trap LONGINT C_GetWRefCon(WindowPtr w);

extern pascal trap void C_SetWindowPic(WindowPtr w, PicHandle p);

extern pascal trap PicHandle C_GetWindowPic(WindowPtr w);

extern pascal trap LONGINT C_PinRect(Rect *r, Point p);

extern pascal trap LONGINT C_DragTheRgn(RgnHandle rgn, Point startp,
                                        Rect *limit, Rect *slop, INTEGER axis, ProcPtr proc);
extern pascal trap LONGINT C_DragGrayRgn(RgnHandle rgn, Point startp,
                                         Rect *limit, Rect *slop, INTEGER axis, ProcPtr proc);
extern pascal trap void C_ClipAbove(WindowPeek w);

extern pascal trap BOOLEAN C_CheckUpdate(EventRecord *ev);

extern pascal trap void C_SaveOld(WindowPeek w);

extern pascal trap void C_PaintOne(WindowPeek w, RgnHandle clobbered);

extern pascal trap void C_PaintBehind(WindowPeek w, RgnHandle clobbered);

extern pascal trap void C_CalcVis(WindowPeek w);

extern pascal trap void C_CalcVisBehind(WindowPeek w,
                                        RgnHandle clobbered);
extern pascal trap void C_DrawNew(WindowPeek w, BOOLEAN flag);

extern pascal trap INTEGER C_GetWVariant(WindowPtr w);

extern pascal trap INTEGER C_FindWindow(Point p, GUEST<WindowPtr> *wpp);

extern pascal trap BOOLEAN C_TrackBox(WindowPtr wp,
                                      Point pt, INTEGER part);
extern pascal trap BOOLEAN C_TrackGoAway(WindowPtr w, Point p);

extern pascal trap void C_ZoomWindow(WindowPtr wp,
                                     INTEGER part, BOOLEAN front);
extern pascal trap void C_MoveWindow(WindowPtr wp, INTEGER h, INTEGER v,
                                     BOOLEAN front);
extern pascal trap void C_DragWindow(WindowPtr wp, Point p, Rect *rp);

extern pascal trap LONGINT C_GrowWindow(WindowPtr w, Point startp,
                                        Rect *rp);
extern pascal trap void C_SizeWindow(WindowPtr w, INTEGER width,
                                     INTEGER height, BOOLEAN flag);
extern pascal trap void C_InvalRect(Rect *r);

extern pascal trap void C_InvalRgn(RgnHandle r);

extern pascal trap void C_ValidRect(Rect *r);

extern pascal trap void C_ValidRgn(RgnHandle r);

extern pascal trap void C_BeginUpdate(WindowPtr w);

extern pascal trap void C_EndUpdate(WindowPtr w);

extern pascal trap void C_SetWinColor(WindowPtr w, CTabHandle new_w_ctab);
extern pascal trap void C_SetDeskCPat(PixPatHandle);

}
#endif /* __WINDOW__ */
