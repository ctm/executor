#include "codeseg.h"
#include "disasm.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static unsigned char *find_ref_list (unsigned char *map, const char *type,
				     int *num_resources);

/* Returns a linked list of the code segments in the specified file, or
 * NULL if none were found.
 */
codeseg_t *
read_code_segments (const char *filename)
{
  FILE *rsrc_fp;
  unsigned char *rsrc, *orig_rsrc, *ref_list;
  unsigned long file_size;
  int i, num_code_resources;
  codeseg_t *prev;

  rsrc_fp = fopen (filename, "rb");
  if (rsrc_fp == NULL)
    {
      perror (filename);
      return NULL;
    }

  /* Compute the file's size. */
  fseek (rsrc_fp, 0, SEEK_END);
  file_size = ftell (rsrc_fp);
  fseek (rsrc_fp, 0, SEEK_SET);

  /* Read the entire resource file into core. */
  rsrc = orig_rsrc = (unsigned char *)malloc (file_size);
  if (fread (rsrc, 1, file_size, rsrc_fp) != file_size)
    {
      perror (filename);
      fclose (rsrc_fp);
      free (orig_rsrc);
      return NULL;
    }
  fclose (rsrc_fp);

  /* First, skip 512 bytes to skip over extra AppleDouble space. */
  rsrc += 512;

  if (read_long (rsrc) != 256)
    {
      fprintf (stderr, "%s: not a resource file (data offset != 256).\n",
	       filename);
      free (orig_rsrc);
      return NULL;
    }

  ref_list = find_ref_list (rsrc + read_long (rsrc + 4), "CODE",
			    &num_code_resources);
  if (ref_list == NULL || num_code_resources == 0)
    {
      fprintf (stderr, "%s: no code resources found.\n", filename);
      free (orig_rsrc);
      return NULL;
    }

  /* Read in all of the code resources. */
  for (i = 0, prev = NULL; i < num_code_resources; i++)
    {
      codeseg_t *c;
      unsigned char *data_start;

      c = (codeseg_t *)malloc (sizeof *c);
      memset (c, 0, sizeof *c);
      c->segment = read_short (ref_list);
      data_start = rsrc + 256 + (read_long (ref_list + 4) & 0xFFFFFF);
      c->num_code_bytes = read_long (data_start);
      c->code = (unsigned char *)malloc (c->num_code_bytes);
      if (c->code == NULL)
	{
	  fprintf (stderr, "Tried to malloc 0x%lX bytes and failed.\n",
		   (unsigned long)c->num_code_bytes);
	  free (orig_rsrc);
	  return NULL;
	}
      memcpy (c->code, data_start + 4, c->num_code_bytes);
      c->next = prev;
      prev = c;

      /* Move on to next ref_list entry. */
      ref_list += 12;
    }

  free (orig_rsrc);

  return prev;
}


static unsigned char *
find_ref_list (unsigned char *map, const char *type, int *num_resources)
{
  int i, num_types;
  unsigned char *type_list;

  *num_resources = 0;  /* Default. */

  type_list = map + 28;
  num_types = read_short (type_list) + 1;
  for (i = 0, type_list += 2; i < num_types; i++)
    {
      if (!memcmp (type_list, type, 4))
	{
	  *num_resources = read_short (type_list + 4) + 1;
	  return map + 28 + read_short (type_list + 6);
	}

      /* Move on to next type list entry. */
      type_list += 8;
    }

  return NULL;
}


long
read_long (const unsigned char *p)
{
  return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

short
read_short (const unsigned char *p)
{
  return (p[0] << 8) | p[1];
}
