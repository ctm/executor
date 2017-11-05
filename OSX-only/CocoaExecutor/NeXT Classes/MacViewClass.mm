#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_MacViewClass[] =
    "$Id: MacViewClass.m,v 2.34 1997/05/09 09:51:14 ctm Exp $";
#endif

#import <Cocoa/Cocoa.h>
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
#include "rsys/vdriver.h"
#include "host_vdriver.h"
#include "rsys/parse.h"
#include "rsys/notmac.h"
#include "rsys/host.h"
#include <assert.h>

using namespace Executor;

/* NOTE: This isn't a very good MACOSX_ object, because much of its
 * data is stored in static variables, instead of in instance
 * variables.  That means we can only have one instantiation.  We do
 * this because we are constrained to adhere to the C semantics of the
 * vdriver interface, and because we have no interest in running two
 * of these simultaneously.
 */

/* NSBitmapImageReps for the screen in various formats. */
static NSBitmapImageRep *rgb_screen_bitmap;  
static NSBitmapImageRep *two_bpp_screen_bitmap;
NSCursor *Executor::realcursor, *Executor::blankcursor;
NSImage *blankcursorimage, *cursorimage;
NSBitmapImageRep *cursorrep, *blankcursorrep;
NSTimer *our_timer = nil;

static NSBitmapImageRep *current_screen_bitmap; /* An alias for one of the other three */
static NSBitmapImageRep *one_bpp_screen_bitmap;

/* Do we need to convert from internal screen to real screen? */
static boolean_t screen_needs_conversion_p;

/* This is here for easy access by C functions. */
static MacViewClass *self_view;

/* Function to convert the screen to NeXT format. */
static depthconv_func_t conversion_func = NULL;

/* Space for depth conversion tables. */
static uint8 depth_table_space[DEPTHCONV_MAX_TABLE_SIZE];

/* Color mapping table. */
static Executor::ColorSpec color_table[256];

/* Pointer to base memory for the NeXT-format frame buffer. */
static uint32 *ns_fbuf;

/* Pointer to base memory for the shadow screen (for refresh). */
static uint32 *shadow_fbuf;

/* Row bytes for the NeXT-format frame buffer. */
static uint32 ns_fbuf_row_bytes;

/* Size of allocated internal frame buffer. */
static uint32 fbuf_size;


/* These variables are required by the vdriver interface. */
extern vdriver_nextstep_mode_t vdriver_nextstep_modes;

namespace Executor {
uint8 *vdriver_fbuf;

int vdriver_row_bytes;

int vdriver_width = VDRIVER_DEFAULT_SCREEN_WIDTH;

int vdriver_height = VDRIVER_DEFAULT_SCREEN_HEIGHT;

int vdriver_bpp, vdriver_log2_bpp;

int vdriver_max_bpp, vdriver_log2_max_bpp;

rgb_spec_t *vdriver_rgb_spec = NULL;

static rgb_spec_t ns_rgb_spec;

/* For now, just force black and white cursors. */
int host_cursor_depth = 1;

/* True iff our display is two bit grayscale. */
static bool two_bit_grayscale_display_p;

/* True if we should be drawing in grayscale. */
bool vdriver_grayscale_p;

/* True if the CLUT cannot be changed. */
bool vdriver_fixed_clut_p;

void
vdriver_shutdown (void)
{
}

