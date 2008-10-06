/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_map_to_c[] =
		"$Id: map_to_c.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include <ctype.h>

#include "rsys/image.h"

/* consume input from o_fp, produce output to o_fp */
FILE *i_fp, *o_fp;
char *in_file, *out_file;

char *programname;

int lineno = 1;

pixel_image_desc_t image_desc;

int orig_style_p;
int parsing_p;

struct rgb_color
{
  int16 red, green, blue;
} colors[256];

int n_colors;

void
fatal_error (char *message)
{
  if (parsing_p)
    fprintf (stderr, "%s:%d; %s\n", in_file, lineno, message);
  else
    fprintf (stderr, "fatal error; %s\n", message);
  exit (EXIT_FAILURE);
}

void *
xmalloc (int size)
{
  void *p;

  p = malloc (size);
  if (!p)
    fatal_error ("virtual memory exhuasted");
  return p;
}

void *
xrealloc (void *p, int new_size)
{
  p = realloc (p, new_size);
  if (!p)
    fatal_error ("virtual memory exhuasted");
  return p;
}

void
emit_prologue (char *map_name, int depth)
{
  fprintf (o_fp, "char %s_%d_bits[] =\n", map_name, depth);
  fprintf (o_fp, "{\n");
}

void
emit_row_start ()
{
  fprintf (o_fp, "  ");
}

void
emit_row_end ()
{
  fprintf (o_fp, "\n");
}

void
emit_char (unsigned char x)
{
  fprintf (o_fp, "%u, ", (unsigned int) x);
}

Rect bounds;

/* max possible number of `bits' */
image_bits_desc_t bits[5];

/* number of bits */
int n_bits;

void
emit_epilogue (char *map_name, int row_bytes, int bpp)
{
  /* end bits */
  fprintf (o_fp, "};\n");

  bits[n_bits].row_bytes = row_bytes;
  bits[n_bits].bpp = bpp;
  n_bits ++;
}

/* emit the description structure and initialize function */

void
emit_rest (char *map_name)
{
  int i;
  
  if (orig_style_p)
    {
      for (i = 0; i < n_bits; i ++)
	{
	  fprintf (o_fp, "BitMap %s_%d =\n", map_name, bits[i].bpp);
	  fprintf (o_fp, "{\n");
	  fprintf (o_fp, "  NULL,");
	  fprintf (o_fp, "  CWC (%d),\n", bits[i].row_bytes);
	  fprintf (o_fp, "  { CWC (%d), CWC (%d), CWC (%d), CWC (%d) }\n",
		   bounds.top, bounds.left,
		   bounds.bottom, bounds.right);
	  fprintf (o_fp, "};\n");
	}
    }
  else
    {
      fprintf (o_fp, "pixel_image_desc_t %s_desc =\n",
	       map_name);
      fprintf (o_fp, "{\n");
      fprintf (o_fp, "  { %d, %d, %d, %d },\n",
	       bounds.top, bounds.left,
	       bounds.bottom, bounds.right);
      fprintf (o_fp, "  {\n");
      for (i = 0; i < n_bits; i ++)
	{
	  fprintf (o_fp, "    {\n");
	  fprintf (o_fp, "      %s_%d_bits,\n", map_name, bits[i].bpp);
	  fprintf (o_fp, "      %d,\n", bits[i].row_bytes);
	  fprintf (o_fp, "      %d,\n", bits[i].bpp);
	  fprintf (o_fp, "    },\n");
	}
      fprintf (o_fp, "  }\n");
      fprintf (o_fp, "};\n");
      
      fprintf (o_fp, "pixel_image_t *%s;\n", map_name);
      fprintf (o_fp, "void\n");
      fprintf (o_fp, "image_%s_init ()\n", map_name);
      fprintf (o_fp, "{\n");  
      fprintf (o_fp, "  %s = image_init (&%s_desc);\n",
	       map_name, map_name);
      if (n_colors)
	{
	  int color_i;
	  
	  fprintf (o_fp, "  {\n");
	  fprintf (o_fp, "    static const RGBColor colors[] =\n");
	  fprintf (o_fp, "      {\n");
	  for (color_i = 0; color_i < n_colors; color_i ++)
	    {
	      struct rgb_color *color = &colors[color_i];
	      
	      fprintf (o_fp, "        { %d, %d, %d },\n",
		       CW (color->red),
		       CW (color->green),
		       CW (color->blue));
	    }
	  fprintf (o_fp, "      };\n");
	  fprintf (o_fp, "    image_update_ctab (%s, colors, %d);\n",
		   map_name, n_colors - 1);
	  fprintf (o_fp, "  }\n");
	}
      fprintf (o_fp, "}\n");
    }
}

