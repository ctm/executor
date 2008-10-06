/* Copyright 1998, 1999 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_win_cookie[] = "$Id: win_cookie.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"
#include "rsys/cookie.h"
#include "rsys/custom.h"

#include <windows.h>
#include <ctype.h>

#if defined (TIME_OUT_DAYS)

#define COOKIE_KEY_ROOT	"SOFTWARE\\ARDI\\xp"

PRIVATE const char *
create_cookie_key (void)
{
  char *retval;
  int length;

  length = strlen (COOKIE_KEY_ROOT) + 1;

  if (ROMlib_mac_demo_idp)
    length += 1 + strlen (ROMlib_mac_demo_idp->chars);
  retval = malloc (length);
  if (retval)
    {
      strcpy (retval, COOKIE_KEY_ROOT);
      if (ROMlib_mac_demo_idp)
	{
	  char *ip, *op;

	  /* make sure there's just one \ separator and that we always use
	     lower case in the key */

	  for (ip = ROMlib_mac_demo_idp->chars; *ip == '\\'; ++ip)
	    ;
	  for (op = retval + strlen (retval), *op++ = '\\'; *ip; ++ip)
	    *op++ = tolower (*ip);
	  *op = 0;
	}
    }
  return retval;
}

PRIVATE void
destroy_cookie_key (const char *cookie_key)
{
  free ((char *) cookie_key);
}

PUBLIC int
win_retrieve_cookie (cookie_t *cookiep)
{
  int retval;
  HKEY key;
  const char *cookie_key;

  retval = 0;
  cookie_key = create_cookie_key ();
  if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,	cookie_key, 0, KEY_READ, &key)
      == ERROR_SUCCESS)
    {
      DWORD val_len;

      val_len = sizeof *cookiep;
      retval = (RegQueryValueEx (key, "xp", NULL, NULL, (LPBYTE) cookiep,
				 &val_len) == ERROR_SUCCESS);
      RegCloseKey (key);
    }
  destroy_cookie_key (cookie_key);
  return retval;
}

PUBLIC int
win_leave_cookie (const cookie_t *cookiep)
{
  int retval;
  HKEY key;
  DWORD disp;
  const char *cookie_key;

  retval = 0;
  cookie_key = create_cookie_key ();
  if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, cookie_key, 0, NULL,
		      REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
		      &key, &disp) == ERROR_SUCCESS)
    {
      DWORD val_len;

      val_len = sizeof *cookiep;
      retval = (RegSetValueEx (key, "xp", 0, REG_BINARY, (LPBYTE) cookiep,
			       sizeof *cookiep) == ERROR_SUCCESS);
      RegCloseKey (key);
    }
  destroy_cookie_key (cookie_key);
  return retval;
}

#endif
