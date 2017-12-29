#if !defined(__LIST__)
#define __LIST__

/*
 * Copyright 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "ControlMgr.h"

namespace Executor
{
typedef Point Cell;

#if 1 || !defined(__alpha)
typedef Byte DataArray[32001];
#else /* defined(__alpha) */
typedef Byte DataArray;
// FIXME: #warning incorrect typedef to make gcc happy
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

enum
{
    lDoVAutoscroll = 2,
    lDoHAutoscroll = 1,
};

enum
{
    lOnlyOne = -128,
    lExtendDrag = 64,
    lNoDisjoint = 32,
};

enum
{
    lNoExtend = 16,
    lNoRect = 8,
    lUseSense = 4,
    lNoNilHilite = 2,
};

enum
{
    lInitMsg = 0,
    lDrawMsg = 1,
    lHiliteMsg = 2,
    lCloseMsg = 3,
};

extern pascal trap void C_LFind(GUEST<INTEGER> *offsetp,
                                GUEST<INTEGER> *lenp, Cell cell, ListHandle list);
PASCAL_FUNCTION(LFind);
extern pascal trap BOOLEAN C_LNextCell(BOOLEAN hnext,
                                       BOOLEAN vnext, GUEST<Cell> *cellp, ListHandle list);
PASCAL_FUNCTION(LNextCell);
extern pascal trap void C_LRect(Rect *cellrect,
                                Cell cell, ListHandle list);
PASCAL_FUNCTION(LRect);
extern pascal trap BOOLEAN
C_LSearch(Ptr dp,
          INTEGER dl, Ptr proc, GUEST<Cell> *cellp, ListHandle list);
extern pascal trap void C_LSize(INTEGER width,
                                INTEGER height, ListHandle list);
PASCAL_FUNCTION(LSize);
extern pascal trap INTEGER C_LAddColumn(INTEGER count,
                                        INTEGER coln, ListHandle list);
PASCAL_FUNCTION(LAddColumn);
extern pascal trap INTEGER C_LAddRow(INTEGER count,
                                     INTEGER rown, ListHandle list);
PASCAL_FUNCTION(LAddRow);
extern pascal trap void C_LDelColumn(INTEGER count,
                                     INTEGER coln, ListHandle list);
PASCAL_FUNCTION(LDelColumn);
extern pascal trap void C_LDelRow(INTEGER count,
                                  INTEGER rown, ListHandle list);
PASCAL_FUNCTION(LDelRow);
extern pascal trap ListHandle C_LNew(Rect *rview,
                                     Rect *bounds, Point csize, INTEGER proc, WindowPtr wind,
                                     BOOLEAN draw, BOOLEAN grow, BOOLEAN scrollh, BOOLEAN scrollv);
PASCAL_FUNCTION(LNew);
extern pascal trap void C_LDispose(ListHandle list);
PASCAL_FUNCTION(LDispose);
extern pascal trap void C_LDraw(Cell cell,
                                ListHandle list);
PASCAL_FUNCTION(LDraw);
extern pascal trap void C_LDoDraw(BOOLEAN draw,
                                  ListHandle list);
PASCAL_FUNCTION(LDoDraw);
extern pascal trap void C_LScroll(INTEGER ncol,
                                  INTEGER nrow, ListHandle list);
PASCAL_FUNCTION(LScroll);
extern pascal trap void C_LAutoScroll(ListHandle list);
PASCAL_FUNCTION(LAutoScroll);
extern pascal trap void C_LUpdate(RgnHandle rgn,
                                  ListHandle list);
PASCAL_FUNCTION(LUpdate);
extern pascal trap void C_LActivate(BOOLEAN act,
                                    ListHandle list);
PASCAL_FUNCTION(LActivate);
extern pascal void C_ROMlib_mytrack(ControlHandle ch, INTEGER part);

extern pascal trap BOOLEAN C_LClick(Point pt,
                                    INTEGER mods, ListHandle list);
PASCAL_FUNCTION(LClick);
extern pascal trap LONGINT C_LLastClick(ListHandle list);
PASCAL_FUNCTION(LLastClick);
extern pascal trap void C_LSetSelect(BOOLEAN setit,
                                     Cell cell, ListHandle list);
PASCAL_FUNCTION(LSetSelect);
extern pascal trap void C_LAddToCell(Ptr dp, INTEGER dl,
                                     Cell cell, ListHandle list);
PASCAL_FUNCTION(LAddToCell);
extern pascal trap void C_LClrCell(Cell cell,
                                   ListHandle list);
PASCAL_FUNCTION(LClrCell);
extern pascal trap void C_LGetCell(Ptr dp, GUEST<INTEGER> *dlp,
                                   Cell cell, ListHandle list);
PASCAL_FUNCTION(LGetCell);
extern pascal trap void C_LSetCell(Ptr dp, INTEGER dl,
                                   Cell cell, ListHandle list);
PASCAL_FUNCTION(LSetCell);
extern pascal trap void C_LCellSize(Point csize,
                                    ListHandle list);
PASCAL_FUNCTION(LCellSize);
extern pascal trap BOOLEAN C_LGetSelect(BOOLEAN next,
                                        GUEST<Cell> *cellp, ListHandle list);
PASCAL_FUNCTION(LGetSelect);
}
#endif /* __LIST__ */
