/* 
 * Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (__syswm_vars_h__)
#define __syswm_vars_h__

#if defined (_WIN32) && !defined (WIN32)
#define WIN32
#endif

#include "SDL/SDL.h"
#include "SDL/SDL_syswm.h"

/* System dependent variables */
#if defined(__unix__) || defined (MACOSX)
/* * */
extern Display *SDL_Display;
extern Window SDL_Window;

#elif defined(_WIN32)
/* * */
extern HWND SDL_Window;

#endif /* OS */

#endif
