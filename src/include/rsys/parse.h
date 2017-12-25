#if !defined(_RSYS_PARSE_H_)
#define _RSYS_PARSE_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "rsys/options.h"
extern "C" {
extern int yyparse(void); /* ick -- that's what yacc produces */
extern void ROMlib_HideScreen(void);
extern void ROMlib_SetTitle(char *name);
extern void ROMlib_SetLocation(pair_t *pairsp);
extern void ROMlib_ShowScreen(void);
extern void ROMlib_SetSize(pair_t *pairsp, pair_t *pairs2p);
extern char *ROMlib_GetTitle(void);
extern void ROMlib_FreeTitle(char *title);
}
#endif
