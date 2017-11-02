/* Copyright 1997, 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_sdlevents[] = "$Id: sdlevents.c 88 2005-05-25 03:59:37Z ctm $";
#endif

#include "rsys/common.h"
#include "QuickDraw.h"
#include "EventMgr.h"
#include "SegmentLdr.h"
#include "OSEvent.h"
#include "ToolboxEvent.h"
#include "ScrapMgr.h"

#include "rsys/segment.h"
#include "rsys/m68kint.h"
#include "rsys/osevent.h"
#include "rsys/keyboard.h"
#include "rsys/adb.h"
#include "rsys/scrap.h"
#include "rsys/toolevent.h"
#include "rsys/keyboard.h"

#include "SDL/SDL.h"

using namespace Executor;

PRIVATE bool use_scan_codes = false;

PUBLIC void
ROMlib_set_use_scancodes (bool val)
{
  use_scan_codes = val; 
}

#if SDL_MAJOR_VERSION == 0 && SDL_MINOR_VERSION < 9

#include "sdlk_to_mkv.h"

PRIVATE void
init_sdlk_to_mkv (void)
{
}
#else

enum { NOTAKEY = 0x89 };

PRIVATE unsigned char sdlk_to_mkv[SDLK_LAST];

typedef struct
{
  SDLKey sdlk;
  unsigned char mkv;
}
sdl_to_mkv_map_t;

PRIVATE sdl_to_mkv_map_t map[] =
{
  { SDLK_BACKSPACE, MKV_BACKSPACE },
  { SDLK_TAB, MKV_TAB, },
  { SDLK_CLEAR, NOTAKEY, },
  { SDLK_RETURN, MKV_RETURN, },
  { SDLK_ESCAPE, MKV_ESCAPE, },
  { SDLK_SPACE, MKV_SPACE, },
  { SDLK_QUOTE, MKV_TICK, },
  { SDLK_COMMA, MKV_COMMA, },
  { SDLK_MINUS, MKV_MINUS, },
  { SDLK_PERIOD, MKV_PERIOD, },
  { SDLK_SLASH, MKV_SLASH, },
  { SDLK_0, MKV_0, },
  { SDLK_1, MKV_1, },
  { SDLK_2, MKV_2, },
  { SDLK_3, MKV_3, },
  { SDLK_4, MKV_4, },
  { SDLK_5, MKV_5, },
  { SDLK_6, MKV_6, },
  { SDLK_7, MKV_7, },
  { SDLK_8, MKV_8, },
  { SDLK_9, MKV_9, },
  { SDLK_SEMICOLON, MKV_SEMI, },
  { SDLK_EQUALS, MKV_EQUAL, },
  { SDLK_KP0, MKV_NUM0, },
  { SDLK_KP1, MKV_NUM1, },
  { SDLK_KP2, MKV_NUM2, },
  { SDLK_KP3, MKV_NUM3, },
  { SDLK_KP4, MKV_NUM4, },
  { SDLK_KP5, MKV_NUM5, },
  { SDLK_KP6, MKV_NUM6, },
  { SDLK_KP7, MKV_NUM7, },
  { SDLK_KP8, MKV_NUM8, },
  { SDLK_KP9, MKV_NUM9, },
  { SDLK_KP_PERIOD, MKV_NUMPOINT, },
  { SDLK_KP_DIVIDE, MKV_NUMDIVIDE, },
  { SDLK_KP_MULTIPLY, MKV_NUMMULTIPLY, },
  { SDLK_KP_MINUS, MKV_NUMMINUS, },
  { SDLK_KP_PLUS, MKV_NUMPLUS, },
  { SDLK_KP_ENTER, MKV_NUMENTER, },
  { SDLK_LEFTBRACKET, MKV_LEFTBRACKET, },
  { SDLK_BACKSLASH, MKV_BACKSLASH, },
  { SDLK_RIGHTBRACKET, MKV_RIGHTBRACKET, },
  { SDLK_BACKQUOTE, MKV_BACKTICK, },
  { SDLK_a, MKV_a, },
  { SDLK_b, MKV_b, },
  { SDLK_c, MKV_c, },
  { SDLK_d, MKV_d, },
  { SDLK_e, MKV_e, },
  { SDLK_f, MKV_f, },
  { SDLK_g, MKV_g, },
  { SDLK_h, MKV_h, },
  { SDLK_i, MKV_i, },
  { SDLK_j, MKV_j, },
  { SDLK_k, MKV_k, },
  { SDLK_l, MKV_l, },
  { SDLK_m, MKV_m, },
  { SDLK_n, MKV_n, },
  { SDLK_o, MKV_o, },
  { SDLK_p, MKV_p, },
  { SDLK_q, MKV_q, },
  { SDLK_r, MKV_r, },
  { SDLK_s, MKV_s, },
  { SDLK_t, MKV_t, },
  { SDLK_u, MKV_u, },
  { SDLK_v, MKV_v, },
  { SDLK_w, MKV_w, },
  { SDLK_x, MKV_x, },
  { SDLK_y, MKV_y, },
  { SDLK_z, MKV_z, },
  { SDLK_DELETE, MKV_DELFORWARD, },
  { SDLK_F1, MKV_F1, },
  { SDLK_F2, MKV_F2, },
  { SDLK_F3, MKV_F3, },
  { SDLK_F4, MKV_F4, },
  { SDLK_F5, MKV_F5, },
  { SDLK_F6, MKV_F6, },
  { SDLK_F7, MKV_F7, },
  { SDLK_F8, MKV_F8, },
  { SDLK_F9, MKV_F9, },
  { SDLK_F10, MKV_F10, },
  { SDLK_F11, MKV_F11, },
  { SDLK_F12, MKV_F12, },
  { SDLK_F13, MKV_F13, },
  { SDLK_F14, MKV_F14, },
  { SDLK_F15, MKV_F15, },
  { SDLK_PAUSE, MKV_PAUSE, },
  { SDLK_NUMLOCK, MKV_NUMCLEAR, },
  { SDLK_UP, MKV_UPARROW, },
  { SDLK_DOWN, MKV_DOWNARROW, },
  { SDLK_RIGHT, MKV_RIGHTARROW, },
  { SDLK_LEFT, MKV_LEFTARROW, },
  { SDLK_INSERT, MKV_HELP, },
  { SDLK_HOME, MKV_HOME, },
  { SDLK_END, MKV_END, },
  { SDLK_PAGEUP, MKV_PAGEUP, },
  { SDLK_PAGEDOWN, MKV_PAGEDOWN, },
  { SDLK_CAPSLOCK, MKV_CAPS, },
  { SDLK_SCROLLOCK, MKV_SCROLL_LOCK, },
  { SDLK_RSHIFT, MKV_RIGHTSHIFT, },
  { SDLK_LSHIFT, MKV_LEFTSHIFT, },
  { SDLK_RCTRL, MKV_RIGHTCNTL, },
  { SDLK_LCTRL, MKV_LEFTCNTL, },
  { SDLK_RALT, MKV_RIGHTOPTION, },
  { SDLK_LALT, MKV_CLOVER, },
  { SDLK_RMETA, MKV_RIGHTOPTION, },
  { SDLK_LMETA, MKV_CLOVER, },
  { SDLK_HELP, MKV_HELP, },
  { SDLK_PRINT, MKV_PRINT_SCREEN, },
  { SDLK_SYSREQ, NOTAKEY, },
  { SDLK_MENU, NOTAKEY, },
  { SDLK_BREAK, NOTAKEY, },
};

PRIVATE void
init_sdlk_to_mkv (void)
{
  static bool been_here = false;

  if (!been_here)
    {
      unsigned int i;

      for (i = 0; i < NELEM (sdlk_to_mkv); ++i)
	sdlk_to_mkv[i] = NOTAKEY;

      for (i = 0; i < NELEM (map); ++i)
	{
	  SDLKey sdlk;
	  unsigned char mkv;

	  sdlk = map[i].sdlk;
	  mkv  = map[i].mkv;
	  sdlk_to_mkv[sdlk] = mkv;
	}
      been_here = true;
    }
}

#endif

#include "sdlevents.h"
#include "sdlscrap.h"
#include "sdlquit.h"
#include "syswm_map.h"

/* The current state of the keyboard modifiers */
static uint16 keymod = 0;
static uint16 right_button_keymod = 0;
static GUEST<Point>  mouseloc;		/* To save mouse location at interrupt */

