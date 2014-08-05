/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qStdLine[] =
	    "$Id: qStdLine.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/quick.h"
#include "rsys/cquick.h"
#include "rsys/picture.h"

using namespace Executor;
using namespace ByteSwap;

/*
 * eight scan convert routines... all very similar.
 * sco:  do the scan convert and build two tables (using offsets)
 * scr:  do the scan convert but built rgn style output
 * dydx: dy is greater than or equal to dx.
 * x1x2: x1 is greater than x2.
 */

#define OUT(y, x) (*op2++ = (*op++ = (y)) + offy, \
		   *op2++ = (*op++ = (x)) + offx)

namespace Executor {
  PRIVATE void scodydxx1x2(register LONGINT y1, register INTEGER x1,
						   INTEGER dy, INTEGER dx, INTEGER ** opp, INTEGER ** opp2,
						   register INTEGER offy, register INTEGER offx);
  PRIVATE INTEGER *scrdydxx1x2(register LONGINT y1, register INTEGER x1,
							   INTEGER dy, INTEGER dx, register INTEGER * op);
  PRIVATE void scodydxx2x1(register LONGINT y1, register INTEGER x1,
						   INTEGER dy, INTEGER dx, INTEGER ** opp, INTEGER ** opp2,
						   register INTEGER offy, register INTEGER offx);
  PRIVATE INTEGER *scrdydxx2x1(register LONGINT y1, register INTEGER x1,
							   INTEGER dy, INTEGER dx, register INTEGER * op);
  PRIVATE void scodxdyx1x2(register INTEGER y1, register LONGINT x1,
						   register INTEGER dy, register INTEGER dx, INTEGER ** opp,
						   INTEGER ** opp2, register INTEGER offy, register INTEGER offx);
  PRIVATE INTEGER *scrdxdyx1x2(register INTEGER y1, register LONGINT x1,
							   register INTEGER dy, register INTEGER dx, register INTEGER * op);
  PRIVATE void scodxdyx2x1(register INTEGER y1, register LONGINT x1,
						   register INTEGER dy, register INTEGER dx, INTEGER ** opp,
						   INTEGER ** opp2, register INTEGER offy, register INTEGER offx);
  PRIVATE INTEGER *scrdxdyx2x1(register INTEGER y1, register LONGINT x1,
							   register INTEGER dy, register INTEGER dx, register INTEGER *op);
  PRIVATE void regionify1(register INTEGER * ip1,
						  register INTEGER * ip2, RgnPtr rp);
}

A8(PRIVATE, void, scodydxx1x2, register LONGINT, y1, register INTEGER, x1,
	    INTEGER, dy, INTEGER, dx, INTEGER **, opp, INTEGER **, opp2,
				register INTEGER, offy, register INTEGER, offx)
{
    register INTEGER *op, *op2;
    register INTEGER x2;
    register LONGINT incr;

    op  = *opp;
    op2 = *opp2;
    x2 = x1 - dx;
    if (dy > dx) {
	incr = ((LONGINT) dy << 16) / (dx+1) + 1;
	OUT(y1, x1);
	y1 = (y1 << 16) | (1L << 15);
	while (x1 != x2) {
	    y1 += incr;
	    OUT(y1 >> 16, --x1);
	}
    } else {
	OUT(y1, x1);
	while (x1 != x2)
	    OUT(++y1, --x1);
    }
    *opp  = op;
    *opp2 = op2;
}

#define OUT2(y, x1, x2)	(*op++ = BigEndianValue(y),		\
			 *op++ = BigEndianValue(x1),	\
			 *op++ = BigEndianValue(x2),	\
			 *op++ = RGNSTOPX)

A5(PRIVATE, INTEGER *, scrdydxx1x2, register LONGINT, y1, register INTEGER, x1,
			    INTEGER, dy, INTEGER, dx, register INTEGER *, op)
{
    register INTEGER x2;
    register LONGINT incr;

    x2 = x1 - dx;
    if (dy > dx) {
	incr = ((LONGINT) dy << 16) / (dx+1) + 1;
	y1 = (y1 << 16) | (1L << 15);
	while (x1 != x2) {
	    y1 += incr;
	    OUT2(y1 >> 16, x1-1, x1);
	    --x1;
	}
    } else {
	while (x1 != x2) {
	    OUT2(++y1, x1-1, x1);
	    --x1;
	}
    }
    return op;
}

