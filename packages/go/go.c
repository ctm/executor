#include "go.h"

#include "go.proto.h"
#include "misc.proto.h"
#include "mouse.proto.h"
#include "keyboard.proto.h"
#include "update.proto.h"
#include "init.proto.h"

/* globals */
long g_screensavetime = 54000;	/* 60*60*15 = 15 minutes is sixtieths of seconds */
long g_lastclick = 0;
short g_done;
short ourid = 12;

typedef enum
  {
    uninitialized_state, closable_state, nonclosable_state
  }
window_state_t;

void
doevents (void)
{
  EventRecord ev;
  static window_state_t old_front_window_state = uninitialized_state;
  window_state_t new_front_window_state;

  g_done = false;
  while (!g_done)
    {
      WaitNextEvent (everyEvent, &ev, g_screensavetime, (RgnHandle) 0);
      switch (ev.what)
	{
	case nullEvent:
	  screensaver ();
	  break;
	case mouseDown:
	  domousedown (&ev);
	  break;
	case keyDown:
	  dokeydown (&ev);
	  break;
	case updateEvt:
	  doupdate (&ev);
	  break;
	case activateEvt:
	  doactivate (&ev);
	  break;
	case diskEvt:
	  break;
	case mouseUp:
	  g_lastclick = ev.when;
	  break;

	case autoKey:
	case keyUp:
	case networkEvt:
	case driverEvt:
	case app1Evt:
	case app2Evt:
	case app3Evt:
	case app4Evt:
/* DO NOTHING */
	  break;
	default:
	  break;
	}
      new_front_window_state = is_closable (FrontWindow ())
	? closable_state : nonclosable_state;
      if (new_front_window_state != old_front_window_state)
	{
	  if (new_front_window_state == closable_state)
	    enable_menu_item (close_menuid);
	  else if (new_front_window_state == nonclosable_state)
	    disable_menu_item (close_menuid);
	  old_front_window_state = new_front_window_state;
	}
    }
}

void
main (void)
{
  init ();
  doevents ();
  savestate ();
}
