/* Copyright 1994, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#warning Has some hard-coded Executors in it

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_x[] = "$Id: x.c 89 2005-05-25 04:15:34Z ctm $";
#endif

#define Region Mac_Region
#define Cursor Mac_Cursor

#include "rsys/common.h"
#include "QuickDraw.h"
#include "ScrapMgr.h"
#include "OSUtil.h"

#undef Region
#undef Cursor

#include "rsys/commonevt.h"
#include "rsys/dirtyrect.h"
#include "rsys/keyboard.h"
#include "rsys/scrap.h"

#include <assert.h>

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

#include "rsys/sigio_multiplex.h"

#undef Time

#include <X11/X.h>
#include <X11/Xlib.h>
/* declares a data type `Region' */
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

#include "CQuickDraw.h"
#include "MemoryMgr.h"
#include "OSEvent.h"
#include "EventMgr.h"
#include "ToolboxEvent.h"
#include "rsys/adb.h"

#include "rsys/cquick.h"
#include "rsys/prefs.h"
#include "rsys/mman.h"
#include "rsys/refresh.h"
#include "rsys/vdriver.h"
#include <syn68k_public.h>
#include "rsys/m68kint.h"
#include "rsys/depthconv.h"
#include "rsys/rgbutil.h"
#include "rsys/option.h"
#include "rsys/flags.h"
#include "rsys/host.h"
#include "rsys/x.h"
#include "rsys/parse.h"
#include "rsys/osevent.h"
#include "rsys/notmac.h"

#include "x_keycodes.h"

PRIVATE boolean_t use_scan_codes = FALSE;

PUBLIC void
ROMlib_set_use_scancodes (boolean_t val)
{
  use_scan_codes = val; 
}

/* These variables are required by the vdriver interface. */

namespace Executor
{
uint8 *vdriver_fbuf;

int vdriver_row_bytes;

int vdriver_width = VDRIVER_DEFAULT_SCREEN_WIDTH;

int vdriver_height = VDRIVER_DEFAULT_SCREEN_HEIGHT;

int vdriver_bpp, vdriver_log2_bpp;

int vdriver_max_bpp, vdriver_log2_max_bpp;

rgb_spec_t *vdriver_rgb_spec;

bool vdriver_grayscale_p;
}

using namespace Executor;

/* save the original sigio flag; used when we shutdown */
static int orig_sigio_flag;
static int orig_sigio_owner; 
static int x_fd = -1;

#define EXECUTOR_WINDOW_EVENT_MASK (  KeyPressMask | KeyReleaseMask \
                                    | ButtonPressMask | ButtonReleaseMask \
                                    | EnterWindowMask | LeaveWindowMask \
                                    | ExposureMask | PointerMotionMask \
                                    | ColormapChangeMask)

void cursor_init (void);

/* visual we are using the for x_window */
XVisualInfo *visual;

/* size of internal frame buffer */
static int fbuf_size;

/* x window being driven */
static Window x_window;

/* if we are using a private colormap */
static int private_cmap_p;

/* if we prefer to use a truecolor visual */
static int truecolor_p;

/* if using a private colormap, this is it */
static Colormap color_map;

/* XImage used to draw mac frame buffer to the screen */
static XImage *x_image;

/* bytes per row of internal frame buffer */
static int fbuf_allocated_row_bytes;

/* shadow buffer; created on demand */
static unsigned char *shadow_fbuf;

static XImage *x_x_image;
static unsigned char *x_fbuf;
static int x_fbuf_bpp;
static int x_fbuf_row_bytes;

depthconv_func_t conversion_func = NULL;
static int (*x_put_image) (Display *x_dpy, Window x_window, GC copy_gc,
			   XImage *x_image, int src_x, int src_y, int dst_x,
			   int dst_y, unsigned int width,
			   unsigned int height) = NULL;

/* max dimensions */
static int max_width, max_height, max_bpp;

/* x cursor stuff */
static Cursor x_hidden_cursor, x_cursor = -1;

static XImage *x_image_cursor_data, *x_image_cursor_mask;
static Pixmap pixmap_cursor_data, pixmap_cursor_mask;

static GC copy_gc, cursor_data_gc, cursor_mask_gc, accel_gc;

static int cursor_visible_p = FALSE;

static Atom x_selection_atom;

static rgb_spec_t x_rgb_spec;

static char *selectiontext = NULL;
static int selectionlength;

/* true if `vdriver_set_colors ()' has been called, otherwise we can
   blow off expose events */
static int colors_initialized_p = FALSE;

/* TRUE if we should turn of autorepeat when the pointer is the
   executor window */
static int frob_autorepeat_p = FALSE;

static XrmOptionDescRec opts[] =
{
  { "-synchronous", ".synchronous", XrmoptionNoArg, "off" },
  { "-geometry", ".geometry", XrmoptionSepArg, 0 },
  { "-privatecmap", ".privateColormap", XrmoptionNoArg, "on" },
  { "-truecolor", ".trueColor", XrmoptionNoArg, "on" },

  /* options that are transfered from the x resource database to the
     `common' executor options database */
#define FIRST_COMMON_OPT 4
  { "-nosplash", ".noSplash", XrmoptionNoArg, "off" },
  { "-debug", ".debug", XrmoptionSepArg, 0 },
};

void
Executor::vdriver_opt_register (void)
{
  opt_register ("vdriver", {
  { "synchronous", "run in synchronous mode", opt_no_arg },
  { "geometry", "specify the executor window geometry", opt_sep },
  { "privatecmap", "have executor use a private x colormap", opt_no_arg },
  { "truecolor", "have executor use a TrueColor visual", opt_no_arg },
});

//x_opts, NELEM (x_opts));
}

static XrmDatabase xdb;

Display *x_dpy;
static int x_screen;

int
get_string_resource (char *resource, char **retval)
{
  char *res_type, res_name[256], res_class[256];
  XrmValue v;

  sprintf (res_name, "executor.%s", resource);
  sprintf (res_class, "Executor.%s", resource);
  
  if (! XrmGetResource (xdb, res_name, res_class, &res_type, &v))
    return FALSE;
  *retval = v.addr;
  return TRUE;
}

int
get_bool_resource (char *resource, int *retval)
{
  char *res_type, res_name[256], res_class[256];
  XrmValue v;

  sprintf (res_name, "executor.%s", resource);
  sprintf (res_class, "Executor.%s", resource);
  
  if (! XrmGetResource (xdb, res_name, res_class, &res_type, &v))
    return FALSE;
  if (   !strcasecmp (v.addr, "on")
      || !strcasecmp (v.addr, "true")
      || !strcasecmp (v.addr, "yes")
      || !strcasecmp (v.addr, "1"))
    {
      *retval = TRUE;
      return TRUE;
    }
  if (   !strcasecmp (v.addr, "off")
      || !strcasecmp (v.addr, "false")
      || !strcasecmp (v.addr, "no")
      || !strcasecmp (v.addr, "0"))
    {
      *retval = FALSE;
      return TRUE;
    }

  /* FIXME: print warning */
  *retval = FALSE;
  return TRUE;
}

static int have_shm, shm_err = FALSE;

static int shm_major_opcode;
static int shm_first_event;
static int shm_first_error;

int
x_error_handler (Display *err_dpy, XErrorEvent *err_evt)
{
  if (err_evt->request_code == shm_major_opcode)
    shm_err = TRUE;
  else
    {
      char error_text[256];
      char t[256];

      XGetErrorText (err_dpy, err_evt->error_code, error_text, 256);
      fprintf (stderr, "%s: XError `%d': %s\n", program_name,
	       err_evt->error_code, error_text);

      sprintf (t, "%d", err_evt->request_code);
      XGetErrorDatabaseText (x_dpy, "XRequest", t, "",
			     error_text, sizeof error_text);
      fprintf (stderr, "%s\n", error_text);
      
      exit (EXIT_FAILURE);
    }
  /* what do x error handlers return? */
  return 0;
}



int
x_normal_put_image (Display *x_dpy, Drawable x_window, GC copy_gc, XImage *x_image,
		    int src_x, int src_y, int dst_x, int dst_y,
		    unsigned int width, unsigned int height)
{
  int retval;
  
  retval = XPutImage (x_dpy, x_window, copy_gc, x_image,
		      src_x, src_y, dst_x, dst_y, width, height);
  /* flush the output buffers */
  XFlush (x_dpy);
  
  return retval;
}

int
x_shm_put_image (Display *x_dpy, Drawable x_window, GC copy_gc, XImage *x_image,
		 int src_x, int src_y, int dst_x, int dst_y,
		 unsigned int width, unsigned int height)
{
  int retval;

  retval = XShmPutImage (x_dpy, x_window, copy_gc, x_image,
			 src_x, src_y, dst_x, dst_y, width, height, False);
  /* flush the output buffers */
  XFlush (x_dpy);
  
  return retval;
}

