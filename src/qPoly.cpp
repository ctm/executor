/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qPoly[] =
	    "$Id: qPoly.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

/* HLock checked by ctm on Mon May 13 17:55:01 MDT 1991 */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/quick.h"
#include "rsys/cquick.h"

using namespace Executor;
using namespace ByteSwap;

P0(PUBLIC pascal trap, PolyHandle, OpenPoly)
{
    PolyHandle ph;
    
    ph = (PolyHandle) NewHandle ((Size) SMALLPOLY);
    HxX(ph, polySize) = CWC (SMALLPOLY);
    PORT_POLY_SAVE_X (thePort) = RM ((Handle) ph);
    HidePen();
    return(ph);
}

P0(PUBLIC pascal trap, void, ClosePoly)
{
    INTEGER top = 32767, left = 32767, bottom = -32767, right = -32767;
    INTEGER *ip, *ep, i;
    PolyHandle ph;
    
    ph = (PolyHandle) PORT_POLY_SAVE (thePort);
    for (ip = (INTEGER *)((char *)STARH(ph) + SMALLPOLY),
         ep = (INTEGER *)((char *)STARH(ph) + Hx(ph, polySize));
         ip != ep;) {
        if ((i = BigEndianValue(*ip)) <= top)
            top = i;
	++ip;
        if (i >= bottom)
            bottom = i;
        if ((i = BigEndianValue(*ip)) <= left)
            left = i;
	++ip;
        if (i >= right)
            right = i;
    }
    HxX(ph, polyBBox.top)    = BigEndianValue(top);
    HxX(ph, polyBBox.left)   = BigEndianValue(left);
    HxX(ph, polyBBox.bottom) = BigEndianValue(bottom);
    HxX(ph, polyBBox.right)  = BigEndianValue(right);
    PORT_POLY_SAVE_X (thePort) = (Handle)CWC (0);
    ShowPen();
}

P1(PUBLIC pascal trap, void, KillPoly, PolyHandle, poly)
{
    DisposHandle((Handle) poly);
}

P3(PUBLIC pascal trap, void, OffsetPoly, PolyHandle, poly,
		    INTEGER, dh, INTEGER, dv)   /* Note: IM I-191 is wrong */
{
    Point *pp, *ep;
    
    if (dh || dv) {
	HxX(poly, polyBBox.top)    = BigEndianValue(Hx(poly, polyBBox.top)    + dv);
	HxX(poly, polyBBox.bottom) = BigEndianValue(Hx(poly, polyBBox.bottom) + dv);
	HxX(poly, polyBBox.left)   = BigEndianValue(Hx(poly, polyBBox.left)   + dh);
	HxX(poly, polyBBox.right)  = BigEndianValue(Hx(poly, polyBBox.right)  + dh);
	pp = HxX(poly, polyPoints);
	ep = (Point *) (((char *) STARH(poly)) + Hx(poly, polySize));
	while (pp != ep) {
	    pp->h = BigEndianValue(BigEndianValue(pp->h) + (dh));
	    pp->v = BigEndianValue(BigEndianValue(pp->v) + (dv));
	    pp++;
	}
    }
}
