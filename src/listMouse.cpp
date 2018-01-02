/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

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

#if defined(BINCOMPAT)
typedef pascal BOOLEAN (*clickproc)(void);
#endif

PRIVATE void findcell(GUEST<Cell> *, ListHandle);
PRIVATE void setselectnilflag(BOOLEAN setit, Cell cell,
                              ListHandle list, BOOLEAN hiliteempty);
static inline BOOLEAN ROMlib_CALLCLICK(clickproc);
PRIVATE void scrollbyvalues(ListHandle);
PRIVATE void rect2value(Rect *in, Rect *butnotin,
                        INTEGER value, ListHandle list,
                        BOOLEAN hiliteempty);
PRIVATE void rectvalue(Rect *rp, INTEGER value,
                       ListHandle list, BOOLEAN hiliteempty);

PRIVATE void findcell(GUEST<Cell> *cp, ListHandle list)
{
    cp->h = CW((CW(cp->h) - Hx(list, rView.left)) / Hx(list, cellSize.h) + Hx(list, visible.left));
    cp->v = CW((CW(cp->v) - Hx(list, rView.top)) / Hx(list, cellSize.v) + Hx(list, visible.top));

    if(CW(cp->h) >= Hx(list, visible.right))
        cp->h = CWC(32767);
    if(CW(cp->v) >= Hx(list, visible.bottom))
        cp->v = CWC(32767);
}

PRIVATE void setselectnilflag(BOOLEAN setit, Cell cell, ListHandle list, BOOLEAN hiliteempty)
{
    GrafPtr saveport;
    GUEST<RgnHandle> saveclip;
    Rect r;
    GUEST<INTEGER> *ip;
    INTEGER off0wbit, off0, off1;
    LISTDECL();

    if((ip = ROMlib_getoffp(cell, list)))
    {
        off0wbit = CW(*ip);
        if(setit)
            *ip = CW(off0wbit | 0x8000);
        else
            *ip = CW(off0wbit & 0x7FFF);
        if(PtInRect(cell, &HxX(list, visible)) && (!(off0wbit & 0x8000) ^ !setit))
        {
            off0 = off0wbit & 0x7FFF;
            off1 = CW(ip[1]) & 0x7FFF;
            if(hiliteempty || off0 != off1)
            {

                C_LRect(&r, cell, list);

                saveport = thePort;
                SetPort(HxP(list, port));
                saveclip = PORT_CLIP_REGION_X(thePort);
                PORT_CLIP_REGION_X(thePort) = RM(NewRgn());
                ClipRect(&r);

                LISTBEGIN(list);
/* #define TEMPORARY_HACK_DO_NOT_CHECK_IN */
#if !defined(TEMPORARY_HACK_DO_NOT_CHECK_IN)
                LISTCALL(lHiliteMsg, setit, &r, cell, off0, off1 - off0, list);
#endif
                LISTEND(list);

                DisposeRgn(PORT_CLIP_REGION(thePort));
                PORT_CLIP_REGION_X(thePort) = saveclip;
                SetPort(saveport);
            }
        }
    }
}

PRIVATE void rectvalue(Rect *rp, INTEGER value, ListHandle list, BOOLEAN hiliteempty)
{
    GUEST<INTEGER> *ip, *ep;
    GUEST<INTEGER> *sp;
    Cell c;
    LISTDECL();

    LISTBEGIN(list);
    for(c.v = CW(rp->top); c.v < CW(rp->bottom); c.v++)
    {
        c.h = CW(rp->left);
        if((sp = ip = ROMlib_getoffp(c, list)))
        {
            for(ep = ip + (CW(rp->right) - CW(rp->left)); ip != ep; ip++)
                if(!(CW(*ip) & 0x8000) ^ !value)
                {
                    c.h = CW(rp->left) + (ip - sp);
                    setselectnilflag(value, c, list, hiliteempty);
                }
        }
    }
    LISTEND(list);
}

