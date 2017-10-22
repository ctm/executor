/* Copyright 1989 - 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_listAddDel[] =
	    "$Id: listAddDel.c 74 2004-12-30 03:38:55Z ctm $";
#endif

/* Forward declarations in ListMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ListMgr.h"
#include "MemoryMgr.h"
#include "rsys/list.h"

using namespace Executor;

P3(PUBLIC pascal trap, INTEGER, LAddColumn, INTEGER, count,	/* IMIV-271 */
					      INTEGER, coln, ListHandle, list)
{
    Rect todraw;
    Point c;
    INTEGER ncols, nrows, noffsets, offset;
    Size nbefore, nafter;
    Ptr ip, op;
    int i;
    Point p;

    nrows = Hx(list, dataBounds.bottom) - Hx(list, dataBounds.top);
    noffsets = count * nrows;
    if (coln > Hx(list, dataBounds.right))
	coln = Hx(list, dataBounds.right);

    if (coln < Hx(list, dataBounds.left))
	coln = Hx(list, dataBounds.left);

    HxX(list, dataBounds.right) = CW(Hx(list, dataBounds.right) + count);

    if (noffsets) {
	TRAPBEGIN();
	SetHandleSize((Handle) list,
		    GetHandleSize((Handle) list) + noffsets * sizeof(INTEGER));
	ncols = Hx(list, dataBounds.right) - Hx(list, dataBounds.left);
	nbefore = (coln - Hx(list, dataBounds.left)) * sizeof(INTEGER);
	nafter  = (ncols - count) * sizeof(INTEGER) - nbefore;
	ip = (Ptr) (HxX(list, cellArray) + nrows * (ncols - count) + 1);
	op = (Ptr) (HxX(list, cellArray) + nrows * ncols + 1);
	op -= sizeof(INTEGER);
	ip -= sizeof(INTEGER);
	*(INTEGER *)op = *(INTEGER *)ip;	/* sentinel */
	/* SPEEDUP:  merge the two BlockMoves ... (unroll begining and end) */
	while (--nrows >= 0) {
	    ip -= nafter;
	    op -= nafter;
	    BlockMove(ip, op, nafter);
	    offset = CW(*(GUEST<INTEGER> *)op) & 0x7FFF;
	    for (i = 0; ++i <= count; )
	      {
		op -= sizeof(INTEGER);
		*(GUEST<INTEGER> *)op = CW(offset);
	      }
	    ip -= nbefore;
	    op -= nbefore;
	    BlockMove(ip, op, nbefore);
	}

	p.h = Hx(list, cellSize.h);
	p.v = Hx(list, cellSize.v);
	C_LCellSize(p, list);		/* recalcs visible */

	if (Hx(list, listFlags) & DODRAW) {
	    todraw = HxX(list, dataBounds);
	    todraw.left = CW(coln);
	    SectRect(&todraw, &HxX(list, visible), &todraw);
	    for (c.v = CW(todraw.top) ; c.v < CW(todraw.bottom); c.v++)
		for (c.h = CW(todraw.left) ; c.h < CW(todraw.right); c.h++)
		    C_LDraw(c, list);
	}
	TRAPEND();
    }
    return coln;
}

