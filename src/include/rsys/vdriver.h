
#if !defined(_VDRIVER_H_)
#define _VDRIVER_H_

namespace Executor
{
struct ColorSpec;
}

#include "rsys/rgbutil.h"
#include "host_bltmacros.h"

namespace Executor
{

/* Minimum screen size we'll allow. */
#define VDRIVER_MIN_SCREEN_WIDTH 512
#define VDRIVER_MIN_SCREEN_HEIGHT 342

/* Suggested default screen size (may be safely ignored). */
#define VDRIVER_DEFAULT_SCREEN_WIDTH 640
#define VDRIVER_DEFAULT_SCREEN_HEIGHT 480

/* This is a macro to facilitate static declarations for hosts
 * where the number of modes is fixed. */
#define VDRIVER_MODE_LIST_TYPE(num_entries)                    \
    struct                                                     \
    {                                                          \
        int continuous_range_p; /* 2 entry inclusive range? */ \
        int num_sizes; /* Length of size[] array.  */          \
        struct                                                 \
        {                                                      \
            short width, height;                               \
        } size[num_entries];                                   \
    }

typedef VDRIVER_MODE_LIST_TYPE(0) vdriver_modes_t;

#define VDRIVER_MODE_LIST_SIZE(nelt) \
    (sizeof(vdriver_modes_t)         \
     + ((nelt) * sizeof(((vdriver_modes_t *)0)->size[0])))

typedef struct
{
    int top, left, bottom, right;
} vdriver_rect_t;

typedef enum {
    VDRIVER_ACCEL_NO_UPDATE,
    VDRIVER_ACCEL_FULL_UPDATE,
    VDRIVER_ACCEL_HOST_SCREEN_UPDATE_ONLY
} vdriver_accel_result_t;
}

/* host_vdriver.h can override some of the following with macros. */
#include "host_vdriver.h"

namespace Executor
{
#if !defined(vdriver_init)
extern bool vdriver_init(int max_width, int max_height, int max_bpp,
                         bool fixed_p, int *argc, char *argv[]);
#endif

#if !defined(vdriver_shutdown)
extern void vdriver_shutdown(void);
#endif

#if !defined(vdriver_update_screen)
extern int vdriver_update_screen(int top, int left, int bottom, int right,
                                 bool cursor_p);
#endif

#if !defined(vdriver_update_screen_rects)
extern int vdriver_update_screen_rects(int num_rects, const vdriver_rect_t *r,
                                       bool cursor_p);
#endif

#if !defined(vdriver_acceptable_mode_p)
extern bool vdriver_acceptable_mode_p(int width, int height, int bpp,
                                      bool grayscale_p,
                                      bool exact_match_p);
#endif

#if !defined(vdriver_set_colors)
extern void vdriver_set_colors(int first_color, int num_colors,
                               const struct ColorSpec *color_array);
#endif

#if !defined(vdriver_get_colors)
extern void vdriver_get_colors(int first_color, int num_colors,
                               struct ColorSpec *color_array);
#endif

#if !defined(vdriver_set_mode)
extern bool vdriver_set_mode(int width, int height, int bpp,
                             bool grayscale_p);
#endif

#if !defined(vdriver_flush_display)
extern void vdriver_flush_display(void);
#endif

#if !defined(vdriver_system_busy)
extern void vdriver_system_busy(bool busy_p);
#endif

#if !defined(vdriver_opt_register)
extern void vdriver_opt_register(void);
#endif

#if !defined(vdriver_accel_rect_fill)
extern vdriver_accel_result_t vdriver_accel_rect_fill(int top, int left,
                                                      int bottom, int right,
                                                      uint32 color);
#endif

#if !defined(vdriver_accel_rect_scroll)
extern vdriver_accel_result_t vdriver_accel_rect_scroll(int top, int left,
                                                        int bottom, int right,
                                                        int dx, int dy);
#endif

#if !defined(vdriver_accel_wait)
extern void vdriver_accel_wait(void);
#endif

#if !defined(vdriver_fbuf)
extern uint8 *vdriver_fbuf;
#endif

#if !defined(vdriver_row_bytes)
extern int vdriver_row_bytes;
#endif

#if !defined(vdriver_width)
extern int vdriver_width;
#endif

#if !defined(vdriver_height)
extern int vdriver_height;
#endif

#if !defined(vdriver_bpp)
extern int vdriver_bpp;
#endif

#if !defined(vdriver_log2_bpp)
extern int vdriver_log2_bpp;
#endif

#if !defined(vdriver_max_bpp)
extern int vdriver_max_bpp;
#endif

#if !defined(vdriver_log2_max_bpp)
extern int vdriver_log2_max_bpp;
#endif

#if !defined(vdriver_mode_list)
extern vdriver_modes_t *vdriver_mode_list;
#endif

#if !defined(vdriver_rgb_spec)
extern rgb_spec_t *vdriver_rgb_spec;
#endif

#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)

#if !defined(vdriver_real_screen_blit_possible_p)
extern bool vdriver_real_screen_blit_possible_p;
#endif

#if !defined(vdriver_flip_real_screen_pixels_p)
extern bool vdriver_flip_real_screen_pixels_p;
#endif

#if !defined(vdriver_real_screen_row_bytes)
extern int vdriver_real_screen_row_bytes;
#endif

#if !defined(vdriver_real_screen_baseaddr)
extern uint8 *vdriver_real_screen_baseaddr;
#endif

#if !defined(vdriver_set_up_internal_screen)
extern void vdriver_set_up_internal_screen(void);
#endif

#endif /* VDRIVER_SUPPORTS_REAL_SCREEN_BLITS */

#if !defined(vdriver_grayscale_p)
extern bool vdriver_grayscale_p;
#endif

#if !defined(vdriver_fixed_clut_p)
extern bool vdriver_fixed_clut_p;
#endif

#ifdef VDRIVER_PUMP_EVENTS
extern void vdriver_pump_events();
#endif
}
#endif /* !_VDRIVER_H_ */
