#if !defined (__Exec_HOST__)
#define __Exec_HOST__

#include "CQuickDraw.h"
namespace Executor {
extern int host_cursor_depth;

extern void host_set_cursor (char *cursor_data,
			     unsigned short cursor_mask[16],
			     int hotspot_x, int hotspot_y);
extern int host_set_cursor_visible (int show_p);
extern boolean_t host_hide_cursor_if_intersects (int top, int left,
						 int bottom, int right);
extern void host_beep_at_user (void);
extern void host_flush_shadow_screen (void);
}
#endif /* !__HOST__ */
