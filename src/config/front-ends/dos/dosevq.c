/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_dosevq[] = "$Id: dosevq.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "dosevq.h"
#include "rsys/m68kint.h"
#include <go32.h>
#include <dpmi.h>
#include <sys/farptr.h>
#include "rsys/blockinterrupts.h"
#include "dpmilock.h"
#include "dosmem.h"
#include "rsys/flags.h"
#include "dpmicall.h"


/* These are symbols found in deintr.s.  We declare them here to
 * trick the C compiler.
 */
extern char dosevq_asm_begin, dosevq_asm_end;
extern char dosevq_kbd_int;
extern void dosevq_handle_mouse_callback (void);

/* These hold the registers for the mouse callback. */
static __dpmi_regs mouse_callback_regs;

/* Vector for the keyboard interrupt. */
#define KBD_VEC 0x09

/* Old keyboard interrupt vector.  Can't be static since deintr.s
 * references this.
 */
__dpmi_paddr old_kbd_handler;

/* New keyboard interrupt vector. */
__dpmi_paddr new_kbd_handler;

/* Real mode address of mouse callback. */
__dpmi_raddr mouse_callback_addr;

/* Selector for DOS memory for mouse stub and event queue. */
uint16 mouse_handler_and_queue_sel;

/* These flags tell us whether we need to shut these down when we exit. */
static char keyboard_initialized = 0;
static char mouse_initialized = 0;

/* Global flag; use DPMI real mode callback for mouse driver? */
boolean_t use_mouse_rmcb_p = TRUE;


/* This dummy function is used to mark the beginning of memory to
 * "wire down" so it won't get paged out.  This is admittedly a hack,
 * but there's no nice way to do it.
 */
void dosevq_lock_functions_begin (void) { }


/* We use this instead of _farpeekw so we can guarantee that it
 * is in locked down memory even when we're not inlining.
 */
inline uint16
_get_queue_mem16 (unsigned long offset)
{
  uint16 result;
  asm volatile (QUOTED_DATA16 "movw _mouse_handler_and_queue_sel,%%fs\n\t"
		".byte 0x64\n\t"
		"movw (%k1),%w0"
		: "=r" (result)
		: "r" (offset));
  return result;
}

/* We use this instead of _farpokew so we can guarantee that it
 * is in locked down memory even when we're not inlining.
 */
inline void
_set_queue_mem16 (unsigned long offset, uint16 val)
{
  asm volatile (QUOTED_DATA16 "movw _mouse_handler_and_queue_sel,%%fs\n\t"
		".byte 0x64\n\t"
		"movw %w0,(%k1)"
		: : "ri" (val), "r" (offset));
}



/* This private routine enqueues an event.  Since it is always called
 * at interrupt time, there is no need to block interrupts.
 */
void
dosevq_enqueue (dosevq_type_t type, unsigned char which,
		unsigned short keyflags)
{
  dosevq_record_t e;
  unsigned qhead, next_qhead;

  /* Disallow enqueueing when there is no queue. */
  if (mouse_handler_and_queue_sel == 0)
    return;

  /* See if the queue is full.  If so, fail. */
  qhead = DOSEVQ_QHEAD ();
  next_qhead = (qhead + 1) % DOSEVQ_QUEUE_SIZE;
  if (next_qhead == DOSEVQ_QTAIL ())
    return;

  /* Actually enqueue the element. */
  e.type     = type;
  e.which    = which;
  e.keyflags = keyflags;
  DOSEVQ_SET_QELT (qhead, *(uint32 *) &e);

  /* Notify Executor that an event has come in. */
  cpu_state.interrupt_pending[M68K_EVENT_PRIORITY] = 1;
  cpu_state.interrupt_status_changed = INTERRUPT_STATUS_CHANGED;

  /* Advance the queue head. */
  DOSEVQ_SET_QHEAD (next_qhead);
}


/* Checks to see if the conventional memory queue is requesting
 * a virtual interrupt, and if so, signals the appropriate virtual
 * interrupt(s).
 */
void
dosevq_note_mouse_interrupt (void)
{
  if (mouse_handler_and_queue_sel != 0)
    {
      int pending;

      pending = DOSEVQ_INTERRUPT_PENDING_MASK ();
      if (pending)
	{
	  DOSEVQ_SET_INTERRUPT_PENDING_MASK (0);
	  if (pending & DOSEVQ_MOUSE_MOVED_PENDING_MASK)
	    cpu_state.interrupt_pending[M68K_MOUSE_MOVED_PRIORITY] = 1;
	  if (pending & DOSEVQ_EVENT_PENDING_MASK)
	    cpu_state.interrupt_pending[M68K_EVENT_PRIORITY] = 1;
	  cpu_state.interrupt_status_changed = INTERRUPT_STATUS_CHANGED;
	}
    }
}