A8(PRIVATE, void, scodydxx2x1, register LONGINT, y1, register INTEGER, x1,
	    INTEGER, dy, INTEGER, dx, INTEGER **, opp, INTEGER **, opp2,
				register INTEGER, offy, register INTEGER, offx)
{
    register INTEGER x2;
    register LONGINT incr;
    register INTEGER *op, *op2;

    op  = *opp;
    op2 = *opp2;
    x2 = x1 + dx;
    if (dy > dx) {
	incr = ((LONGINT) dy << 16) / (dx+1) + 1;
	OUT(y1, x1);
	y1 = (y1 << 16) | (1L << 15);
	while (x1 != x2) {
	    y1 += incr;
	    OUT(y1 >> 16, ++x1);
	}
    } else {
	OUT(y1, x1);
	while (x1 != x2)
	    OUT(++y1, ++x1);
    }
    *opp  = op;
    *opp2 = op2;
}

A5(PRIVATE, INTEGER *, scrdydxx2x1, register LONGINT, y1, register INTEGER, x1,
			    INTEGER, dy, INTEGER, dx, register INTEGER *, op)
{
    register INTEGER x2;
    register LONGINT incr;

    x2 = x1 + dx;
    if (dy > dx) {
	incr = ((LONGINT) dy << 16) / (dx+1) + 1;
	y1 = (y1 << 16) | (1L << 15);
	while (x1 != x2) {
	    y1 += incr;
	    OUT2(y1 >> 16, x1, ++x1);	/* order of ++ is defined because */
	}				/* OUT2 is a macro */
    } else {
	while (x1 != x2)
	    OUT2(++y1, x1, ++x1);
    }
    return op;
}

A8(PRIVATE, void, scodxdyx1x2, register INTEGER, y1, register LONGINT, x1,
	    register INTEGER, dy, register INTEGER, dx, INTEGER **, opp,
	    INTEGER **, opp2, register INTEGER, offy, register INTEGER, offx)
{
    register INTEGER y2;
    register LONGINT incr;
    register INTEGER *op, *op2;

    op  = *opp;
    op2 = *opp2;
    y2 = y1 + dy;
    incr = ((LONGINT) dx << 16) / (dy+1) + 1;
    x1 = ((x1 << 16) | (1L << 15)) -1;
    while (y1 <= y2) {
	x1 -= incr;
	OUT(y1++, x1 >> 16);
    }
    *opp = op;
    *opp2 = op2;
}

A5(PRIVATE, INTEGER *, scrdxdyx1x2, register INTEGER, y1, register LONGINT, x1,
	    register INTEGER, dy, register INTEGER, dx, register INTEGER *, op)
{
    register INTEGER y2, ox, x2;
    register LONGINT incr;

    x2 = x1 - dx;
    y2 = y1 + dy;
    incr = ((LONGINT) dx << 16) / (dy+1) + 1;
    ox = x1-1;
    x1 = ((x1 << 16) | (1L << 15)) -1;
    x1 -= incr;
    if ((x1 >> 16) <  ox) {
	OUT2(y1++, x1 >> 16, ox);
	ox = x1 >> 16;
    } else
	y1++;
    while (y1 <= y2) {
	x1 -= incr;
	OUT2(y1++, x1 >> 16, ox);
	ox = x1 >> 16;
    }
    op[-3] = BigEndianValue(x2-1);
    return op;
}

A8(PRIVATE, void, scodxdyx2x1, register INTEGER, y1, register LONGINT, x1,
	    register INTEGER, dy, register INTEGER, dx, INTEGER **, opp,
	    INTEGER **, opp2, register INTEGER, offy, register INTEGER, offx)
{
    register INTEGER y2;
    register LONGINT incr;
    register INTEGER *op, *op2;

    op  = *opp;
    op2 = *opp2;
    y2 = y1 + dy;
    incr = ((LONGINT) dx << 16) / (dy+1) + 1;
    x1 = (x1 << 16) | (1L << 15);
    while (y1 <= y2) {
	x1 += incr;
	OUT(y1++, x1 >> 16);
    }
    *opp = op;
    *opp2 = op2;
}

