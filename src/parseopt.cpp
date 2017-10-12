/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_parseopt[] =
	    "$Id: parseopt.c 63 2004-12-24 18:19:43Z ctm $";
#endif


#include "rsys/common.h"
#include "Gestalt.h"
#include "rsys/parseopt.h"
#include "rsys/vdriver.h"
#include "rsys/flags.h"
#include "rsys/prefs.h"
#include "rsys/version.h"

#include <ctype.h>
#include <string.h>

using namespace Executor;
using namespace std;

/* Parse version e.g. "executor -system 7.0.2".  Omitted
 * digits will be zero, so "executor -system 7" is equivalent to
 * "executor -system 7.0.0".  Returns TRUE on success, else FALSE.
 */

boolean_t
Executor::ROMlib_parse_version (string vers, uint32 *version_out)
{
  boolean_t success_p;
  int major_version, minor_version, teeny_version;
  char *major_str, *minor_str, *teeny_str;
  char *temp_str, *system_str;

  /* Copy the version to a temp string we can manipulate. */
  system_str = (char *) alloca (vers.length() + 1);
  strcpy (system_str, vers.c_str());

  major_str = system_str;
  
  temp_str = strchr (major_str, '.');
  if (temp_str)
    {
      *temp_str = 0;
      minor_str = &temp_str[1];
    }
  else
    minor_str = "0";
  
  temp_str = strchr (minor_str, '.');
  if (temp_str)
    {
      *temp_str = 0;
      teeny_str = &temp_str[1];
    }
  else
    teeny_str = "0";
  
  major_version = atoi (major_str);
  minor_version = atoi (minor_str);
  teeny_version = atoi (teeny_str);
  
  if (   major_version <= 0 || major_version > 0xF
      || minor_version < 0 || minor_version > 0xF
      || teeny_version < 0 || teeny_version > 0xF)
    success_p = FALSE;
  else
    {
      *version_out = CREATE_SYSTEM_VERSION (major_version, minor_version,
					    teeny_version);
      success_p = TRUE;
    }

  return success_p;
}

/* Parse -system option, e.g. "executor -system 7.0.2".  Omitted
 * digits will be zero, so "executor -system 7" is equivalent to
 * "executor -system 7.0.0".  Returns TRUE on success, else FALSE.
 */
boolean_t
Executor::parse_system_version (string vers)
{
  boolean_t retval;

  retval = ROMlib_parse_version (vers, &system_version);
  if (retval)
    ROMlib_set_system_version (system_version);
  else
    {
      fprintf (stderr, "%s: bad option `-system': invalid version\n",
	       program_name);
    }

  return retval;
}

/* Parse -size option, e.g. "executor -size 640x480".  Returns FALSE
 * on parse error.
 */
boolean_t
Executor::parse_size_opt (string opt, string arg1)
{
  boolean_t success_p;
  int w, h;
  const char *arg = arg1.c_str();
  
  w = h = 0;
  if (arg != NULL)
    {
#if defined (CYGWIN32)
      if (strcasecmp (arg, "maximum") == 0)
	{
	  h = os_maximum_window_height ();
	  w = os_maximum_window_width ();
	}
      else
#else
#warning we should support "-size maximum"
#endif
	{
	  const char *p;
	  for (p = arg; isdigit (*p); p++)
	    w = (10 * w) + (*p - '0');
	  if (*p == 'x')
	    {
	      for (++p; isdigit (*p); p++)
		h = (10 * h) + (*p - '0');
	      if (*p != '\0')
		h = 0;
	    }
	}
    }

  if (w == 0 || h == 0)
    {
      fprintf (stderr, "Invalid screen size.  Use something like "
	       "\"-%s 640x480\".\n", opt.c_str());
      success_p = FALSE;
    }
  else if (w < VDRIVER_MIN_SCREEN_WIDTH || h < VDRIVER_MIN_SCREEN_HEIGHT)
    {
      fprintf (stderr, "Screen size must be at least %dx%d.\n",
	       VDRIVER_MIN_SCREEN_WIDTH, VDRIVER_MIN_SCREEN_HEIGHT);
      success_p = FALSE;
    }
  else
    {
      flag_width = w;
      flag_height = h;
      success_p = TRUE;
    }

  return success_p;
}

/*
 * It's silly for us to do this by hand, but it was real quick to write.
 */

PUBLIC boolean_t
Executor::parse_prres_opt (INTEGER *outx, INTEGER *outy, string arg1)
{
  boolean_t retval;
  INTEGER x, y, *p;
  const char *arg = arg1.c_str();

  x = 0;
  y = 0;
  p = &x;

  for (retval = TRUE; *arg && retval; ++arg)
    {
      switch (*arg)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  {
	    int old_val;
	    int digit;

	    old_val = *p;
	    digit = *arg - '0';
	    *p = 10 * *p + digit;
	    if (*p <= old_val)
	      retval = FALSE; /* overflow */
	  }
	  break;
	case 'x':
	  if (p == &x)
	    p = &y;
	  else
	    retval = FALSE; /* extra x */ 
	  break;
	default:
	  retval = FALSE; /* unknown character */
	  break;
	}
    }

  if (x <= 0 || y <= 0)
    retval = FALSE;
 
  if (retval)
    {
      *outx = x;
      *outy = y;
    }
  
  return retval;
}