static int modifier_p(unsigned char virt, uint16 *modstore)
{
  /* Note: shift and control can be cleared if right* and left* are pressed */
  switch (virt)
    {
    case MKV_LEFTSHIFT:
    case MKV_RIGHTSHIFT:
      *modstore = shiftKey;
      break;
    case MKV_CAPS:
      *modstore = alphaLock;
      break;
    case MKV_LEFTCNTL:
    case MKV_RIGHTCNTL:
      *modstore = ControlKey;
      break;
    case MKV_CLOVER:
      *modstore = cmdKey;
      break;
    case MKV_LEFTOPTION:
    case MKV_RIGHTOPTION:
      *modstore = optionKey;
      break;
    default:
      *modstore = 0;
      return 0;
    }
  return 1;
}
 
syn68k_addr_t
handle_sdl_mouse(syn68k_addr_t interrupt_addr, void *unused)
{
        MouseLocation = mouseloc;
  adb_apeiron_hack(false);
  return(MAGIC_RTE_ADDRESS);
}

syn68k_addr_t
handle_sdl_events(syn68k_addr_t interrupt_addr, void *unused)
{
        SDL_Event event;

  while ( SDL_PollEvent(&event) )
    {
      switch (event.type)
	{
	case SDL_ACTIVEEVENT:
          {
            if ( event.active.state & SDL_APPINPUTFOCUS )
              {
                if ( !event.active.gain )
		  sendsuspendevent ();
		else
                  {
                    if ( !we_lost_clipboard () )
		      sendresumeevent (false);
		    else
                      {
                        ZeroScrap ();
			sendresumeevent (true);
                      }
                  }
              }
          }
	break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
	  {
	    bool down_p;
	    int32     when;
	    Point     where;

	    if ( event.button.button == 3 )
	      {
		if (ROMlib_right_button_modifier !=
		    (optionKey|keyDownMask|keyUpMask))
		  {
		    if (event.type == SDL_MOUSEBUTTONDOWN)
		      right_button_keymod |= ROMlib_right_button_modifier;
		    else
		      right_button_keymod &= ~ROMlib_right_button_modifier;
		  }
		else
		  {
		    /* rewrite the event as though it were the option
		       key rather than a mouse click */
		    if (event.type == SDL_MOUSEBUTTONDOWN)
		      {
			event.type = SDL_KEYDOWN;
			event.key.state = SDL_PRESSED;
		      }
		    else
		      {
			event.type = SDL_KEYUP;
			event.key.state = 0;
		      }
		    event.key.keysym.scancode = 0x64;
		    event.key.keysym.sym = SDLK_RALT;
/*-->*/		    goto key_down_or_key_up;
		  }
	      }

	    down_p = (event.button.state == SDL_PRESSED);
	    if ( down_p )
	      keymod &= ~btnState;
	    else
	      keymod |=  btnState;
	    when = TickCount ();
	    where.h = event.button.x;
	    where.v = event.button.y;
	    ROMlib_PPostEvent(down_p ? mouseDown : mouseUp,
			      0, (GUEST<EvQElPtr> *) 0, when, where,
			      keymod | right_button_keymod);
	    adb_apeiron_hack (false);
	  }
        break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
key_down_or_key_up:
	  {
	    bool down_p;
	    unsigned char mkvkey;
	    uint16 mod;
	    LONGINT keywhat;
	    int32 when;
	    Point where;

	    init_sdlk_to_mkv ();
	    down_p = (event.key.state == SDL_PRESSED);

	    if (use_scan_codes)
	      mkvkey = ibm_virt_to_mac_virt[event.key.keysym.scancode];
	    else
	      mkvkey = sdlk_to_mkv[event.key.keysym.sym];
	    mkvkey = ROMlib_right_to_left_key_map (mkvkey);
	    if ( modifier_p(mkvkey, &mod) )
	      {
		if ( down_p )
		  keymod |=  mod;
		else
		  keymod &= ~mod;
	      }
	    when = TickCount ();
	    where.h = CW(MouseLocation.h);
	    where.v = CW(MouseLocation.v);
	    keywhat = ROMlib_xlate(mkvkey, keymod, down_p);
	    post_keytrans_key_events(down_p ? keyDown : keyUp,
				     keywhat, when, where,
				     keymod | right_button_keymod, mkvkey);
	  }
        break;

        case SDL_QUIT:
	  {
	    ROMlib_exit = 1;
	    ExitToShell();
	  }
        break;

	}
    }
  return(MAGIC_RTE_ADDRESS);
}