void
alloc_x_image (int bpp, int width, int height,
	       int *row_bytes_return,
	       XImage **x_image_return, unsigned char **fbuf_return,
	       int *bpp_return)
{
  /* return value */
  /* assignments to shut up gcc */
  XImage *x_image = NULL;
  int row_bytes = 0;
  unsigned char *fbuf = NULL;
  int resultant_bpp = 0;

  if (have_shm)
    {
      XShmSegmentInfo *shminfo;

      /* note: this memory doesn't get reclaimed */
      shminfo = (XShmSegmentInfo*)malloc (sizeof *shminfo);
      memset (shminfo, '\0', sizeof *shminfo);
      
      x_image = XShmCreateImage (x_dpy, visual->visual,
				 bpp, ZPixmap, (char *) 0,
				 shminfo, width, height);
      if (x_image)
	{
	  row_bytes = x_image->bytes_per_line;
	  resultant_bpp = x_image->bits_per_pixel;
	  shminfo->shmid = shmget (IPC_PRIVATE,
				   row_bytes * height,
				   IPC_CREAT | 0777);
	  if (shminfo->shmid < 0)
	    {
	      XDestroyImage (x_image);
	      free (shminfo);
	      x_image = 0;
	    }
	  else
	    {
	      fbuf = (unsigned char *) (shminfo->shmaddr = x_image->data
					= (char*)shmat (shminfo->shmid, 0, 0));
	      if (fbuf == (unsigned char *) -1)
		{
		  /* do we need to delete the shmid here? */
		  free (shminfo);
		  XDestroyImage (x_image);
		  fbuf = NULL;
		}
	      else
		{
		  shminfo->readOnly = False;
		  XShmAttach (x_dpy, shminfo);
		  XSync (x_dpy, False);
		  if (shm_err)
		    {
		      /* do we need to delete the shmid here? */
		      free (shminfo);
		      XDestroyImage (x_image);
		      fbuf = NULL;
		      /* reset the shm_err flag */
		      shm_err = FALSE;
		    }
		  else
		    {
		      shmctl (shminfo->shmid, IPC_RMID, 0);
		      if (x_put_image == NULL)
			x_put_image = x_shm_put_image;
		      else if (x_put_image != x_shm_put_image)
			gui_abort ();
		    }
		}
	    }
	}
    }

  if (fbuf == NULL)
    {
      warning_unexpected ("not using shared memory");
      row_bytes = ((width * bpp + 31) / 32) * 4;
      fbuf = (unsigned char*)calloc (row_bytes * height, 1);
      x_image = XCreateImage (x_dpy, visual->visual, bpp, ZPixmap, 0,
			      (char *) fbuf, width, height, 8, row_bytes);
      resultant_bpp = x_image->bits_per_pixel;
      if (x_put_image == NULL)
	x_put_image = x_normal_put_image;
      else if (x_put_image != x_normal_put_image)
	gui_abort ();
    }

  x_image->byte_order  = MSBFirst;
  x_image->bitmap_bit_order  = MSBFirst;


  *fbuf_return = fbuf;
  *row_bytes_return = row_bytes;
  *x_image_return = x_image;
  *bpp_return = resultant_bpp;
}

#define LINEFD	0x36

#define R11	0x84
#define R13	0x85
#define R15	0x86
#define R7	0x87
#define R9	0x88
#define NOTKEY	0x89	

static uint8 latin_one_table_data[] =
{
  MKV_PRINT_SCREEN,
  NOTKEY,	/* 1 */
  NOTKEY,	/* 2 */
  NOTKEY,	/* 3 */
  NOTKEY,	/* 4 */
  NOTKEY,	/* 5 */
  NOTKEY,	/* 6 */
  NOTKEY,	/* 7 */
  NOTKEY,	/* 8 */
  NOTKEY,	/* 9 */
  NOTKEY,	/* 10 */
  NOTKEY,	/* 11 */
  NOTKEY,	/* 12 */
  NOTKEY,	/* 13 */
  NOTKEY,	/* 14 */
  NOTKEY,	/* 15 */
  NOTKEY,	/* 16 */
  NOTKEY,	/* 17 */
  NOTKEY,	/* 18 */
  NOTKEY,	/* 19 */
  NOTKEY,	/* 20 */
  NOTKEY,	/* 21 */
  NOTKEY,	/* 22 */
  NOTKEY,	/* 23 */
  NOTKEY,	/* 24 */
  NOTKEY,	/* 25 */
  NOTKEY,	/* 26 */
  NOTKEY,	/* 27 */
  NOTKEY,	/* 28 */
  NOTKEY,	/* 29 */
  NOTKEY,	/* 30 */
  NOTKEY,	/* 31 */
  MKV_SPACE,	/* SPACE */
  MKV_1,	/* ! */
  MKV_SLASH,	/* ? */
  MKV_3,	/* # */
  MKV_4,	/* $ */
  MKV_5,	/* % */
  MKV_7,	/* & */
  MKV_TICK,	/* ' */
  MKV_9,	/* ( */
  MKV_0,	/* ) */
  MKV_8,	/* * */
  MKV_EQUAL,	/* + */
  MKV_COMMA,	/* , */
  MKV_MINUS,	/* - */
  MKV_PERIOD,	/* . */
  MKV_SLASH,	/* / */
  MKV_0,	/* 0 */
  MKV_1,	/* 1 */
  MKV_2,	/* 2 */
  MKV_3,	/* 3 */
  MKV_4,	/* 4 */
  MKV_5,	/* 5 */
  MKV_6,	/* 6 */
  MKV_7,	/* 7 */
  MKV_8,	/* 8 */
  MKV_9,	/* 9 */
  MKV_SEMI,	/* : */
  MKV_SEMI,	/* ; */
  MKV_COMMA,	/* < */
  MKV_EQUAL,	/* = */
  MKV_PERIOD,	/* > */
  MKV_SLASH,	/* ? */
  MKV_2,	/* @ */
  MKV_a,	/* A */
  MKV_b,	/* B */
  MKV_c,	/* C */
  MKV_d,	/* D */
  MKV_e,	/* E */
  MKV_f,	/* F */
  MKV_g,	/* G */
  MKV_h,	/* H */
  MKV_i,	/* I */
  MKV_j,	/* J */
  MKV_k,	/* K */
  MKV_l,	/* L */
  MKV_m,	/* M */
  MKV_n,	/* N */
  MKV_o,	/* O */
  MKV_p,	/* P */
  MKV_q,	/* Q */
  MKV_r,	/* R */
  MKV_s,	/* S */
  MKV_t,	/* T */
  MKV_u,	/* U */
  MKV_v,	/* V */
  MKV_w,	/* W */
  MKV_x,	/* X */
  MKV_y,	/* Y */
  MKV_z,	/* Z */
  MKV_LEFTBRACKET,	/* [ */
  MKV_BACKSLASH,	/* \ */
  MKV_RIGHTBRACKET,	/* ] */
  MKV_6,	/* ^ */
  MKV_MINUS,	/* _ */
  MKV_BACKTICK,	/* ` */
  MKV_a,	/* a */
  MKV_b,	/* b */
  MKV_c,	/* c */
  MKV_d,	/* d */
  MKV_e,	/* e */
  MKV_f,	/* f */
  MKV_g,	/* g */
  MKV_h,	/* h */
  MKV_i,	/* i */
  MKV_j,	/* j */
  MKV_k,	/* k */
  MKV_l,	/* l */
  MKV_m,	/* m */
  MKV_n,	/* n */
  MKV_o,	/* o */
  MKV_p,	/* p */
  MKV_q,	/* q */
  MKV_r,	/* r */
  MKV_s,	/* s */
  MKV_t,	/* t */
  MKV_u,	/* u */
  MKV_v,	/* v */
  MKV_w,	/* w */
  MKV_x,	/* x */
  MKV_y,	/* y */
  MKV_z,	/* z */
};

