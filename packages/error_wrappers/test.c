#include <stdio.h>

#include "error.h"

int main(void)
{
  struct foo **h;

  h = NewHandle (100);

  HANDLE_CHECK (h);
  DisposHandle (h);
  
  if (1)
    ERROR_CHECK_INHIBIT();
    fprintf (stderr, "neato\n");
    ERROR_CHECK_RESTORE();
  else
    fprintf (stderr, "ick\n");

  return 0;
}
