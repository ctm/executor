/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qPoint[] = "$Id: qPoint.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "rsys/cquick.h"

using namespace Executor;

P2(PUBLIC pascal trap, void, AddPt, Point, src, GUEST<Point> *, dst)
{
    dst->h = CW(CW(dst->h) + (src.h));
    dst->v = CW(CW(dst->v) + (src.v));
}

P2(PUBLIC pascal trap, void, SubPt, Point, src, GUEST<Point> *, dst)
{
    dst->h = CW(CW(dst->h) - (src.h));
    dst->v = CW(CW(dst->v) - (src.v));
}

P3(PUBLIC pascal trap, void, SetPt, GUEST<Point> *, pt, INTEGER, h, INTEGER, v)
{
    pt->h = CW(h);
    pt->v = CW(v);
}

P2(PUBLIC pascal trap, BOOLEAN, EqualPt, Point, p1, Point, p2)
{
    return (p1.h == p2.h && p1.v == p2.v);
}

P1(PUBLIC pascal trap, void, LocalToGlobal, GUEST<Point> *, pt)
{
    if(thePortX)
    {
        pt->h = CW(CW(pt->h) - (CW(PORT_BOUNDS(thePort).left)));
        pt->v = CW(CW(pt->v) - (CW(PORT_BOUNDS(thePort).top)));
    }
}

P1(PUBLIC pascal trap, void, GlobalToLocal, GUEST<Point> *, pt)
{
    if(thePortX)
    {
        pt->h = CW(CW(pt->h) + (Cx(PORT_BOUNDS(thePort).left)));
        pt->v = CW(CW(pt->v) + (Cx(PORT_BOUNDS(thePort).top)));
    }
}
