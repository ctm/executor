/* Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_cleanup[] =
		"$Id: cleanup.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include <rsys/common.h>
#include <rsys/cleanup.h>
#include <rsys/file.h>
#include <rsys/notmac.h>

#include <stdarg.h>

#if defined (MSDOS) || defined (CYGWIN32)

#if defined(CYGWIN32)
#include <windows.h>
#endif

#if defined (MSDOS)
#include <process.h>
#endif

PUBLIC void
add_to_cleanup (const char *s, ...)
{
  char *batch_file;
  FILE *fp;
  struct stat sbuf;

  batch_file = copystr (CLEANUP_BATCH_FILE_NAME);
  if (stat (batch_file, &sbuf) == 0)
    fp = fopen (batch_file, "a");
  else
    {
      fp = fopen (batch_file, "w");
      if (fp)
	{
	  fprintf (fp, "@echo off\n");
	  if (ROMlib_start_drive)
	    fprintf (fp, "%c:\n", ROMlib_start_drive);
	}
    }
  free (batch_file);
  if (fp)
    {
      va_list ap;

      va_start (ap, s);
      vfprintf (fp, s, ap);
      fclose (fp);
    }
}

PUBLIC void
call_cleanup_bat (void)
{
  char *batch_file;
  struct stat sbuf;

  batch_file = copystr (CLEANUP_BATCH_FILE_NAME);
  if (stat (batch_file, &sbuf) == 0)
    {
      add_to_cleanup ("del \"%s\"\n", batch_file);
#if defined(MSDOS)
      spawnl (P_OVERLAY, batch_file, batch_file, 0);
#else
      {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	memset (&si, 0, sizeof si);
	si.cb = sizeof si;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	CreateProcess (batch_file, NULL, NULL, NULL, FALSE, 0, NULL, NULL,
		       &si, &pi);
      }
#endif
    }
}

#endif