P3(PUBLIC pascal trap, INTEGER, LAddRow, INTEGER, count,	/* IMIV-271 */
					      INTEGER, rown, ListHandle, list)
{
    Rect todraw;
    Cell c;
    INTEGER ncols, nrows, noffsets, offset;
    Size nbefore, nafter;
    GUEST<INTEGER> *ip, *op;
    Point p;

    ncols = Hx(list, dataBounds.right) - Hx(list, dataBounds.left);
    noffsets = count * ncols;
    if (rown > Hx(list, dataBounds.bottom))
	rown = Hx(list, dataBounds.bottom);

    if (rown < Hx(list, dataBounds.top))
      rown = Hx(list, dataBounds.top);

    HxX(list, dataBounds.bottom) = CW(Hx(list, dataBounds.bottom) + count);

    if (noffsets) {
	TRAPBEGIN();
	SetHandleSize((Handle) list,
		    GetHandleSize((Handle) list) + noffsets * sizeof(INTEGER));
	nrows = Hx(list, dataBounds.bottom) - Hx(list, dataBounds.top);
	nbefore = (rown - Hx(list, dataBounds.top));
	nafter  = ((nrows - count) - nbefore) * ncols;
	ip = HxX(list, cellArray) + (nrows - count) * ncols + 1;
	op = HxX(list, cellArray) + nrows * ncols + 1;
	*--op = *--ip;	/* sentinel */
	ip -= nafter;
	op -= nafter;
	offset = CW(*ip) & 0x7FFF;
	BlockMove((Ptr) ip, (Ptr) op, nafter * sizeof(INTEGER));
						    /* move the after rows */
	while (--noffsets >= 0)
	    *--op = CW(offset);

	p.h = Hx(list, cellSize.h);
	p.v = Hx(list, cellSize.v);
	C_LCellSize(p, list);		/* recalcs visible */

	if (Hx(list, listFlags) & DODRAW) {
	    todraw = HxX(list, dataBounds);
	    todraw.top = CW(rown);
	    SectRect(&todraw, &HxX(list, visible), &todraw);
	    for (c.v = CW(todraw.top) ; c.v < CW(todraw.bottom); c.v++)
		for (c.h = CW(todraw.left) ; c.h < CW(todraw.right); c.h++)
		    C_LDraw(c, list);
	}
	TRAPEND();
    }
    return rown;
}

PRIVATE void
compute_visible_rect (Rect *rp, ListHandle list, INTEGER top, INTEGER left,
     INTEGER bottom, INTEGER right)
{
  INTEGER h, v;
  INTEGER new_top, new_left, new_bottom, new_right;

  h = Hx (list, cellSize.h);
  v = Hx (list, cellSize.v);

  new_top    = Hx (list, rView.top ) + (top  - Hx (list, visible.top )) * v;
  new_left   = Hx (list, rView.left) + (left - Hx (list, visible.left)) * h;
  new_bottom = new_top  + (bottom - top ) * v;
  new_right  = new_left + (right  - left) * h;

  rp->top    = CW (new_top);
  rp->left   = CW (new_left);
  rp->bottom = CW (new_bottom);
  rp->right  = CW (new_right);

  SectRect (rp, &HxX (list, rView), rp);
}