static uint8 misc_table_data[] =
{
  MKV_BACKSPACE,	/* 8 back space */
  MKV_TAB,		/* 9 tab */
  LINEFD,		/* 10 line feed */
  MKV_NUMCLEAR,		/* 11 clear */
  NOTKEY,		/* 12 */
  MKV_RETURN,		/* 13 return */
  NOTKEY,		/* 14 */
  NOTKEY,		/* 15 */
  NOTKEY,		/* 16 */
  NOTKEY,		/* 17 */
  NOTKEY,		/* 18 */
  MKV_PAUSE,		/* 19 pause */
  NOTKEY,		/* 20 */
  NOTKEY,		/* 21 */
  NOTKEY,		/* 22 */
  NOTKEY,		/* 23 */
  NOTKEY,		/* 24 */
  NOTKEY,		/* 25 */
  NOTKEY,		/* 26 */
  MKV_ESCAPE,		/* 27 escape */
  NOTKEY,		/* 28 */
  NOTKEY,		/* 29 */
  NOTKEY,		/* 30 */
  NOTKEY,		/* 31 */
  MKV_RIGHTCNTL,	/* 32 multi-key character preface */
  NOTKEY,		/* 33 kanji */
  NOTKEY,		/* 34 */
  NOTKEY,		/* 35 */
  NOTKEY,		/* 36 */
  NOTKEY,		/* 37 */
  NOTKEY,		/* 38 */
  NOTKEY,		/* 39 */
  NOTKEY,		/* 40 */
  NOTKEY,		/* 41 */
  NOTKEY,		/* 42 */
  NOTKEY,		/* 43 */
  NOTKEY,		/* 44 */
  NOTKEY,		/* 45 */
  NOTKEY,		/* 46 */
  NOTKEY,		/* 47 */
  NOTKEY,		/* 48 */
  NOTKEY,		/* 49 */
  NOTKEY,		/* 50 */
  NOTKEY,		/* 51 */
  NOTKEY,		/* 52 */
  NOTKEY,		/* 53 */
  NOTKEY,		/* 54 */
  NOTKEY,		/* 55 */
  NOTKEY,		/* 56 */
  NOTKEY,		/* 57 */
  NOTKEY,		/* 58 */
  NOTKEY,		/* 59 */
  NOTKEY,		/* 60 */
  NOTKEY,		/* 61 */
  NOTKEY,		/* 62 */
  NOTKEY,		/* 63 */
  NOTKEY,		/* 64 */
  NOTKEY,		/* 65 */
  NOTKEY,		/* 66 */
  NOTKEY,		/* 67 */
  NOTKEY,		/* 68 */
  NOTKEY,		/* 69 */
  NOTKEY,		/* 70 */
  NOTKEY,		/* 71 */
  NOTKEY,		/* 72 */
  NOTKEY,		/* 73 */
  NOTKEY,		/* 74 */
  NOTKEY,		/* 75 */
  NOTKEY,		/* 76 */
  NOTKEY,		/* 77 */
  NOTKEY,		/* 78 */
  NOTKEY,		/* 79 */
  MKV_HOME,		/* 80 home */
  MKV_LEFTARROW,	/* left arrow */
  MKV_UPARROW,	/* up arrow */
  MKV_RIGHTARROW,	/* right arrow */
  MKV_DOWNARROW,	/* down arrow */
  MKV_PAGEUP,		/* prior */
  MKV_PAGEDOWN,	/* next */
  MKV_END,		/* end */
  NOTKEY,	/* 88 begin */
  NOTKEY,		/* 89 */
  NOTKEY,		/* 90 */
  NOTKEY,		/* 91 */
  NOTKEY,		/* 92 */
  NOTKEY,		/* 93 */
  NOTKEY,		/* 94 */
  NOTKEY,		/* 95 */
  NOTKEY,	/* 96 select */
  NOTKEY,	/* print */
  NOTKEY,	/* execute */
  MKV_HELP,	/* 99 insert/help */
  NOTKEY,		/* 100 */
  NOTKEY,	/* 101 undo */
  NOTKEY,	/* redo */
  NOTKEY,	/* menu */
  NOTKEY,	/* find */
  NOTKEY,	/* cancel */
  MKV_HELP,	/* help */
  NOTKEY,	/* 107 break */
  NOTKEY,		/* 108 */
  NOTKEY,		/* 109 */
  NOTKEY,		/* 110 */
  NOTKEY,		/* 111 */
  NOTKEY,		/* 112 */
  NOTKEY,		/* 113 */
  NOTKEY,		/* 114 */
  NOTKEY,		/* 115 */
  NOTKEY,		/* 116 */
  NOTKEY,		/* 117 */
  NOTKEY,		/* 118 */
  NOTKEY,		/* 119 */
  NOTKEY,		/* 120 */
  NOTKEY,		/* 121 */
  NOTKEY,		/* 122 */
  NOTKEY,		/* 123 */
  NOTKEY,		/* 124 */
  MKV_SCROLL_LOCK,	/* 125 unassigned (but scroll lock remapped) */
  MKV_LEFTOPTION,	/* 126 mode switch (also scroll lock) */
  MKV_NUMCLEAR,	/* 127 num lock */
  NOTKEY,	/* 128 key space */
  NOTKEY,	/* 129 */
  NOTKEY,	/* 130 */
  NOTKEY,	/* 131 */
  NOTKEY,	/* 132 */
  NOTKEY,	/* 133 */
  NOTKEY,	/* 134 */
  NOTKEY,	/* 135 */
  NOTKEY,	/* 136 */
  NOTKEY,	/* 137 key tab */
  NOTKEY,	/* 138 */
  NOTKEY,	/* 139 */
  NOTKEY,	/* 140 */
  MKV_NUMENTER,	/* 141 key enter */
  NOTKEY,	/* 142 */
  NOTKEY,	/* 143 */
  NOTKEY,	/* 144 */
  NOTKEY,	/* 145 key f1 */
  NOTKEY,	/* key f2 */
  NOTKEY,	/* key f3 */
  NOTKEY,	/* 148 key f4 */

  MKV_NUM7, /* 149 */
  MKV_NUM4, /* 150 */
  MKV_NUM8, /* 151 */
  MKV_NUM6, /* 152 */
  MKV_NUM2, /* 153 */
  MKV_NUM9, /* 154 */
  MKV_NUM3, /* 155 */
  MKV_NUM1, /* 156 */
  MKV_NUM5, /* 157 */
  MKV_NUM0, /* 158 */
  MKV_NUMPOINT, /* 159 */

  NOTKEY,	/* 160 */
  NOTKEY,	/* 161 */
  NOTKEY,	/* 162 */
  NOTKEY,	/* 163 */
  NOTKEY,	/* 164 */
  NOTKEY,	/* 165 */
  NOTKEY,	/* 166 */
  NOTKEY,	/* 167 */
  NOTKEY,	/* 168 */
  NOTKEY,	/* 169 */
  MKV_NUMDIVIDE,	/* 170 key multiply */
  MKV_NUMPLUS,	/* key plus */
  NOTKEY,	/* key comma */
  MKV_NUMMULTIPLY,	/* key minus */
  MKV_NUMPOINT,	/* key decimal point */
  MKV_NUMEQUAL,	/* key divide */
  MKV_NUM0,	/* key 0 */
  MKV_NUM1,	/* key 1 */
  MKV_NUM2,	/* key 2 */
  MKV_NUM3,	/* key 3 */
  MKV_NUM4,	/* key 4 */
  MKV_NUM5,	/* key 5 */
  MKV_NUM6,	/* key 6 */
  MKV_NUM7,	/* key 7 */
  MKV_NUM8,	/* key 8 */
  MKV_NUM9,	/* 185 */
  NOTKEY,	/* 186 */
  NOTKEY,	/* 187 */
  NOTKEY,	/* 188 */
  MKV_NUMEQUAL,	/* 189 key equals */
  MKV_F1,	/* f1 */
  MKV_F2,	/* f2 */
  MKV_F3,	/* f3 */
  MKV_F4,	/* f4 */
  MKV_F5,	/* f5 */
  MKV_F6,	/* f6 */
  MKV_F7,	/* f7 */
  MKV_F8,	/* f8 */
  MKV_F9,	/* f9 */
  MKV_F10,	/* f10 */
  MKV_F11,	/* l1 */
  MKV_F12,	/* l2 */
  MKV_F13,	/* l3 */
  MKV_F14,	/* l4 */
  MKV_F15,	/* l5 */
  NOTKEY,	/* l6 */ /* I don't know what these ones are */
  NOTKEY,	/* l7 */
  NOTKEY,	/* l8 */
  NOTKEY,	/* l9 */
  NOTKEY,	/* l10 */
  MKV_HELP,	/* r1 */
  MKV_HOME,	/* r2 */
  MKV_PAGEUP,	/* r3 */
  MKV_DELFORWARD,	/* r4 */
  MKV_END,	/* r5 */
  MKV_PAGEDOWN,	/* r6 */
  R7,	/* r7 */
  MKV_UPARROW,	/* r8 */
  R9,	/* r9 */
  MKV_LEFTARROW,	/* r10 */
  R11,	/* r11 */
  MKV_RIGHTARROW,	/* r12 */
  R13,	/* r13 */
  MKV_DOWNARROW,	/* r14 */
  R15,	/* r15 */
  MKV_LEFTSHIFT,	/* left shift */
  MKV_RIGHTSHIFT,	/* right shift */
  MKV_LEFTCNTL,	/* left control */
  MKV_RIGHTCNTL,	/* right control */
  MKV_CAPS,	/* caps lock */
  NOTKEY,	/* shift lock */
  /* cliff wants left Alt/Meta key to be `clover', the right
     Alt/Meta key to be `alt/option'.  and so it is;

     don't change these unless you also change `COMMONSTATE' in x.c
     to agree with them */
  MKV_CLOVER,	/* left meta */
  MKV_RIGHTOPTION,	/* right meta */
  MKV_CLOVER,	/* left alt */
  MKV_RIGHTOPTION,	/* right alt */
  NOTKEY,	/* left super */
  NOTKEY,	/* right super */
  NOTKEY,	/* left hyper */
  NOTKEY,	/* 238 right hyper */
  NOTKEY,	/* 239 */
  NOTKEY,	/* 240 */
  NOTKEY,	/* 241 */
  NOTKEY,	/* 242 */
  NOTKEY,	/* 243 */
  NOTKEY,	/* 244 */
  NOTKEY,	/* 245 */
  NOTKEY,	/* 246 */
  NOTKEY,	/* 247 */
  NOTKEY,	/* 248 */
  NOTKEY,	/* 249 */
  NOTKEY,	/* 250 */
  NOTKEY,	/* 251 */
  NOTKEY,	/* 252 */
  NOTKEY,	/* 253 */
  NOTKEY,	/* 254 */
  MKV_DELFORWARD,	/* 255 delete */
};

typedef struct key_table_data
{
  uint8 high_byte;
  int min;
  int size;
  uint8 *data;
} key_table_t;

key_table_t key_tables[] =
{
  /* latin one table */
  { 0, 0, NELEM (latin_one_table_data), latin_one_table_data },
  /* misc table */
  { 0xFF, 8, NELEM (misc_table_data), misc_table_data },
  { 0, 0, 0, NULL },
};

/* convert x keysym to mac virtual `keywhat'; return true the
   conversion was successful */

static int
x_keysym_to_mac_keywhat (unsigned int keysym, int16 button_state,
			 LONGINT *retval_out, boolean_t down_p,
			 unsigned char *virt_out)
{
  key_table_t *table;
  uint8 keysym_high_byte, keysym_low_byte;
  int16 keywhat;
  int i;

  if (use_scan_codes)
    {
      if (keysym == 0xff)
	keywhat = NOTKEY;
      else
	keywhat = keysym;
    }
  else
    {
      keysym_high_byte = (keysym >> 8) & 0xFF;
      keysym_low_byte = keysym & 0xFF;
  
      /* switch off the high 8bits of the keysym to determine which table
	 to index into */
      for (i = 0; key_tables[i].data; i ++)
	{
	  table = &key_tables[i];
	  if (table->high_byte == keysym_high_byte)
	    break;
	}
      if (key_tables[i].data == NULL)
	return FALSE;
  
      if (keysym_low_byte < table->min  
	  || keysym_low_byte > (table->min + table->size))
	return FALSE;
  
      keywhat = table->data[keysym_low_byte - table->min];
    }
  
  if (keywhat == NOTKEY)
    return FALSE;
  
  keywhat = ROMlib_right_to_left_key_map (keywhat);

  *virt_out = keywhat;
  *retval_out = ROMlib_xlate (keywhat, button_state, down_p);
  return TRUE;
}

PRIVATE boolean_t
keydown (uint8 key)
{
  boolean_t retval;
  int i;
  uint8 bit;

  retval = FALSE;

  if (ROMlib_get_index_and_bit (key, &i, &bit) && (KeyMap[i] & bit))
    retval = TRUE;
    
  return retval;
}

