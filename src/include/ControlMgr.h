#if !defined(__CONTROL__)
#define __CONTROL__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
enum
{
    pushButProc = 0,
    checkBoxProc = 1,
    radioButProc = 2,
    useWFont = 8,
    scrollBarProc = 16,
};

enum
{
    inButton = 10,
    inCheckBox = 11,
    inUpButton = 20,
    inDownButton = 21,
    inPageUp = 22,
    inPageDown = 23,
    inThumb = 129,
};

enum
{
    popupFixedWidth = (1 << 0),
    popupUseAddResMenu = (1 << 2),
    popupUseWFont = (1 << 3),
};

enum
{
    popupTitleBold = (1 << 8),
    popupTitleItalic = (1 << 9),
    popupTitleUnderline = (1 << 10),
    popupTitleOutline = (1 << 11),
    popupTitleShadow = (1 << 12),
    popupTitleCondense = (1 << 13),
    popupTitleExtend = (1 << 14),
    popupTitleNoStyle = (1 << 15),
};

enum
{
    popupTitleLeftJust = (0x00),
    popupTitleCenterJust = (0x01),
    popupTitleRightJust = (0xFF),
};

enum
{
    drawCntl = 0,
    testCntl = 1,
    calcCRgns = 2,
    initCntl = 3,
    dispCntl = 4,
    posCntl = 5,
    thumbCntl = 6,
    dragCntl = 7,
    autoTrack = 8,
};
enum
{
    calcCntlRgn = 10,
    calcThumbRgn = 11,
};

/* control color table parts */
enum
{
    cFrameColor = 0,
    cBodyColor = 1,
    cTextColor = 2,
    cThumbColor = 3,
};
/* #define ???		4 */
enum
{
    cArrowsColorLight = 5,
    cArrowsColorDark = 6,
    cThumbLight = 7,
    cThumbDark = 8,
    cHiliteLight = 9,
    cHiliteDark = 10,
    cTitleBarLight = 11,
    cTitleBarDark = 12,
    cTingeLight = 13,
    cTingeDark = 14,
};

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

const LowMemGlobal<AuxCtlHandle> AuxCtlHead { 0xCD4 }; // ControlMgr IMV-216 (true);

extern void C_SetCTitle(ControlHandle c,
                                    StringPtr t);
PASCAL_TRAP(SetCTitle, 0xA95F);
extern void C_GetCTitle(ControlHandle c,
                                    StringPtr t);
PASCAL_TRAP(GetCTitle, 0xA95E);
extern void C_HideControl(ControlHandle c);
PASCAL_TRAP(HideControl, 0xA958);

extern void C_ShowControl(ControlHandle c);
PASCAL_TRAP(ShowControl, 0xA957);

extern void C_HiliteControl(ControlHandle c,
                                        INTEGER state);
PASCAL_TRAP(HiliteControl, 0xA95D);
extern void C_DrawControls(WindowPtr w);
PASCAL_TRAP(DrawControls, 0xA969);

extern void C_Draw1Control(ControlHandle c);
PASCAL_TRAP(Draw1Control, 0xA96D);

extern void C_UpdtControl(WindowPtr wp,
                                      RgnHandle rh);
PASCAL_TRAP(UpdtControl, 0xA953);
extern ControlHandle C_NewControl(WindowPtr wst, Rect *r,
                                              StringPtr title, BOOLEAN vis, INTEGER value, INTEGER min,
                                              INTEGER max, INTEGER procid, LONGINT rc);
PASCAL_TRAP(NewControl, 0xA954);
extern ControlHandle C_GetNewControl(
    INTEGER cid, WindowPtr wst);
PASCAL_TRAP(GetNewControl, 0xA9BE);
extern void C_DisposeControl(ControlHandle c);
PASCAL_TRAP(DisposeControl, 0xA955);

extern void C_KillControls(WindowPtr w);
PASCAL_TRAP(KillControls, 0xA956);

extern void C_SetCRefCon(ControlHandle c,
                                     LONGINT data);
PASCAL_TRAP(SetCRefCon, 0xA95B);
extern LONGINT C_GetCRefCon(ControlHandle c);
PASCAL_TRAP(GetCRefCon, 0xA95A);

extern void C_SetCtlAction(ControlHandle c,
                                       ProcPtr a);
PASCAL_TRAP(SetCtlAction, 0xA96B);
extern ProcPtr C_GetCtlAction(ControlHandle c);
PASCAL_TRAP(GetCtlAction, 0xA96A);

extern INTEGER C_GetCVariant(ControlHandle c);
PASCAL_TRAP(GetCVariant, 0xA809);

extern BOOLEAN C_GetAuxCtl(ControlHandle c,
                                       GUEST<AuxCtlHandle> *acHndl);
PASCAL_TRAP(GetAuxCtl, 0xAA44);
extern INTEGER C_FindControl(Point p,
                                         WindowPtr w, GUEST<ControlHandle> *cp);
PASCAL_TRAP(FindControl, 0xA96C);
extern INTEGER C_TrackControl(
    ControlHandle c, Point p, ProcPtr a);
PASCAL_TRAP(TrackControl, 0xA968);
extern INTEGER C_TestControl(
    ControlHandle c, Point p);
PASCAL_TRAP(TestControl, 0xA966);
extern void C_SetCtlValue(ControlHandle c,
                                      INTEGER v);
PASCAL_TRAP(SetCtlValue, 0xA963);
extern INTEGER C_GetCtlValue(
    ControlHandle c);
PASCAL_TRAP(GetCtlValue, 0xA960);
extern void C_SetCtlMin(ControlHandle c,
                                    INTEGER v);
PASCAL_TRAP(SetCtlMin, 0xA964);
extern INTEGER C_GetCtlMin(ControlHandle c);
PASCAL_TRAP(GetCtlMin, 0xA961);

extern void C_SetCtlMax(ControlHandle c,
                                    INTEGER v);
PASCAL_TRAP(SetCtlMax, 0xA965);
extern INTEGER C_GetCtlMax(ControlHandle c);
PASCAL_TRAP(GetCtlMax, 0xA962);

extern void C_MoveControl(ControlHandle c,
                                      INTEGER h, INTEGER v);
PASCAL_TRAP(MoveControl, 0xA959);
extern void C_DragControl(ControlHandle c,
                                      Point p, Rect *limit, Rect *slop, INTEGER axis);
PASCAL_TRAP(DragControl, 0xA967);
extern void C_SizeControl(ControlHandle c,
                                      INTEGER width, INTEGER height);
PASCAL_TRAP(SizeControl, 0xA95C);
extern void C_SetCtlColor(ControlHandle ctl, CCTabHandle ctab);
PASCAL_TRAP(SetCtlColor, 0xAA43);
}
#endif /* __CONTROL__ */
