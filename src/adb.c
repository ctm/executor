/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_adb[] =
		"$Id: adb.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "EventMgr.h"
#include "OSEvent.h"
#include "ADB.h"
#include "rsys/adb.h"
#include "rsys/osevent.h"
#include "rsys/pstuff.h"

#include <stdarg.h>
/*
 * NOTE: most of the code in adb.c was originally written solely to support
 * apeiron (and potentially other games).  Apeiron patches the mouse ADB
 * service routines and makes a mental note of some of the data that it
 * finds in there.  Specifically it examines:
 *
 * a0@(1) bits 0-6 as a signed (bit 6 is sign bit) y delta of the mouse
 * a0@(2) bits 0-6 as a signed (bit 6 is sign bit) x delta of the mouse
 * a0@(1) bit 7 represents the current button state (0 = down)
 * a0@(2) bit 7 must be set
 *
 * We are playing very fast and loose with ADB below, specifically trying to
 * provide just enough information so Apeiron can detect mouse motion and
 * mouse presses and releases.  We realize that this code, especially
 * adb_apeiron_hack and the front-end code that calls it, have many problems:
 *
 *     We "support" the mouse, but not the keyboard
 *     We count on the installed driver not modifying the data
 *     We count on the installed driver not caring about other low-memory
 *         globals (i.e. we update them before the "driver" is called)
 *     The stub driver that we give them never does any work, so it can't
 *         be used to simulate mouse activity
 *     Probably tons more; the hack is so incomplete that it doesn't even
 *         make sense to think of all the ways it is incomplete
 */

PUBLIC void
ADBReInit (void)
{
  warning_unimplemented (NULL_STRING);
}

PUBLIC OSErr
ADBOp (Ptr data, ProcPtr procp, Ptr buffer, INTEGER command)
{
  warning_unimplemented (NULL_STRING);
  return noErr;
}

PUBLIC INTEGER
CountADBs (void)
{
  warning_unimplemented (NULL_STRING);
  return 1;
}

enum { SPOOFED_MOUSE_ADDR = 3 };

PRIVATE Ptr adb_service_procp = 0; /* stored as though in lowglobal */
PRIVATE Ptr adb_data_ptr = (Ptr) 0x90ABCDEF;

PUBLIC void
C_adb_service_stub (void)
{
}

PUBLIC void
reset_adb_vector (void)
{
  adb_service_procp = 0;
}

PUBLIC OSErr
GetIndADB (ADBDataBlock *adbp, INTEGER index)
{
  OSErr retval;

  warning_unimplemented (NULL_STRING);
  if (index != 1)
    retval = -1;
  else
    {
      adbp->devType = CB (0); /* should check on Mac to see what mouse is */
      adbp->origADBAddr = CB (SPOOFED_MOUSE_ADDR);
      if (!adb_service_procp)
	adb_service_procp = (Ptr) RM (P_adb_service_stub);
      adbp->dbServiceRtPtr = adb_service_procp;
      adbp->dbDataAreaAddr = RM (adb_data_ptr);
      retval = SPOOFED_MOUSE_ADDR;
    }
  return retval;
}

PUBLIC OSErr
GetADBInfo (ADBDataBlock *adbp, INTEGER address)
{
  OSErr retval;

  warning_unimplemented (NULL_STRING);
  if (address != SPOOFED_MOUSE_ADDR)
    retval = -1;
  else
    retval = GetIndADB (adbp, 1);
  return retval;
}

PUBLIC OSErr
SetADBInfo (ADBSetInfoBlock *adbp, INTEGER address)
{
  OSErr retval;

  warning_unimplemented (NULL_STRING);
  if (address != SPOOFED_MOUSE_ADDR)
    retval = -1;
  else
    {
      adb_service_procp = adbp->siServiceRtPtr;
      adb_data_ptr = MR (adbp->siDataAreaAddr);
      retval = noErr;
    }
  return retval;
}

PRIVATE boolean_t
adb_vector_is_not_our_own (void)
{
  boolean_t retval;

  retval = adb_service_procp != 0 &&
    adb_service_procp != (Ptr) RM (P_adb_service_stub);
  return retval;
}

PRIVATE void
call_patched_adb_vector (char *message)
{
  unsigned long save_d0, save_a0;

  save_d0 = EM_D0;
  save_a0 = EM_A0;
  EM_D0 = SPOOFED_MOUSE_ADDR << 4; /* based on Apeiron's code */
  EM_A0 = (unsigned long) US_TO_SYN68K(message);
  CALL_EMULATOR ((syn68k_addr_t) CL ((long) adb_service_procp));
  EM_D0 = save_d0;
  EM_A0 = save_a0;
}

static int
pin (int val, int min, int max)
{
  int retval;

  if (val < min)
    retval = min;
  else if (val > max)
    retval = max;
  else
    retval = val;
  return retval;
}

enum { LENGTH_OFFSET = 0, Y_OFFSET = 1, X_OFFSET = 2, MOUSE_OFFSET = 1 };
enum { BUTTON_UP_BIT = 0x80 };

/*
 * if deltas_p is TRUE, dx and dy are supplied as arguments, otherwise they
 * have to be computed as deltas off the last known MouseLocation.  Eventually
 * we should probably always have them be supplied as arguments since we
 * should probably make our mouse handling more true to real life.
 */

PUBLIC void
adb_apeiron_hack (boolean_t deltas_p, ...)
{
  static boolean_t been_here = FALSE;
  static long old_x;
  static long old_y;
  long x, y;
  boolean_t button_is_down;
  char message[3];

  x = CW (MouseLocation.h);
  y = CW (MouseLocation.v);
  button_is_down = !(ROMlib_mods & btnState);

/* begin code for PegLeg */

  if (button_is_down)
    MBState = CRACKER_ZERO;
  else
    MBState = 0xFF;

  MTemp.h = MouseLocation.h;
  MTemp.v = MouseLocation.v;

/* end code for PegLeg */

  MouseLocation2 = MouseLocation;

  if (!been_here)
    {
      old_x = x;
      old_y = y;
      been_here = TRUE;
    }
  if (adb_vector_is_not_our_own ())
    {
      int dx, dy;

      if (deltas_p)
	{
	  va_list ap;

	  va_start (ap, deltas_p);
	  dx = va_arg (ap, int);
	  dy = va_arg (ap, int);
	  va_end (ap);
	}
      else
	{
	  dx = x - old_x;
	  dy = y - old_y;
	}

      do
	{
	  int this_dx, this_dy;

	  this_dx = pin (dx, -64, 63);
	  this_dy = pin (dy, -64, 63);
	  message[LENGTH_OFFSET] = sizeof (message) - 1;
	  message[X_OFFSET] = (this_dx) | 0x80;  /* Apeiron expects the high */
	  message[Y_OFFSET] = (this_dy) | 0x80;  /*   bit to be set.         */
	  if (button_is_down)
	    message[MOUSE_OFFSET] &= ~BUTTON_UP_BIT;
	  else
	    message[MOUSE_OFFSET] |= BUTTON_UP_BIT;
	  call_patched_adb_vector (message);
	  dx -= this_dx;
	  dy -= this_dy;
	}
      while (dx || dy);
    }
  old_x = x;
  old_y = y;
}
