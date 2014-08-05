/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qStdRRect[] =
	    "$Id: qStdRRect.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/picture.h"

using namespace Executor;
using namespace ByteSwap;

#define TERM (*ip++ = RGNSTOPX)

#define ADD4(y, x1, x2)						\
    (*ip++ = BigEndianValue((y)), *ip++ = BigEndianValue((x1)), *ip++ = BigEndianValue((x2)), TERM)

#define ADD6(y, x1, x2, x3, x4)					\
    (*ip++ = BigEndianValue((y)), *ip++ = BigEndianValue((x1)), *ip++ = BigEndianValue((x2)),	\
                  *ip++ = BigEndianValue((x3)), *ip++ = BigEndianValue((x4)), TERM)

A1(PUBLIC, RgnHandle, ROMlib_circrgn, Rect *, r)		/* INTERNAL */
{
    RgnHandle rh;
    register INTEGER x, y, temp;    /* some variables need to be longs */
    INTEGER d, e, ny, oy, nx, ox, savex, rad;
    INTEGER scalex, scaley;

    INTEGER dh, dv;
    Size maxsize;
    INTEGER top, bottom, centl, centr, centt, left, right;
    INTEGER *ip, *op, *ip2, *ep;
    INTEGER first;
    long long_region_size; /* so we can test for overflow */

#if !defined (LETGCCWAIL)
    ox = 0;
#endif /* LETGCCWAIL */
            
    top    = BigEndianValue(r->top);
    bottom = BigEndianValue(r->bottom);
    dv = bottom - top;
    dh = (right = BigEndianValue(r->right)) - (left = BigEndianValue(r->left));

    maxsize = 10 + (6 * dv + 1) * sizeof(INTEGER);
    rh = (RgnHandle) NewHandle(maxsize);
    HxX (rh, rgnBBox) = *r;

    if (dh == dv && dh < 10) {	/* do small ones by hand */
	ip = (INTEGER *) STARH(rh) + 5;
	if (dh >= 4) {
	    if (dh < 7) {
		ADD4(top     , left+1, right-1);
		ADD6(top   +1, left  , left+1, right-1, right);
		ADD6(bottom-1, left  , left+1, right-1, right);
		ADD4(bottom  , left+1, right-1);
		TERM;
	    } else {
		ADD4(top     , left+2, right-2);
		ADD6(top   +1, left+1, left+2, right-2, right-1);
		ADD6(top   +2, left  , left+1, right-1, right  );
		ADD6(bottom-2, left  , left+1, right-1, right  );
		ADD6(bottom-1, left+1, left+2, right-2, right-1);
		ADD4(bottom  , left+2, right-2);
		TERM;
	    }
	}
	HxX(rh, rgnSize) = BigEndianValue((char *) ip - (char *) STARH(rh));
	SetHandleSize((Handle) rh, (Size) Hx(rh, rgnSize));
/*-->*/	return rh;
    }
    
    if (dh > dv) {
        scalex = FALSE;
        scaley = TRUE;
        rad = dh;
    } else if (dv > dh) {
        scalex = TRUE;
        scaley = FALSE;
        rad = dv;
    } else {
        scalex = FALSE;
        scaley = FALSE;
        rad = dv;
    }
    
    centl = BigEndianValue(r->left) + dh / 2;
    centr = BigEndianValue(r->left) + (dh + 1) / 2;
    centt = top + dv / 2;
    first = TRUE;
    
    op = (INTEGER *) STARH(rh) + 5;    
    x = 0;
    y = rad;
    oy = centt - top;
    d = 3 - 2 * rad;
    e = 3 - 4 * rad;
    while (y >= 0) {
        if (d < 0) {
            temp = 4 * x;
            d += temp + 6;
            e += temp + 4;
            x++;
        } else {
            savex = x;
            if (e > 0) {
                temp = 4 * y;
                d -= temp - 4;
                e += -temp + 6;
            } else {
                d += (temp = 4 * (x - y) + 10);
                e += temp;
                x++;
            }
            y--;
            if (scaley)
                ny = ((LONGINT) dv * y / dh) / 2;
            else
                ny = y / 2;
            if (ny != oy) {
                if (scalex)
                    nx = ((LONGINT) dh * savex / dv) / 2;
                else
                    nx = savex / 2;
                if (first) {
                    *op++ = BigEndianValue(top);
                    *op++ = BigEndianValue(centl - nx);
                    *op++ = BigEndianValue(centr + nx);
                    *op++ = RGNSTOPX;
                    ox = nx;
                    first = FALSE;
                } else {
                    if (nx != ox) {
                        *op++ = BigEndianValue(centt - oy);
                        *op++ = BigEndianValue(centl - nx);
                        *op++ = BigEndianValue(centl - ox);
                        *op++ = BigEndianValue(centr + ox);
                        *op++ = BigEndianValue(centr + nx);
                        *op++ = RGNSTOPX;
                        ox = nx;
                    }
                }
                oy = ny;
            }
        }
    }
    ip = op-1;
    ep = (INTEGER *) STARH(rh) + 4;
    while (ip != ep) {
        ip -= 4;
        while (ip != ep && *ip != RGNSTOPX)
            ip -= 2;
        ip2 = ip + 1;
        *op++ = BigEndianValue(bottom - (BigEndianValue(*ip2++) - top));
        while ((*op++ = *ip2++) != RGNSTOPX)
            ;
    }
    *op++ = RGNSTOPX;

    long_region_size = sizeof(INTEGER) * (op - (INTEGER *) STARH(rh));
    if (long_region_size >= 32768) /* test for overflow */
      SetEmptyRgn (rh);
    else
      {
	HxX(rh, rgnSize) = BigEndianValue(sizeof(INTEGER) * (op - (INTEGER *) STARH(rh)));
	SetHandleSize((Handle) rh, (Size) Hx(rh, rgnSize));
      }

    return rh;
}

