/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_listMouse[] =
	    "$Id: listMouse.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in ListMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "EventMgr.h"
#include "ToolboxEvent.h"
#include "ToolboxUtil.h"
#include "OSEvent.h"
#include "ControlMgr.h"
#include "ListMgr.h"

#include "rsys/cquick.h"
#include "rsys/list.h"
#include "rsys/pstuff.h"
#include "rsys/hook.h"

using namespace Executor;
using namespace ByteSwap;

#if defined (BINCOMPAT)
typedef pascal BOOLEAN (*clickproc)( void );
#endif

namespace Executor {
  PRIVATE void findcell(Cell*,ListHandle);
  PRIVATE void setselectnilflag(BOOLEAN setit, Cell cell,
	 ListHandle list, BOOLEAN hiliteempty);
  static inline BOOLEAN ROMlib_CALLCLICK(clickproc);
  PRIVATE void scrollbyvalues(ListHandle);
  PRIVATE void rect2value(register Rect * in, register Rect * butnotin,
						  register INTEGER value, register ListHandle list,
						  BOOLEAN hiliteempty);
  PRIVATE void rectvalue(register Rect * rp, register INTEGER value,
						 register ListHandle list, BOOLEAN hiliteempty);
}

A2(PRIVATE, void, findcell, Cell *, cp, ListHandle, list)
{
    cp->h = BigEndianValue((BigEndianValue(cp->h) - Hx(list, rView.left)) / Hx(list, cellSize.h) +
						       Hx(list, visible.left));
    cp->v = BigEndianValue((BigEndianValue(cp->v) - Hx(list, rView.top))  / Hx(list, cellSize.v) +
						        Hx(list, visible.top));

    if (BigEndianValue(cp->h) >= Hx(list, visible.right))
	cp->h  = CWC(32767);
    if (BigEndianValue(cp->v) >= Hx(list, visible.bottom))
	cp->v  = CWC(32767);
}

A4(PRIVATE, void, setselectnilflag, BOOLEAN, setit, Cell, cell,
					ListHandle, list, BOOLEAN, hiliteempty)
{
    GrafPtr saveport;
    RgnHandle saveclip;
    Rect r;
    INTEGER *ip, off0wbit, off0, off1;
    LISTDECL();

    if ((ip = ROMlib_getoffp(cell, list))) {
	off0wbit = BigEndianValue(*ip);
	if (setit)
	    *ip = BigEndianValue(off0wbit | 0x8000);
	else
	    *ip = BigEndianValue(off0wbit & 0x7FFF);
	if (PtInRect(cell, &HxX(list, visible)) &&
					     (!(off0wbit & 0x8000) ^ !setit)) {
	    off0 = off0wbit & 0x7FFF;
	    off1 =    BigEndianValue(ip[1]) & 0x7FFF;
	    if (hiliteempty || off0 != off1) {

		C_LRect(&r, cell, list);

		saveport = thePort;
		SetPort(HxP(list, port));
		saveclip = PORT_CLIP_REGION_X (thePort);
		PORT_CLIP_REGION_X (thePort) = RM (NewRgn ());
		ClipRect(&r);

		LISTBEGIN(list);
/* #define TEMPORARY_HACK_DO_NOT_CHECK_IN */
#if !defined(TEMPORARY_HACK_DO_NOT_CHECK_IN)
		LISTCALL(lHiliteMsg, setit, &r, cell, off0, off1 - off0, list);
#endif
		LISTEND(list);

		DisposeRgn (PORT_CLIP_REGION (thePort));
		PORT_CLIP_REGION_X (thePort) = saveclip;
		SetPort (saveport);
	    }
	}
    }
}

A4(PRIVATE, void, rectvalue, register Rect *, rp, register INTEGER, value,
			       register ListHandle, list, BOOLEAN, hiliteempty)
{
    register INTEGER *ip, *ep;
    register INTEGER *sp;
    Cell c;
    LISTDECL();

    LISTBEGIN(list);
    for (c.v = BigEndianValue(rp->top) ; c.v < BigEndianValue(rp->bottom); c.v++) {
	c.h = BigEndianValue(rp->left);
	if ((sp = ip = ROMlib_getoffp(c, list))) {
	    for (ep = ip + (BigEndianValue(rp->right) - BigEndianValue(rp->left)); ip != ep; ip++)
		if (!(BigEndianValue(*ip) & 0x8000) ^ !value) {
		    c.h = BigEndianValue(rp->left) + (ip - sp);
		    setselectnilflag(value, c, list, hiliteempty);
		}
	}
    }
    LISTEND(list);
}

