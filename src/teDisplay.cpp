/* Copyright 1986, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_teDisplay[] =
	    "$Id: teDisplay.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in TextEdit.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "WindowMgr.h"
#include "ControlMgr.h"
#include "TextEdit.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"
#include "rsys/tesave.h"
#include "rsys/region.h"

using namespace Executor;
using namespace ByteSwap;

P2 (PUBLIC pascal trap, void, TESetJust, INTEGER, j, TEHandle, teh)
{
  TE_SLAM (teh);
  HxX (teh, just) = BigEndianValue(j);
  TECalText (teh);
  TE_SLAM (teh);
}

P2 (PUBLIC pascal trap, void, TEUpdate, Rect *, r, TEHandle, te)
{
  TE_SLAM (te);
  if (!ROMlib_emptyvis)
    {
      TESAVE (te);
      TRAPBEGIN ();
      TE_DO_TEXT (te, 0, TE_LENGTH (te), teDraw);
      if (TE_ACTIVE (te) && TE_CARET_STATE (te) != 255)
	ROMlib_togglelite (te);
      TRAPEND ();
      TERESTORE ();
    }
  TE_SLAM (te);
}

P4 (PUBLIC pascal trap, void, TextBox, Ptr, p, int32, ln,
    Rect *, r, int16, j)
{
  TEHandle teh;
  Rect viewrect;

  /* #### should we do anything at all here if !ROMlib_emptyvis? */
  
  viewrect = *r;
  
#if 0
  /*
   * The following InsetRect has been in here for a long time, but it
   * was causing trouble with CIM 2.1.4, so there's good reason to believe
   * that it never should have been here.  -- ctm
   */
  InsetRect(&viewrect, -3, -3);
#endif
  
  teh = TENew(r, &viewrect);
  TESetText(p, ln, teh);
  TESetJust(j, teh);
  if (!ROMlib_emptyvis)
    EraseRect(r);
  TEUpdate(r, teh);
  TEDispose(teh);
}

P3 (PUBLIC pascal trap, void, TEScroll, int16, dh, int16, dv, TEHandle, te)
{
  RgnHandle rh, save_vis;
  Rect r, vis_rgn_bbox;
  
  TESAVE (te);
  TE_SLAM  (te);
  TRAPBEGIN ();
  rh = NewRgn ();
  
  r = TE_VIEW_RECT (te);
  
  ScrollRect (&r, dh, dv, rh);
  OffsetRect (&TE_DEST_RECT (te), dh, dv);
  
  save_vis = PORT_VIS_REGION (thePort);
  SectRgn (rh, save_vis, rh);
  PORT_VIS_REGION_X (thePort) = RM (rh);
  
  vis_rgn_bbox = RGN_BBOX (PORT_VIS_REGION (thePort));
  TEUpdate (&vis_rgn_bbox, te);
  
  PORT_VIS_REGION_X (thePort) = RM (save_vis);
  DisposeRgn (rh);
  
  TRAPEND ();
  TE_SLAM (te);
  TERESTORE ();
}
