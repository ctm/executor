/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"

#include "rsys/quick.h"
#include "rsys/cquick.h"

using namespace Executor;

#define FillCxxx(CALLxxx)                                            \
    do                                                               \
    {                                                                \
        if(CGrafPort_p(thePort))                                     \
        {                                                            \
            GUEST<PixPatHandle> orig_fill_pixpat_x;                  \
                                                                     \
            PenMode(patCopy);                                        \
            orig_fill_pixpat_x = CPORT_FILL_PIXPAT_X(thePort);       \
            /* ROMlib_fill_pixpat (pixpat); */                       \
            CPORT_FILL_PIXPAT_X(thePort) = RM(pixpat);               \
            CALLxxx;                                                 \
            CPORT_FILL_PIXPAT_X(thePort) = orig_fill_pixpat_x;       \
        }                                                            \
        else                                                         \
        {                                                            \
            PATASSIGN(PORT_FILL_PAT(thePort), PIXPAT_1DATA(pixpat)); \
            CALLxxx;                                                 \
        }                                                            \
    } while(false)

PUBLIC pascal trap void Executor::C_FillCRect(Rect *r, PixPatHandle pixpat)
{
    /* #warning "restore settings after FillCxxx?" */
    FillCxxx(CALLRECT(fill, r));
}

PUBLIC pascal trap void Executor::C_FillCRoundRect(const Rect *r, short ovalWidth, short ovalHeight, PixPatHandle pixpat)
{
    FillCxxx(CALLRRECT(fill, (Rect *)r, ovalWidth, ovalHeight));
}

PUBLIC pascal trap void Executor::C_FillCOval(const Rect *r, PixPatHandle pixpat)
{
    FillCxxx(CALLOVAL(fill, (Rect *)r));
}

PUBLIC pascal trap void Executor::C_FillCArc(const Rect *r, short startAngle, short arcAngle, PixPatHandle pixpat)
{
    FillCxxx(CALLARC(fill, (Rect *)r, startAngle, arcAngle));
}

PUBLIC pascal trap void Executor::C_FillCPoly(PolyHandle poly, PixPatHandle pixpat)
{
    FillCxxx(CALLPOLY(fill, poly));
}

PUBLIC pascal trap void Executor::C_FillCRgn(RgnHandle rgn, PixPatHandle pixpat)
{
    FillCxxx(CALLRGN(fill, rgn));
}
