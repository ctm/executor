/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_svgalib[] = "$Id: svgalib.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/vgavdriver.h"
#include "rsys/cquick.h"
#include "rsys/host.h"
#include "rsys/dirtyrect.h"


/* Booleans for whether certain functionality is present. */
boolean_t svgalib_have_blitwait_p;
static boolean_t have_fillblit_p;
static boolean_t have_bitblit_p;
static boolean_t have_setrw_page_p;

/* Size of frame buffer to allocate. */
static int num_mode_sets = 0;


/* This function will be (sometimes) be called when svgalib detects
 * a crash.  We need it to restore the screen, keyboard, and mouse.
 */
static void
svgalib_crash (int ignored)
{
  event_shutdown ();
  vdriver_shutdown ();
  exit (1);
}


void
vdriver_opt_register (void)
{
}


boolean_t
vgahost_init (int max_width, int max_height, int max_bpp, boolean_t fixed_p,
	      int *argc, char *argv[])
{
  struct sigaction action;
  int i;

  atexit (vdriver_shutdown);
#if 0
  /* vga_safety_fork is fried ... we'll trap all possible signals ourselves
     sometime */
  vga_safety_fork (svgalib_crash);
#else
  action.sa_handler = svgalib_crash;
  action.sa_flags = 0;
  action.sa_restorer = 0;
  for (i = 1; i <= NSIG; ++i)
    {
      switch (i)
	{
	case SIGALRM:
	case SIGSTOP:
	case SIGCONT:
	case SIGTSTP:
	case SIGTTIN:
	case SIGTTOU:
	case SIGIO:
	case SIGVTALRM:
	case SIGPROF:
	case SIGWINCH:
	case SIGCHLD:
	  /* do nothing */
	  break;
	default:
	  sigemptyset (&action.sa_mask);
	  sigaddset (&action.sa_mask, i);
	  sigaction (i, &action, 0);
	}
    }
#endif
  return TRUE;
}


void
vgahost_shutdown (void)
{
  static char beenhere = FALSE;
  if (!beenhere)
    {
      vga_setmode (TEXT);
      beenhere = TRUE;
    }
}


void
vgahost_alloc_fbuf (unsigned long size)
{
  vdriver_fbuf = (uint8 *) malloc (size);
}


boolean_t
vgahost_mmap_linear_fbuf (const vga_mode_t *mode)
{
  if (num_mode_sets == 1)
    {
      free (vdriver_fbuf);
      vdriver_fbuf = vdriver_real_screen_baseaddr;
    }

  return (vdriver_fbuf == vdriver_real_screen_baseaddr);
}


boolean_t
vgahost_illegal_mode_p (int width, int height, int bpp,
			boolean_t exact_match_p)
{
  if (vga_current_mode == NULL)
    return FALSE;
  return (vdriver_fbuf == vdriver_real_screen_baseaddr
	  && bpp != (1 << vga_current_mode->log2_bpp));
}
			

