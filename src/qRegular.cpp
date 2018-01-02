/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "DialogMgr.h"
#include "rsys/quick.h"
#include "rsys/cquick.h"
#include "rsys/ctl.h"
#include "rsys/wind.h"

using namespace Executor;

PUBLIC pascal trap void Executor::C_FrameRect(Rect * r)
{
    CALLRECT(frame, r);
}

PUBLIC pascal trap void Executor::C_PaintRect(Rect * r)
{
    CALLRECT(paint, r);
}

PUBLIC pascal trap void Executor::C_EraseRect(Rect * r)
{
    CALLRECT(erase, r);
}

PUBLIC pascal trap void Executor::C_InvertRect(Rect * r)
{
    CALLRECT(invert, r);
}

PUBLIC pascal trap void Executor::C_FillRect(Rect * r, Pattern pat)
{
    if(!EmptyRgn(PORT_VIS_REGION(thePort)))
    {
        ROMlib_fill_pat(pat);
        CALLRECT(fill, r);
    }
}

PUBLIC pascal trap void Executor::C_FrameOval(Rect * r)
{
    CALLOVAL(frame, r);
}

PUBLIC pascal trap void Executor::C_PaintOval(Rect * r)
{
    CALLOVAL(paint, r);
}

PUBLIC pascal trap void Executor::C_EraseOval(Rect * r)
{
    CALLOVAL(erase, r);
}

PUBLIC pascal trap void Executor::C_InvertOval(Rect * r)
{
    CALLOVAL(invert, r);
}

PUBLIC pascal trap void Executor::C_FillOval(Rect * r, Pattern pat)
{
    ROMlib_fill_pat(pat);
    CALLOVAL(fill, r);
}

PRIVATE bool
rect_matches_control_item(WindowPtr w, Rect *rp)
{
    bool retval;
    ControlHandle c;

    retval = false;
    for(c = WINDOW_CONTROL_LIST(w); !retval && c; c = CTL_NEXT_CONTROL(c))
    {
        Rect r;

        r = CTL_RECT(c);
        retval = ((CW(r.top) - CW(rp->top) == CW(rp->bottom) - CW(r.bottom)) && (CW(r.left) - CW(rp->left) == CW(rp->right) - CW(r.right)));
    }

    return retval;
}

PUBLIC pascal trap void Executor::C_FrameRoundRect(Rect * r, INTEGER ow, INTEGER oh)
{
    bool do_rect;

    do_rect = false;

    if(ROMlib_cdef0_is_rectangular)
    {
        AuxWinHandle aux;

        aux = MR(*lookup_aux_win(thePort));
        if(aux && rect_matches_control_item(HxP(aux, awOwner), r))
            do_rect = true;
    }

    if(do_rect)
        FrameRect(r);
    else
        CALLRRECT(frame, r, ow, oh);
}

PUBLIC pascal trap void Executor::C_PaintRoundRect(Rect * r, INTEGER ow, INTEGER oh)
{
    CALLRRECT(paint, r, ow, oh);
}

PUBLIC pascal trap void Executor::C_EraseRoundRect(Rect * r, INTEGER ow, INTEGER oh)
{
    CALLRRECT(erase, r, ow, oh);
}

PUBLIC pascal trap void Executor::C_InvertRoundRect(Rect * r, INTEGER ow, INTEGER oh)
{
    CALLRRECT(invert, r, ow, oh);
}

PUBLIC pascal trap void Executor::C_FillRoundRect(Rect * r, INTEGER ow, INTEGER oh, Pattern pat)
{
    ROMlib_fill_pat(pat);
    CALLRRECT(fill, r, ow, oh);
}

PUBLIC pascal trap void Executor::C_FrameArc(Rect * r, INTEGER start, INTEGER angle)
{
    CALLARC(frame, r, start, angle);
}

PUBLIC pascal trap void Executor::C_PaintArc(Rect * r, INTEGER start, INTEGER angle)
{
    CALLARC(paint, r, start, angle);
}

PUBLIC pascal trap void Executor::C_EraseArc(Rect * r, INTEGER start, INTEGER angle)
{
    CALLARC(erase, r, start, angle);
}

PUBLIC pascal trap void Executor::C_InvertArc(Rect * r, INTEGER start, INTEGER angle)
{
    CALLARC(invert, r, start, angle);
}

PUBLIC pascal trap void Executor::C_FillArc(Rect * r, INTEGER start, INTEGER angle, Pattern pat)
{
    ROMlib_fill_pat(pat);
    CALLARC(fill, r, start, angle);
}

PUBLIC pascal trap void Executor::C_FrameRgn(RgnHandle rh)
{
    CALLRGN(frame, rh);
}

PUBLIC pascal trap void Executor::C_PaintRgn(RgnHandle rh)
{
    CALLRGN(paint, rh);
}

PUBLIC pascal trap void Executor::C_EraseRgn(RgnHandle rh)
{
    CALLRGN(erase, rh);
}

PUBLIC pascal trap void Executor::C_InvertRgn(RgnHandle rh)
{
    CALLRGN(invert, rh);
}

PUBLIC pascal trap void Executor::C_FillRgn(RgnHandle rh, Pattern pat)
{
    ROMlib_fill_pat(pat);
    CALLRGN(fill, rh);
}

PUBLIC pascal trap void Executor::C_FramePoly(PolyHandle poly)
{
    CALLPOLY(frame, poly);
}

PUBLIC pascal trap void Executor::C_PaintPoly(PolyHandle poly)
{
    CALLPOLY(paint, poly);
}

PUBLIC pascal trap void Executor::C_ErasePoly(PolyHandle poly)
{
    CALLPOLY(erase, poly);
}

PUBLIC pascal trap void Executor::C_InvertPoly(PolyHandle poly)
{
    CALLPOLY(invert, poly);
}

PUBLIC pascal trap void Executor::C_FillPoly(PolyHandle poly, Pattern pat)
{
    ROMlib_fill_pat(pat);
    CALLPOLY(fill, poly);
}