/* This dummy function is used to mark the end of memory to "wire
 * down" so it won't get paged out.  This is a hack, but there's no
 * nice way to do it.
 */
void dosevq_lock_functions_end (void) { }


/* These functions are used to communicate with the mouse driver. */
#define MOUSE_INIT		0x0000
#define MOUSE_HIDE		0x0002
#define MOUSE_SET_XY		0x0004
#define MOUSE_SET_XLIMIT	0x0007
#define MOUSE_SET_YLIMIT	0x0008
#define MOUSE_MOVE_DELTA	0x000B
#define MOUSE_INSTALL_HANDLER	0x000C

#define MOUSE_DRIVER_CALL(AX, CX, DX)			\
  do {							\
    /* Don't zero regs here!  Other regs may be set. */	\
    regs.x.ax = (AX);					\
    regs.x.cx = (CX);					\
    regs.x.dx = (DX);					\
    __dpmi_int (0x33, &regs);				\
  } while (0)


/* Sets up a real mode mouse stub, to use in place of a real-mode callback.
 * We seem to need this for WinNT 3.51.
 */
static boolean_t
setup_mouse_stub (void)
{
  int seg, sel;

  seg = __dpmi_allocate_dos_memory ((DOS_EVENT_QUEUE_OFFSET
				     + sizeof (dosevq_queue_t) + 15U) / 16,
				    &sel);
  if (seg != -1)
    {
      /* We used to use a DPMI real mode callback for mouse events.
       * That seems to be broken under WinNT, so now we copy a
       * real-mode mouse event handler to conventional memory and let
       * it process the mouse events for us.
       */
      mouse_callback_addr.segment = seg;
      mouse_callback_addr.offset16 = 0;
      mouse_handler_and_queue_sel = sel;
      movedata (dos_pm_ds, (unsigned) &dos_mouse_stub[0],
		sel, 0, dos_mouse_stub_bytes_to_copy);

      /* Hard code in the proper segment value for %ds.  We could
       * instead assume %cs has the right value, but this is safer.
       */
      _farpokew (sel,
		 ((char *) &dos_mouse_stub_segment
		  - (char *) &dos_mouse_stub[0]),
		 seg);
    }
  else
    {
      mouse_callback_addr.segment = 0;
      mouse_callback_addr.offset16 = 0;
      mouse_handler_and_queue_sel = 0;
    }

  return (seg != -1);
}  

typedef enum { initialize, reinitialize } what_init_t;

static const char *
init_mouse_common (what_init_t what)
{
  const char *msg = NULL;
  __dpmi_regs regs;

  /* Initialize the mouse driver. */
  dpmi_zero_regs (&regs);
  MOUSE_DRIVER_CALL (MOUSE_INIT, 0, 0);
  if (regs.x.ax != 0xFFFF)
    {
      msg = "No mouse driver detected; you need a mouse to run Executor.\n";
      goto done;
    }

  /* Hide the mouse (probably not necessary). */
  dpmi_zero_regs (&regs);
  MOUSE_DRIVER_CALL (MOUSE_HIDE, 0, 0);
  
  /* Read the motion counters to clear them. */
  dpmi_zero_regs (&regs);
  MOUSE_DRIVER_CALL (MOUSE_MOVE_DELTA, 0, 0);

  if (what == initialize && !setup_mouse_stub ())
    {
      msg = "Unable to allocate conventional memory for mouse driver.\n";
      goto done;
    }

  /* Install the mouse event handler. */
  dpmi_zero_regs (&regs);
  regs.x.es = mouse_callback_addr.segment;
  MOUSE_DRIVER_CALL (MOUSE_INSTALL_HANDLER, 
		     (MOUSE_MOTION_MASK | MOUSE_LEFT_PRESSED_MASK
		      | MOUSE_LEFT_RELEASED_MASK),
		     mouse_callback_addr.offset16);

done:
  mouse_initialized = (msg == NULL);
  return msg;
}

static const char *
init_mouse (void)
{
  const char *retval;

  retval = init_mouse_common (initialize);
  return retval;
}

