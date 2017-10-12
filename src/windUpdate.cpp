/* Copyright 1986, 1989, 1990, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_windUpdate[] =
	"$Id: windUpdate.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in WindowMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "WindowMgr.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"

using namespace Executor;

P1(PUBLIC pascal trap, void, InvalRect, Rect *, r)
{
  if (thePort)
    {
      RgnHandle rh;
      Rect r_copy;

      r_copy = *r;  /* Just in case NewRgn moves memory */
      rh = NewRgn();
      RectRgn(rh, &r_copy);
      OffsetRgn(rh, -CW (PORT_BOUNDS (thePort).left),
		-CW (PORT_BOUNDS (thePort).top));
      UnionRgn(rh, WINDOW_UPDATE_REGION (thePort),
	       WINDOW_UPDATE_REGION (thePort));
      DisposeRgn(rh);
    }
}

P1 (PUBLIC pascal trap, void, InvalRgn, RgnHandle, r)
{
  GrafPtr current_port;
  RgnHandle update_rgn;
  int top, left;
  
  current_port = thePort;
  top  = CW (PORT_BOUNDS (current_port).top);
  left = CW (PORT_BOUNDS (current_port).left);
  
  OffsetRgn (r, -left, -top);
  
  update_rgn = WINDOW_UPDATE_REGION (current_port);
  UnionRgn (r, update_rgn, update_rgn);
  
  OffsetRgn (r, left, top);
}

P1(PUBLIC pascal trap, void, ValidRect, Rect *, r)
{
    RgnHandle rh;

    rh = NewRgn();
    RectRgn(rh, r);
    OffsetRgn(rh, -CW (PORT_BOUNDS (thePort).left),
                  -CW (PORT_BOUNDS (thePort).top));
    DiffRgn (WINDOW_UPDATE_REGION (thePort), rh,
	     WINDOW_UPDATE_REGION (thePort));
    DisposeRgn(rh);
}

P1(PUBLIC pascal trap, void, ValidRgn, RgnHandle, r)
{
    OffsetRgn(r, -CW (PORT_BOUNDS (thePort).left),
                 -CW (PORT_BOUNDS (thePort).top));
    DiffRgn (WINDOW_UPDATE_REGION (thePort), r,
	     WINDOW_UPDATE_REGION (thePort));
    OffsetRgn (r, CW (PORT_BOUNDS (thePort).left),
                  CW (PORT_BOUNDS (thePort).top));
}

PUBLIC int Executor::ROMlib_emptyvis = 0;

P1(PUBLIC pascal trap, void, BeginUpdate, WindowPtr, w)
{
/* #warning Should SaveVisRgn ever become 0? */
  if (!SaveVisRgn)
    SaveVisRgn = (RgnHandle) RM(NewHandle(0));
  
  CopyRgn(PORT_VIS_REGION (w), MR(SaveVisRgn));
  
  if (EmptyRgn (WINDOW_UPDATE_REGION (w)))
    ROMlib_emptyvis = 1;
#if 0
  else /* our ROMlib_emptyvis hack below doesn't work if there are some
	  routines that write to the screen and ignore it */
#endif
    {
      CopyRgn (WINDOW_UPDATE_REGION (w), PORT_VIS_REGION (w));
      OffsetRgn (PORT_VIS_REGION (w),
		 CW (PORT_BOUNDS (w).left),
		 CW (PORT_BOUNDS (w).top));
      SectRgn (PORT_VIS_REGION (w), MR (SaveVisRgn), PORT_VIS_REGION (w));
      SetEmptyRgn (WINDOW_UPDATE_REGION (w));
      ROMlib_emptyvis = EmptyRgn (PORT_VIS_REGION (w));
    }
}

P1(PUBLIC pascal trap, void, EndUpdate, WindowPtr, w)
{
  CopyRgn(MR(SaveVisRgn), PORT_VIS_REGION (w));
  CopyRgn (WINDOW_CONT_REGION (w), MR (SaveVisRgn));
  OffsetRgn (MR (SaveVisRgn),
	     CW (PORT_BOUNDS (w).left),
	     CW (PORT_BOUNDS (w).top));
  ROMlib_emptyvis = 0;
}
