/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_dosevents[] = "$Id: dosevents.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "QuickDraw.h"
#include "dosevents.h"
#include "dosevq.h"
#include "rsys/time.h"
#include "rsys/segment.h"
#include "rsys/m68kint.h"
#include "OSEvent.h"
#include "rsys/osevent.h"
#include "EventMgr.h"
#include "rsys/keyboards.h"
#include "ToolboxEvent.h"
#include "SegmentLdr.h"
#include "rsys/vgavdriver.h"
#include "rsys/myabort.h"
#include "rsys/blockinterrupts.h"
#include <sys/exceptn.h>
#include "rsys/adb.h"
#include "rsys/keyboard.h"
#include "dpmicall.h"
#include "rsys/redrawscreen.h"

static int pinned_mouse_x, pinned_mouse_y;

PRIVATE boolean_t caps_currently_locked = FALSE;

PUBLIC boolean_t need_hacky_screen_update = FALSE;

/* Extract the charcode from keywhat. */
#define CHARCODE_MASK 0xFF


/* Called soon after the mouse moves. */
static syn68k_addr_t
mouse_moved (syn68k_addr_t interrupt_addr, void *unused)
{
  virtual_int_state_t old_virt;
  int dx1, dx2, dy1, dy2, x, y;

  /* Fetch the current dx and dy, and try to minimize the chance of
   * a cursor interrupt giving us an inconsistent dx,dy pair.  Blocking
   * interrupts is relatively expensive, so we avoid doing that.
   */
  do
    {
      dx1 = DOSEVQ_MOUSE_DX ();
      dy1 = DOSEVQ_MOUSE_DY ();
      dx2 = DOSEVQ_MOUSE_DX ();
      dy2 = DOSEVQ_MOUSE_DY ();
    }
  while (dx1 != dx2 || dy1 != dy2);

  DOSEVQ_SET_MOUSE_DX (0);
  DOSEVQ_SET_MOUSE_DY (0);

  x = CW (MouseLocation.h) + dx1;
  y = CW (MouseLocation.v) + dy1;

  if (x < 0)
    x = 0;
  else if (x > pinned_mouse_x)
    x = pinned_mouse_x;

  if (y < 0)
    y = 0;
  else if (y > pinned_mouse_y)
    y = pinned_mouse_y;

  /* Actually redraw the cursor. */
  old_virt = block_virtual_ints ();

  if (need_hacky_screen_update)
    {
      vdriver_set_mode (0, 0, 0, vdriver_grayscale_p);
      redraw_screen ();
      need_hacky_screen_update = FALSE;
    }
  vga_update_cursor (x, y);
  restore_virtual_ints (old_virt);

  /* Record the current mouse position. */
  MouseLocation.h = CW (x);
  MouseLocation.v = CW (y);

  adb_apeiron_hack (TRUE, dx1, dy2);

  return MAGIC_RTE_ADDRESS;
}


/* These four originally come from the extended keyboard flags. */
#define ARDI_KB_CAPS_LOCK 0x40
#define ARDI_KB_RIGHT_ALT 0x08
#define ARDI_KB_LEFT_ALT  0x02
#define ARDI_KB_CTRL      (0x04 | 0x01)

/* This originally comes from the normal keyboard flags << 4. */
#define ARDI_KB_SHIFT     (0x10 | 0x20)

#define DOS_KEYCODE_LEFT_SHIFT  0x2A
#define DOS_KEYCODE_RIGHT_SHIFT 0x36
#define DOS_KEYCODE_CTRL        0x1D
#define DOS_KEYCODE_LEFT_ALT    0x38  /* ACK! These are the same under DOS! */
#define DOS_KEYCODE_RIGHT_ALT   0x38  /* Oh well; we'll approximate goodness.*/
#define DOS_KEYCODE_CAPS_LOCK   0x3A