P3(PUBLIC pascal trap, void, LDelColumn, INTEGER, count,	/* IMIV-271 */
					      INTEGER, coln, ListHandle, list)
{
    Rect todraw;
    Cell c;
    INTEGER ncols, nrows, noffsets, delta;
    Size nbefore, nafter, ntomove;
    GUEST<uint16_t> *ip, *op;	/* unsigned INTEGER */
    Ptr dataip, dataop;
    INTEGER off1, off2, off3, off4, off5;
    int i;
    ControlHandle control;
    Point p;

    if (!list || coln >= Hx(list, dataBounds.right))
/*-->*/	return;			/* invalid */

    if (count == 0 || (coln == Hx(list, dataBounds.left) &&
	      count >= Hx(list, dataBounds.right) - Hx(list, dataBounds.left))) {
	SetHandleSize((Handle) HxP(list, cells), (Size) 0);
	SetHandleSize((Handle) list, sizeof(ListRec));
	HxX(list, cellArray)[0] = 0;
	HxX(list, dataBounds.right) = HxX(list, dataBounds.left);
	HxX(list, visible.left)  = HxX(list, dataBounds.left);
	p.h = Hx(list, cellSize.h);
	p.v = Hx(list, cellSize.v);
	C_LCellSize(p, list);		/* recalcs visible */
	if (Hx(list, listFlags) & DODRAW)
	    EraseRect(&HxX(list, rView));
	if ((control = HxP(list, hScroll)))
	    SetCtlMax(control, Hx(control, contrlMin));
/*-->*/	return;		/* quick delete of everything */
    }

    if (coln + count > Hx(list, dataBounds.right))
	count = Hx(list, dataBounds.right) - coln;

    nrows = Hx(list, dataBounds.bottom) - Hx(list, dataBounds.top);
    noffsets = count * nrows;
    if (coln > Hx(list, dataBounds.right))
	coln = Hx(list, dataBounds.right);

    HxX(list, dataBounds.right) = CW(Hx(list, dataBounds.right) - count);

    if (noffsets) {
        INTEGER visible_right, bounds_right;

	TRAPBEGIN();
	ncols = Hx(list, dataBounds.right) - Hx(list, dataBounds.left);
	nbefore = (coln - Hx(list, dataBounds.left));
	nafter  = ncols - nbefore;
	ip = op = (GUEST<uint16_t> *) HxX(list, cellArray);
	dataip = dataop = (Ptr) STARH(HxP(list, cells));
	delta = 0;
	/* SPEEDUP:  partial loop unrolling ... combine things and don't
		     bother adding delta when we know that it's zero */
	while (--nrows >= 0) {
	    off1 = CW(*ip) & 0x7FFF;
	    for (i = nbefore; --i >= 0; )	/* copy before-offsets */
		*op++ = CW(CW(*ip++) - delta);

	    off2 = CW(*ip) & 0x7FFF;
	    ntomove = off2 - off1;
	    BlockMove(dataip, dataop, ntomove);	/* copy before-data */
	    dataip += ntomove;
	    dataop += ntomove;

	    ip += count;			/* skip count offsets */

	    off3 = CW(*ip) & 0x7FFF;
	    ntomove = off3 - off2;
	    dataip += ntomove;			/* skip appropriate data */
	    delta  += ntomove;			/* note this */

	    off4 = CW(*ip) & 0x7FFF;
	    for (i = nafter; --i >= 0; )	/* copy after-offsets */
		*op++ = CW(CW(*ip++) - delta);

	    off5 = CW(*ip) & 0x7FFF;
	    ntomove = off5 - off4;
	    BlockMove(dataip, dataop, ntomove);	/* copy before-data */
	    dataip += ntomove;
	    dataop += ntomove;
	}
	*op++ = CW(CW(*ip++) - delta);	/* sentinel */
	SetHandleSize((Handle) list,
		    GetHandleSize((Handle) list) - noffsets * sizeof(INTEGER));
	SetHandleSize((Handle) HxP(list, cells),
			       GetHandleSize((Handle) HxP(list, cells)) - delta);

	p.h = Hx(list, cellSize.h);
	p.v = Hx(list, cellSize.v);


	/* save visible_right and bounds_right now, because LCellSize
	   will adjust them and we won't be able to figure out if we
	   needed to force a scroll */

	visible_right = Hx(list, visible.right);
	bounds_right = Hx(list, dataBounds.right);
	C_LCellSize(p, list);		/* recalcs visible */
	if ((control = HxP(list, hScroll))) {
	    INTEGER visible_left;

	    visible_left = Hx(list, visible.left);

	    /* Determine whether or not we need to scroll up one location
	       (because we were maximally scrolled down and we deleted
	       something that was visible) */

	    if (visible_left > 0 && visible_right > bounds_right) {
	      --visible_left;
	      HxX(list, visible.left) = CW (visible_left);
	      coln = visible_left;
	    }
	}

	if (Hx(list, listFlags) & DODRAW) {
	    Rect eraser;

	    compute_visible_rect (&eraser, list, Hx (list, visible.top),
				  Hx (list, visible.right),
				  Hx (list, visible.bottom),
				  visible_right);
	    EraseRect (&eraser);

	    todraw = HxX(list, dataBounds);
	    todraw.left = CW(coln);
	    SectRect(&todraw, &HxX(list, visible), &todraw);
	    for (c.v = CW(todraw.top) ; c.v < CW(todraw.bottom); c.v++)
		for (c.h = CW(todraw.left) ; c.h < CW(todraw.right); c.h++)
		    C_LDraw(c, list);
	}
	TRAPEND();
    }
}

