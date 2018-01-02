/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

/* HLock checked by ctm on Mon May 13 17:55:01 MDT 1991 */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/quick.h"
#include "rsys/cquick.h"

using namespace Executor;

PolyHandle Executor::C_OpenPoly()
{
    PolyHandle ph;

    ph = (PolyHandle)NewHandle((Size)SMALLPOLY);
    HxX(ph, polySize) = CWC(SMALLPOLY);
    PORT_POLY_SAVE_X(thePort) = RM((Handle)ph);
    HidePen();
    return (ph);
}

void Executor::C_ClosePoly()
{
    INTEGER top = 32767, left = 32767, bottom = -32767, right = -32767;
    GUEST<INTEGER> *ip, *ep;
    INTEGER i;
    PolyHandle ph;

    ph = (PolyHandle)PORT_POLY_SAVE(thePort);
    for(ip = (GUEST<INTEGER> *)((char *)STARH(ph) + SMALLPOLY),
    ep = (GUEST<INTEGER> *)((char *)STARH(ph) + Hx(ph, polySize));
        ip != ep;)
    {
        if((i = CW(*ip)) <= top)
            top = i;
        ++ip;
        if(i >= bottom)
            bottom = i;
        if((i = CW(*ip)) <= left)
            left = i;
        ++ip;
        if(i >= right)
            right = i;
    }
    HxX(ph, polyBBox.top) = CW(top);
    HxX(ph, polyBBox.left) = CW(left);
    HxX(ph, polyBBox.bottom) = CW(bottom);
    HxX(ph, polyBBox.right) = CW(right);
    PORT_POLY_SAVE_X(thePort) = nullptr;
    ShowPen();
}

void Executor::C_KillPoly(PolyHandle poly)
{
    DisposHandle((Handle)poly);
}

void Executor::C_OffsetPoly(PolyHandle poly, INTEGER dh,
                            INTEGER dv) /* Note: IM I-191 is wrong */
{
    GUEST<Point> *pp, *ep;

    if(dh || dv)
    {
        HxX(poly, polyBBox.top) = CW(Hx(poly, polyBBox.top) + dv);
        HxX(poly, polyBBox.bottom) = CW(Hx(poly, polyBBox.bottom) + dv);
        HxX(poly, polyBBox.left) = CW(Hx(poly, polyBBox.left) + dh);
        HxX(poly, polyBBox.right) = CW(Hx(poly, polyBBox.right) + dh);
        pp = HxX(poly, polyPoints);
        ep = (GUEST<Point> *)(((char *)STARH(poly)) + Hx(poly, polySize));
        while(pp != ep)
        {
            pp->h = CW(CW(pp->h) + (dh));
            pp->v = CW(CW(pp->v) + (dv));
            pp++;
        }
    }
}
