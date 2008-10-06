#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* if Lava software wants to use this, we should probably make a quiet
   option */

/*
 * Special purpose program to delete the cookie that Executor Demo uses
 * to determine whether or not the demo has been installed previously.
 *
 * make delkey 'CC=i486-pc-cygwin32-gcc -windows' CFLAGS='-O -Wall' LDLIBS=-ladvapi32 
 */

#define ROOT "SOFTWARE\\ARDI\\xp"
#define PREFIX "del_"

static char *
get_key_from_command_line (void)
{
  char *line;
  char *retval;

  retval = NULL;
  line = GetCommandLine ();
  if (line)
    {
      char *lower_line;
      char *ip, *op;
      char *old_del;

      lower_line = malloc (strlen (line) + 1);
      if (lower_line)
	{
	  for (ip = line, op = lower_line; *ip; ++ip, ++op)
	    *op++ = tolower (*ip++);
	  *op = 0;

	  old_del = strstr (line, PREFIX);
	  if (old_del)
	    {
	      char *new_del;
	      char *p;

	      do
		{
		  new_del = strstr (old_del + 1, PREFIX);
		  if (new_del)
		    old_del = new_del;
		}
	      while (new_del);
	      old_del += sizeof PREFIX - 1;
	      retval = malloc (strlen (ROOT) + 1 + strlen (old_del) + 1);
	      if (retval)
		{
		  sprintf (retval, "%s\\%s", ROOT, old_del);
		  for (p = retval; *p; ++p)
		    if (isspace (*p) || *p == '.')
		      *p = 0;
		}
	    }
	  free (lower_line);
	}
    }
  return retval;
}

static void
message (const char *mess)
{
  MessageBox (NULL, mess, "delkey", MB_OK);
}

int STDCALL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
  char *key;
  int retval;
  LONG del_success;

  key = get_key_from_command_line ();
  if (!key)
    {
      message ("key not found");
      retval = ERROR_BAD_ARGUMENTS;
    }
  else
    {
      del_success = RegDeleteKey (HKEY_LOCAL_MACHINE, key);
      retval = del_success;
      if (del_success == ERROR_SUCCESS)
	message ("cookie removed");
      else
	message ("unable to remove cookie");
      free (key);
    }

  return retval;
}