A5(PRIVATE, void, rect2value, register Rect *, in, register Rect *, butnotin,
	    register INTEGER, value, register ListHandle, list,
							BOOLEAN, hiliteempty)
{
    register INTEGER *ip;
    Cell c;

    for (c.v = BigEndianValue(in->top) ; c.v < BigEndianValue(in->bottom); c.v++)
	for (c.h = BigEndianValue(in->left) ; c.h < BigEndianValue(in->right); c.h++)
	    if (!PtInRect(c, butnotin) && (ip = ROMlib_getoffp(c, list)))
		if (!(BigEndianValue(*ip) & 0x8000) ^ !value)
		    setselectnilflag(value, c, list, hiliteempty);
}

A1(PRIVATE, void, scrollbyvalues, ListHandle, list)
{
    INTEGER h, v;
    ControlHandle ch;
    Point p;

    h = (ch = HxP(list, hScroll)) ? GetCtlValue(ch) : Hx(list, visible.left);
    v = (ch = HxP(list, vScroll)) ? GetCtlValue(ch) : Hx(list, visible.top);
    C_LScroll(h - Hx(list, visible.left), v - Hx(list, visible.top), list);
    HxX(list, visible.left) = BigEndianValue(h);
    HxX(list, visible.top)  = BigEndianValue(v);
    p.h = Hx(list, cellSize.h);
    p.v = Hx(list, cellSize.v);
    C_LCellSize(p, list);
}

P2(PUBLIC, pascal void,  ROMlib_mytrack, ControlHandle, ch, INTEGER, part)
{
    INTEGER quant, page;
    ListPtr lp;

    lp = (ListPtr) (long) STARH((Handle) (long) MR(HxX(ch, contrlRfCon)));

    page = ch == MR(lp->hScroll) ?
		     BigEndianValue(lp->visible.right)  - BigEndianValue(lp->visible.left) - 1
		 :
		     BigEndianValue(lp->visible.bottom) - BigEndianValue(lp->visible.top)  - 1;

    switch (part) {
    case inUpButton:
	quant = -1;
	break;
    case inDownButton:
	quant =  1;
	break;
    case inPageUp:
	quant = -page;
	break;
    case inPageDown:
	quant =  page;
	break;
    default:
	gui_assert(0);
	quant = 0;
	break;
    }
    SetCtlValue(ch, GetCtlValue(ch) + quant);
    scrollbyvalues((ListHandle) (long) MR(HxX(ch, contrlRfCon)));
}

#if !defined (BINCOMPAT)
#define CALLCLICK(f)	(CallPascalB(f))
#else /* BINCOMPAT */
#define CALLCLICK(f)	ROMlib_CALLCLICK((clickproc)(f))

A1(static inline, BOOLEAN, ROMlib_CALLCLICK, clickproc, fp)
{
    BOOLEAN retval;

    ROMlib_hook(list_clicknumber);
    HOOKSAVEREGS();
    retval = CToPascalCall(&fp, CTOP_Button);
    HOOKRESTOREREGS();
    return retval;
}

#endif /* BINCOMPAT */

