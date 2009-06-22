#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_MacViewClass[] =
    "$Id: MacViewClass.m,v 2.34 1997/05/09 09:51:14 ctm Exp $";
#endif

#define Cursor NeXT_Cursor
#include "rsys/common.h"
#import  "MacViewClass.h"
#undef Cursor
#include "ToolboxEvent.h"
#include "rsys/vdriver.h"
#include "rsys/depthconv.h"
#include "rsys/cquick.h"
#include "rsys/dirtyrect.h"
#include "rsys/next.h"
#include "rsys/nextprint.h"
#include "rsys/blockinterrupts.h"
#include "rsys/license.h"
#include "rsys/refresh.h"
#include "rsys/print.h"
#include "rsys/flags.h"
#include "rsys/PSstrings.h"
#include "ourstuff.h"
#include "OSEvent.h"
#include "rsys/osevent.h"
#include "contextswitch.h"
#include "rsys/executor.h"
#include "MemoryMgr.h"
#include "ScrapMgr.h"
#include <assert.h>

#ifdef OPENSTEP
#include <AppKit/dpsclient.h>
#endif /* OPENSTEP */

@implementation MacViewClass


/* NOTE: This isn't a very good NEXTSTEP object, because much of its
 * data is stored in static variables, instead of in instance
 * variables.  That means we can only have one instantiation.  We do
 * this because we are constrained to adhere to the C semantics of the
 * vdriver interface, and because we have no interest in running two
 * of these simultaneously.
 */


#ifndef OPENSTEP
/* NXBitmapImageReps for the screen in various formats. */
static id rgb_screen_bitmap;  
static id two_bpp_screen_bitmap;
id realcursor, blankcursor;
NXImage *blankcursorimage, *cursorimage;
NXBitmapImageRep *cursorrep, *blankcursorrep;
static void timerswitch (DPSTimedEntry notused, double timenow, void *data);
static DPSTimedEntry timer;

#else /* OPENSTEP */

/* NSBitmapImageReps for the screen in various formats. */
static NSBitmapImageRep *rgb_screen_bitmap;  
static NSBitmapImageRep *two_bpp_screen_bitmap;
NSCursor *realcursor, *blankcursor;
NSImage *blankcursorimage, *cursorimage;
NSBitmapImageRep *cursorrep, *blankcursorrep;
NSTimer *our_timer = 0;

#endif /* OPENSTEP */

static id current_screen_bitmap; /* An alias for one of the other three */
static id one_bpp_screen_bitmap;

/* Do we need to convert from internal screen to real screen? */
static boolean_t screen_needs_conversion_p;

/* This is here for easy access by C functions. */
static MacViewClass *self_view;

/* Function to convert the screen to NeXT format. */
static depthconv_func_t conversion_func = NULL;

/* Space for depth conversion tables. */
static uint8 depth_table_space[DEPTHCONV_MAX_TABLE_SIZE];

/* Color mapping table. */
static ColorSpec color_table[256];

/* Pointer to base memory for the NeXT-format frame buffer. */
static uint32 *ns_fbuf;

/* Pointer to base memory for the shadow screen (for refresh). */
static uint32 *shadow_fbuf;

/* Row bytes for the NeXT-format frame buffer. */
static uint32 ns_fbuf_row_bytes;

/* Size of allocated internal frame buffer. */
static uint32 fbuf_size;


/* These variables are required by the vdriver interface. */

uint8 *vdriver_fbuf;

int vdriver_row_bytes;

int vdriver_width = VDRIVER_DEFAULT_SCREEN_WIDTH;

int vdriver_height = VDRIVER_DEFAULT_SCREEN_HEIGHT;

int vdriver_bpp, vdriver_log2_bpp;

int vdriver_max_bpp, vdriver_log2_max_bpp;

rgb_spec_t *vdriver_rgb_spec = NULL;


static rgb_spec_t ns_rgb_spec;

vdriver_nextstep_mode_t vdriver_nextstep_modes;

/* For now, just force black and white cursors. */
int host_cursor_depth = 1;

/* True iff our display is two bit grayscale. */
static boolean_t two_bit_grayscale_display_p;

/* True iff we should be drawing in grayscale. */
boolean_t vdriver_grayscale_p;

/* True iff the CLUT cannot be changed. */
boolean_t vdriver_fixed_clut_p;

#if defined (OPENSTEP)
static void
create_our_timer (id who)
{
  SEL temp_sel;
  NSMethodSignature *temp_sig;
  NSInvocation *invoke_step;

  temp_sel = @selector (step);
  temp_sig = [who methodSignatureForSelector:temp_sel];
  invoke_step = [NSInvocation invocationWithMethodSignature:temp_sig];
  [invoke_step setSelector:temp_sel];
  [invoke_step setTarget:who];
  our_timer = [NSTimer scheduledTimerWithTimeInterval:.00000001
    invocation:invoke_step repeats:YES];
}
#endif

#if !defined (OPENSTEP)

- initFrame:(const NXRect *)frameRect
{
  NXSize size;
  char *datap;
  int bytecount;

  [super initFrame:frameRect];

  [self setClipping:NO]; /* important for printing */

  [self setFlipped:NO];
  [self setAutodisplay:YES];

  /* Since we will be repeatedly focused on, allocate a gstate. */
  [self allocateGState];

  two_bit_grayscale_display_p
    = ([NXApp colorScreen]->depth == NX_TwoBitGrayDepth);

  vdriver_fixed_clut_p = two_bit_grayscale_display_p;
  vdriver_grayscale_p = two_bit_grayscale_display_p;

  /* Set up our max bpp appropriately. */
  if (two_bit_grayscale_display_p)
    vdriver_max_bpp = 2;
  else
    vdriver_max_bpp = 8;
  vdriver_log2_max_bpp = ROMlib_log2[vdriver_max_bpp];

  current_screen_bitmap = nil;
  self_view = self;

  vdriver_width = frameRect->size.width;
  vdriver_height = frameRect->size.height;

  /* make the cursor */

  cursorrep = [NXBitmapImageRep allocFromZone:[self zone]];
  [cursorrep initData: 0 pixelsWide: 16 pixelsHigh:16
   bitsPerSample: 1 samplesPerPixel: 2 hasAlpha: YES isPlanar:YES
   colorSpace: NX_OneIsBlackColorSpace bytesPerRow:0
   bitsPerPixel:0];
  memset ([cursorrep data], 0, [cursorrep bytesPerRow] * 16 * 2);

  size.width = size.height = 16;
  cursorimage = [[NXImage allocFromZone: [self zone]] initSize:&size];
  [cursorimage setDataRetained:YES];
  [cursorimage setScalable:NO];
  [cursorimage useRepresentation:cursorrep];
  realcursor = [NXCursor allocFromZone:[self zone]];
  [realcursor initFromImage:cursorimage];

  /* make the blank cursor (for when it's hidden) */

  blankcursorrep =[NXBitmapImageRep allocFromZone:[self zone]];
  [blankcursorrep initData: 0 pixelsWide: 16 pixelsHigh:16
   bitsPerSample: 1 samplesPerPixel: 2 hasAlpha: YES isPlanar:YES
   colorSpace: NX_OneIsBlackColorSpace bytesPerRow:0 bitsPerPixel:0];

  datap = (char *)[blankcursorrep data];
  bytecount = [blankcursorrep bytesPerRow] * 16;
  memset (datap, 0, bytecount);
  memset (datap + bytecount, ~0, 16 * sizeof (short));

  size.width = size.height = 16;
  blankcursorimage = [[NXImage allocFromZone: [self zone]] initSize:&size];
  [blankcursorimage setDataRetained:YES];
  [blankcursorimage setScalable:NO];
  [blankcursorimage useRepresentation:blankcursorrep];
  blankcursor = [NXCursor allocFromZone:[self zone]];
  [blankcursor initFromImage:blankcursorimage];

  DPSSetDeadKeysEnabled (DPSGetCurrentContext (), 0);

  timer = DPSAddTimedEntry (0, &timerswitch, self, NX_BASETHRESHOLD);

  return self;
}

#else /* OPENSTEP */

- (id) initWithFrame:(NSRect)frameRect
{
  NSSize size;
  char *datap;
  int bytecount;

  [super initWithFrame:frameRect];

  /* Since we will be repeatedly focused on, allocate a gstate. */
  [self allocateGState];

  two_bit_grayscale_display_p
    = NSBitsPerSampleFromDepth([NSWindow defaultDepthLimit]) == 2;

  vdriver_fixed_clut_p = two_bit_grayscale_display_p;
  vdriver_grayscale_p = two_bit_grayscale_display_p;

  /* Set up our max bpp appropriately. */
  if (two_bit_grayscale_display_p)
    vdriver_max_bpp = 2;
  else
    vdriver_max_bpp = 8;
  vdriver_log2_max_bpp = ROMlib_log2[vdriver_max_bpp];

  current_screen_bitmap = nil;
  self_view = self;

  vdriver_width = frameRect.size.width;
  vdriver_height = frameRect.size.height;

  /* make the cursor */

  cursorrep = [NSBitmapImageRep alloc];
  [cursorrep initWithBitmapDataPlanes:NULL pixelsWide: 16 pixelsHigh:16
   bitsPerSample: 1 samplesPerPixel: 2 hasAlpha: YES isPlanar:YES
   colorSpaceName: NSDeviceBlackColorSpace bytesPerRow:0
   bitsPerPixel:0];
  memset ([cursorrep bitmapData], 0, [cursorrep bytesPerRow] * 16 * 2);

  size.width = size.height = 16;
  cursorimage = [[NSImage alloc] initWithSize:size];
  [cursorimage setDataRetained:YES];
  [cursorimage addRepresentation:cursorrep];

  realcursor = [NSCursor alloc];

  {
    NSPoint spot;
 
    spot.x = 1;
    spot.y = 1;  
    [realcursor initWithImage:cursorimage hotSpot:spot];
  }
  /* make the blank cursor (for when it's hidden) */

  blankcursorrep =[NSBitmapImageRep alloc];
  [blankcursorrep initWithBitmapDataPlanes:NULL pixelsWide: 16 pixelsHigh:16
   bitsPerSample: 1 samplesPerPixel: 2 hasAlpha: YES isPlanar:YES
   colorSpaceName: NSDeviceBlackColorSpace bytesPerRow:0 bitsPerPixel:0];

  datap = (char *)[blankcursorrep bitmapData];
  bytecount = [blankcursorrep bytesPerRow] * 16;
  memset (datap, 0, bytecount);
  memset (datap + bytecount, ~0, 16 * sizeof (short));

  size.width = size.height = 16;
  blankcursorimage = [[NSImage alloc] initWithSize:size];
  [blankcursorimage setDataRetained:YES];
  [blankcursorimage addRepresentation:blankcursorrep];
  blankcursor = [NSCursor alloc];
  {
    NSPoint spot;

    spot.x = 1;
    spot.y = 1;
    [blankcursor initWithImage:blankcursorimage hotSpot:spot];
  }

#warning no longer disabling dead keys.  Eventually we should rethink
#warning how Executor handles the keyboard

/*
 * see the comment around line 540 of NEXT.c -- basically we should give
 * the user the option of using the keymappings that are already present
 * on the underlying operating system.
 */
  create_our_timer (self);

  return self;
}

