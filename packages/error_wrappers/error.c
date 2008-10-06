/*
 * This file contains definition for the global variables
 * that we use to help track down errors, as well as the definition for all
 * the stub helper routines which we need.
 */

#define __COMPILING_ERROR_C__
#include "error.h"

/* error.h has comments for each of these global variables */

int die_on_error = 0;
long int last_error;
const char *last_error_file;
int last_error_line;

/* Definition for each routine we monitor.  If the routine doesn't
   return a value, it can be error checked inside error.h.  If it does
   return a value then the stub has to hold onto that value, then
   check for the error, then return that value.  Since we always want
   to know the filename and line number where the error was found, we
   have to pass that information into our stub below and then hand it
   to ERROR_CHECK, since it wouldn't be of any use if we logged
   "error.c" as the file that contained the error. */


void _DisposHandle (Handle h)
{
  DisposHandle (h);
}

Handle _NewHandle (Size s, const char *file, int line)
{
  Handle retval;

  retval = NewHandle (s);
  ERROR_CHECK (MemError (), file, line);
  return retval;
}
