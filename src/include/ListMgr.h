#if !defined(__LIST__)
#define __LIST__

/*
 * Copyright 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: ListMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "ControlMgr.h"

namespace Executor
{
typedef Point Cell;

#if 1 || !defined(__alpha)
typedef Byte DataArray[32001];
#else /* defined(__alpha) */
typedef Byte DataArray;
#warning incorrect typedef to make gcc happy
#endif /* defined(__alpha) */

typedef DataArray *DataPtr;

typedef GUEST<DataPtr> *DataHandle;

struct ListRec
{
    GUEST_STRUCT;
    GUEST<Rect> rView;
    GUEST<GrafPtr> port;
    GUEST<Point> indent;
    GUEST<Point> cellSize;
    GUEST<Rect> visible;
    GUEST<ControlHandle> vScroll;
    GUEST<ControlHandle> hScroll;
    GUEST<SignedByte> selFlags;
    GUEST<BOOLEAN> lActive;
    GUEST<SignedByte> lReserved;
    GUEST<SignedByte> listFlags;
    GUEST<LONGINT> clikTime;
    GUEST<Point> clikLoc;
    GUEST<Point> mouseLoc;
    GUEST<Ptr> lClikLoop;
    GUEST<Cell> lastClick;
    GUEST<LONGINT> refCon;
    GUEST<Handle> listDefProc;
    GUEST<Handle> userHandle;
    GUEST<Rect> dataBounds;
    GUEST<DataHandle> cells;
    GUEST<INTEGER> maxIndex;
    GUEST<INTEGER[1]> cellArray;
};
typedef ListRec *ListPtr;

typedef GUEST<ListPtr> *ListHandle;

#define lDoVAutoscroll 2
#define lDoHAutoscroll 1

#define lOnlyOne -128
#define lExtendDrag 64
#define lNoDisjoint 32

#define lNoExtend 16
#define lNoRect 8
#define lUseSense 4
#define lNoNilHilite 2

#define lInitMsg 0
#define lDrawMsg 1
#define lHiliteMsg 2
#define lCloseMsg 3

extern pascal trap void C_LFind(GUEST<INTEGER> *offsetp,
                                GUEST<INTEGER> *lenp, Cell cell, ListHandle list);
extern pascal trap BOOLEAN C_LNextCell(BOOLEAN hnext,
                                       BOOLEAN vnext, GUEST<Cell> *cellp, ListHandle list);
extern pascal trap void C_LRect(Rect *cellrect,
                                Cell cell, ListHandle list);
extern pascal trap BOOLEAN
C_LSearch(Ptr dp,
          INTEGER dl, Ptr proc, GUEST<Cell> *cellp, ListHandle list);
extern pascal trap void C_LSize(INTEGER width,
                                INTEGER height, ListHandle list);
extern pascal trap INTEGER C_LAddColumn(INTEGER count,
                                        INTEGER coln, ListHandle list);
extern pascal trap INTEGER C_LAddRow(INTEGER count,
                                     INTEGER rown, ListHandle list);
extern pascal trap void C_LDelColumn(INTEGER count,
                                     INTEGER coln, ListHandle list);
extern pascal trap void C_LDelRow(INTEGER count,
                                  INTEGER rown, ListHandle list);
extern pascal trap ListHandle C_LNew(Rect *rview,
                                     Rect *bounds, Point csize, INTEGER proc, WindowPtr wind,
                                     BOOLEAN draw, BOOLEAN grow, BOOLEAN scrollh, BOOLEAN scrollv);
extern pascal trap ListHandle P_LNew(Rect *rview,
                                     Rect *bounds, Point csize, INTEGER proc, WindowPtr wind,
                                     BOOLEAN draw, BOOLEAN grow, BOOLEAN scrollh, BOOLEAN scrollv);
extern pascal trap void C_LDispose(ListHandle list);
extern pascal trap void C_LDraw(Cell cell,
                                ListHandle list);
extern pascal trap void C_LDoDraw(BOOLEAN draw,
                                  ListHandle list);
extern pascal trap void C_LScroll(INTEGER ncol,
                                  INTEGER nrow, ListHandle list);
extern pascal trap void C_LAutoScroll(ListHandle list);
extern pascal trap void C_LUpdate(RgnHandle rgn,
                                  ListHandle list);
extern pascal trap void C_LActivate(BOOLEAN act,
                                    ListHandle list);
extern pascal void C_ROMlib_mytrack(ControlHandle ch, INTEGER part);

extern pascal trap BOOLEAN C_LClick(Point pt,
                                    INTEGER mods, ListHandle list);
extern pascal trap LONGINT C_LLastClick(ListHandle list);
extern pascal trap void C_LSetSelect(BOOLEAN setit,
                                     Cell cell, ListHandle list);
extern pascal trap void C_LAddToCell(Ptr dp, INTEGER dl,
                                     Cell cell, ListHandle list);
extern pascal trap void C_LClrCell(Cell cell,
                                   ListHandle list);
extern pascal trap void C_LGetCell(Ptr dp, GUEST<INTEGER> *dlp,
                                   Cell cell, ListHandle list);
extern pascal trap void C_LSetCell(Ptr dp, INTEGER dl,
                                   Cell cell, ListHandle list);
extern pascal trap void C_LCellSize(Point csize,
                                    ListHandle list);
extern pascal trap BOOLEAN C_LGetSelect(BOOLEAN next,
                                        GUEST<Cell> *cellp, ListHandle list);
}
#endif /* __LIST__ */
