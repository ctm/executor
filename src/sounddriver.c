/* Copyright 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_sounddriver[] =
	    "$Id: sounddriver.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/sounddriver.h"
#include "rsys/soundfake.h"

/* This is the current sound driver. */
sound_driver_t sound_driver;

typedef boolean_t (*sound_init_func) (sound_driver_t *);

static sound_init_func sound_driver_init_funcs[] =
{
#if defined (SOUND_SDL)
  sound_sdl_init,
#endif

#if defined (SOUND_LINUX)
  sound_linux_init,
#endif

#if defined (SOUND_DJGPP)
  sound_djgpp_init,
#endif

  /* This should always initialize successfully. */
  sound_fake_init,
};


void
sound_init (void)
{
  boolean_t found_one_p;
  int i;

  /* Try all available sound drivers and keep the first one that works. */
  found_one_p = FALSE;
  for (i = 0; !found_one_p && i < (int) NELEM (sound_driver_init_funcs); i++)
    if ((sound_driver_init_funcs[i]) (&sound_driver))
      found_one_p = TRUE;

  /* At least the fake sound driver must always match! */
  gui_assert (found_one_p);
}
