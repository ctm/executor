#include <windows.h>
#include <stdio.h>

/*
 * Special purpose program to register the directory that gsdll32.dll 
 * winds up in.  This is so that we don't have to build a full-blown
 * InstallShield installer just to get the key registered.
 *
 * make setkey 'CC=i486-pc-cygwin32-gcc -windows' CFLAGS='-O -Wall' LDLIBS=-ladvapi32 
 */

#define GS_DLL "gsdll32.dll"

#define GS_KEY \
  "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\" GS_DLL

int STDCALL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
  char buf[1024], *p;
  DWORD len;
  int retval;
  
  len = GetCurrentDirectory (sizeof buf, buf);
  if (len < sizeof buf)
    p = buf;
  else
    {
      p = alloca (len + 1);
      len = GetCurrentDirectory (len + 1, p);
    }

  if (len <= 0)
    retval = 1;
  else
    {
      HKEY key;
      DWORD disp;

      if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, GS_KEY, 0, NULL,
			  REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
			  &key, &disp) != ERROR_SUCCESS)
	retval = 1;
      else
	{
	  char *newbufp;
	  DWORD newlen;

	  newbufp = alloca (strlen (p) + 1 + sizeof GS_DLL);
	  newlen = sprintf (newbufp, "%s\\%s", p, GS_DLL) + 1;
	  retval = (RegSetValueEx (key, "", 0, REG_SZ,
				   (LPBYTE) newbufp, newlen) == ERROR_SUCCESS);
	  RegCloseKey (key);
	}
    }
  return retval;
}
