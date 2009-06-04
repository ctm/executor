#if !defined (__CONTROL__)
#define __CONTROL__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: ControlMgr.h 63 2004-12-24 18:19:43Z ctm $
 */


#define pushButProc	0
#define checkBoxProc	1
#define radioButProc	2
#define useWFont	8
#define scrollBarProc	16

#define inButton	10
#define inCheckBox	11
#define inUpButton	20
#define inDownButton	21
#define inPageUp	22
#define inPageDown	23
#define inThumb		129

#define popupFixedWidth		(1 << 0)
#define popupUseAddResMenu	(1 << 2)
#define popupUseWFont		(1 << 3)

#define popupTitleBold		(1 << 8)
#define popupTitleItalic	(1 << 9)
#define popupTitleUnderline	(1 << 10)
#define popupTitleOutline	(1 << 11)
#define popupTitleShadow	(1 << 12)
#define popupTitleCondense	(1 << 13)
#define popupTitleExtend	(1 << 14)
#define popupTitleNoStyle	(1 << 15)

#define popupTitleLeftJust	(0x00)
#define popupTitleCenterJust	(0x01)
#define popupTitleRightJust	(0xFF)

#define noConstraint	0
#define hAxisOnly	1
#define vAxisOnly	2

#define drawCntl	0
#define testCntl	1
#define calcCRgns	2
#define initCntl	3
#define dispCntl	4
#define posCntl		5
#define thumbCntl	6
#define dragCntl	7
#define autoTrack	8
enum
{
  calcCntlRgn = 10,
  calcThumbRgn = 11,
};

/* control color table parts */
#define cFrameColor	0
#define cBodyColor	1
#define cTextColor	2
#define cThumbColor	3
/* #define ???		4 */
#define cArrowsColorLight	5
#define cArrowsColorDark	6
#define cThumbLight	7
#define cThumbDark 	8
#define cHiliteLight	9
#define cHiliteDark	10
#define cTitleBarLight	11
#define cTitleBarDark	12
#define cTingeLight	13
#define cTingeDark	14

typedef struct __cr ControlRecord;
typedef ControlRecord *ControlPtr;
typedef struct { ControlPtr p PACKED_P; } HIDDEN_ControlPtr;
typedef HIDDEN_ControlPtr *ControlHandle;
typedef struct { ControlHandle p PACKED_P; } HIDDEN_ControlHandle;

#include "WindowMgr.h"

struct __cr {
    ControlHandle nextControl	PACKED_P;	/* actually ControlHandle */
    WindowPtr contrlOwner	PACKED_P;
    Rect contrlRect	LPACKED;
    Byte contrlVis	LPACKED;
    Byte contrlHilite	LPACKED;
    INTEGER contrlValue	PACKED;
    INTEGER contrlMin	PACKED;
    INTEGER contrlMax	PACKED;
    Handle contrlDefProc	PACKED_P;
    Handle contrlData	PACKED_P;
    ProcPtr contrlAction	PACKED_P;
    LONGINT contrlRfCon	PACKED;
    Str255 contrlTitle	LPACKED;
};

typedef struct {
    LONGINT ccSeed	PACKED;
    INTEGER ccReserved	PACKED;
    INTEGER ctSize	PACKED;
    cSpecArray ctTable	LPACKED;
} CtlCTab, *CCTabPtr;
typedef struct { CCTabPtr p PACKED_P; } HIDDEN_CCTabPtr;
typedef HIDDEN_CCTabPtr *CCTabHandle;

typedef struct AuxCtlRec *AuxCtlPtr;
typedef struct { AuxCtlPtr p PACKED_P; } HIDDEN_AuxCtlPtr;
typedef HIDDEN_AuxCtlPtr *AuxCtlHandle;
typedef struct { AuxCtlHandle p PACKED_P; } HIDDEN_AuxCtlHandle;

typedef struct AuxCtlRec {
    AuxCtlHandle acNext		PACKED_P;
    ControlHandle acOwner	PACKED_P;
    CCTabHandle acCTable	PACKED_P;
    INTEGER acFlags		PACKED;
    LONGINT acReserved		PACKED;
    LONGINT acRefCon		PACKED;
} AuxCtlRec;

#if !defined (AuxCtlHead_H)
extern HIDDEN_AuxCtlHandle AuxCtlHead_H;
#endif

#define AuxCtlHead	(AuxCtlHead_H.p)

#if !defined (__STDC__)
extern void SetCTitle(); 
extern void GetCTitle(); 
extern void HideControl(); 
extern void ShowControl(); 
extern void HiliteControl(); 
extern void DrawControls(); 
extern void Draw1Control(); 
extern void UpdtControl(); 
extern ControlHandle NewControl(); 
extern ControlHandle GetNewControl(); 
extern void DisposeControl(); 
extern void KillControls(); 
extern void SetCRefCon(); 
extern LONGINT GetCRefCon(); 
extern void SetCtlAction(); 
extern ProcPtr GetCtlAction(); 
extern INTEGER GetCVariant(); 
extern BOOLEAN GetAuxCtl(); 
extern INTEGER FindControl(); 
extern INTEGER TrackControl(); 
extern INTEGER TestControl(); 
extern void SetCtlValue(); 
extern INTEGER GetCtlValue(); 
extern void SetCtlMin(); 
extern INTEGER GetCtlMin(); 
extern void SetCtlMax(); 
extern INTEGER GetCtlMax(); 
extern void MoveControl(); 
extern void DragControl(); 
extern void SizeControl(); 
#else /* __STDC__ */
extern pascal trap void C_SetCTitle( ControlHandle c, 
 StringPtr t ); extern pascal trap void P_SetCTitle( ControlHandle c, 
 StringPtr t ); 
