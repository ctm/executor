/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qStdOval[] =
	    "$Id: qStdOval.c 63 2004-12-24 18:19:43Z ctm $";
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

P2(PUBLIC pascal trap, void, StdOval, GrafVerb, v, Rect *, rp)
{
    Rect r;
    RgnHandle rh, rh2;
    PAUSEDECL;
    
    if (!EmptyRect(rp)) {
      PIC_SAVE_EXCURSION
	({
	  ROMlib_drawingverbrectpicupdate (v, rp);
	  PICOP (OP_frameOval + (int) v);
	  PICWRITE (rp, sizeof (*rp));
	});

	PAUSERECORDING;
	if (BigEndianValue(rp->bottom) - BigEndianValue(rp->top) < 4 &&
					      BigEndianValue(rp->right) - BigEndianValue(rp->left) < 4)
	    StdRect(v, rp);
	else {
	    rh = ROMlib_circrgn(rp);
	    switch (v) {
	    case frame:
		if (PORT_REGION_SAVE_X (thePort))
		    XorRgn (rh,
			    (RgnHandle) PORT_REGION_SAVE (thePort),
			    (RgnHandle) PORT_REGION_SAVE (thePort));
		if (PORT_PEN_VIS (thePort) >= 0)
		  {
		    r.top    = BigEndianValue (BigEndianValue (rp->top)    + BigEndianValue (PORT_PEN_SIZE (thePort).v));
		    r.left   = BigEndianValue (BigEndianValue (rp->left)   + BigEndianValue (PORT_PEN_SIZE (thePort).h));
		    r.bottom = BigEndianValue (BigEndianValue (rp->bottom) - BigEndianValue (PORT_PEN_SIZE (thePort).v));
		    r.right  = BigEndianValue (BigEndianValue (rp->right)  - BigEndianValue (PORT_PEN_SIZE (thePort).h));
		    if (BigEndianValue (r.top) < BigEndianValue (r.bottom) && BigEndianValue (r.left) < BigEndianValue (r.right))
		      {
			rh2 = ROMlib_circrgn(&r);
			XorRgn(rh, rh2, rh);
			DisposeRgn(rh2);
		      }
		    StdRgn(paint, rh);
		  }
		break;
	    case paint:
	    case erase:
	    case invert:
	    case fill:
		StdRgn(v, rh);
	    }
	    DisposeRgn(rh);
	}
	RESUMERECORDING;
    }
}
