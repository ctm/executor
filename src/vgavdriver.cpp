/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_vgavdriver[] = "$Id: vgavdriver.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* This file is not a standalone vdriver; it merely captures the
 * commonality between various interfaces to vga hardware.  It relies
 * on set of host-specific functions to perform tasks whose implementations
 * vary from host to host.
 */

#include "rsys/common.h"
#include "rsys/vgavdriver.h"

#if defined(USE_VGAVDRIVER)

#include "QuickDraw.h"
#include "rsys/cquick.h"
#include "rsys/flags.h"
#include "rsys/blockinterrupts.h"
#include "rsys/refresh.h"
#include "rsys/host.h"
#include "rsys/dirtyrect.h"
#include "rsys/autorefresh.h"

using namespace Executor;

#if defined(VGA_SCREEN_NEEDS_FAR_PTR)
#define VGA_IF_SEL(x) x
#else
#define VGA_IF_SEL(x)
#endif

/* These variables are required by the vdriver interface. */

uint8 *vdriver_fbuf;

int vdriver_row_bytes;

int vdriver_width, vdriver_height;

int vdriver_bpp, vdriver_log2_bpp;

int vdriver_max_bpp, vdriver_log2_max_bpp;

vdriver_modes_t *vdriver_mode_list;

rgb_spec_t *vdriver_rgb_spec;

/* Size of the internal frame buffer, in bytes. */
static int fbuf_size;

/* Current hardware graphics mode. */
const vga_mode_t *vga_current_mode;

/* Default mode. */
static const vga_mode_t *default_mode;

/* List of "raw" modes reported by the host. */
static vga_mode_t *vga_mode_list;

/* Space for lookup tables used during depth conversion. */
static uint32 depth_conversion_table[256][2];

/* Function to update the screen in the current mode. */
static void (*screen_update_func)(unsigned, unsigned, unsigned, unsigned);

/* Offset from screen base address to get to "real" upper left.  We
 * use this to center screens smaller than the physical screen.
 */
int vga_first_byte_offset;

/* Current read and write windows (may always be equal; depends on
 * the SVGA board).
 */
int vga_write_window, vga_read_window;

/* Rectangle enclosing the cursor. */
static volatile int cursor_top, cursor_left, cursor_bottom, cursor_right;

/* X, Y of cursor upper left corner point on the screen. */
static volatile int cursor_x, cursor_y;

/* Actual hotspot for the cursor, relative to the cursor's upper left. */
static volatile int cursor_hotspot_x, cursor_hotspot_y;

/* Boolean: can the cursor be seen on the screen right now? */
static unsigned char cursor_visible_p;

/* Boolean: should we be drawing the cursor at all? */
static unsigned char draw_cursor_p;

/* Base address of either the 64K VESA window, or the full hardware screen,
 * depending on the current graphics mode.  This may be in a different
 * address space.
 */
uint8 *vga_portal_baseaddr;

/* Similar to vdriver_portal_baseaddr, but only valid in modes without
 * multiple windows (e.g. linear modes).  It also may be offset
 * from the actual screen base address to center Mac screens smaller
 * than the SVGA mode.
 */
uint8 *vdriver_real_screen_baseaddr;

/* true iff the blitter is allowed to go straight to the real screen. */
bool vdriver_real_screen_blit_possible_p;

/* true iff the blitter should assume real screen bits are flipped. */
bool vdriver_flip_real_screen_pixels_p;

/* Bytes per row for the "real" screen. */
int vdriver_real_screen_row_bytes;

/* Are we grayscale-only? */
bool vdriver_grayscale_p;

typedef struct
{
    uint8 and_mask, xor_mask;
} vga_cursor_xfer_t;

typedef struct
{
    int elt_size;
    int row_bytes;
    union {
        vga_cursor_xfer_t *packed;
        unsigned char *raw_bits;
    } cursor_bits;
} vga_cursor_mask_desc_t;

/* The or_mask and xor_mask indicate how we are to transfer the cursor
 * to the screen.  First we AND the and_mask with the screen, then we XOR the
 * xor_mask to the screen.  Clever use of these two operators lets us
 * set, clear, or toggle any bits on the screen.
 */
static union {
    vga_cursor_xfer_t bpp1[8][16][3];
    vga_cursor_xfer_t bpp2[4][16][5];
    vga_cursor_xfer_t bpp4[2][16][9];
    vga_cursor_xfer_t bpp8[16][16];
} cursor_masks;

/* Size of a frame buffer, in bytes.  We round up so row_bytes % 4 == 0. */
#define FBUF_SIZE(width, height, log2_bpp) \
    ((((width) << (log2_bpp)) + 31) / 32 * 4 * (height))

static void make_depth_tables(int in_bpp, int out_bpp);
static const vga_mode_t *compute_default_mode(void);
static void compute_mode_list(void);
static void canonicalize_vga_mode_list(void);
static void draw_cursor(int x, int y);

bool vdriver_init(int max_width, int max_height, int max_bpp, bool fixed_p,
                  int *argc, char *argv[])
{
    const vga_mode_t *v;
    unsigned long size, max_possible_size;

    /* Perform any host-specific initialization. */
    if(!vgahost_init(max_width, max_height, max_bpp, fixed_p, argc, argv))
        return false;

    /* Grab the "raw" list of modes we have available to us. */
    vga_mode_list = vgahost_compute_vga_mode_list();
    if(vga_mode_list == NULL)
        return false;
    canonicalize_vga_mode_list();

    /* Compute the maximum bpp allowed. */
    vdriver_log2_max_bpp = 0;
    for(v = vga_mode_list; v->width != 0; v++)
        if(v->log2_bpp > vdriver_log2_max_bpp)
            vdriver_log2_max_bpp = v->log2_bpp;
    vdriver_max_bpp = 1 << vdriver_log2_max_bpp;

    /* Figure out the maximum size the hardware could possibly demand. */
    max_possible_size = 0;
    for(v = vga_mode_list; v->width != 0; v++)
    {
        unsigned long new_size;
        new_size = FBUF_SIZE(v->width, v->height, v->log2_bpp);
        if(new_size > max_possible_size)
            max_possible_size = new_size;
    }

    /* If no max specified here, default to a maximum of what the user
   * specified on the command line.  If you start out with 640x480,
   * you can never get any bigger, etc.
   */
    if(max_width == 0)
        max_width = flag_width;
    if(max_height == 0)
        max_height = flag_height;

    /* If no max_width or height is specified, then we'll just allocate
   * enough room for the default mode.
   */
    default_mode = compute_default_mode();
    if(default_mode == NULL)
        return false;
    if(max_width == 0)
        max_width = default_mode->width;
    if(max_height == 0)
        max_height = default_mode->height;
    if(max_bpp == 0)
        max_bpp = vdriver_max_bpp;

    /* Don't let them specify something _too_ small. */
    if(max_width < VDRIVER_MIN_SCREEN_WIDTH)
        max_width = VDRIVER_MIN_SCREEN_WIDTH;
    if(max_height < VDRIVER_MIN_SCREEN_HEIGHT)
        max_height = VDRIVER_MIN_SCREEN_HEIGHT;

    /* Figure out how much space we need to allocate. */
    size = MIN(FBUF_SIZE(max_width, max_height, vdriver_log2_max_bpp),
               max_possible_size);

    /* Allocate memory for the frame buffer. */
    if(size != 0)
    {
        vgahost_alloc_fbuf(size);
        if(vdriver_fbuf != NULL)
            fbuf_size = size;
    }
    else
    {
        vdriver_fbuf = NULL;
        fbuf_size = 0;
    }

    /* Start out in no mode at all. */
    vga_current_mode = NULL;

    /* Compute the list of modes we support. */
    compute_mode_list();

    /* Force a cleanup on program exit. */
    atexit(vdriver_shutdown);

    return (vdriver_fbuf != NULL && default_mode != NULL);
}

/* qsort helper function. */
static int
compare_vga_modes(const void *p1, const void *p2)
{
    const vga_mode_t *v1, *v2;
    int diff;

    v1 = (const vga_mode_t *)p1;
    v2 = (const vga_mode_t *)p2;

    /* First sort by screen size. */
    diff = (v1->width * v1->height) - (v2->width * v2->height);
    if(diff != 0)
        return diff;

    /* Then sort by bpp. */
    diff = v1->log2_bpp - v2->log2_bpp;
    if(diff != 0)
        return diff;

    /* Then sort by planarity.  Put non-planar first. */
    diff = v1->planar_p - v2->planar_p;
    if(diff != 0)
        return diff;

    /* Prefer single window modes (e.g. linear fbufs). */
    diff = v1->multi_window_p - v2->multi_window_p;
    if(diff != 0)
        return diff;

    /* Then sort by interlaced.  Put non-interlaced first. */
    diff = v1->interlaced_p - v2->interlaced_p;
    if(diff != 0)
        return diff;

    /* Prefer modes with distinct read and write windows. */
    diff = (v2->win_read != v2->win_write) - (v1->win_read != v1->win_write);
    if(diff != 0)
        return diff;

    /* They are equal as far as we care.  Just sort by mode number. */
    return v1->mode_number - v2->mode_number;
}

static void
canonicalize_vga_mode_list(void)
{
    vga_mode_t *v, *list_end;
    int num_vga_modes;

    /* Count the number of VGA modes. */
    for(v = vga_mode_list, num_vga_modes = 0; v->width != 0; v++)
        num_vga_modes++;

    /* Put vga modes in canonical order. */
    qsort(vga_mode_list, num_vga_modes, sizeof vga_mode_list[0],
          compare_vga_modes);

    /* Axe all vga modes for which we have no use. */
    list_end = &vga_mode_list[num_vga_modes];
    for(v = vga_mode_list; v->width != 0;)
    {
        if(v->width < VDRIVER_MIN_SCREEN_WIDTH
           || v->height < VDRIVER_MIN_SCREEN_HEIGHT
           || (v->log2_bpp != 2 && v->planar_p) /* only use 16 color planar */
           || v->log2_bpp > 3
           || (v->row_bytes % 4))
        {
            /* Nuke this mode.  Memmove down the width == 0 sentinel as well. */
            memmove(v, v + 1, (char *)list_end - (char *)v);
        }
        else
            v++;
    }
}

