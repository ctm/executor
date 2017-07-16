
#ifndef __NEXT_HOST_VDRIVER_H_
#define __NEXT_HOST_VDRIVER_H_
#include <string>

#define vdriver_system_busy 0
#define VDRIVER_DISPLAYED_IN_WINDOW

#define vdriver_accel_rect_fill(t, l, b, r, c) VDRIVER_ACCEL_NO_UPDATE
#define vdriver_accel_rect_scroll(t, l, b, r, dx, dy) VDRIVER_ACCEL_NO_UPDATE
#define vdriver_accel_wait()

namespace Executor {
	extern std::string SystemDiskLocation();
}

#define VDRIVER_DISPLAYED_IN_WINDOW

#define VDRIVER_BYPASS_INTERNAL_FBUF_P() FALSE

typedef VDRIVER_MODE_LIST_TYPE (2) vdriver_nextstep_mode_t;

#define VDRIVER_DIRTY_RECT_BYTE_ALIGNMENT 4   /* Align to long boundaries. */

extern "C" vdriver_nextstep_mode_t vdriver_nextstep_modes;

#define vdriver_mode_list \
  ((const vdriver_modes_t *) &vdriver_nextstep_modes)

#endif