P3(PUBLIC pascal trap, void, LDelRow, INTEGER, count,		/* IMIV-272 */
					      INTEGER, rown, ListHandle, list)
{
    Rect todraw;
    Cell c;
    INTEGER ncols, nrows, noffsets;
    Size nbefore, nafter;
    GUEST<uint16_t> *ip, *op;
    ControlHandle control;
    INTEGER off1, off2, off3;
    Size delta, ntomove;
    Point p;

    if (!list || rown >= Hx(list, dataBounds.bottom))
/*-->*/	return;			/* invalid */

    if (count == 0 || (rown == Hx(list, dataBounds.top) &&
	      count >= Hx(list, dataBounds.bottom) - Hx(list, dataBounds.top))) {
	SetHandleSize((Handle) HxP(list, cells), (Size) 0);
	SetHandleSize((Handle) list, sizeof(ListRec));
	HxX(list, cellArray)[0] = CWC(0);
	HxX(list, dataBounds.bottom) = HxX(list, dataBounds.top);
	HxX(list, visible.top)  = HxX(list, dataBounds.top);
	p.h = Hx(list, cellSize.h);
	p.v = Hx(list, cellSize.v);
	C_LCellSize(p, list);		/* recalcs visible */
	if (Hx(list, listFlags) & DODRAW)
	    EraseRect(&HxX(list, rView));
	if ((control = HxP(list, vScroll)))
	    SetCtlMax(control, Hx(control, contrlMin));
/*-->*/	return;	/* quick delete of all */
    }

    if (rown + count > Hx(list, dataBounds.bottom))
	count = Hx(list, dataBounds.bottom) - rown;

    ncols = Hx(list, dataBounds.right) - Hx(list, dataBounds.left);
    noffsets = count * ncols;

    HxX(list, dataBounds.bottom) = CW(Hx(list, dataBounds.bottom) - count);

    if (noffsets) {
        INTEGER visible_bottom, bounds_bottom;

	TRAPBEGIN();
	nrows = Hx(list, dataBounds.bottom) - Hx(list, dataBounds.top);
	nbefore = (rown - Hx(list, dataBounds.top)) * ncols;
	nafter  = nrows * ncols  - nbefore;
	ip = op = (GUEST<uint16_t> *) HxX(list, cellArray) + nbefore;
	ip += noffsets;
	off1 = CW(*op) & 0x7FFF;
	off2 = CW(*ip) & 0x7FFF;
	delta = off2 - off1;

	while (--nafter >= 0)
	    *op++ = CW(CW(*ip++) - delta);
	off3 = CW(*ip) & 0x7FFF;
	*op = CW(CW(*ip) - delta);	/* sentinel */

	ntomove = off3 - off2;
	BlockMove((Ptr) STARH(HxP(list, cells)) + off2,
			       (Ptr) STARH(HxP(list, cells)) + off1, ntomove);

	SetHandleSize((Handle) list,
		    GetHandleSize((Handle) list) - noffsets * sizeof(INTEGER));
	SetHandleSize((Handle) HxP(list, cells),
			       GetHandleSize((Handle) HxP(list, cells)) - delta);

	p.h = Hx(list, cellSize.h);
	p.v = Hx(list, cellSize.v);

	/* save visible_bottom and bounds_bottom now, because LCellSize
	   will adjust them and we won't be able to figure out if we
	   needed to force a scroll */

	visible_bottom = Hx(list, visible.bottom);
	bounds_bottom = Hx(list, dataBounds.bottom);
	C_LCellSize(p, list);		/* recalcs visible */
	if ((control = HxP(list, vScroll))) {
	    INTEGER visible_top;

	    visible_top = Hx(list, visible.top);

	    /* Determine whether or not we need to scroll up one location
	       (because we were maximally scrolled down and we deleted
	       something that was visible) */

	    if (visible_top > 0 && visible_bottom > bounds_bottom) {
	      --visible_top;
	      HxX(list, visible.top) = CW (visible_top);
	      rown = visible_top;
	    }
	}

	if (Hx(list, listFlags) & DODRAW) {
	    Rect eraser;

	    compute_visible_rect (&eraser, list, Hx (list, visible.bottom),
				  Hx (list, visible.left),  visible_bottom,
				  Hx (list, visible.right));
	    EraseRect (&eraser);

	    todraw = HxX(list, dataBounds);
	    todraw.top = CW(rown);
	    SectRect(&todraw, &HxX(list, visible), &todraw);

	    for (c.v = CW(todraw.top) ; c.v < CW(todraw.bottom); c.v++)
		for (c.h = CW(todraw.left) ; c.h < CW(todraw.right); c.h++)
		    C_LDraw(c, list);
	}
	TRAPEND();
    }
}