PRIVATE void rect2value(Rect *in, Rect *butnotin, INTEGER value, ListHandle list, BOOLEAN hiliteempty)
{
    GUEST<INTEGER> *ip;
    Cell c;

    for(c.v = CW(in->top); c.v < CW(in->bottom); c.v++)
        for(c.h = CW(in->left); c.h < CW(in->right); c.h++)
            if(!PtInRect(c, butnotin) && (ip = ROMlib_getoffp(c, list)))
                if(!(CW(*ip) & 0x8000) ^ !value)
                    setselectnilflag(value, c, list, hiliteempty);
}

PRIVATE void scrollbyvalues(ListHandle list)
{
    INTEGER h, v;
    ControlHandle ch;
    Point p;

    h = (ch = HxP(list, hScroll)) ? GetCtlValue(ch) : Hx(list, visible.left);
    v = (ch = HxP(list, vScroll)) ? GetCtlValue(ch) : Hx(list, visible.top);
    C_LScroll(h - Hx(list, visible.left), v - Hx(list, visible.top), list);
    HxX(list, visible.left) = CW(h);
    HxX(list, visible.top) = CW(v);
    p.h = Hx(list, cellSize.h);
    p.v = Hx(list, cellSize.v);
    C_LCellSize(p, list);
}

PUBLIC pascal void Executor::C_ROMlib_mytrack(ControlHandle ch, INTEGER part)
{
    INTEGER quant, page;
    ListPtr lp;

    lp = STARH(MR(guest_cast<ListHandle>(HxX(ch, contrlRfCon))));

    page = ch == MR(lp->hScroll) ? CW(lp->visible.right) - CW(lp->visible.left) - 1
                                 : CW(lp->visible.bottom) - CW(lp->visible.top) - 1;

    switch(part)
    {
        case inUpButton:
            quant = -1;
            break;
        case inDownButton:
            quant = 1;
            break;
        case inPageUp:
            quant = -page;
            break;
        case inPageDown:
            quant = page;
            break;
        default:
            gui_assert(0);
            quant = 0;
            break;
    }
    SetCtlValue(ch, GetCtlValue(ch) + quant);
    scrollbyvalues(MR(guest_cast<ListHandle>(HxX(ch, contrlRfCon))));
}

#if !defined(BINCOMPAT)
#define CALLCLICK(f) (CallPascalB(f))
#else /* BINCOMPAT */
#define CALLCLICK(f) ROMlib_CALLCLICK((clickproc)(f))

static inline BOOLEAN ROMlib_CALLCLICK(clickproc fp)
{
    BOOLEAN retval;

    ROMlib_hook(list_clicknumber);
    HOOKSAVEREGS();
    retval = CToPascalCall((void *)fp, ctop(&C_Button));
    HOOKRESTOREREGS();
    return retval;
}

#endif /* BINCOMPAT */

