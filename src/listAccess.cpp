/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ListMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ListMgr.h"
#include "MemoryMgr.h"
#include "IntlUtil.h"
#include "rsys/list.h"
#include "rsys/hook.h"

using namespace Executor;

void Executor::C_LFind(GUEST<INTEGER> *offsetp, GUEST<INTEGER> *lenp,
                       Cell cell, ListHandle list) /* IMIV-274 */
{
    GUEST<INTEGER> *ip;

    if((ip = ROMlib_getoffp(cell, list)))
    {
        *offsetp = *ip++ & CWC(0x7FFF);
        *lenp = CW((CW(*ip) & 0x7FFF) - CW(*offsetp));
    }
    else
        *offsetp = *lenp = CWC(-1);
}

BOOLEAN Executor::C_LNextCell(BOOLEAN hnext, BOOLEAN vnext, GUEST<Cell> *cellp,
                              ListHandle list) /* IMIV-274 */
{
    BOOLEAN retval;
    Point scratch;
    INTEGER right, bottom;

    scratch.v = CW(cellp->v);
    scratch.h = CW(cellp->h);
    right = Hx(list, dataBounds.right);
    bottom = Hx(list, dataBounds.bottom);
    if(hnext)
    {
        if(++scratch.h >= right)
            if(vnext && ++scratch.v < bottom)
            {
                scratch.h = 0;
                retval = true;
            }
            else
                retval = false;
        else
            retval = true;
    }
    else
    {
        if(vnext && ++scratch.v < bottom)
            retval = true;
        else
            retval = false;
    }
    if(retval)
    {
        cellp->v = CW(scratch.v);
        cellp->h = CW(scratch.h);
    }
    return retval;
}

void Executor::C_LRect(Rect *cellrect, Cell cell, ListHandle list) /* IMIV-274 */
{
    Point csize;
    INTEGER temp;

    if(PtInRect(cell, &HxX(list, visible)))
    {
        csize.h = Hx(list, cellSize.h);
        csize.v = Hx(list, cellSize.v);
        *cellrect = HxX(list, rView);
        cellrect->top = CW(CW(cellrect->top) + ((cell.v - Hx(list, visible.top)) * csize.v));
        cellrect->left = CW(CW(cellrect->left) + ((cell.h - Hx(list, visible.left)) * csize.h));
        if((temp = CW(cellrect->top) + csize.v) < CW(cellrect->bottom))
            cellrect->bottom = CW(temp);
        if((temp = CW(cellrect->left) + csize.h) < CW(cellrect->right))
            cellrect->right = CW(temp);
    }
    else
    {
        cellrect->top = cellrect->left = cellrect->bottom = cellrect->right = CWC(0);
    }
}

typedef INTEGER (*cmpf)(Ptr p1, Ptr p2, INTEGER len1, INTEGER len2);


#define CALLCMP(a1, a2, a3, a4, fp) \
    ROMlib_CALLCMP(a1, a2, a3, a4, (cmpf)fp)

namespace Executor
{
static inline INTEGER ROMlib_CALLCMP(Ptr, Ptr, INTEGER, INTEGER, cmpf);
}

static inline INTEGER Executor::ROMlib_CALLCMP(Ptr p1, Ptr p2, INTEGER l1, INTEGER l2, cmpf fp)
{
    INTEGER retval;

    if(fp == (cmpf)P_IUMagString)
        retval = C_IUMagString(p1, p2, l1, l2);
    else
    {
        ROMlib_hook(list_cmpnumber);
        retval = CToPascalCall((void *)fp, ctop(&C_IUMagString), p1, p2, l1, l2);
    }
    return retval;
}


