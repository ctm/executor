/* Copyright 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_redrawscreen[] =
"$Id: redrawscreen.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "SysErr.h"
#include "WindowMgr.h"
#include "MenuMgr.h"
#include "rsys/redrawscreen.h"
#include "rsys/cquick.h"
#include "rsys/vdriver.h"

using namespace Executor;

void
Executor::redraw_screen (void)
{
  TheGDeviceGuard guard(MR(MainDevice));
       vdriver_set_colors (0, 1 << vdriver_bpp,
			   CTAB_TABLE (PIXMAP_TABLE
				       (GD_PMAP (MR (MainDevice)))));
			   
       if (WWExist == EXIST_YES)
	 {
	   WindowPeek frontp;
	   frontp = (WindowPeek) FrontWindow ();
	   if (frontp)
	     {
	       RgnHandle screen_rgn;
	       Rect b;
	       
	       b = PIXMAP_BOUNDS (GD_PMAP (MR (MainDevice)));
	       screen_rgn = NewRgn ();
	       RectRgn (screen_rgn, &b);
	       PaintBehind (frontp, screen_rgn);
	       DisposeRgn (screen_rgn);
	     }
	 }
       
       DrawMenuBar ();
}
