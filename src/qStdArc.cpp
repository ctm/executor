/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qStdArc[] = "$Id: qStdArc.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"
#include "ToolboxUtil.h"

#include "rsys/cquick.h"
#include "rsys/picture.h"

using namespace Executor;

#define returnhv(hh, vv) \
    {                    \
        ptp->h = hh;     \
        ptp->v = vv;     \
        return;          \
    }

namespace Executor
{
PRIVATE void getpoint(INTEGER, Rect *, Point *);
PRIVATE INTEGER findwall(Rect *r, INTEGER h, INTEGER v);
}

A3(PRIVATE, void, getpoint, INTEGER, angle, Rect *, r, Point *, ptp)
{
    INTEGER left = CW(r->left), top = CW(r->top),
            right = CW(r->right), bottom = CW(r->bottom);
    INTEGER radh = (right - left) / 2,
            radv = (bottom - top) / 2;
    INTEGER centh = left + radh,
            centv = top + radv;

    MoveTo(centh, centv);
    if(angle == 0)
    {
        /*-->*/ returnhv(centh, top)
    }
    else if(angle == 90)
    {
        /*-->*/ returnhv(right, centv)
    }
    else if(angle == 180)
    {
        /*-->*/ returnhv(centh, bottom)
    }
    else if(angle == 270)
    {
        /*-->*/ returnhv(left, centv)
    }
    else if(angle >= 45 && angle <= 135)
    {
        /*-->*/ returnhv(right, centv - (INTEGER)(FixMul(SlopeFromAngle(90 - angle), -(LONGINT)radv << 16) >> 16))
    }
    else if(angle >= 225 && angle <= 315)
    {
        /*-->*/ returnhv(left, centv + (INTEGER)(FixMul(SlopeFromAngle(90 - angle), -(LONGINT)radv << 16) >> 16))
    }
    else if(angle < 45 || angle > 315)
    {
        /*-->*/ returnhv(centh + (INTEGER)(FixMul(SlopeFromAngle(angle),
                                                  -(LONGINT)radh << 16)
                                           >> 16),
                         top)
    }
    else
    {
        /*-->*/ returnhv(centh - (INTEGER)(FixMul(SlopeFromAngle(angle),
                                                  -(LONGINT)radh << 16)
                                           >> 16),
                         bottom)
    }
}

#define RTop 0
#define RRight 1
#define RBottom 2
#define RLeft 3

A3(PRIVATE, INTEGER, findwall, Rect *, r, INTEGER, h, INTEGER, v)
{
    if(v == CW(r->top)) /* the order of tests is important */
        return (RTop); /* don't change them if you don't see */
    else if(h == CW(r->right)) /* why */
        return (RRight);
    else if(v == CW(r->bottom))
        return (RBottom);
    else
        return (RLeft);
}

P4(PUBLIC pascal trap, void, StdArc, GrafVerb, verb, Rect *, r,
   INTEGER, starta, INTEGER, arca)
{
    INTEGER left = CW(r->left), top = CW(r->top),
            right = CW(r->right), bottom = CW(r->bottom);
    INTEGER ewall;
    Point spt, ept;
    INTEGER h, v;
    INTEGER enda;
    INTEGER done;
    GUEST<RgnHandle> saveclip;
    RgnHandle rh;
    GUEST<Point> saveloc;
    GUEST<INTEGER> tmpvis;
    GUEST<INTEGER> swappedarca, swappedstarta;
    PAUSEDECL;

    if(EmptyRect(r))
        /*-->*/ return;

    if(arca <= -360 || arca >= 360)
    {
        StdOval(verb, r);
        /*-->*/ return;
    }

    if(thePort->picSave)
    {
        ROMlib_drawingverbrectpicupdate(verb, r);
        PICOP(OP_frameArc + (int)verb);
        PICWRITE(r, sizeof(*r));
        swappedstarta = CW(starta);
        PICWRITE(&swappedstarta, sizeof(swappedstarta));
        swappedarca = CW(arca);
        PICWRITE(&swappedarca, sizeof(swappedarca));
    }

    if(PORT_PEN_VIS(thePort) < 0)
        /*-->*/ return;
    saveloc = PORT_PEN_LOC(thePort);
    PAUSERECORDING;
    tmpvis = PORT_PEN_VIS_X(thePort);
    PORT_PEN_VIS_X(thePort) = CWC(0);
    OpenRgn();
    enda = starta + arca;
    if(arca < 0)
    {
        arca = starta; /* use arca as a temp... not needed anymore */
        starta = enda;
        enda = arca;
    }
    while(starta < 0)
        starta += 360;
    while(enda < 0)
        enda += 360;
    starta %= 360;
    enda %= 360;

    getpoint(starta, r, &spt);
    getpoint(enda, r, &ept);

    MoveTo(left + (right - left) / 2, top + (bottom - top) / 2);

    ewall = findwall(r, ept.h, ept.v);
    LineTo(h = spt.h, v = spt.v);

    for(done = false; !done;)
        switch(findwall(r, h, v))
        {
            case RTop:
                if(ewall == RTop && h <= ept.h)
                {
                    LineTo(ept.h, top);
                    done = true;
                    break;
                }
                LineTo(h = right, v = top);
            case RRight:
                if(ewall == RRight && v <= ept.v)
                {
                    LineTo(right, ept.v);
                    done = true;
                    break;
                }
                LineTo(h = right, v = bottom);
            case RBottom:
                if(ewall == RBottom && h >= ept.h)
                {
                    LineTo(ept.h, bottom);
                    done = true;
                    break;
                }
                LineTo(h = left, v = bottom);
            case RLeft:
                if(ewall == RLeft && v >= ept.v)
                {
                    LineTo(left, ept.v);
                    done = true;
                    break;
                }
                LineTo(h = left, v = top);
        }

    LineTo(left + (right - left) / 2, top + (bottom - top) / 2);
    rh = NewRgn();
    CloseRgn(rh);
    PORT_PEN_VIS_X(thePort) = tmpvis;
    saveclip = PORT_CLIP_REGION_X(thePort);
    SectRgn(MR(saveclip), rh, rh);
    PORT_CLIP_REGION_X(thePort) = RM(rh);
    StdOval(verb, r);
    PORT_CLIP_REGION_X(thePort) = saveclip;
    DisposeRgn(rh);
    RESUMERECORDING;
    PORT_PEN_LOC(thePort) = saveloc;
}