static void
compute_mode_list(void)
{
    int i, prev_width, prev_height, num_sizes, max_sizes;
    unsigned long max_bit_size;
    vga_mode_t *m;

    /* If no frame buffer memory has been allocated yet, list all modes. */
    max_bit_size = fbuf_size * 8;

    /* Find the maximum bits per pixel. */
    vdriver_log2_max_bpp = 0;
    for(i = 0; vga_mode_list[i].width != 0; i++)
    {
        int log2_bpp = vga_mode_list[i].log2_bpp;
        if(log2_bpp > vdriver_log2_max_bpp)
            vdriver_log2_max_bpp = log2_bpp;
    }
    vdriver_max_bpp = 1 << vdriver_log2_max_bpp;

    /* Compute the maximum possible number of sizes. */
    for(m = vga_mode_list, max_sizes = 0; m->width != 0; m++)
        max_sizes++;

    /* Allocate enough space for the list. */
    vdriver_mode_list = ((vdriver_modes_t *)
                             malloc(VDRIVER_MODE_LIST_SIZE(max_sizes)));
    memset(vdriver_mode_list, 0, VDRIVER_MODE_LIST_SIZE(max_sizes));

    /* Now list out all the unique sizes we actually found. */
    prev_width = prev_height = 0;
    for(m = vga_mode_list, num_sizes = 0; m->width != 0; m++)
    {
        if((m->width != prev_width || m->height != prev_height)
           && ((m->width * m->height) << vdriver_log2_max_bpp <= max_bit_size))
        {
            vdriver_mode_list->size[num_sizes].height = prev_height = m->height;
            vdriver_mode_list->size[num_sizes].width = prev_width = m->width;
            num_sizes++;
        }
    }

    /* Discard any size that cannot be supported at our max bpp. */
    while(num_sizes > 0)
    {
        int h = vdriver_mode_list->size[num_sizes - 1].height;
        int w = vdriver_mode_list->size[num_sizes - 1].width;

        /* See if we can find a size match at max bpp. */
        for(m = vga_mode_list; m->width != 0; m++)
            if(m->log2_bpp == vdriver_log2_max_bpp
               && m->width == w && m->height == h)
                break;

        /* Axe this mode if we found no match at max bpp. */
        if(m->width == 0)
            --num_sizes;
        else
            break;
    }

    /* Minimize the size of the list. */
    vdriver_mode_list = ((vdriver_modes_t *)
                             realloc(vdriver_mode_list,
                                     VDRIVER_MODE_LIST_SIZE(num_sizes)));

    vdriver_mode_list->num_sizes = num_sizes;
}

/* Default mode will be near to
 * VDRIVER_DEFAULT_SCREEN_WIDTH x VDRIVER_DEFAULT_SCREEN_HEIGHT,
 * with maximum allowed bpp.
 */
static const vga_mode_t *
compute_default_mode(void)
{
    const vga_mode_t *mode;
    long best_error;
    int i;

    mode = NULL;

    best_error = 0x7FFFFFFF;
    for(i = 0; vga_mode_list[i].width != 0; i++)
    {
        if(vga_mode_list[i].log2_bpp == vdriver_log2_max_bpp)
        {
            long err;
            err = (ABS(VDRIVER_DEFAULT_SCREEN_WIDTH
                       - vga_mode_list[i].width)
                   + ABS(VDRIVER_DEFAULT_SCREEN_HEIGHT
                         - vga_mode_list[i].height));
            if(err < best_error)
            {
                mode = &vga_mode_list[i];
                best_error = err;
            }
        }
    }

    return mode;
}

void vdriver_shutdown(void)
{
    static char beenhere = false;

    if(!beenhere)
    {
        /* Let the host perform any necessary cleanup. */
        vgahost_shutdown();

        /* Note that the frame buffer is history.  This isn't really
       * a memory leak, since we're about to exit anyway.
       */
        vdriver_fbuf = NULL;
        fbuf_size = 0;

        beenhere = true;
    }
}

/* This table is used when extracting plane bits; the process of compacting
 * every 4th bit into a byte leaves them in a weird order, so this array
 * compensates for that and places them in the desired order.  It also
 * flips all of the bits.
 */
static uint8 vga_planar_4bpp_unscramble[256] = {
    0xFF, 0xBF, 0xFB, 0xBB, 0xEF, 0xAF, 0xEB, 0xAB, 0xFE, 0xBE, 0xFA,
    0xBA, 0xEE, 0xAE, 0xEA, 0xAA, 0x7F, 0x3F, 0x7B, 0x3B, 0x6F, 0x2F,
    0x6B, 0x2B, 0x7E, 0x3E, 0x7A, 0x3A, 0x6E, 0x2E, 0x6A, 0x2A, 0xF7,
    0xB7, 0xF3, 0xB3, 0xE7, 0xA7, 0xE3, 0xA3, 0xF6, 0xB6, 0xF2, 0xB2,
    0xE6, 0xA6, 0xE2, 0xA2, 0x77, 0x37, 0x73, 0x33, 0x67, 0x27, 0x63,
    0x23, 0x76, 0x36, 0x72, 0x32, 0x66, 0x26, 0x62, 0x22, 0xDF, 0x9F,
    0xDB, 0x9B, 0xCF, 0x8F, 0xCB, 0x8B, 0xDE, 0x9E, 0xDA, 0x9A, 0xCE,
    0x8E, 0xCA, 0x8A, 0x5F, 0x1F, 0x5B, 0x1B, 0x4F, 0x0F, 0x4B, 0x0B,
    0x5E, 0x1E, 0x5A, 0x1A, 0x4E, 0x0E, 0x4A, 0x0A, 0xD7, 0x97, 0xD3,
    0x93, 0xC7, 0x87, 0xC3, 0x83, 0xD6, 0x96, 0xD2, 0x92, 0xC6, 0x86,
    0xC2, 0x82, 0x57, 0x17, 0x53, 0x13, 0x47, 0x07, 0x43, 0x03, 0x56,
    0x16, 0x52, 0x12, 0x46, 0x06, 0x42, 0x02, 0xFD, 0xBD, 0xF9, 0xB9,
    0xED, 0xAD, 0xE9, 0xA9, 0xFC, 0xBC, 0xF8, 0xB8, 0xEC, 0xAC, 0xE8,
    0xA8, 0x7D, 0x3D, 0x79, 0x39, 0x6D, 0x2D, 0x69, 0x29, 0x7C, 0x3C,
    0x78, 0x38, 0x6C, 0x2C, 0x68, 0x28, 0xF5, 0xB5, 0xF1, 0xB1, 0xE5,
    0xA5, 0xE1, 0xA1, 0xF4, 0xB4, 0xF0, 0xB0, 0xE4, 0xA4, 0xE0, 0xA0,
    0x75, 0x35, 0x71, 0x31, 0x65, 0x25, 0x61, 0x21, 0x74, 0x34, 0x70,
    0x30, 0x64, 0x24, 0x60, 0x20, 0xDD, 0x9D, 0xD9, 0x99, 0xCD, 0x8D,
    0xC9, 0x89, 0xDC, 0x9C, 0xD8, 0x98, 0xCC, 0x8C, 0xC8, 0x88, 0x5D,
    0x1D, 0x59, 0x19, 0x4D, 0x0D, 0x49, 0x09, 0x5C, 0x1C, 0x58, 0x18,
    0x4C, 0x0C, 0x48, 0x08, 0xD5, 0x95, 0xD1, 0x91, 0xC5, 0x85, 0xC1,
    0x81, 0xD4, 0x94, 0xD0, 0x90, 0xC4, 0x84, 0xC0, 0x80, 0x55, 0x15,
    0x51, 0x11, 0x45, 0x05, 0x41, 0x01, 0x54, 0x14, 0x50, 0x10, 0x44,
    0x04, 0x40, 0x00,
};

/* This table is used when extracting plane bits; the process of compacting
 * every other bit into a byte leaves them in a weird order, so this array
 * compensates for that and places them in the desired order.  It also
 * flips all of the bits.
 */
static uint8 vga_planar_2bpp_unscramble[256] = {
    0xFF, 0xEF, 0xFE, 0xEE, 0xDF, 0xCF, 0xDE, 0xCE, 0xFD, 0xED, 0xFC,
    0xEC, 0xDD, 0xCD, 0xDC, 0xCC, 0xBF, 0xAF, 0xBE, 0xAE, 0x9F, 0x8F,
    0x9E, 0x8E, 0xBD, 0xAD, 0xBC, 0xAC, 0x9D, 0x8D, 0x9C, 0x8C, 0xFB,
    0xEB, 0xFA, 0xEA, 0xDB, 0xCB, 0xDA, 0xCA, 0xF9, 0xE9, 0xF8, 0xE8,
    0xD9, 0xC9, 0xD8, 0xC8, 0xBB, 0xAB, 0xBA, 0xAA, 0x9B, 0x8B, 0x9A,
    0x8A, 0xB9, 0xA9, 0xB8, 0xA8, 0x99, 0x89, 0x98, 0x88, 0x7F, 0x6F,
    0x7E, 0x6E, 0x5F, 0x4F, 0x5E, 0x4E, 0x7D, 0x6D, 0x7C, 0x6C, 0x5D,
    0x4D, 0x5C, 0x4C, 0x3F, 0x2F, 0x3E, 0x2E, 0x1F, 0x0F, 0x1E, 0x0E,
    0x3D, 0x2D, 0x3C, 0x2C, 0x1D, 0x0D, 0x1C, 0x0C, 0x7B, 0x6B, 0x7A,
    0x6A, 0x5B, 0x4B, 0x5A, 0x4A, 0x79, 0x69, 0x78, 0x68, 0x59, 0x49,
    0x58, 0x48, 0x3B, 0x2B, 0x3A, 0x2A, 0x1B, 0x0B, 0x1A, 0x0A, 0x39,
    0x29, 0x38, 0x28, 0x19, 0x09, 0x18, 0x08, 0xF7, 0xE7, 0xF6, 0xE6,
    0xD7, 0xC7, 0xD6, 0xC6, 0xF5, 0xE5, 0xF4, 0xE4, 0xD5, 0xC5, 0xD4,
    0xC4, 0xB7, 0xA7, 0xB6, 0xA6, 0x97, 0x87, 0x96, 0x86, 0xB5, 0xA5,
    0xB4, 0xA4, 0x95, 0x85, 0x94, 0x84, 0xF3, 0xE3, 0xF2, 0xE2, 0xD3,
    0xC3, 0xD2, 0xC2, 0xF1, 0xE1, 0xF0, 0xE0, 0xD1, 0xC1, 0xD0, 0xC0,
    0xB3, 0xA3, 0xB2, 0xA2, 0x93, 0x83, 0x92, 0x82, 0xB1, 0xA1, 0xB0,
    0xA0, 0x91, 0x81, 0x90, 0x80, 0x77, 0x67, 0x76, 0x66, 0x57, 0x47,
    0x56, 0x46, 0x75, 0x65, 0x74, 0x64, 0x55, 0x45, 0x54, 0x44, 0x37,
    0x27, 0x36, 0x26, 0x17, 0x07, 0x16, 0x06, 0x35, 0x25, 0x34, 0x24,
    0x15, 0x05, 0x14, 0x04, 0x73, 0x63, 0x72, 0x62, 0x53, 0x43, 0x52,
    0x42, 0x71, 0x61, 0x70, 0x60, 0x51, 0x41, 0x50, 0x40, 0x33, 0x23,
    0x32, 0x22, 0x13, 0x03, 0x12, 0x02, 0x31, 0x21, 0x30, 0x20, 0x11,
    0x01, 0x10, 0x00,
};

