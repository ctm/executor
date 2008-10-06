#if !defined(__RSYS_EXECUTOR__)
#define __RSYS_EXECUTOR__

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: executor.h 63 2004-12-24 18:19:43Z ctm $
 */

#define BROWSER_NAME "Browser"

extern LONGINT debugnumber;
extern LONGINT ROMlib_creator;
extern syn68k_addr_t alinehandler(syn68k_addr_t pc, void *ignored);
extern void setupsignals(void);
extern void filltables (void);
extern void executor_main (void);

#endif
