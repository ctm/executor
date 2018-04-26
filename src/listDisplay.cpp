/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ListMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ListMgr.h"

#include "rsys/cquick.h"
#include "rsys/list.h"
#include "rsys/hook.h"
#include "rsys/functions.impl.h"

using namespace Executor;

void Executor::C_LDraw(Cell cell, ListHandle list) /* IMIV-275 */
{
    GrafPtr saveport;
    GUEST<RgnHandle> saveclip;
    Rect r;
    GUEST<INTEGER> *ip;
    INTEGER off0, off1;
    BOOLEAN setit;
    LISTDECL();

    if((ip = ROMlib_getoffp(cell, list)))
    {
        off0 = CW(ip[0]) & 0x7FFF;
        off1 = CW(ip[1]) & 0x7FFF;
        setit = (CW(ip[0]) & 0x8000) && Hx(list, lActive);
        saveport = thePort;
        SetPort(HxP(list, port));
        saveclip = PORT_CLIP_REGION_X(thePort);
        PORT_CLIP_REGION_X(thePort) = RM(NewRgn());

        C_LRect(&r, cell, list);
        ClipRect(&r);
        LISTBEGIN(list);
        LISTCALL(lDrawMsg, setit, &r, cell, off0, off1 - off0, list);
        LISTEND(list);

        DisposeRgn(PORT_CLIP_REGION(thePort));
        PORT_CLIP_REGION_X(thePort) = saveclip;
        SetPort(saveport);
    }
}

void Executor::C_LDoDraw(BOOLEAN draw, ListHandle list) /* IMIV-275 */
{
    if(draw)
    {
        HxX(list, listFlags) |= DODRAW;
        if(Hx(list, lActive))
        {
            if(HxP(list, vScroll))
                ShowControl(HxP(list, vScroll));
            if(HxP(list, hScroll))
                ShowControl(HxP(list, hScroll));
        }
    }
    else
        HxX(list, listFlags) &= ~DODRAW;
}

void Executor::C_LScroll(INTEGER ncol, INTEGER nrow,
                         ListHandle list) /* IMIV-275 */
{
    RgnHandle rh;
    Rect r;
    ControlHandle ch;
    INTEGER tmpi;
    Point p;

    r = HxX(list, rView);

    /*
 * TODO:  if either the horizontal or vertical component of a view rectangle
 *        isn't a multiple of the appropriate component of the  cell size then
 *	  you need to allow an extra (blank) cell when scrolled totally to
 *	  the bottom or to the right.
 */

    if(nrow < 0)
    {
        tmpi = Hx(list, dataBounds.top) - Hx(list, visible.top);
        if(tmpi > nrow)
            nrow = tmpi;
    }
    else if(nrow > 0)
    {
        tmpi = Hx(list, dataBounds.bottom) - Hx(list, visible.bottom) + 1;
        if(tmpi < nrow)
            nrow = tmpi;
    }

    if(ncol < 0)
    {
        tmpi = Hx(list, dataBounds.left) - Hx(list, visible.left);
        if(tmpi > ncol)
            ncol = tmpi;
    }
    else if(ncol > 0)
    {
        tmpi = Hx(list, dataBounds.right) - Hx(list, visible.right) + 1;
        if(tmpi < ncol)
            ncol = tmpi;
    }

    HxX(list, visible.top) = CW(Hx(list, visible.top) + nrow);
    HxX(list, visible.left) = CW(Hx(list, visible.left) + ncol);

    p.h = Hx(list, cellSize.h);
    p.v = Hx(list, cellSize.v);
    C_LCellSize(p, list); /*  recalculates visible */

    if(ncol && (ch = HxP(list, hScroll)))
        SetCtlValue(ch, Hx(list, visible.left));
    if(nrow && (ch = HxP(list, vScroll)))
        SetCtlValue(ch, Hx(list, visible.top));

    if(Hx(list, listFlags) & DODRAW)
    {
        rh = NewRgn();
        ScrollRect(&r, -ncol * Hx(list, cellSize.h),
                   -nrow * Hx(list, cellSize.v), rh);
        C_LUpdate(rh, list);
        DisposeRgn(rh);
    }
}

void Executor::C_LAutoScroll(ListHandle list) /* IMIV-275 */
{
    GUEST<Cell> gcell;
    Cell cell;

    gcell.h = HxX(list, dataBounds.left);
    gcell.v = HxX(list, dataBounds.top);
    if(C_LGetSelect(true, &gcell, list))
    {
        cell = gcell.get();
        if(!PtInRect(cell, &HxX(list, visible)))
        {
            C_LScroll(cell.h - Hx(list, visible.left),
                      cell.v - Hx(list, visible.top), list);
        }
    }
}

