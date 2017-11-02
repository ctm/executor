/* 
 * Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 *
 * Derived from public domain source code written by Sam Lantinga
 */

#if !defined (__sdlscrap_h__)
#define __sdlscrap_h__

#if defined (CYGWIN32)
#include "SDL/SDL.h"

extern SDL_Surface *surface_from_dib (void *lp);
#endif

extern bool we_lost_clipboard (void);
extern void put_scrap (int type, int srclen, char *src);
extern void get_scrap (int type, int *dstlen, char **dst);
extern void export_scrap (const SDL_Event *event);

#endif