boolean_t
vgahost_set_mode (vga_mode_t *mode)
{
  vga_modeinfo *info;
  int i;

  ++num_mode_sets;

  if (vga_current_mode == NULL
      || vga_current_mode->mode_number != mode->mode_number)
    {
      vga_setmode (mode->mode_number);

      /* Reset CLUT to 50% gray. */
      for (i = 0; i < 256; i++)
	vgahost_set_colors (i, 1, &ROMlib_gray_cspec);
    }

  /* Figure out what accelerated blitter functions are present. */
  info = vga_getmodeinfo (vga_getcurrentmode ());
  if (info->flags & CAPABLE_LINEAR)
    {
      int mapped_size, needed_size;

      /* Try to set linear addressing.  It's possible we'll fail,
       * even though svgalib claimed linear addressing is available.
       * Yahoo.  We'll try to handle this case gracefully.
       */
      mapped_size = vga_setlinearaddressing ();
      needed_size = mode->row_bytes * mode->height;
      if (mapped_size == -1 || mapped_size * 1024 < needed_size)
	mode->multi_window_p = (needed_size > VGA_WINDOW_SIZE (mode));
    }

  have_fillblit_p = ((info->haveblit & HAVE_FILLBLIT) != 0);
  have_bitblit_p  = ((info->haveblit & HAVE_BITBLIT ) != 0);

#if 0
  have_setrw_page_p = ((info->flags & HAVE_RWPAGE) != 0);
#else
/* #warning "separate r/w page support seems to be broken under Mach32 svgalib" */
  have_setrw_page_p = FALSE;
#endif

  /* Record whether we have blitwait.  Don't bother claiming we do
   * if we don't have any other accelerated blitter functions.
   */
  svgalib_have_blitwait_p = ((info->haveblit & HAVE_BLITWAIT)
			     && (have_fillblit_p || have_bitblit_p));

  if (mode->multi_window_p)
    {
      vga_read_window = vga_write_window = -1;
      vgahost_set_read_window (0);
      vgahost_set_write_window (0);
    }

  vga_portal_baseaddr = (uint8 *) vga_getgraphmem ();
  vdriver_real_screen_baseaddr = vga_portal_baseaddr;

  return TRUE;
}


void
vgahost_set_colors (int first_color, int num_colors,
		    const ColorSpec *color_array)
{
  int *rgb_list, *p;
  int i;

  rgb_list = (int *) alloca (3 * num_colors * sizeof rgb_list[0]);
  for (i = 0, p = rgb_list; i < num_colors; p += 3, i++)
    {
      int r, g, b;
      const ColorSpec *c = &color_array[i];

      r = *(const uint8 *)&c->rgb.red;  /* Grab MSB */
      if (r < 0xFC)
	r += (r & 2);  /* Round to nearest. */
      p[0] = r >> 2;

      g = *(const uint8 *)&c->rgb.green;  /* Grab MSB */
      if (g < 0xFC)
	g += (g & 2);  /* Round to nearest. */
      p[1] = g >> 2;

      b = *(const uint8 *)&c->rgb.blue;  /* Grab MSB */
      if (b < 0xFC)
	b += (b & 2);  /* Round to nearest. */
      p[2] = b >> 2;
    }

  vga_setpalvec (first_color, num_colors, rgb_list);
}


vga_mode_t *
vgahost_compute_vga_mode_list (void)
{
  int mode, last_mode, num_modes;
  vga_mode_t *vmode;

  last_mode = vga_lastmodenumber ();
  vmode = (vga_mode_t *) calloc (last_mode + 2, sizeof *vmode);

  /* Loop over all available modes.  There seems to be no good way to
   * find the index of the first mode; this is how one of the svgalib
   * demo programs does it.
   */
  for (mode = G320x200x16, num_modes = 0; mode <= last_mode; mode++)
    if (vga_hasmode (mode))
      {
	vga_modeinfo *info;
	vga_mode_t *v;
	int log2_bpp;

	info = vga_getmodeinfo (mode);
	if (info == NULL
	    || info->width < VDRIVER_MIN_SCREEN_WIDTH
	    || info->height < VDRIVER_MIN_SCREEN_HEIGHT
	    || (info->flags & IS_MODEX))
	  continue;

	/* Compute log base 2 of bits per pixel. */
	if (info->colors == 2 || info->colors == 4 || info->colors == 16)
	  log2_bpp = ROMlib_log2[ROMlib_log2[info->colors]];
	else if (info->colors == 256)
	  log2_bpp = 3;
	else if (info->colors > 256 && info->colors <= 65536)
	  log2_bpp = 4;
	else if (info->colors > 65536)
	  log2_bpp = 5;
	else
	  continue;  /* Fishy mode, ignore it. */

	v = &vmode[num_modes++];

	v->log2_bpp = log2_bpp;
	v->width = info->width;
	v->height = info->height;
	v->mode_number = mode;
		       
	v->planar_p = (info->bytesperpixel == 0);
	v->interlaced_p = ((info->flags & IS_INTERLACED) != 0);
	v->row_bytes = info->linewidth;
	v->multi_window_p = ((v->row_bytes * v->height > 65536)
			     && !(info->flags & CAPABLE_LINEAR));

	/* Under svgalib, we simulate distinct read and write windows
	 * by recording different window numbers.  The actual window
	 * number values aren't used by svgalib.
	 */
	v->win_read = 0;
	v->win_write = (info->flags & HAVE_RWPAGE) ? 1 : 0;
      }

  return vmode;
}


