/* Copyright 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_cygwin32[] = "$Id: cygwin32.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/os.h"
#include "rsys/memsize.h"
#include "rsys/mman.h"
#include "win_memory.h"
#include "win_except.h"
#include "win_keyboard.h"

static void
guess_good_memory_settings (void)
{
  unsigned long new_appl_size;

  new_appl_size = physical_memory () / 4;
  if (new_appl_size > (unsigned long) ROMlib_applzone_size)
    ROMlib_applzone_size = MIN ((unsigned long) MAX_APPLZONE_SIZE,
				new_appl_size);
}

PUBLIC bool
os_init (void)
{
  bool retval;

  ROMlib_set_caps_lock_off ();
  guess_good_memory_settings ();
  install_exception_handler ();
  retval = TRUE;
  return retval;
}

PUBLIC int
geteuid (void)
{
  int retval;

  retval = 1;
  return retval;
}

#if defined (free)

#undef free

void*	free	(void* pObject); /* from <stdlib.h> which we can't include
				    a second time.  ICK. */

void *
free_hack (void *p)
{
  void *retval;

  if (p)
    free (p);

  retval = 0;
  return retval;
}
#endif