const char *
dosevq_reinit_mouse (void)
{
  const char *retval;

  retval = init_mouse_common (reinitialize);
  return retval;
}

static const char *
init_keyboard (void)
{
  const char *errmsg;

  /* Chain keyboard interrupt. */
  new_kbd_handler.offset32 = (unsigned long) &dosevq_kbd_int;
  new_kbd_handler.selector = dos_pm_cs;
  if ((__dpmi_get_protected_mode_interrupt_vector (KBD_VEC, &old_kbd_handler)
       != -1)
      && (__dpmi_set_protected_mode_interrupt_vector (KBD_VEC,
						      &new_kbd_handler)
	  != -1))
    {
      errmsg = NULL;
    }
  else
    {
      errmsg = "Unable to initialize keyboard.\n";
    }

  keyboard_initialized = (errmsg == NULL);

  return errmsg;
}


const char *
dosevq_init (void)
{
  const char *errmsg;

  /* Lock all memory we might touch at interrupt time. */
  dpmi_lock_memory (&mouse_callback_regs, sizeof mouse_callback_regs);
  dpmi_lock_memory (&old_kbd_handler, sizeof old_kbd_handler);
  dpmi_lock_memory (&new_kbd_handler, sizeof new_kbd_handler);
  dpmi_lock_memory (&dosevq_asm_begin,
		    &dosevq_asm_end - &dosevq_asm_begin);
  dpmi_lock_memory ((char *) dosevq_lock_functions_begin,
		    ((char *) dosevq_lock_functions_end
		     - (char *) dosevq_lock_functions_begin));
  dpmi_lock_memory (&mouse_handler_and_queue_sel,
		    sizeof mouse_handler_and_queue_sel);

  /* Set up the keyboard and mouse. */
  errmsg = init_keyboard ();
  if (errmsg == NULL)
    errmsg = init_mouse ();

  /* See if we got an error initializing stuff. */
  if (errmsg == NULL)
    atexit (dosevq_shutdown);
  else
    dosevq_shutdown ();

  return errmsg;
}


void
dosevq_shutdown (void)
{
  if (keyboard_initialized)
    {
      __dpmi_set_protected_mode_interrupt_vector (KBD_VEC, &old_kbd_handler);
      keyboard_initialized = FALSE;
    }

  if (mouse_initialized)
    {
      __dpmi_regs regs;
      int sel_to_free;

      /* Reinitializing the mouse driver will remove our handler. */
      dpmi_zero_regs (&regs);
      MOUSE_DRIVER_CALL (MOUSE_INIT, 0, 0);

      /* Avoid a race condition with the timer code that checks the queue
       * for an interrupt pending notice by zeroing this selector now,
       * before we invalidate it.
       */
      sel_to_free = mouse_handler_and_queue_sel;
      mouse_handler_and_queue_sel = 0;

      /* Free up the mouse real-mode callback, if we allocated one,
       * and the conventional memory for our queue.
       */
      if (use_mouse_rmcb_p
	  && (mouse_callback_addr.segment || mouse_callback_addr.offset16))
	__dpmi_free_real_mode_callback (&mouse_callback_addr);
      if (sel_to_free != 0)
	__dpmi_free_dos_memory (sel_to_free);
      mouse_callback_addr.segment = 0;
      mouse_callback_addr.offset16 = 0;

      mouse_initialized = FALSE;
    }
}


/* Returns by reference the next event in the queue.  Returns the
 * type of the event, or EVTYPE_NONE if there are no events left
 * in the queue.
 */
dosevq_type_t
dosevq_dequeue (dosevq_record_t *e)
{
  e->type = EVTYPE_NONE;  /* Default value. */

  if (mouse_handler_and_queue_sel != 0)
    {
      unsigned qtail;
      
      qtail = DOSEVQ_QTAIL ();
      if (DOSEVQ_QHEAD () != qtail)
	{
	  *(uint32 *) e = DOSEVQ_QELT (qtail);
	  DOSEVQ_SET_QTAIL ((qtail + 1) % DOSEVQ_QUEUE_SIZE);
	}
      
      if (DOSEVQ_MOUSE_DX () || DOSEVQ_MOUSE_DY ())
	{
	  cpu_state.interrupt_pending[M68K_MOUSE_MOVED_PRIORITY] = 1;
	  cpu_state.interrupt_status_changed = INTERRUPT_STATUS_CHANGED;
	}
    }

  return e->type;
}
