/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ControlMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "WindowMgr.h"
#include "ControlMgr.h"
#include "EventMgr.h"
#include "ToolboxUtil.h"

#include "rsys/ctl.h"

using namespace Executor;

void Executor::C_MoveControl(ControlHandle c, INTEGER h, INTEGER v) /* IMI-325 */
{
    if(Hx(c, contrlVis))
    {
        HideControl(c);
        HxX(c, contrlRect.right) = CW(Hx(c, contrlRect.right)
                                      + h - Hx(c, contrlRect.left));
        HxX(c, contrlRect.bottom) = CW(Hx(c, contrlRect.bottom)
                                       + v - Hx(c, contrlRect.top));
        HxX(c, contrlRect.left) = CW(h);
        HxX(c, contrlRect.top) = CW(v);
        ShowControl(c);
    }
    else
    {
        HxX(c, contrlRect.right) = CW(Hx(c, contrlRect.right) + h - Hx(c, contrlRect.left));
        HxX(c, contrlRect.bottom) = CW(Hx(c, contrlRect.bottom) + v - Hx(c, contrlRect.top));
        HxX(c, contrlRect.left) = CW(h);
        HxX(c, contrlRect.top) = CW(v);
    }
}

void Executor::C_DragControl(ControlHandle c, Point p, Rect *limit, Rect *slop,
                             INTEGER axis) /* IMI-325 */
{
    RgnHandle rh;
    LONGINT l;

    CtlCallGuard guard(c);

    if(!(CTLCALL(c, dragCntl, 0) & 0xf000))
    {
        rh = NewRgn();
        CTLCALL(c, calcCntlRgn, ptr_to_longint(rh));
        l = DragGrayRgn(rh, p, limit, slop, axis, (ProcPtr)0);
        if((uint32_t)l != 0x80008000)
            MoveControl(c, Hx(c, contrlRect.left) + LoWord(l),
                        Hx(c, contrlRect.top) + HiWord(l));

        DisposeRgn(rh);
    }
}

void Executor::C_SizeControl(ControlHandle c, INTEGER width,
                             INTEGER height) /* IMI-326 */
{
    if(Hx(c, contrlVis))
    {
        HideControl(c);
        HxX(c, contrlRect.right) = CW(Hx(c, contrlRect.left) + width);
        HxX(c, contrlRect.bottom) = CW(Hx(c, contrlRect.top) + height);
        ShowControl(c);
    }
    else
    {
        HxX(c, contrlRect.right) = CW(Hx(c, contrlRect.left) + width);
        HxX(c, contrlRect.bottom) = CW(Hx(c, contrlRect.top) + height);
    }
}