   std::string SystemDiskLocation()
   {
	  const char* fsr = [[@"~/Executor/" stringByExpandingTildeInPath] fileSystemRepresentation];// [[[NSBundle mainBundle] resourcePath] fileSystemRepresentation];
	  if (fsr) {
		 return std::string(fsr) + "/";
	  } else {
		 return "";
	  }
   }
   
void
vdriver_opt_register (void)
{
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
}

}

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
Executor::vdriver_update_screen_rects (int num_rects, const vdriver_rect_t *r,
                                       bool cursor_p)
{
   NSRect *nxr;
   int i;

   if (num_rects == 0 || current_screen_bitmap == nil)
      return 0;

   /*
    * While we're printing, send no bits to the screen.  This is ugly, but
    * that's the way E/NS 1.3x works.
    */

   if (printstate != __idle)
      return 0;

   if (screen_needs_conversion_p) {
      if (conversion_func == NULL)
      {
         if (current_screen_bitmap == rgb_screen_bitmap)
            conversion_func = (depthconv_make_ind_to_rgb_table
                               (depth_table_space, vdriver_bpp,
                                NULL, color_table, &ns_rgb_spec));
         else if (current_screen_bitmap == two_bpp_screen_bitmap)
         {
            if (vdriver_bpp == 1) {
               static const uint32 one_to_two_table[2] = { 0x3, 0 };
               
               conversion_func = (depthconv_make_raw_table
                                  (depth_table_space, 1, 2,
                                   NULL, one_to_two_table));
            } else if (vdriver_bpp == 2) {
               conversion_func = flip_2bpp_pixels;
            } else
               abort ();
         } else
            abort ();
      }

      /* Convert from the current format to our internal screen format. */
      for (i = 0; i < num_rects; i++) {
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
   nxr = (NSRect *) alloca (num_rects * sizeof nxr[0]);
   for (i = 0; i < num_rects; i++) {
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

      /* I'm not sure where Mat learned about this call */
      NSRectClipList (nxr, num_rects);

      [current_screen_bitmap draw];
      [self_view unlockFocus];
   });

   return 0;
}


int
Executor::vdriver_update_screen (int top, int left, int bottom, int right,
                                 bool cursor_p)
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
Executor::vdriver_set_colors (int first_color, int num_colors, const Executor::ColorSpec *colors)
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


bool
Executor::vdriver_acceptable_mode_p (int width, int height, int bpp,
                                     bool grayscale_p,
                                     bool exact_match_p)
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


bool
Executor::vdriver_set_mode (int width, int height, int bpp, bool grayscale_p)
{
   if (!vdriver_acceptable_mode_p (width, height, bpp, grayscale_p, FALSE))
      return FALSE;

   if (width == 0)
      width = vdriver_width;
   if (height == 0)
      height = vdriver_height;
   if (bpp == 0) {
      bpp = vdriver_bpp;
      if (bpp == 0)
         bpp = vdriver_max_bpp;
   }

   vdriver_width = width;
   vdriver_height = height;

   if (bpp != vdriver_bpp) {
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
Executor::vdriver_get_colors (int first_color, int num_colors,
                              struct Executor::ColorSpec *color_array)
{
   if (vdriver_fixed_clut_p) {
      static const Executor::ColorSpec one_bpp_gray_cspecs[2] = {
         { CWC (0), { CWC ((unsigned short)0xFFFF), CWC ((unsigned short)0xFFFF), CWC ((unsigned short)0xFFFF) } },
         { CWC (1), { CWC (0x0000), CWC (0x0000), CWC (0x0000) } },
      };
      static const Executor::ColorSpec two_bpp_gray_cspecs[4] = {
         { CWC (0), { CWC ((unsigned short)0xFFFF), CWC ((unsigned short)0xFFFF), CWC ((unsigned short)0xFFFF) } },
         { CWC (1), { CWC ((unsigned short)0xAAAA), CWC ((unsigned short)0xAAAA), CWC ((unsigned short)0xAAAA) } },
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
   } else {
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
Executor::vdriver_flush_display (void)
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
      [[NSGraphicsContext currentContext] flushGraphics];
   });
}


void
Executor::host_beep_at_user (void)
{
   NSBeep ();
}


void Executor::querypointerX( LONGINT *xp, LONGINT *yp, LONGINT *modp )
{
   NSPoint p;

   BLOCK_REAL_INTERRUPTS_EXCURSION
   ({
      p = [[self_view window] mouseLocationOutsideOfEventStream];
      *xp = p.x;
      *yp = (vdriver_height - p.y);
   });
}

void
Executor::showcursorX(LONGINT show)
{
   BLOCK_REAL_INTERRUPTS_EXCURSION
   ({
      if (show)
         [realcursor set];
      else
         [blankcursor set];
   });
}

extern void Executor::setcursorX(INTEGER *data, INTEGER *mask, LONGINT hotx, LONGINT hoty )
{
   char *datap;
   short i;
   short mymask[16], mydata[16], gray_bits;
   NSPoint p;
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
      
      datap = (char *)[cursorrep bitmapData];
      memcpy (datap, mydata, 32);
      memcpy (datap + 32, mymask, 32);
      [cursorimage recache];
      if (CrsrVis)
         [blankcursor set];

      p.x = hotx;
      p.y = hoty;

      realcursor = [[NSCursor alloc] initWithImage:cursorimage hotSpot:p];
      
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

long Executor::ROMlib_printtimeout = 10000;	/* any positive number will do */

bool
Executor::vdriver_init (int _max_width, int _max_height, int _max_bpp,
                        bool fixed_p, int *argc, char *argv[])
{
   int width, height, i;
   NSWindow *view_window;

   make_rgb_spec (&ns_rgb_spec, 16, FALSE, CLC_RAW (0x000F000F),
#if defined (LITTLEENDIAN)
                  4, 4, 4, 0, 4, 12,
#else
                  4, 12, 4, 8, 4, 4,
#endif
				  GetCTSeed ());

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

   ns_fbuf = (uint32*)valloc (((ns_fbuf_row_bytes * height) + 8191) & ~8191);

   /* Allocate the internal bitmap, enough space for max bpp.  Make each
    * rowbytes divisible by 16.
    */
   vdriver_row_bytes = (((width << vdriver_log2_max_bpp) + 127) / 128U) * 16;
   fbuf_size = vdriver_row_bytes * height;
   vdriver_fbuf = (uint8*)valloc (fbuf_size);
   memset (vdriver_fbuf, 0, fbuf_size);

   if (!two_bit_grayscale_display_p) {
      uint32 white_pixel;

      white_pixel = ns_rgb_spec.white_pixel;
      for (i = ns_fbuf_row_bytes * height / sizeof (uint32) - 1; i >= 0; i--)
         ns_fbuf[i] = white_pixel;


      rgb_screen_bitmap = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:(unsigned char **)&ns_fbuf
                                                                  pixelsWide:width pixelsHigh:height bitsPerSample:4
                                                             samplesPerPixel:3 hasAlpha:NO isPlanar:NO
                                                              colorSpaceName:NSDeviceRGBColorSpace
                                                                 bytesPerRow:ns_fbuf_row_bytes bitsPerPixel:16];
      
      one_bpp_screen_bitmap = nil;
      two_bpp_screen_bitmap = nil;
   } else {
      memset (ns_fbuf, ~0, ns_fbuf_row_bytes * height);
      
      two_bpp_screen_bitmap = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:(unsigned char **)&ns_fbuf
                                                                      pixelsWide:width pixelsHigh:height bitsPerSample:2
                                                                 samplesPerPixel:1 hasAlpha:NO isPlanar:NO
                                                                  colorSpaceName:NSDeviceWhiteColorSpace
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

void converthex(char **h, const unsigned char *table)
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

void Executor::convertchars(char *data, long length, const unsigned char *table)
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
      data[1] = table[*(unsigned char *)data];
      data++;
   }
}

static int unixrtfconvert(int length, const char *ip, char *op)
{
   const char *fromp, *ep;
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

static inline void copyandtranslate( char *cp, const char **ipp, char **opp)
{
   *cp = *(*ipp)++;
   if (*cp == '\r')
      *cp = '\n';
   *(*opp)++ = *cp;
}

typedef NS_ENUM(int, convertdir_t) {
   MacToUNIX,
   UNIXToMac,
   MacRTFToUNIX,
   UNIXRTFToMac
};

static const unsigned char mactonext[] = {
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

static const unsigned char nexttomac[] = {
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

static int macrtfconvert(int length, const char *ip, char *op)
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

static int convertreturns(const char *datain, char *dataout, int length,
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

enum {
   TEXT = (Executor::OSType)(('T'<<24)|('E'<<16)|('X'<<8)|'T'),
   EPS = (Executor::OSType)(('E'<<24)|('P'<<16)|('S'<<8)|' '),
   RTF = (Executor::OSType)(('R'<<24)|('T'<<16)|('F'<<8)|' '),
   TIFF = (Executor::OSType)(('T'<<24)|('I'<<16)|('F'<<8)|'F'),
   PICT = (Executor::OSType)(('P'<<24)|('I'<<16)|('C'<<8)|'T')
};

NSPasteboard *Executor::ROMlib_pasteboard = nil;
NSInteger ROMlib_ourchangecount;

void Executor::PutScrapX (Executor::OSType type, LONGINT length, char *p, int scrp_cnt)
{
   static int count = 0;
   static int textcount = 0, epscount = 0, rtfcount = 0, tiffcount = 0,
   pictcount = 0;
   char *textdata = nullptr, *epsdata = nullptr, *rtfdata = nullptr, *tiffdata = nullptr, *pictdata = nullptr;
   long textlength = 0, epslength = 0, rtflength = 0, tifflength = 0, pictlength = 0;
   char doit;
   NSMutableArray *types;
   virtual_int_state_t block;
   long fonttblextra;
   
   SETUPA5;
   types = [NSMutableArray arrayWithCapacity:5];
   
   block = block_virtual_ints ();
   if (!ROMlib_pasteboard)
      ROMlib_pasteboard = [[NSPasteboard generalPasteboard] retain];
   doit = NO;
   switch (type) {
      case TEXT:
         if (count == textcount)
            ++count;
         textcount = count;
         if (textdata)
            free (textdata);
         textdata = (char*)malloc (length);
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
         epsdata = (char*)malloc (length);
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
         
         rtfdata = (char*)malloc (length + fonttblextra);
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
         tiffdata = (char*)malloc (length);
         memcpy (tiffdata, p, length);
         tifflength = length;
         doit = YES;
         break;
      case PICT:
         if (count == pictcount)
            ++count;
         pictcount = count;
         if (pictdata)
            free (pictdata);
         pictdata = (char*)malloc (length);
         memcpy (pictdata, p, length);
         pictlength = length;
         doit = YES;
         break;
      default:
         ;
   }
   
   if (doit) {
      if (rtfcount == count)
         [types addObject:NSRTFPboardType];
      if (epscount == count)
         [types addObject:NSPostScriptPboardType];
      if (textcount == count)
         [types addObject:NSStringPboardType];
      if (tiffcount == count)
         [types addObject:NSTIFFPboardType];
      if (pictcount == count)
         [types addObject:NSPICTPboardType];
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
         [ROMlib_pasteboard setData:[NSData dataWithBytesNoCopy:pictdata length:pictlength] forType:NSPICTPboardType];
   }
   ROMlib_ourchangecount =[ROMlib_pasteboard changeCount];
   restore_virtual_ints (block);
   RESTOREA5;
}

LONGINT Executor::GetScrapX (OSType type, Executor::Handle h)
{
   @autoreleasepool {
      NSArray *types;
      NSData *data;
      NSString *tofind;
      long retval;
      int temp;
      virtual_int_state_t block;

      SETUPA5;
      block = block_virtual_ints ();
      if (!ROMlib_pasteboard)
         ROMlib_pasteboard = [[NSPasteboard generalPasteboard] retain];
      retval = -1;
      switch (type) {
         case TEXT:
            tofind = NSStringPboardType;
            break;
         case RTF:
            tofind = NSRTFPboardType;
            break;
         case EPS:
            tofind = NSPostScriptPboardType;
            break;
         case TIFF:
            tofind = NSTIFFPboardType;
            break;
         case PICT:
            tofind = NSPICTPboardType;
            break;
         default:
            tofind = 0;
            break;
      }
      if (tofind &&
          (temp = [ROMlib_pasteboard changeCount]) > ROMlib_ourchangecount)
      {
         types =[ROMlib_pasteboard types];
         
         if ([types indexOfObject:tofind] != NSNotFound
             && (data = [ROMlib_pasteboard dataForType:tofind]))
         {
            if (tofind == NSRTFPboardType)
            {
               ReallocHandle ((Handle) h, [data length] * 2);
               if (MemErr != CWC(noErr))
               {
                  retval = -1;
                  /*-->*/ goto DONE;
               }
               retval = convertreturns ((const char*)[data bytes], (char*)MR (*h), [data length], UNIXRTFToMac);
               convertchars ((char*)MR (*h), retval, nexttomac);
               ReallocHandle ((Handle) h, retval);
               if (MemErr != CWC(noErr))
                  retval = -1;
               /*-->*/ goto DONE;
            } else {
               ReallocHandle ((Handle) h, [data length]);
               if (MemErr != CWC(noErr)) {
                  retval = -1;
                  /*-->*/ goto DONE;
               }
               if (![tofind isEqualToString: NSTIFFPboardType] && ![tofind isEqualToString:NSPICTPboardType])
                  convertreturns ((const char*)[data bytes],  (char*)MR (*h), [data length], UNIXToMac);
               else
                  memcpy (MR (*h), [data bytes], [data length]);
               if ([tofind isEqualToString:NSStringPboardType])
                  convertchars ((char*)MR (*h), [data length], nexttomac);
            }
            retval = [data length];
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
}


void
ROMlib_SetTitle (char *newtitle)
{
   BLOCK_REAL_INTERRUPTS_EXCURSION
   ({
      @autoreleasepool {
         [[self_view window] setTitle:[NSString stringWithCString:newtitle encoding:NSMacOSRomanStringEncoding]];
      }
   });
}

void
Executor::host_set_cursor (char *cursor_data,
                           unsigned short cursor_mask[16],
                           int hotspot_x, int hotspot_y)
{
   setcursorX ((short *)cursor_data, (short *)cursor_mask,
               hotspot_x, hotspot_y);
}


int
Executor::host_set_cursor_visible (int show_p)
{
   static int prev_show_p = FALSE;
   int retval;

   showcursorX (show_p);
   retval = prev_show_p;
   prev_show_p = show_p;
   return retval;
}


void
Executor::host_flush_shadow_screen (void)
{
   int top_long, left_long, bottom_long, right_long;

   /* Lazily allocate a shadow screen.  We won't be doing refresh that often,
    * so don't waste the memory unless we need it.
    */
   if (shadow_fbuf == NULL)
   {
      shadow_fbuf = (uint32*)malloc (fbuf_size);
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

typedef enum {
   APP_NOT_STARTED,
   APP_IS_RUNNING,
   APP_DIED
} app_state_t;

static app_state_t app_state = APP_NOT_STARTED;

extern void ROMlib_startapp( void );

void ROMlib_startapp( void )
{
   id abort_cell;

   abort_cell = [global_menu itemWithTag:11];
   [abort_cell setTitle:@"Abort..."];
   app_state = APP_IS_RUNNING;
}

char *
ROMlib_GetTitle (void)
{
   NSString *temp_title;
   char *retval;

   temp_title = [[self_view window] title];
   NSUInteger maxLen = [temp_title lengthOfBytesUsingEncoding:NSMacOSRomanStringEncoding];
   retval = (char*)malloc (maxLen + 1);
   [temp_title getCString:retval maxLength:maxLen + 1 encoding:NSMacOSRomanStringEncoding];
   return retval;
}

void
ROMlib_FreeTitle (char *title)
{
   free (title);
}


#if 0
void set_malloc_debug (int level)
{
   malloc_debug (level);
}
#endif


@implementation MacViewClass

- (id) initWithFrame:(NSRect)frameRect
{
  NSSize size;
  char *datap;
  int bytecount;

  self = [super initWithFrame:frameRect];
   if (!self) {
      [self release];
      return nil;
   }

  /* Since we will be repeatedly focused on, allocate a gstate. */
  [self allocateGState];

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

- (void) drawRect:(NSRect)rect
{
  SETUPA5;

  switch (printstate) {
    case __idle:
      BLOCK_REAL_INTERRUPTS_EXCURSION
	({
	  if (current_screen_bitmap == nil)
	    {
          [[NSColor whiteColor] set];
	      NSRectFill ([self bounds]);
	    }
	  else
	    {
	      NSRectClip (rect); /* is this needed? */
	      [current_screen_bitmap draw];
	    }
	});
      break;
    case seenOpenPage:
	RESTOREA5;
	do {
		
	} while (printstate != __idle && printstate != seenClosePage);
	goto avoidrestorea5;
	break;
    default:
	/* seenClosePage may get us in here, but we can't do anything
		without sploding (I think) */
	break;
    }
  RESTOREA5;
avoidrestorea5:
  return;
}

- (id)step
{
  /* NOTE: we may need to be doing SETUPA5 now as well as protect us stuff,
   since timerswitch is no longer called */
  
  [self lockFocus];
  
  do {
	  
  } while (printstate != seenOpenDoc && printstate != seenPageSetUp
         && [NSApp nextEventMatchingMask:NSAnyEventMask
                               untilDate:[NSDate distantPast]
                                  inMode:NSDefaultRunLoopMode
                                 dequeue:NO] == nil
         );
  
  [self unlockFocus];
  {
    SETUPA5;
    if (printstate == seenOpenDoc)
    {
      [self print:self];
      if (ROMlib_printtimeout == 0)
        ROMlib_printtimeout = -1;
    }
    else if (printstate == seenPageSetUp)
    {
      [NSApp runPageLayout:self];
      printstate = __idle;
    }
  RESTOREA5;
  }
  return self;
}


/* The following allows MacViewClass to grab the mousedown event that activates
 * the window. By default, the View's acceptsFirstMouse returns NO.
 */
- (BOOL) acceptsFirstResponder
{
  return YES;
}

/* These are methods in this file and also Mac #defines. */
#undef mouseUp
#undef mouseDown
#undef keyDown
#undef keyUp

- (void)mouseDown:(NSEvent *) eventp
{
  SETUPA5;
  postnextevent (eventp);
  RESTOREA5;
}

- (void)mouseMoved:(NSEvent *) eventp
{
  SETUPA5;
  ROMlib_updatemouselocation (eventp);
  RESTOREA5;
}

-(void)mouseDragged:(NSEvent *) eventp
{
  SETUPA5;
  ROMlib_updatemouselocation (eventp);
  RESTOREA5;
}

- (void)mouseUp:(NSEvent *) eventp
{
  SETUPA5;
  postnextevent (eventp);
  RESTOREA5;
}

-(void)mouseEntered:(NSEvent *) eventp
{
  SETUPA5;
  if (CrsrVis)
    [realcursor set];
  else
    [blankcursor set];
  postnextevent (eventp);
  RESTOREA5;
}

-(void)mouseExited:(NSEvent *) eventp
{
  SETUPA5;
#warning need to call setOnMouseExited when we set up the view ...
  postnextevent (eventp);
  RESTOREA5;
}

-(void)keyDown:(NSEvent *) eventp
{
  SETUPA5;
  postnextevent (eventp);
  RESTOREA5;
}

static struct
{
  NSEventModifierFlags mask;
  char key;
} maskkeys[] = {
  { NSAlphaShiftKeyMask,  0x39, },
  { NSShiftKeyMask,       0x38, },
  { NSControlKeyMask,     0x3B, },
  { NSAlternateKeyMask,   0x3A, },
  { NSCommandKeyMask,     0x37, },
};

-(void)flagsChanged:(NSEvent *) eventp
{
  int i;

  SETUPA5;
  for (i = NELEM (maskkeys); --i >= 0;)
    {
      ROMlib_zapmap (maskkeys[i].key, !!([eventp modifierFlags] & maskkeys[i].mask));
    }
  /* If shift is down, assume caps lock is not.  The NeXT doesn't give us
   * a separate bit for just caps lock, which we need for Solarian.  This
   * will give us a decent (but imperfect) approximation...
   */
  if ([eventp modifierFlags] & NSShiftKeyMask)
    ROMlib_zapmap (0x39, 0);
  ROMlib_mods = ROMlib_next_butmods_to_mac_butmods ([eventp modifierFlags]);
  RESTOREA5;
}

-(BOOL) performKeyEquivalent:(NSEvent *) eventp
{
  if (app_state == APP_IS_RUNNING) {
      SETUPA5;
      postnextevent (eventp);
      RESTOREA5;
      return YES;
    } else {
      return [super performKeyEquivalent:eventp];
    }
}

-(void) keyUp:(NSEvent *) eventp
{
  SETUPA5;
  postnextevent (eventp);
  RESTOREA5;
}


- (IBAction) pause:(id)sender
{
  id pause_cell;
  static char oldtitle[80];
  
  pause_cell = [global_menu itemWithTag:10];
  if ([[pause_cell title] characterAtIndex:0] == 'P') {
      [our_timer invalidate];
      [[[self window] title] getCString:oldtitle maxLength:sizeof(oldtitle)-1 encoding:NSUTF8StringEncoding];
      [[self window] setTitle:@"Executor is PAUSED"];
      [pause_cell setTitle:@"Continue"];
    } else {
      [[self window] setTitle:@(oldtitle)];
      [pause_cell setTitle:@"Pause"];
      create_our_timer (self);
    }
}

- (IBAction) abort:(id)sender
{
  if (app_state != APP_IS_RUNNING
      || (NSRunAlertPanel
	  (@"Are You Sure",
	   @"Hitting the \"Abort Anyway\" button will cause Executor to stop immediately, without giving the currently running program a chance to quit gracefully.  This can result in corrupted files.",
	   @"Cancel", @"Abort Anyway", nil)
	  == NSAlertAlternateReturn))
    [NSApp terminate:sender];
}


- (id)validRequestorForSendType:(NSString*)typeSent andReturnType:(NSString*) typeReturned
{
  return self;
}

- (void)readSelectionFromPasteboard:(NSPasteboard*)pboard
{
  id saveROMlib_pasteboard;

  {
    SETUPA5;
    sendsuspendevent ();
    RESTOREA5;
  }
  saveROMlib_pasteboard = ROMlib_pasteboard;
  ROMlib_pasteboard = pboard;
  {
    SETUPA5;
    sendresumeevent (YES);
    RESTOREA5;
  }
  ROMlib_pasteboard = saveROMlib_pasteboard;
  {
    SETUPA5;
    sendpaste ();
    RESTOREA5;
  }
}

- (BOOL) writeSelectionToPasteboard:(NSPasteboard *) pboard
						types:(NSArray *) types
{
  id saveROMlib_pasteboard;

  {
    SETUPA5;
    sendcopy ();
    RESTOREA5;
  }
  saveROMlib_pasteboard = ROMlib_pasteboard;
  ROMlib_pasteboard = pboard;
  {
    SETUPA5;
    sendsuspendevent ();
    RESTOREA5;
  }
  ROMlib_pasteboard = saveROMlib_pasteboard;
  {
    SETUPA5;
    sendresumeevent (NO);
    RESTOREA5;
  }
  return YES;
}

-(NSRect) rectForPage:(NSInteger) page
{
  NSRect retval;

  pagewanted = page;
  retval.origin.x = 0;
  retval.origin.y = 0;
  retval.size = [[NSPrintInfo sharedPrintInfo] paperSize];
  ROMlib_printtimeout = 10000;
  while (printstate != __idle && printstate != seenOpenPage &&
		 --ROMlib_printtimeout != 0) {
	 }
  if (ROMlib_printtimeout == 0)
    printstate = __idle;
  return printstate == __idle ? NSZeroRect : retval;
}

-(BOOL) knowsPagesFirst:(int *) firstPageNum last:(int *) lastPageNum
{
  return YES;
}

char Executor::ROMlib_needtorestore;

#if 0
- (void) beginPageSetupRect:(NSRect) aRect placement:(NSPoint) location
{
  [super beginPageSetupRect: aRect placement:location];
  ROMlib_needtorestore = 0;
  ROMlib_suppressclip = 0;
}

-(void) addToPageSetup
{
  float scale;
#warning need to implement scaling factor (dictionary lookup)
  scale = 1.0;
  PStranslate (0,[[NSPrintInfo sharedPrintInfo] paperSize].height);
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
  [super addToPageSetup];
}

/*
 * NOTE: %% is needed to get just one %
 */

-(void)endPageSetup
{
  [super endPageSetup];

#if 0
  DPSPrintf (DPSGetCurrentContext (),
	     "%% The following is a lie that is necessary because Word 5\n"
	     "%% doesn't wrap EPS files properly and we can't tell where\n"
	     "%% the PostScript we're printing comes from.\n"
	     "%%%%BeginDocument: IWishWeDidntHaveToDoThis\n");
#endif
}

- (void) endPage
{
  if (ROMlib_needtorestore)
    PSgrestore ();
  DPSPrintf (DPSGetCurrentContext (), ROMlib_page_end);
  [super endPage];
}

- (void) endTrailer
{
  [super endTrailer];

  pagewanted = 1024 * 1024;	/* i.e. real big */

  /* we can't repaint the screen from where we are, but we can accrue a
     very large dirty rect so the next repaint will redraw everything */

  dirty_rect_accrue (0, 0, vdriver_height, vdriver_width);
}
#endif

#if 0
- (void) endPrologue
{
  extern int pageno;

  pageno = 0;

  //DPSPrintf(DPSGetCurrentContext(), ROMlib_doc_prolog);
  [super endPrologue];
}
#endif

@end
