#if !defined (_DOSEVQ_DEFS_H_)
#define _DOSEVQ_DEFS_H_

/* This file is #included from assembly code, so it can contain
 * only preprocessor stuff.
 */


/* Internal event queue types. */
#define EVTYPE_NONE		0
#define EVTYPE_RAWKEY		1
#define EVTYPE_MOUSE_DOWN	2
#define EVTYPE_MOUSE_UP		3

#define EVMASK_NONE	   0
#define EVMASK_RAWKEY      (1 << EVTYPE_RAWKEY)
#define EVMASK_MOUSE_DOWN  (1 << EVTYPE_MOUSE_DOWN)
#define EVMASK_MOUSE_UP    (1 << EVTYPE_MOUSE_UP)

/* Number of entries in our event queue.  Must be a power of 2. */
#define DOSEVQ_QUEUE_SIZE 128

/* These flags are passed in %ax by the mouse driver when something happens. */
#define MOUSE_MOTION_MASK	  0x1
#define MOUSE_LEFT_PRESSED_MASK	  0x2
#define MOUSE_LEFT_RELEASED_MASK  0x4

#endif /* _DOSEVQ_DEFS_H_ */