PRIVATE uint16
x_to_mac_state (unsigned int x_state)
{
  uint16 retval;

  retval = 
    (   (x_state & ShiftMask    ? shiftKey   : 0)
      | (x_state & LockMask     ? alphaLock  : 0)
      | (x_state & ControlMask  ? ControlKey : 0)
      | (x_state & Button1Mask  ? 0          : btnState));

#if 0
  if (use_scan_codes)
    {
#endif
      if (keydown (MKV_CLOVER))
	retval |= cmdKey;
      if (keydown (MKV_LEFTOPTION) || keydown (MKV_RIGHTOPTION))
	retval |= optionKey;
#if 0
    }
  else
    {
      retval |= 
	  (x_state & Mod1Mask     ? cmdKey     : 0)
	| (x_state & Mod3Mask     ? optionKey  : 0)
	| (x_state & Mod5Mask     ? optionKey  : 0);
    }
#endif

  return retval;
}

#define X_TO_MAC_STATE(x_state) x_to_mac_state (x_state)

static uint16 which_modifier_virt (unsigned char virt)
{
  uint16 retval;

  retval = 0;
  switch (virt)
    {
    case MKV_LEFTSHIFT:
    case MKV_RIGHTSHIFT:
      retval = shiftKey;
      break;

    case MKV_LEFTCNTL:
    case MKV_RIGHTCNTL:
      retval = ControlKey;
      break;

    case MKV_CAPS:
      retval = alphaLock;
      break;

    case MKV_CLOVER:
      retval = cmdKey;
      break;

    case MKV_LEFTOPTION:
    case MKV_RIGHTOPTION:
      retval = optionKey;
      break;

    default:
      break;
    }

  return retval;
}

static boolean_t
x_modifier_p (unsigned int keysym, uint16 *return_mac_modifier)
{
  int16 modifier;

  *return_mac_modifier = 0;
  switch (keysym)
    {
    case XK_Shift_L:
    case XK_Shift_R:
      modifier = shiftKey;
      break;
      
    case XK_Control_L:
    case XK_Control_R:
      modifier = ControlKey;
      break;

    case XK_Caps_Lock:
      modifier = alphaLock;
      break;
      
      /* ### XK_Shift_Lock */
      
    case XK_Meta_L:
    case XK_Meta_R:
      modifier = cmdKey;
      break;
    case XK_Alt_L:
    case XK_Alt_R:
    case XK_Mode_switch:
      modifier = optionKey;
      break;
    default:
      return FALSE;
    }

  *return_mac_modifier = modifier;
  return TRUE;
}

boolean_t
x_event_pending_p (void)
{
  fd_set fds;
  struct timeval no_wait;
  boolean_t retval;

  FD_ZERO (&fds);
  FD_SET (x_fd, &fds);
  no_wait.tv_sec = no_wait.tv_usec = 0;
  retval = select(x_fd+1, &fds, 0, 0, &no_wait) > 0;
  return retval;
}

syn68k_addr_t
post_pending_x_events (syn68k_addr_t interrupt_addr, void *unused)
{
  XEvent evt;
  Point where;
  int32 when;
  
  when = TickCount ();
  while (XCheckTypedEvent (x_dpy, SelectionRequest, &evt)
	 || XCheckMaskEvent (x_dpy, ~0L, &evt))
    {
      switch (evt.type)
	{
	case SelectionRequest:
	  {
	    Atom property;
	    XSelectionEvent xselevt;
	    
	    property = evt.xselectionrequest.property;
	    if (property == None)
	      property = x_selection_atom;
	    
	    XChangeProperty (x_dpy, evt.xselectionrequest.requestor, property,
			     XA_STRING, (int) 8, (int) PropModeReplace,
			     (unsigned char *) selectiontext, selectionlength);
	    
	    xselevt.type      = SelectionNotify;
	    xselevt.requestor = evt.xselectionrequest.requestor; /* check this */
	    xselevt.selection = evt.xselectionrequest.selection;
	    xselevt.target    = evt.xselectionrequest.target;
	    xselevt.property  = property;
	    xselevt.time      = evt.xselectionrequest.time;
	    
	    XSendEvent (x_dpy, xselevt.requestor, False, 0L,
			(XEvent *) &xselevt);
	    
	    break;
	  }
	case KeyPress:
	case KeyRelease:
	  {
	    LONGINT keywhat;
	    uint16 button_state;
	    uint16 modifier;
	    unsigned keysym;
	    unsigned char virt;
	    
	    where.h = evt.xkey.x;
	    where.v = evt.xkey.y;
	    button_state = X_TO_MAC_STATE (evt.xkey.state);

	    modifier = 0;
	    if (use_scan_codes)
	      {
		uint8 keycode;

		keycode = evt.xkey.keycode;
		if (keycode < NELEM(x_keycode_to_mac_virt))
		  {
		    keysym = x_keycode_to_mac_virt[keycode];
		    modifier = which_modifier_virt (keysym);
		  }
		else
		  keysym = NOTKEY;
	      }
	    else
	      {
		keysym = XLookupKeysym (&evt.xkey, 0);

		/* This hack is because the default is to map BACKSPACE to
		   the same key that DELETE produces just because people
		   don't like BACKSPACE generating a ^H ... yahoo! */

		if (keysym == 0xFFFF && evt.xkey.keycode == 0x16)
		  keysym = 0xFF08;

		/* This hack is because the default is to map Scroll Lock
		   and Right-Alt to the same key.  I'm not sure why */

		if (keysym == 0xFF7E && evt.xkey.keycode == 0x4e)
		  keysym = 0xFF7D;

		x_modifier_p (keysym, &modifier);
	      }

	    if (modifier)
	      {
		if (evt.type == KeyPress)
		  button_state |= modifier;
		else
		  button_state &= ~modifier;
	      }
		    
	    if (x_keysym_to_mac_keywhat (keysym, button_state, &keywhat,
					 evt.type == KeyPress, &virt))
	      {
		INTEGER evcode;

		evcode = evt.type == KeyRelease ? keyUp : keyDown;
		post_keytrans_key_events (evcode, keywhat, when, where,
					  button_state, virt);
	      }
	    break;
	  }
	case ButtonPress:
	case ButtonRelease:
	  {
	    int16 button_state;
	    
	    where.h = evt.xbutton.x;
	    where.v = evt.xbutton.y;
	    button_state = X_TO_MAC_STATE (evt.xbutton.state);
	    if (evt.type == ButtonPress)
	      button_state &= ~btnState;
	    else
	      button_state |= btnState;
	    ROMlib_PPostEvent ((evt.type == ButtonRelease) ? mouseUp : mouseDown,
			       0, (GUEST<EvQElPtr> *) 0,
			       when, where,
			       button_state);
	    adb_apeiron_hack (FALSE);
	    break;
	  }
	case Expose:
	  if (colors_initialized_p)
	    vdriver_update_screen (evt.xexpose.y, evt.xexpose.x,
				   evt.xexpose.y + evt.xexpose.height,
				   evt.xexpose.x + evt.xexpose.width, FALSE);
	  break;
	case EnterNotify:
	  if (frob_autorepeat_p)
	    XAutoRepeatOff (x_dpy);
	  {
	    boolean_t cvt;
	    Window selection_owner;

	    selection_owner = XGetSelectionOwner (x_dpy, XA_PRIMARY);
	    cvt = selection_owner != None && selection_owner != x_window;
	    if (cvt)
	      ZeroScrap ();
	    sendresumeevent (cvt);
	  }
	  break;
	case LeaveNotify:
	  if (frob_autorepeat_p)
	    XAutoRepeatOn (x_dpy);
	  sendsuspendevent ();
	  break;
        case MotionNotify:
	  MouseLocation.h = CW (evt.xmotion.x);
	  MouseLocation.v = CW (evt.xmotion.y);
	  adb_apeiron_hack (FALSE);
	  break;
	}
    }
  return MAGIC_RTE_ADDRESS;
}

void
x_event_handler (int signo)
{
  /* request syncint */
  cpu_state.interrupt_pending[M68K_EVENT_PRIORITY] = 1; 
  cpu_state.interrupt_status_changed = INTERRUPT_STATUS_CHANGED; 
}

