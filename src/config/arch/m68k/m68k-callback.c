/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_m68k_callback[] = "$Id: m68k-callback.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

typedef struct
{
  void *arg;
  callback_handler_t func;
} callback_data_t;

#define EMPTY_SLOT ((callback_handler_t) -1)

/* These are actually asm symbols, so we need the extern here. */
extern callback_data_t callback_data[NUM_CALLBACK_SLOTS];
extern uint32 callback_stubs[NUM_CALLBACK_SLOTS];

/* Last slot checked, for simple round-robin allocation algorithm
 * (which is fine since we almost never free anything).
 */
static unsigned last_stub_tried = -1;


void
callback_init (void)
{
  int i;

  for (i = NUM_CALLBACK_SLOTS - 1; i >= 0; i--)
    callback_data[i].func = EMPTY_SLOT;

  /* Flush the caches. */
  ROMlib_destroy_blocks (0, ~0, false);
}


syn68k_addr_t
callback_install (callback_handler_t func, void *arbitrary_argument)
{
  unsigned i;

  for (i = last_stub_tried + 1; i != last_stub_tried; i++)
    {
      if (i >= NUM_CALLBACK_SLOTS)
	i = -1;
      else if (callback_data[i].func == EMPTY_SLOT)
	{
	  callback_data[i].func = func;
	  callback_data[i].arg = arbitrary_argument;
	  last_stub_tried = i;
	  return (syn68k_addr_t) (&callback_stubs[i]);
	}
    }

  /* No slots free! */
  gui_abort ();
}


void *
callback_argument (syn68k_addr_t callback_address)
{
  return callback_data[(uint32 *) callback_address - callback_stubs].arg;
}


callback_handler_t
callback_function (syn68k_addr_t callback_address)
{
  return callback_data[(uint32 *) callback_address - callback_stubs].func;
}


void
callback_remove (syn68k_addr_t m68k_address)
{
  callback_data[(uint32 *) m68k_address - callback_stubs].func = EMPTY_SLOT;
}
