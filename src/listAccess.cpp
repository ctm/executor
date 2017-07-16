/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_listAccess[] =
	    "$Id: listAccess.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in ListMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ListMgr.h"
#include "MemoryMgr.h"
#include "IntlUtil.h"
#include "rsys/list.h"
#include "rsys/hook.h"

using namespace Executor;
using namespace ByteSwap;

P4(PUBLIC pascal trap, void, LFind, INTEGER *, offsetp,		/* IMIV-274 */
				INTEGER *, lenp, Cell, cell, ListHandle, list)
{
    INTEGER *ip;

    if ((ip = ROMlib_getoffp(cell, list))) {
	*offsetp =  *ip++ & CWC(0x7FFF);
	*lenp    = BigEndianValue((BigEndianValue(*ip)   & 0x7FFF) - BigEndianValue(*offsetp));
    } else
	*offsetp = *lenp = CWC(-1);
}

P4(PUBLIC pascal trap, BOOLEAN, LNextCell, BOOLEAN, hnext,	/* IMIV-274 */
			      BOOLEAN, vnext, Cell *, cellp, ListHandle, list)
{
    BOOLEAN retval;
    Cell scratch;
    INTEGER right, bottom;

    scratch.v = BigEndianValue(cellp->v);
    scratch.h = BigEndianValue(cellp->h);
    right   = Hx(list, dataBounds.right);
    bottom  = Hx(list, dataBounds.bottom);
    if (hnext) {
	if (++scratch.h >= right)
	    if (vnext && ++scratch.v < bottom) {
		scratch.h = 0;
		retval = TRUE;
	    } else
		retval = FALSE;
	else
	    retval = TRUE;
    } else {
	if (vnext && ++scratch.v < bottom)
	    retval = TRUE;
	else
	    retval = FALSE;
    }
    if (retval) {
	cellp->v = BigEndianValue(scratch.v);
	cellp->h = BigEndianValue(scratch.h);
    }
    return retval;
}

P3(PUBLIC pascal trap, void, LRect, Rect *, cellrect,		/* IMIV-274 */
						 Cell, cell, ListHandle, list)
{
    Point csize;
    INTEGER temp;

    if (PtInRect(cell, &HxX(list, visible))) {
	csize.h = Hx(list, cellSize.h);
	csize.v = Hx(list, cellSize.v);
	*cellrect = HxX(list, rView);
	cellrect->top = BigEndianValue(BigEndianValue(cellrect->top) +
			        ((cell.v - Hx(list, visible.top))  * csize.v));
	cellrect->left = BigEndianValue(BigEndianValue(cellrect->left) +
				((cell.h - Hx(list, visible.left)) * csize.h));
	if ((temp = BigEndianValue(cellrect->top)  + csize.v) < BigEndianValue(cellrect->bottom))
	    cellrect->bottom = BigEndianValue(temp);
	if ((temp = BigEndianValue(cellrect->left) + csize.h) < BigEndianValue(cellrect->right))
	    cellrect->right = BigEndianValue(temp);
    } else {
	cellrect->top    = cellrect->left =
	cellrect->bottom = cellrect->right = CWC(0);
    }
}

#if !defined (__STDC__)

typedef INTEGER (*cmpf)();

#define CALLCMP(a1, a2, a3, a4, fp) CallPascalW4(a1, a2, a3, a4, fp)

#else /* __STDC__ */

typedef pascal INTEGER (*cmpf)(Ptr p1, Ptr p2, INTEGER len1, INTEGER len2);

#if !defined (BINCOMPAT)

#define CALLCMP(a1, a2, a3, a4, fp) ((*(cmpffp))(a1, a2, a3, a4))

#else /* BINCOMPAT */

#define CALLCMP(a1, a2, a3, a4, fp)	\
				      ROMlib_CALLCMP(a1, a2, a3, a4, (cmpf) fp)

namespace Executor {
  static inline INTEGER ROMlib_CALLCMP(Ptr,Ptr,INTEGER,INTEGER,cmpf);
}

A5(static inline, INTEGER, ROMlib_CALLCMP, Ptr, p1, Ptr, p2, INTEGER, l1,
							 INTEGER, l2, cmpf, fp)
{
    INTEGER retval;

    if (fp == (cmpf) P_IUMagString)
	retval = C_IUMagString(p1, p2, l1, l2);
    else {
	ROMlib_hook(list_cmpnumber);
	HOOKSAVEREGS();
	retval = CToPascalCall(&fp, CTOP_IUMagString, p1, p2, l1, l2);
	HOOKRESTOREREGS();
    }
    return retval;
}

#endif /* BINCOMPAT */

#endif /* __STDC__ */