A5(PRIVATE, INTEGER *, scrdxdyx2x1, register INTEGER, y1, register LONGINT, x1,
	    register INTEGER, dy, register INTEGER, dx, register INTEGER *, op)
{
    register INTEGER y2, ox;
    register LONGINT incr;

    y2 = y1 + dy;
    incr = ((LONGINT) dx << 16) / (dy+1) + 1;
    ox = x1;
    x1 = (x1 << 16) | (1L << 15);
    while (y1 <= y2) {
	x1 += incr;
	OUT2(y1++, ox, ox = x1 >> 16);
    }
    return op;
}

/*
 * NOTE: regionify1 creates a "special" region that has two properties
 *	 to bear in mind:  1) it's not really a region, it's start/stop
 *	 pairs.  2)  the start stop pairs are kept in native endianness.
 */

A3(PRIVATE, void, regionify1, register INTEGER *, ip1,
					   register INTEGER *, ip2, RgnPtr, rp)
{
    INTEGER *tempp;
    INTEGER *op;
    INTEGER x1, x2, y1, y2;

    if (ip1[1] > ip2[1]) {
	tempp = ip1;
	ip1   = ip2;
	ip2   = tempp;
    }
    op = (INTEGER *) rp + 5;
    y1 = *ip1++;
    y2 = *ip2++;
    x1 = *ip1;
    x2 = *ip2;
    gui_assert(y1 == y2);
    for (;;) {
	if (y1 < y2) {
	    x1 = *ip1++;
	    gui_assert(x1 < x2 || (x1 == 32767 && x2 == 32767));
	    *op++ = BigEndianValue(y1);
	    *op++ = x1;
	    if (x1 == 32767)
/*-->*/		break;
	    *op++ = x2;
	    *op++ = 32767;
	    y1 = *ip1++;
	} else if (y1 > y2) {
	    x2 = *ip2++;
	    gui_assert(x1 < x2 || (x1 == 32767 && x2 == 32767));
	    *op++ = BigEndianValue(y2);
	    *op++ = x1;
	    if (x1 == 32767)
/*-->*/		break;
	    *op++ = x2;
	    *op++ = 32767;
	    y2 = *ip2++;
	} else {
	    if (y1 == 32767)
/*-->*/		break;
	    x1 = *ip1++;
	    x2 = *ip2++;
	    gui_assert(x1 < x2 || (x1 == 32767 && x2 == 32767));
	    *op++ = BigEndianValue(y1);
	    *op++ = x1;
	    if (x1 == 32767)
/*-->*/		break;
	    *op++ = x2;
	    *op++ = 32767;
	    y1 = *ip1++;
	    y2 = *ip2++;
	}
    }
    *op++ = CWC(32767);
    rp->rgnSize = BigEndianValue(-32768 + (op - (INTEGER *) rp) * sizeof(INTEGER));
}

#define SWAP std::swap
#define MAXNPOINTS(dy) ((dy+3)*2 + 1)