static INTEGER
compute_button_mods_from_dos_event (dosevq_record_t *event)
{
  static const struct { unsigned char mask; INTEGER val; } mapping[] = {
    { ARDI_KB_SHIFT,     shiftKey   },
    { ARDI_KB_CTRL,      ControlKey },
    { ARDI_KB_LEFT_ALT,  cmdKey     },
    { ARDI_KB_RIGHT_ALT, optionKey  },
#if 0
    /* Don't use this one; we process caps-lock by hand now */
    { ARDI_KB_CAPS_LOCK, alphaLock  }
#endif
  };
  unsigned short bios_flags;
  unsigned char flags;
  INTEGER retval;
  int i;

  bios_flags = event->keyflags;

  /* Now we have to adjust the flags appropriately for rawkey events.
   * For example, if you press the shift key, the flags returned by
   * the keyboard will indicate the shift key is *not* depressed
   * on the very first event you received, and when you release it,
   * the flags will indicate that shift was down.  This makes Lemmings
   * unhappy, since when it gets the shift key up event, the flags still
   * indicate that shift is down...since we only update these flags on
   * key events, and we don't get any more key events, Lemmings acts
   * as if the shift key is still down.  To compensate, if we get a rawkey
   * event for one of the flags keys, we ask BIOS for the key flags again.
   */
  if (event->type == EVTYPE_RAWKEY)
    {
      unsigned char keycode;
 
      keycode = event->which & 0x7F;
      if (keycode == DOS_KEYCODE_LEFT_SHIFT
 	  || keycode == DOS_KEYCODE_RIGHT_SHIFT
 	  || keycode == DOS_KEYCODE_CTRL
 	  || keycode == DOS_KEYCODE_LEFT_ALT
 	  || keycode == DOS_KEYCODE_RIGHT_ALT
 	  || keycode == DOS_KEYCODE_CAPS_LOCK)
 	{
 	  __dpmi_regs regs;
  
 	  dpmi_zero_regs (&regs);
 	  regs.x.ax = 0x1200;
 	  __dpmi_int (0x16, &regs);
 	  bios_flags = regs.x.ax;
 	}
    }

  /* Convert BIOS keyboard flags to our own one-byte style. */
/* #warning "Historical; punt this and just use BIOS flags." */
  flags = ((bios_flags & 3) << 4) | ((bios_flags >> 8) & 0xCF);
  
  /* Loop through and map the appropriate bits. */
  for (i = 0, retval = 0; i < sizeof mapping / sizeof mapping[0]; i++)
    if (flags & mapping[i].mask)
      retval |= mapping[i].val;

  /* Add in ROMlib_mods. */
  retval |= ROMlib_mods & btnState;

  if (caps_currently_locked)
    retval |= alphaLock;

  return retval;
}


static LONGINT
dos_keydata_to_mac_keydata (dosevq_record_t *event, INTEGER mods,
			    int map_arrow, boolean_t down_p,
			    unsigned char *virtp)
{
  unsigned char keycode;
  unsigned char scancode;
  INTEGER keywhat;

/* #warning "We don't set the keycode and stuff properly." */

  scancode = event->which & 0x7F;

  if (map_arrow)
    switch (scancode) {
    case 0x1c: /* keyboard enter */
      scancode = 0x60;
      break;
    case 0x2a: /* used as a prefix for many keys */
      scancode = 0;
      break;
    case 0x35: /* numeric slash */
      scancode = 0x62;
      break;
    case 0x37: /* Print Screen */
      scancode = 0x63;
      break;
    case DOS_KEYCODE_RIGHT_ALT:
      scancode = 0x64;
      break;
    case 0x47: /* Home */
      scancode = 0x66;
      break;
    case 0x48:  /* The PC feeds us 0xE0/0x48 for up arrow. */
      scancode = 0x67;
      break;
    case 0x49: /* Page Up */
      scancode = 0x68;
      break;
    case 0x4B:  /* The PC feeds us 0xE0/0x4B for left arrow. */
      scancode = 0x69;
      break;
    case 0x4D:  /* The PC feeds us 0xE0/0x4D for right arrow. */
      scancode = 0x6A;
      break;
    case 0x4F: /* End */
      scancode = 0x6b;
      break;
    case 0x50:  /* The PC feeds us 0xE0/0x50 for down arrow. */
      scancode = 0x6C;
      break;
    case 0x51: /* Page Down */
      scancode = 0x76;
      break;
    case 0x52: /* Insert */
      scancode = 0x6E;
      break;
    case 0x53: /* Delete */
      scancode = 0x6f;
      break;
    default:
      warning_unexpected ("scancode = 0x%02x", scancode);
      break;
    }

  keycode = ibm_virt_to_mac_virt[scancode];

  *virtp = keycode;
  keywhat = ROMlib_xlate (keycode, mods, down_p);
  return keywhat;
}

enum { DOS_alt_status_address = 0x40 * 16 + 0x17,
	 DOS_alt_status_hold_address };

enum { caps_lock_bit = (1 << 6) };