void Executor::C_LUpdate(RgnHandle rgn, ListHandle list) /* IMIV-275 */
{
    Rect r;
    Cell c, csize;
    INTEGER cleft;
    INTEGER top, left, bottom, right;
    ControlHandle ch;

    cleft = c.h = Hx(list, visible.left);
    c.v = Hx(list, visible.top);
    csize.h = Hx(list, cellSize.h);
    csize.v = Hx(list, cellSize.v);
    C_LRect(&r, c, list);
    top = CW(r.top);
    left = CW(r.left);
    bottom = top + (Hx(list, visible.bottom) - Hx(list, visible.top)) * csize.v;
    right = left + (Hx(list, visible.right) - Hx(list, visible.left)) * csize.h;
    while(CW(r.top) < bottom)
    {
        while(CW(r.left) < right)
        {
            if(RectInRgn(&r, rgn))
                C_LDraw(c, list);
            r.left = CW(CW(r.left) + (csize.h));
            r.right = CW(CW(r.right) + (csize.h));
            c.h++;
        }
        c.h = cleft;
        c.v++;
        r.top = CW(CW(r.top) + (csize.v));
        r.bottom = CW(CW(r.bottom) + (csize.v));
        r.left = CW(left);
        r.right = CW(left + csize.h);
    }
    if((ch = HxP(list, hScroll)))
    {
        if(RectInRgn(&HxX(ch, contrlRect), rgn))
            Draw1Control(ch);
    }
    if((ch = HxP(list, vScroll)))
    {
        if(RectInRgn(&HxX(ch, contrlRect), rgn))
            Draw1Control(ch);
    }
}

void Executor::C_LActivate(BOOLEAN act, ListHandle list) /* IMIV-276 */
{
    Cell c;
    Rect r;
    ControlHandle ch;
    BOOLEAN sel;
    GUEST<INTEGER> *ip;
    INTEGER off0, off1;
    GUEST<RgnHandle> saveclip;
    LISTDECL();

    if(!act ^ !Hx(list, lActive))
    {

        if(act)
        {
            sel = true;
            if((ch = HxP(list, hScroll)))
                ShowControl(ch);
            if((ch = HxP(list, vScroll)))
                ShowControl(ch);
        }
        else
        {
            sel = false;
            if((ch = HxP(list, hScroll)))
                HideControl(ch);
            if((ch = HxP(list, vScroll)))
                HideControl(ch);
        }
        LISTBEGIN(list);
        for(c.v = Hx(list, visible.top); c.v < Hx(list, visible.bottom);
            c.v++)
        {
            for(c.h = Hx(list, visible.left); c.h < Hx(list, visible.right);
                c.h++)
            {
                if((ip = ROMlib_getoffp(c, list)) && (CW(*ip) & 0x8000))
                {
                    off0 = CW(ip[0]) & 0x7FFF;
                    off1 = CW(ip[1]) & 0x7FFF;
                    saveclip = PORT_CLIP_REGION_X(thePort);
                    PORT_CLIP_REGION_X(thePort) = RM(NewRgn());
                    C_LRect(&r, c, list);
                    ClipRect(&r);
                    LISTCALL(lHiliteMsg, sel, &r, c, off0, off1 - off0, list);
                    DisposeRgn(PORT_CLIP_REGION(thePort));
                    PORT_CLIP_REGION_X(thePort) = saveclip;
                }
            }
        }
        LISTEND(list);
        HxX(list, lActive) = !!act;
    }
}

void Executor::ROMlib_listcall(INTEGER mess, BOOLEAN sel, Rect *rp, Cell cell, INTEGER off,
                               INTEGER len, ListHandle lhand)
{
    Handle listdefhand;

    listdefhand = HxP(lhand, listDefProc);
    if(listdefhand)
    {
        listprocp lp;

        lp = MR(*(GUEST<listprocp> *)listdefhand);
        if(!lp)
        {
            LoadResource(HxP(lhand, listDefProc));
            lp = MR(*(GUEST<listprocp> *)HxP(lhand, listDefProc));
        }
        if(lp == &ldef0)
            ldef0(mess, sel, rp, cell, off, len, lhand);
        else
        {
            ROMlib_hook(list_ldefnumber);
            lp(mess, sel, rp, cell, off, len, lhand);
        }
    }
}