/* Extracts out every 4th bit into a byte, mapping bit 24->0, 28->1, 16->2,
 * 20->3, 8->4, 12->5, 0->6, 4->7.  Call this with x, x >> 1, x >> 2,
 * and x >> 3 to extract the 4 planes from a 32 bit value.
 */
static inline uint8 const
extract_plane_4bpp_bits(unsigned long n)
{
    n &= 0x11111111;
    n |= n >> 15;
    n |= n >> 6;
    return vga_planar_4bpp_unscramble[n & 0xFF];
}

/* Extracts out every other bit of a short into a byte, mapping bit
 * 8->0, 10->1, 12->2, 14->3, 0->4, 2->5, 4->6, 6->7.
 */
static inline uint8 const
extract_plane_2bpp_bits(unsigned n)
{
    n &= 0x5555;
    n |= n >> 7;
    return vga_planar_2bpp_unscramble[n & 0xFF];
}

#define BLT_ROW_TO_PLANE(plane, bits_func, dst_row_bytes, in_type)          \
    {                                                                       \
        register char *tmp_out asm("%di");                                  \
        int j;                                                              \
                                                                            \
        VGA_SET_WRITE_PLANE(plane);                                         \
                                                                            \
        tmp_out = out;                                                      \
        for(j = (dst_row_bytes)-1; j >= 0; j--)                             \
        {                                                                   \
            asm volatile("stosb" /* Implicitly uses %es for dst segment. */ \
                         : "=D"(tmp_out)                                    \
                         : "0"(tmp_out),                                    \
                           "a"(bits_func((((unsigned in_type *)in)[j])      \
                                         >> (plane)))                       \
                         : "memory");                                       \
        }                                                                   \
    }

static void
update_2bpp_4bpp_planar(unsigned top, unsigned left, unsigned bottom,
                        unsigned right)
{
    uint8 *in, *out;
    unsigned plane_row_bytes;
    int i;

    plane_row_bytes = (right - left) / 8; /* Each pixel takes 1 bit per plane */
    in = &vdriver_fbuf[(top * vdriver_row_bytes) + (left * 2) / 8];
    out = &vga_portal_baseaddr[top * vga_current_mode->row_bytes
                               + left / 8 /* planar -> bpp is effectively 1 */
                               + plane_row_bytes - 1
                               + vga_first_byte_offset];

    for(i = bottom - top; i > 0; i--)
    {
        asm("std"); /* Here in case check_virtual_interrupt() changes it. */

        BLT_ROW_TO_PLANE(0, extract_plane_2bpp_bits, plane_row_bytes,
                         short);
        BLT_ROW_TO_PLANE(1, extract_plane_2bpp_bits, plane_row_bytes,
                         short);

        in += vdriver_row_bytes;
        out += vga_current_mode->row_bytes;
    }
}

static void
update_4bpp_4bpp_planar(unsigned top, unsigned left, unsigned bottom,
                        unsigned right)
{
    uint8 *in, *out;
    unsigned plane_row_bytes;
    int i;

    plane_row_bytes = (right - left) / 8; /* Each pixel takes 1 bit per plane */
    in = &vdriver_fbuf[(top * vdriver_row_bytes) + (left * 4) / 8];
    out = &vga_portal_baseaddr[top * vga_current_mode->row_bytes
                               + left / 8 /* planar -> bpp is effectively 1 */
                               + plane_row_bytes - 1
                               + vga_first_byte_offset];

    for(i = bottom - top; i > 0; i--)
    {
        asm("std"); /* Here in case check_virtual_interrupt() changes it. */

        BLT_ROW_TO_PLANE(0, extract_plane_4bpp_bits, plane_row_bytes, long);
        BLT_ROW_TO_PLANE(1, extract_plane_4bpp_bits, plane_row_bytes, long);
        BLT_ROW_TO_PLANE(2, extract_plane_4bpp_bits, plane_row_bytes, long);
        BLT_ROW_TO_PLANE(3, extract_plane_4bpp_bits, plane_row_bytes, long);

        in += vdriver_row_bytes;
        out += vga_current_mode->row_bytes;
    }
}

#define TRANSFER_NOFLIP_1_1(a, i, o, c)                 \
    do                                                  \
    {                                                   \
        int __unused;                                   \
                                                        \
        asm volatile("cld\n\t"                          \
                     "rep\n\t"                          \
                     "movsl"                            \
                     : "=S"(i), "=D"(o), "=c"(__unused) \
                     : "0"(i), "1"(o), "2"(c));         \
    } while(false)

#define TRANSFER_1_1(a, inp, outp, c)                                                                                                                  \
    do                                                                                                                                                 \
    {                                                                                                                                                  \
        int __unused;                                                                                                                                  \
                                                                                                                                                       \
        asm volatile(VGA_IF_SEL("pushl %%ds\n\t") "pushl %%ebp\n\t"                                                                                    \
                                                  "movl %%esi,%%ebp\n\t" VGA_IF_SEL("movw %3,%%ds\n\t") "movl %%ebx,%%ecx\n\t"                         \
                                                                                                        "shrl $2,%%ecx\n\t"                            \
                                                                                                        "andb $3,%%bl\n\t"                             \
                                                                                                        "jz 2f\n"                                      \
                                                                                                        "0:\n\t"                                       \
                                                                                                        "movl (%%ebp),%%eax\n\t"                       \
                                                                                                        "xorl $-1,%%eax\n\t"                           \
                                                                                                        "addl $4,%%ebp\n\t"                            \
                                                                                                        "movl %%eax,(%%edi)\n\t"                       \
                                                                                                        "addl $4,%%edi\n\t"                            \
                                                                                                        "decb %%bl\n\t"                                \
                                                                                                        "jnz 0b\n\t"                                   \
                                                                                                        "decl %%ecx\n\t"                               \
                                                                                                        "js 3f\n"                                      \
                                                                                                        "1:\n\t"                                       \
                                                                                                        "movl (%%ebp),%%eax\n\t"                       \
                                                                                                        "movl 4(%%ebp),%%ebx\n\t"                      \
                                                                                                        "xorl $-1,%%eax\n\t" /* xorl is pairable... */ \
                                                                                                        "xorl $-1,%%ebx\n\t" /*   ...notl isn't.    */ \
                                                                                                        "movl %%eax,(%%edi)\n\t"                       \
                                                                                                        "movl %%ebx,4(%%edi)\n\t"                      \
                                                                                                        "movl 8(%%ebp),%%eax\n\t"                      \
                                                                                                        "movl 12(%%ebp),%%ebx\n\t"                     \
                                                                                                        "xorl $-1,%%eax\n\t"                           \
                                                                                                        "xorl $-1,%%ebx\n\t"                           \
                                                                                                        "movl %%eax,8(%%edi)\n\t"                      \
                                                                                                        "movl %%ebx,12(%%edi)\n\t"                     \
                                                                                                        "addl $16,%%ebp\n\t"                           \
                                                                                                        "addl $16,%%edi\n"                             \
                                                                                                        "2:\n\t"                                       \
                                                                                                        "decl %%ecx\n\t"                               \
                                                                                                        "jns 1b\n"                                     \
                                                                                                        "3:\n\t"                                       \
                                                                                                        "movl %%ebp,%%esi\n\t"                         \
                                                                                                        "popl %%ebp" VGA_IF_SEL("\n\tpopl %%ds")       \
                     : "=S"(inp), "=D"(outp), "=b"(__unused)                                                                                           \
                     : "g"(VGA_SELECTOR), "0"(inp), "1"(outp), "2"(c)                                                                                  \
                     : "ax", "cx", "cc");                                                                                                              \
    } while(false)

#define TRANSFER_1_2(a, i, o, c)                                           \
    do                                                                     \
    {                                                                      \
        int __unused;                                                      \
                                                                           \
        asm volatile("xorl %%ebx,%%ebx\n\t"                                \
                     "xorl %%edx,%%edx\n\t"                                \
                     "jmp 1f\n"                                            \
                     "0:\n\t"                                              \
                     "movb (%%esi),%%bl\n\t"                               \
                     "movb 1(%%esi),%%dl\n\t"                              \
                     "movl %3(,%%ebx,8),%%eax\n\t"                         \
                     "movb 2(%%esi),%%bl\n\t"                              \
                     "orl %3+4(,%%edx,8),%%eax\n\t"                        \
                     "movb 3(%%esi),%%dl\n\t"                              \
                     "stosl\n\t"                                           \
                     "movl %3(,%%ebx,8),%%eax\n\t"                         \
                     "addl $4,%%esi\n\t"                                   \
                     "orl %3+4(,%%edx,8),%%eax\n\t"                        \
                     "stosl\n\t"                                           \
                     "1:\n"                                                \
                     "decl %%ecx\n\t"                                      \
                     "jns 0b"                                              \
                     : "=S"(i), "=D"(o), "=c"(__unused)                    \
                     : "m"(*(volatile char *)&(a)), "0"(i), "1"(o), "2"(c) \
                     : "ax", "bx", "dx", "cc");                            \
    } while(false)

#define TRANSFER_1_4(a, i, o, c)                                               \
    do                                                                         \
    {                                                                          \
        int __unused;                                                          \
                                                                               \
        /* We need inline assembly to handle stosl (for segment overrides). */ \
        asm volatile("xorl %%ebx,%%ebx\n\t"                                    \
                     "jmp 1f\n"                                                \
                     "0:\n\t"                                                  \
                     "movb (%%esi),%%bl\n\t"                                   \
                     "movl %3(,%%ebx,4),%%eax\n\t"                             \
                     "movb 1(%%esi),%%bl\n\t"                                  \
                     "stosl\n\t"                                               \
                     "movl %3(,%%ebx,4),%%eax\n\t"                             \
                     "movb 2(%%esi),%%bl\n\t"                                  \
                     "stosl\n\t"                                               \
                     "movl %3(,%%ebx,4),%%eax\n\t"                             \
                     "movb 3(%%esi),%%bl\n\t"                                  \
                     "stosl\n\t"                                               \
                     "movl %3(,%%ebx,4),%%eax\n\t"                             \
                     "stosl\n\t"                                               \
                     "addl $4,%%esi\n"                                         \
                     "1:\n\t"                                                  \
                     "decl %%ecx\n\t"                                          \
                     "jns 0b"                                                  \
                     : "=S"(i), "=D"(o), "=c"(__unused)                        \
                     : "m"(*(volatile char *)&(a)), "0"(i), "1"(o), "2"(c)     \
                     : "ax", "bx", "cc");                                      \
    } while(false)