P5(PUBLIC pascal trap, BOOLEAN, LSearch, Ptr, dp,		/* IMIV-274 */
		      INTEGER, dl, Ptr, proc, Cell *, cellp, ListHandle, list)
{
    INTEGER off, len;
    Cell cell, swappedcell;
    cmpf fp;

    HLock((Handle) list);
    HLock((Handle) HxP(list, cells));

    fp = proc ? (cmpf) proc : (cmpf) P_IUMagString;
    cell.h = BigEndianValue(cellp->h);
    cell.v = BigEndianValue(cellp->v);
    swappedcell = *cellp;
    /* TODO: SPEEDUP:  the following is a stupid way to do the loop, instead ip
		 and ep should be used! */
    while ((C_LFind(&off, &len, cell, list), len = BigEndianValue (len), off = BigEndianValue (off),
	    len != -1) &&
	       CALLCMP(dp, (Ptr) STARH(HxP(list, cells)) + off, dl, len, fp) != 0)
	if (!C_LNextCell(TRUE, TRUE, &swappedcell, list)) {
	    cell.h = Hx(list, dataBounds.right);
	    cell.v = Hx(list, dataBounds.bottom);
	} else {
	    cell.h = BigEndianValue (swappedcell.h);
	    cell.v = BigEndianValue (swappedcell.v);
	}

    HUnlock((Handle) HxP(list, cells));
    HUnlock((Handle) list);
    if (len != -1) {
	cellp->h = BigEndianValue(cell.h);
	cellp->v = BigEndianValue(cell.v);
/*-->*/	return TRUE;
    } else
	return FALSE;
}


P3(PUBLIC pascal trap, void, LSize, INTEGER, width,		/* IMIV-274 */
					    INTEGER, height, ListHandle, list)
{
    INTEGER oldright, oldbottom, newright, newbottom;
    ControlHandle cv, ch;
    RgnHandle rectrgn, updatergn;
    Rect r;
    Point p;

    TRAPBEGIN();
    oldright  = Hx(list, rView.right);
    oldbottom = Hx(list, rView.bottom);
    newright  = Hx(list, rView.left) + width;
    HxX(list, rView.right) = BigEndianValue(newright);
    newbottom = Hx(list, rView.top)  + height;
    HxX(list, rView.bottom) = BigEndianValue(newbottom);
    ch        = HxP(list, hScroll);
    cv        = HxP(list, vScroll);

    p.h = Hx(list, cellSize.h);
    p.v = Hx(list, cellSize.v);
    C_LCellSize(p, list);		/* sets visible */

    updatergn = NewRgn();
    rectrgn = NewRgn();
    if (newright != oldright) {
	if (newbottom != oldbottom) {	/* both are different */
	    if (ch) {
		MoveControl(ch, Hx(list, rView.left) - 1, newbottom);
		SizeControl(ch, newright - Hx(list, rView.left) + 2, 16);
	    }
	    if (cv) {
		MoveControl(cv, newright, Hx(list, rView.top) - 1);
		SizeControl(cv, 16, newbottom - Hx(list, rView.top) + 2);
	    }
	    r.top    = BigEndianValue(MIN(oldbottom, newbottom));
	    r.bottom = BigEndianValue(MAX(oldbottom, newbottom));
	    r.left   = BigEndianValue(Hx(list, rView.left) - 1);
	    r.right  = BigEndianValue(MAX(oldright, newright));
	    if (ch)
		r.bottom = BigEndianValue(BigEndianValue(r.bottom) + (16));
	    RectRgn(rectrgn, &r);
	    UnionRgn(rectrgn, updatergn, updatergn);
	} else {	/* just right different */
	    if (ch) {
		SizeControl(ch, newright - Hx(list, rView.left) + 2, 16);
	    }
	    if (cv)
		MoveControl(cv, newright, Hx(list, rView.top) - 1);
	}
	r.left   = BigEndianValue(MIN(oldright, newright));
	r.right	 = BigEndianValue(MAX(oldright, newright));
	r.top    = BigEndianValue(Hx(list, rView.top) - 1);
	r.bottom = BigEndianValue(MAX(oldbottom, newbottom));
	if (cv)
	    r.right = BigEndianValue(BigEndianValue(r.right) + (16));
	RectRgn(rectrgn, &r);
	UnionRgn(rectrgn, updatergn, updatergn);
    } else if (newbottom != oldbottom) {	/* just bottom different */
	if (ch)
	    MoveControl(ch, Hx(list, rView.left) - 1, newbottom);
	if (cv) {
	    SizeControl(cv, 16, newbottom - Hx(list, rView.top) + 2);
	}
	r.top    = BigEndianValue(MIN(oldbottom, newbottom));
	r.bottom = BigEndianValue(MAX(oldbottom, newbottom));
	r.left   = BigEndianValue(Hx(list, rView.left) - 1);
	r.right  = BigEndianValue(MAX(oldright, newright));
	if (ch)
	    r.bottom = BigEndianValue(BigEndianValue(r.bottom) + (16));
	RectRgn(rectrgn, &r);
	UnionRgn(rectrgn, updatergn, updatergn);
    }
    C_LUpdate(updatergn, list);
    DisposeRgn(updatergn);
    DisposeRgn(rectrgn);
    TRAPEND();
}
