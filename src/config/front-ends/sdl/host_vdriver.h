
/* SDL is after all, a semi-windowing environment */
#define VDRIVER_DISPLAYED_IN_WINDOW
#define vdriver_fixed_clut_p false
#define vdriver_grayscale_p false

/* What's this for again?  We may need this later... */
#define vdriver_rgb_spec NULL

/* We don't have a function to notify the user that the system is busy */
#define vdriver_system_busy 0

/* We don't provide any accelerated display functions under SDL (yet) */
#define VDRIVER_BYPASS_INTERNAL_FBUF_P() false
#define vdriver_accel_rect_fill(t, l, b, r, c) VDRIVER_ACCEL_NO_UPDATE
#define vdriver_accel_rect_scroll(t, l, b, r, dx, dy) VDRIVER_ACCEL_NO_UPDATE
#define vdriver_accel_wait()
