#include "rsys/prefs.h"
#include "rsys/flags.h"

#define vdriver_flush_display()
#define vdriver_system_busy 0
#define VDRIVER_DIRTY_RECT_BYTE_ALIGNMENT 4
#define VDRIVER_SORT_DIRTY_RECTS_BY_TOP /* less VESA window thrashing */

#define VDRIVER_SVGALIB

extern bool svgalib_have_blitwait_p;
#define vdriver_accel_wait()        \
    do                              \
    {                               \
        if(svgalib_have_blitwait_p) \
            vga_blitwait();         \
    } while(0)

#define vdriver_fixed_clut_p false

/* We cannot yet unmap the screen (well, we can't remap it, and
 * we don't want to unmap it out from svgalib).
 */
#define vgahost_unmap_linear_fbuf(num_bytes) false

#define VGA_WINDOW_GRANULARITY(mode) 65536U
#define VGA_WINDOW_SIZE(mode) 65536U

#define USE_VGAVDRIVER

#define VGA_SELECTOR 0 /* Doesn't end up getting used for real. */

#define VDRIVER_SUPPORTS_REAL_SCREEN_BLITS

#define VDRIVER_BYPASS_INTERNAL_FBUF_P()                      \
    ((vdriver_real_screen_blit_possible_p && !ROMlib_refresh) \
     || !ROMlib_shadow_screen_p)
