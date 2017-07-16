/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_ctlMouse[] =
	    "$Id: ctlMouse.c 74 2004-12-30 03:38:55Z ctm $";
#endif

/* Forward declarations in ControlMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "WindowMgr.h"
#include "ControlMgr.h"
#include "EventMgr.h"
#include "ToolboxUtil.h"
#include "ToolboxEvent.h"
#include "OSEvent.h"
#include "ListMgr.h"

#include "rsys/ctl.h"
#include "rsys/wind.h"
#include "rsys/pstuff.h"
#include "rsys/stdfile.h"
#include "rsys/hook.h"

/*
 * Loser suggests that controls are tested in the opposite order that
 * they're on the linked list.  That would make sense, since it would
 * insure that when drawn under normal circumstances, the control that
 * is visible and clicked is the one that will be FindControl will
 * return.  Recursion is you friend.
 */

using namespace Executor;
using namespace ByteSwap;

INTEGER
find_control_helper (Point p, ControlHandle c,
		     HIDDEN_ControlHandle *cp)
{
  INTEGER retval;
  ControlHandle next;
  
  if (!c)
    retval = 0;
  else
    {
      next = CTL_NEXT_CONTROL(c);
      if (!(retval = find_control_helper(p, next, cp)))
	{
	  if (CTL_VIS (c) && CTL_HILITE (c) != 255)
	    {
	      retval = TestControl (c, p);
	      if (retval)
		cp->p = RM (c);
	    }
	}
    }
  return retval;
}

P3(PUBLIC pascal trap, INTEGER, FindControl, Point, p,	/* IMI-323 */
   WindowPtr, w, HIDDEN_ControlHandle *, cp)
{
  INTEGER retval;

  retval = w ? find_control_helper(p, WINDOW_CONTROL_LIST(w), cp) : 0;
  if (!retval)
    cp->p = 0;
  return retval;
}

typedef pascal void (*actionp)(ControlHandle c, INTEGER part);

namespace Executor {
  PRIVATE inline void CALLACTION(ControlHandle, INTEGER,
	 ProcPtr);
}

A3(PRIVATE inline, void, CALLACTION, ControlHandle, ch, INTEGER, inpart,
								    ProcPtr, a)
{
    ROMlib_hook(ctl_cdefnumber);
    HOOKSAVEREGS();
    if (a == (ProcPtr) P_ROMlib_mytrack)
	C_ROMlib_mytrack(ch, inpart);
    else if (a == (ProcPtr) P_ROMlib_stdftrack)
	C_ROMlib_stdftrack(ch, inpart);
    else
	CToPascalCall(&a, CTOP_ROMlib_mytrack, ch, inpart);
    HOOKRESTOREREGS();
}

