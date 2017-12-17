#if !defined(__CONTROL__)
#define __CONTROL__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: ControlMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor
{
#define pushButProc 0
#define checkBoxProc 1
#define radioButProc 2
#define useWFont 8
#define scrollBarProc 16

#define inButton 10
#define inCheckBox 11
#define inUpButton 20
#define inDownButton 21
#define inPageUp 22
#define inPageDown 23
#define inThumb 129

#define popupFixedWidth (1 << 0)
#define popupUseAddResMenu (1 << 2)
#define popupUseWFont (1 << 3)

#define popupTitleBold (1 << 8)
#define popupTitleItalic (1 << 9)
#define popupTitleUnderline (1 << 10)
#define popupTitleOutline (1 << 11)
#define popupTitleShadow (1 << 12)
#define popupTitleCondense (1 << 13)
#define popupTitleExtend (1 << 14)
#define popupTitleNoStyle (1 << 15)

#define popupTitleLeftJust (0x00)
#define popupTitleCenterJust (0x01)
#define popupTitleRightJust (0xFF)

#define noConstraint 0
#define hAxisOnly 1
#define vAxisOnly 2

#define drawCntl 0
#define testCntl 1
#define calcCRgns 2
#define initCntl 3
#define dispCntl 4
#define posCntl 5
#define thumbCntl 6
#define dragCntl 7
#define autoTrack 8
enum
{
    calcCntlRgn = 10,
    calcThumbRgn = 11,
};

/* control color table parts */
#define cFrameColor 0
#define cBodyColor 1
#define cTextColor 2
#define cThumbColor 3
/* #define ???		4 */
#define cArrowsColorLight 5
#define cArrowsColorDark 6
#define cThumbLight 7
#define cThumbDark 8
#define cHiliteLight 9
#define cHiliteDark 10
#define cTitleBarLight 11
#define cTitleBarDark 12
#define cTingeLight 13
#define cTingeDark 14

typedef struct __cr ControlRecord;
typedef ControlRecord *ControlPtr;

typedef GUEST<ControlPtr> *ControlHandle;
}

#include "WindowMgr.h"

