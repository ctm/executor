/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_m68k[] = "$Id: m68k.c 63 2004-12-24 18:19:43Z ctm $";
#endif
#include "rsys/common.h"


CPUState cpu_state;

boolean_t
arch_init (void)
{
  memset (&cpu_state, 0, sizeof cpu_state);
  callback_init ();
  return TRUE;
}