/* This function runs in a separate thread (usually) */
int sdl_event_interrupt(const SDL_Event *event)
{
  if ( event->type == SDL_MOUSEMOTION )
    {
      mouseloc.h = CW (event->motion.x);
      mouseloc.v = CW (event->motion.y);
      cpu_state.interrupt_pending[M68K_MOUSE_MOVED_PRIORITY] = 1;
      cpu_state.interrupt_status_changed = INTERRUPT_STATUS_CHANGED;
      return(0);			/* Drop the event */
    }
  else if ( event->type == SDL_QUIT )
    {
      /* Query whether or not we should quit */
      if ( ! sdl_really_quit() )
        return(0);
    }
  else if ( event->type == SDL_SYSWMEVENT )
    {
      /* Pass it to a system-specific event handler */
      return(sdl_syswm_event(event));
    }

  /* All other events go here */
  cpu_state.interrupt_pending[M68K_EVENT_PRIORITY] = 1;
  cpu_state.interrupt_status_changed = INTERRUPT_STATUS_CHANGED;
  return(1);
}

PUBLIC SDL_cond *ROMlib_shouldbeawake_cond = NULL;
PUBLIC SDL_mutex *ROMlib_shouldbeawake_mutex = NULL;

void sdl_events_init(void)
{
  syn68k_addr_t mouse_callback;
  syn68k_addr_t event_callback;

  ROMlib_shouldbeawake_cond = SDL_CreateCond ();
  ROMlib_shouldbeawake_mutex = SDL_CreateMutex ();


  /* hook into syn68k synchronous interrupts */
  mouse_callback = callback_install (handle_sdl_mouse, NULL);
  *(GUEST<syn68k_addr_t> *) SYN68K_TO_US(M68K_MOUSE_MOVED_VECTOR * 4) = CL (mouse_callback);
  event_callback = callback_install (handle_sdl_events, NULL);
  *(GUEST<syn68k_addr_t> *) SYN68K_TO_US(M68K_EVENT_VECTOR * 4) = CL (event_callback);

  /* then set up a filter that triggers the event interrupt */
  SDL_SetEventFilter(sdl_event_interrupt);
  SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
}
