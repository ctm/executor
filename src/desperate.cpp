/* Copyright 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_desperate[] =
	    "$Id: desperate.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/file.h"
#include "rsys/desperate.h"
#include "OSUtil.h"

using namespace Executor;

static int
desperate_switch_index (int argc, char **argv)
{
  int i;
  for (i = 0; i < argc; i++)
    if (!strcmp ("-desperate", argv[i]))
      return i;
  return -1;
}


static void
print_cmd_line_arg (const char *str)
{
  char *c;

  c = (char*)alloca (strlen (str) + 1);
  strcpy (c, str);
#if defined (MSDOS)
  convert_slashs_to_backslashs (c);
#endif

  if (c[0] == '\0' || strchr (c, ' ') || strchr (c, '\t'))
    printf (" \"%s\"", c);
  else
    printf (" %s", c);
}


/* FIXME - these should be moved to configuration files somehow. */
#if defined (MSDOS)
static const char *desperate_switches[] = {
  "-vga",
  "-oldtimer",
  "-nosound",
  "-skipaspi",
  "-nofilescheck",
  "-noautorefresh",
  "-memory",
  "2M",
  "-macdrives",
  "",
  "-dosdrives",
  ""
};
#elif defined (LINUX)
static const char *desperate_switches[] = {
  "-nosound",
  "-nodrivesearch",
  "-memory",
  "2M",
};
#else
static const char *desperate_switches[] = {
  "-nosound",
  "-memory",
  "2M",
};
#endif



/* Searches for an occurrence of "-desperate" in the command line.
 * If it finds one, it is replaced with a list of command-line
 * switches whose purpose is to put Executor into "desperate" mode,
 * where it uses as few system features as possible.  If such
 * a switch is found, *argcp and *argvp are modified to reflect
 * the new command line switches.
 *
 * Returns true except in case of a parsing error (right now that
 * can only happen with duplicate "-desperate" switches).
 */
bool
Executor::handle_desperate_switch (int *argcp, char ***argvp)
{
  bool success_p;
  char **argv, **new_argv;
  int i, s, argc, new_argc;

  argc = *argcp;
  argv = *argvp;

  /* See if the "-desperate" switch is in the list. */
  s = desperate_switch_index (argc, argv);
  if (s == -1)
    {
      success_p = true;
      goto done;
    }

  /* See if the "-desperate" switch is in the list twice. */
  if (desperate_switch_index (argc - s - 1, argv + s + 1) != -1)
    {
      fprintf (stderr, "Duplicate \"-desperate\" switch.\n");
      success_p = false;
      goto done;
    }

  /* Construct the new list by replacing the "-desperate" switch
   * with all of the switches it implies.  There is a small memory
   * leak here since we'll never reclaim this memory.
   */
  new_argc = argc + NELEM (desperate_switches) - 1;
  new_argv = (char**)malloc ((new_argc + 1) * sizeof new_argv[0]);
  memcpy (new_argv, argv, s * sizeof new_argv[0]);
  memcpy (new_argv + s, desperate_switches, sizeof desperate_switches);
  memcpy (new_argv + s + NELEM (desperate_switches),
	  argv + s + 1, (argc - s - 1) * sizeof new_argv[0]);
  new_argv[new_argc] = NULL;

  puts (
"Executor is now entering desperation mode and attempting to use as few\n"
"system features as possible.  This is useful if you find that Executor\n"
"won't start for you, a problem usually caused by driver conflicts.  If\n"
"you find that this \"desperation mode\" works, you can experiment and\n"
"see which of these command-line switches are necessary for you to get\n"
"Executor to work:\n"
);
  for (i = 0; i < new_argc; i++)
    print_cmd_line_arg (new_argv[i]);

  fputs ("\n\nWaiting ten seconds...", stdout);
  fflush (stdout);
#if !defined (CYGWIN32)
  sleep (10);
#else
  Delay (60 * 10, (LONGINT *) 0);
#endif
  puts ("done.");
  fflush (stdout);

  *argcp = new_argc;
  *argvp = new_argv;
  success_p = true;

 done:
  return success_p;
}