namespace Executor
{
struct __cr
{
    GUEST_STRUCT;
    GUEST<ControlHandle> nextControl;
    GUEST<WindowPtr> contrlOwner;
    GUEST<Rect> contrlRect;
    GUEST<Byte> contrlVis;
    GUEST<Byte> contrlHilite;
    GUEST<INTEGER> contrlValue;
    GUEST<INTEGER> contrlMin;
    GUEST<INTEGER> contrlMax;
    GUEST<Handle> contrlDefProc;
    GUEST<Handle> contrlData;
    GUEST<ProcPtr> contrlAction;
    GUEST<LONGINT> contrlRfCon;
    GUEST<Str255> contrlTitle;
};

typedef struct CtlCTab
{
    GUEST_STRUCT;
    GUEST<LONGINT> ccSeed;
    GUEST<INTEGER> ccReserved;
    GUEST<INTEGER> ctSize;
    GUEST<cSpecArray> ctTable;
} * CCTabPtr;

typedef GUEST<CCTabPtr> *CCTabHandle;

typedef struct AuxCtlRec *AuxCtlPtr;

typedef GUEST<AuxCtlPtr> *AuxCtlHandle;

struct AuxCtlRec
{
    GUEST_STRUCT;
    GUEST<AuxCtlHandle> acNext;
    GUEST<ControlHandle> acOwner;
    GUEST<CCTabHandle> acCTable;
    GUEST<INTEGER> acFlags;
    GUEST<LONGINT> acReserved;
    GUEST<LONGINT> acRefCon;
};

extern pascal trap void C_SetCTitle(ControlHandle c,
                                    StringPtr t);
extern pascal trap void P_SetCTitle(ControlHandle c,
                                    StringPtr t);
extern pascal trap void C_GetCTitle(ControlHandle c,
                                    StringPtr t);
extern pascal trap void P_GetCTitle(ControlHandle c,
                                    StringPtr t);
extern pascal trap void C_HideControl(ControlHandle c);
extern pascal trap void P_HideControl(ControlHandle c);
extern pascal trap void C_ShowControl(ControlHandle c);
extern pascal trap void P_ShowControl(ControlHandle c);
extern pascal trap void C_HiliteControl(ControlHandle c,
                                        INTEGER state);
extern pascal trap void P_HiliteControl(ControlHandle c,
                                        INTEGER state);
extern pascal trap void C_DrawControls(WindowPtr w);
extern pascal trap void P_DrawControls(WindowPtr w);
extern pascal trap void C_Draw1Control(ControlHandle c);
extern pascal trap void P_Draw1Control(ControlHandle c);
extern pascal trap void C_UpdtControl(WindowPtr wp,
                                      RgnHandle rh);
extern pascal trap void P_UpdtControl(WindowPtr wp,
                                      RgnHandle rh);
extern pascal trap ControlHandle C_NewControl(WindowPtr wst, Rect *r,
                                              StringPtr title, BOOLEAN vis, INTEGER value, INTEGER min,
                                              INTEGER max, INTEGER procid, LONGINT rc);
extern pascal trap ControlHandle P_NewControl(WindowPtr wst, Rect *r,
                                              StringPtr title, BOOLEAN vis, INTEGER value, INTEGER min,
                                              INTEGER max, INTEGER procid, LONGINT rc);
extern pascal trap ControlHandle C_GetNewControl(
    INTEGER cid, WindowPtr wst);
extern pascal trap ControlHandle P_GetNewControl(
    INTEGER cid, WindowPtr wst);
extern pascal trap void C_DisposeControl(ControlHandle c);
extern pascal trap void P_DisposeControl(ControlHandle c);
extern pascal trap void C_KillControls(WindowPtr w);
extern pascal trap void P_KillControls(WindowPtr w);
extern pascal trap void C_SetCRefCon(ControlHandle c,
                                     LONGINT data);
extern pascal trap void P_SetCRefCon(ControlHandle c,
                                     LONGINT data);
extern pascal trap LONGINT C_GetCRefCon(ControlHandle c);
extern pascal trap LONGINT P_GetCRefCon(ControlHandle c);
extern pascal trap void C_SetCtlAction(ControlHandle c,
                                       ProcPtr a);
extern pascal trap void P_SetCtlAction(ControlHandle c,
                                       ProcPtr a);
extern pascal trap ProcPtr C_GetCtlAction(ControlHandle c);
extern pascal trap ProcPtr P_GetCtlAction(ControlHandle c);
extern pascal trap INTEGER C_GetCVariant(ControlHandle c);
extern pascal trap INTEGER P_GetCVariant(ControlHandle c);
extern pascal trap BOOLEAN C_GetAuxCtl(ControlHandle c,
                                       GUEST<AuxCtlHandle> *acHndl);
extern pascal trap BOOLEAN P_GetAuxCtl(ControlHandle c,
                                       AuxCtlHandle acHndl);
extern pascal trap INTEGER C_FindControl(Point p,
                                         WindowPtr w, GUEST<ControlHandle> *cp);
extern pascal trap INTEGER P_FindControl(Point p,
                                         WindowPtr w, GUEST<ControlHandle> *cp);
extern pascal trap INTEGER C_TrackControl(
    ControlHandle c, Point p, ProcPtr a);
extern pascal trap INTEGER P_TrackControl(
    ControlHandle c, Point p, ProcPtr a);
extern pascal trap INTEGER C_TestControl(
    ControlHandle c, Point p);
extern pascal trap INTEGER P_TestControl(
    ControlHandle c, Point p);
extern pascal trap void C_SetCtlValue(ControlHandle c,
                                      INTEGER v);
extern pascal trap void P_SetCtlValue(ControlHandle c,
                                      INTEGER v);
extern pascal trap INTEGER C_GetCtlValue(
    ControlHandle c);
extern pascal trap INTEGER P_GetCtlValue(
    ControlHandle c);
extern pascal trap void C_SetCtlMin(ControlHandle c,
                                    INTEGER v);
extern pascal trap void P_SetCtlMin(ControlHandle c,
                                    INTEGER v);
extern pascal trap INTEGER C_GetCtlMin(ControlHandle c);
extern pascal trap INTEGER P_GetCtlMin(ControlHandle c);
extern pascal trap void C_SetCtlMax(ControlHandle c,
                                    INTEGER v);
extern pascal trap void P_SetCtlMax(ControlHandle c,
                                    INTEGER v);
extern pascal trap INTEGER C_GetCtlMax(ControlHandle c);
extern pascal trap INTEGER P_GetCtlMax(ControlHandle c);
extern pascal trap void C_MoveControl(ControlHandle c,
                                      INTEGER h, INTEGER v);
extern pascal trap void P_MoveControl(ControlHandle c,
                                      INTEGER h, INTEGER v);
extern pascal trap void C_DragControl(ControlHandle c,
                                      Point p, Rect *limit, Rect *slop, INTEGER axis);
extern pascal trap void P_DragControl(ControlHandle c,
                                      Point p, Rect *limit, Rect *slop, INTEGER axis);
extern pascal trap void C_SizeControl(ControlHandle c,
                                      INTEGER width, INTEGER height);
extern pascal trap void P_SizeControl(ControlHandle c,
                                      INTEGER width, INTEGER height);
extern pascal trap void C_SetCtlColor(ControlHandle ctl, CCTabHandle ctab);
}
#endif /* __CONTROL__ */
