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

P2(PUBLIC pascal trap, void, StdOval, GrafVerb, v, Rect *, rp)
{
    Rect r;
    RgnHandle rh, rh2;
    PAUSEDECL;
    
    if (!EmptyRect(rp)) {
      if (thePort->picSave)
      {
	  ROMlib_drawingverbrectpicupdate (v, rp);
	  PICOP (OP_frameOval + (int) v);
	  PICWRITE (rp, sizeof (*rp));
      };

	PAUSERECORDING;
	if (CW(rp->bottom) - CW(rp->top) < 4 &&
					      CW(rp->right) - CW(rp->left) < 4)
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
		    r.top    = CW (CW (rp->top)    + CW (PORT_PEN_SIZE (thePort).v));
		    r.left   = CW (CW (rp->left)   + CW (PORT_PEN_SIZE (thePort).h));
		    r.bottom = CW (CW (rp->bottom) - CW (PORT_PEN_SIZE (thePort).v));
		    r.right  = CW (CW (rp->right)  - CW (PORT_PEN_SIZE (thePort).h));
		    if (CW (r.top) < CW (r.bottom) && CW (r.left) < CW (r.right))
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