boolean_t
Executor::vdriver_init (int _max_width, int _max_height, int _max_bpp,
	      boolean_t fixed_p, int *argc, char *argv[])
{
  int i;
      
  XVisualInfo *visuals, vistemplate;
  int n_visuals;
  int dummy_int;
  unsigned int geom_width, geom_height;
  int dummy_bpp;
  int num_red_bits, low_red_bit;
  int num_green_bits, low_green_bit;
  int num_blue_bits, low_blue_bit;
  char *geom;
  int t_int;
  char *t_str;
  
  char *xdefs;
  if (x_dpy != NULL)
    {
      gui_fatal ("Internal error: vdriver_init() called twice!\n");
      return FALSE;
    }

  x_dpy = XOpenDisplay ("");
  if (x_dpy == NULL)
    {
      fprintf (stderr, "%s: could not open x server `%s'.\n",
	       program_name, XDisplayName (""));
      exit (EXIT_FAILURE);
    }
  x_screen = XDefaultScreen (x_dpy);
  
  XSetErrorHandler (x_error_handler);

  /* determine if the server supports the `XShm' extension */
  have_shm = XQueryExtension (x_dpy, "MIT-SHM",
			      &shm_major_opcode,
			      &shm_first_event,
			      &shm_first_error);

  xdefs = XResourceManagerString (x_dpy);
  if (xdefs)
    xdb = XrmGetStringDatabase (xdefs);

  XrmParseCommand (&xdb, opts, NELEM (opts), "Executor", argc, argv);
  
  get_bool_resource ("privateColormap", &private_cmap_p);

  get_bool_resource ("trueColor", &truecolor_p);
  
  if (get_bool_resource ("noSplash", &t_int))
    opt_put_int_val (common_db, "nosplash", t_int, pri_x_resource, FALSE);
  
  if (get_string_resource ("debug", &t_str))
    opt_put_val (common_db, "debug", t_str, pri_x_resource, FALSE);

  if (!get_string_resource ("geometry", &geom))
    geom = 0;
  /* default */
  geom_width = VDRIVER_DEFAULT_SCREEN_WIDTH;
  geom_height = VDRIVER_DEFAULT_SCREEN_HEIGHT;
  if (geom)
    {
      /* override the maximum with possible geometry values */
      XParseGeometry (geom, &dummy_int, &dummy_int,
		      &geom_width, &geom_height);
    }
  if (!_max_width)
    _max_width = MAX (VDRIVER_DEFAULT_SCREEN_WIDTH,
		      (MAX (geom_width, flag_width)));
  max_width = _max_width;
  if (!_max_height)
    _max_height = MAX (VDRIVER_DEFAULT_SCREEN_HEIGHT,
		       MAX (geom_height, flag_height));
  max_height = _max_height;
  if (!_max_bpp)
    _max_bpp = 32;
  max_bpp = _max_bpp;

  vdriver_x_modes.size[1].width  = max_width;
  vdriver_x_modes.size[1].height = max_height;
  
  /* first attempt to find a 8bpp pseudocolor visual */
  vistemplate.screen = x_screen;
  vistemplate.depth = 8;
  vistemplate.c_class = PseudoColor;
  vistemplate.colormap_size = 256;

  visual = NULL;

  if (!truecolor_p)
    {
      visuals = XGetVisualInfo (x_dpy,
				(  VisualScreenMask | VisualDepthMask
				 | VisualClassMask | VisualColormapSizeMask),
				&vistemplate, &n_visuals);
      if (n_visuals)
	{
	  /* just use the first visual that came up */
	  visual = visuals;
	  x_fbuf_bpp = 8;
	}

      if (visual == NULL)
	{
	  /* now try for 4bpp */
	  vistemplate.depth = 4;
	  vistemplate.colormap_size = 16;
	  visuals = XGetVisualInfo (x_dpy,
				    (  VisualScreenMask | VisualDepthMask
				     | VisualClassMask
				     | VisualColormapSizeMask),
				    &vistemplate, &n_visuals);
	  if (n_visuals)
	    {
	      /* just use the first visual that came up */
	      visual = visuals;
	      x_fbuf_bpp = 4;
	    }
	}

      if (visual == NULL)
	{
	  /*  now try for 1bpp */
	  vistemplate.depth = 1;
	  vistemplate.c_class = StaticGray;
	  visuals = XGetVisualInfo (x_dpy,
				    (  VisualScreenMask | VisualDepthMask
				     | VisualClassMask),
				    &vistemplate, &n_visuals);
	  if (n_visuals)
	    {
	      /* just use the first visual that came up */
	      visual = visuals;
	      x_fbuf_bpp = 1;
	    }
	}
    }
  
  if (visual == NULL)
    {
      vistemplate.screen = x_screen;
      vistemplate.c_class = TrueColor;
      
      visuals = XGetVisualInfo (x_dpy,
				(VisualScreenMask | VisualClassMask),
				&vistemplate, &n_visuals);
      if (n_visuals)
	{
	  for (i = 0; i < n_visuals; i ++)
	    {
	      if (visuals[i].depth >= 12)
		{
		  /* use the first visual >= 12bpp */
		  visual = &visuals[i];
		  x_fbuf_bpp = visual->depth;
		  break;
		}
	    }

	  if (x_fbuf_bpp)
	    {
	      /* allocate an image now; since `vdriver_set_colors ()'
		 expects it to be allocated and we will have to do so
		 eventually */
	      alloc_x_image (x_fbuf_bpp, max_width, max_height,
			     &x_fbuf_row_bytes, &x_x_image, &x_fbuf,
			     &x_fbuf_bpp);

	      low_red_bit    = ffs (visual->red_mask) - 1;
	      low_green_bit  = ffs (visual->green_mask) - 1;
	      low_blue_bit   = ffs (visual->blue_mask) - 1;
	      num_red_bits   = ffs ((visual->red_mask
				     >> low_red_bit) + 1) - 1;
	      num_green_bits = ffs ((visual->green_mask
				     >> low_green_bit) + 1) - 1;
	      num_blue_bits  = ffs ((visual->blue_mask
				     >> low_blue_bit) + 1) - 1;
	      make_rgb_spec (&x_rgb_spec,
			     x_x_image->bits_per_pixel, FALSE, 0,
			     num_red_bits, low_red_bit,
			     num_green_bits, low_green_bit,
			     num_blue_bits, low_blue_bit,
			     CL_RAW (GetCTSeed ()));
	    }
	}
    }
  
  if (visual == NULL)
    {
      fprintf (stderr, "%s: no acceptable visual found, exiting.\n",
	       program_name);
      exit (EXIT_FAILURE);
    }
  
  if (max_bpp > 32)
    max_bpp = 32;
  if (max_bpp > x_fbuf_bpp)
    max_bpp = x_fbuf_bpp;
  
  vdriver_max_bpp = max_bpp;
  vdriver_log2_max_bpp = ROMlib_log2[max_bpp];
  
  alloc_x_image (max_bpp, max_width, max_height,
		 &vdriver_row_bytes, &x_image, &vdriver_fbuf, &dummy_bpp);
  fbuf_allocated_row_bytes = vdriver_row_bytes;
  fbuf_size = vdriver_row_bytes * max_height;
  
  cursor_init ();
  
  x_selection_atom = XInternAtom (x_dpy, "ROMlib_selection", False);
  
  /* hook into syn68k synchronous interrupts */
  {
    syn68k_addr_t event_callback;
    
    event_callback = callback_install (post_pending_x_events, NULL);
    *(GUEST<ULONGINT> *) SYN68K_TO_US(M68K_EVENT_VECTOR * 4) = CL ((ULONGINT)event_callback);
  }

  /* set up the async x even handler */
  {
    x_fd = XConnectionNumber (x_dpy);
    
    sigio_multiplex_install_handler (x_fd, x_event_handler);
    
    fcntl (x_fd, F_GETOWN, &orig_sigio_owner);
    fcntl (x_fd, F_SETOWN, getpid ());
    orig_sigio_flag = fcntl (x_fd, F_GETFL, 0) & ~FASYNC;
    fcntl (x_fd, F_SETFL, orig_sigio_flag | FASYNC);
    
    /* call the event loop to bootstrap things, and set up the signal
       handler */
    x_event_handler (SIGIO);
  }
  
  /* Force a cleanup on program exit. */
  atexit (vdriver_shutdown);
  
  return TRUE;
}


void
Executor::vdriver_flush_display (void)
{
  XFlush (x_dpy);
}


void
alloc_x_window (int width, int height, int bpp, boolean_t grayscale_p)
{
  XSetWindowAttributes xswa;
  XSizeHints size_hints;
  XGCValues gc_values;
  
  char *geom;
  int geom_mask;
  int x = 0, y = 0;
  
  memset (&size_hints, 0, sizeof size_hints);
  
  /* size and base size are set below after parsing geometry */
  size_hints.flags = PSize | PMinSize | PMaxSize | PBaseSize | PWinGravity;
  
  size_hints.min_width = VDRIVER_MIN_SCREEN_WIDTH;
  size_hints.min_height = VDRIVER_MIN_SCREEN_HEIGHT;
  
  size_hints.max_width = max_width;
  size_hints.max_height = max_height;
  
  if (   !width
	 || !height)
    {
      geom = NULL;
      get_string_resource ("geometry", &geom);
      if (geom)
	{
	  geom_mask = XParseGeometry (geom, &x, &y,
				      (unsigned int *) &vdriver_width,
				      (unsigned int *) &vdriver_height);
	  if (geom_mask & WidthValue)
	    {
	      if (vdriver_width < VDRIVER_MIN_SCREEN_WIDTH)
		vdriver_width = VDRIVER_MIN_SCREEN_WIDTH;
	      else if (vdriver_width > max_width)
		vdriver_width = max_width;

	      size_hints.flags |= USSize;
	    }
	  if (geom_mask & HeightValue)
	    {
	      if (vdriver_height < VDRIVER_MIN_SCREEN_HEIGHT)
		vdriver_height = VDRIVER_MIN_SCREEN_HEIGHT;
	      else if (vdriver_height > max_height)
		vdriver_height = max_height;
	      size_hints.flags |= USSize;
	    }
	  if (geom_mask & XValue)
	    {
	      if (geom_mask & XNegative)
		x = XDisplayWidth (x_dpy, x_screen) + x - vdriver_width;
	      size_hints.flags |= USPosition;
	    }
	  if (geom_mask & YValue)
	    {
	      if (geom_mask & YNegative)
		y = XDisplayHeight (x_dpy, x_screen) + y - vdriver_height;
	      size_hints.flags |= USPosition;
	    }
	  switch (geom_mask & (XNegative | YNegative))
	    {
	    case 0:
	      size_hints.win_gravity = NorthWestGravity;  break;
	    case XNegative:
	      size_hints.win_gravity = NorthEastGravity;  break;
	    case YNegative:
	      size_hints.win_gravity = SouthWestGravity;  break;
	    default:
	      size_hints.win_gravity = SouthEastGravity;  break;
	    }
	}
    }
  else
    {
      /* size was specified; we aren't using defaults */
      size_hints.flags |= USSize;
      size_hints.win_gravity = NorthWestGravity;
      
      if (width < 512)
	width = 512;
      else if (width > max_width)
	width = max_width;

      if (height < 342)
	height = 342;
      else if (height > max_height)
	height = max_height;
      
      vdriver_width  = width;
      vdriver_height = height;
    }
  
  size_hints.min_width = size_hints.max_width
    = size_hints.base_width  = size_hints.width  = vdriver_width;
  size_hints.min_height = size_hints.max_height
     = size_hints.base_height = size_hints.height = vdriver_height;
  
  size_hints.x = x;
  size_hints.y = y;

  /* #### allow command line options to select {16, 32}bpp modes, but
     don't make them the default since they still have problems */
  if (bpp == 0)
    bpp = MIN (max_bpp, 8);
  else if (bpp > max_bpp)
    bpp = max_bpp;
  
  vdriver_bpp = bpp;
  vdriver_log2_bpp = ROMlib_log2[bpp];
  
  vdriver_grayscale_p = grayscale_p;
  
  /* create the executor window */
  x_window = XCreateWindow (x_dpy, XRootWindow (x_dpy,  x_screen),
			    x, y, vdriver_width, vdriver_height,
			    0, 0, InputOutput, CopyFromParent, 0, &xswa);

  
  XDefineCursor (x_dpy, x_window, x_hidden_cursor);
  
  gc_values.function = GXcopy;
  gc_values.foreground = XBlackPixel (x_dpy, x_screen);
  gc_values.background = XWhitePixel (x_dpy, x_screen);
  gc_values.plane_mask = AllPlanes;
  
  copy_gc = XCreateGC (x_dpy, x_window,
		       (  GCFunction | GCForeground
			  | GCBackground | GCPlaneMask), &gc_values);
  
  accel_gc = XCreateGC (x_dpy, x_window,
			(  GCFunction | GCForeground
			 | GCBackground | GCPlaneMask), &gc_values);
  
  {
    /* various hints for `XSetWMProperties ()' */
    XWMHints wm_hints;
    XClassHint class_hint;
    XTextProperty name;

    memset (&wm_hints, 0, sizeof wm_hints);
    
    class_hint.res_name = "executor";
    class_hint.res_class = "Executor";
    
    XStringListToTextProperty (&program_name, 1, &name);
    
    XSetWMProperties (x_dpy, x_window,
		      &name, &name, /* _argv, *_argc, */ NULL, 0,
		      &size_hints, &wm_hints, &class_hint);
  }

  XSelectInput (x_dpy, x_window, EXECUTOR_WINDOW_EVENT_MASK);
  
  XMapRaised (x_dpy, x_window);
  XClearWindow (x_dpy, x_window);
  XFlush (x_dpy);
}

