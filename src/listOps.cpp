/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_listOps[] =
		"$Id: listOps.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in ListMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ListMgr.h"
#include "MemoryMgr.h"
#include "rsys/list.h"

using namespace Executor;
using namespace ByteSwap;

A2(PUBLIC, INTEGER *, ROMlib_getoffp, Cell, cell,		/* INTERNAL */
							     ListHandle, list)
{
    Rect *rp;
    INTEGER ncols, *retval;

    if (list && PtInRect(cell, rp = &HxX(list, dataBounds))) {
	ncols = BigEndianValue(rp->right) - BigEndianValue(rp->left);
	retval = HxX(list, cellArray) + ncols * (cell.v - BigEndianValue(rp->top)) +
						       (cell.h - BigEndianValue(rp->left));
    } else
	retval = 0;
    return retval;
}

typedef enum { Add, Rep } AddOrRep;

namespace Executor {
  PRIVATE void cellhelper(AddOrRep, Ptr, INTEGER, Cell, ListHandle);
}

A5(PRIVATE, void, cellhelper, AddOrRep, addorrep, Ptr, dp, INTEGER, dl,
						Cell, cell, ListHandle, list)
{
    INTEGER *ip, *ep, off0, off1, off2, delta, len;
    Ptr sp;
    Cell temp;
    LONGINT ip_offset, ep_offset;

    if ((dl > 0 || (addorrep == Rep && dl == 0)) &&
					  (ip = ROMlib_getoffp(cell, list))) {
	temp.h = Hx(list, dataBounds.right)  - 1;
	temp.v = Hx(list, dataBounds.bottom) - 1;
	ep = ROMlib_getoffp(temp, list) + 1;
	ip_offset = (char *) ip - (char *) STARH (list);
	ep_offset = (char *) ep - (char *) STARH (list);
	off0 = BigEndianValue(ip[0]) & 0x7FFF;
	off1 = BigEndianValue(ip[1]) & 0x7FFF;
	off2 = BigEndianValue(ep[0]) & 0x7FFF;
	len = off1 - off0;

/*
 * TODO:  Do this with Munger
 */

	if (addorrep == Add)
	    delta = dl;
	else
	    delta = dl - len;

	if (delta > 0)
	  {
	    OSErr err;
	    Size current_size, new_size;

	    current_size = GetHandleSize((Handle) HxP(list, cells));
	    new_size = current_size + delta;
	    SetHandleSize((Handle) HxP(list, cells), new_size);
	    err = MemError ();
	    if (err != noErr)
	      {
		warning_unexpected ("err = %d, delta = %d", err, delta);
		return;
	      }
	  }

	sp = (Ptr) STARH(HxP(list, cells)) + off1;
	BlockMove(sp, sp + delta, (Size) off2 - off1);

	if (delta < 0)
	  {
	    Size current_size, new_size;
	    OSErr err;

	    current_size = GetHandleSize((Handle) HxP(list, cells));
	    new_size = current_size + delta;
	    SetHandleSize((Handle) HxP(list, cells), new_size);
	    err = MemError ();
	    if (err != noErr)
	      warning_unexpected ("err = %d, delta = %d", err, delta);
	  }

	BlockMove(dp, (Ptr) STARH(HxP(list, cells)) + off0 +
				      (addorrep == Add ? len : 0) , (Size)dl);

	ip = (INTEGER *) ((char *) STARH (list) + ip_offset);
	ep = (INTEGER *) ((char *) STARH (list) + ep_offset);

	if (delta) {
	    while (++ip <= ep)
		*ip = BigEndianValue(BigEndianValue(*ip) + (delta));
	}
	if (Hx(list, listFlags) & DODRAW)
	    C_LDraw(cell, list);
    }
}

P4(PUBLIC pascal trap, void, LAddToCell, Ptr, dp, INTEGER, dl,	/* IMIV-272 */
						  Cell, cell, ListHandle, list)
{
    cellhelper(Add, dp, dl, cell, list);
}

P2(PUBLIC pascal trap, void, LClrCell, Cell, cell,		/* IMIV-272 */
						   ListHandle, list)
{
    cellhelper(Rep, (Ptr) 0, 0, cell, list);
}

P4(PUBLIC pascal trap, void, LGetCell, Ptr, dp, INTEGER *, dlp,	/* IMIV-272 */
						 Cell, cell, ListHandle, list)
{
    INTEGER *ip, off1, off2;
    INTEGER ntomove;

    if ((ip = ROMlib_getoffp(cell, list))) {
	off1 = BigEndianValue(*ip++) & 0x7fff;
	off2 = BigEndianValue(*ip)   & 0x7fff;
	ntomove = off2 - off1;
	if (ntomove > BigEndianValue(*dlp))
	    ntomove = BigEndianValue(*dlp);
	BlockMove((Ptr) STARH(HxP(list, cells)) + off1, dp, (Size) ntomove);
	*dlp = BigEndianValue(ntomove);
    }
}

