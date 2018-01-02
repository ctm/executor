/* Copyright 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in TextEdit.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "TextEdit.h"
#include "ToolboxEvent.h"
#include "MemoryMgr.h"

#include "rsys/tesave.h"
#include "rsys/cquick.h"
#include "rsys/mman.h"

using namespace Executor;

void Executor::C_TEPinScroll(int16_t dh, int16_t dv, TEHandle te) /* IMIV-57 */
{
    Rect *view_rect;
    Rect *dest_rect;
    SignedByte te_flags;
    TEPtr tep;

    te_flags = HGetState((Handle)te);
    HLock((Handle)te);
    tep = STARH(te);

    view_rect = &TEP_VIEW_RECT(tep);
    dest_rect = &TEP_DEST_RECT(tep);

    if(dv > 0)
    {
        int view_rect_top = CW(view_rect->top);
        int dest_rect_top = CW(dest_rect->top);

        /* `destRect.top' must stay below `viewRect.bottom' */

        if(dest_rect_top + dv > view_rect_top)
            dv = view_rect_top - dest_rect_top;
    }
    else
    {
        Point end_pt;
        int view_rect_bottom = CW(view_rect->bottom);
        int dest_rect_bottom;

        {
            int lineno;
            int length;

            length = TEP_LENGTH(tep);
            /* ### this should use `dest_rect -> bottom', but that isn't
	   updated correctly at the moment */
            TEP_CHAR_TO_POINT(tep, length, &end_pt);

            lineno = TEP_CHAR_TO_LINENO(tep, length);
            dest_rect_bottom = end_pt.v + TEP_HEIGHT_FOR_LINE(tep, lineno);
        }

        /* `destRect.bottom' must stay above `viewRect.bottom' */
        if(dest_rect_bottom + dv < view_rect_bottom)
            dv = view_rect_bottom - dest_rect_bottom;
    }

    {
        /* ### i fixed the above code, but didn't take a look at horiz
       scrolling yet */
        int16_t maxshift;

        if(dh > 0)
        {
            maxshift = CW(view_rect->left) - CW(dest_rect->left);
            if(maxshift > 0)
                dh = MIN(maxshift, dh);
            else
                dh = 0;
        }
        else
        {
            maxshift = CW(view_rect->left) - CW(dest_rect->left);
            if(maxshift < 0)
                dh = MAX(maxshift, dh);
            else
                dh = 0;
        }
    }

    HSetState((Handle)te, te_flags);

    if(dh || dv)
        TEScroll(dh, dv, te);
}

/*
 * teh paramater not within spec.  Shouldn't hurt anything with C calling
 * conventions.
 */

void Executor::ROMlib_teautoloop(TEHandle teh)
{
    GUEST<Point> pt;

    GetMouse(&pt);
    if(CW(pt.v) < Hx(teh, viewRect.top))
        TEPinScroll(0, Hx(teh, lineHeight), teh);
    else if(CW(pt.v) > Hx(teh, viewRect.bottom))
        TEPinScroll(0, -Hx(teh, lineHeight), teh);
}

static int16_t
getdelta(int16_t selstart, int16_t selstop,
         int16_t viewstart, int16_t viewstop)
{
    if(selstart < viewstart)
        return viewstart - selstart;
    else if(selstop > viewstop)
    {
        if(selstop - selstart > viewstop - viewstart)
            return viewstart - selstart;
        else
            return viewstop - selstop;
    }
    else
        return 0;
}

/*
 * NOTE:  We should be using SelRect in TESelView and elsewhere (sigh).
 */

void Executor::C_TESelView(TEHandle teh) /* IMIV-57 */
{
    int16_t dh, dv;
    Point start, stop;

    if(STARH(TEHIDDENH(teh))->flags & CLC(TEAUTOVIEWBIT))
    {
        TE_CHAR_TO_POINT(teh, TE_SEL_START(teh), &start);
        TE_CHAR_TO_POINT(teh, TE_SEL_END(teh), &stop);
        stop.v += Hx(teh, lineHeight);
        dv = getdelta(start.v, stop.v + Hx(teh, lineHeight),
                      Hx(teh, viewRect.top), Hx(teh, viewRect.bottom));
        dh = getdelta(start.h, stop.h,
                      Hx(teh, viewRect.left), Hx(teh, viewRect.right));
        TEPinScroll(dh, dv, teh);
    }
}

void Executor::C_TEAutoView(BOOLEAN autoflag, TEHandle teh) /* IMIV-57 */
{
    if(autoflag)
        STARH(TEHIDDENH(teh))->flags.raw_or(CLC(TEAUTOVIEWBIT));
    else
        STARH(TEHIDDENH(teh))->flags.raw_and(CLC(~TEAUTOVIEWBIT));
}
