/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/picture.h"

using namespace Executor;

void Executor::C_StdRect(GrafVerb v, Rect *rp)
{
    RgnHandle rh, rh2;
    PAUSEDECL;
    Rect patcheduprect;

#define MOREINSANECOMPATIBILITY
#if defined(MOREINSANECOMPATIBILITY)
    if(v == frame && PORT_REGION_SAVE_X(thePort))
    {
        if(CW(rp->left) > CW(rp->right))
        {
            patcheduprect = *rp;
            patcheduprect.left = rp->right;
            patcheduprect.right = rp->left;
            if(CW(rp->top) > CW(rp->bottom))
            {
                patcheduprect.top = rp->bottom;
                patcheduprect.bottom = rp->top;
            }
            rp = &patcheduprect;
        }
        else if(CW(rp->top) > CW(rp->bottom))
        {
            patcheduprect = *rp;
            patcheduprect.top = rp->bottom;
            patcheduprect.bottom = rp->top;
            rp = &patcheduprect;
        }
        if(rp == &patcheduprect)
        {
            rh = NewRgn();
            RectRgn(rh, rp);
            XorRgn(rh,
                   (RgnHandle)PORT_REGION_SAVE(thePort),
                   (RgnHandle)PORT_REGION_SAVE(thePort));
            DisposeRgn(rh);
            /*-->*/ return;
        }
    }
#endif /* MOREINSANECOMPATIBILITY */

    if(EmptyRect(rp))
        /*-->*/ return;

    if(thePort->picSave)
    {
        ROMlib_drawingverbrectpicupdate(v, rp);
        PICOP(OP_frameRect + (int)v);
        PICWRITE(rp, sizeof(*rp));
    }

    PAUSERECORDING;
    rh = NewRgn();
    RectRgn(rh, rp);
    switch(v)
    {
        case frame:
            if(PORT_REGION_SAVE_X(thePort))
                XorRgn(rh,
                       (RgnHandle)PORT_REGION_SAVE(thePort),
                       (RgnHandle)PORT_REGION_SAVE(thePort));
            if(PORT_PEN_VIS(thePort) >= 0)
            {
                rh2 = NewRgn();
                RectRgn(rh2, rp);
                InsetRgn(rh2,
                         Cx(PORT_PEN_SIZE(thePort).h),
                         Cx(PORT_PEN_SIZE(thePort).v));
                XorRgn(rh, rh2, rh);
                StdRgn(paint, rh);
                DisposeRgn(rh2);
            }
            break;
        case paint:
        case erase:
        case invert:
        case fill:
            StdRgn(v, rh);
    }
    DisposeRgn(rh);
    RESUMERECORDING;
}
