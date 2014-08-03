/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_gensplash[] =
	    "$Id: gensplash.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#include <stdarg.h>

#include "rsys/splash.h"

using namespace Executor;

int bpp, log2_bpp;

struct splash_screen_color color_buf[256];

int alloc_pixel;

boolean_t pixmap_p;

void
_errno_fatal (const char *file, int line, const char *fn,
	      const char *fmt, ...)
{
  va_list ap;
  int save_errno = errno;
  
  fprintf (stderr, "%s:%d; fatal error in `%s': ",
	   file, line, fn);
  
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  
  fprintf (stderr, ": %s\n", strerror (save_errno));
  
  exit (-1);
}

void
init_color_buf (void)
{
  memset (color_buf, '\377', sizeof *color_buf);
  color_buf[(1 << bpp) - 1].dummy_1 = -1;
  alloc_pixel = 0;
}

uint8
color_pixel (struct splash_screen_color *color)
{
  int i;
  
  color->dummy_1 = -1;
  
  for (i = 0; i <= alloc_pixel; i ++)
    {
      if (! memcmp (&color_buf[i], color, sizeof *color))
	return i;
    }
  
  if (! memcmp (&color_buf[(1 << bpp) - 1], color, sizeof *color))
    return (1 << bpp) - 1;
  
  alloc_pixel ++;
  if (alloc_pixel == (1 << bpp) - 1)
    {
      fprintf (stderr, "unable to find mapping for color %d %d %d\n",
	       color->red, color->green, color->blue);
      exit (-1);
    }
  
  color_buf[alloc_pixel] = *color;
  return alloc_pixel;
}

void
ppm_read_write_bits (int height, int width, int row_bytes,
		     FILE *fp, uint8 *bits)
{
  int max_cmp_value;
  uint8 *dst;

  if (pixmap_p)  
    fscanf (fp, "%d", &max_cmp_value);
  
  {
    int r, b, p;
  
    for (dst = bits, r = 0; r < height; r ++)
      {
	for (b = 0; b < row_bytes; b ++)
	  {
	    uint8 pixel = 0;
	    
	    for (p = 1 << (3 - log2_bpp); p; p --)
	      {
		if (pixmap_p)
		  {
		    uint32 red, green, blue;
		    struct splash_screen_color color;
		
		    fscanf (fp, "%d %d %d", &red, &green, &blue);
		
		    color.red   = CW ((red * 65535) / max_cmp_value);
		    color.green = CW ((green * 65535) / max_cmp_value);
		    color.blue  = CW ((blue * 65535) / max_cmp_value);
		
		    pixel = (pixel << bpp) | color_pixel (&color);
		  }
		else
		  {
		    int bit;
		    
#if 0
		    /* this didn't work on beaut */
		    fscanf (fp, "%1d", &bit);
#else
		    {
		      int c;
		      
		      do
			c = fgetc (fp);
		      while (c != EOF && c != '0' && c != '1');
		      switch (c)
			{
			case '0':
			  bit = 0;
			  break;
			case '1':
			  bit = 1;
			  break;
			default:
			  abort ();
			  break;
			}
		    }
#endif		    
		    if (bpp != 1)
		      abort ();
		    
		    switch (bit)
		      {
		      case 0:
			pixel = pixel << 1;  break;
		      case 1:
			pixel = (pixel << 1) | bit;  break;
		      default:
			abort ();
		      }
		  }
	      }
	    
	    *dst++ = pixel;
	  }
      }
  }
}