int Executor::host_cursor_depth = 1;

Cursor
create_x_cursor (char *data, char *mask,
		 int hotspot_x, int hotspot_y)
{
  Cursor retval;  
  char x_mask[32], x_data[32];
  int i;
  
  static XColor x_black = {  0,  0,  0,  0 },
                x_white = { (unsigned long)~0,
			    (unsigned short)~0,
 			    (unsigned short)~0,
			    (unsigned short)~0 };

  for (i = 0; i < 32; i++)
    {
      x_mask[i] = data[i] | mask[i];
      x_data[i] = data[i];
    }
  
  x_image_cursor_data->data = x_data;
  x_image_cursor_mask->data = x_mask;
  
  XPutImage (x_dpy, pixmap_cursor_data, cursor_data_gc, x_image_cursor_data,
	     0, 0, 0, 0, 16, 16);
  XPutImage (x_dpy, pixmap_cursor_mask, cursor_mask_gc, x_image_cursor_mask,
	     0, 0, 0, 0, 16, 16);

  if (hotspot_x < 0)
    hotspot_x = 0;
  else if (hotspot_x > 16)
    hotspot_x = 16;

  if (hotspot_y < 0)
    hotspot_y = 0;
  else if (hotspot_y > 16)
    hotspot_y = 16;

  retval = XCreatePixmapCursor (x_dpy, pixmap_cursor_data, pixmap_cursor_mask,
				&x_black, &x_white,
				hotspot_x, hotspot_y);
  return retval;
}

void
Executor::host_set_cursor (char *cursor_data,
		 unsigned short cursor_mask[16],
		 int hotspot_x, int hotspot_y)
{
  Cursor orig_x_cursor = x_cursor;
  
  x_cursor = create_x_cursor (cursor_data, (char *)cursor_mask,
			      hotspot_x, hotspot_y);
  
  /* if visible, set `x_cursor' to be the current cursor */
  if (cursor_visible_p)
    XDefineCursor (x_dpy, x_window, x_cursor);
  
  if (orig_x_cursor != -1)
    XFreeCursor (x_dpy, orig_x_cursor);
}

int
Executor::host_set_cursor_visible (int show_p)
{
  int orig_cursor_visible_p = cursor_visible_p;

  if (cursor_visible_p)
    {
      if (!show_p)
	{
	  cursor_visible_p = FALSE;
	  XDefineCursor (x_dpy, x_window, x_hidden_cursor);
	}
    }
  else
    {
      if (show_p)
	{
	  cursor_visible_p = TRUE;
	  XDefineCursor (x_dpy, x_window, x_cursor);
	}
    }

  return orig_cursor_visible_p;
}

void
cursor_init (void)
{
  XGCValues gc_values;
  static char zero[2 * 16] = { 0, };
  
  /* the following are used to create x cursors, they must
     be done before calling `create_x_cursor ()' */
  x_image_cursor_data = XCreateImage (x_dpy, XDefaultVisual (x_dpy, x_screen),
				      1, XYBitmap, 0, NULL, 16, 16, 8, 2);
  x_image_cursor_mask = XCreateImage (x_dpy, XDefaultVisual (x_dpy, x_screen),
				      1, XYBitmap, 0, NULL, 16, 16, 8, 2);

  x_image_cursor_data->byte_order = MSBFirst;
  x_image_cursor_mask->byte_order = MSBFirst;

  x_image_cursor_data->bitmap_bit_order = MSBFirst;
  x_image_cursor_mask->bitmap_bit_order = MSBFirst;

  pixmap_cursor_data = XCreatePixmap (x_dpy, XRootWindow (x_dpy, x_screen),
				      16, 16, 1);
  pixmap_cursor_mask = XCreatePixmap (x_dpy, XRootWindow (x_dpy, x_screen),
				      16, 16, 1);

  gc_values.function = GXcopy;
  gc_values.foreground = ~0;
  gc_values.background = 0;
  
  gc_values.plane_mask = AllPlanes;

  cursor_data_gc = XCreateGC (x_dpy, pixmap_cursor_data,
			      (  GCFunction | GCForeground
			       | GCBackground | GCPlaneMask), &gc_values);
  cursor_mask_gc = XCreateGC (x_dpy, pixmap_cursor_mask,
			      (  GCFunction | GCForeground
			       | GCBackground | GCPlaneMask), &gc_values);

  x_hidden_cursor = create_x_cursor (zero, zero, 0, 0);
}

#define RGB_DIST(red, green, blue)			\
  ({							\
    int _red = (red), _green = (green), _blue = (blue);	\
    (_red * _red + _green * _green + _blue * _blue);	\
  })

#define MAX_CDIST (RGB_DIST (65535 >> 1, 65535 >> 1, 65535 >> 1) + 1U)

/* distance between two ColorSpec */

static inline int
cs_cs_dist (const ColorSpec *c0, const ColorSpec *c1)
{
  return RGB_DIST ((CW (c0->rgb.red) - CW (c1->rgb.red)) >> 1,
		   (CW (c0->rgb.green) - CW (c1->rgb.green)) >> 1,
		   (CW (c0->rgb.blue) - CW (c1->rgb.blue)) >> 1);
}


#define CS_X_DIST(r, g, b, x) RGB_DIST ((r) - (x).red,		\
					(g) - (x).green,	\
					(b) - (x).blue)

static char x_alloced[256];
static boolean_t x_cmap_initialized_p;
static XColor x_cmap[256];
static struct 
{
  uint16 red, green, blue;  /* Each is shifted right one bit. */
} shifted_x_cmap[256];

/* maps mac colors to the x color table */
static uint32 _cmap_mapping[256];

/* mapping from mac color table index values to x color map index
   values; or NULL if the mapping is the identity */
static uint32 *cmap_mapping;

static void
compute_new_mapping (int index, const ColorSpec *c)
{
  int i;
  int min = -1;
  unsigned min_dist = MAX_CDIST;
  int shifted_c_red, shifted_c_blue, shifted_c_green;
  
  shifted_c_red   = CW (c->rgb.red) >> 1;
  shifted_c_green = CW (c->rgb.green) >> 1;
  shifted_c_blue  = CW (c->rgb.blue) >> 1;
  
  for (i = (1 << x_fbuf_bpp) - 1; i >= 0; i --)
    {
      if (x_alloced[i])
	{
	  unsigned t;

	  t = CS_X_DIST (shifted_c_red, shifted_c_green, shifted_c_blue,
			 shifted_x_cmap[i]);
	  if (t < min_dist)
	    {
	      min = x_cmap[i].pixel;
	      if (t == 0)  /* perfect match? */
		break; 
	      min_dist = t;
	    }
	}
    }
  if (min < 0)
    gui_abort ();
  _cmap_mapping[index] = min;
}

/* zero'ed */
static ColorSpec origin;

