/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_next[] = "$Id: next.c 119 2005-07-11 21:36:20Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/os.h"


bool
os_init (void)
{
  return true;
}

PUBLIC bool host_has_spfcommon (void)
{
  return false;
}

PUBLIC bool
host_spfcommon (host_spf_reply_block *replyp, const char *prompt,
		const char *incoming_filename, void *fp, void *filef, int numt,
		void *tl, getorput_t getorput, sf_flavor_t flavor,
		void *activeList, void *activateproc, void *yourdatap)
{
  return false;
}