char
read_char_internal ()
{
  char x;
  
  x = fgetc (i_fp);
  if (feof (i_fp))
    fatal_error ("unexpected end of file");
  
  return x;
}

void
read_white_space ()
{
  char x;

  for (;;)
    {
      x = read_char_internal ();
      if (x == '\n')
	lineno ++;
      else if (! isspace (x))
	{
	  ungetc (x, i_fp);
	  return;
	}
    }
}

int
read_char (char char_to_read)
{
  read_white_space ();

  return read_char_internal () == char_to_read;
}

char *
read_string ()
{
  char *string_base;
  int size;
  int i;

  read_white_space ();

  if (!read_char ('"'))
    return NULL;
  
  size = 64;
  string_base = xmalloc (size);

  i = 0;
  do
    {
      if (i == size)
	string_base = xrealloc (string_base, size *= 2);
      string_base[i] = read_char_internal ();
    } while (string_base[i ++] != '"');

  /* nul-out the ending `"' */
  string_base[--i] = '\0';
  
  return string_base;
}

int
valid_idn_char_p (char x)
{
  return isalnum (x) || x == '_';
}

char *
read_idn ()
{
  char *idn_base;
  int size;
  int i;

  read_white_space ();

  size = 64;
  idn_base = xmalloc (size);

  i = 0;
  do
    {
      if (i == size)
	idn_base = xrealloc (idn_base, size *= 2);
      idn_base[i] = read_char_internal ();
    } while (valid_idn_char_p (idn_base[i ++]));

  /* put back the last char read, which wasn't a valid identifier
     character */
  ungetc (idn_base[--i], i_fp);
  
  /* nul-out the invalid character */
  idn_base[i] = '\0';
  
  return idn_base;
}

int
valid_num_char_p (char x)
{
  return isdigit (x);
}

int
read_num (int *err)
{
  char *num_base;
  int size;
  int retval;
  int i;

  read_white_space ();

  size = 64;
  num_base = xmalloc (size);

  i = 0;
  do
    {
      if (i == size)
	num_base = xrealloc (num_base, size *= 2);
      num_base[i] = read_char_internal ();
    } while (valid_num_char_p (num_base[i ++]));

  /* put back the last char read, which wasn't a valid numeric
     character */
  ungetc (num_base[--i], i_fp);
  
  /* if we never actually read any characters, increase the error
     count */
  if (!i)
    {
      (*err) ++;
      return /* dummy */ -1;
    }
  
  /* nul-out the invalid character */
  num_base[i] = '\0';
  
  retval = atoi (num_base);
  free (num_base);

  return retval;
}

int
valid_hexnum_char_p (char x)
{
  return isxdigit (x);
}

int
read_hexnum (int *err)
{
  char *num_base;
  int size;
  int retval;
  int i;

  read_white_space ();
  
  size = 64;
  num_base = xmalloc (size);
  
  i = 0;
  do
    {
      if (i == size)
	num_base = xrealloc (num_base, size *= 2);
      num_base[i] = read_char_internal ();
    } while (valid_hexnum_char_p (num_base[i ++]));

  /* put back the last char read, which wasn't a valid numeric
     character */
  ungetc (num_base[--i], i_fp);
  
  /* if we never actually read any characters, increase the error
     count */
  if (!i)
    {
      (*err) ++;
      return /* dummy */ -1;
    }
  
  /* nul-out the invalid character */
  num_base[i] = '\0';
  
  retval = strtol (num_base, NULL, 16);
  free (num_base);
  
  return retval;
}

int
find_index (char *spec, char pixel)
{
  int i;
  
  for (i = 0; spec[i]; i ++)
    if (spec[i] == pixel)
      return i;

  fatal_error ("unknown pixel");
  return -1;  /* will never get here; to quiet gcc */
}