void
vgahost_set_rw_windows (int win)
{
  if (win != vga_read_window || win != vga_write_window)
    {
      vga_setpage (win);
      vga_read_window = vga_write_window = win;
    }
}


void
vgahost_set_read_window (int win)
{
  if (win != vga_read_window)
    {
      if (have_setrw_page_p)
	vga_setreadpage (win);
      else
	{
	  vga_setpage (win);
	  vga_write_window = win;  /* both read and write set */
	}

      vga_read_window = win;
    }
}


void
vgahost_set_write_window (int win)
{
  if (win != vga_write_window)
    {
      if (have_setrw_page_p)
	vga_setwritepage (win);
      else
	{
	  vga_setpage (win);
	  vga_read_window = win;  /* both read and write set */
	}
      vga_write_window = win;
    }
}


vdriver_accel_result_t
vdriver_accel_rect_fill (int top, int left, int bottom, int right,
			 uint32 color)
{
  int width, height;
  boolean_t bypass_p;

  if (!have_fillblit_p || !vga_current_mode || vga_current_mode->log2_bpp != 3)
    return VDRIVER_ACCEL_NO_UPDATE;

  width = right - left;
  height = bottom - top;

  if (width <= 0 || height <= 0)
    return VDRIVER_ACCEL_FULL_UPDATE;

  bypass_p = VDRIVER_BYPASS_INTERNAL_FBUF_P ();

  if (!bypass_p)
    dirty_rect_update_screen ();

  vga_fillblt (top * vga_current_mode->row_bytes
	       + left + vga_first_byte_offset,
	       width, height, vga_current_mode->row_bytes,
	       ((color ^ (vdriver_flip_real_screen_pixels_p ? 0xFF : 0))
		& ((1 << vdriver_bpp) - 1)));

  return (bypass_p
	  ? VDRIVER_ACCEL_FULL_UPDATE
	  : VDRIVER_ACCEL_HOST_SCREEN_UPDATE_ONLY);
}


vdriver_accel_result_t
vdriver_accel_rect_scroll (int top, int left, int bottom, int right,
			   int dx, int dy)
{
#if 0
  int width, height;
  boolean_t bypass_p;

  if (!have_fillblit_p || !vga_current_mode || vga_current_mode->log2_bpp != 3)
    return VDRIVER_ACCEL_NO_UPDATE;

  width = right - left;
  height = bottom - top;

  if (width <= 0 || height <= 0 || (dx == 0 && dy == 0))
    return VDRIVER_ACCEL_FULL_UPDATE;

  bypass_p = VDRIVER_BYPASS_INTERNAL_FBUF_P ();

  if (!bypass_p)
    dirty_rect_update_screen ();

  vga_bitblt (top * fbuf_row_bytes + left + upper_left_byte_offset,
	      ((top + dy) * fbuf_row_bytes + (left + dx)
	       + upper_left_byte_offset),
	       width, height, fbuf_row_bytes);

  return (bypass_p
	  ? VDRIVER_ACCEL_FULL_UPDATE
	  : VDRIVER_ACCEL_HOST_SCREEN_UPDATE_ONLY);
#endif
  return VDRIVER_ACCEL_NO_UPDATE;
}