P3 (PUBLIC pascal trap, INTEGER, TrackControl,	/* IMI-323 */
    ControlHandle, c, Point, p, ProcPtr, a)
{
  INTEGER partstart, inpart;
  EventRecord ev;
  thumbstr thumb;
  RgnHandle rh;
  LONGINT l;
  Point whereunswapped;
  
  int retval;
  
  CTL_CALL_EXCURSION
    (c,
     {
       partstart = inpart = TestControl(c, p);
       
       /* Super-Dice-It hack: It appears that Super Dice It 1.1
	  calls TrackControl with p in Global coordinates.  Without
	  this mod, we get hosed */
       if (!partstart)
	 {
	   GetMouse (&p);
	   BigEndianInPlace(p.h);
	   BigEndianInPlace(p.v);
	   partstart = inpart = TestControl (c, p);
	 }
       
       if (CTL_ACTION_AS_LONG (c) == -1L)
	 {
	   /* if we don't draw before/after, then jim's demo cdefs
	      don't update pop up menu bars */
	   CTLCALL (c, drawCntl, 0);
	   /* this is not how IMI says to do it, but it makes
	      Microsoft Word work. */
	   CTLCALL (c, autoTrack, inpart);
	   CTLCALL (c, drawCntl, 0);
	   
	   /* NOTE 1: force a return of inpart */
	   partstart = inpart;
	   goto done;
	 }
       
       if (a == (ProcPtr) -1L)
	 a = CTL_ACTION (c);

/* #if 0 reading the above code suggests that
   it's impossible to get here */
       if (0 && a == (ProcPtr) -1L)
	 {
	   /* totally custom */	
	   while (!GetOSEvent(mUpMask, &ev))
	     {
	       GlobalToLocal(&ev.where);
	       whereunswapped.h = BigEndianValue(ev.where.h);
	       whereunswapped.v = BigEndianValue(ev.where.v);
	       inpart = TestControl(c, whereunswapped);
	       CTLCALL(c, autoTrack, inpart);
	     }
	   CTLCALL (c, posCntl, (  (((int32) whereunswapped.v - p.v) << 16)
				   | (uint16) (whereunswapped.h - p.h)));
	   inpart = TestControl(c, whereunswapped);
	 }
       else
	 /* #endif */
	 if (partstart > 128)
	 {
	   /* indicator */
	   /* The code used to & the result of CTLCALL with 0xf000
	      for no apparent reason.  Taking it out fixed a bug in
	      Quicken. */
	   if (!CTLCALL (c, dragCntl, partstart))
	     {
	       thumb._tlimit.left = BigEndianValue(p.h);
	       thumb._tlimit.top  = BigEndianValue(p.v);
	       CTLCALL(c, thumbCntl, (LONGINT) (long) &thumb);
	       rh = NewRgn();
		 
	       CTLCALL(c, calcThumbRgn, (LONGINT) (long) rh);
		 
	       PATASSIGN(DragPattern, ltGray);
	       l = DragTheRgn(rh, p, &thumb._tlimit, &thumb._tslop,
			      BigEndianValue(thumb._taxis), a);
	       if ((uint32) l != 0x80008000)
		 {
		   CTLCALL(c, posCntl, l);
		   inpart = partstart;
		 }
	       DisposeRgn(rh);
	     }
	 }
       else
	 {
	   /* not an indicator */
	   HxX(c, contrlHilite) = partstart;
	   CTLCALL(c, drawCntl, partstart);
	   /* CALLACTION can remove mouse up events which is why the
	      following line is not a GetOSEvent call. */
	   while (!OSEventAvail(mUpMask, &ev) && StillDown())
	     {
	       GlobalToLocal(&ev.where);
	       whereunswapped.h = BigEndianValue(ev.where.h);
	       whereunswapped.v = BigEndianValue(ev.where.v);
	       inpart = TestControl(c, whereunswapped);
	       if (inpart && inpart != partstart)
		 inpart = 0;
	       if (inpart != U(HxX(c, contrlHilite)))
		 {
		   HxX(c, contrlHilite) = inpart;
		   CTLCALL(c, drawCntl, partstart);
		 }
	       if (a && inpart)
		 CALLACTION(c, inpart, a);
	     }
	   GetOSEvent(mUpMask, &ev);
	   GlobalToLocal(&ev.where);
	   whereunswapped.h = BigEndianValue(ev.where.h);
	   whereunswapped.v = BigEndianValue(ev.where.v);
	   if (HxX(c, contrlHilite))
	     {
	       HxX(c, contrlHilite) = 0;
	       CTLCALL(c, drawCntl, partstart);
	     }
	   inpart = TestControl(c, whereunswapped);
	 }
     done:;
     });
    
  retval = (partstart == inpart ? inpart : 0); /* DON'T CHANGE THIS
						  line w/o looking to Note 1
						  above */
  return retval;
}

P2(PUBLIC pascal trap, INTEGER, TestControl,			/* IMI-325 */
						ControlHandle, c, Point, p)
{
  int16 retval;

  CTL_CALL_EXCURSION
    (c,
     {
       if (Hx(c, contrlVis) && U(Hx(c, contrlHilite)) != 255)
	 retval = CTLCALL(c, testCntl, ((LONGINT) p.v << 16) |
			  (unsigned short) p.h);
       else
	 retval = 0;
     });
  return retval;
}
