#if !defined(_LAUNCH_H_)
#define _LAUNCH_H_

/*
 * Copyright 1999 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: launch.h 63 2004-12-24 18:19:43Z ctm $
 */

extern uint32 ROMlib_version_long;
extern void ROMlib_set_ppc (boolean_t val);
extern int ROMlib_uaf;

typedef enum
{
  launch_no_failure,
  launch_cfm_requiring,
  launch_ppc_only,
  launch_damaged,
  launch_compressed_ge7,
  launch_compressed_lt7,
}
launch_failure_t;

extern launch_failure_t ROMlib_launch_failure;
extern INTEGER ROMlib_exevrefnum;


#endif
