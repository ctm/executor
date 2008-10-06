#include <stdlib.h>
#include <stdio.h>
#include "disasm.h"
#include "codeseg.h"
#include "label.h"


int
main (int argc, char *argv[])
{
  codeseg_t *seg;
  char *d;
  unsigned long entry_point;

  if (argc != 2)
    {
      fprintf (stderr, "Usage: %s <resource file>\n", argv[0]);
      return EXIT_FAILURE;
    }

  /* Read in the code segments. */
  seg = read_code_segments (argv[1]);
  if (seg == NULL)
    return EXIT_FAILURE;

  d = disasm_code_segments (seg, &entry_point);
  d = add_labels (d, entry_point, FALSE);

  if (d != NULL)
    fputs (d, stdout);
  return EXIT_SUCCESS;
}