void
init_x_cmap (void)
{
  int n_colors;
  int i, j;
  /* shut up gcc */
  int prev = 0;
  int alloc_order[256], k = 0;
  char mark[256], failure[256];
  unsigned max_dist;
  int n_alloced = 0;
  ColorSpec *orig_colors;

  n_colors = 1 << x_fbuf_bpp;
  
  memset (mark, '\0', sizeof mark);
  memset (failure, '\0', sizeof x_alloced);

  switch (n_colors)
    {
    case 256:
      orig_colors = &ctab_8bpp_values[0];
      break;
    case 16:
      orig_colors = &ctab_4bpp_values[0];
      break;
    case 4:
      orig_colors = &ctab_2bpp_values[0];
      break;
    case 2:
      orig_colors = &ctab_1bpp_values[0];
      break;
    default:
      gui_abort ();
    }
  
  /* first, sort the `ctab_8bpp_values' by diversity;
     start with the brightest color */
  max_dist = 0;
  for (i = 0; i < n_colors; i ++)
    {
      ColorSpec *c = &orig_colors[i];
      unsigned t = cs_cs_dist (&origin, c);
      
      if (max_dist <= t)
	{
	  max_dist = t;
	  prev = i;
	}
    }
  alloc_order[k ++] = prev;
  mark[prev] = 1;

  while (k < n_colors)
    {
      int next = -1;
      
      max_dist = 0;
      for (i = 0; i < n_colors; i ++)
	{
	  unsigned t;
	  if (mark[i])
	    continue;
	  
	  t = cs_cs_dist (&orig_colors[i],
			  &orig_colors[prev]);
	  
	  if (max_dist <= t)
	    {
	      max_dist = t;
	      next = i;
	    }
	}
      if (next < 0)
	gui_abort ();
      
      prev = next;
      alloc_order[k ++] = prev;
      mark[prev] = 1;
    }

#if 0
  for (i = 0; i < n_colors; i ++)
    {
      ColorSpec *c;

      c = &orig_colors[alloc_order[i]];
      fprintf (stderr, "%d, %d: (%d %d %d)\n",
	       i, alloc_order[i],
	       CW (c->rgb.red),
	       CW (c->rgb.green),
	       CW (c->rgb.blue));
    }
#endif

  /* now allocate colors */
  for (i = 0; i < n_colors; i ++)
    {
      ColorSpec *c;
      XColor x_color;
      int pixel = alloc_order[i];
      
      c = &orig_colors[pixel];

      x_color.red   = CW (c->rgb.red);
      x_color.green = CW (c->rgb.green);
      x_color.blue  = CW (c->rgb.blue);
      
      x_color.flags = DoRed | DoGreen | DoBlue;
      
      if (XAllocColor (x_dpy, XDefaultColormap (x_dpy, x_screen),
		       &x_color))
	{
	  /* success */
	  _cmap_mapping[pixel] = x_color.pixel;
	  x_alloced[x_color.pixel] = 1;
	  n_alloced ++;
	}
      else
	{
	  /* failure */
	  failure[pixel] = 1;
	}
    }

#if 0
  fprintf (stderr, "first pass %d allocated\n", n_alloced);
#endif

  /* we use the contents of the x colormap to aid in matching mac
     colors to x colors */
  for (i = 0; i < n_colors; i ++)
    x_cmap[i].pixel = i;
  XQueryColors (x_dpy, XDefaultColormap (x_dpy, x_screen),
		x_cmap, n_colors);

  /* Note the shifted versions of those colors. */
  for (j = NELEM (shifted_x_cmap) - 1; j >= 0; j--)
    {
      shifted_x_cmap[j].red   = x_cmap[j].red >> 1;
      shifted_x_cmap[j].green = x_cmap[j].green >> 1;
      shifted_x_cmap[j].blue  = x_cmap[j].blue >> 1;
    }

  /* for any color that hasn't been allocated, find the closest
     match in the colortable, and try to allocate that */
  if (n_alloced < n_colors)
    {
      for (i = 0; i < n_colors; i ++)
	{
	  if (failure[i])
	    {
	      ColorSpec *c = &orig_colors[i];
	      unsigned min_dist = MAX_CDIST;
	      unsigned min = 0;
	      int shifted_c_red, shifted_c_green, shifted_c_blue;
	      int j;
	      
	      shifted_c_red   = CW (c->rgb.red) >> 1;
	      shifted_c_green = CW (c->rgb.green) >> 1;
	      shifted_c_blue  = CW (c->rgb.blue) >> 1;
	      
	      /* find the closest color in the colortable */
	      for (j = 0; j < n_colors; j ++)
		{
		  unsigned t;

		  /* don't try to re-allocate colors we have already
		     allocated */
		  if (x_alloced[j])
		    continue;
		  t = CS_X_DIST (shifted_c_red,
				 shifted_c_green,
				 shifted_c_blue,
				 shifted_x_cmap[j]);
		  if (min_dist <= t)
		    {
		      min_dist = t;
		      min = j;
		    }
		}
	      if (min < 0)
		gui_abort ();
	      if (XAllocColor (x_dpy, XDefaultColormap (x_dpy, x_screen),
			       &x_cmap[min]))
		{
		  _cmap_mapping[i] = x_cmap[min].pixel;
		  failure[i] = 0;
		  x_alloced[x_cmap[min].pixel] = 1;
		  n_alloced ++;
		}
	    }
	}
    }
  
#if 0
  fprintf (stderr, "second pass %d allocated\n", n_alloced);
#endif

#if 0
  /* match the remaining colors to ones we were able to allocate.
     these colors will not be unique */
  for (i = 0; i < n_colors; i ++)
    {
      if (failure[i])
	compute_new_mapping (i, &ctab_8bpp_values[i]);
    }
#endif

  x_cmap_initialized_p = TRUE;
}

static ColorSpec cmap[256];
static uint8 depth_table_space[DEPTHCONV_MAX_TABLE_SIZE];

void
Executor::vdriver_get_colors (int first_color, int num_colors,
		    ColorSpec *colors)
{
  gui_fatal ("`!vdriver_fixed_clut_p' and `vdriver_get_colors ()' called");
}


void
Executor::vdriver_set_colors (int first_color, int num_colors,
		    const ColorSpec *colors)
{
  int i;
  
  if (x_fbuf_bpp > 8)
    {
      if (vdriver_bpp > 8)
	{
	  const rgb_spec_t *mac_rgb_spec;
	  
	  mac_rgb_spec = ((vdriver_bpp == 16)
			  ? &mac_16bpp_rgb_spec
			  : &mac_32bpp_rgb_spec);
	  
	  conversion_func
	    = depthconv_make_rgb_to_rgb_table (depth_table_space, NULL,
					       mac_rgb_spec, &x_rgb_spec);
	}
      else
	{
	  memcpy (&cmap[first_color], colors, num_colors * sizeof *colors);
	  
	  conversion_func
	    = depthconv_make_ind_to_rgb_table (depth_table_space, vdriver_bpp,
					       NULL, colors, &x_rgb_spec);
	  vdriver_update_screen (0, 0, vdriver_height, vdriver_width, FALSE);
	}
    }
  else
    {
      if (private_cmap_p)
	{
	  XColor x_colors[256];
	  
	  if (color_map == 0)
	    {
	      color_map = XCreateColormap (x_dpy, x_window,
					   visual->visual, AllocAll);
	      XSetWindowColormap (x_dpy, x_window, color_map);
	    }
	  
	  for (i = 0; i < num_colors; i ++)
	    {
	      x_colors[i].pixel = first_color + i;
	      
	      x_colors[i].red   = CW (colors[i].rgb.red);
	      x_colors[i].green = CW (colors[i].rgb.green);
	      x_colors[i].blue  = CW (colors[i].rgb.blue);
	      
	      x_colors[i].flags = DoRed | DoGreen | DoBlue;
	    }
	  
	  XStoreColors (x_dpy, color_map, x_colors, num_colors);
	}
      else
	{
	  if (!x_cmap_initialized_p)
	    init_x_cmap ();
	  
	  for (i = 0; i < num_colors; i ++)
	    compute_new_mapping (first_color + i, &colors[i]);

	  cmap_mapping = _cmap_mapping;
	  
	  conversion_func
	    = depthconv_make_raw_table (depth_table_space, vdriver_bpp,
					x_fbuf_bpp, NULL, cmap_mapping);
	  vdriver_update_screen (0, 0, vdriver_height, vdriver_width, FALSE);
	}
    }
  XSync (x_dpy, False);
  
  colors_initialized_p = TRUE;
}

int
Executor::vdriver_update_screen_rects (int num_rects, const vdriver_rect_t *r,
			     boolean_t cursor_p)
{
  boolean_t convert_p;
  int i;
  
  convert_p = (x_fbuf_bpp == vdriver_bpp
	       && conversion_func == NULL);

  if (! convert_p)
    {
      /* we need to convert the mac screen to something the
	 x screen can take */
      /* allocate the double buffer */
      if (x_fbuf == NULL)
	{
	  alloc_x_image (x_fbuf_bpp, max_width, max_height,
			 &x_fbuf_row_bytes, &x_x_image, &x_fbuf, &x_fbuf_bpp);
	  if (x_fbuf_bpp > 8)
	    {
	      assert (conversion_func != NULL);
	    }
	  else
	    {
	      conversion_func
		= depthconv_make_raw_table (depth_table_space, vdriver_bpp,
					    x_fbuf_bpp, NULL, cmap_mapping);
	    }
	}
    }
  for (i = 0; i < num_rects; i ++)
    {
      int top, left, bottom, right;
      
      top    = r[i].top;
      left   = r[i].left;
      bottom = r[i].bottom;
      right  = r[i].right;
      
      if (convert_p)
	(*x_put_image) (x_dpy, x_window, copy_gc, x_image,
			left, top, left, top, right - left, bottom - top);
      else
	{
	  if (conversion_func)
	    (*conversion_func) (depth_table_space, vdriver_fbuf,
				vdriver_row_bytes, x_fbuf, x_fbuf_row_bytes,
				top, left, bottom, right);
	  
	  (*x_put_image) (x_dpy, x_window, copy_gc, x_x_image,
			  left, top, left, top, right - left, bottom - top);
	}
      
    }
  return 0;
}

int
Executor::vdriver_update_screen (int top, int left, int bottom, int right,
		       boolean_t cursor_p)
{
  vdriver_rect_t r;
  
  if (top < 0)
    top = 0;
  if (left < 0)
    left = 0;
  
  if (bottom > vdriver_height)
    bottom = vdriver_height;
  if (right > vdriver_width)
    right = vdriver_width;
  
  r.top    = top;
  r.left   = left;
  r.bottom = bottom;
  r.right  = right;
  
  return vdriver_update_screen_rects (1, &r, cursor_p);
}

void
Executor::vdriver_shutdown (void)
{
  if (x_dpy == NULL)
    return;

  /* no more sigio */
  signal (SIGIO, SIG_IGN);
  
  fcntl (x_fd, F_SETOWN, orig_sigio_owner);
  fcntl (x_fd, F_SETFL, orig_sigio_flag);
  
  XCloseDisplay (x_dpy);
  
  x_dpy = NULL;
}

vdriver_x_mode_t vdriver_x_modes =
{
  /* contiguous_range_p */ TRUE,
  /* num_sizes */ 2,
  {
    /* min */ { 512, 342 },
    /* default maximum */
    /* max */ { VDRIVER_DEFAULT_SCREEN_WIDTH, VDRIVER_DEFAULT_SCREEN_HEIGHT },
  },
};


