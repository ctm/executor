/* Copyright 1986, 1988, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_ctlDisplay[] =
	    "$Id: ctlDisplay.c 85 2005-05-24 22:06:13Z ctm $";
#endif

/* Forward declarations in ControlMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "WindowMgr.h"
#include "ControlMgr.h"

#include "rsys/ctl.h"
#include "rsys/wind.h"

using namespace Executor;

P2(PUBLIC pascal trap, void, SetCTitle, ControlHandle, c,	/* IMI-321 */
								StringPtr, t)
{
  if (c)
    {
      CTL_CALL_EXCURSION
	(c,
	 {
	   RgnHandle rh;
       
	   rh = NewRgn();
	   CTLCALL(c, calcCntlRgn, (LONGINT) (long) rh);
	   EraseRgn(rh);
	   str255assign(HxX(c, contrlTitle), t);
	   CTLCALL(c, drawCntl, ENTIRECONTROL);
	   DisposeRgn(rh);
	 });
    }
}

P2(PUBLIC pascal trap, void, GetCTitle, ControlHandle, c,	/* IMI-321 */
								  StringPtr, t)
{
  if (c)
    str255assign(t, HxX(c, contrlTitle));
}

P1(PUBLIC pascal trap, void, HideControl, ControlHandle, c)	/* IMI-322 */
{
  if (c)
    {
      CTL_CALL_EXCURSION
	(c,
	 {
	   RgnHandle rh;
       
	   if (c && CTL_VIS (c))
	     {
	       rh = NewRgn();
	       /* #### warning; should this be called with the control
		  owner as the current port, or no? */
	       CTLCALL(c, calcCntlRgn, (LONGINT) (long) rh);


	       /* The following code is a hack that works around a problem
		  due to the different meanings of the fgColor and bgColor
		  fields of a grafport vs. a cgrafport.  In a grafport, those
		  fields contain magic numbers, but in a cgrafport they
		  contain colortable indices.  This means that if a grafport
		  is turned into a cgrafport, the old values may cause
		  trouble.  This makes me wonder whether or not our use of
		  them as indices is really correct.  W/o the following fix,
		  LogDig will erase magenta when you open a file and then
		  use Cmd-= to create a check-box, if you check and un-check
		  the check-box. */

	       if (CGrafPort_p (thePort))
		 {
		   RGBForeColor (&CPORT_RGB_FG_COLOR (thePort));
		   RGBBackColor (&CPORT_RGB_BK_COLOR (thePort));
		 }

	       EraseRgn(rh);
	       InvalRgn(rh);
	       DisposeRgn(rh);
	       CTL_VIS (c) = FALSE;
	     }
	 });
    }
}

/* It appears that ShowControl does not erase anything before calling
   drawCntl.  This is based on Loser running on the Mac, vs. running
   under Executor.  The EraseRgn used to be an EraseRect that was put
   into ctlDisplay a long time ago, and with no fanfare.  Perhaps
   there is more going on here; I don't know. */

P1(PUBLIC pascal trap, void, ShowControl, ControlHandle, c)	/* IMI-322 */
{
  if (c)
    {
      CTL_CALL_EXCURSION
	(c,
	 {
	   /* #if SHOWCONTROL_ERASES
	      RgnHandle rh; */
	   
	   if (c && !CTL_VIS (c))
	     {
	       /* #if SHOWCONTROL_ERASES
		  rh = NewRgn(); */
	       
	       HxX(c, contrlVis) = 255;
	       
	       /* #if SHOWCONTROL_ERASES
		  CTLCALL(c, calcCntlRgn, (LONGINT) (long) rh);
		  EraseRgn(rh);
		  DisposeRgn(rh); */
	   
	       CTLCALL(c, drawCntl, 0);
	     }
	 });
    }
}

P2 (PUBLIC pascal trap, void, HiliteControl, ControlHandle, c, 	/* IMI-322 */
    INTEGER, state)
{
  if (c && c->p)
    {
      if (CTL_VIS (c))
	{
	  int16 oldh;
	  
	  oldh = CTL_HILITE (c);
	  if (oldh != state)
	    {
	      CTL_CALL_EXCURSION
		(c,
		 {
		   HxX (c, contrlHilite) = state;
		   if (oldh == INACTIVE || state == INACTIVE)
		     CTLCALL(c, drawCntl, 0);
		   else
		     CTLCALL(c, drawCntl, state);
		 });
	    }
	}
      else
	CTL_HILITE_X (c) = state;
    }
}

P1(PUBLIC pascal trap, void, DrawControls, WindowPtr, w)	/* IMI-322 */
{
  ControlHandle c;
  
  if (! ROMlib_emptyvis)
    {
      for (c = WINDOW_CONTROL_LIST (w); c ; c = CTL_NEXT_CONTROL (c))
	Draw1Control(c);
    }
}
