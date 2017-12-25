#if !defined(__RSYS_EXECUTOR__)
#define __RSYS_EXECUTOR__

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#define BROWSER_NAME "Browser"
namespace Executor
{
extern LONGINT debugnumber;
extern LONGINT ROMlib_creator;
extern syn68k_addr_t alinehandler(syn68k_addr_t pc, void *ignored);
extern void setupsignals(void);
extern void filltables(void);
extern void executor_main(void);
}
#endif
