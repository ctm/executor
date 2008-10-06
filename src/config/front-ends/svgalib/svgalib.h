#if !defined (_SVGALIB_H_)
#define _SVGALIB_H_

#define EVENT_SVGALIB

#include <vga.h>

extern boolean_t event_init (int max_mouse_x, int max_mouse_y);
extern void event_shutdown (void);

#endif /* !_SVGALIB_H_ */
