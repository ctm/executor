/* THIS CODE IS USELESS until I find a way to use spaces in Makefile
 * variables.  It seemed like a good idea at the time and I'm not deleting
 * it now because I'm a packrat. --Cliff
 */


/*
 * This program combines with bash's IFS variable to provide smarter
 * word splitting than bash normally provides.
 *
 * cflags='-Dthis -Dthat="argument with spaces"'
 * can cause problems when ${cflags} if expanded and word split and
 * becomes these four arguments:
 * -Dthis
 * -Dthat="argument
 * with
 * spaces"
 *
 * So instead use:
 * saveifs="$IFS"
 * cflags=`smartsplit "${cflags}"`
 * IFS=`smartsplit -ifs "${cflags}"`
 * ${cflags}
 * IFS="$saveifs"
 *
 * Ugly, but it's the best thing I thought of in order to allow me to have
 * spaces in my configuration file.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*
 * returns 1 if successful, 0 otherwise.  NOTE: In theory, any character
 * that's not currently in the string could be a separator.  However,
 * empirical evidence suggests that at least under bash 1.14.7.1, control-a
 * will not work as a separator.  So, we try a few specific separators by
 * hand, then try any printable character, then try control-b, then give
 * up.
 */

static int
findsep (const char *str, char *ifsp)
{
  int retval;
  static char nice_separators[] = "@:+";

  retval = 0;

  if (!retval)
    {
      const char *nicep;
      
      for (nicep = nice_separators; !retval && *nicep; ++nicep)
	{
	  if (!strchr (str, *nicep))
	    {
	      *ifsp = *nicep;
	      retval = 1;
	    }
	}
    }

  if (!retval)
    {
      int c;

      for (c = 0; !retval && c < 255; ++c)
	{
	  if (isprint (c))
	    {
	      if (!strchr (str, c))
		{
		  *ifsp = c;
		  retval = 1;
		}
	    }
	}
    }

  if (!retval)
    if (!strchr (str, 2))
      {
	*ifsp = 2;
	retval = 1;
      }
  return retval;
}

static int
smartsplit (const char *str, char *ifsp, char **strpp)
{
  int retval;

  retval = findsep (str, ifsp);
  if (retval && strpp)
    {
      const char *ip;
      char *op;
      char current_quote;
      
      *strpp = malloc (strlen (str) + 1);
      current_quote = 0;
      for (ip = str, op = *strpp; *ip; ++ip)
	{
	  if (current_quote)
	    {
	      *op++ = *ip;
	      if (*ip == current_quote)
		current_quote = 0;
	    }
	  else
	    {
	      if (*ip == '\'' || *ip == '"')
		{
		  *op++ = *ip;
		  current_quote = *ip;
		}
	      else if (isspace ((unsigned char) *ip))
		*op++ = *ifsp;
	      else
		*op++ = *ip;
	    }
	}
      *op = 0;
  }
  
  return retval;
}

static void
output_prog_name (const char *prog)
{
  const char *lastslash;

  lastslash = strrchr (prog, '/');
  fputs (lastslash ? lastslash + 1 : prog, stderr);
}

static void
usage (const char *prog)
{
  fputs ("Usage: ", stderr);
  output_prog_name (prog);
  fputs (" [-ifs] string", stderr);
}

int
main (int argc, char *argv[])
{
  int retval;

  retval = 0;
  switch (argc)
    {
    case 2:
      {
	int success;
	char ifs;
	char *str;

	success = smartsplit (argv[1], &ifs, &str);
	if (success)
	  puts (str);
	else
	  {
	    output_prog_name (argv[0]);
	    fprintf (stderr, ": all separators used\n");
	    retval = 1;
	  }
      }
      break;
    case 3:
      if (strcmp (argv[1], "-ifs") == 0)
	{
	  char ifs;

	  smartsplit (argv[2], &ifs, 0);
	  putchar (ifs);
	}
      else
	{
	  usage (argv[0]);
	  retval = 1;
	}
      break;
    default:
      usage (argv[0]);
      retval = 2;
      break;
    }
  return retval;
}
