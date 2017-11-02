/* Copyright 1994, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_sdlwm[] = "$Id: sdlwm.c 88 2005-05-25 03:59:37Z ctm $";
#endif

#include <stdio.h>
#include "SDL/SDL.h"

#include "rsys/common.h"
#include "rsys/host.h"
#include "rsys/parse.h" /* FIXME: name one good reason why SetTitle should be declared in a file named parse.h */

/* Globals */
int Executor::host_cursor_depth = 1;

/* Window manager interface functions */

extern "C" void
ROMlib_SetTitle (char *title)
{
  SDL_WM_SetCaption(title, "executor");
}

extern "C" char *
ROMlib_GetTitle (void)
{
  char *retval;

  SDL_WM_GetCaption(&retval, (char **)0);
  return retval;
}

extern "C" void
ROMlib_FreeTitle (char *title)
{
}

/* This is really inefficient.  We should hash the cursors */
void
Executor::host_set_cursor (char *cursor_data,
                 unsigned short cursor_mask[16],
                 int hotspot_x, int hotspot_y)
{
  SDL_Cursor *old_cursor, *new_cursor;

  old_cursor = SDL_GetCursor();
  new_cursor = SDL_CreateCursor((unsigned char *) cursor_data,
				(unsigned char *) cursor_mask,
				16, 16, hotspot_x, hotspot_y);
  if ( new_cursor != NULL )
    {
      SDL_SetCursor(new_cursor);
      SDL_FreeCursor(old_cursor);
    }
}

int
Executor::host_set_cursor_visible (int show_p)
{
  return(SDL_ShowCursor(show_p));
}