PUBLIC pascal trap BOOLEAN Executor::C_LClick(Point pt, /* IMIV-273 */
                                              INTEGER mods, ListHandle list)
{
    ControlHandle ch, scrollh, scrollv;
    struct
    {
        short top, left, bottom, right;
    } r;
    GUEST<Rect> rswapped;
    BOOLEAN doubleclick, ctlchanged;
    BOOLEAN hiliteempty, onlyone, userects, disjoint, extend;
    BOOLEAN initial;
    enum
    {
        Off,
        On,
        UseSense
    } cellvalue; /* order is important here */
    Byte flags;
    EventRecord evt;
    Rect anchor, oldselrect, newselrect, newcellr, pinrect;
    Cell c, oldcellunswapped, newcellunswapped;
    GUEST<Cell> newcell, oldcell, cswapped;
    LONGINT l;
    INTEGER dh, dv;
    Point p;

    doubleclick = false;
    if(PtInRect(pt, &HxX(list, rView)))
    {

        flags = Hx(list, selFlags);
        newcell.h = CW(pt.h);
        newcell.v = CW(pt.v);
        findcell(&newcell, list);
        if(newcell.h == HxX(list, lastClick.h) && newcell.v == HxX(list, lastClick.v) && TickCount() < Hx(list, clikTime) + CL(DoubleTime))
            doubleclick = true;
        HxX(list, lastClick) = newcell;
        hiliteempty = !(flags & lNoNilHilite);
        if(((mods & shiftKey) || (flags & lExtendDrag)) && !(flags & lOnlyOne))
        {
            onlyone = false;
            disjoint = !(flags & lNoDisjoint);
            userects = !(flags & lNoRect);
            cellvalue = (flags & lUseSense) ? UseSense : On;
            extend = (flags & lUseSense) ? false : !(flags & lNoExtend);
        }
        else if((mods & cmdKey) && !(flags & lOnlyOne))
        {
            onlyone = false;
            disjoint = !(flags & lNoDisjoint);
            userects = false;
            cellvalue = UseSense;
            extend = false;
        }
        else
        {
            onlyone = true;
            disjoint = false;
            userects = false;
            cellvalue = On;
            extend = false;
        }
        initial = C_LGetSelect(false, &newcell, list);
        if(cellvalue == UseSense)
            cellvalue = initial ? Off : On;
        if(!disjoint && !initial)
            rectvalue(&HxX(list, dataBounds), Off, list, hiliteempty);

        if(userects)
        {
            anchor.top = anchor.bottom = 0;
            if(extend)
            {
                rswapped = HxX(list, dataBounds);
                r.top = CW(rswapped.top);
                r.left = CW(rswapped.left);
                r.bottom = CW(rswapped.bottom);
                r.right = CW(rswapped.right);
                for(c.h = r.left; c.h < r.right; c.h++)
                    for(c.v = r.top; c.v < r.bottom; c.v++)
                    {
                        cswapped.h = CW(c.h);
                        cswapped.v = CW(c.v);
                        if(C_LGetSelect(false, &cswapped, list))
                            goto out1;
                    }
            out1:
                c.h = CW(cswapped.h);
                c.v = CW(cswapped.v);
                if(c.h != r.right)
                {
                    anchor.left = CW(c.h);

                    for(c.h = r.right - 1; c.h >= r.left; c.h--)
                        for(c.v = r.top; c.v < r.bottom; c.v++)
                        {
                            cswapped.h = CW(c.h);
                            cswapped.v = CW(c.v);
                            if(C_LGetSelect(false, &cswapped, list))
                                goto out2;
                        }
                out2:
                    c.h = CW(cswapped.h);
                    c.v = CW(cswapped.v);
                    anchor.right = CW(c.h + 1);

                    cswapped.h = CW(r.left);
                    cswapped.v = CW(r.top);
                    C_LGetSelect(true, &cswapped, list);
                    anchor.top = cswapped.v;

                    for(c.v = r.bottom - 1; c.v >= r.top; c.v--)
                        for(c.h = r.left; c.h < r.right; c.h++)
                        {
                            cswapped.h = CW(c.h);
                            cswapped.v = CW(c.v);
                            if(C_LGetSelect(false, &cswapped, list))
                                goto out3;
                        }
                out3:
                    anchor.bottom = CW(CW(cswapped.v) + 1);
                }
            }
            if(anchor.top == anchor.bottom)
            {
                anchor.top = newcell.v;
                anchor.left = newcell.h;
                anchor.bottom = CW(CW(anchor.top) + 1);
                anchor.right = CW(CW(anchor.left) + 1);
            }
            c.h = CW(anchor.left);
            c.v = CW(anchor.top);
            C_LRect(&rswapped, c, list);
            if(pt.h < CW(rswapped.right) && pt.v < CW(rswapped.bottom))
            {
                anchor.top = CW(CW(anchor.bottom) - 1);
                anchor.left = CW(CW(anchor.right) - 1);
            }
            else
            {
                anchor.bottom = CW(CW(anchor.top) + 1);
                anchor.right = CW(CW(anchor.left) + 1);
            }
            oldselrect = (flags & lUseSense) ? anchor : HxX(list, dataBounds);
        }

        HxX(list, clikTime) = CL(TickCount());
        HxX(list, clikLoc.h) = CW(pt.h);
        HxX(list, clikLoc.v) = CW(pt.v);
        oldcell.h = CWC(32767);

        evt.where.h = CW(pt.h);
        evt.where.v = CW(pt.v);
        pinrect = HxX(list, rView);
        pinrect.left = CW(CW(pinrect.left) - 1);
        pinrect.bottom = CW(CW(pinrect.bottom) - 1);
        do
        {
            HxX(list, mouseLoc) = evt.where;
            if(HxP(list, lClikLoop))
                if(CALLCLICK(HxP(list, lClikLoop)))
                    /*-->*/ break;
            p.h = CW(evt.where.h);
            p.v = CW(evt.where.v);
            if(!PtInRect(p, &HxX(list, rView)))
            {
                ctlchanged = false;
                scrollh = HxP(list, hScroll);
                scrollv = HxP(list, vScroll);
                dh = 0;
                dv = 0;
                if(CW(evt.where.h) < Hx(list, rView.left))
                {
                    if(scrollh)
                    {
                        SetCtlValue(scrollh, GetCtlValue(scrollh) - 1);
                        ctlchanged = true;
                    }
                    else
                        dh = -1;
                }
                else if(CW(evt.where.h) > Hx(list, rView.right))
                {
                    if(scrollh)
                    {
                        SetCtlValue(scrollh, GetCtlValue(scrollh) + 1);
                        ctlchanged = true;
                    }
                    else
                        dh = 1;
                }
                if(CW(evt.where.v) < Hx(list, rView.top))
                {
                    if(scrollv)
                    {
                        SetCtlValue(scrollv, GetCtlValue(scrollv) - 1);
                        ctlchanged = true;
                    }
                    else
                        dv = -1;
                }
                else if(CW(evt.where.v) > Hx(list, rView.bottom))
                {
                    if(scrollv)
                    {
                        SetCtlValue(scrollv, GetCtlValue(scrollv) + 1);
                        ctlchanged = true;
                    }
                    else
                        dv = 1;
                }
                if(ctlchanged)
                    scrollbyvalues(list);
                else
                    C_LScroll(dh, dv, list);
            }
            p.h = CW(evt.where.h);
            p.v = CW(evt.where.v);
            l = PinRect(&pinrect, p);
            newcell.h = CW(LoWord(l));
            newcell.v = CW(HiWord(l));
            findcell(&newcell, list);
            if(userects)
            {
                newcellr.top = newcell.v;
                newcellr.left = newcell.h;
                newcellr.bottom = CW(CW(newcellr.top) + 1);
                newcellr.right = CW(CW(newcellr.left) + 1);
                UnionRect(&anchor, &newcellr, &newselrect);
                rect2value(&oldselrect, &newselrect, !cellvalue, list,
                           hiliteempty);
                rectvalue(&newselrect, cellvalue, list, hiliteempty);
                oldselrect = newselrect;
            }
            else
            {
                if(newcell.h != oldcell.h || newcell.v != oldcell.v)
                {
                    if(onlyone && oldcell.h != CWC(32767))
                    {
                        oldcellunswapped.h = CW(oldcell.h);
                        oldcellunswapped.v = CW(oldcell.v);
                        setselectnilflag(false, oldcellunswapped, list,
                                         hiliteempty);
                    }
                    newcellunswapped.h = CW(newcell.h);
                    newcellunswapped.v = CW(newcell.v);
                    setselectnilflag(cellvalue, newcellunswapped, list,
                                     hiliteempty);
                    oldcell = newcell;
                }
            }
        } while(!OSEventAvail(mUpMask, &evt) && (GlobalToLocal(&evt.where), true));
    }
    else if(((ch = HxP(list, hScroll)) && PtInRect(pt, &HxX(ch, contrlRect))) || ((ch = HxP(list, vScroll)) && PtInRect(pt, &HxX(ch, contrlRect))))
    {
        if(TestControl(ch, pt) == inThumb)
        {
            TrackControl(ch, pt, (ProcPtr)0);
            scrollbyvalues(list);
        }
        else
            TrackControl(ch, pt, (ProcPtr)P_ROMlib_mytrack);
    }
    return doubleclick;
    return 0;
}

PUBLIC pascal trap LONGINT Executor::C_LLastClick(ListHandle list) /* IMIV-273 */
{
    return ((LONGINT)Hx(list, lastClick.v) << 16) | (unsigned short)Hx(list, lastClick.h);
}

PUBLIC pascal trap void Executor::C_LSetSelect(BOOLEAN setit, /* IMIV-273 */
                                               Cell cell, ListHandle list)
{
    setselectnilflag(setit, cell, list, true);
}
