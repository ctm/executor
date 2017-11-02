/* Copyright 1998 - 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_win_clip[] = "$Id: win_clip.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <rsys/scrap.h>
#include <rsys/error.h>

#include "SDL/SDL.h"
#include "SDL_bmp.h"
#include "SDL/SDL_syswm.h"

#include "win_win.h"

#define FORMAT_PREFIX "Executor_0x"

PUBLIC unsigned long
ROMlib_executor_format (LONGINT type)
{
  char *str;
  UINT retval;
  static struct
  {
    LONGINT type;
    UINT value;
    bool valid;
  } cache;

  if (cache.valid && type == cache.type)
    retval = cache.value;
  else
    {
      str = alloca (sizeof FORMAT_PREFIX + 8);
      sprintf (str, "%s%08lx", FORMAT_PREFIX, (unsigned long) type);
      retval = RegisterClipboardFormat (str);
      cache.type = type;
      cache.value = retval;
      cache.valid = true;
    }
  return retval;
}

PUBLIC HWND
cygwin_sdlwindow (void)
{
  HWND retval;
  SDL_SysWMinfo info;
  int success; 

  memset (&info, 0, sizeof info);
  info.version.major = SDL_MAJOR_VERSION;
  info.version.minor = SDL_MINOR_VERSION;
  success = SDL_GetWMInfo (&info); /* NOTE: 1 is success, -1 is failure */
  retval = (success == 1) ? info.window : NULL;
  return retval;
}

PUBLIC LONGINT
GetScrapX (LONGINT type, char **h)
{
  UINT format;
  LONGINT retval;

  retval = -1;
  switch (type)
    {
    case T ('T', 'E', 'X', 'T'):
      format = CF_TEXT;
      break;
    default:
      format = ROMlib_executor_format (type);
      if (type == T('P','I','C','T'))
	{
	  typeof (format) newval;
	  UINT formats[2] = { format, CF_DIB };
	  
	  newval = GetPriorityClipboardFormat (formats, NELEM (formats));
	  if (newval != 0 && newval != (UINT) -1)
	    format = newval;
 	  }
      break;
    }
  if (IsClipboardFormatAvailable (format) && OpenClipboard (cygwin_sdlwindow ()))
    {
      HANDLE data;

      data = GetClipboardData (format);
      if (data)
	{
	  LPVOID lp;

	  lp = GlobalLock (data);
	  switch (type)
	    {
	    case T ('T', 'E', 'X', 'T'):
	      {
		int len;

		len = strlen (lp);
		retval = get_scrap_helper (h, lp, len, true);
	      }
	    break;
	    default:
	      {
#if defined (SDL)
		if (format == CF_DIB)
		  retval = get_scrap_helper_dib (h, lp);
		else
#endif
		  {
		    int32 len;
		    len = *(int32 *)lp;

		    retval = get_scrap_helper (h, lp+sizeof(int32),
					       len, false);
		  }
	      }
	      break;
	    }
	  GlobalUnlock (data);
	  CloseClipboard ();
	}
    }
  return retval;
}

PRIVATE int
calc_length_and_format (UINT *formatp, LONGINT type, LONGINT length,
			const char *p)
{
  int retval;

  switch (type)
    {
    case T ('T', 'E', 'X', 'T'):
      retval = length + count_char (p, length, '\r') + 1;
      *formatp = CF_TEXT;
      break;
    default:
      retval = length+4;
      *formatp = ROMlib_executor_format (type);
      break;
    }
  return retval;
}

PRIVATE void
fill_in_data (char *destp, LONGINT type, LONGINT length, const char *p)
{
  switch (type)
    {
    case T ('T', 'E', 'X', 'T'):
      while (--length >= 0)
	{
	  char c;
	  
	  c = *p++;
	  *destp++ = c;
	  if (c == '\r')
	    *destp++ = '\n';
	}
      *destp++ = 0;
      break;
    default:
      *(int32 *)destp = length;
      memcpy (destp+sizeof(int32), p, length);
      break;
    }
}

PRIVATE HANDLE clip_data = NULL; /* to hold a PICT that may need conversion
				    to a CF_DIB */

PUBLIC void
PutScrapX (LONGINT type, LONGINT length, char *p, int scrap_count)
{
  static int old_count = -1;

  if (OpenClipboard (cygwin_sdlwindow ()) &&
      (scrap_count == old_count || EmptyClipboard ()))
    {
      UINT format;
      int new_length;
      HANDLE data;

      new_length = calc_length_and_format (&format, type, length, p);
      data = GlobalAlloc (GMEM_MOVEABLE|GMEM_DDESHARE, new_length);
      if (type == TICK ("PICT"))
	{
	  if (clip_data)
	    LocalFree (clip_data);
	  clip_data = LocalAlloc (LMEM_FIXED, new_length);
	}
      if (data)
	{
	  char *destp;

	  destp = GlobalLock (data);
	  fill_in_data (destp, type, length, p);
	  GlobalUnlock (data);
	  SetClipboardData (format, data);
	  if (type == TICK ("PICT"))
	    {
	      fill_in_data ((char *) clip_data, type, length, p);
	      SetClipboardData (CF_DIB, NULL); /* we can create a DIB if
						    asked to do so */
	    }
	  CloseClipboard ();
	  old_count = scrap_count;
	}
    }
}

PUBLIC bool
we_lost_clipboard (void)
{
  bool retval;

  retval = GetClipboardOwner () != cygwin_sdlwindow ();
  return retval;
}

PUBLIC void
write_pict_as_dib_to_clipboard (void)
{
  if (clip_data)
    put_scrap_helper_dib ((LPVOID) clip_data);
}

PUBLIC void
write_pict_as_pict_to_clipboard (void)
{
  if (clip_data)
    {
      size_t len;
      HGLOBAL hg;

      len = LocalSize (clip_data);
      hg = GlobalAlloc (GMEM_DDESHARE, len);
      if (hg)
	{
	  LPVOID lp;

	  lp = GlobalLock (hg);
	  memcpy (lp, clip_data, len);
	  GlobalUnlock (hg);
	  SetClipboardData (ROMlib_executor_format (T('P','I','C','T')), hg);
	}
    }
}

void
write_surfp_to_clipboard (SDL_Surface *surfp)
{
  char *bytesp;
  size_t len;

  if (SDL_SaveCF_DIB (surfp, &bytesp, &len) == 0)
    {
      HGLOBAL hg;

      hg = GlobalAlloc (GMEM_DDESHARE, len);
      if (hg)
	{
	  void *lp;

	  lp = GlobalLock (hg);
	  memcpy (lp, bytesp, len);
	  GlobalUnlock (hg);
	  SetClipboardData (CF_DIB, hg);
	}
      free (bytesp);
    }
}
