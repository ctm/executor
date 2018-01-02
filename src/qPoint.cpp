/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "rsys/cquick.h"

using namespace Executor;

PUBLIC pascal trap void Executor::C_AddPt(Point src, GUEST<Point> * dst)
{
    dst->h = CW(CW(dst->h) + (src.h));
    dst->v = CW(CW(dst->v) + (src.v));
}

PUBLIC pascal trap void Executor::C_SubPt(Point src, GUEST<Point> * dst)
{
    dst->h = CW(CW(dst->h) - (src.h));
    dst->v = CW(CW(dst->v) - (src.v));
}

PUBLIC pascal trap void Executor::C_SetPt(GUEST<Point> * pt, INTEGER h, INTEGER v)
{
    pt->h = CW(h);
    pt->v = CW(v);
}

PUBLIC pascal trap BOOLEAN Executor::C_EqualPt(Point p1, Point p2)
{
    return (p1.h == p2.h && p1.v == p2.v);
}

PUBLIC pascal trap void Executor::C_LocalToGlobal(GUEST<Point> * pt)
{
    if(thePortX)
    {
        pt->h = CW(CW(pt->h) - (CW(PORT_BOUNDS(thePort).left)));
        pt->v = CW(CW(pt->v) - (CW(PORT_BOUNDS(thePort).top)));
    }
}

PUBLIC pascal trap void Executor::C_GlobalToLocal(GUEST<Point> * pt)
{
    if(thePortX)
    {
        pt->h = CW(CW(pt->h) + (Cx(PORT_BOUNDS(thePort).left)));
        pt->v = CW(CW(pt->v) + (Cx(PORT_BOUNDS(thePort).top)));
    }
}