int
main (int argc, char *argv[])
{
  char *paramfile;
  char splashfile[1024], buttonfile[1024], outfile[1024];
  int splashfd, buttonfd, outfd, paramfd;
  FILE *splashfp, *buttonfp, *paramfp;
  char buf[1024];
  
  int splash_width, splash_height;
  int button_width, button_height;
  
  struct splash_screen_header header;
  
  int splash_row_bytes, button_row_bytes;
  uint8 *splash_bits, *button_bits;
  
  int retval, i;
  
  if (argc != 2)
    {
      fprintf (stderr, "usage: %s paramfile\n", *argv);
      exit (-1);
    }
  
  paramfile = argv[1];
  
  paramfd = open (paramfile, O_RDONLY);
  if (paramfd == -1)
    errno_fatal ("open %s", paramfile);
  paramfp = fdopen (paramfd, "r");
  if (paramfp == NULL)
    errno_fatal ("fdopen");
  
  fscanf (paramfp, "splash %s\n", splashfile);
  fscanf (paramfp, "button %s\n", buttonfile);
  fscanf (paramfp, "out %s\n", outfile);

  fprintf (stderr, "\
splash file `%s'\n\
button file `%s'\n\
output file `%s'\n",
	   splashfile, buttonfile, outfile);
  
  splashfd = open (splashfile, O_RDONLY);
  if (splashfd == -1)
    errno_fatal ("open %s", splashfile);
  splashfp = fdopen (splashfd, "r");
  if (splashfp == NULL)
    errno_fatal ("fdopen");

  buttonfd = open (buttonfile, O_RDONLY);
  if (buttonfd == -1)
    errno_fatal ("open %s", buttonfile);
  buttonfp = fdopen (buttonfd, "r");
  if (buttonfp == NULL)
    errno_fatal ("fdopen");
  
  outfd = open (outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (outfd == -1)
    errno_fatal ("open %s", outfile);

  fscanf (paramfp, "bpp %d\n", &bpp);
  switch (bpp)
    {
    case 8: log2_bpp = 3;  break;
    case 4: log2_bpp = 2;  break;
    case 2: log2_bpp = 1;  break;
    case 1: log2_bpp = 0;  break;
    default:
      abort ();
    }

  {
    int16 n_buttons;
    
    fscanf (paramfp, "n_buttons %hd\n", &n_buttons);
    
    for (i = 0; i < n_buttons; i ++)
      {
	int16 top, left, bottom, right;
	struct splash_screen_rect *rect;
	
	fscanf (paramfp, "rect %hd %hd %hd %hd\n",
		&top, &left, &bottom, &right);
	rect = &header.button_rects[i];
	
	rect->top = CW (top);
	rect->left = CW (left);
	rect->bottom = CW (bottom);
	rect->right = CW (right);
      }
    
    header.n_buttons = CW (n_buttons);
  }
  
  init_color_buf ();
  
  {
    uint32 red, green, blue;
    struct splash_screen_color color;
    
    fscanf (paramfp, "bk_color %d %d %d", &red, &green, &blue);
    
    color.red   = CW (red);
    color.green = CW (green);
    color.blue  = CW (blue);
    
    header.bg_pixel = color_pixel (&color);
  }
  
  fscanf (splashfp, "%s", buf);
  if (! strcmp (buf, "P3"))
    pixmap_p = TRUE;
  else if (! strcmp (buf, "P1"))
    pixmap_p = FALSE;
  else
    {
      fprintf (stderr, "ppm type must be `P3' or `P1'\n");
      exit (-1);
    }
  
  fscanf (splashfp, "%d %d", &splash_width, &splash_height);
  
  if (splash_width != SPLASH_SCREEN_WIDTH
      || splash_height != SPLASH_SCREEN_HEIGHT)
    {
      fprintf (stderr,
	       "ppm width and height must be %d and %d, respectively\n",
	       SPLASH_SCREEN_WIDTH, SPLASH_SCREEN_HEIGHT);
      exit (-1);
    }
  
  splash_row_bytes = splash_width >> (3 - log2_bpp);
  splash_bits = malloc (splash_row_bytes * splash_height);
  if (splash_bits == NULL)
    {
      fprintf (stderr, "unable to allocate `%d' bytes for bits buffer\n",
	       splash_row_bytes * splash_height);
      exit (-1);
    }
  
  ppm_read_write_bits (splash_height, splash_width, splash_row_bytes,
		       splashfp, splash_bits);
  
  fscanf (buttonfp, "%s", buf);
  if (! strcmp (buf, "P3"))
    pixmap_p = TRUE;
  else if (! strcmp (buf, "P1"))
    pixmap_p = FALSE;
  else
    {
      fprintf (stderr, "ppm type must be `P3' or `P1'\n");
      exit (-1);
    }
  
  fscanf (buttonfp, "%d %d", &button_width, &button_height);
  
  if (button_width > splash_width || button_height > splash_height)
    {
      fprintf (stderr, "button image cannot be larger than splash image\n");
      exit (-1);
    }
  
  if (button_width & ((1 << (3 - log2_bpp)) - 1))
    {
      fprintf (stderr, "button but be intergral number of bytes wide");
      exit (-1);
    }
  
  button_row_bytes = button_width >> (3 - log2_bpp);
  button_bits = malloc (button_row_bytes * button_height);
  if (button_bits == NULL)
    {
      fprintf (stderr, "unable to allocate `%d' bytes for bits buffer\n",
	       button_row_bytes * button_height);
      exit (-1);
    }
  
  ppm_read_write_bits (button_height, button_width, button_row_bytes,
		       buttonfp, button_bits);
  
  header.button_height = CW (button_height);
  header.button_y = CW (splash_height - 12 - button_height);
  header.button_x_byte = CW ((splash_width - 16 - button_width)
			     >> (3 - log2_bpp));
  header.button_row_bytes = CL (button_row_bytes);
  
  header.bpp = CL (bpp);
  header.log2_bpp = CL (log2_bpp);
  
  header.color_count = CL (1 << bpp);
  
  header.color_offset = CL (sizeof header);
  header.splash_bits_offset = CL (sizeof header + (sizeof *color_buf << bpp));
  header.button_bits_offset = CL (CL (header.splash_bits_offset)
				  + splash_row_bytes * splash_height);
  
  retval = write (outfd, &header, sizeof header);
  if (retval != sizeof header)
    errno_fatal ("write");
  
  retval = write (outfd, color_buf, sizeof *color_buf << bpp);
  if (retval != sizeof *color_buf << bpp)
    errno_fatal ("write");
  
  retval = write (outfd, splash_bits, splash_row_bytes * splash_height);
  if (retval != splash_row_bytes * splash_height)
    errno_fatal ("%d = write", retval);
  
  retval = write (outfd, button_bits, button_row_bytes * button_height);
  if (retval != button_row_bytes * button_height)
    errno_fatal ("%d = write", retval);
  
  fclose (splashfp);
  fclose (buttonfp);
  close (splashfd);
  close (buttonfd);
  close (outfd);
  
  exit (EXIT_SUCCESS);
}
