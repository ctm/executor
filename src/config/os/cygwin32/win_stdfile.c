/* Copyright 2005 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_win_stdfile[] = "$Id: win_stdfile.c 119 2005-07-11 21:36:20Z ctm $";
#endif

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"
#include "rsys/error.h"

#include <windows.h>

#include "win_win.h"

PUBLIC boolean_t host_has_spfcommon (void)
{
  return true;
}

PUBLIC boolean_t
host_spfcommon (host_spf_reply_block *replyp, const char *prompt,
		const char *incoming_filename, void *fp, void *filef, int numt,
		void *tl, getorput_t getorput, sf_flavor_t flavor,
		void *activeList, void *activateproc, void *yourdatap)
{
  OPENFILENAME ofn;
  TCHAR filename[2000];
  BOOL ret;

  extern HWND SDL_Window;

  memset (&ofn, 0, sizeof ofn);
  ofn.lStructSize = sizeof ofn;
  ofn.hwndOwner = SDL_Window;
  filename[0] = 0;
  ofn.lpstrFile = filename;
  ofn.nMaxFile = sizeof (filename);
  if (getorput == put)
    ret = GetSaveFileName (&ofn);
  else
    ret = GetOpenFileName (&ofn);

  return false;
}