BOOLEAN Executor::C_LSearch(Ptr dp, INTEGER dl, Ptr proc, GUEST<Cell> *cellp,
                            ListHandle list) /* IMIV-274 */
{
    GUEST<INTEGER> offS, lenS;
    INTEGER off, len;

    Cell cell;
    GUEST<Cell> swappedcell;
    cmpf fp;

    HLock((Handle)list);
    HLock((Handle)HxP(list, cells));

    fp = proc ? (cmpf)proc : (cmpf)P_IUMagString;
    cell.h = CW(cellp->h);
    cell.v = CW(cellp->v);
    swappedcell = *cellp;
    /* TODO: SPEEDUP:  the following is a stupid way to do the loop, instead ip
		 and ep should be used! */
    while((C_LFind(&offS, &lenS, cell, list), len = CW(lenS), off = CW(offS),
           len != -1)
          && CALLCMP(dp, (Ptr)STARH(HxP(list, cells)) + off, dl, len, fp) != 0)
        if(!C_LNextCell(true, true, &swappedcell, list))
        {
            cell.h = Hx(list, dataBounds.right);
            cell.v = Hx(list, dataBounds.bottom);
        }
        else
        {
            cell.h = CW(swappedcell.h);
            cell.v = CW(swappedcell.v);
        }

    HUnlock((Handle)HxP(list, cells));
    HUnlock((Handle)list);
    if(len != -1)
    {
        cellp->h = CW(cell.h);
        cellp->v = CW(cell.v);
        /*-->*/ return true;
    }
    else
        return false;
}

void Executor::C_LSize(INTEGER width, INTEGER height,
                       ListHandle list) /* IMIV-274 */
{
    INTEGER oldright, oldbottom, newright, newbottom;
    ControlHandle cv, ch;
    RgnHandle rectrgn, updatergn;
    Rect r;
    Point p;

    oldright = Hx(list, rView.right);
    oldbottom = Hx(list, rView.bottom);
    newright = Hx(list, rView.left) + width;
    HxX(list, rView.right) = CW(newright);
    newbottom = Hx(list, rView.top) + height;
    HxX(list, rView.bottom) = CW(newbottom);
    ch = HxP(list, hScroll);
    cv = HxP(list, vScroll);

    p.h = Hx(list, cellSize.h);
    p.v = Hx(list, cellSize.v);
    C_LCellSize(p, list); /* sets visible */

    updatergn = NewRgn();
    rectrgn = NewRgn();
    if(newright != oldright)
    {
        if(newbottom != oldbottom)
        { /* both are different */
            if(ch)
            {
                MoveControl(ch, Hx(list, rView.left) - 1, newbottom);
                SizeControl(ch, newright - Hx(list, rView.left) + 2, 16);
            }
            if(cv)
            {
                MoveControl(cv, newright, Hx(list, rView.top) - 1);
                SizeControl(cv, 16, newbottom - Hx(list, rView.top) + 2);
            }
            r.top = CW(MIN(oldbottom, newbottom));
            r.bottom = CW(MAX(oldbottom, newbottom));
            r.left = CW(Hx(list, rView.left) - 1);
            r.right = CW(MAX(oldright, newright));
            if(ch)
                r.bottom = CW(CW(r.bottom) + (16));
            RectRgn(rectrgn, &r);
            UnionRgn(rectrgn, updatergn, updatergn);
        }
        else
        { /* just right different */
            if(ch)
            {
                SizeControl(ch, newright - Hx(list, rView.left) + 2, 16);
            }
            if(cv)
                MoveControl(cv, newright, Hx(list, rView.top) - 1);
        }
        r.left = CW(MIN(oldright, newright));
        r.right = CW(MAX(oldright, newright));
        r.top = CW(Hx(list, rView.top) - 1);
        r.bottom = CW(MAX(oldbottom, newbottom));
        if(cv)
            r.right = CW(CW(r.right) + (16));
        RectRgn(rectrgn, &r);
        UnionRgn(rectrgn, updatergn, updatergn);
    }
    else if(newbottom != oldbottom)
    { /* just bottom different */
        if(ch)
            MoveControl(ch, Hx(list, rView.left) - 1, newbottom);
        if(cv)
        {
            SizeControl(cv, 16, newbottom - Hx(list, rView.top) + 2);
        }
        r.top = CW(MIN(oldbottom, newbottom));
        r.bottom = CW(MAX(oldbottom, newbottom));
        r.left = CW(Hx(list, rView.left) - 1);
        r.right = CW(MAX(oldright, newright));
        if(ch)
            r.bottom = CW(CW(r.bottom) + (16));
        RectRgn(rectrgn, &r);
        UnionRgn(rectrgn, updatergn, updatergn);
    }
    C_LUpdate(updatergn, list);
    DisposeRgn(updatergn);
    DisposeRgn(rectrgn);
}