#define TRANSFER_1_8(a, i, o, c)                                                                                                                                                                                                                                                                                         \
    do                                                                                                                                                                                                                                                                                                                   \
    {                                                                                                                                                                                                                                                                                                                    \
        int __unused;                                                                                                                                                                                                                                                                                                    \
                                                                                                                                                                                                                                                                                                                         \
        /* We need inline assembly to handle segment overrides. */                                                                                                                                                                                                                                                       \
        asm volatile("xorl %%ebx,%%ebx\n\t"                                                                                                                                                                                                                                                                              \
                     "jmp 1f\n"                                                                                                                                                                                                                                                                                          \
                     "0:\n\t"                                                                                                                                                                                                                                                                                            \
                     "movb (%%esi),%%bl\n\t"                                                                                                                                                                                                                                                                             \
                     "movl %3(,%%ebx,8),%%eax\n\t"                                                                                                                                                                                                                                                                       \
                     "movl %3+4(,%%ebx,8),%%edx\n\t"                                                                                                                                                                                                                                                                     \
                     "movb 1(%%esi),%%bl\n\t"                                                                                                                                                                                                                                                                            \
                     "movl %%eax," VGA_IF_SEL("%%es:") "(%%edi)\n\t"                                                                                                                                                                                                                                                     \
                                                       "movl %%edx," VGA_IF_SEL("%%es:") "4(%%edi)\n\t"                                                                                                                                                                                                                  \
                                                                                         "movl %3(,%%ebx,8),%%eax\n\t"                                                                                                                                                                                                   \
                                                                                         "movl %3+4(,%%ebx,8),%%edx\n\t"                                                                                                                                                                                                 \
                                                                                         "movb 2(%%esi),%%bl\n\t"                                                                                                                                                                                                        \
                                                                                         "movl %%eax," VGA_IF_SEL("%%es:") "8(%%edi)\n\t"                                                                                                                                                                                \
                                                                                                                           "movl %%edx," VGA_IF_SEL("%%es:") "12(%%edi)\n\t"                                                                                                                                             \
                                                                                                                                                             "movl %3(,%%ebx,8),%%eax\n\t"                                                                                                                               \
                                                                                                                                                             "movl %3+4(,%%ebx,8),%%edx\n\t"                                                                                                                             \
                                                                                                                                                             "movb 3(%%esi),%%bl\n\t"                                                                                                                                    \
                                                                                                                                                             "movl %%eax," VGA_IF_SEL("%%es:") "16(%%edi)\n\t"                                                                                                           \
                                                                                                                                                                                               "movl %%edx," VGA_IF_SEL("%%es:") "20(%%edi)\n\t"                                                                         \
                                                                                                                                                                                                                                 "movl %3(,%%ebx,8),%%eax\n\t"                                                           \
                                                                                                                                                                                                                                 "movl %3+4(,%%ebx,8),%%edx\n\t"                                                         \
                                                                                                                                                                                                                                 "movl %%eax," VGA_IF_SEL("%%es:") "24(%%edi)\n\t"                                       \
                                                                                                                                                                                                                                                                   "movl %%edx," VGA_IF_SEL("%%es:") "28(%%edi)\n\t"     \
                                                                                                                                                                                                                                                                                                     "addl $4,%%esi\n\t" \
                                                                                                                                                                                                                                                                                                     "addl $32,%%edi\n"  \
                                                                                                                                                                                                                                                                                                     "1:\n\t"            \
                                                                                                                                                                                                                                                                                                     "decl %%ecx\n\t"    \
                                                                                                                                                                                                                                                                                                     "jns 0b"            \
                     : "=S"(i), "=D"(o), "=c"(__unused)                                                                                                                                                                                                                                                                  \
                     : "m"(*(volatile char *)&(a)), "0"(i), "1"(o), "c"(2)                                                                                                                                                                                                                                               \
                     : "ax", "bx", "dx", "cc");                                                                                                                                                                                                                                                                          \
    } while(false)

#define UPDATE_FUNC(name, transfer, out_size)                                  \
    static void                                                                \
    name(unsigned top, unsigned left, unsigned bottom, unsigned right)         \
    {                                                                          \
        unsigned out_index;                                                    \
        register uint8 *in asm("%si");                                         \
        int rows_left;                                                         \
        unsigned long win_gran, win_size;                                      \
        long left_byte, right_byte, xfer_in_bytes, xfer_out_bytes;             \
        int extra_longs;                                                       \
        int window_num;                                                        \
        unsigned long vga_row_bytes;                                           \
                                                                               \
        left_byte = (left << 2) >> (5 - vdriver_log2_bpp);                     \
        right_byte = (right << 2) >> (5 - vdriver_log2_bpp);                   \
        xfer_in_bytes = right_byte - left_byte;                                \
        xfer_out_bytes = xfer_in_bytes * (out_size);                           \
        vga_row_bytes = vga_current_mode->row_bytes;                           \
                                                                               \
        /* Compute the indices of the first bytes in and out. */               \
        in = &vdriver_fbuf[top * vdriver_row_bytes + left_byte];               \
        out_index = (top * vga_row_bytes + left_byte * (out_size)              \
                     + vga_first_byte_offset);                                 \
        win_gran = VGA_WINDOW_GRANULARITY(vga_current_mode);                   \
        win_size = (vga_current_mode->multi_window_p                           \
                        ? VGA_WINDOW_SIZE(vga_current_mode)                    \
                        : 0x4000000 /* arbitrary large power of 2. */);        \
        window_num = 0;                                                        \
        for(rows_left = bottom - top, extra_longs = 0; rows_left > 0;)         \
        {                                                                      \
            long bytes_left_in_window, rows;                                   \
            register uint8 *out asm("%di");                                    \
            uint8 *out_start;                                                  \
                                                                               \
            if(vga_current_mode->multi_window_p)                               \
            {                                                                  \
                VGA_IF_SEL(asm("pushl %ds ; popl %es"));                       \
                window_num = out_index / win_gran;                             \
                vgahost_set_write_window(window_num);                          \
                VGA_IF_SEL(asm("movw %0,%%es"                                  \
                               :                                               \
                               : "g"(VGA_SELECTOR)));                          \
            }                                                                  \
                                                                               \
            /* Fetch a pointer to the first long. */                           \
            out = out_start = &vga_portal_baseaddr[out_index                   \
                                                   - (window_num * win_gran)]; \
                                                                               \
            /* Transfer any extra cruft on this row. */                        \
            if(extra_longs > 0)                                                \
            {                                                                  \
                transfer(depth_conversion_table, in, out, extra_longs);        \
                out += vga_row_bytes - xfer_out_bytes;                         \
                in += vdriver_row_bytes - xfer_in_bytes;                       \
                rows_left--;                                                   \
            }                                                                  \
                                                                               \
            /* Figure out how many bytes we have left in this window. */       \
            bytes_left_in_window = &vga_portal_baseaddr[win_size] - out;       \
                                                                               \
            /* Transfer everything that fits within this write window. */      \
            rows = bytes_left_in_window / vga_row_bytes;                       \
            if(rows * vga_row_bytes + xfer_out_bytes <= bytes_left_in_window)  \
                rows++;                                                        \
            if(rows > rows_left)                                               \
                rows = rows_left;                                              \
                                                                               \
            /* Account for the memory we are about to process. */              \
            rows_left -= rows;                                                 \
            bytes_left_in_window -= rows * vga_row_bytes;                      \
                                                                               \
            if(xfer_out_bytes == vga_row_bytes                                 \
               && xfer_in_bytes == vdriver_row_bytes)                          \
            {                                                                  \
                transfer(depth_conversion_table, in, out,                      \
                         rows *xfer_in_bytes / 4);                             \
            }                                                                  \
            else /* Oh well, these rows aren't the width of the screen. */     \
            {                                                                  \
                unsigned xfer_longs, in_add, out_add;                          \
                                                                               \
                in_add = vdriver_row_bytes - xfer_in_bytes;                    \
                out_add = vga_row_bytes - xfer_out_bytes;                      \
                xfer_longs = xfer_in_bytes / 4;                                \
                                                                               \
                for(; rows > 0; rows--)                                        \
                {                                                              \
                    transfer(depth_conversion_table, in, out, xfer_longs);     \
                    in += in_add;                                              \
                    out += out_add;                                            \
                }                                                              \
            }                                                                  \
                                                                               \
            /* Now handle cleanup for any fractional row. */                   \
            if(rows_left > 0 && bytes_left_in_window > 0)                      \
            {                                                                  \
                long frac_longs;                                               \
                                                                               \
                /* Compute how many extra longs we have, and how many we       \
                 * need to polish off once we switch banks.                    \
                 */                                                            \
                frac_longs = (bytes_left_in_window / (4 * (out_size)));        \
                extra_longs = xfer_in_bytes / 4 - frac_longs;                  \
                transfer(depth_conversion_table, in, out, frac_longs);         \
            }                                                                  \
            else                                                               \
            {                                                                  \
                extra_longs = 0;                                               \
            }                                                                  \
                                                                               \
            /* Note the index of the next byte to transfer. */                 \
            out_index += out - out_start;                                      \
                                                                               \
            check_virtual_interrupt();                                         \
        }                                                                      \
    }

/* Here we define the functions for different transfer types.
 * For example, if our internal frame buffer is 1 bpp and the hardware
 * is 2bpp, we would use `update_1_2'.  If instead we are doing
 * 4bpp->8bpp, we still use `update_1_2'; it's the ratio of in to out
 * that matters.
 */
UPDATE_FUNC(update_packed_no_flip, TRANSFER_NOFLIP_1_1, 1)
UPDATE_FUNC(update_packed, TRANSFER_1_1, 1)
UPDATE_FUNC(update_1_2, TRANSFER_1_2, 2)
UPDATE_FUNC(update_1_4, TRANSFER_1_4, 4)
UPDATE_FUNC(update_1_8, TRANSFER_1_8, 8)

/* When the entire hardware frame buffer fits within the low memory
 * window, and we don't need to flip bits as we transfer them, we'll
 * use this simpler blitter.  D flag is known to be clear on entry.
 */