P3(PUBLIC pascal trap, BOOLEAN, LClick, Point, pt,		/* IMIV-273 */
					      INTEGER, mods, ListHandle, list)
{
    ControlHandle ch, scrollh, scrollv;
    Rect r, rswapped;
    BOOLEAN doubleclick, ctlchanged;
    BOOLEAN hiliteempty, onlyone, userects, disjoint, extend;
    BOOLEAN initial;
    enum { Off, On, UseSense } cellvalue;	/* order is important here */
    Byte flags;
    EventRecord evt;
    Rect anchor, oldselrect, newselrect, newcellr, pinrect;
    Cell oldcell, newcell, c, cswapped, oldcellunswapped, newcellunswapped;
    LONGINT l;
    INTEGER dh, dv;
    Point p;

    doubleclick = FALSE;
    if (PtInRect(pt, &HxX(list, rView))) {
	TRAPBEGIN();
	flags = Hx(list, selFlags);
	newcell.h = BigEndianValue(pt.h);
	newcell.v = BigEndianValue(pt.v);
	findcell(&newcell, list);
	if (newcell.h == HxX(list, lastClick.h) &&
	    newcell.v == HxX(list, lastClick.v) &&
			     TickCount() < Hx(list, clikTime) + BigEndianValue(DoubleTime))
	    doubleclick = TRUE;
	HxX(list, lastClick) = newcell;
	hiliteempty = !(flags & lNoNilHilite);
	if (((mods & shiftKey) || (flags & lExtendDrag)) &&
							 !(flags & lOnlyOne)) {
	    onlyone   = FALSE;
	    disjoint  = !(flags & lNoDisjoint);
	    userects  = !(flags & lNoRect);
	    cellvalue =  (flags & lUseSense) ? UseSense : On;
	    extend    =  (flags & lUseSense) ? FALSE : !(flags & lNoExtend);
	} else if ((mods & cmdKey) && !(flags & lOnlyOne)) {
	    onlyone   = FALSE;
	    disjoint  = !(flags & lNoDisjoint);
	    userects  = FALSE;
	    cellvalue = UseSense;
	    extend    = FALSE;
	} else {
	    onlyone   = TRUE;
	    disjoint  = FALSE;
	    userects  = FALSE;
	    cellvalue = On;
	    extend    = FALSE;
	}
	initial = C_LGetSelect(FALSE, &newcell, list);
	if (cellvalue == UseSense)
	    cellvalue = initial ? Off : On;
	if (!disjoint && !initial)
	    rectvalue(&HxX(list, dataBounds), Off, list, hiliteempty);

	if (userects) {
	    anchor.top = anchor.bottom = 0;
	    if (extend) {
		rswapped = HxX(list, dataBounds);
		r.top    = BigEndianValue(rswapped.top);
		r.left   = BigEndianValue(rswapped.left);
		r.bottom = BigEndianValue(rswapped.bottom);
		r.right  = BigEndianValue(rswapped.right);
		for (c.h = r.left; c.h < r.right ; c.h++)
		    for (c.v = r.top; c.v < r.bottom ; c.v++) {
			cswapped.h = BigEndianValue(c.h);
			cswapped.v = BigEndianValue(c.v);
			if (C_LGetSelect(FALSE, &cswapped, list))
			    goto out1;
		    }
		out1:
		c.h = BigEndianValue(cswapped.h);
		c.v = BigEndianValue(cswapped.v);
		if (c.h != r.right) {
		    anchor.left = BigEndianValue(c.h);

		    for (c.h = r.right-1; c.h >= r.left ; c.h--)
			for (c.v = r.top; c.v < r.bottom ; c.v++) {
			    cswapped.h = BigEndianValue(c.h);
			    cswapped.v = BigEndianValue(c.v);
			    if (C_LGetSelect(FALSE, &cswapped, list))
				goto out2;
		    }
		    out2:
		    c.h = BigEndianValue(cswapped.h);
		    c.v = BigEndianValue(cswapped.v);
		    anchor.right = BigEndianValue(c.h + 1);

		    cswapped.h = BigEndianValue(r.left);
		    cswapped.v = BigEndianValue(r.top);
		    C_LGetSelect(TRUE, &cswapped, list);
		    anchor.top = cswapped.v;

		    for (c.v = r.bottom - 1; c.v >= r.top ; c.v--)
			for (c.h = r.left; c.h < r.right ; c.h++) {
			    cswapped.h = BigEndianValue(c.h);
			    cswapped.v = BigEndianValue(c.v);
			    if (C_LGetSelect(FALSE, &cswapped, list))
				goto out3;
		    }
		    out3:
		    anchor.bottom = BigEndianValue(BigEndianValue(cswapped.v) + 1);
		}
	    }
	    if (anchor.top == anchor.bottom) {
		anchor.top    = newcell.v;
		anchor.left   = newcell.h;
		anchor.bottom = BigEndianValue(BigEndianValue(anchor.top)  + 1);
		anchor.right  = BigEndianValue(BigEndianValue(anchor.left) + 1);
	    }
	    c.h = BigEndianValue(anchor.left);
	    c.v = BigEndianValue(anchor.top);
	    C_LRect(&rswapped, c, list);
	    if (pt.h < BigEndianValue(rswapped.right) && pt.v < BigEndianValue(rswapped.bottom)) {
		anchor.top  = BigEndianValue(BigEndianValue(anchor.bottom) - 1);
		anchor.left = BigEndianValue(BigEndianValue(anchor.right)  - 1);
	    } else {
		anchor.bottom = BigEndianValue(BigEndianValue(anchor.top)  + 1);
		anchor.right  = BigEndianValue(BigEndianValue(anchor.left) + 1);
	    }
	    oldselrect = (flags & lUseSense) ? anchor : HxX(list, dataBounds);
	}

	HxX(list, clikTime) = BigEndianValue(TickCount());
	HxX(list, clikLoc.h)  = BigEndianValue(pt.h);
	HxX(list, clikLoc.v)  = BigEndianValue(pt.v);
	oldcell.h = CWC(32767);

        evt.where.h = BigEndianValue(pt.h);
        evt.where.v = BigEndianValue(pt.v);
	pinrect = HxX(list, rView);
	pinrect.left = BigEndianValue(BigEndianValue(pinrect.left) - 1);
	pinrect.bottom = BigEndianValue(BigEndianValue(pinrect.bottom) - 1);
	do {
	    HxX(list, mouseLoc) = evt.where;
	    if (HxP(list, lClikLoop))
		if (CALLCLICK(HxP(list, lClikLoop)))
/*-->*/		    break;
	    p.h = BigEndianValue(evt.where.h);
	    p.v = BigEndianValue(evt.where.v);
	    if (!PtInRect(p, &HxX(list, rView))) {
		ctlchanged = FALSE;
		scrollh = HxP(list, hScroll);
		scrollv = HxP(list, vScroll);
		dh = 0;
		dv = 0;
		if (BigEndianValue(evt.where.h) < Hx(list, rView.left)) {
		    if (scrollh) {
			SetCtlValue(scrollh, GetCtlValue(scrollh)-1);
			ctlchanged = TRUE;
		    } else
			dh = -1;
		} else if (BigEndianValue(evt.where.h) > Hx(list, rView.right)) {
		    if (scrollh) {
			SetCtlValue(scrollh, GetCtlValue(scrollh)+1);
			ctlchanged = TRUE;
		    } else
			dh = 1;
		}
		if (BigEndianValue(evt.where.v) < Hx(list, rView.top)) {
		    if (scrollv) {
			SetCtlValue(scrollv, GetCtlValue(scrollv)-1);
			ctlchanged = TRUE;
		    } else
			dv = -1;
		} else if (BigEndianValue(evt.where.v) > Hx(list, rView.bottom)) {
		    if (scrollv) {
			SetCtlValue(scrollv, GetCtlValue(scrollv)+1);
			ctlchanged = TRUE;
		    } else
			dv = 1;
		}
		if (ctlchanged)
		    scrollbyvalues(list);
		else
		    C_LScroll(dh, dv, list);
	    }
	    p.h = BigEndianValue(evt.where.h);
	    p.v = BigEndianValue(evt.where.v);
	    l = PinRect(&pinrect, p);
	    newcell.h = BigEndianValue(LoWord(l));
	    newcell.v = BigEndianValue(HiWord(l));
	    findcell(&newcell, list);
	    if (userects) {
		newcellr.top    = newcell.v;
		newcellr.left   = newcell.h;
		newcellr.bottom = BigEndianValue(BigEndianValue(newcellr.top)  + 1);
		newcellr.right  = BigEndianValue(BigEndianValue(newcellr.left) + 1);
		UnionRect(&anchor, &newcellr, &newselrect);
		rect2value(&oldselrect, &newselrect, !cellvalue, list,
								  hiliteempty);
		rectvalue(&newselrect, cellvalue, list, hiliteempty);
		oldselrect = newselrect;
	    } else {
		if (newcell.h != oldcell.h ||
		    newcell.v != oldcell.v) {
		    if (onlyone && oldcell.h != 32767) {
			oldcellunswapped.h = BigEndianValue(oldcell.h);
			oldcellunswapped.v = BigEndianValue(oldcell.v);
			setselectnilflag(FALSE, oldcellunswapped, list,
								  hiliteempty);
		    }
		    newcellunswapped.h = BigEndianValue(newcell.h);
		    newcellunswapped.v = BigEndianValue(newcell.v);
		    setselectnilflag(cellvalue, newcellunswapped, list,
								  hiliteempty);
		    oldcell = newcell;
		}
	    }
	} while (!OSEventAvail(mUpMask, &evt) &&
					    (GlobalToLocal(&evt.where), TRUE));
	TRAPEND();
    } else if (((ch = HxP(list, hScroll)) && PtInRect(pt, &HxX(ch, contrlRect))) ||
               ((ch = HxP(list, vScroll)) && PtInRect(pt, &HxX(ch, contrlRect)))) {
	if (TestControl(ch, pt) == inThumb) {
	    TrackControl(ch, pt, (ProcPtr) 0);
	    scrollbyvalues(list);
	} else
	    TrackControl(ch, pt, (ProcPtr) P_ROMlib_mytrack);
    }
    return doubleclick;
    return 0;
}

P1(PUBLIC pascal trap, LONGINT, LLastClick, ListHandle, list)	/* IMIV-273 */
{
    return ((LONGINT) Hx(list, lastClick.v) << 16) |
					(unsigned short) Hx(list, lastClick.h);
}

P3(PUBLIC pascal trap, void, LSetSelect, BOOLEAN, setit,	/* IMIV-273 */
						 Cell, cell, ListHandle, list)
{
    setselectnilflag(setit, cell, list, TRUE);
}
