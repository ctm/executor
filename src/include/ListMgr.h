#if !defined (__LIST__)
#define __LIST__

/*
 * Copyright 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: ListMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "ControlMgr.h"

typedef Point Cell;

#if 1 || !defined(__alpha)
typedef Byte DataArray[32001];
#else /* defined(__alpha) */
typedef Byte DataArray;
#warning incorrect typedef to make gcc happy
#endif /* defined(__alpha) */

typedef DataArray *DataPtr;
typedef struct { DataPtr p PACKED_P; } HIDDEN_DataPtr;
typedef HIDDEN_DataPtr *DataHandle;

typedef struct {
    Rect rView	LPACKED;
    GrafPtr port	PACKED_P;
    Point indent	LPACKED;
    Point cellSize	LPACKED;
    Rect visible	LPACKED;
    ControlHandle vScroll	PACKED_P;
    ControlHandle hScroll	PACKED_P;
    SignedByte selFlags	LPACKED;
    BOOLEAN lActive	LPACKED;
    SignedByte lReserved	LPACKED;
    SignedByte listFlags	LPACKED;
    LONGINT clikTime	PACKED;
    Point clikLoc	LPACKED;
    Point mouseLoc	LPACKED;
    Ptr lClikLoop	PACKED_P;
    Cell lastClick	LPACKED;
    LONGINT refCon	PACKED;
    Handle listDefProc	PACKED_P;
    Handle userHandle	PACKED_P;
    Rect dataBounds	LPACKED;
    DataHandle cells	PACKED_P;
    INTEGER maxIndex	PACKED;
    INTEGER cellArray[1]	PACKED;
} ListRec;
typedef ListRec *ListPtr;
typedef struct { ListPtr p PACKED_P; } HIDDEN_ListPtr;
typedef HIDDEN_ListPtr *ListHandle;

#define lDoVAutoscroll	2
#define lDoHAutoscroll	1

#define lOnlyOne	-128
#define lExtendDrag	64
#define lNoDisjoint	32

#define lNoExtend	16
#define lNoRect		8
#define lUseSense	4
#define lNoNilHilite	2

#define lInitMsg	0
#define lDrawMsg	1
#define lHiliteMsg	2
#define lCloseMsg	3


/* DO NOT DELETE THIS LINE */
#if !defined (__STDC__)
extern void LFind(); 
extern BOOLEAN LNextCell(); 
extern void LRect(); 
extern BOOLEAN LSearch(); 
extern void LSize(); 
extern INTEGER LAddColumn(); 
extern INTEGER LAddRow(); 
extern void LDelColumn(); 
extern void LDelRow(); 
extern ListHandle LNew(); 
extern void LDispose(); 
extern void LDraw(); 
extern void LDoDraw(); 
extern void LScroll(); 
extern void LAutoScroll(); 
extern void LUpdate(); 
extern void LActivate(); 
extern pascal void ROMlib_mytrack(); 
extern BOOLEAN LClick(); 
extern LONGINT LLastClick(); 
extern void LSetSelect(); 
extern void LAddToCell(); 
extern void LClrCell(); 
extern void LGetCell(); 
extern void LSetCell(); 
extern void LCellSize(); 
extern BOOLEAN LGetSelect(); 
#else /* __STDC__ */
extern pascal trap void C_LFind( INTEGER *offsetp, 
 INTEGER *lenp, Cell cell, ListHandle list );
extern pascal trap BOOLEAN C_LNextCell( BOOLEAN hnext, 
 BOOLEAN vnext, Cell *cellp, ListHandle list );
extern pascal trap void C_LRect( Rect *cellrect, 
 Cell cell, ListHandle list );
extern pascal trap BOOLEAN C_LSearch( Ptr dp, 
 INTEGER dl, Ptr proc, Cell *cellp, ListHandle list );
extern pascal trap void C_LSize( INTEGER width, 
 INTEGER height, ListHandle list );
extern pascal trap INTEGER C_LAddColumn( INTEGER count, 
 INTEGER coln, ListHandle list );
extern pascal trap INTEGER C_LAddRow( INTEGER count, 
 INTEGER rown, ListHandle list );
extern pascal trap void C_LDelColumn( INTEGER count, 
 INTEGER coln, ListHandle list );
extern pascal trap void C_LDelRow( INTEGER count, 
 INTEGER rown, ListHandle list );
extern pascal trap ListHandle C_LNew( Rect *rview, 
 Rect *bounds, Point csize, INTEGER proc, WindowPtr wind, 
 BOOLEAN draw, BOOLEAN grow, BOOLEAN scrollh, BOOLEAN scrollv );extern pascal trap ListHandle P_LNew( Rect *rview, 
 Rect *bounds, Point csize, INTEGER proc, WindowPtr wind, 
 BOOLEAN draw, BOOLEAN grow, BOOLEAN scrollh, BOOLEAN scrollv ); 
extern pascal trap void C_LDispose( ListHandle list );
extern pascal trap void C_LDraw( Cell cell, 
 ListHandle list );
extern pascal trap void C_LDoDraw( BOOLEAN draw, 
 ListHandle list );
extern pascal trap void C_LScroll( INTEGER ncol, 
 INTEGER nrow, ListHandle list );
extern pascal trap void C_LAutoScroll( ListHandle list );
extern pascal trap void C_LUpdate( RgnHandle rgn, 
 ListHandle list );
extern pascal trap void C_LActivate( BOOLEAN act, 
 ListHandle list );
extern pascal void C_ROMlib_mytrack( ControlHandle ch, INTEGER part );

extern pascal trap BOOLEAN C_LClick( Point pt, 
 INTEGER mods, ListHandle list );
extern pascal trap LONGINT C_LLastClick( ListHandle list );
extern pascal trap void C_LSetSelect( BOOLEAN setit, 
 Cell cell, ListHandle list );
extern pascal trap void C_LAddToCell( Ptr dp, INTEGER dl, 
 Cell cell, ListHandle list );
extern pascal trap void C_LClrCell( Cell cell, 
 ListHandle list );
extern pascal trap void C_LGetCell( Ptr dp, INTEGER *dlp, 
 Cell cell, ListHandle list );
extern pascal trap void C_LSetCell( Ptr dp, INTEGER dl, 
 Cell cell, ListHandle list );
extern pascal trap void C_LCellSize( Point csize, 
 ListHandle list );
extern pascal trap BOOLEAN C_LGetSelect( BOOLEAN next, 
 Cell *cellp, ListHandle list );
#endif /* __STDC__ */
#endif /* __LIST__ */
