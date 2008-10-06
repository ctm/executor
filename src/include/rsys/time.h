#if !defined (_RSYS_TIME_H_)
#define _RSYS_TIME_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: time.h 63 2004-12-24 18:19:43Z ctm $
 */

extern struct timeval ROMlib_start_time;

extern unsigned long msecs_elapsed (void);

#if !defined (USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)
extern QHdr ROMlib_timehead;

#if defined (SYN68K)
extern syn68k_addr_t catchalarm (syn68k_addr_t pc, void *unused);
#endif
#endif

#endif /* _RSYS_TIME_H_ */
