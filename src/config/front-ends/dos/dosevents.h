#if !defined (_dosevents_h_)
#define _dosevents_h_

#if defined (MSDOS)

#include <syn68k_public.h>

extern void init_dos_events (int max_mouse_x, int max_mouse_y);
extern void querypointerX (LONGINT *xp, LONGINT *yp, LONGINT *notused);
extern void update_vga_cursor (int x, int y);

/* Cursor hotspot. */
extern int cursor_hot_x, cursor_hot_y;

#endif /* MSDOS */

#endif /* Not _dosevents_h_ */
