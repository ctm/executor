/* Copyright 1986, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_windMouse[] =
	    "$Id: windMouse.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in WindowMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "WindowMgr.h"
#include "EventMgr.h"
#include "OSEvent.h"
#include "MemoryMgr.h"
#include "MenuMgr.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"
#include "rsys/menu.h"

using namespace Executor;

#if !defined (No_STEF_zoommods)
/* WINDOW_ZOOMED returns TRUE if w is currently in stdState (big) */

#define WINDOW_ZOOMED(w) (ROMlib_window_zoomed(w))

#endif

P2(PUBLIC pascal trap, INTEGER, FindWindow, Point, p, GUEST<WindowPtr> *, wpp)
{
    WindowPeek wp;
    LONGINT pointaslong, val;
    INTEGER retval;

    pointaslong = ((LONGINT)p.v << 16)|(unsigned short)p.h;

    *wpp = 0;
    if (MBDFCALL(mbHit, 0, pointaslong) != -1)
      return inMenuBar;
    for (wp = MR (WindowList); wp ; wp = WINDOW_NEXT_WINDOW (wp))
      {
	if (WINDOW_VISIBLE_X (wp) && PtInRgn (p, WINDOW_STRUCT_REGION (wp)))
	  {
	    *wpp = RM ((WindowPtr) wp);
	    if (WINDOW_KIND (wp) < 0)
	      {
		retval = inSysWindow;
		goto DONE;
	      }
	    val = WINDCALL((WindowPtr) wp, wHit, pointaslong);
	    if (val == wNoHit)
	      retval = DeskHook ? inSysWindow : inDesk;
	    else
	      retval = val + 2; /* datadesk showed us that this is how it's
				   done */
	    goto DONE;
	  }
      }
    retval = inDesk;
DONE:
    return retval;
}

namespace Executor {
  PRIVATE BOOLEAN xTrackBox(WindowPtr wp, Point pt,
							INTEGER part);

}

A3(PRIVATE, BOOLEAN, xTrackBox, WindowPtr, wp, Point, pt,
						INTEGER, part)	/* IMIV-50 */
{
  BOOLEAN inpart = TRUE, inp;
  EventRecord ev;

  THEPORT_SAVE_EXCURSION
    (MR (wmgr_port),
     {
       SetClip(MR(GrayRgn));

       WINDCALL(wp, wDraw, part);
       while (!GetOSEvent(mUpMask, &ev))
	 {
           Point evwhere = ev.where.get();
	   CALLDRAGHOOK();
	   if (pt.h != evwhere.h || pt.v != evwhere.v)
	     {
	       pt.h = evwhere.h;
	       pt.v = evwhere.v;
	       inp =
		 (WINDCALL
		  (wp, wHit, ((LONGINT)pt.v << 16)|(unsigned short) pt.h) == part);
            if (inpart != inp)
	      {
                WINDCALL(wp, wDraw, part);
                inpart = inp;
	      }
	     }
	 }
     });
  return inpart;
}

P3(PUBLIC pascal trap, BOOLEAN, TrackBox, WindowPtr, wp,	/* IMIV-50 */
						    Point, pt, INTEGER, part)
{
  if (part)
    part -= 2;
  return xTrackBox(wp, pt, part);
}

P2(PUBLIC pascal trap, BOOLEAN, TrackGoAway, WindowPtr, w, Point, p)
{
    return xTrackBox(w, p, wInGoAway);
}

P3(PUBLIC pascal trap, void, ZoomWindow, WindowPtr, wp,		/* IMIV-50 */
					       INTEGER, part, BOOLEAN, front)
{
  RgnHandle behind;
#if !defined (No_STEF_zoommods)
  Boolean instdstate;
  Rect *u;
#if !defined (THEPORTNEEDNTBEWMGRPORT)
  GrafPtr gp;
#endif /* THEPORTNEEDNTBEWMGRPORT */
  
  instdstate = WINDOW_ZOOMED((WindowPeek) wp);
  
  if ((part == inZoomIn && instdstate)
      || (part == inZoomOut && !instdstate))
#else /* No_STEF_zoommods */
    
  if ((part == inZoomIn && WINDOW_SPARE_FLAG (wp) == inZoomOut)
      || (part == inZoomOut && (WINDOW_SPARE_FLAG (wp) == inZoomIn)))
#endif /* No_STEF_zoommods */
    {
#if !defined (No_STEF_zoommods)
      /* Save userState if not in stdstate */
      if (!instdstate)
	{
	  u = &((WStateData *) STARH (WINDOW_DATA (wp)))->userState;
	  u->top
	    = CW (CW (PORT_RECT (wp).top) - CW (PORT_BOUNDS (wp).top));
	  u->left
	    = CW (CW (PORT_RECT (wp).left) - CW (PORT_BOUNDS (wp).left));
	  u->bottom
	    = CW (CW (PORT_RECT (wp).bottom) - CW(PORT_BOUNDS (wp).top));
	  u->right
	    = CW (CW (PORT_RECT (wp).right) - CW (PORT_BOUNDS (wp).left));
	}
#endif
      behind = NewRgn();
      CopyRgn (WINDOW_STRUCT_REGION (wp), behind);
      if (part == inZoomIn)
	PORT_RECT (wp) =
	  MR(*(GUEST<WStateData *>*) WINDOW_DATA (wp))->userState;
      else
	PORT_RECT (wp) =
	  MR (*(GUEST<WStateData *>*) WINDOW_DATA (wp))->stdState;
      OffsetRect (&PORT_BOUNDS (wp),
		  -CW(PORT_RECT (wp).left) - CW(PORT_BOUNDS (wp).left),
		  -CW(PORT_RECT (wp).top)  - CW(PORT_BOUNDS (wp).top));
      
      OffsetRect (&PORT_RECT (wp),
		  -CW (PORT_RECT (wp).left), -CW (PORT_RECT (wp).top));
      WINDCALL(wp, wCalcRgns, 0);
      UnionRgn(behind, WINDOW_STRUCT_REGION (wp), behind);
      
      CalcVisBehind((WindowPeek) wp, behind);
      PaintBehind (WINDOW_NEXT_WINDOW (wp), behind);
      
#if !defined (THEPORTNEEDNTBEWMGRPORT)
      gp = thePort;
      SetPort (MR (wmgr_port));
#endif /* THEPORTNEEDNTBEWMGRPORT */
      SetClip (WINDOW_STRUCT_REGION (wp));
      ClipAbove((WindowPeek)wp);
      WINDCALL((WindowPtr) wp, wDraw, 0);
      EraseRgn (WINDOW_CONT_REGION (wp));
      CopyRgn (WINDOW_CONT_REGION (wp), WINDOW_UPDATE_REGION (wp));
#if !defined (THEPORTNEEDNTBEWMGRPORT)
      SetPort(gp);
#endif /* THEPORTNEEDNTBEWMGRPORT */
      
#if !defined (No_STEF_zoommods)
#else
      WINDOW_SPARE_FLAG_X (wp) = part;
#endif /* No_STEF_zoommods */
      DisposeRgn (behind);
      if (front)
	SelectWindow(wp);
    }
}