void
read_and_process_block (char *map_name,
			int which_depth, int depth,
			int width, int height)
{
  char *spec;
  /* bytes per row */
  int row_bytes;
  int bounds_bytes;
  int i;

  /* flag indicating if there are slop bits that need to be emitted
     after looping through the pixels; TRUE if the width is not a
     multiple of 8 */
  int bounds_slop_p;
  
  if (!read_char ('{'))
    fatal_error ("`{' expected");

  /* always make the rowbytes even */
  row_bytes = ((width * depth + 15) / 16) * 2;
  bounds_bytes = (width * depth + 7) / 8;
  bounds_slop_p = (width * depth) & 7;

  emit_prologue (map_name, depth);
  
  /* read in char specifier */
  spec = read_string ();
  if (!spec)
    fatal_error ("no spec; string expected");
  if (strlen (spec) > (1 << depth))
    fatal_error ("malformed spec; too long");

  for (i = 0; i < height; i ++)
    {
      char *current_row, *t;
      int j;

      /* output things a char at a type, keep the state
	 of the current char being built up here;
	 current_char contains the partially computed
	 character,
	 shift_count is the number of bits to shift
	 up the next pixel (bits to go - depth) */
      char current_char;
      int shift_count;

      t = current_row = read_string ();
      if (!current_row)
	fatal_error ("no row; string expected");
      if (strlen (current_row) != width)
	fatal_error ("malformed row; wrong length");
      
      emit_row_start ();

      current_char = 0;
      shift_count = 8 - depth;
      while (*t)
	{
	  int current_pixel;

	  current_pixel = find_index (spec, *t);
	  current_char |= (current_pixel << shift_count);
	  if (!shift_count)
	    {
	      emit_char (current_char);
	      current_char = 0;
	      shift_count = 8 - depth;
	    }
	  else
	    shift_count -= depth;

	  t ++;
	}
      /* if the number of bits needed is not a multiple of eight bits,
	 there is slop to be emitted after the loop finishes */
      if ((width * depth) & 7)
	emit_char (current_char);

      for (j = row_bytes - bounds_bytes; j > 0; j --)
	emit_char ('\0');
      
      /* be friendly, free the current_row, which was malloc'ed
	 in `read_string ()' */
      free (current_row);
      
      emit_row_end ();
    }

  read_white_space ();
  switch (read_char_internal ())
    {
    case '{':
      {
	int color_i, err;
	/* read rgb color information */
	
	for (color_i = err = 0;;
	     color_i ++)
	  {
	    struct rgb_color *color;
	    
	    color = &colors[color_i];
	    color->red = read_hexnum (&err);
	    if (err)
	      break;
	    if (!read_char (':'))
	      fatal_error ("`:' expected");
	    color->green = read_hexnum (&err);
	    if (!read_char (':'))
	      fatal_error ("`:' expected");
	    color->blue = read_hexnum (&err);
	  }
	n_colors = color_i;
	if (!read_char ('}')
	    || !read_char ('}'))
	  fatal_error ("`}' expected");
	break;
      }
    case '}':
      break;
    default:
      fatal_error ("`{' or `}' expected");
    }
  
  emit_epilogue (map_name, row_bytes, depth);

  /* be nice, free the spec */
  free (spec);
}

void
read_and_process_file ()
{
  int err;
  
  char *map_name;
  int n_depths;
  /* maximum four possible depths; 1, 2, 4, and 8 */
  int depths[4];
  int width, height;
  int i;

  map_name = read_idn ();
  if (!map_name)
    fatal_error ("no map name specified; identifier expected\n");

  for (n_depths = 0;;)
    {
      int depth;

      err = 0;
      depth = read_num (&err);
      if (err)
	break;
      else
	depths[n_depths ++] = depth;
    }
  if (! n_depths)
    fatal_error ("no depths specified");

  /* for now, we only handle 2 depths; ie., dual visuals */
  if (n_depths != 2)
    fatal_error ("invalid number of depths specified; only 2 allowed");
  
  if (!read_char (';'))
    fatal_error ("`;' expected");

  err = 0;
  width = read_num (&err);
  height = read_num (&err);
  if (err)
    fatal_error ("size expected `width, height'");
  
  bounds.top = image_desc.bounds.left = 0;
  bounds.bottom = height;
  bounds.right = width;
  
  for (i = 0; i < n_depths; i ++)
    read_and_process_block (map_name, i, depths[i], width, height);

  emit_rest (map_name);
}

int
main (int argc, char *argv[])
{
  int i;
  
  programname = argv[0];
  
  for (i = 1; i < argc; i ++)
    {
      if (*argv[i] == '-')
	{
	  if (!strcmp (argv[i], "-orig"))
	    {
	      orig_style_p = 1;
	    }
	  else if (!strcmp (argv[i], "-o"))
	    {
	      if (out_file)
		fatal_error ("multiple output files specified");
	    }
	  else
	    {
	      fatal_error ("unknown argument");
	    }
	}
      else
	{
	  if (in_file)
	    fatal_error ("multiple input files specified");
	}
    }

  if (in_file)
    {
      i_fp = fopen (in_file, "r");
      if (!i_fp)
	fatal_error ("unable to open input file");
    }
  else
    {
      in_file = "<stdin>";
      i_fp = stdin;
    }
  if (out_file)
    {
      o_fp = fopen (out_file, "w");
      if (!o_fp)
	fatal_error ("unable to open output file");
    }
  else
    {
      out_file = "<stdout>";
      o_fp = stdout;
    }

  parsing_p = 1;
  read_and_process_file ();
  
  exit (EXIT_SUCCESS);
}