P4(PUBLIC pascal trap, void, LSetCell, Ptr, dp, INTEGER, dl,	/* IMIV-272 */
						 Cell, cell, ListHandle, list)
{
    cellhelper(Rep, dp, dl, cell, list);
}

P2(PUBLIC pascal trap, void, LCellSize, Point, csize,		/* IMIV-273 */
						      ListHandle, list)
{
    ListPtr lp;
    GrafPtr gp;
    FontInfo fi;
    INTEGER nh, nv;

    lp = STARH(list);
    if (!(lp->cellSize.h = BigEndianValue(csize.h)))
	lp->cellSize.h = BigEndianValue((BigEndianValue(lp->rView.right) - BigEndianValue(lp->rView.left)) /
			    MAX(1, (BigEndianValue(lp->dataBounds.right)
				    - BigEndianValue(lp->dataBounds.left))));
    if (!(lp->cellSize.v = BigEndianValue(csize.v))) {
	gp = thePort;
	SetPort(MR(lp->port));
	GetFontInfo(&fi);
	lp = STARH(list);	/* could have moved */
	lp->cellSize.v = BigEndianValue(BigEndianValue(fi.ascent) + BigEndianValue(fi.descent) + BigEndianValue(fi.leading));
	SetPort(gp);
    }
    lp->visible.right  = lp->dataBounds.right;
    lp->visible.bottom = lp->dataBounds.bottom;
    nh = (BigEndianValue(lp->rView.right) - BigEndianValue(lp->rView.left) + BigEndianValue(lp->cellSize.h) - 1) /
							    BigEndianValue(lp->cellSize.h);
    nv = (BigEndianValue(lp->rView.bottom) - BigEndianValue(lp->rView.top) + BigEndianValue(lp->cellSize.v) - 1) /
							    BigEndianValue(lp->cellSize.v);
    if (BigEndianValue(lp->visible.right) - BigEndianValue(lp->visible.left) > nh)
	lp->visible.right = BigEndianValue(BigEndianValue(lp->visible.left) + nh);

    if (BigEndianValue(lp->visible.bottom) - BigEndianValue(lp->visible.top) > nv)
	lp->visible.bottom = BigEndianValue(BigEndianValue(lp->visible.top) + nv);
    {
      ControlHandle control;

      if ((control = HxP(list, hScroll)))
	{
	  INTEGER min, max;

	  ROMlib_hminmax(&min, &max, STARH(list));
	  SetCtlMax(control, max);
	}
    }
    {
      ControlHandle control;

      if ((control = HxP(list, vScroll)))
	{
	  INTEGER min, max;
	  
	  ROMlib_vminmax(&min, &max, STARH(list));
	  SetCtlMax(control, max);
	}
    }
}

P3(PUBLIC pascal trap, BOOLEAN, LGetSelect, BOOLEAN, next,	/* IMIV-273 */
					      Cell *, cellp, ListHandle, list)
{
    INTEGER *ip, *ep, nint, ncols, rown, coln;
    BOOLEAN retval;
    Cell temp, c;
    Point p;

    if (!list || !cellp)
        retval = FALSE;
    else if (next) {
	c.h = BigEndianValue(cellp->h);
	c.v = BigEndianValue(cellp->v);
	if (!(ip = ROMlib_getoffp(c, list))) {
	    temp.h = 0;
	    temp.h = BigEndianValue(cellp->v) + 1;
	    ip = ROMlib_getoffp(temp, list);
	}
	if (!ip)
	    retval = FALSE;
	else {
	    temp.h = Hx(list, dataBounds.right)  - 1;
	    temp.v = Hx(list, dataBounds.bottom) - 1;
	    ep = ROMlib_getoffp(temp, list) + 1;
	    while (ip != ep && !(BigEndianValue(*ip) & 0x8000))
		ip++;
	    if (ip == ep)
		retval = FALSE;
	    else {
		nint = ip - HxX(list, cellArray);
		ncols = Hx(list, dataBounds.right) - Hx(list, dataBounds.left);
		rown = nint / ncols;
		coln = nint % ncols;
		cellp->v = BigEndianValue(Hx(list, dataBounds.top)  + rown);
		cellp->h = BigEndianValue(Hx(list, dataBounds.left) + coln);
		retval = TRUE;
	    }
	}
    } else {
	p.h = BigEndianValue(cellp->h);
	p.v = BigEndianValue(cellp->v);
	if (!(ip = ROMlib_getoffp(p, list)))
	    retval = FALSE;
	else
	    retval = (BigEndianValue(*ip) & 0x8000) ? TRUE : FALSE;
    }
    return retval;
}

/* LSetSelect in listMouse.c */