static void
update_packed_no_paging(unsigned top, unsigned left, unsigned bottom,
                        unsigned right)
{
    register uint8 *in asm("%si");
    register uint8 *out asm("%di");
    unsigned bytes_in;
    unsigned long_width, log2_bpp;

    log2_bpp = vdriver_log2_bpp;
    long_width = (right - left) >> (5 - log2_bpp);
    bytes_in = (top * vga_current_mode->row_bytes) + (left >> (3 - log2_bpp));
    out = &vga_portal_baseaddr[bytes_in + vga_first_byte_offset];

    if(long_width * 4 == vdriver_row_bytes
       && vdriver_row_bytes == vga_current_mode->row_bytes)
    {
        in = &vdriver_fbuf[bytes_in];

        /* Special case transfers the width of the screen. */
        TRANSFER_NOFLIP_1_1(NULL, in, out, long_width * (bottom - top));
    }
    else
    {
        int rows_left;
        int in_add, out_add;

        in = &vdriver_fbuf[(top * vdriver_row_bytes) + (left >> (3 - log2_bpp))];
        in_add = vdriver_row_bytes - long_width * 4;
        out_add = vga_current_mode->row_bytes - long_width * 4;

        for(rows_left = bottom - top; rows_left > 0; rows_left--)
        {
            TRANSFER_NOFLIP_1_1(NULL, in, out, long_width);
            in += in_add;
            out += out_add;
        }
    }
    check_virtual_interrupt();
}

/* This function fills the entire visible frame buffer with a particular
 * long.  We can't just use vdriver_update_screen() since the internal
 * frame buffer may be smaller than the real frame buffer.
 */
static void
fill_hardware_fbuf(unsigned long fill_long)
{
    unsigned long addr, size, xfer_longs, win_size;

    size = vga_current_mode->row_bytes * vga_current_mode->height;
    if(vga_current_mode->multi_window_p)
        win_size = VGA_WINDOW_SIZE(vga_current_mode);
    else
        win_size = size;
    xfer_longs = win_size / 4;

    /* Loop through the entire frame buffer, filling it with the byte. */
    for(addr = 0; addr < size; addr += win_size)
    {
        int unused;

        /* Set up the write window appropriately. */
        if(vga_current_mode->multi_window_p)
            vgahost_set_write_window(addr
                                     / VGA_WINDOW_GRANULARITY(vga_current_mode));

        /* Make sure we don't write past the end. */
        if(addr + win_size > size)
            xfer_longs = (size - addr) / 4;

        /* Actually do the transfer. */
        asm volatile(VGA_IF_SEL("pushl %%es ; movw %2,%%es\n\t") "cld\n\t"
                                                                 "rep\n\t"
                                                                 "stosl" VGA_IF_SEL("\n\tpopl %%es")
                     : "=c"(unused), "=D"(unused)
                     : "g"(VGA_SELECTOR), "1"(vga_portal_baseaddr),
                       "0"(xfer_longs), "a"(fill_long));
    }
}

/* Updates the hardware screen from the internal frame buffer with
 * the specified rectangle.  Returns true if it succeeded, or false
 * if the update is busy (which can only happen for the cursor).
 * If you're not drawing the cursor, you should ignore this
 * return value.
 */
int vdriver_update_screen(int top, int left, int bottom, int right,
                          bool cursor_p)
{
    static uint8 busy_p = false;
    static uint8 redraw_cursor_p = false;
    uint8 old_busy_p;
    int log2_bpp;

    if(vdriver_fbuf == NULL || vga_current_mode == NULL
       || VDRIVER_BYPASS_INTERNAL_FBUF_P())
        return false;
    log2_bpp = vdriver_log2_bpp;

    /* Pin the rectangle inside the frame buffer and line left and right
   * up on long boundaries.
   */
    if(top < 0)
        top = 0;
    if(left <= 0)
        left = 0;
    else /* Line left up on a long. */
        left &= ~(31 >> log2_bpp);
    if(right >= vdriver_width)
        right = vdriver_width;
    else /* Line right up on a long. */
        right = (((right << log2_bpp) + 31) & ~31) >> log2_bpp;
    if(bottom > vdriver_height)
        bottom = vdriver_height;

    asm volatile("movb $1,%0\n\t" /* Atomically set busy_p to true  */
                 "xchgb %b0,%1" /*  and fetch busy_p's old value. */
                 : "=abcd"(old_busy_p), "=m"(busy_p));

    redraw_cursor_p |= old_busy_p;

    if(!old_busy_p && right > left && bottom > top)
    {
        VGA_IF_SEL(volatile uint16 saved_es);

        /* Set up %es to access the raw frame buffer window. */
        VGA_IF_SEL(asm volatile("movw %%es,%0\n\t"
                                "movw %w1,%%es"
                                : "=m"(saved_es)
                                : "g"(VGA_SELECTOR)));

        /* Make sure the d flag is clear (for postincrement). */
        asm("cld");

        /* Really update the screen. */
        (*screen_update_func)(top, left, bottom, right);

        /* Restore es, and assure the d flag is still clear (broken djgpp
       * libs expect a clear d flag).
       */
        VGA_IF_SEL(asm("movw %0,%%es"
                       :
                       : "m"(saved_es)));
        asm("cld");

        /* Restore the old busy flag. */
        busy_p = old_busy_p;

        /* If the area we just displayed intersects the cursor,
       * or if a cursor update came in while we were busy updating the
       * screen, then redraw the cursor.
       */
        if(redraw_cursor_p
           || (!cursor_p && cursor_visible_p
               && (left < cursor_right && right > cursor_left
                   && top < cursor_bottom && bottom > cursor_top)))
        {
            redraw_cursor_p = false;
            draw_cursor(cursor_x, cursor_y);
        }
    }
    else
    {
        busy_p = old_busy_p;
    }

    return !old_busy_p;
}

int vdriver_update_screen_rects(int num_rects, const vdriver_rect_t *r,
                                bool cursor_p)
{
    int i, p;

    for(i = 0, p = false; i < num_rects; i++)
    {
        p = vdriver_update_screen(r[i].top, r[i].left, r[i].bottom, r[i].right,
                                  cursor_p);
        check_virtual_interrupt();
    }

    return p;
}

bool vdriver_acceptable_mode_p(int width, int height, int bpp,
                               bool grayscale_p,
                               bool exact_match_p)
{
    const vdriver_modes_t *vm;
    int i, log2_bpp, new_width;

    /* Get the currently legal modes. */
    vm = vdriver_mode_list;
    if(vm->num_sizes == 0)
        return false;

    if(width == 0)
    {
        if(vdriver_width == 0)
            width = default_mode->width;
        else
            width = vdriver_width;
    }

    if(height == 0)
    {
        if(vdriver_height == 0)
            height = default_mode->height;
        else
            height = vdriver_height;
    }

    if(bpp == 0)
    {
        if(vdriver_log2_bpp == 0)
            log2_bpp = default_mode->log2_bpp;
        else
            log2_bpp = vdriver_log2_bpp;
        bpp = 1 << log2_bpp;
    }
    else
        log2_bpp = ROMlib_log2[bpp];

    /* First, check the bpp. */
    if((bpp != 1 && bpp != 2 && bpp != 4 && bpp != 8)
       || bpp > vdriver_max_bpp)
        return false;

    /* If their requested width isn't an even multiple of 32 bits wide,
   * bump it up a bit so it is.
   */
    new_width = (((width << log2_bpp) + 31) & ~31) >> log2_bpp;
    if(exact_match_p && new_width != width)
        return false;
    width = new_width;

    /* If they want more memory than we've allocated, they fail! */
    if(FBUF_SIZE(width, height, log2_bpp) > fbuf_size)
        return false;

    /* If their size is too small, fail! */
    if(width < VDRIVER_MIN_SCREEN_WIDTH || height < VDRIVER_MIN_SCREEN_HEIGHT)
        return false;

    /* Make sure the mode isn't too large for our hardware. */
    for(i = vm->num_sizes - 1; i >= 0; i--)
    {
        if(exact_match_p)
        {
            if(width == vm->size[i].width && height == vm->size[i].height)
                break;
        }
        else if(width <= vm->size[i].width && height <= vm->size[i].height)
            break;
    }

    if(i < 0)
        return false;

    if(vgahost_illegal_mode_p(width, height, bpp, exact_match_p))
        return false;

    return true;
}

void vdriver_get_colors(int first_color, int num_colors, ColorSpec *colors)
{
    gui_fatal("`!vdriver_fixed_clut_p' and `vdriver_get_colors ()' called");
}

void vdriver_set_colors(int first_color, int num_colors,
                        const ColorSpec *color_array)
{
    /* Can't set colors before a mode set. */
    if(vga_current_mode == NULL)
        return;

    /* You aren't allowed to change the colors in 1bpp planar mode
   * (they have magic values).  Also, punt empty color lists.
   */
    if((vga_current_mode->planar_p && vdriver_log2_bpp == 0)
       || num_colors == 0)
        return;

    /* Map the specified color to the actual hardware color we use.
   * We don't have a one-to-one mapping because we want the border
   * to be black, and it's fixed at color 0 in SVGA modes.
   * Therefore we flip bits when mapping from internal->hardware
   * colors.  This causes a performance hit in 8bpp/8bpp mode, but
   * just changes the lookup tables when we are doing depth
   * conversions.
   */
    if(vdriver_flip_real_screen_pixels_p)
    {
        ColorSpec *new_color_array;
        int i;

        new_color_array = (ColorSpec *)alloca(num_colors
                                              * sizeof new_color_array[0]);

        /* Reverse the color ordering. */
        for(i = num_colors - 1; i >= 0; i--)
            new_color_array[i] = color_array[num_colors - 1 - i];

        /* Note the actual hardware colors we want to change. */
        color_array = new_color_array;
        first_color = -(first_color + num_colors);
        if(vga_current_mode->planar_p)
            first_color += 16;
        else
            first_color += 1 << vdriver_bpp;
    }

    vgahost_set_colors(first_color, num_colors, color_array);
}

/* Sets the graphics mode to the fastest mode which present the
 * specified display to the user.  The actual hardware mode chosen may
 * or may not have the same bits per pixel specified, but it will
 * appear to.  If width or height is zero, the current width and
 * height will be retained; if there is not yet a current width and
 * height, it will choose sensible defaults.  Clears the display to
 * color 0.  Returns true if it successfully changed the mode, else
 * false.
 */