#endif


#ifndef OPENSTEP
- free
{
  [rgb_screen_bitmap free];
  [one_bpp_screen_bitmap free];
  [two_bpp_screen_bitmap free];
  return [super free];
}
#endif

#ifndef OPENSTEP
- drawSelf:(const NXRect *)rects :(int)rectCount
#else /* OPENSTEP */
- (void) drawRect:(NSRect)rect
#endif /* OPENSTEP */
{
  SETUPA5;

  switch (printstate)
    {
    case __idle:
      BLOCK_REAL_INTERRUPTS_EXCURSION
	({
	  if (current_screen_bitmap == nil)
	    {
#ifndef OPENSTEP
	      PSsetgray (NX_WHITE);
	      NXRectFill (&bounds);
#else /* OPENSTEP */
	      PSsetgray (NSWhite);
	      NSRectFill ([self bounds]);
#endif /* OPENSTEP */
	    }
	  else
	    {
#ifndef OPENSTEP
	      if (rectCount == 1)
		NXRectClip (&rects[0]);
	      else
		NXRectClipList (&rects[1], 2);
#else /* OPENSTEP */
	      NSRectClip (rect); /* is this needed? */
#endif /* OPENSTEP */
	      [current_screen_bitmap draw];
	    }
	});
      break;
    case seenOpenPage:
	RESTOREA5;
	do
	    contextswitch(&nextstep_sp, &romlib_sp);
	while (printstate != __idle && printstate != seenClosePage);
	goto avoidrestorea5;
	break;
    default:
	/* seenClosePage may get us in here, but we can't do anything
		without sploding (I think) */
	break;
    }
  RESTOREA5;
avoidrestorea5:
#ifndef OPENSTEP
  return self;
#endif /* not OPENSTEP */
}