boolean_t
Executor::vdriver_acceptable_mode_p (int width, int height, int bpp,
			   boolean_t grayscale_p,
			   boolean_t exact_match_p)
{
  if (width == 0)
    width = vdriver_width;
  if (height == 0)
    height = vdriver_height;
  if (bpp == 0)
    bpp = vdriver_bpp;
  
  if (   width > max_width
      || width < 512
      || height > max_height
      || height < 342
      || bpp > max_bpp
      || (   bpp != 8
	  && bpp != 4
	  && bpp != 2
	  && bpp != 1))
    return FALSE;
  return TRUE;
}

boolean_t
Executor::vdriver_set_mode (int width, int height, int bpp, boolean_t grayscale_p)
{
  if (!x_window)
    {
      alloc_x_window (width, height, bpp, grayscale_p);
      return TRUE;
    }
  
  if (width == 0)
    width = vdriver_width;
  else if (width > max_width)
    width = max_width;
  else if (width < VDRIVER_MIN_SCREEN_WIDTH)
    width = VDRIVER_MIN_SCREEN_WIDTH;
  
  if (height == 0)
    height = vdriver_height;
  else if (height > max_height)
    height = max_height;
  else if (height < VDRIVER_MIN_SCREEN_HEIGHT)
    height = VDRIVER_MIN_SCREEN_HEIGHT;
  
  if (bpp == 0)
    {
      bpp = vdriver_bpp;
      if (bpp == 0)
	bpp = MIN (8, vdriver_max_bpp);
    }

  if (!vdriver_acceptable_mode_p (width, height, bpp, grayscale_p,
				  /* ignored */ FALSE))
    return FALSE;
  
  if (width != vdriver_width
      || height != vdriver_height)
    {
      /* resize; the event code will deal with things when the resize
	 event comes through */
      XResizeWindow (x_dpy, x_window, width, height);
    }
  if (bpp != vdriver_bpp)
    {
      /* change depth */
      vdriver_bpp = bpp;
      vdriver_log2_bpp = ROMlib_log2[bpp];
      
      /* compute the new row bytes */
      vdriver_row_bytes = ((width << vdriver_log2_bpp) + 31) / 32 * 4;
      /* ### i'm not sure we can just change the `bytes_per_line'
	 field of a XImage structure and have it automagically work;
	 but this will only happen when the allocated depth is greater
	 than the `native' x depth, which probably doesn't happen
	 right now */
      if (vdriver_bpp == x_fbuf_bpp
	  && vdriver_row_bytes != fbuf_allocated_row_bytes)
	warning_unexpected ("dubious assignment to `x_image->bytes_per_line'");
      x_image->bytes_per_line = vdriver_row_bytes;
      
      /* ### if the bpp is greater than eight, we'll need to set it to
	 something other than `0xFF' */
      memset (vdriver_fbuf, 0xFF, vdriver_row_bytes * height);
      
      /* invalidate the conversion function */
      conversion_func = NULL;
    }
  
  if (vdriver_grayscale_p != grayscale_p)
    {
      vdriver_grayscale_p = grayscale_p;
      
      /* invalidate the conversion function */
      conversion_func = NULL;
    }
  
  /* Compute the rgb spec. */
  vdriver_rgb_spec = (conversion_func == NULL
		      ? (visual->c_class == TrueColor
			 ? &x_rgb_spec
			 : NULL)
		      : (vdriver_bpp == 32
			 ? &mac_32bpp_rgb_spec
			 : (vdriver_bpp == 16
			    ? &mac_16bpp_rgb_spec
			    : NULL)));

  return TRUE;
}

vdriver_accel_result_t
Executor::vdriver_accel_rect_fill (int top, int left, int bottom,
			 int right, uint32 color)
{
  XGCValues gc_values;
  uint32 x_color;

  /* Don't use accel fills in refresh mode, and don't bother if that
   * rect is going to be transferred to the screen anyway (this happens
   * often when windows get drawn).
   */
  if (ROMlib_refresh || dirty_rect_subsumed_p (top, left, bottom, right))
    return VDRIVER_ACCEL_NO_UPDATE;
  
  if (cmap_mapping)
    x_color = cmap_mapping[color];
  else if (x_fbuf_bpp > 8)
    {
      if (vdriver_bpp > 8)
	{
	  rgb_spec_t *mac_rgb_spec = (vdriver_bpp == 32
				      ? &mac_32bpp_rgb_spec
				      : &mac_16bpp_rgb_spec);
	  RGBColor rgb_color;
	  
	  (*mac_rgb_spec->pixel_to_rgbcolor) (mac_rgb_spec,
					      color,
					      &rgb_color);
	  
	  x_color = (*x_rgb_spec.rgbcolor_to_pixel) (&x_rgb_spec,
						     &rgb_color,
						     TRUE);
	}
      else
	{
	  x_color = (*x_rgb_spec.rgbcolor_to_pixel) (&x_rgb_spec,
						     &cmap[color].rgb,
						     TRUE);
	}
    }
  else
    x_color = color;
  
  dirty_rect_update_screen ();
  gc_values.foreground = x_color;
  XChangeGC (x_dpy, accel_gc, GCForeground, &gc_values);
  XFillRectangle (x_dpy, x_window, accel_gc, left, top,
		  right - left, bottom - top);
  
  return VDRIVER_ACCEL_HOST_SCREEN_UPDATE_ONLY;
}

/* stuff from x.c */

void
Executor::host_beep_at_user ( void )
{
  /* 50 for now */
  XBell (x_dpy, 0);
}

void
Executor::PutScrapX (OSType type, LONGINT length, char *p, int scrap_count)
{
  if (type == TICK ("TEXT"))
    {
      if (selectiontext)
	free (selectiontext);
      selectiontext = (char*)malloc (length);
      if (selectiontext)
	{
	  selectionlength = length;
	  memcpy (selectiontext, p, length);
	  {
	    char *ip, *ep;

	    for (ip = selectiontext, ep = ip + length; ip < ep; ++ip)
	      if (*ip == '\r')
		*ip = '\n';
	  }
	  XSetSelectionOwner (x_dpy, XA_PRIMARY, x_window, CurrentTime);
	}
    }
}

void
WeOwnScrapX (void)
{
  XSetSelectionOwner (x_dpy, XA_PRIMARY, x_window, CurrentTime);
}

int
Executor::GetScrapX (OSType type, Handle h)
{
  int retval;
  
  retval = -1;
  if (type == TICK ("TEXT"))
    {
      Window selection_owner;

      selection_owner = XGetSelectionOwner (x_dpy, XA_PRIMARY);
      if (selection_owner != None && selection_owner != x_window)
	{
	  XEvent xevent;
	  Atom rettype;
	  LONGINT actfmt;
	  unsigned long nitems, ul;
	  unsigned long nbytesafter;
	  unsigned char *propreturn;
	  int i;
	  Ptr p;

	  XConvertSelection (x_dpy, XA_PRIMARY, XA_STRING, None, x_window,
			     CurrentTime);
	  for (i = 10;
	       (--i >= 0
		&& !XCheckTypedEvent (x_dpy, SelectionNotify, &xevent));
	       Delay (10L, (LONGINT *) 0))
	    ;
	  if (i >= 0 && xevent.xselection.property != None)
	    {
	      XGetWindowProperty (x_dpy, xevent.xselection.requestor,
				  xevent.xselection.property, 0L, 16384L,
				  (LONGINT) True, XA_STRING, &rettype, &actfmt,
				  &nitems, &nbytesafter, &propreturn);
	      if (rettype == XA_STRING && actfmt == 8)
		{
		  SetHandleSize ((Handle) h, nitems);
		  if (MemErr == CWC(noErr))
		    {
		      memcpy (MR (*h), propreturn, nitems);
		      for (ul = nitems, p = MR (*h); ul > 0; ul--)
			if (*p++ == '\n')
			  p[-1] = '\r';
		      retval = nitems;
		    }
		}
	      XFree ((char *) propreturn);
	    }
	}
    }
  return retval;
}


void
ROMlib_SetTitle (char *newtitle)
{
  XSizeHints xsh;

  memset (&xsh, 0, sizeof xsh);
  XSetStandardProperties (x_dpy, x_window, newtitle, newtitle, None,
			  nullptr, 0, &xsh);
}

char *
ROMlib_GetTitle (void)
{
  char *retval;

  XFetchName (x_dpy, x_window, &retval);
  return retval;
}

void
ROMlib_FreeTitle (char *title)
{
  XFree (title);
}

int
lookupkeysymX (char *evt)
{
  return XLookupKeysym ((XKeyEvent *) evt, 0);
}

void
Executor::autorepeatonX (void)
{
  XAutoRepeatOn (x_dpy);
  XSync (x_dpy, 0);
}

void
Executor::querypointerX (int *xp, int *yp, int *modp)
{
  Window dummy_window;
  Window child_window;
  int dummy_int;
  unsigned int mods;
  
  XQueryPointer (x_dpy, x_window, &dummy_window,
		 &child_window, &dummy_int, &dummy_int,
		 xp, yp, &mods);
  *modp = X_TO_MAC_STATE (mods);
}

/* host functions that should go away */

void
Executor::host_flush_shadow_screen (void)
{
  int top_long, left_long, bottom_long, right_long;

  /* Lazily allocate a shadow screen.  We won't be doing refresh that often,
   * so don't waste the memory unless we need it.
   */
  if (shadow_fbuf == NULL)
    {
      shadow_fbuf = (unsigned char*) malloc (fbuf_size);
      memcpy (shadow_fbuf, vdriver_fbuf, vdriver_row_bytes * vdriver_height);
      vdriver_update_screen (0, 0, vdriver_height, vdriver_width, FALSE);
    }
  else if (find_changed_rect_and_update_shadow ((uint32 *) vdriver_fbuf,
						(uint32 *) shadow_fbuf,
						(vdriver_row_bytes
						 / sizeof (uint32)),
						vdriver_height,
						&top_long, &left_long,
						&bottom_long, &right_long))
    {
      vdriver_update_screen (top_long, (left_long * 32) >> vdriver_log2_bpp,
			     bottom_long,
			     (right_long * 32) >> vdriver_log2_bpp, FALSE);
    }
}