/*
 * TODO:  speed up this code... it is ridiculously slow
 */

P4(PUBLIC pascal trap, void, StdRRect, GrafVerb, verb, Rect *, r,
					       INTEGER, width, INTEGER, height)
{
    RgnHandle rh, oval, corner, smallr;
    Rect tempr;
    INTEGER ovaldx, ovaldy, rectdx, rectdy;
    Point p;
    PAUSEDECL;

    PIC_SAVE_EXCURSION
      ({
	p.h = BigEndianValue (width);
	p.v = BigEndianValue (height);
	ROMlib_drawingverbrectovalpicupdate (verb, r, &p);
	PICOP (OP_frameRRect + (int) verb);
	PICWRITE (r, sizeof(*r));
      });

    if (PORT_PEN_VIS (thePort) < 0
	&& (!PORT_REGION_SAVE (thePort) || verb != frame))
/*-->*/ return;

    PAUSERECORDING;
    ovaldx = BigEndianValue(r->right)  - BigEndianValue(r->left) - width;
    ovaldy = BigEndianValue(r->bottom) - BigEndianValue(r->top)  - height;
    if (width < 4 && height < 4)
	StdRect(verb, r);
    else {
	rectdx = BigEndianValue(r->right)  - BigEndianValue(r->left) - width/2;
	rectdy = BigEndianValue(r->bottom) - BigEndianValue(r->top)  - height/2;
	    
	rh = NewRgn();
	corner = NewRgn();
	smallr = NewRgn();

	RectRgn(rh, r);
	
	SetRect(&tempr, BigEndianValue(r->left), BigEndianValue(r->top),
					 BigEndianValue(r->left)+width, BigEndianValue(r->top)+height);
	oval = ROMlib_circrgn(&tempr);

	SetRect(&tempr, BigEndianValue(r->left), BigEndianValue(r->top),
				     BigEndianValue(r->left)+width/2, BigEndianValue(r->top)+height/2);
	RectRgn(smallr, &tempr);
	
	DiffRgn(smallr, oval, corner);
	DiffRgn(rh, corner, rh);

	OffsetRgn(oval, ovaldx, 0);
	OffsetRgn(smallr, rectdx, 0);
	DiffRgn(smallr, oval, corner);
	DiffRgn(rh, corner, rh);
	
	OffsetRgn(oval, 0, ovaldy);
	OffsetRgn(smallr, 0, rectdy);
	DiffRgn(smallr, oval, corner);
	DiffRgn(rh, corner, rh);
	
	OffsetRgn(oval, -ovaldx, 0);
	OffsetRgn(smallr, -rectdx, 0);
	DiffRgn(smallr, oval, corner);
	DiffRgn(rh, corner, rh);
	
	StdRgn(verb, rh);
	
	DisposeRgn(smallr);
	DisposeRgn(corner);
	DisposeRgn(oval);
	DisposeRgn(rh);
    }
    RESUMERECORDING;
}