bool vdriver_set_mode(int width, int height, int bpp, bool grayscale_p)
{
    vga_mode_t *v, *best;
    int log2_bpp, best_bpp_diff, bpp_diff;
    unsigned long best_area_diff;
    int success_p = true;
    bool old_shadow_p;

    if(!vdriver_acceptable_mode_p(width, height, bpp, grayscale_p, false))
        return false;

    /* Note whether we used to have a shadow screen. */
    old_shadow_p = ROMlib_shadow_screen_p;

    if(width == 0)
    {
        if(vdriver_width == 0)
            width = default_mode->width;
        else
            width = vdriver_width;
    }

    if(height == 0)
    {
        if(vdriver_height == 0)
            height = default_mode->height;
        else
            height = vdriver_height;
    }

    if(bpp == 0)
    {
        if(vdriver_log2_bpp == 0)
            log2_bpp = default_mode->log2_bpp;
        else
            log2_bpp = vdriver_log2_bpp;
        bpp = 1 << log2_bpp;
    }
    else
        log2_bpp = ROMlib_log2[bpp];

    /* If their requested width isn't an even multiple of 32 bits wide,
   * bump it up until it is.
   */
    width = (((width << log2_bpp) + 31) & ~31) >> log2_bpp;

    best_bpp_diff = 1000;
    best_area_diff = ~0UL;
    for(v = vga_mode_list, best = NULL; v->width != 0; v++)
    {
        if(v->width >= width && v->height >= height && v->log2_bpp >= log2_bpp)
        {
            unsigned long area_diff;

            area_diff = (v->width * v->height) - (width * height);
            if(area_diff <= best_area_diff) /* Screen size overrides. */
            {
                int log2_bpp_diff = v->log2_bpp - log2_bpp;

                /* We can get 1bpp from planar 4bpp "for free". */
                if(log2_bpp == 0 && v->planar_p)
                    log2_bpp_diff = 0;

                if(area_diff < best_area_diff)
                {
                    /* Find the closest match in area first. */
                    best_area_diff = area_diff;
                    best_bpp_diff = log2_bpp_diff;
                    best = v;
                }
                else if(log2_bpp_diff < best_bpp_diff
                        || (log2_bpp != 0
                            && best != NULL
                            && best->planar_p
                            && !v->planar_p))
                {
                    /* Otherwise, find best bits-per-pixel match. */
                    best_bpp_diff = log2_bpp_diff;
                    best = v;
                }
            }
        }
    }

    if(best == NULL)
    {
        success_p = false;
        goto done;
    }

    /* Set up the write window to point to window 0.  We do this for
   * old-style VGA modes, since we might be switching to one from a
   * multibank VESA mode.  Of course, the mode set PROBABLY resets the
   * window, but maybe some driver doesn't, so let's be paranoid.
   */
    if(vga_current_mode != NULL)
    {
        vga_read_window = vga_write_window = -1;
        vgahost_set_read_window(0);
        vgahost_set_write_window(0);
    }

    /* Switch to this hardware mode. */
    if(!vgahost_set_mode(best))
        return false;

    /* Note some information about doing direct-to-screen blits. */

    /* We only allow direct-to-screen blits for packed pixel screens
   * where no depth conversion is required.
   */
    vdriver_real_screen_blit_possible_p = ((!best->planar_p || log2_bpp == 0)
                                           && !best->multi_window_p
                                           && (best->log2_bpp == log2_bpp
                                               || (best->planar_p
                                                   && log2_bpp == 0)));
    vdriver_flip_real_screen_pixels_p = (!vdriver_real_screen_blit_possible_p
                                         && (!best->planar_p || log2_bpp != 0));

    ROMlib_shadow_screen_p = true;
    if(vdriver_real_screen_blit_possible_p
       && vgahost_mmap_linear_fbuf(best))
    {
        vga_portal_baseaddr = vdriver_fbuf;
        vdriver_real_screen_baseaddr = vdriver_fbuf;
        ROMlib_shadow_screen_p = false;
    }

#if defined(MSDOS)
    /* We only allow one try to use the fat %ds hack.  If we're not
   * using it by now, we'll never turn it on.  This is to avoid
   * the frame buffer moving around and other strangeness.
   */
    try_to_use_fat_ds_vga_hack_p = false;
#endif

    /* Make sure refresh is doing the right thing (its behavior
   * changes depending on whether there's a shadow screen).
   */
    if(vga_current_mode != NULL)
        set_refresh_rate(ROMlib_refresh);

    /* If we need a RAM screen, make sure the memory is there! */
    if(ROMlib_shadow_screen_p)
    {
        if(!vgahost_unmap_linear_fbuf(FBUF_SIZE(width, height, log2_bpp)))
            warning_unexpected("Failed to unmap screen memory!");
    }

    if(bpp == 1 && best->planar_p)
        bpp_diff = 0;
    else
        bpp_diff = (1 << best->log2_bpp) - bpp;

    switch(bpp_diff)
    {
        case 0:
            if(!best->multi_window_p && !vdriver_flip_real_screen_pixels_p)
                screen_update_func = update_packed_no_paging;
            else if(bpp != 1 && best->planar_p)
                screen_update_func = update_4bpp_4bpp_planar;
            else
            {
                if(vdriver_flip_real_screen_pixels_p)
                    screen_update_func = update_packed;
                else
                    screen_update_func = update_packed_no_flip;
            }
            break;
        case 8 - 1:
            screen_update_func = update_1_8;
            break;
        case 8 - 2:
            screen_update_func = update_1_4;
            break;
        case 2 - 1:
        case 8 - 4:
            screen_update_func = update_1_2;
            break;
        case 4 - 1:
            if(best->planar_p)
                gui_fatal("Internal error:  Using 4 bpp planar mode for "
                          "1 bpp graphics");
            else
                screen_update_func = update_1_4;
            break;
        case 4 - 2:
            if(best->planar_p)
                screen_update_func = update_2bpp_4bpp_planar;
            else
                screen_update_func = update_1_2;
            break;
        default:
            gui_fatal("Internal error: can't handle %d bpp on %d hardware bpp.\n",
                      bpp, 1 << best->log2_bpp);
            break;
    }

    /* Note the current mode. */
    vga_current_mode = best;

    if(best->planar_p)
    {
        int unused;

        /* Disable set/reset, so our writes go straight to the screen. */
        VGA_SET_SR_MASK(0);

        /* Make sure we are writing to all 8 pixels in each byte. */
        VGA_SET_BIT_MASK(0xFF);

        /* Guarantee that the rotate count and function are 0. */
        VGA_SET_DATA_ROTATE(0);

        /* Clear the base address by writing 0 to both the start address
       * high and start address low registers.  This code is here
       * because one customer described symptoms consistent with this
       * base address not being set.
       */
        asm volatile("outb %%al,%%dx\n\t"
                     "xorb %%al,%%al\n\t"
                     "incl %%edx\n\t"
                     "outb %%al,%%dx\n\t"
                     "movb %2,%%al\n\t"
                     "decl %%edx\n\t"
                     "outb %%al,%%dx\n\t"
                     "xorb %%al,%%al\n\t"
                     "incl %%edx\n\t"
                     "outb %%al,%%dx"
                     : "=a"(unused), "=d"(unused)
                     : "g"(VGA_START_ADDR_LOW_REG),
                       "0"(VGA_START_ADDR_HIGH_REG), "1"(VGA_CRTC_ADDR_PORT)
                     : "cc");

        if(bpp <= 2)
        {
            ColorSpec planar_color_table[256];
            int white_bits, i;

            if(bpp == 1)
                white_bits = 0xE;
            else
                white_bits = 0xF;

            /* Considering only the low four bits, all colors are black
	   * except one special color is white.
	   */
            memset(planar_color_table, 0, sizeof planar_color_table);
            for(i = white_bits; i < 256; i += 16)
                planar_color_table[i] = ROMlib_white_cspec;

            vgahost_set_colors(0, 256, planar_color_table);
        }

        /* Clear the full screen to black.  This is also important
       * because all unused plane bits must be 1.  That way hardware
       * color 0 is black, making the border color be black even in
       * SVGA planar modes.  In 1bpp mode, we will only use colors 0xE
       * (white) and 0xF (black).  In 2bpp mode, we use colors 0xC
       * (black), 0xD, 0xE, 0xF (white).
       */
        if(bpp == 1)
        {
            VGA_SET_WRITE_PLANE_MASK(0xF);
            fill_hardware_fbuf(~0);
        }
        else if(bpp == 2)
        {
            VGA_SET_WRITE_PLANE_MASK(0xC);
            fill_hardware_fbuf(~0);
            VGA_SET_WRITE_PLANE_MASK(0x3);
            fill_hardware_fbuf(0);
        }
        else /* bpp == 4 */
        {
            VGA_SET_WRITE_PLANE_MASK(0xF);
            fill_hardware_fbuf(0);
        }

        /* Default to only touching plane 0. */
        VGA_SET_WRITE_PLANE(0);
    }
    else /* not planar */
    {
        if(log2_bpp != best->log2_bpp)
        {
            /* Fill in the depth conversion tables appropriately. */
            make_depth_tables(bpp, 1 << best->log2_bpp);
        }
    }

    /* Record the new frame buffer dimensions. */
    vdriver_width = width;
    vdriver_height = height;
    vdriver_log2_bpp = log2_bpp;
    vdriver_bpp = 1 << log2_bpp;
    vdriver_row_bytes = FBUF_SIZE(width, height, log2_bpp) / height;
    vdriver_rgb_spec = NULL;

    /* Compute the byte offset that will center their display as best we can. */
    vga_first_byte_offset = (best->height - height) / 2 * best->row_bytes;
    vga_first_byte_offset += (((best->width - width) << 1)
                              >> (5 - best->log2_bpp));
    vga_first_byte_offset &= ~3;

    vdriver_real_screen_row_bytes = best->row_bytes;
    if(vdriver_real_screen_blit_possible_p)
        vdriver_real_screen_baseaddr += vga_first_byte_offset;

    if(!best->planar_p)
        fill_hardware_fbuf(vdriver_flip_real_screen_pixels_p ? 0 : ~0);

    /* Note that the cursor is invisible right now. */
    cursor_visible_p = false;

    /*#warning "Old interface cruft" */
    host_cursor_depth = 1 << vdriver_log2_bpp;

    /* Clear the internal screen to black, if we'll use it. */
    if(!VDRIVER_BYPASS_INTERNAL_FBUF_P())
        memset(vdriver_fbuf, ~0, vdriver_height * vdriver_row_bytes);

    vdriver_grayscale_p = grayscale_p;

done:

    return success_p;
}

