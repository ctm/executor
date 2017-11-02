/* Copyright 1999 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_alpha[] =
	    "$Id: alpha.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/arch.h"

/* unclear what we need to do here */

bool
arch_init (void)
{
  return TRUE;
}

SWAP16_FUNC_DEFN

SWAP32_FUNC_DEFN
