/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_ctlIMIV[] =
"$Id: ctlIMIV.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in ControlMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "ControlMgr.h"

#include "rsys/ctl.h"
#include "rsys/wind.h"

using namespace Executor;

P1(PUBLIC pascal trap, void, Draw1Control, ControlHandle, c)	/* IMIV-53 */
{
  if (CTL_VIS_X (c))
    {
      CTL_CALL_EXCURSION
	(c,
	 {
	   CTLCALL(c, drawCntl, 0);
	 });
    }
}

P2 (PUBLIC pascal trap, void, UpdtControl, WindowPtr, wp,	/* IMIV-53 */
    RgnHandle, rh)
{
  ControlHandle c;
  
  if (! ROMlib_emptyvis)
    {
      for (c = WINDOW_CONTROL_LIST (wp); c ; c = CTL_NEXT_CONTROL (c))
	if (RectInRgn (&CTL_RECT (c), rh))
	  Draw1Control(c);
    }
}