static void
make_depth_tables(int in_bpp, int out_bpp)
{
    uint8 *p, *table;
    unsigned n, in_mask;
    int start_out_shift, out_shift_add;
    int double_p;

    /* Set up output bit shifting appropriately. */
    start_out_shift = 8 - out_bpp;
    out_shift_add = -out_bpp;

    /* Should we double output 16 bit values into 32 bit values? */
    double_p = (out_bpp == in_bpp * 2);

    /* Compute a pointer to the beginning of table memory. */
    table = (uint8 *)&depth_conversion_table;

    /* Loop over all 256 possible input bytes and crank out the
   * resulting bit pattern.
   */
    in_mask = (1 << in_bpp) - 1;
    for(n = 0, p = (uint8 *)table; n < 256; n++)
    {
        int in_shift, out_shift;
        unsigned in;
        uint8 v;

        /* Grab the input value and XOR it. */
        in = n ^ 0xFF;

        for(v = 0, in_shift = 8 - in_bpp, out_shift = start_out_shift;
            in_shift >= 0;
            in_shift -= in_bpp)
        {
            unsigned out_pixel;

            /* Compute the next output pixel. */
            out_pixel = (in >> in_shift) & in_mask;

            v |= out_pixel << out_shift;
            out_shift += out_shift_add;
            if(out_shift & ~7)
            {
                *p++ = v;
                v = 0;
                out_shift = start_out_shift;
            }
        }

        /* If we're doubling 16 bit values, double it and move on.
       * This is a hack where what would normally be a 16 bit output
       * value becomes a 64 bit output value.  The 16 bit value occupies
       * the first and last 16 bits of the 64 bit value, and the middle
       * bits are zero.  This is useful to avoid shifting when assembling
       * 32 bit values for output.
       */
        if(double_p)
        {
            p[0] = p[1] = p[2] = p[3] = 0;
            p[4] = p[-2];
            p[5] = p[-1];
            p += 6;
        }
    }

    /* Make sure we haven't exceeded the size of our lookup table. */
    gui_assert(p <= ((uint8 *)&depth_conversion_table
                     + sizeof depth_conversion_table));
}

/* Saved data used to erase the old cursor. */
static uint8 cursor_restore_data[16 * sizeof(uint32) * 16];
static uint8 *cursor_restore_first_byte;
static int cursor_restore_height, cursor_restore_width;
VGA_IF_SEL(static uint16 cursor_restore_selector;)
static int cursor_restore_row_bytes;

static void
erase_cursor(void)
{
    unsigned char *in, *out;
    int rows_left, out_add;
    VGA_IF_SEL(volatile uint16 saved_es);

    /* Set stuff up for movsb. */
    VGA_IF_SEL(asm volatile("movw %%es,%0\n\t"
                            "movw %1,%%es"
                            : "=m"(saved_es)
                            : "g"(cursor_restore_selector)));

    /* Set up transfer pointers. */
    in = cursor_restore_data;
    out = cursor_restore_first_byte;
    out_add = cursor_restore_row_bytes - cursor_restore_width;

    /* Restore the internal framebuffer. */
    for(rows_left = cursor_restore_height; rows_left > 0; rows_left--)
    {
        int unused;

        asm volatile("cld\n\t"
                     "rep\n\t"
                     "movsb"
                     : "=S"(in), "=D"(out), "=c"(unused)
                     : "0"(in), "1"(out), "2"(cursor_restore_width));
        out += out_add;
    }

    VGA_IF_SEL(asm volatile("movw %0,%%es"
                            :
                            : "m"(saved_es)));

    cursor_restore_height = cursor_restore_width = 0;

    cursor_visible_p = false;
}

/* Interface glue...delete this eventually once the API is cleaned up. */

/* depth required for `cursor_data' by `host_set_cursor ()' */
int host_cursor_depth;

/* Draws the cursor with the upper left hand corner at x, y. */
static void
draw_cursor(int x, int y)
{
    int top, left, bottom, right;
    int old_top, old_left, old_bottom, old_right;
    int height, byte_width;
    char old_virt;

    if(vdriver_fbuf == NULL)
        return;

    old_virt = block_virtual_ints();

    vdriver_accel_wait();

    if(cursor_restore_height > 0)
    {
        dirty_rect_update_screen();
        erase_cursor();
        cursor_visible_p = false;
    }

    /* Compute the bounding rectangle for the cursor. */
    top = y;
    left = x;
    bottom = top + 16;
    right = left + 16;

    old_top = cursor_top;
    old_left = cursor_left;
    old_bottom = cursor_bottom;
    old_right = cursor_right;

    if(draw_cursor_p)
    {
        unsigned cursor_elts_per_row;
        const vga_cursor_xfer_t *cursor_bits;
        unsigned char *p, *save;
        int w, h, x_byte;
        int target_row_bytes;
        VGA_IF_SEL(volatile uint16 saved_fs);

        dirty_rect_update_screen();

        /* Note the cursor bounding rect, aligned to the nearest bytes. */
        cursor_top = top;
        cursor_left = left & (~(7 >> vdriver_log2_bpp));
        cursor_bottom = bottom;
        /* we set up cursor_right below. */

        /* If the old cursor was visible, union in the rectangle it used to
       * occupy.  That way, when we update the screen we'll automatically
       * erase it.  Since the cursor won't often move very much, doing this
       * union will usually be faster than doing a separate cursor erase
       * and redraw.
       */
        if(cursor_visible_p)
        {
            if(old_left < left)
                left = old_left;
            if(old_right > right)
                right = old_right;
            if(old_top < top)
                top = old_top;
            if(old_bottom > bottom)
                bottom = old_bottom;
        }

        /* Fetch information about how to interpret the cursor. */
        switch(vdriver_log2_bpp)
        {
            case 0:
                cursor_bits = &cursor_masks.bpp1[x & 7][0][0];
                cursor_elts_per_row = 3;
                break;
            case 1:
                cursor_bits = &cursor_masks.bpp2[x & 3][0][0];
                cursor_elts_per_row = 5;
                break;
            case 2:
                cursor_bits = &cursor_masks.bpp4[x & 1][0][0];
                cursor_elts_per_row = 9;
                break;
            case 3:
                cursor_bits = &cursor_masks.bpp8[0][0];
                cursor_elts_per_row = 16;
                break;
            default:
                abort();
        }

        /* Compute cursor_right appropriately. */
        cursor_right = (cursor_left + ((cursor_elts_per_row << 3)
                                       >> vdriver_log2_bpp));

        /* Pin the cursor within the frame buffer. */
        height = 16;
        if(y < 0)
        {
            height -= -y;
            cursor_bits += (cursor_elts_per_row * -y);
            y = 0;
        }
        if(y + height > vdriver_height)
            height -= (y + height) - vdriver_height;

        byte_width = cursor_elts_per_row;
        x_byte = (x << 2) >> (5 - vdriver_log2_bpp);
        if(x_byte < 0)
        {
            byte_width -= -x_byte;
            cursor_bits += -x_byte;
            x_byte = 0;
        }
        else if(x_byte + byte_width > vdriver_row_bytes)
            byte_width -= (x_byte + byte_width) - vdriver_row_bytes;

        /* Actually draw the cursor. */
        save = cursor_restore_data;
        if(VDRIVER_BYPASS_INTERNAL_FBUF_P())
        {
            target_row_bytes = vdriver_real_screen_row_bytes;
            p = vdriver_real_screen_baseaddr;
        }
        else
        {
            target_row_bytes = vdriver_row_bytes;
            p = vdriver_fbuf;
        }

        cursor_restore_row_bytes = target_row_bytes;
        p += y * target_row_bytes + x_byte;
        cursor_restore_first_byte = p;

#if defined(VGA_SCREEN_NEEDS_FAR_PTR)
        asm("movw %%fs,%0"
            : "=m"(saved_fs));
        if(VDRIVER_BYPASS_INTERNAL_FBUF_P())
            asm volatile("movw %0,%%fs"
                         :
                         : "g"(vga_screen_selector));
        else
            asm("pushl %ds ; popl %fs");
        asm("movw %%fs,%0"
            : "=g"(cursor_restore_selector));
#endif

        for(h = height; h > 0; h--)
        {
            for(w = 0; w < byte_width; w++)
            {
                unsigned char t;

                asm("movb " VGA_IF_SEL("%%fs:") "(%1,%2),%0"
                    : "=abcd"(t)
                    : "r"(p), "r"(w));
                save[w] = t;
                t &= cursor_bits[w].and_mask;
                t ^= cursor_bits[w].xor_mask;
                asm volatile("movb %0," VGA_IF_SEL("%%fs:") "(%1,%2)"
                             :
                             : "abcd"(t), "r"(p), "r"(w));
            }
            save += byte_width;
            p += vdriver_row_bytes;
            cursor_bits += cursor_elts_per_row;
        }

        VGA_IF_SEL(asm("movw %0,%%fs"
                       :
                       : "m"(saved_fs)));
    }
    else
    {
        cursor_restore_first_byte = 0;
        byte_width = height = 0;
    }

    /* Note information so we can restore the cursor. */
    cursor_restore_height = height;
    cursor_restore_width = byte_width;

    /* Update the frame buffer to reflect the cursor's new status. */
    if(!VDRIVER_BYPASS_INTERNAL_FBUF_P())
    {
        dirty_rect_update_screen();

        if(!vdriver_update_screen(top, left, bottom, right, true))
        {
            /* If the update machinery is already busy, restore the old
	   * information about the cursor's current location.
	   */
            cursor_top = old_top;
            cursor_left = old_left;
            cursor_bottom = old_bottom;
            cursor_right = old_right;
        }

        /* Erase the cursor from the offscreen frame buffer, but leave it
       * visible on the real screen.
       */
        if(draw_cursor_p && cursor_restore_height)
            erase_cursor();
    }

    cursor_visible_p = draw_cursor_p;

    restore_virtual_ints(old_virt);
}

int host_set_cursor_visible(int show_p)
{
    int old_draw_p;

    old_draw_p = draw_cursor_p;

    BlockVirtualInterruptsGuard guard;

    if(show_p)
    {
        if(!draw_cursor_p)
        {
            draw_cursor_p = true;
            if(!cursor_visible_p)
                draw_cursor(cursor_x, cursor_y);
        }
    }
    else /* Hide it. */
    {
        if(draw_cursor_p)
        {
            draw_cursor_p = false;
            if(cursor_visible_p)
            {
                vdriver_accel_wait();

                if(!VDRIVER_BYPASS_INTERNAL_FBUF_P())
                {
                    if(vdriver_update_screen(cursor_top, cursor_left,
                                             cursor_bottom, cursor_right,
                                             true))
                    {
                        cursor_visible_p = false;
                        cursor_top = cursor_bottom = 0;
                    }
                }
                else
                {
                    erase_cursor();
                    cursor_visible_p = false;
                    cursor_top = cursor_bottom = 0;
                }
            }
        }
    }

    return old_draw_p;
}

