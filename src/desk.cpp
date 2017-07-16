/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_desk[] =
		"$Id: desk.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in DeskMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "EventMgr.h"
#include "WindowMgr.h"
#include "DeviceMgr.h"
#include "DeskMgr.h"
#include "MenuMgr.h"
#include "QuickDraw.h"
#include "OSEvent.h"
#include "ToolboxEvent.h"
#include "OSUtil.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"
#include "rsys/hook.h"
#include "rsys/aboutbox.h"

using namespace Executor;
using namespace ByteSwap;

P1(PUBLIC pascal trap, INTEGER, OpenDeskAcc, Str255, acc)	/* IMI-440 */
{
    INTEGER retval;
    DCtlHandle dctlh;
    WindowPtr wp;

    if (EqualString (acc, about_box_menu_name_pstr, TRUE, TRUE))
      {
	do_about_box ();
	retval = 0;
	goto done;
      }

    if (OpenDriver(acc, &retval) == noErr) {
	retval = BigEndianValue(retval);
	dctlh = GetDCtlEntry(retval);
	if (dctlh)
	  {
	    wp = HxP(dctlh, dCtlWindow);
	    if (wp)
	      {
		ShowWindow(wp);
		SelectWindow(wp);
	      }
	  }
    }

  done:

    SEvtEnb = TRUE;
    return retval;
}

P1(PUBLIC pascal trap, void, CloseDeskAcc, INTEGER, rn)
{
    CloseDriver(rn);
}

P2(PUBLIC pascal trap, void, SystemClick, EventRecord *, evp, WindowPtr, wp)
{
    Point p;
    LONGINT pointaslong, val;
    Rect bounds;
    LONGINT templ;

    if (wp) {
	p.h = BigEndianValue(evp->where.h);
	p.v = BigEndianValue(evp->where.v);
	if (PtInRgn (p, WINDOW_STRUCT_REGION (wp)))
	  {
	    pointaslong = ((LONGINT)p.v << 16)|(unsigned short)p.h;
	    val = WINDCALL((WindowPtr) wp, wHit, pointaslong);
	    switch (val) {
	    case wInContent:
		if (WINDOW_HILITED_X (wp))
		  {
		    templ = (LONGINT) (long) RM(evp);
		    Control (WINDOW_KIND (wp), accEvent, (Ptr) &templ);
		} else
		    SelectWindow(wp);
		break;
	    case wInDrag:
		bounds.top    = BigEndianValue (BigEndianValue (MBarHeight) + 4);
		bounds.left   = BigEndianValue (BigEndianValue (GD_BOUNDS (MR (TheGDevice)).left) + 4);
		bounds.bottom = BigEndianValue (BigEndianValue (GD_BOUNDS (MR (TheGDevice)).bottom) - 4);
		bounds.right  = BigEndianValue (BigEndianValue (GD_BOUNDS (MR (TheGDevice)).right) - 4);
		DragWindow(wp, p, &bounds);
		break;
	    case wInGoAway:
		if (TrackGoAway(wp, p))
		    CloseDeskAcc (WINDOW_KIND (wp));
		break;
	    }
	} else {
	    if (DeskHook) {
		ROMlib_hook(desk_deskhooknumber);
		EM_D0 = -1;
		EM_A0 = (LONGINT) (long) US_TO_SYN68K(evp);
		CALL_EMULATOR((syn68k_addr_t) (long) BigEndianValue((long) DeskHook));
	    }
	}
    }
}

P1(PUBLIC pascal trap, BOOLEAN, SystemEdit, INTEGER, editcmd)
{
    WindowPeek wp;
    BOOLEAN retval;

    wp = (WindowPeek) FrontWindow();
    if (!wp)
      retval = FALSE;
    else
      if ((retval = WINDOW_KIND (wp) < 0))
	Control (WINDOW_KIND (wp), editcmd + accUndo, (Ptr) 0);
    return retval;
}

#define rntodctlh(rn)	(MR(MR(UTableBase)[-((rn)+1)].p))
#define itorn(i)	((-i)-1)

P0(PUBLIC pascal trap, void, SystemTask)
{
    DCtlHandle dctlh;
    INTEGER i;

    for (i = 0; i < BigEndianValue(UnitNtryCnt); ++i) {
	dctlh = MR(MR(UTableBase)[i].p);
	if ((HxX(dctlh, dCtlFlags) & CWC(NEEDTIMEBIT)) &&
				      TickCount() >= Hx(dctlh, dCtlCurTicks)) {
	    Control(itorn(i), accRun, (Ptr) 0);
	    HxX(dctlh, dCtlCurTicks) = BigEndianValue(Hx(dctlh, dCtlCurTicks) +
							 Hx(dctlh, dCtlDelay));
	}
    }
}

P1(PUBLIC pascal trap, BOOLEAN, SystemEvent, EventRecord *, evp)
{
    BOOLEAN retval;
    WindowPeek wp;
    INTEGER rn;
    DCtlHandle dctlh;
    LONGINT templ;

    if (SEvtEnb) {
	wp = 0;
	switch (evp->what) {
	default:
	case CWC(nullEvent):
	case CWC(mouseDown):
	case CWC(networkEvt):
	case CWC(driverEvt):
	case CWC(app1Evt):
	case CWC(app2Evt):
	case CWC(app3Evt):
	case CWC(app4Evt):
	    break;
	case CWC(mouseUp):
	case CWC(keyDown):
	case CWC(keyUp):
	case CWC(autoKey):
	    wp = (WindowPeek) FrontWindow();
	    break;
	case CWC(updateEvt):
	case CWC(activateEvt):
	    wp = (WindowPeek) MR(evp->message);
	    break;
	case CWC(diskEvt):
	    /* NOTE:  I think the code around toolevent.c:277 should
		      really be here.  I'm not going to get all excited
		      about it right now though. */
	    break;
	}
	if (wp) {
	    rn = WINDOW_KIND (wp);
	    if ((retval = rn < 0)) {
		dctlh = rntodctlh(rn);
		if (Hx(dctlh, dCtlEMask) & (1 << BigEndianValue(evp->what))) {
		    templ = (LONGINT) (long) RM(evp);
		    Control(rn, accEvent, (Ptr) &templ);
		}
	    }
	} else
	    retval = FALSE;

    } else
	retval = FALSE;
    return retval;
}

P1(PUBLIC pascal trap, void, SystemMenu, LONGINT, menu)
{
    INTEGER i;
    DCtlHandle dctlh;

    for (i = 0; i < BigEndianValue(UnitNtryCnt); ++i) {
	dctlh = MR(MR(UTableBase)[i].p);
	if (HxX(dctlh, dCtlMenu) == MBarEnable) {
	    menu = BigEndianValue(menu);
	    Control(itorn(i), accMenu, (Ptr) &menu);
/*-->*/	    break;
	}
    }
}
