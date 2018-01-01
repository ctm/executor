#if !defined(_RSYS_TIME_H_)
#define _RSYS_TIME_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

extern struct timeval ROMlib_start_time;

extern unsigned long msecs_elapsed(void);

namespace Executor
{
#if !defined(USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)
extern QHdr ROMlib_timehead;

extern syn68k_addr_t catchalarm(syn68k_addr_t pc, void *unused);
#endif
}

#endif /* _RSYS_TIME_H_ */
