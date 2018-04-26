/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

/* HLock checked by ctm on Mon May 13 17:57:59 MDT 1991 */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/picture.h"

using namespace Executor;

static void polyrgn(PolyHandle ph, RgnHandle rh)
{
    GUEST<Point> *pp, *ep;
    Point firstp;
    GUEST<INTEGER> tmpvis;
    INTEGER state;

    state = HGetState((Handle)ph);
    HLock((Handle)ph);
    pp = HxX(ph, polyPoints);
    ep = (GUEST<Point> *)((char *)STARH(ph) + Hx(ph, polySize));
    firstp.h = Hx(ph, polyPoints[0].h);
    firstp.v = Hx(ph, polyPoints[0].v);
    if(CW(ep[-1].h) == firstp.h && CW(ep[-1].v) == firstp.v)
        ep--;

    tmpvis = PORT_PEN_VIS_X(thePort);
    PORT_PEN_VIS_X(thePort) = CWC(0);
    OpenRgn();
    MoveTo(CW(pp->h), CW(pp->v));
    pp++;
    while(pp != ep)
    {
        LineTo(CW(pp->h), CW(pp->v));
        pp++;
    }
    LineTo(firstp.h, firstp.v);
    CloseRgn(rh);
    PORT_PEN_VIS_X(thePort) = tmpvis;
    HSetState((Handle)ph, state);
}

void Executor::C_StdPoly(GrafVerb verb, PolyHandle ph)
{
    RgnHandle rh;
    Point p;
    GUEST<Point> *pp, *ep;
    Point firstp;
    INTEGER state;
    PAUSEDECL;

    if(!ph || !*ph || HxX(ph, polySize) == CWC(10) || EmptyRect(&HxX(ph, polyBBox)))
        /*-->*/ return;

    state = HGetState((Handle)ph);
    HLock((Handle)ph);
    if(thePort->picSave)
    {
        ROMlib_drawingverbpicupdate(verb);
        PICOP(OP_framePoly + (int)verb);
        PICWRITE(STARH(ph), Hx(ph, polySize));
    }

    if(PORT_PEN_VIS(thePort) < 0 && !PORT_REGION_SAVE_X(thePort)
       && verb != frame)
    {
        HSetState((Handle)ph, state);
        /*-->*/ return;
    }

    PAUSERECORDING;
    switch(verb)
    {
        case frame:
            /* we used to unconditionally close the polygon here, but
	   testing on the mac shows that is incorrect */

            pp = HxX(ph, polyPoints);
            ep = (GUEST<Point> *)((char *)STARH(ph) + Hx(ph, polySize));
            firstp.h = CW(pp[0].h);
            firstp.v = CW(pp[0].v);
            MoveTo(firstp.h, firstp.v);
            for(++pp; pp != ep; pp++)
            {
                p.h = CW(pp[0].h);
                p.v = CW(pp[0].v);
                StdLine(p);
                PORT_PEN_LOC(thePort) = pp[0];
            }

            break;
        case paint:
        case erase:
        case invert:
        case fill:
            rh = NewRgn();
            polyrgn(ph, rh);
            StdRgn(verb, rh);
            DisposeRgn(rh);
    }
    RESUMERECORDING;
    HSetState((Handle)ph, state);
}