bool host_hide_cursor_if_intersects(int top, int left, int bottom, int right)
{
    if(top < cursor_bottom && bottom > cursor_top
       && left < cursor_right && right > cursor_left)
        return host_set_cursor_visible(false);
    else
        return draw_cursor_p;
}

void host_set_cursor(char *cursor_data,
                     unsigned short cursor_mask[16],
                     int hotspot_x, int hotspot_y)
{
    int x, y, row_bytes;
    int and, xor;
    unsigned char *mask_baseaddr, *data_baseaddr;
    int current_hot_x, current_hot_y;

    current_hot_x = cursor_x + cursor_hotspot_x;
    current_hot_y = cursor_y + cursor_hotspot_y;

    cursor_hotspot_x = hotspot_x;
    cursor_hotspot_y = hotspot_y;

    row_bytes = 2 << vdriver_log2_bpp; /* == 16 * bpp / 8 */

    data_baseaddr = cursor_data;

    if(vdriver_log2_bpp != 0)
    {
        PixMap mask_pixmap, target_pixmap;

        mask_pixmap.baseAddr = RM((Ptr)&cursor_mask[0]);
        mask_pixmap.rowBytes = CWC(2);
        mask_pixmap.bounds = ROMlib_cursor_rect;
        mask_pixmap.cmpCount = CWC(1);
        mask_pixmap.pixelType = CWC(0);
        mask_pixmap.pixelSize = mask_pixmap.cmpSize = CWC(1);
        mask_pixmap.pmTable = RM(validate_relative_bw_ctab());

        mask_baseaddr = alloca(16 * row_bytes);

        target_pixmap.baseAddr = RM(mask_baseaddr);
        target_pixmap.rowBytes = CW(row_bytes);
        target_pixmap.bounds = ROMlib_cursor_rect;
        target_pixmap.cmpCount = CWC(1);
        target_pixmap.pixelType = CWC(0);
        target_pixmap.pixelSize = target_pixmap.cmpSize
            = CW(1 << vdriver_log2_bpp);
        /* The target pixmap colortable is not used by `convert_pixmap ()'
       * target_pixmap.pmTable = ...;
       */

        convert_pixmap(&mask_pixmap, &target_pixmap, &ROMlib_cursor_rect, NULL);
    }
    else
    {
        mask_baseaddr = (unsigned char *)cursor_mask;
    }

    switch(vdriver_log2_bpp)
    {
        case 0:
        {
            const uint16 *d, *m;
            int xmod;

            d = (const uint16 *)data_baseaddr;
            m = (const uint16 *)mask_baseaddr;

            for(xmod = 0; xmod < 8; xmod++)
                for(y = 0; y < 16; y++)
                {
                    and = ~(CW(m[y]) << (16 - xmod));
                    cursor_masks.bpp1[xmod][y][0].and_mask = and >> 24;
                    cursor_masks.bpp1[xmod][y][1].and_mask = and >> 16;
                    cursor_masks.bpp1[xmod][y][2].and_mask = and >> 8;

                    xor = CW(d[y]) << (16 - xmod);
                    cursor_masks.bpp1[xmod][y][0].xor_mask = xor >> 24;
                    cursor_masks.bpp1[xmod][y][1].xor_mask = xor >> 16;
                    cursor_masks.bpp1[xmod][y][2].xor_mask = xor >> 8;
                }
            break;
        }
        case 1:
        {
            const ULONGINT *d, *m;
            int xmod;

            d = (const ULONGINT *)data_baseaddr;
            m = (const ULONGINT *)mask_baseaddr;

            for(xmod = 0; xmod < 4; xmod++)
                for(y = 0; y < 16; y++)
                {
                    and = ~(CL(m[y]) >> (xmod * 2));
                    cursor_masks.bpp2[xmod][y][0].and_mask = and >> 24;
                    cursor_masks.bpp2[xmod][y][1].and_mask = and >> 16;
                    cursor_masks.bpp2[xmod][y][2].and_mask = and >> 8;
                    cursor_masks.bpp2[xmod][y][3].and_mask = and;
                    cursor_masks.bpp2[xmod][y][4].and_mask = ~(CL(m[y]) << (8 - (xmod * 2)));

                    xor = CL(d[y]) >> (xmod * 2);
                    cursor_masks.bpp2[xmod][y][0].xor_mask = xor >> 24;
                    cursor_masks.bpp2[xmod][y][1].xor_mask = xor >> 16;
                    cursor_masks.bpp2[xmod][y][2].xor_mask = xor >> 8;
                    cursor_masks.bpp2[xmod][y][3].xor_mask = xor;
                    cursor_masks.bpp2[xmod][y][4].xor_mask = CL(d[y]) << (8 - (xmod * 2));
                }
            break;
        }
        case 2:
        {
            const unsigned char *d, *m;
            int xmod;

            d = (const unsigned char *)data_baseaddr;
            m = (const unsigned char *)mask_baseaddr;

            for(xmod = 0; xmod < 2; xmod++)
                for(y = 0; y < 16; y++)
                {
                    const unsigned char *d_base = &d[y * row_bytes];
                    const unsigned char *m_base = &m[y * row_bytes];

                    for(x = 0; x < 9; x++)
                    {
                        unsigned char d0, d1, m0, m1;

                        d0 = (x > 0) ? d_base[x - 1] : 0;
                        d1 = (x < 8) ? d_base[x] : 0;
                        m0 = (x > 0) ? m_base[x - 1] : 0;
                        m1 = (x < 8) ? m_base[x] : 0;
                        cursor_masks.bpp4[xmod][y][x].and_mask = ~((m0 << (8 - (xmod * 4))) | (m1 >> (xmod * 4)));
                        cursor_masks.bpp4[xmod][y][x].xor_mask = ((d0 << (8 - (xmod * 4))) | (d1 >> (xmod * 4)));
                    }
                }
        }
        break;

#define CURSOR_SETUP(size, which)                                  \
    {                                                              \
        const unsigned char *base_data, *base_mask;                \
        int x;                                                     \
                                                                   \
        base_data = (const unsigned char *)data_baseaddr;          \
        base_mask = (const unsigned char *)mask_baseaddr;          \
                                                                   \
        for(y = 0; y < 16; y++)                                    \
        {                                                          \
            for(x = size - 1; x >= 0; x--)                         \
            {                                                      \
                cursor_masks.which[y][x].and_mask = ~base_mask[x]; \
                cursor_masks.which[y][x].xor_mask = base_data[x];  \
            }                                                      \
                                                                   \
            base_data += row_bytes;                                \
            base_mask += row_bytes;                                \
        }                                                          \
    }

        case 3:
            CURSOR_SETUP(16, bpp8);
            break;

#undef CURSOR_SETUP
        default:
            gui_fatal("bad cursor depth `%d'", 1 << vdriver_log2_bpp);
    }

    /* Hide and redraw the new cursor if it used to be visible. */
    if(draw_cursor_p)
    {
        host_set_cursor_visible(false);
        cursor_x = current_hot_x - cursor_hotspot_x;
        cursor_y = current_hot_y - cursor_hotspot_y;
        host_set_cursor_visible(true);
    }
}

/* Erases the old cursor and draws a new cursor, if appropriate. */
void vga_update_cursor(int hot_x, int hot_y)
{
    int x, y;

    x = hot_x - cursor_hotspot_x;
    y = hot_y - cursor_hotspot_y;

    /* If nothing has changed, don't bother redrawing. */
    if(x != cursor_x || y != cursor_y)
    {
        cursor_x = x;
        cursor_y = y;
        draw_cursor(x, y);
    }
}

void host_flush_shadow_screen(void)
{
    if(!ROMlib_shadow_screen_p)
        return;

    if(screen_update_func == update_packed_no_paging
       || screen_update_func == update_packed_no_flip
       || screen_update_func == update_packed)
    {
        /* If the shadow screen and the real screen have the same depth,
       * it's faster just to copy from one to the other than it is
       * to check to see what's changed and then copy.
       */
        vdriver_update_screen(0, 0, vdriver_height, vdriver_width, false);
    }
    else
    {
        /* For other modes, flushing the whole screen all the time is
       * too expensive.  Instead, let's see what parts of the screen
       * have changed (if any).
       */
        int top, left_long, bottom, right_long;
        static uint32 *last_refreshed_screen = NULL;

        /* Lazily allocate the screen that keeps track of the last image
       * we refreshed.
       */
        if(last_refreshed_screen == NULL)
        {
#if defined(SBRK_PERMANENT_MEMORY)
            last_refreshed_screen = (uint32 *)sbrk((fbuf_size + 31) & ~31);
            if(last_refreshed_screen == (uint32 *)-1)
                last_refreshed_screen = NULL;
#else
            last_refreshed_screen = (uint32 *)malloc(fbuf_size);
#endif
            vdriver_update_screen(0, 0, vdriver_height, vdriver_width, false);
            if(last_refreshed_screen != NULL)
                memcpy(last_refreshed_screen, vdriver_fbuf,
                       vdriver_row_bytes * vdriver_height);
        }
        else if(find_changed_rect_and_update_shadow((const uint32 *)vdriver_fbuf, last_refreshed_screen,
                                                    vdriver_row_bytes / sizeof(uint32), vdriver_height,
                                                    &top, &left_long, &bottom, &right_long))
        {
            vdriver_update_screen(top, left_long << (5 - vdriver_log2_bpp),
                                  bottom, right_long << (5 - vdriver_log2_bpp),
                                  false);
        }
    }
}

#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
void vdriver_set_up_internal_screen(void)
{
    if(vdriver_real_screen_blit_possible_p && ROMlib_shadow_screen_p)
    {
        uint32 len;
        int unused;

        len = vdriver_row_bytes * vdriver_height / sizeof(uint32);
        asm volatile(VGA_IF_SEL("pushl %%ds ; movw %3,%%ds\n\t") "cld\n\t"
                                                                 "rep\n\t"
                                                                 "movsl" VGA_IF_SEL("\n\tpopl %%ds")
                     : "=c"(unused), "=S"(unused), "=D"(unused)
                     : "g"(VGA_SELECTOR), "1"(vdriver_real_screen_baseaddr),
                       "2"(vdriver_fbuf), "0"(len));

        note_executor_changed_screen(0, vdriver_height);
    }
}
#endif /* VDRIVER_SUPPORTS_REAL_SCREEN_BLITS */

#endif /* USE_VGAVDRIVER */