extern pascal trap void C_GetCTitle( ControlHandle c, 
 StringPtr t ); extern pascal trap void P_GetCTitle( ControlHandle c, 
 StringPtr t ); 
extern pascal trap void C_HideControl( ControlHandle c ); extern pascal trap void P_HideControl( ControlHandle c); 
extern pascal trap void C_ShowControl( ControlHandle c ); extern pascal trap void P_ShowControl( ControlHandle c); 
extern pascal trap void C_HiliteControl( ControlHandle c, 
 INTEGER state ); extern pascal trap void P_HiliteControl( ControlHandle c, 
 INTEGER state ); 
extern pascal trap void C_DrawControls( WindowPtr w ); extern pascal trap void P_DrawControls( WindowPtr w); 
extern pascal trap void C_Draw1Control( ControlHandle c ); extern pascal trap void P_Draw1Control( ControlHandle c); 
extern pascal trap void C_UpdtControl( WindowPtr wp, 
 RgnHandle rh ); extern pascal trap void P_UpdtControl( WindowPtr wp, 
 RgnHandle rh ); 
extern pascal trap ControlHandle C_NewControl( WindowPtr wst, Rect *r, 
 StringPtr title, BOOLEAN vis, INTEGER value, INTEGER min, 
 INTEGER max, INTEGER procid, LONGINT rc );extern pascal trap ControlHandle P_NewControl( WindowPtr wst, Rect *r, 
 StringPtr title, BOOLEAN vis, INTEGER value, INTEGER min, 
 INTEGER max, INTEGER procid, LONGINT rc ); 
extern pascal trap ControlHandle C_GetNewControl( 
 INTEGER cid, WindowPtr wst ); extern pascal trap ControlHandle P_GetNewControl( 
 INTEGER cid, WindowPtr wst ); 
extern pascal trap void C_DisposeControl( ControlHandle c ); extern pascal trap void P_DisposeControl( ControlHandle c); 
extern pascal trap void C_KillControls( WindowPtr w ); extern pascal trap void P_KillControls( WindowPtr w); 
extern pascal trap void C_SetCRefCon( ControlHandle c, 
 LONGINT data ); extern pascal trap void P_SetCRefCon( ControlHandle c, 
 LONGINT data ); 
extern pascal trap LONGINT C_GetCRefCon( ControlHandle c ); extern pascal trap LONGINT P_GetCRefCon( ControlHandle c); 
extern pascal trap void C_SetCtlAction( ControlHandle c, 
 ProcPtr a ); extern pascal trap void P_SetCtlAction( ControlHandle c, 
 ProcPtr a ); 
extern pascal trap ProcPtr C_GetCtlAction( ControlHandle c ); extern pascal trap ProcPtr P_GetCtlAction( ControlHandle c); 
extern pascal trap INTEGER C_GetCVariant( ControlHandle c ); extern pascal trap INTEGER P_GetCVariant( ControlHandle c); 
extern pascal trap BOOLEAN C_GetAuxCtl( ControlHandle c, 
 HIDDEN_AuxCtlHandle *acHndl ); extern pascal trap BOOLEAN P_GetAuxCtl( ControlHandle c, 
 AuxCtlHandle acHndl ); 
extern pascal trap INTEGER C_FindControl( Point p, 
 WindowPtr w, HIDDEN_ControlHandle *cp ); extern pascal trap INTEGER P_FindControl( Point p, 
 WindowPtr w, HIDDEN_ControlHandle *cp ); 
extern pascal trap INTEGER C_TrackControl( 
 ControlHandle c, Point p, ProcPtr a ); extern pascal trap INTEGER P_TrackControl( 
 ControlHandle c, Point p, ProcPtr a ); 
extern pascal trap INTEGER C_TestControl( 
 ControlHandle c, Point p ); extern pascal trap INTEGER P_TestControl( 
 ControlHandle c, Point p ); 
extern pascal trap void C_SetCtlValue( ControlHandle c, 
 INTEGER v ); extern pascal trap void P_SetCtlValue( ControlHandle c, 
 INTEGER v ); 
extern pascal trap INTEGER C_GetCtlValue( 
 ControlHandle c ); extern pascal trap INTEGER P_GetCtlValue( 
 ControlHandle c ); 
extern pascal trap void C_SetCtlMin( ControlHandle c, 
 INTEGER v ); extern pascal trap void P_SetCtlMin( ControlHandle c, 
 INTEGER v ); 
extern pascal trap INTEGER C_GetCtlMin( ControlHandle c ); extern pascal trap INTEGER P_GetCtlMin( ControlHandle c); 
extern pascal trap void C_SetCtlMax( ControlHandle c, 
 INTEGER v ); extern pascal trap void P_SetCtlMax( ControlHandle c, 
 INTEGER v ); 
extern pascal trap INTEGER C_GetCtlMax( ControlHandle c ); extern pascal trap INTEGER P_GetCtlMax( ControlHandle c); 
extern pascal trap void C_MoveControl( ControlHandle c, 
 INTEGER h, INTEGER v ); extern pascal trap void P_MoveControl( ControlHandle c, 
 INTEGER h, INTEGER v ); 
extern pascal trap void C_DragControl( ControlHandle c, 
 Point p, Rect *limit, Rect *slop, INTEGER axis ); extern pascal trap void P_DragControl( ControlHandle c, 
 Point p, Rect *limit, Rect *slop, INTEGER axis ); 
extern pascal trap void C_SizeControl( ControlHandle c, 
 INTEGER width, INTEGER height ); extern pascal trap void P_SizeControl( ControlHandle c, 
 INTEGER width, INTEGER height );
extern pascal trap void C_SetCtlColor (ControlHandle ctl, CCTabHandle ctab);
#endif /* __STDC__ */
#endif /* __CONTROL__ */
