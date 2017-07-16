/*
 * Copyright 1992, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: blockinterrupts.h 63 2004-12-24 18:19:43Z ctm $
 */

#if !defined (_BLOCKINTERRUPTS_H_)
#define _BLOCKINTERRUPTS_H_

#include "rsys/time.h"

namespace Executor {
typedef uint32 real_int_state_t;

# define block_real_ints()	\
    ((real_int_state_t) sigblock (sigmask (SIGALRM)))
# define restore_real_ints(n)	\
    ((void) sigsetmask (n))


#if defined (SYN68K)
typedef uint8 virtual_int_state_t;
extern virtual_int_state_t _virtual_interrupts_blocked;

/* A virtual_int_state_t is a four bit value.  The low bit controls
 * whether Executor sees the interrupts as masked.  The other three
 * bits are the old interrupt mask from the 68k status register.
 */
# define block_virtual_ints()				\
({ virtual_int_state_t _old_state;			\
   _old_state = ((_virtual_interrupts_blocked & TRUE)	\
		 | ((cpu_state.sr >> 7) & 0xE));	\
   cpu_state.sr |= (7 << 8);	/* int mask = 7 */	\
   _virtual_interrupts_blocked = TRUE;			\
   _old_state;						\
 })
# define restore_virtual_ints(n)					     \
do {									     \
  virtual_int_state_t _new_int_state = (n);				     \
  _virtual_interrupts_blocked = _new_int_state & 1;			     \
  cpu_state.sr = (cpu_state.sr & ~(7 << 8)) | ((_new_int_state & 0xE) << 7); \
  /* Now force a recheck, in case an interrupt really did come in	     \
   * but was masked out so it got ignored.				     \
   */									     \
  cpu_state.interrupt_status_changed = INTERRUPT_STATUS_CHANGED;	     \
} while (0)

#else /* !SYN68K */

typedef real_int_state_t virtual_int_state_t;
# define block_virtual_ints()    ((virtual_int_state_t) block_real_ints())
# define restore_virtual_ints(n) restore_real_ints ((real_int_state_t) (n))

#endif /* !SYN68K */


#if defined (MSDOS)
# define IF_MSDOS(x) x
#else
# define IF_MSDOS(x)
#endif


#if defined (SYN68K)

extern void do_virtual_interrupt (void);
#define check_virtual_interrupt()			\
do							\
{							\
  if (!_virtual_interrupts_blocked)			\
    {							\
      IF_MSDOS (dosevq_note_mouse_interrupt ());	\
      if (INTERRUPT_PENDING ())				\
	do_virtual_interrupt ();			\
    }							\
} while (0)
#else /* !SYN68K */
# define check_virtual_interrupt()
#endif /* !SYN68K */


#define BLOCK_VIRTUAL_INTERRUPTS_EXCURSION(body)	\
do {							\
  virtual_int_state_t __vstate = block_virtual_ints ();	\
  { body }						\
  restore_virtual_ints (__vstate);			\
} while (0)

#define BLOCK_REAL_INTERRUPTS_EXCURSION(body)		\
do {							\
  real_int_state_t __rstate = block_real_ints ();	\
  { body }						\
  restore_real_ints (__rstate);				\
} while (0)
}

#endif /* !_BLOCKINTERRUPTS_H_ */
