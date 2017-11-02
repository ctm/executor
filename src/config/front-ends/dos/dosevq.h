#if !defined (_DOSEVQ_H_)
#define _DOSEVQ_H_

#include "dosevq_defs.h"
#include <sys/farptr.h>

extern bool need_hacky_screen_update;

/* This struct must be 32 bits, or else you'll have to change some
 * accessor macros and assembly code.
 */
typedef struct
{
  uint8 type PACKED;		/* The type of event.                   */
  uint8 which PACKED;		/* Keycode, scancode, or button number. */
  uint16 keyflags PACKED;	/* Flags for modifier keys (shift, etc. */
} dosevq_record_t PACKED;

typedef uint8 dosevq_type_t;

#define DOSEVQ_QUEUE_TYPE(nelts)					     \
struct									     \
{									     \
  uint16 qhead, qtail;		/* Head and tail indices.           */	     \
  int16 mouse_dx, mouse_dy;	/* Delta mouse position since last query */  \
  uint16 mouse_interrupt_pending_mask;	/* mask for different interrupts. */ \
  uint16 padding;		/* to keep queue long-aligned. */	     \
  dosevq_record_t queue[nelts]; /* qsize elements in this array. */	     \
}

typedef DOSEVQ_QUEUE_TYPE (DOSEVQ_QUEUE_SIZE) dosevq_queue_t;

#define DOSEVQ_MOUSE_MOVED_PENDING_MASK 0x00FF
#define DOSEVQ_EVENT_PENDING_MASK	0xFF00

extern const char *dosevq_init (void);
extern const char *dosevq_reinit_mouse (void);
extern void dosevq_shutdown (void);
extern dosevq_type_t dosevq_dequeue (dosevq_record_t *e);
extern void dosevq_note_mouse_interrupt (void);

/* Selector for the low-level DOS event queue. */
extern uint16 dosevq_queue_sel;

extern uint8 dos_mouse_stub[];
extern unsigned dos_mouse_stub_bytes_to_copy;
extern uint16 dos_mouse_stub_segment;
extern uint8 _dos_event_queue_start;

/* Offset in bytes from the beginning of our DOS buffer to the event queue. */
#define DOS_EVENT_QUEUE_OFFSET (&_dos_event_queue_start - &dos_mouse_stub[0])

/* Accessor macros for queue elements. */
extern uint16 _get_queue_mem16 (unsigned long offset);
extern void _set_queue_mem16 (unsigned long offset, uint16 val);

#define _DOSEVQ_GET_FIELD(field)				\
  ((typeof (((dosevq_queue_t *)0)->field))			\
   _get_queue_mem16 (DOS_EVENT_QUEUE_OFFSET			\
		     + offsetof (dosevq_queue_t, field)))
#define _DOSEVQ_SET_FIELD(field, val)				\
   _set_queue_mem16 ((DOS_EVENT_QUEUE_OFFSET			\
		      + offsetof (dosevq_queue_t, field)),	\
		     (val))

#define DOSEVQ_QHEAD()	  _DOSEVQ_GET_FIELD (qhead)
#define DOSEVQ_QTAIL()	  _DOSEVQ_GET_FIELD (qtail)
#define DOSEVQ_MOUSE_DX() _DOSEVQ_GET_FIELD (mouse_dx)
#define DOSEVQ_MOUSE_DY() _DOSEVQ_GET_FIELD (mouse_dy)
#define DOSEVQ_INTERRUPT_PENDING_MASK() \
	_DOSEVQ_GET_FIELD (mouse_interrupt_pending_mask)

extern uint32 dosevq_qelt (int which_elt);
#define DOSEVQ_QELT(elt) dosevq_qelt (elt)

#define DOSEVQ_SET_QHEAD(val)	 _DOSEVQ_SET_FIELD (qhead, val)
#define DOSEVQ_SET_QTAIL(val)	 _DOSEVQ_SET_FIELD (qtail, val)
#define DOSEVQ_SET_MOUSE_DX(val) _DOSEVQ_SET_FIELD (mouse_dx, val)
#define DOSEVQ_SET_MOUSE_DY(val) _DOSEVQ_SET_FIELD (mouse_dy, val)
#define DOSEVQ_SET_INTERRUPT_PENDING_MASK(val) \
	_DOSEVQ_SET_FIELD (mouse_interrupt_pending_mask, val)

extern void dosevq_set_qelt (int which_elt, uint32 e);
#define DOSEVQ_SET_QELT(elt, val) dosevq_set_qelt ((elt), (val))

#endif /* !_DOSEVQ_H_ */
