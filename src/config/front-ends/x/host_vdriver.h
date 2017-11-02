#define vdriver_system_busy 0

typedef VDRIVER_MODE_LIST_TYPE (2) vdriver_x_mode_t;

extern vdriver_x_mode_t vdriver_x_modes;

#define vdriver_fixed_clut_p false

#define vdriver_mode_list \
  ((const vdriver_modes_t *) &vdriver_x_modes)

#define VDRIVER_DISPLAYED_IN_WINDOW

#define vdriver_accel_rect_scroll(t, l, b, r, dx, dy) VDRIVER_ACCEL_NO_UPDATE
#define vdriver_accel_wait()

#define VDRIVER_BYPASS_INTERNAL_FBUF_P() false