/* Takes a DOS event and posts it as a Mac event. */
static void
post_dos_event (dosevq_record_t *event)
{
  LONGINT when;
  INTEGER button_mods;
  Point where;
  LONGINT keywhat;
  int scancode;
  static char last_rawkey_was_0xe0 = 0;
  static char last_rawkey_was_arrow = 0;
  boolean_t is_caps_lock;

  when = TickCount ();
  button_mods = ROMlib_mods = compute_button_mods_from_dos_event (event);
  where.h = CW (MouseLocation.h);
  where.v = CW (MouseLocation.v);

  switch (event->type)
    {
    case EVTYPE_RAWKEY:
      scancode = event->which;

      /* PC keyboards seem to preface arrow key sequences with 0xe0. */
      if (scancode == 0xE0)
	{
	  last_rawkey_was_arrow = 0;
	  last_rawkey_was_0xe0 = 1;
	}
      else
	{
	  unsigned char virt;

	  if (last_rawkey_was_0xe0)
	    {
	      last_rawkey_was_0xe0 = 0;
	      last_rawkey_was_arrow = 1;
	    }
	  else
	    last_rawkey_was_arrow = 0;

	  where.h = CW (MouseLocation.h);
	  where.v = CW (MouseLocation.v);

/*
 * NOTE: DOS keyboards don't mechanically lock caps-lock; instead the
 *       BIOS keeps track on its own.  When the BIOS thinks caps-lock
 *       is down then we shouldn't pass a key-up event for the caps-lock.
 *       That way Executor will continue to think caps-lock is down.
 *	 Similarly if the BIOS thinks that caps-lock is up, we don't bother
 *       passing the key-down event.
 */

	  is_caps_lock = (scancode & 0x7f) == DOS_KEYCODE_CAPS_LOCK;
	  if (is_caps_lock)
	    {
	      unsigned char b;

	      dosmemget (DOS_alt_status_address, sizeof (b), &b);
	      caps_currently_locked = !! (b & caps_lock_bit);
	  
	      if (scancode & 0x80) /* key up */
		{
		  if (caps_currently_locked)
		    break;
		}
	      else
		if (!caps_currently_locked)
		  break;
	    }

	  keywhat = dos_keydata_to_mac_keydata (event, button_mods,
						last_rawkey_was_arrow,
						!(scancode & 0x80), &virt);
	  if (keywhat != -1)
	    post_keytrans_key_events ((scancode & 0x80) ? keyUp : keyDown,
				      keywhat, when, where, button_mods, virt);
	}
      break;

    case EVTYPE_MOUSE_DOWN:
      button_mods &= ~btnState;
      ROMlib_PPostEvent (mouseDown, 0, (GUEST<EvQElPtr> *) 0, when, where,
			 button_mods);
      adb_apeiron_hack (TRUE, 0, 0);
      break;
    case EVTYPE_MOUSE_UP:
      button_mods |= btnState;
      ROMlib_PPostEvent (mouseUp, 0, (GUEST<EvQElPtr> *) 0, when, where,
			 button_mods);
      adb_apeiron_hack (TRUE, 0, 0);
      break;
    default:
      gui_abort ();
    }
}

static syn68k_addr_t
post_all_pending_dos_events (syn68k_addr_t interrupt_addr, void *unused)
{
  dosevq_record_t e;
  int have_rawkey;

  have_rawkey = FALSE;
  while (dosevq_dequeue (&e) != EVTYPE_NONE)
    {
      if (e.type == EVTYPE_RAWKEY)
	have_rawkey = TRUE;
      post_dos_event (&e);
    }

  if (have_rawkey)
    {
      __dpmi_regs regs;

      /* Drain the BIOS key buffer dry, so it doesn't overflow */
      while (1)
	{
	  dpmi_zero_regs (&regs);
	  regs.x.ax = 0x1100;
	  __dpmi_int (0x16, &regs);
	  if (regs.x.flags & I386_CC_ZERO_MASK)
	    break;

	  dpmi_zero_regs (&regs);
	  regs.x.ax = 0x1000;
	  __dpmi_int (0x16, &regs);
	}
    }

  return MAGIC_RTE_ADDRESS;
}

/* Call this once right away. */
void
init_dos_events (int max_mouse_x, int max_mouse_y)
{
  const char *errmsg;
  syn68k_addr_t mouse_moved_callback, event_callback;

  /* Turn off ctrl-c, make ctrl-break fatal. */
  __djgpp_set_ctrl_c (FALSE);

  pinned_mouse_x = max_mouse_x - 1;
  pinned_mouse_y = max_mouse_y - 1;

  errmsg = dosevq_init ();

  if (errmsg != NULL)
    {
      vdriver_shutdown ();  /* So fprintf stays visible. */
      fprintf (stderr, "%s\n", errmsg);
      exit (-100);
    }

  /* Set up faux m68k interrupts to process mouse moving and events. */
  mouse_moved_callback = callback_install (mouse_moved, NULL);
  event_callback = callback_install (post_all_pending_dos_events, NULL);
  *(syn68k_addr_t *)SYN68K_TO_US(M68K_MOUSE_MOVED_VECTOR * 4) = CL (mouse_moved_callback);
  *(syn68k_addr_t *)SYN68K_TO_US(M68K_EVENT_VECTOR * 4)       = CL (event_callback);
}


/* Grabs the current mouse position. */
void
querypointerX (LONGINT *xp, LONGINT *yp, LONGINT *notused)
{
  *xp = CW (MouseLocation.h);
  *yp = CW (MouseLocation.v);
}
