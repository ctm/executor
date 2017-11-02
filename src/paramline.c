/* Copyright 1988 - 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_paramline[] =
		"$Id: paramline.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "paramline.h"

typedef enum { false, true } bool;

int
count_params (const char *p, int n)
{
  int retval;
  char literal;
  bool reading_whitespace_p;

  retval = 0;
  reading_whitespace_p = true;
  literal = 0;
  while (n-- > 0)
    {
      if (!literal && isspace (*p))
	{
	  reading_whitespace_p = true;
	}
      else if (*p == literal)
	{
	  literal = 0;
	}
      else
	{
	  switch (*p)
	    {
	    case '\'':
	    case '"':
	      if (!literal)
		literal = *p;
	      /* FALL THROUGH */
	    default:
	      if (reading_whitespace_p)
		++retval;
	      reading_whitespace_p = false;
	      break;
	    }
	}
      ++p;
    }
  return retval;
}

char *
get_param (const char **bufpp, int *nleftp)
{
  char *retval;
  const char *bufp;
  int nleft;

  retval = 0;
  
  nleft = *nleftp;
  bufp = *bufpp;
  while (nleft > 0 && isspace (*bufp))
    {
      ++bufp;
      --nleft;
    }
  
  if (nleft > 0)
    {
      char literal;
      char *string;
      bool done;

      string = "";
      literal = 0;
      done = false;
      while (nleft > 0 && !done)
	{
	  if (*bufp == literal)
	    literal = 0;
	  else if (!literal && isspace (*bufp))
	    done = true;
	  else
	    switch (*bufp)
	      {
	      case '\'':
	      case '"':
		if (!literal)
		  {
		    literal = *bufp;
		    break;
		  }
		/* FALL THROUGH */
	      default:
		{
		  char *oldstring;

		  oldstring = string;
		  string = alloca (strlen (oldstring) + 1 + 1);
		  sprintf (string, "%s%c", oldstring, *bufp);
		}
	      }
	  ++bufp;
	  --nleft;
	}
      retval = malloc (strlen (string) + 1);
      strcpy (retval, string);
    }

  *bufpp = bufp;
  *nleftp = nleft;

  return retval;
}

void
paramline_to_argcv (const char *cmdp, int *argcp, char ***argvp)
{
  int len;
  int argc;
  char **argv;

  len = strlen (cmdp);
  argc = count_params (cmdp, len);
  argv = malloc((argc+1) * sizeof *argv);
  if (!argv)
    argc = -1;
  else
    {
      int i;

      for (i = 0; i < argc; ++i)
	argv[i] = get_param (&cmdp, &len);
      argv[i] = 0;

      *argcp = argc;
      *argvp = argv;
    }
}