boolean_t
vdriver_init (int _max_width, int _max_height, int _max_bpp,
	      boolean_t fixed_p, int *argc, char *argv[])
{
  int width, height, i;
#ifndef OPENSTEP
  NXRect my_frame, tempr;
  Window *window;
#else /* OPENSTEP */
  NSWindow *view_window;
#endif /* OPENSTEP */

  make_rgb_spec (&ns_rgb_spec, 16, FALSE, CLC (0x000F000F),
#if defined (LITTLEENDIAN)
		 4, 4, 4, 0, 4, 12,
#else
		 4, 12, 4, 8, 4, 4,
#endif
		 CL (GetCTSeed ()));

  /* Allocate the NeXT bitmap. */
  width = flag_width ? flag_width : MAX (_max_width, vdriver_width);
  width = MAX (width, VDRIVER_MIN_SCREEN_WIDTH);
  width = (width + 7) & ~7;  /* round width up to multiple of 8. */
  height = flag_height ? flag_height : MAX (_max_height, vdriver_height);
  height = MAX (height, VDRIVER_MIN_SCREEN_HEIGHT);
  if (two_bit_grayscale_display_p)
    ns_fbuf_row_bytes = (width / 4 + 15) & ~15;
  else
    ns_fbuf_row_bytes = (width * 2 + 15) & ~15;

  vdriver_width = width;
  vdriver_height = height;

  /* Size the window to the appropriate size. */
#ifndef OPENSTEP
  window = [self_view window];
  [window getFrame:&my_frame];
  [window sizeWindow:width :height];
  [window moveTopLeftTo: my_frame.origin.x
   :my_frame.origin.y + my_frame.size.height];
  [self_view getFrame:&tempr];
  [self_view convertRect:&tempr toView:nil];
  [window makeKeyAndOrderFront:self_view];
  [window setTrackingRect:&tempr inside:YES owner:self_view
   tag:1 left:NO right:NO];
  [window useOptimizedDrawing:YES];
  [window addToEventMask:NX_MOUSEMOVEDMASK];
  [window addToEventMask:NX_MOUSEDRAGGEDMASK];
  [window addToEventMask:NX_FLAGSCHANGEDMASK];
#else /* OPENSTEP */
  view_window = [self_view window];

  {
    NSSize temp_size;

    temp_size.width = width;
    temp_size.height = height;
    [view_window setContentSize:temp_size];
  }

  [view_window makeKeyAndOrderFront:self_view];

#warning need to set up tracking rectangle

  [view_window useOptimizedDrawing:YES];
  [view_window setAcceptsMouseMovedEvents:YES];
#endif /* OPENSTEP */

  ns_fbuf = valloc (((ns_fbuf_row_bytes * height) + 8191) & ~8191);

  /* Allocate the internal bitmap, enough space for max bpp.  Make each
   * rowbytes divisible by 16.
   */
  vdriver_row_bytes = (((width << vdriver_log2_max_bpp) + 127) / 128U) * 16;
  fbuf_size = vdriver_row_bytes * height;
  vdriver_fbuf = valloc (fbuf_size);
  memset (vdriver_fbuf, 0, fbuf_size);

  if (!two_bit_grayscale_display_p)
    {
      uint32 white_pixel;

      white_pixel = ns_rgb_spec.white_pixel;
      for (i = ns_fbuf_row_bytes * height / sizeof (uint32) - 1; i >= 0; i--)
	ns_fbuf[i] = white_pixel;

#ifndef OPENSTEP
      rgb_screen_bitmap
	= [[NXBitmapImageRep alloc] initData:(void *)ns_fbuf
#else /* OPENSTEP */
     
      rgb_screen_bitmap = [NSBitmapImageRep alloc];

      [rgb_screen_bitmap initWithBitmapDataPlanes:(void *)&ns_fbuf
#endif /* OPENSTEP */
	   pixelsWide:width pixelsHigh:height bitsPerSample:4
	   samplesPerPixel:3 hasAlpha:NO isPlanar:NO
#ifndef OPENSTEP
	   colorSpace:NX_RGBColorSpace
#else /* OPENSTEP */
	   colorSpaceName:NSDeviceRGBColorSpace
#endif /* OPENSTEP */
	   bytesPerRow:ns_fbuf_row_bytes bitsPerPixel:16];

      one_bpp_screen_bitmap = nil;
      two_bpp_screen_bitmap = nil;
    }
  else
    {
      memset (ns_fbuf, ~0, ns_fbuf_row_bytes * height);

#ifdef OPENSTEP
      two_bpp_screen_bitmap = [NSBitmapImageRep alloc];

#endif /* OPENSTEP */
      two_bpp_screen_bitmap
#ifndef OPENSTEP
	= [[NXBitmapImageRep alloc] initData:(void *)ns_fbuf
#else /* OPENSTEP */
	= [two_bpp_screen_bitmap initWithBitmapDataPlanes:(void *)&ns_fbuf
#endif /* OPENSTEP */
	   pixelsWide:width pixelsHigh:height bitsPerSample:2
	   samplesPerPixel:1 hasAlpha:NO isPlanar:NO
#ifndef OPENSTEP
	   colorSpace:NX_OneIsWhiteColorSpace
#else /* OPENSTEP */
	   colorSpaceName:NSDeviceWhiteColorSpace
#endif /* OPENSTEP */
	   bytesPerRow:(((width >> 2) + 15) & ~15)
	   bitsPerPixel:2];

      /* No need for one bpp screen...we'll do the conversion ourselves. */
      one_bpp_screen_bitmap = nil;
      rgb_screen_bitmap = nil;
    }

  vdriver_nextstep_modes.size[1].width  = width;
  vdriver_nextstep_modes.size[1].height = height;

  return TRUE;
}


void
vdriver_shutdown (void)
{
}


void
vdriver_opt_register (void)
{
}


static void
flip_2bpp_pixels (const void *table,
		  const uint8 *src_base, int src_row_bytes,
		  uint8 *dst_base, int dst_row_bytes,
		  int top, int left, int bottom, int right)
{
  int bytes_from_left;
  int num_longs, num_rows;
  int src_add, dst_add;
  const uint32 *s;
  uint32 *d;

  bytes_from_left = ((left >> 2) & ~3);
  s = (const uint32 *) (src_base + (top * src_row_bytes) + bytes_from_left);
  d = (uint32 *) (dst_base + (top * dst_row_bytes) + bytes_from_left);

  num_longs = ((right + 15) >> 4) - (left >> 4);

  src_add = src_row_bytes - (num_longs * 4);
  dst_add = dst_row_bytes - (num_longs * 4);

#define FLIP_LOOP(extra)					\
  do {								\
    for (num_rows = bottom - top; num_rows > 0; num_rows--)	\
      {								\
	int n;							\
								\
	for (n = num_longs; (n -= 4) >= 0; )			\
	  {							\
	    *d++ = ~*s++;					\
	    *d++ = ~*s++;					\
	    *d++ = ~*s++;					\
	    *d++ = ~*s++;					\
	  }							\
	extra;							\
								\
	s = (const uint32 *) ((const uint8 *) s + src_add);	\
	d = (uint32 *) ((uint8 *) d + dst_add);			\
      }								\
  } while (0)

  switch (num_longs & 3)
    {
    case 0:
      FLIP_LOOP ((void) 0);
      break;
    case 1:
      FLIP_LOOP (*d++ = ~*s++);
      break;
    case 2:
      FLIP_LOOP (*d++ = ~*s++; *d++ = ~*s++);
      break;
    case 3:
      FLIP_LOOP (*d++ = ~*s++; *d++ = ~*s++; *d++ = ~*s++);
      break;
    }
}


int
vdriver_update_screen_rects (int num_rects, const vdriver_rect_t *r,
			     boolean_t cursor_p)
{
#ifndef OPENSTEP
  NXRect *nxr;
#else /* OPENSTEP */
  NSRect *nxr;
#endif /* OPENSTEP */
  int i;

  if (num_rects == 0 || current_screen_bitmap == nil)
    return 0;

/*
 * While we're printing, send no bits to the screen.  This is ugly, but
 * that's the way E/NS 1.3x works.
 */

  if (printstate != __idle)
    return 0;

  if (screen_needs_conversion_p)
    {
      if (conversion_func == NULL)
	{
	  if (current_screen_bitmap == rgb_screen_bitmap)
	    conversion_func = (depthconv_make_ind_to_rgb_table
			       (depth_table_space, vdriver_bpp,
				NULL, color_table, &ns_rgb_spec));
	  else if (current_screen_bitmap == two_bpp_screen_bitmap)
	    {
	      if (vdriver_bpp == 1)
		{
		  static const uint32 one_to_two_table[2] = { 0x3, 0 };

		  conversion_func = (depthconv_make_raw_table
				     (depth_table_space, 1, 2,
				      NULL, one_to_two_table));
		}
	      else if (vdriver_bpp == 2)
		{
		  conversion_func = flip_2bpp_pixels;
		}
	      else
		abort ();
	    }
	  else
	    abort ();
	}

      /* Convert from the current format to our internal screen format. */
      for (i = 0; i < num_rects; i++)
	{
	  int top, left, bottom, right;
	  
	  top    = r[i].top;
	  left   = r[i].left;
	  bottom = r[i].bottom;
	  right  = r[i].right;
	  
	  (*conversion_func) (depth_table_space, vdriver_fbuf,
			      vdriver_row_bytes, (uint8 *) ns_fbuf,
			      ns_fbuf_row_bytes, top, left, bottom, right);
	}
    }

  /* Set up to transfer to the real screen. */
#ifndef OPENSTEP
  nxr = (NXRect *) alloca (num_rects * sizeof nxr[0]);
#else /* OPENSTEP */
  nxr = (NSRect *) alloca (num_rects * sizeof nxr[0]);
#endif /* OPENSTEP */
  for (i = 0; i < num_rects; i++)
    {
      int left, top, height;
      left = r[i].left;
      nxr[i].origin.x    = left;
      nxr[i].size.width  = r[i].right - left;
      top = r[i].top;
      height = r[i].bottom - top;
      nxr[i].size.height = height;
      nxr[i].origin.y    = vdriver_height - top - height;
    }

  BLOCK_REAL_INTERRUPTS_EXCURSION
    ({
      [self_view lockFocus];
#ifndef OPENSTEP
      NXRectClipList (nxr, num_rects);
#else /* OPENSTEP */

      /* I'm not sure where Mat learned about this call */
      NSRectClipList (nxr, num_rects);

#endif /* OPENSTEP */
      [current_screen_bitmap draw];
      [self_view unlockFocus];
    });

  return 0;
}


int
vdriver_update_screen (int top, int left, int bottom, int right,
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

static void
repaint_screen (void)
{
  dirty_rect_accrue (0, 0, vdriver_height, vdriver_width);
  dirty_rect_update_screen ();
}

void
vdriver_set_colors (int first_color, int num_colors, const ColorSpec *colors)
{
  if (!vdriver_fixed_clut_p
      && memcmp (&color_table[first_color], colors,
		 num_colors * sizeof colors[0]))
    {
      memcpy (&color_table[first_color], colors,
	      num_colors * sizeof colors[0]);
      conversion_func = NULL;  /* Invalidate old conversion stuff. */
      repaint_screen ();
      vdriver_flush_display();	/* Make sure changed colors are visible. */
    }
}


boolean_t
vdriver_acceptable_mode_p (int width, int height, int bpp,
			   boolean_t grayscale_p,
			   boolean_t exact_match_p)
{
  if (exact_match_p && two_bit_grayscale_display_p && !grayscale_p)
    return FALSE;

  if (width == 0)
    width = vdriver_width;
  if (height == 0)
    height = vdriver_height;
  if (bpp == 0)
    {
      bpp = vdriver_bpp;
      if (bpp == 0)
	bpp = vdriver_max_bpp;
    }

  if (width < VDRIVER_MIN_SCREEN_WIDTH || width > vdriver_width
      || height < VDRIVER_MIN_SCREEN_HEIGHT || height > vdriver_height
      || bpp < 1 || bpp > vdriver_max_bpp
      || (bpp != 1 && bpp != 2 && bpp != 4 && bpp != 8))
    return FALSE;

  return TRUE;
}


boolean_t
vdriver_set_mode (int width, int height, int bpp, boolean_t grayscale_p)
{
  if (!vdriver_acceptable_mode_p (width, height, bpp, grayscale_p, FALSE))
    return FALSE;

  if (width == 0)
    width = vdriver_width;
  if (height == 0)
    height = vdriver_height;
  if (bpp == 0)
    {
      bpp = vdriver_bpp;
      if (bpp == 0)
	bpp = vdriver_max_bpp;
    }

  vdriver_width = width;
  vdriver_height = height;

  if (bpp != vdriver_bpp)
    {
      static boolean_t first_mode_set_p = TRUE;
      int i;

      /* Change depth. */
      vdriver_bpp = bpp;
      vdriver_log2_bpp = ROMlib_log2[bpp];

      vdriver_row_bytes = ((((width << vdriver_log2_bpp) + 127) / 128U) * 16);

      /* Invalidate the conversion function. */
      conversion_func = NULL;

      if (bpp == 1 && one_bpp_screen_bitmap != nil)
	{
	  current_screen_bitmap = one_bpp_screen_bitmap;
	  vdriver_row_bytes = [one_bpp_screen_bitmap bytesPerRow];
	  screen_needs_conversion_p = FALSE;
	}
      else if ((bpp == 1 || bpp == 2) && two_bpp_screen_bitmap != nil)
	{
	  current_screen_bitmap = two_bpp_screen_bitmap;
	  screen_needs_conversion_p = TRUE;
	}
      else
	{
	  /* Set CLUT to all gray; this makes depths sets look nicer. */
	  for (i = 0; i < 256; i++)
	    color_table[i] = ROMlib_gray_cspec;

	  current_screen_bitmap = rgb_screen_bitmap;
	  screen_needs_conversion_p = TRUE;
	}

      /* Clear the screen. */
      memset (vdriver_fbuf, first_mode_set_p ? 0 : ~0,
	      vdriver_height * vdriver_row_bytes);
      first_mode_set_p = FALSE;

      assert (current_screen_bitmap != nil);
    }

  vdriver_grayscale_p = (two_bit_grayscale_display_p || grayscale_p);

  return TRUE;
}


void
vdriver_get_colors (int first_color, int num_colors,
		    struct ColorSpec *color_array)
{
  if (vdriver_fixed_clut_p)
    {
      static const ColorSpec one_bpp_gray_cspecs[2] = {
	{ CWC (0), { CWC (0xFFFF), CWC (0xFFFF), CWC (0xFFFF) } },
	{ CWC (1), { CWC (0x0000), CWC (0x0000), CWC (0x0000) } },
      };
      static const ColorSpec two_bpp_gray_cspecs[4] = {
	{ CWC (0), { CWC (0xFFFF), CWC (0xFFFF), CWC (0xFFFF) } },
	{ CWC (1), { CWC (0xAAAA), CWC (0xAAAA), CWC (0xAAAA) } },
	{ CWC (2), { CWC (0x5555), CWC (0x5555), CWC (0x5555) } },
	{ CWC (3), { CWC (0x0000), CWC (0x0000), CWC (0x0000) } },
      };

      gui_assert (two_bit_grayscale_display_p);
      gui_assert (first_color >= 0 && first_color + num_colors <= 4);

      memcpy (color_array,
	      ((vdriver_bpp == 2)
	       ? &two_bpp_gray_cspecs[first_color]
	       : &one_bpp_gray_cspecs[first_color]),
	      num_colors * sizeof color_array[0]);
    }
  else
    {
      gui_fatal ("should not be asking for colors!  CLUT not fixed");
    }
}


vdriver_nextstep_mode_t vdriver_nextstep_modes =
{
  /* contiguous_range_p */ TRUE,
  /* num_sizes */ 2,
  {
    /* min */ { VDRIVER_MIN_SCREEN_WIDTH, VDRIVER_MIN_SCREEN_HEIGHT },
    /* default maximum */
    /* max */ { VDRIVER_DEFAULT_SCREEN_WIDTH, VDRIVER_DEFAULT_SCREEN_HEIGHT },
  },
};


void
vdriver_flush_display (void)
{
  /* If you take this out, Speedometer 3.23 starts paging massively
   * during the 8-bit color QuickDraw tests.  It appears that tons
   * of stuff starts getting queued up via the Mach message passing
   * mechanisms, although that's a very rough guess.  Always flushing
   * makes it consistently slow but we don't get massive paging.
   * And of course we only call this in animation mode.
   */
  BLOCK_REAL_INTERRUPTS_EXCURSION
    ({
#ifndef OPENSTEP
      DPSFlush ();
#else /* OPENSTEP */
      [[NSDPSContext currentContext] flush];
#endif /* OPENSTEP */
    });
}


void
host_beep_at_user (void)
{
#ifndef OPENSTEP
  NXBeep ();
#else /* OPENSTEP */
  NSBeep ();
#endif /* OPENSTEP */
}


void 
querypointerX (long *xp, long *yp, long *notused)
{
#ifndef OPENSTEP
  NXPoint p;
#else /* OPENSTEP */
  NSPoint p;
#endif /* OPENSTEP */

  BLOCK_REAL_INTERRUPTS_EXCURSION
    ({
#ifndef OPENSTEP
      [[self_view window] getMouseLocation:&p];
      [self_view convertPoint: &p fromView:nil];
#else /* OPENSTEP */
      p = [[self_view window] mouseLocationOutsideOfEventStream];
#endif /* OPENSTEP */
      *xp = p.x;
      *yp = (vdriver_height - p.y);
    });
}

void 
showcursorX (long show)
{
  BLOCK_REAL_INTERRUPTS_EXCURSION
    ({
      if (show)
	[realcursor set];
      else
	[blankcursor set];
    });
}

void 
setcursorX (short *data, short *mask, long hotx, long hoty)
{
  char *datap;
  short i;
  short mymask[16], mydata[16], gray_bits;
#ifndef OPENSTEP
  NXPoint p;
#else /* OPENSTEP */
  NSPoint p;
#endif /* OPENSTEP */
  static char beenhere = NO;

  if (hotx < 0)
    hotx = 0;
  else if (hotx > 16)
    hotx = 16;

  if (hoty < 0)
    hoty = 0;
  else if (hoty > 16)
    hoty = 16;

  BLOCK_REAL_INTERRUPTS_EXCURSION
    ({
      if (!beenhere)
	{
	  [[self_view window] disableCursorRects];
	  beenhere = YES;
	}
      gray_bits = 0x5555;
      for (i = 0; i < 16; ++i)
	{
	  mymask[i] = ~(data[i] | mask[i]);
	  gray_bits = ~gray_bits;
	  mydata[i] = data[i] & ~(gray_bits & (~mask[i] & data[i]));
	}
      
#ifndef OPENSTEP
      datap = (char *)[cursorrep data];
#else /* OPENSTEP */
      datap = (char *)[cursorrep bitmapData];
#endif /* OPENSTEP */
      memcpy (datap, mydata, 32);
      memcpy (datap + 32, mymask, 32);
      [cursorimage recache];
      if (CrsrVis)
	[blankcursor set];

#ifndef OPENSTEP
      [realcursor setImage:cursorimage];
#endif

      p.x = hotx;
      p.y = hoty;

#ifndef OPENSTEP
      [realcursor setHotSpot:&p];
#else /* OPENSTEP */
      [realcursor initWithImage:cursorimage hotSpot:p];
#endif /* OPENSTEP */
      
      if (CrsrVis)
	[realcursor set];
    });
}
     
/*
 * ROMlib_printtimeout is specifically for Excel, when you try to
 * print an empty page.  Excel starts the printing process and then
 * writes an message to the screen, informing you that you have a
 * blank worksheet.  This doesn't fit in with our printing paradigm,
 * so ROMlib_printtimeout is a hack to solve this one instance of the
 * general class of printing problems.
 *
 * ROMlib_printtimeout is positive when we're counting context
 *      switches, looking for an OpenPage.
 *
 * ROMlib_printtimeout is zero if we've timed out, but haven't
 *      yet finished printing from the NeXT's perspective.
 *
 * ROMlib_printtimeout is negative when we've finished printing.
 *      This will result in the screen being updated once we
 *      drop back into the Mac universe.
 */

long ROMlib_printtimeout = 10000;	/* any positive number will do */

- step
{
#ifndef OPENSTEP
  NXEvent dummyEvent;
#else /* OPENSTEP */
/* NOTE: we may need to be doing SETUPA5 now as well as protect us stuff,
	since timerswitch is no longer called */
#endif /* OPENSTEP */

  [self lockFocus];

  do
    {
      contextswitch (&nextstep_sp, &romlib_sp);
    }
#ifndef OPENSTEP
  while (printstate != seenOpenDoc && printstate != seenPageSetUp &&
         [NXApp peekNextEvent: NX_ALLEVENTS into:&dummyEvent] == NULL);
#else /* OPENSTEP */
  while (printstate != seenOpenDoc && printstate != seenPageSetUp
    && [NSApp nextEventMatchingMask:NSAnyEventMask
	untilDate:[NSDate distantPast]
	inMode:NSDefaultRunLoopMode
	dequeue:NO] == nil
  );
#endif /* OPENSTEP */

  [self unlockFocus];
  {
    SETUPA5;
    if (printstate == seenOpenDoc)
      {
#ifndef OPENSTEP
        [self printPSCode:self];
#else /* OPENSTEP */
        [self print:self];
#endif /* OPENSTEP */
	if (ROMlib_printtimeout == 0)
	  ROMlib_printtimeout = -1;
      }
    else if (printstate == seenPageSetUp)
      {
#ifndef OPENSTEP
        [NXApp runPageLayout:self];
#else /* OPENSTEP */
        [NSApp runPageLayout:self];
#endif /* OPENSTEP */
	printstate = __idle;
      }
    RESTOREA5;
  }

  return self;
}

#ifndef OPENSTEP
static void
timerswitch (DPSTimedEntry notused, double timenow, void *data)
{
  static double oldnow;

  SETUPA5;
  if (oldnow == 0.0 || timenow - oldnow > 5 * 50)
    {
      protectus (0, 0);
      oldnow = timenow;
    }
  /* might be useful to look at timenow and update various low memory
     globals */
  [(id) data step];
  RESTOREA5;
}


#endif /* not OPENSTEP */
typedef enum { MacToUNIX, UNIXToMac, MacRTFToUNIX, UNIXRTFToMac } convertdir_t;

typedef struct {
    long first;
    long second;
} pair_t;

unsigned char mactonext[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    26,
    27,
    28,
    29,
    30,
    31,
    32,
    33,
    34,
    35,
    36,
    37,
    38,
    39,			/* NEXTCHAR_QUOTESINGLE, */
    40,
    41,
    42,
    43,
    44,
    45,
    46,
    47,
    48,
    49,
    50,
    51,
    52,
    53,
    54,
    55,
    56,
    57,
    58,
    59,
    60,
    61,
    62,
    63,
    64,
    65,
    66,
    67,
    68,
    69,
    70,
    71,
    72,
    73,
    74,
    75,
    76,
    77,
    78,
    79,
    80,
    81,
    82,
    83,
    84,
    85,
    86,
    87,
    88,
    89,
    90,
    91,
    92,
    93,
    94,
    95,
    96,			/* NEXTCHAR_GRAVE, */
    97,
    98,
    99,
    100,
    101,
    102,
    103,
    104,
    105,
    106,
    107,
    108,
    109,
    110,
    111,
    112,
    113,
    114,
    115,
    116,
    117,
    118,
    119,
    120,
    121,
    122,
    123,
    124,
    125,
    126,
    127,
    NEXTCHAR_ADIERESIS,
    NEXTCHAR_ARING,
    NEXTCHAR_CCEDILLA,
    NEXTCHAR_EACUTE,
    NEXTCHAR_NTILDE,
    NEXTCHAR_ODIERESIS,
    NEXTCHAR_UDIERESIS,
    NEXTCHAR_aACUTE,
    NEXTCHAR_aGRAVE,
    NEXTCHAR_aCIRCUMFLEX,
    NEXTCHAR_aDIERESIS,
    NEXTCHAR_aTILDE,
    NEXTCHAR_aRING,
    NEXTCHAR_cCEDILLA,
    NEXTCHAR_eACUTE,
    NEXTCHAR_eGRAVE,
    NEXTCHAR_eCIRCUMFLEX,
    NEXTCHAR_eDIERESIS,
    NEXTCHAR_iACUTE,
    NEXTCHAR_iGRAVE,
    NEXTCHAR_iCIRCUMFLEX,
    NEXTCHAR_iDIERESIS,
    NEXTCHAR_nTILDE,
    NEXTCHAR_oACUTE,
    NEXTCHAR_oGRAVE,
    NEXTCHAR_oCIRCUMFLEX,
    NEXTCHAR_oDIERESIS,
    NEXTCHAR_oTILDE,
    NEXTCHAR_uACUTE,
    NEXTCHAR_uGRAVE,
    NEXTCHAR_uCIRCUMFLEX,
    NEXTCHAR_uDIERESIS,
    NEXTCHAR_DAGGER,
    NEXTCHAR_DEGREE,
    NEXTCHAR_CENT,
    NEXTCHAR_STERLING,
    NEXTCHAR_SECTION,
    NEXTCHAR_BULLET,
    NEXTCHAR_PARAGRAPH,
    NEXTCHAR_GERMANDBLS,
    NEXTCHAR_REGISTERED,
    NEXTCHAR_COPYRIGHT,
    NEXTCHAR_TRADEMARK,
    NEXTCHAR_ACUTE,
    NEXTCHAR_DIERESIS,
    NEXTCHAR_NOTEQUAL,
    NEXTCHAR_AE,
    NEXTCHAR_OSLASH,
    NEXTCHAR_INFINITY,
    NEXTCHAR_PLUSMINUS,
    NEXTCHAR_LESSEQUAL,
    NEXTCHAR_GREATEQUAL,
    NEXTCHAR_YEN,
    NEXTCHAR_MU,
    NEXTCHAR_PARTIALDIFF,
    NEXTCHAR_SUMMATION,
    NEXTCHAR_PRODUCT,
    NEXTCHAR_PI,
    NEXTCHAR_INTEGRAL,
    NEXTCHAR_ORDFEMININE,
    NEXTCHAR_ORDMASCULINE,
    NEXTCHAR_OMEGA,
    NEXTCHAR_ae,
    NEXTCHAR_oSLASH,
    NEXTCHAR_QUESTIONDOWN,
    NEXTCHAR_EXCLAMDOWN,
    NEXTCHAR_LOGICALNOT,
    NEXTCHAR_RADICAL,
    NEXTCHAR_FLORIN,
    NEXTCHAR_APPROXEQUAL,
    NEXTCHAR_DELTA,
    NEXTCHAR_GUILLEMOTLEFT,
    NEXTCHAR_GUILLEMOTRIGHT,
    NEXTCHAR_ELLIPSIS,
    NEXTCHAR_FIGSP,
    NEXTCHAR_AGRAVE,
    NEXTCHAR_ATILDE,
    NEXTCHAR_OTILDE,
    NEXTCHAR_OE,
    NEXTCHAR_oe,
    NEXTCHAR_ENDASH,
    NEXTCHAR_EMDASH,
    NEXTCHAR_QUOTEDBLLEFT,
    NEXTCHAR_QUOTEDBLRIGHT,
    NEXTCHAR_GRAVE,			/* NEXTCHAR_QUOTELEFT, */
    NEXTCHAR_QUOTESINGLE,		/* NEXTCHAR_QUOTERIGHT, */
    NEXTCHAR_DIVIDE,
    NEXTCHAR_LOZENGE,
    NEXTCHAR_yDIERESIS,
    NEXTCHAR_YDIERESIS,
    NEXTCHAR_FRACTION,
    NEXTCHAR_CURRENCY,
    NEXTCHAR_GUILSINGLLEFT,
    NEXTCHAR_GUILSINGLRIGHT,
    NEXTCHAR_FI,
    NEXTCHAR_FL,
    NEXTCHAR_DAGGERDBL,
    NEXTCHAR_PERIODCENTERED,
    NEXTCHAR_QUOTESINGLBASE,
    NEXTCHAR_QUOTEDBLBASE,
    NEXTCHAR_PERTHOUSAND,
    NEXTCHAR_ACIRCUMFLEX,
    NEXTCHAR_ECIRCUMFLEX,
    NEXTCHAR_AACUTE,
    NEXTCHAR_EDIERESIS,
    NEXTCHAR_EGRAVE,
    NEXTCHAR_IACUTE,
    NEXTCHAR_ICIRCUMFLEX,
    NEXTCHAR_IDIERESIS,
    NEXTCHAR_IGRAVE,
    NEXTCHAR_OACUTE,
    NEXTCHAR_OCIRCUMFLEX,
    NEXTCHAR_APPLE,
    NEXTCHAR_OGRAVE,
    NEXTCHAR_UACUTE,
    NEXTCHAR_UCIRCUMFLEX,
    NEXTCHAR_UGRAVE,
    NEXTCHAR_DOTLESSI,
    NEXTCHAR_CIRCUMFLEX,
    NEXTCHAR_TILDE,
    NEXTCHAR_MACRON,
    NEXTCHAR_BREVE,
    NEXTCHAR_DOTACCENT,
    NEXTCHAR_RING,
    NEXTCHAR_CEDILLA,
    NEXTCHAR_HUNGARUMLAUT,
    NEXTCHAR_OGONEK,
    NEXTCHAR_CARON
};

unsigned char nexttomac[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    26,
    27,
    28,
    29,
    30,
    31,
    32,
    33,
    34,
    35,
    36,
    37,
    38,
    39,				/* MACCHAR_QUOTERIGHT, */
    40,
    41,
    42,
    43,
    44,
    45,
    46,
    47,
    48,
    49,
    50,
    51,
    52,
    53,
    54,
    55,
    56,
    57,
    58,
    59,
    60,
    61,
    62,
    63,
    64,
    65,
    66,
    67,
    68,
    69,
    70,
    71,
    72,
    73,
    74,
    75,
    76,
    77,
    78,
    79,
    80,
    81,
    82,
    83,
    84,
    85,
    86,
    87,
    88,
    89,
    90,
    91,
    92,
    93,
    94,
    95,
    96,				/* MACCHAR_QUOTELEFT, */
    97,
    98,
    99,
    100,
    101,
    102,
    103,
    104,
    105,
    106,
    107,
    108,
    109,
    110,
    111,
    112,
    113,
    114,
    115,
    116,
    117,
    118,
    119,
    120,
    121,
    122,
    123,
    124,
    125,
    126,
    127,
    MACCHAR_FIGSP,
    MACCHAR_AGRAVE,
    MACCHAR_AACUTE,
    MACCHAR_ACIRCUMFLEX,
    MACCHAR_ATILDE,
    MACCHAR_ADIERESIS,
    MACCHAR_ARING,
    MACCHAR_CCEDILLA,
    MACCHAR_EGRAVE,
    MACCHAR_EACUTE,
    MACCHAR_ECIRCUMFLEX,
    MACCHAR_EDIERESIS,
    MACCHAR_IGRAVE,
    MACCHAR_IACUTE,
    MACCHAR_ICIRCUMFLEX,
    MACCHAR_IDIERESIS,
    MACCHAR_ETH,
    MACCHAR_NTILDE,
    MACCHAR_OGRAVE,
    MACCHAR_OACUTE,
    MACCHAR_OCIRCUMFLEX,
    MACCHAR_OTILDE,
    MACCHAR_ODIERESIS,
    MACCHAR_UGRAVE,
    MACCHAR_UACUTE,
    MACCHAR_UCIRCUMFLEX,
    MACCHAR_UDIERESIS,
    MACCHAR_YACUTE,
    MACCHAR_THORN,
    MACCHAR_MU,
    MACCHAR_MULTIPLY,
    MACCHAR_DIVIDE,
    MACCHAR_COPYRIGHT,
    MACCHAR_EXCLAMDOWN,
    MACCHAR_CENT,
    MACCHAR_STERLING,
    MACCHAR_FRACTION,
    MACCHAR_YEN,
    MACCHAR_FLORIN,
    MACCHAR_SECTION,
    MACCHAR_CURRENCY,
    MACCHAR_QUOTERIGHT,			/* MACCHAR_QUOTESINGLE, */
    MACCHAR_QUOTEDBLLEFT,
    MACCHAR_GUILLEMOTLEFT,
    MACCHAR_GUILSINGLLEFT,
    MACCHAR_GUILSINGLRIGHT,
    MACCHAR_FI,
    MACCHAR_FL,
    MACCHAR_REGISTERED,
    MACCHAR_ENDASH,
    MACCHAR_DAGGER,
    MACCHAR_DAGGERDBL,
    MACCHAR_PERIODCENTERED,
    MACCHAR_BROKENBAR,
    MACCHAR_PARAGRAPH,
    MACCHAR_BULLET,
    MACCHAR_QUOTESINGLBASE,
    MACCHAR_QUOTEDBLBASE,
    MACCHAR_QUOTEDBLRIGHT,
    MACCHAR_GUILLEMOTRIGHT,
    MACCHAR_ELLIPSIS,
    MACCHAR_PERTHOUSAND,
    MACCHAR_LOGICALNOT,
    MACCHAR_QUESTIONDOWN,
    MACCHAR_ONESUPERIOR,
    MACCHAR_QUOTELEFT,			/* MACCHAR_GRAVE, */
    MACCHAR_ACUTE,
    MACCHAR_CIRCUMFLEX,
    MACCHAR_TILDE,
    MACCHAR_MACRON,
    MACCHAR_BREVE,
    MACCHAR_DOTACCENT,
    MACCHAR_DIERESIS,
    MACCHAR_TWOSUPERIOR,
    MACCHAR_RING,
    MACCHAR_CEDILLA,
    MACCHAR_THREESUPERIOR,
    MACCHAR_HUNGARUMLAUT,
    MACCHAR_OGONEK,
    MACCHAR_CARON,
    MACCHAR_EMDASH,
    MACCHAR_PLUSMINUS,
    MACCHAR_ONEQUARTER,
    MACCHAR_ONEHALF,
    MACCHAR_THREEQUARTERS,
    MACCHAR_aGRAVE,
    MACCHAR_aACUTE,
    MACCHAR_aCIRCUMFLEX,
    MACCHAR_aTILDE,
    MACCHAR_aDIERESIS,
    MACCHAR_aRING,
    MACCHAR_cCEDILLA,
    MACCHAR_eGRAVE,
    MACCHAR_eACUTE,
    MACCHAR_eCIRCUMFLEX,
    MACCHAR_eDIERESIS,
    MACCHAR_iGRAVE,
    MACCHAR_AE,
    MACCHAR_iACUTE,
    MACCHAR_ORDFEMININE,
    MACCHAR_iCIRCUMFLEX,
    MACCHAR_iDIERESIS,
    MACCHAR_eTH,
    MACCHAR_nTILDE,
    MACCHAR_LSLASH,
    MACCHAR_OSLASH,
    MACCHAR_OE,
    MACCHAR_ORDMASCULINE,
    MACCHAR_oGRAVE,
    MACCHAR_oACUTE,
    MACCHAR_oCIRCUMFLEX,
    MACCHAR_oTILDE,
    MACCHAR_oDIERESIS,
    MACCHAR_ae,
    MACCHAR_uGRAVE,
    MACCHAR_uACUTE,
    MACCHAR_uCIRCUMFLEX,
    MACCHAR_DOTLESSI,
    MACCHAR_uDIERESIS,
    MACCHAR_yACUTE,
    MACCHAR_lSLASH,
    MACCHAR_oSLASH,
    MACCHAR_oe,
    MACCHAR_GERMANDBLS,
    MACCHAR_tHORN,
    MACCHAR_yDIERESIS,
    MACCHAR_NEXT254,
    MACCHAR_NEXT255
};


void converthex(char **h, unsigned char *table)
{
    unsigned int val, val2;
    char *p;

    p = *h;
    if (*p <= 'f' && *p >= 'a')
	val = *p - 'a' + 10;
    else if (*p <= 'F' && *p >= 'A')
	val = *p - 'A' + 10;
    else
	val = *p - '0';

    if (*(p + 1) <= 'f' && *(p + 1) >= 'a')
	val = val * 16 + *(p + 1) - 'a' + 10;
    else if (*(p + 1) <= 'F' && *(p + 1) >= 'A')
	val = val * 16 + *(p + 1) - 'A' + 10;
    else
	val = val * 16 + *(p + 1) - '0';

    val = table[val];
    val2 = val/16;
    if (val2 > 9)
	*p++ = val2 + 'a' - 10;
    else
	*p++ = val2 + '0';

    val2 = val & 0xF;
    if (val2 > 9)
	*p++ = val2 + 'a' - 10;
    else
	*p++ = val2 + '0';
}

void convertchars(char *data, long length, unsigned char *table)
{
    char *ep;
    int escaped;

    escaped = NO;
    ep = data + length;
    while (data < ep) {
	if (escaped && *data == '\'') {
	    *data++ = '\'';
	    converthex(&data, table);
	}
	if (*data == '\\')
	    escaped = YES;
	else
	    escaped = NO;
	*data++ = table[*(unsigned char *)data];
    }
}

#ifndef OPENSTEP
static int unixrtfconvert(int length, char *ip, char *op)
#else /* OPENSTEP */
static int unixrtfconvert(int length, const char *ip, char *op)
#endif /* OPENSTEP */
{
#ifndef OPENSTEP
    char *fromp, *ep;
#else /* OPENSTEP */
    const char *fromp, *ep;
#endif /* OPENSTEP */
    int retval;

    retval = length;
    fromp = ip;
    ep = ip + length;
    while (fromp < ep) {
	if (*fromp == '\n') {
	    *op++ = '\r';
	    fromp++;
	} else if (*fromp == '\\') {
	    *op++ = '\\';
	    fromp++;
	    if (fromp < ep && *fromp == '\n') {
#if !defined (WEDONTNEEDNOSTEENINGPAR)
		strcpy(op, "par \n");
		op += 5;
		fromp++;
		retval += 4;
#else /* WEDONTNEEDNOSTEENINGPAR */
		*op++ = '\r';
		fromp++;
#endif /*  WEDONTNEEDNOSTEENINGPAR */
	    } else if (fromp + 2 < ep && !strncmp(fromp, "ul0", 3)) {
		strcpy(op, "ulnone");
		op += 6;
		fromp += 3;
		retval += 3;
	    } else if (fromp + 6 < ep && !strncmp(fromp, "fonttbl", 6)) {
		strcpy(op, "fonttbl\\f99\\fa b;");
		op += 17;
		fromp += 7;
		retval += 10;
	    } else
		*op++ = *fromp++;
	} else
	    *op++ = *fromp++;
    }
    return retval;
}

static inline void copyandtranslate( char *cp, char **ipp, char **opp)
{
    *cp = *(*ipp)++;
    if (*cp == '\r')
	*cp = '\n';
    *(*opp)++ = *cp;
}

#define PREFIX	"\\endnhere"
#define NCHARPREFIX	(sizeof(PREFIX)-1)

#define STICKIN				\
    "{\\fonttbl"			\
	"{\\f0\\fswiss Helvetica;}"	\
	"{\\f3\\fmodern Courier;}"	\
	"{\\f4\\fmodern Ohlfs;}"	\
	"{\\f20\\froman Times;}"	\
	"{\\f21\\fswiss Helvetica;}"	\
	"{\\f22\\fmodern Courier;}"	\
    "}\\f0"
#define NCHARSTICKIN	(sizeof(STICKIN)-1)

#ifndef OPENSTEP
static int macrtfconvert(int length, char *ip, char *op)
#else /* OPENSTEP */
static int macrtfconvert(int length, const char *ip, char *op)
#endif /* OPENSTEP */
{
    int retval, index;
    char c;
    SETUPA5;

    retval = length;
    index = 0;
    while (length > 0 && index < NCHARPREFIX) {
	copyandtranslate(&c, &ip, &op);
	if (PREFIX[index] == c)
	    ++index;
        else if (PREFIX[0] == c)
            index = 1;
        else
            index = 0;
	--length;
    }
    if (length > 0) {
	retval += insertfonttbl(&op, (char) -1);
    }
    while (--length >= 0)
	copyandtranslate(&c, &ip, &op);
    RESTOREA5;
    return retval;
}

#ifndef OPENSTEP
static int convertreturns(char *datain, char *dataout, int length,
#else /* OPENSTEP */
static int convertreturns(const char *datain, char *dataout, int length,
#endif /* OPENSTEP */
							      convertdir_t dir)
{
    char from, to;

    switch (dir) {
    case MacToUNIX:
	bcopy(datain, dataout, length);
	from = '\r';
	to   = '\n';
	break;
    case UNIXToMac:
	bcopy(datain, dataout, length);
	from = '\n';
	to   = '\r';
	break;
    case MacRTFToUNIX:
/*-->*/	return macrtfconvert(length, datain, dataout);
	break;
    case UNIXRTFToMac:
/*-->*/	return unixrtfconvert(length, datain, dataout);
	break;
    default:
	from = 0;
	to = 0;
	gui_abort();
    }
    while (--length >= 0)
	if (*dataout++ == from)
	    dataout[-1] = to;
    return 0;
}

/*
 * NOTE: the code below should be table driven.
 */

#ifndef OPENSTEP
static NXAtom ARDIPICTPboardType = "PICT";
#else /* OPENSTEP */
#define ARDIPICTPboardType @"PICT"
#endif /* OPENSTEP */

#define TEXT (('T'<<24)|('E'<<16)|('X'<<8)|'T')
#define EPS  (('E'<<24)|('P'<<16)|('S'<<8)|' ')
#define RTF  (('R'<<24)|('T'<<16)|('F'<<8)|' ')
#define TIFF (('T'<<24)|('I'<<16)|('F'<<8)|'F')
#define PICT (('P'<<24)|('I'<<16)|('C'<<8)|'T')

id ROMlib_pasteboard = 0;
int ROMlib_ourchangecount;

void 
PutScrapX (long type, long length, char *p)
{
  static int count = 0;
  static int textcount = 0, epscount = 0, rtfcount = 0, tiffcount = 0,
    pictcount = 0;
  static char *textdata, *epsdata, *rtfdata, *tiffdata, *pictdata;
  static long textlength, epslength, rtflength, tifflength, pictlength;
  char doit;
#ifndef OPENSTEP
  char const *types[5];
  int i;
#else /* OPENSTEP */
  NSMutableArray *types;
#endif /* OPENSTEP */
  virtual_int_state_t block;
  long fonttblextra;

  SETUPA5;
#ifdef OPENSTEP

  types = [NSMutableArray arrayWithCapacity:5];

#endif /* OPENSTEP */
  block = block_virtual_ints ();
  if (!ROMlib_pasteboard)
#ifndef OPENSTEP
    ROMlib_pasteboard =[Pasteboard new];
#else /* OPENSTEP */
    ROMlib_pasteboard =[NSPasteboard generalPasteboard];
#endif /* OPENSTEP */
  doit = NO;
  switch (type)
    {
    case TEXT:
      if (count == textcount)
	++count;
      textcount = count;
      if (textdata)
	free (textdata);
      textdata = malloc (length);
      convertreturns (p, textdata, length, MacToUNIX);
      convertchars (textdata, length, mactonext);
      textlength = length;
      doit = YES;
      break;
    case EPS:
      if (count == epscount)
	++count;
      epscount = count;
      if (epsdata)
	free (epsdata);
      epsdata = malloc (length);
      convertreturns (p, epsdata, length, MacToUNIX);
      epslength = length;
      doit = YES;
      break;
    case RTF:
      if (count == rtfcount)
	++count;
      rtfcount = count;
      if (rtfdata)
	free (rtfdata);


      fonttblextra = insertfonttbl ((char **) 0, (char) 0);

      rtfdata = malloc (length + fonttblextra);
      rtflength = convertreturns (p, rtfdata, length, MacRTFToUNIX);
      convertchars (rtfdata, rtflength, mactonext);
      doit = YES;
      break;
    case TIFF:
      if (count == tiffcount)
	++count;
      tiffcount = count;
      if (tiffdata)
	free (tiffdata);
      tiffdata = malloc (length);
      bcopy (p, tiffdata, length);
      tifflength = length;
      doit = YES;
      break;
    case PICT:
      if (count == pictcount)
	++count;
      pictcount = count;
      if (pictdata)
	free (pictdata);
      pictdata = malloc (length);
      bcopy (p, pictdata, length);
      pictlength = length;
      doit = YES;
      break;
    default:
      ;
    }

  if (doit)
    {
#ifndef OPENSTEP
      i = -1;
      if (rtfcount == count)
	types[++i] = NXRTFPboardType;
      if (epscount == count)
	types[++i] = NXPostScriptPboardType;
      if (textcount == count)
	types[++i] = NXAsciiPboardType;
      if (tiffcount == count)
	types[++i] = NXTIFFPboardType;
      if (pictcount == count)
	types[++i] = ARDIPICTPboardType;
    [ROMlib_pasteboard declareTypes: types num: i + 1 owner:0];
#endif /* not OPENSTEP */
      if (rtfcount == count)
#ifndef OPENSTEP
      [ROMlib_pasteboard writeType: NXRTFPboardType data:rtfdata
      length:rtflength];
#else /* OPENSTEP */
	[types addObject:NSRTFPboardType];
#endif /* OPENSTEP */
      if (epscount == count)
#ifndef OPENSTEP
      [ROMlib_pasteboard writeType: NXPostScriptPboardType data:epsdata
      length:epslength];
#else /* OPENSTEP */
	[types addObject:NSPostScriptPboardType];
#endif /* OPENSTEP */
      if (textcount == count)
#ifndef OPENSTEP
      [ROMlib_pasteboard writeType: NXAsciiPboardType data:textdata
      length:textlength];
#else /* OPENSTEP */
	[types addObject:NSStringPboardType];
#endif /* OPENSTEP */
      if (tiffcount == count)
#ifndef OPENSTEP
      [ROMlib_pasteboard writeType: NXTIFFPboardType data:tiffdata
      length:tifflength];
#else /* OPENSTEP */
	[types addObject:NSTIFFPboardType];
#endif /* OPENSTEP */
      if (pictcount == count)
#ifndef OPENSTEP
      [ROMlib_pasteboard writeType: ARDIPICTPboardType data:pictdata
      length:pictlength];
#else /* OPENSTEP */
	[types addObject:ARDIPICTPboardType];
    [ROMlib_pasteboard declareTypes: types owner:NULL];
    if (rtfcount == count)
      [ROMlib_pasteboard setData: [NSData dataWithBytesNoCopy:rtfdata length:rtflength] forType:NSRTFPboardType];
    if (epscount == count)
      [ROMlib_pasteboard setData:[NSData dataWithBytesNoCopy:epsdata length:epslength] forType:NSPostScriptPboardType];
    if (textcount == count)
      [ROMlib_pasteboard setData:[NSData dataWithBytesNoCopy:textdata length:textlength] forType:NSStringPboardType];
    if (tiffcount == count)
      [ROMlib_pasteboard setData:[NSData dataWithBytesNoCopy:tiffdata length:tifflength] forType:NSTIFFPboardType];
    if (pictcount == count)
      [ROMlib_pasteboard setData:[NSData dataWithBytesNoCopy:pictdata length:pictlength] forType:ARDIPICTPboardType];
#endif /* OPENSTEP */
    }
  ROMlib_ourchangecount =[ROMlib_pasteboard changeCount];
  restore_virtual_ints (block);
  RESTOREA5;
}

#define noErr 0

long 
GetScrapX (long type, char **h)
{
#ifndef OPENSTEP
  const NXAtom *types;
  char *data;
  int length;
  NXAtom tofind;
#else /* OPENSTEP */
  NSArray *types;
  NSData *data;
  NSString *tofind;
#endif /* OPENSTEP */
  long retval;
  int temp;
  virtual_int_state_t block;

  SETUPA5;
  block = block_virtual_ints ();
  if (!ROMlib_pasteboard)
#ifndef OPENSTEP
    ROMlib_pasteboard =[Pasteboard new];
#else /* OPENSTEP */
    ROMlib_pasteboard =[NSPasteboard generalPasteboard];
#endif /* OPENSTEP */
  retval = -1;
  switch (type)
    {
    case TEXT:
#ifndef OPENSTEP
      tofind = NXAsciiPboardType;
#else /* OPENSTEP */
      tofind = NSStringPboardType;
#endif /* OPENSTEP */
      break;
    case RTF:
#ifndef OPENSTEP
      tofind = NXRTFPboardType;
#else /* OPENSTEP */
      tofind = NSRTFPboardType;
#endif /* OPENSTEP */
      break;
    case EPS:
#ifndef OPENSTEP
      tofind = NXPostScriptPboardType;
#else /* OPENSTEP */
      tofind = NSPostScriptPboardType;
#endif /* OPENSTEP */
      break;
    case TIFF:
#ifndef OPENSTEP
      tofind = NXTIFFPboardType;
#else /* OPENSTEP */
      tofind = NSTIFFPboardType;
#endif /* OPENSTEP */
      break;
    case PICT:
      tofind = ARDIPICTPboardType;
      break;
    default:
      tofind = 0;
      break;
    }
  if (tofind &&
      (temp = [ROMlib_pasteboard changeCount]) > ROMlib_ourchangecount)
    {
      types =[ROMlib_pasteboard types];
#ifndef OPENSTEP
      while (*types && strcmp (*types, tofind) != 0)
	++types;
      if (*types
          && [ROMlib_pasteboard readType: *types data: &data length:&length])
#else /* OPENSTEP */

      if ([types indexOfObject:tofind] != NSNotFound
	&& (data = [ROMlib_pasteboard dataForType:tofind]))
#endif /* OPENSTEP */
	{
#ifndef OPENSTEP
	  if (tofind == NXRTFPboardType)
#else /* OPENSTEP */
	  if (tofind == NSRTFPboardType)
#endif /* OPENSTEP */
	    {
#ifndef OPENSTEP
	      ReallocHandle ((Handle) h, length * 2);
#else /* OPENSTEP */
	      ReallocHandle ((Handle) h, [data length] * 2);
#endif /* OPENSTEP */
	      if (MemErr != noErr)
		{
		  retval = -1;
/*-->*/ goto DONE;
		}
#ifndef OPENSTEP
	      retval = convertreturns (data, MR (*h), length, UNIXRTFToMac);
#else /* OPENSTEP */
	      retval = convertreturns ([data bytes], MR (*h), [data length], UNIXRTFToMac);
#endif /* OPENSTEP */
	      convertchars (MR (*h), retval, nexttomac);
	      ReallocHandle ((Handle) h, retval);
	      if (MemErr != noErr)
		retval = -1;
/*-->*/ goto DONE;
	    }
	  else
	    {
#ifndef OPENSTEP
	      ReallocHandle ((Handle) h, length);
#else /* OPENSTEP */
	      ReallocHandle ((Handle) h, [data length]);
#endif /* OPENSTEP */
	      if (MemErr != noErr)
		{
		  retval = -1;
/*-->*/ goto DONE;
		}
#ifndef OPENSTEP
	      if (tofind != NXTIFFPboardType && tofind != ARDIPICTPboardType)
		convertreturns (data, MR (*h), length, UNIXToMac);
#else /* OPENSTEP */
	      if (tofind != NSTIFFPboardType && tofind != ARDIPICTPboardType)
		convertreturns ([data bytes], MR (*h), [data length], UNIXToMac);
#endif /* OPENSTEP */
	      else
#ifndef OPENSTEP
		bcopy (data, MR (*h), length);
	      if (tofind == NXAsciiPboardType)
		convertchars (MR (*h), length, nexttomac);
#else /* OPENSTEP */
		bcopy ([data bytes], MR (*h), [data length]);
	      if (tofind == NSStringPboardType)
		convertchars (MR (*h), [data length], nexttomac);
#endif /* OPENSTEP */
	    }
#ifndef OPENSTEP
	  vm_deallocate (task_self (), (vm_address_t) data, length);
	  retval = length;
#else /* OPENSTEP */
	  retval = [data length];
#endif /* OPENSTEP */
	}
      else
	{
	  ROMlib_ZeroScrap ();
	}
    }
 DONE:
  restore_virtual_ints (block);
  RESTOREA5;
  return retval;
}


void 
ROMlib_SetTitle (char *newtitle)
{
  BLOCK_REAL_INTERRUPTS_EXCURSION
    ({
#ifndef OPENSTEP
      [[self_view window] setTitle:newtitle];
#else /* OPENSTEP */
      [[self_view window] setTitle:[NSString stringWithCString:newtitle]];
#endif /* OPENSTEP */
    });
}


void
host_set_cursor (char *cursor_data,
		 unsigned short cursor_mask[16],
		 int hotspot_x, int hotspot_y)
{
  setcursorX ((short *)cursor_data, (short *)cursor_mask,
	      hotspot_x, hotspot_y);
}


int
host_set_cursor_visible (int show_p)
{
  static int prev_show_p = FALSE;
  int retval;
  
  showcursorX (show_p);
  retval = prev_show_p;
  prev_show_p = show_p;
  return retval;
}


void
host_flush_shadow_screen (void)
{
  int top_long, left_long, bottom_long, right_long;

  /* Lazily allocate a shadow screen.  We won't be doing refresh that often,
   * so don't waste the memory unless we need it.
   */
  if (shadow_fbuf == NULL)
    {
      shadow_fbuf = malloc (fbuf_size);
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


/* The following allows MacViewClass to grab the mousedown event that activates
 * the window. By default, the View's acceptsFirstMouse returns NO.
 */
- (BOOL)acceptsFirstMouse
{
  return YES;
}

- (BOOL) acceptsFirstResponder
{
  return YES;
}

/* These are methods in this file and also Mac #defines. */
#undef mouseUp
#undef mouseDown
#undef keyDown
#undef keyUp

#ifndef OPENSTEP
- mouseDown:(NXEvent *) eventp
#else /* OPENSTEP */
- (void)mouseDown:(NSEvent *) eventp
#endif /* OPENSTEP */
{
  SETUPA5;
#ifndef OPENSTEP
  [self convertPoint: &eventp->location fromView:nil];
#endif /* not OPENSTEP */
  postnextevent (eventp);
  RESTOREA5;
#ifndef OPENSTEP

  return self;
#endif /* not OPENSTEP */
}

#ifndef OPENSTEP
-mouseMoved:(NXEvent *) eventp
#else /* OPENSTEP */
- (void)mouseMoved:(NSEvent *) eventp
#endif /* OPENSTEP */
{
  SETUPA5;
#ifndef OPENSTEP
  [self convertPoint: &eventp->location fromView:nil];
#endif /* not OPENSTEP */
  ROMlib_updatemouselocation (eventp);
  RESTOREA5;
#ifndef OPENSTEP
  return self;
#endif /* not OPENSTEP */
}

#ifndef OPENSTEP
-mouseDragged:(NXEvent *) eventp
#else /* OPENSTEP */
-(void)mouseDragged:(NSEvent *) eventp
#endif /* OPENSTEP */
{
  SETUPA5;
#ifndef OPENSTEP
  [self convertPoint: &eventp->location fromView:nil];
#endif /* not OPENSTEP */
  ROMlib_updatemouselocation (eventp);
  RESTOREA5;
#ifndef OPENSTEP
  return self;
#endif /* not OPENSTEP */
}

#ifndef OPENSTEP
-mouseUp:(NXEvent *) eventp
#else /* OPENSTEP */
- (void)mouseUp:(NSEvent *) eventp
#endif /* OPENSTEP */
{
  SETUPA5;
#ifndef OPENSTEP
  [self convertPoint: &eventp->location fromView:nil];
#endif /* not OPENSTEP */
  postnextevent (eventp);
  RESTOREA5;
#ifndef OPENSTEP

  return self;
#endif /* not OPENSTEP */
}

#ifndef OPENSTEP
-mouseEntered:(NXEvent *) eventp
#else /* OPENSTEP */
-(void)mouseEntered:(NSEvent *) eventp
#endif /* OPENSTEP */
{
  SETUPA5;
  if (CrsrVis)
    [realcursor set];
  else
    [blankcursor set];
#ifndef OPENSTEP
  [self convertPoint: &eventp->location fromView:nil];
#endif /* not OPENSTEP */
  postnextevent (eventp);
  RESTOREA5;
#ifndef OPENSTEP
  return self;
#endif /* not OPENSTEP */
}

#ifndef OPENSTEP
-mouseExited:(NXEvent *) eventp
#else /* OPENSTEP */
-(void)mouseExited:(NSEvent *) eventp
#endif /* OPENSTEP */
{
  SETUPA5;
#ifndef OPENSTEP
  [NXArrow set];
  [self convertPoint: &eventp->location fromView:nil];
#else /* OPENSTEP */
#warning need to call setOnMouseExited when we set up the view ...
#endif /* OPENSTEP */
  postnextevent (eventp);
  RESTOREA5;
#ifndef OPENSTEP
  return self;
#endif /* not OPENSTEP */
}

#ifndef OPENSTEP
-keyDown:(NXEvent *) eventp
#else /* OPENSTEP */
-(void)keyDown:(NSEvent *) eventp
#endif /* OPENSTEP */
{
  SETUPA5;
#ifndef OPENSTEP
  [self convertPoint: &eventp->location fromView:nil];
#endif /* not OPENSTEP */
  postnextevent (eventp);
  RESTOREA5;
#ifndef OPENSTEP
  return self;
#endif /* not OPENSTEP */
}

static struct
{
  long mask;
  char key;
} maskkeys[] = {
#ifndef OPENSTEP
  { NX_ALPHASHIFTMASK,  0x39, },
  { NX_SHIFTMASK,       0x38, },
  { NX_CONTROLMASK,     0x3B, },
  { NX_ALTERNATEMASK,   0x3A, },
  { NX_COMMANDMASK,     0x37, },
#else /* OPENSTEP */
  { NSAlphaShiftKeyMask,  0x39, },
  { NSShiftKeyMask,       0x38, },
  { NSControlKeyMask,     0x3B, },
  { NSAlternateKeyMask,   0x3A, },
  { NSCommandKeyMask,     0x37, },
#endif /* OPENSTEP */
};

#ifndef OPENSTEP
-flagsChanged:(NXEvent *) eventp
#else /* OPENSTEP */
-(void)flagsChanged:(NSEvent *) eventp
#endif /* OPENSTEP */
{
  int i;

  SETUPA5;
  for (i = NELEM (maskkeys); --i >= 0;)
    {
#ifndef OPENSTEP
      ROMlib_zapmap (maskkeys[i].key, !!(eventp->flags & maskkeys[i].mask));
#else /* OPENSTEP */
      ROMlib_zapmap (maskkeys[i].key, !!([eventp modifierFlags] & maskkeys[i].mask));
#endif /* OPENSTEP */
    }
  /* If shift is down, assume caps lock is not.  The NeXT doesn't give us
   * a separate bit for just caps lock, which we need for Solarian.  This
   * will give us a decent (but imperfect) approximation...
   */
#ifndef OPENSTEP
  if (eventp->flags & NX_SHIFTMASK)
#else /* OPENSTEP */
  if ([eventp modifierFlags] & NSShiftKeyMask)
#endif /* OPENSTEP */
    ROMlib_zapmap (0x39, 0);
#ifndef OPENSTEP
  ROMlib_mods = ROMlib_next_butmods_to_mac_butmods (eventp->flags);
#else /* OPENSTEP */
  ROMlib_mods = ROMlib_next_butmods_to_mac_butmods ([eventp modifierFlags]);
#endif /* OPENSTEP */
  RESTOREA5;
#ifndef OPENSTEP
  return self;
#endif /* not OPENSTEP */
}

typedef enum { APP_NOT_STARTED, APP_IS_RUNNING, APP_DIED } app_state_t;

static app_state_t app_state = APP_NOT_STARTED;

void ROMlib_startapp( void )
{
    id abort_cell;

#ifndef OPENSTEP
    abort_cell = [global_menu findCellWithTag:11];
    [abort_cell setTitleNoCopy:"Abort..."];
#else /* OPENSTEP */
    abort_cell = [global_menu itemWithTag:11];
    [abort_cell setTitle:@"Abort..."];
#endif /* OPENSTEP */
    app_state = APP_IS_RUNNING;
}

#ifndef OPENSTEP
-(BOOL) performKeyEquivalent:(NXEvent *) eventp
#else /* OPENSTEP */
-(BOOL) performKeyEquivalent:(NSEvent *) eventp
#endif /* OPENSTEP */
{
  if (app_state == APP_IS_RUNNING)
    {
      SETUPA5;
#ifndef OPENSTEP
      [self convertPoint: &eventp->location fromView:nil];
#endif /* not OPENSTEP */
      postnextevent (eventp);
      RESTOREA5;
      return YES;
    }
  else
    {
      return [super performKeyEquivalent:eventp];
    }
}

#ifndef OPENSTEP
-keyUp:(NXEvent *) eventp
#else /* OPENSTEP */
-(void) keyUp:(NSEvent *) eventp
#endif /* OPENSTEP */
{
  SETUPA5;
#ifndef OPENSTEP
  [self convertPoint: &eventp->location fromView:nil];
#endif /* not OPENSTEP */
  postnextevent (eventp);
  RESTOREA5;
#ifndef OPENSTEP
  return self;
#endif /* not OPENSTEP */
}


#ifndef OPENSTEP
- pause:sender
#else /* OPENSTEP */
- (void) pause:sender
#endif /* OPENSTEP */
{
  id pause_cell;
  static char oldtitle[80];
  
#ifndef OPENSTEP
  pause_cell = [global_menu findCellWithTag:10];
  if ([pause_cell title][0] == 'P')
#else /* OPENSTEP */
  pause_cell = [global_menu itemWithTag:10];
  if ([[pause_cell title] characterAtIndex:0] == 'P')
#endif /* OPENSTEP */
    {
#ifndef OPENSTEP
      DPSRemoveTimedEntry(timer);
      strncpy(oldtitle, [[self window] title], sizeof(oldtitle));
      oldtitle[sizeof(oldtitle)-1] = 0;
      [[self window] setTitle:"Executor is PAUSED"];
      [pause_cell setTitleNoCopy:"Continue"];
#else /* OPENSTEP */
      [our_timer invalidate];
      [[[self window] title] getCString:oldtitle maxLength:sizeof(oldtitle)-1];
      [[self window] setTitle:@"Executor is PAUSED"];
      [pause_cell setTitle:@"Continue"];
#endif /* OPENSTEP */
    } 
  else 
    {
#ifndef OPENSTEP
      [[self window] setTitle:oldtitle];
      [pause_cell setTitleNoCopy:"Pause"];
      timer = DPSAddTimedEntry(0, &timerswitch, self, NX_BASETHRESHOLD);
#else /* OPENSTEP */
      [[self window] setTitle:[NSString stringWithCString:oldtitle]];
      [pause_cell setTitle:@"Pause"];
      create_our_timer (self);
#endif /* OPENSTEP */
    }
#ifndef OPENSTEP
  return self;
#endif /* not OPENSTEP */
}

#ifndef OPENSTEP
- abort:sender
#else /* OPENSTEP */
- (void) abort:sender
#endif /* OPENSTEP */
{
  if (app_state != APP_IS_RUNNING
#ifndef OPENSTEP
      || (NXRunAlertPanel
	  ("Are You Sure",
	   "Hitting the \"Abort Anyway\" button will cause Executor "
	   "to stop immediately, without giving the currently running "
	   "program a chance to quit gracefully.  This can result in "
	   "corrupted files.",
	   "Cancel", "Abort Anyway",
	   (const char *) 0)
	  == NX_ALERTALTERNATE))
    [NXApp terminate:sender];
  return self;
#else /* OPENSTEP */
      || (NSRunAlertPanel
	  (@"Are You Sure",
	   @"Hitting the \"Abort Anyway\" button will cause Executor to stop immediately, without giving the currently running program a chance to quit gracefully.  This can result in corrupted files.",
	   @"Cancel", @"Abort Anyway", nil)
	  == NSAlertAlternateReturn))
    [NSApp terminate:sender];
#endif /* OPENSTEP */
}


#ifndef OPENSTEP
- validRequestorForSendType:(NXAtom)
     typeSent andReturnType:(NXAtom) typeReturned
#else /* OPENSTEP */
- validRequestorForSendType:(NSString)
     typeSent andReturnType:(NSString) typeReturned
#endif /* OPENSTEP */
{
  return self;
}

#ifndef OPENSTEP
-readSelectionFromPasteboard:pboard
#else /* OPENSTEP */
- (void) readSelectionFromPasteboard:pboard
#endif /* OPENSTEP */
{
  id saveROMlib_pasteboard;

  {
    SETUPA5;
    sendsuspendevent ();
    RESTOREA5;
  }
  contextswitch (&nextstep_sp, &romlib_sp);
  contextswitch (&nextstep_sp, &romlib_sp);
  contextswitch (&nextstep_sp, &romlib_sp);
  saveROMlib_pasteboard = ROMlib_pasteboard;
  ROMlib_pasteboard = pboard;
  {
    SETUPA5;
    sendresumeevent (YES);
    RESTOREA5;
  }
  contextswitch (&nextstep_sp, &romlib_sp);
  contextswitch (&nextstep_sp, &romlib_sp);
  contextswitch (&nextstep_sp, &romlib_sp);
  ROMlib_pasteboard = saveROMlib_pasteboard;
  {
    SETUPA5;
    sendpaste ();
    RESTOREA5;
  }
  contextswitch (&nextstep_sp, &romlib_sp);
  contextswitch (&nextstep_sp, &romlib_sp);
  contextswitch (&nextstep_sp, &romlib_sp);
#ifndef OPENSTEP
  return self;
#endif /* not OPENSTEP */
}

#ifndef OPENSTEP
-(BOOL) writeSelectionToPasteboard: pboard types:(NXAtom *) types
#else /* OPENSTEP */
- (BOOL) writeSelectionToPasteboard:(NSPasteboard *) pboard
						types:(NSArray *) types
#endif /* OPENSTEP */
{
  id saveROMlib_pasteboard;

  {
    SETUPA5;
    sendcopy ();
    RESTOREA5;
  }
  contextswitch (&nextstep_sp, &romlib_sp);
  contextswitch (&nextstep_sp, &romlib_sp);
  contextswitch (&nextstep_sp, &romlib_sp);
  saveROMlib_pasteboard = ROMlib_pasteboard;
  ROMlib_pasteboard = pboard;
  {
    SETUPA5;
    sendsuspendevent ();
    RESTOREA5;
  }
  contextswitch (&nextstep_sp, &romlib_sp);
  contextswitch (&nextstep_sp, &romlib_sp);
  contextswitch (&nextstep_sp, &romlib_sp);
  ROMlib_pasteboard = saveROMlib_pasteboard;
  {
    SETUPA5;
    sendresumeevent (NO);
    RESTOREA5;
  }
  return YES;
}

#ifndef OPENSTEP
-(BOOL) getRect:(NXRect *) theRect forPage:(int) page
#else /* OPENSTEP */
-(NSRect) rectForPage:(int) page
#endif /* OPENSTEP */
{
#ifdef OPENSTEP
  NSRect retval;

#endif /* OPENSTEP */
  pagewanted = page;
#ifndef OPENSTEP
  *theRect = *[[NXApp printInfo] paperRect];
#else /* OPENSTEP */
  retval.origin.x = 0;
  retval.origin.y = 0;
  retval.size = [[NSPrintInfo sharedPrintInfo] paperSize];
#endif /* OPENSTEP */
  ROMlib_printtimeout = 10000;
  while (printstate != __idle && printstate != seenOpenPage &&
	 --ROMlib_printtimeout != 0)
    contextswitch (&nextstep_sp, &romlib_sp);
  if (ROMlib_printtimeout == 0)
    printstate = __idle;
#ifndef OPENSTEP
  return printstate == __idle ? NO : YES;
#else /* OPENSTEP */
  return printstate == __idle ? NSZeroRect : retval;
#endif /* OPENSTEP */
}

-(BOOL) knowsPagesFirst:(int *) firstPageNum last:(int *) lastPageNum
{
  return YES;
}

char ROMlib_needtorestore;

#ifndef OPENSTEP
-beginPageSetupRect:(const NXRect *) aRect placement:(const NXPoint *) location
#else /* OPENSTEP */
- (void) beginPageSetupRect:(NSRect) aRect placement:(NSPoint) location
#endif /* OPENSTEP */
{
#ifndef OPENSTEP
  id retval;

  retval =[ super beginPageSetupRect: aRect placement:location];
#else /* OPENSTEP */
  [super beginPageSetupRect: aRect placement:location];
#endif /* OPENSTEP */
  ROMlib_needtorestore = 0;
  ROMlib_suppressclip = 0;
#ifndef OPENSTEP

  return retval;
#endif /* not OPENSTEP */
}

#ifndef OPENSTEP
-addToPageSetup
#else /* OPENSTEP */
-(void) addToPageSetup
#endif /* OPENSTEP */
{
  float scale;
#ifndef OPENSTEP

  scale =[[NXApp printInfo] scalingFactor];
  PStranslate (0,[[NXApp printInfo] paperRect]->size.height);
#else /* OPENSTEP */
#warning need to implement scaling factor (dictionary lookup)
  scale = 1.0;
  PStranslate (0,[[NSPrintInfo sharedPrintInfo] paperSize].height);
#endif /* OPENSTEP */
  PSscale (1 * scale, -1 * scale);
/*
 * NOTE: we should probably check to see whether pse and psb are
 *       defined before blowing them away, but then again, just
 *       'cause they're defined doesn't mean they're what we want.
 */
  DPSPrintf (DPSGetCurrentContext (), "/pse {} def\n"
	     "/psb {} def\n"
	     "/execuserobject {\n"
	     "  dup UserObjects length ge\n"
	     "    { pop }\n"
	     "    { UserObjects exch get exec }\n"
	     "  ifelse\n"
	     "} def\n"
	     "/currentmouse { 0 0 } def\n"
	     "/printobject { pop pop } def\n"
    );
#ifndef OPENSTEP
  return[super addToPageSetup];
#else /* OPENSTEP */
  [super addToPageSetup];
#endif /* OPENSTEP */
}

/*
 * NOTE: %% is needed to get just one %
 */

#ifndef OPENSTEP
-endPageSetup
#else /* OPENSTEP */
-(void)endPageSetup
#endif /* OPENSTEP */
{
#ifndef OPENSTEP
  id retval;

  retval = [super endPageSetup];
#else /* OPENSTEP */
  [super endPageSetup];
#endif /* OPENSTEP */

  DPSPrintf (DPSGetCurrentContext (),
	     "%% The following is a lie that is necessary because Word 5\n"
	     "%% doesn't wrap EPS files properly and we can't tell where\n"
	     "%% the PostScript we're printing comes from.\n"
	     "%%%%BeginDocument: IWishWeDidntHaveToDoThis\n");
#ifndef OPENSTEP
  return retval;
#endif /* not OPENSTEP */
}

#ifndef OPENSTEP
-endPage
#else /* OPENSTEP */
- (void) endPage
#endif /* OPENSTEP */
{
  if (ROMlib_needtorestore)
    PSgrestore ();
  DPSPrintf (DPSGetCurrentContext (), ROMlib_page_end);
#ifndef OPENSTEP
  return [super endPage];
#else /* OPENSTEP */
  [super endPage];
#endif /* OPENSTEP */
}

#ifndef OPENSTEP
-endPSOutput
#else /* OPENSTEP */
- (void) endTrailer
#endif /* OPENSTEP */
{
#ifndef OPENSTEP
  [super endPSOutput];
#else /* OPENSTEP */
  [super endTrailer];
#endif /* OPENSTEP */

  pagewanted = 1024 * 1024;	/* i.e. real big */

  /* we can't repaint the screen from where we are, but we can accrue a
     very large dirty rect so the next repaint will redraw everything */

  dirty_rect_accrue (0, 0, vdriver_height, vdriver_width);
#ifndef OPENSTEP

  return self;
#endif /* not OPENSTEP */
}


char *
ROMlib_GetTitle (void)
{
#ifndef OPENSTEP
  const char *t = [[self_view window] title];
  return strcpy (malloc (strlen (t) + 1), t);
#else /* OPENSTEP */
  NSString *temp_title;
  char *retval;

  temp_title = [[self_view window] title];
  retval = malloc ([temp_title length] + 1);
  [temp_title getCString:retval];
  return retval;  
#endif /* OPENSTEP */
}

void
ROMlib_FreeTitle (char *title)
{
  free (title);
}

#ifndef OPENSTEP
- endPrologue
#else /* OPENSTEP */
- (void) endPrologue
#endif /* OPENSTEP */
{
  extern int pageno;

  pageno = 0;

  DPSPrintf(DPSGetCurrentContext(), ROMlib_doc_prolog);
  return [super endPrologue];
}
#ifdef OPENSTEP

#if !defined(NDEBUG)
void set_malloc_debug (int level)
{
  malloc_debug (level);
}
#endif

@end
#endif /* OPENSTEP */
