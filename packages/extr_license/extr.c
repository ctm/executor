#include <stdio.h>
#include <stdlib.h>
/* #include "target-os-config.h" fails */
#include "/ardi/executor/src/include/rsys/licensetext.h"

#if defined (MSDOS) || defined (CYGWIN32)
#define CR "\r\n"
#else
#define CR "\n"
#endif

static char *
cr_cvt (const char *str)
{
  char *retval;
  const char *p;
  char *op;
  int length;

  length = 1; /* for NUL at end */
  for (p = str; *p; ++p)
    length += *p == '\r' ? sizeof CR - 1 : 1;

  retval = malloc (length);
  for (p = str, op = retval; *p; ++p)
    if (*p != '\r')
      *op++ = *p;
    else
      {
	memcpy (op, CR, sizeof CR - 1);
	op += sizeof CR - 1;
      }
  *op = 0;
  return retval;
}


int
main (int argc, char *argv[])
{
  int i;
  int retval;

  for (i = 0; ROMlib_license[i].body; ++i)
    {
      if (ROMlib_license[i].heading)
	printf ("%s" CR CR, cr_cvt (ROMlib_license[i].heading));
      printf ("%s" CR, cr_cvt (ROMlib_license[i].body));
      if (ROMlib_license[i+1].body)
	printf (CR CR);
    }
  retval = 0;
  return retval;
}