P1(PUBLIC pascal trap, void, StdLine, Point, p)
{
  INTEGER x1, x2, y1, y2, px, py, dx, dy;
  INTEGER *oip;
  INTEGER *op, *op2, *destpoints, *destpoints2;
  Size psize;
  HIDDEN_RgnPtr rp;
  Rect r;
  register INTEGER r32767;
  RgnHandle rh;
  PolyHandle ph;
  Point swappedp;
  ALLOCABEGIN
  PAUSEDECL;
  
  r32767 = 32767;
  x1 = BigEndianValue (PORT_PEN_LOC (thePort).h);
  y1 = BigEndianValue (PORT_PEN_LOC (thePort).v);
  x2 = p.h;
  y2 = p.v;
  
  px = BigEndianValue (PORT_PEN_SIZE (thePort).h);
  py = BigEndianValue (PORT_PEN_SIZE (thePort).v);
  
  if (PORT_POLY_SAVE_X (thePort) && (x1 != x2 || y1 != y2)) {
	ph = (PolyHandle) PORT_POLY_SAVE (thePort);
	psize = GetHandleSize((Handle) ph);
	if (psize == SMALLPOLY) {
	  SetHandleSize((Handle) ph, psize + 2 * sizeof(Point));
	  oip = (INTEGER *)((char *) STARH(ph) + psize);
	  *oip++ = BigEndianValue(y1);
	  *oip++ = BigEndianValue(x1);
	  HxX(ph, polySize) = BigEndianValue(Hx(ph, polySize) + 2 * sizeof(Point));
	} else {
	  SetHandleSize((Handle) ph, psize + sizeof(Point));
	  oip = (INTEGER *)((char *) STARH(ph) + psize);
	  HxX(ph, polySize) = BigEndianValue(Hx(ph, polySize) + sizeof(Point));
	}
	*oip++ = BigEndianValue(y2);
	*oip++ = BigEndianValue(x2);
  }
  
  PIC_SAVE_EXCURSION
  ({
	ROMlib_drawingpicupdate();
	PICOP(OP_Line);
	PICWRITE(&PORT_PEN_LOC (thePort), sizeof (PORT_PEN_LOC (thePort)));
	swappedp.h = BigEndianValue(p.h);
	swappedp.v = BigEndianValue(p.v);
	PICWRITE(&swappedp, sizeof(swappedp));
  })
  
  /*
   * NOTE: this early return used to be before we wrote the picture, but
   *	 that messes up people who embed postscript in pictures and use
   *	 StdLine to figure out where they are...
   */
  if (px == 0 || py == 0)
  /*-->*/ return;
  
  if (x1 == x2 || y1 == y2) {
	/* rectangle */
	if (x2 < x1)
	  SWAP(x1, x2);
	if (y2 < y1)
	  SWAP(y1, y2);
	if (PORT_PEN_VIS (thePort) >= 0) {
	  SetRect(&r, x1, y1, x2 + px, y2 + py);
	  PAUSERECORDING;
	  StdRect(paint, &r);
	  RESUMERECORDING;
	}
	if (PORT_REGION_SAVE_X (thePort) && y1 == y2 && x1 != x2) {
	  rp.p = (RgnPtr) ALLOCA(SMALLRGN + 5 * sizeof(INTEGER));
	  (rp.p)->rgnBBox.top    = BigEndianValue(y1);
	  (rp.p)->rgnBBox.left   = BigEndianValue(x1);
	  (rp.p)->rgnBBox.bottom = BigEndianValue(y2);
	  (rp.p)->rgnBBox.right  = BigEndianValue(x1);
	  (rp.p)->rgnSize = CWC(SMALLRGN + 5 * sizeof(INTEGER));
	  oip = (INTEGER *) ((char *)rp.p + SMALLRGN);
	  *oip++ = BigEndianValue(y1);
	  *oip++ = BigEndianValue(x1);
	  *oip++ = BigEndianValue(x2);
	  *oip++ = RGNSTOPX;
	  *oip++ = RGNSTOPX;
	  rp.p = RM(rp.p);
	  XorRgn (&rp,
			  (RgnHandle) PORT_REGION_SAVE (thePort),
			  (RgnHandle) PORT_REGION_SAVE (thePort));
	}
	ALLOCAEND
	/*-->*/	return;
  } else if (y1 > y2) {
	SWAP(y1, y2);
	SWAP(x1, x2);
  }
  
  dy = y2 - y1;
  dx = ABS(x2 - x1);
  
  if (PORT_REGION_SAVE_X (thePort)) {
	/* size allocated below is overkill */
	rp.p = (RgnPtr) ALLOCA(SMALLRGN + (dy + 1) * sizeof(INTEGER) * 6 +
						   sizeof(INTEGER));
	(rp.p)->rgnBBox.top    = BigEndianValue(y1);
	(rp.p)->rgnBBox.left   = BigEndianValue(MIN(x1, x2));
	(rp.p)->rgnBBox.bottom = BigEndianValue(y2);
	(rp.p)->rgnBBox.right  = BigEndianValue(MAX(x1, x2));
	
	if (dy >= dx)
	  if (x2 > x1)
		op = scrdydxx2x1(y1, x1,   dy, dx, (INTEGER *)rp.p + 5);
	  else
		op = scrdydxx1x2(y1, x1,   dy, dx, (INTEGER *)rp.p + 5);
	  else
		if (x2 > x1)
		  op = scrdxdyx2x1(y1, x1,   dy, dx, (INTEGER *)rp.p + 5);
		else
		  op = scrdxdyx1x2(y1, x1+1, dy, dx, (INTEGER *)rp.p + 5);
	*op++ = RGNSTOPX;
	(rp.p)->rgnSize = BigEndianValue((char *) op - (char *) rp.p);
	rp.p = RM(rp.p);
	XorRgn (&rp,
			(RgnHandle) PORT_REGION_SAVE (thePort),
			(RgnHandle) PORT_REGION_SAVE (thePort));
  }
  
  if (PORT_PEN_VIS (thePort) < 0) {
	ALLOCAEND
	/*-->*/	return;
  }
  
  rp.p = (RgnPtr) ALLOCA(SMALLRGN + (dy + py + 1) * sizeof(LONGINT) * 4 +
						 3 * 2 * sizeof(LONGINT));
  /* Cx(rp->rgnSize) gets filled in later */
  (rp.p)->rgnBBox.top    = BigEndianValue(y1);
  (rp.p)->rgnBBox.left   = BigEndianValue(MIN(x1, x2));
  (rp.p)->rgnBBox.bottom = BigEndianValue(y2 + py);
  (rp.p)->rgnBBox.right  = BigEndianValue(MAX(x1, x2) + px);
  op  = destpoints   = (INTEGER *) ALLOCA(MAXNPOINTS(dy) * sizeof(INTEGER));
  op2 = destpoints2  = (INTEGER *) ALLOCA(MAXNPOINTS(dy) * sizeof(INTEGER));
  
  if (dy >= dx) {
	if (x2 > x1) {
	  if (py > 1) {
		*op2++ = y1;
		*op2++ = x1;
	  }
	  scodydxx2x1(y1, x1+px, dy, dx, &op, &op2, py-1, -px);
	} else {
	  if (py > 1) {
		*op2++ = y1;
		*op2++ = x1+px;
	  }
	  scodydxx1x2(y1, x1, dy, dx, &op, &op2, py-1, px);
	}
	
	*op++  = y2+py;
	*op++  = r32767;
	
	*op2++ = y2+py;
	*op2++ = r32767;
	
	*op    = r32767;
	*op2   = r32767;
	
  } else {
	if (x2 > x1) {
	  *op2++  = y1;
	  *op2++  = x1;
	  
	  scodxdyx2x1(y1, x1+px-1, dy, dx, &op, &op2, py, -(px-1));
	  
	  op[-1]  = x2+px;
	  
	  *op++   = y2 + py;
	  *op++   = r32767;
	  
	  op2[-1] = r32767;
	  
	  *op     = r32767;
	  *op2    = r32767;
	} else {
	  *op2++  = y1;
	  *op2++  = x1+px;
	  
	  scodxdyx1x2(y1, x1+1, dy, dx, &op, &op2, py, px-1);
	  
	  op[-1]  = x2;
	  
	  *op++   = y2 + py;
	  *op++   = r32767;
	  
	  op2[-1] = r32767;
	  
	  *op     = r32767;
	  *op2    = r32767;
	}
  }
  gui_assert((op  - destpoints  + 1)  <=  ((dy+3)*2 + 1));
  gui_assert((op2 - destpoints2 + 1)  <=  ((dy+3)*2 + 1));
  regionify1(destpoints, destpoints2, rp.p);
  
  rh = NewRgn ();
  SectRect (&PORT_BOUNDS (thePort), &PORT_RECT (thePort), &r);
  RectRgn (rh, &r);
  SectRgn (rh, PORT_VIS_REGION (thePort),  rh);
  SectRgn (rh, PORT_CLIP_REGION (thePort), rh);
  rp.p = RM (rp.p);
  SectRgn (&rp, rh, rh);
  
  if (GWorld_p (thePort))
	LockPixels (CPORT_PIXMAP (thePort));
  
  {
	INTEGER adjusted_mode;
	
	adjusted_mode = PORT_PEN_MODE (thePort);
	if (adjusted_mode < blend)
	  adjusted_mode = adjusted_mode % 0x40 | 8;
	ROMlib_blt_pn (rh, adjusted_mode);
  }
  /* ROMlib_bltrgn (rh, thePort->pnPat, Cx(thePort->pnMode) % 0x40 | 8,
   (Rect *) 0, (Rect *) 0); */
  
  if (GWorld_p (thePort))
	UnlockPixels (CPORT_PIXMAP (thePort));
  
  DisposeRgn (rh);
  ALLOCAEND
}
