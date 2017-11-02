#include "rsys/prefs.h"
#include "rsys/flags.h"

#define vdriver_flush_display()
#define vdriver_system_busy 0
#define VDRIVER_DIRTY_RECT_BYTE_ALIGNMENT 4
#define VDRIVER_SORT_DIRTY_RECTS_BY_TOP  /* less VESA window thrashing */

#define vdriver_accel_rect_fill(t, l, b, r, c) VDRIVER_ACCEL_NO_UPDATE
#define vdriver_accel_rect_scroll(t, l, b, r, dx, dy) VDRIVER_ACCEL_NO_UPDATE
#define vdriver_accel_wait()

#define USE_VGAVDRIVER
#define VGAHOST_VGA_MODE_EXTENSIONS	\
unsigned win_granularity, win_size;	\
uint32 phys_base_addr;			\
uint16 screen_selector;

#define VGA_WINDOW_GRANULARITY(mode)	((mode)->win_granularity)
#define VGA_WINDOW_SIZE(mode)		((mode)->win_size)

extern uint16 vga_screen_selector;
extern uint16 vga_window_selector;
#define VGA_SELECTOR vga_screen_selector

#define vdriver_fixed_clut_p false

extern bool try_to_use_fat_ds_vga_hack_p;

#define VDRIVER_SUPPORTS_REAL_SCREEN_BLITS

#define VDRIVER_BYPASS_INTERNAL_FBUF_P()			\
((vdriver_real_screen_blit_possible_p && !ROMlib_refresh)	\
 || !ROMlib_shadow_screen_p)
