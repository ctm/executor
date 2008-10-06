/* Copyright 1999 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_win_screen[] = "$Id: win_screen.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"
#include "rsys/error.h"

#include "syswm_vars.h"

#include <windows.h>

PUBLIC int
os_current_screen_width (void)
{
  int retval;

  retval = GetSystemMetrics (SM_CXSCREEN);
  return retval;
}

PUBLIC int
os_current_screen_height (void)
{
  int retval;

  retval = GetSystemMetrics (SM_CYSCREEN);
  return retval;
}

PUBLIC int
os_maximum_window_height (void)
{
  int retval;

  if (ROMlib_fullscreen_p)
    retval = os_current_screen_height ();
  else
    retval = GetSystemMetrics (SM_CYFULLSCREEN);
  return retval;
}

PUBLIC int
os_maximum_window_width (void)
{
  int retval;

  if (ROMlib_fullscreen_p)
    retval = os_current_screen_width ();
  else
    retval = GetSystemMetrics (SM_CXFULLSCREEN);
  return retval;
}

PUBLIC void
ROMlib_recenter_window (void)
{
  if (!ROMlib_fullscreen_p)
    {
      RECT bounds;

      if (GetWindowRect (SDL_Window, &bounds))
	{
	  int width, height, caption_height;
	  int x, y;

	  caption_height = GetSystemMetrics (SM_CYCAPTION);
	  width = bounds.right-bounds.left;
	  height = bounds.bottom-bounds.top + caption_height;
	  x = (GetSystemMetrics(SM_CXFULLSCREEN)-width)/2;
	  y = (GetSystemMetrics(SM_CYFULLSCREEN)-height)/2;
#if 0
	  {
	    LONG l;

	    l = GetWindowLong (SDL_Window, GWL_STYLE);
	    if (!(l & WS_MINIMIZE))
		l |= WS_MAXIMIZEBOX;
#if 0
	    l &= ~WS_MAXIMIZE;
#endif
	    SetWindowLong (SDL_Window, GWL_STYLE, l);
#if 0
	    {
	      WINDOWPLACEMENT w;
	      
	      memset (&w, 0, sizeof w);
	      w.length = sizeof w;
	      GetWindowPlacement (SDL_Window, &w);
	      w.showCmd = SW_RESTORE;
	      SetWindowPlacement (SDL_Window, &w);
	    }
#endif
	  }
#endif
	  SetWindowPos(SDL_Window, NULL, x, y + caption_height, width, height,
		       (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	}
    }
}
