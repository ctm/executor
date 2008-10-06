/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_syncint[] =
		    "$Id: syncint.c 97 2005-06-22 20:08:38Z ctm $";
#endif

#include "rsys/common.h"

/* We only have synchronous interrupts under syn68k. */
#if defined (SYN68K)

#include "rsys/syncint.h"
#include "rsys/blockinterrupts.h"

#if defined (CYGWIN32)
#include "win_queue.h"
#include "rsys/m68kint.h"
#endif

#if defined (MSDOS)

#include <go32.h>
#include <dpmi.h>
#include "dpmilock.h"
#include "dosmem.h"
#include "dpmicall.h"
#include "rmint70.h"

#define BIOS_SET_EVENT_WAIT   0x8300
#define BIOS_CLEAR_EVENT_WAIT 0x8301


#define TIMER_INT_NUM    0x70
#define WATCHDOG_INT_NUM 0x08

/* This is the old timer interrupt handler.  These cannot be static
 * because they are used by the interrupt asm code.
 */
__dpmi_paddr old_watchdog_handler;
__dpmi_paddr old_pm_timer_handler;
__dpmi_raddr old_rm_timer_handler;

uint16 rm_timer_handler_sel;

/* Here's our new one. */
static __dpmi_paddr new_watchdog_handler;
static __dpmi_paddr new_pm_timer_handler;
static __dpmi_raddr new_rm_timer_handler;

/* TRUE if we shouldn't get concerned if the 1024 Hz clock isn't
 * working well, e.g. during disk accesses.
 */
static boolean_t expect_slow_clock_p;

/* These variables are referenced in our interrupt handlers. */
volatile uint32 watchdog_last_elapsed_1024 = -1;
volatile uint32 slow_clock_count;
uint8 use_bios_timer_p = TRUE;

#define ELAPSED_1024_OFFSET		 0
#define ELAPSED_1024_OFFSET_STR		"0"
#define REMAINING_1024_OFFSET		 4
#define REMAINING_1024_OFFSET_STR	"4"

void
_dummy_timer_asm_function_ (void)
{
  /* We put this asm in a dummy function so we can pass it "arguments"
   * to let it reference cpu_state fields.
   */
  asm (
       /* We maintain two distinct timer interrupts:  "timer" and "watchdog".
	* "timer" is a 1024 Hz interrupt which the BIOS sometimes turns off.
	* "watchdog" is an 18.2 Hz interrupt that checks to see if "timer"
	* has been turned off.  If it has, it sets things up so that
	* "timer" will get turned back on.  This solution should be fairly
	* robust, although if "timer" gets turned off often then we'll
	* appear to "lose time".
	*
	* If use_bios_p is FALSE, then we exclusively use the 18.2 Hz
	* timer.  This may work around some problems with various BIOSes
	* and DPMI providers, although we're not sure.
	*/
       "\n"
       ".text\n\t"
       ".align 4,0x90\n"
       "_timer_handler_data_start:\n"
       "_handle_watchdog_tick:\n\t"

       /* Save stuff we're about to clobber. */
       "pushfl\n\t"
       "pushal\n\t"
       "pushl %%ds\n\t"
       "pushl %%es\n\t"
       "pushl %%fs\n\t"
       "pushl %%gs\n\t"

       /* Set up %ds for protected mode memory. */
       ".byte 0x2E\n\t"  /* %cs: prefix, for gas botch. */
       QUOTED_DATA16 "movw _dos_pm_interrupt_ds,%%ds\n\t"

       /* See if any virtual mouse interrupts are pending. */
       "call _dosevq_note_mouse_interrupt\n\t"

       /* Fetch elapsed 1024ths of a second. */
       "movzwl _rm_timer_handler_sel,%%eax\n\t"
       "testl %%eax,%%eax\n\t"
       "jz 1f\n\t"		/* zero selector? bail! */
       QUOTED_DATA16 "movw %%ax,%%fs\n\t"
       ".byte 0x64\n\t"		/* %fs: prefix */
       "movl " ELAPSED_1024_OFFSET_STR ",%%eax\n\t"

       /* See if we care about the 1024Hz BIOS timer. */
       "cmpb $0,_use_bios_timer_p\n\t"
       "jz 3f\n\t"

       /* See if the 1024Hz timer is still running. */
       "cmpl %%eax,_watchdog_last_elapsed_1024\n\t"
       "jz 4f\n\t"

       /* See if it's running, but very slowly (for WinNT) */
       "pushl %%eax\n\t"
       "subl _watchdog_last_elapsed_1024,%%eax\n\t"
       "cmpl $28,%%eax\n\t"	/* We should get about 56 ticks here. */
       "popl %%eax\n\t"
       
       "jnge 4f\n\t"	/* too slow? */

       /* Clock is moving quickly enough this time. */
       "movl $0,_slow_clock_count\n\t"	/* start "slow count" over */
       "jmp 1f\n"

       "4:\n\t"

       /* Note that the clock has not advanced quickly enough here. */
       "incl _slow_clock_count\n\t"

       /* Indicate a watchdog interrupt. */
       "movb $1,%1\n"
       "movl %3,%2\n\t"

       "3:\n\t"

       /* Increment the 1024 Hz clock by 1/18.2 of a second. */
       "addl $56,%%eax\n\t"
       ".byte 0x64\n\t"		/* %fs: prefix */
       "movl " REMAINING_1024_OFFSET_STR ",%%ebx\n\t"
       ".byte 0x64\n\t"		/* %fs: prefix */
       "movl %%eax," ELAPSED_1024_OFFSET_STR "\n\t"
       "cmpl $0x80000000,%%ebx\n\t"
       "je 1f\n\t"
       "subl $56,%%ebx\n\t"
       "jnle 8f\n\t"
       "movb $1,%0\n\t"
       "movl %3,%2\n\t"
       "movl $0x80000000,%%ebx\n"

       "8:\n\t"
       ".byte 0x64\n\t"		/* %fs: prefix */
       "movl %%ebx," REMAINING_1024_OFFSET_STR "\n\t"

       /* Restore everything we clobbered. */
       "1:\n\t"
       "movl %%eax,_watchdog_last_elapsed_1024\n\t"
       "popl %%gs\n\t"
       "popl %%fs\n\t"
       "popl %%es\n\t"
       "popl %%ds\n\t"
       "popal\n\t"
       "popfl\n\t"

       ".byte 0x2E\n\t"  /* %cs: prefix, for gas botch. */
       "ljmp _old_watchdog_handler\n\t"
       "iret\n\t"  /* Not sure why this is necessary, but djgpp does it. */

       /* This is the timer interrupt handler.  It executes the
	* equivalent of this C code:
	*
	* ++elapsed_1024;
	* r1024 = remaining_1024;
	* if (r1024 > 0)
	*   {
	*     --r1024;
	*     remaining_1024 = r1024;
	*     if (r1024 <= 0)
	*       {
	*         SET_INTERRUPT_STATUS (INTERRUPT_STATUS_CHANGED);
	*       }
	*   }
	*/
       
       ".align 4,0xcf\n"
       "_handle_timer_tick:\n\t"
       
       /* Save stuff we're about to clobber. */
       "pushfl\n\t"
       "pushl %%eax\n\t"
       "pushl %%ds\n\t"
       "pushl %%fs\n\t"
       
       /* Set up %%ds for protected mode. */
       ".byte 0x2E\n\t"  /* %cs: prefix, for gas botch. */
       QUOTED_DATA16 "movw _dos_pm_interrupt_ds,%%ds\n\t"

       /* Set up %fs appropriately. */
       "movzwl _rm_timer_handler_sel,%%eax\n\t"
       "testl %%eax,%%eax\n\t"
       "je 2f\n\t"			/* bail if selector is zero */
       QUOTED_DATA16 "movw %%ax,%%fs\n\t"
       
       /* Increment elapsed_1024, and decrement remaining_1024
	* (if appropriate).  If remaining_1024 hits zero, then set
	* the interrupt_status_changed bit.
	*/
       ".byte 0x64\n\t"			/* %fs prefix */
       "movl " REMAINING_1024_OFFSET_STR ",%%eax\n\t"
       ".byte 0x64\n\t"			/* %fs prefix */
       "incl " ELAPSED_1024_OFFSET_STR "\n\t"
       "cmpl $0x80000000,%%eax\n\t"	/* countdown not running? */
       "je 2f\n\t"
       "subl $1,%%eax\n\t"		/* decl doesn't set carry */
       "jnle 7f\n\t"
       "movb $1,%0\n\t"
       "movl %3,%2\n\t"
       "movl $0x80000000,%%eax\n"

       "7:\n\t"
       ".byte 0x64\n\t"			/* %fs prefix */
       "movl %%eax," REMAINING_1024_OFFSET_STR "\n"
       
       "2:\n\t"

#if 1
       /* Reset real-time clock timer (docs are extremely shaky here). */
       "movb $0x0C,%%al\n\t"
       "outb %%al,$0x70\n\t"
       "inb $0x71,%%al\n\t"
       
       /* Indicate end-of-interrupt to the interrupt controller. */
       "movb $0x20,%%al\n\t"
       "outb %%al,$0xA0\n\t"  /* Needed since INT 0x70 is IRQ 8 */
       "outb %%al,$0x20\n\t"

       /* Restore the stuff we changed. */
       "popl %%fs\n\t"
       "popl %%ds\n\t"
       "popl %%eax\n\t"
       "popfl\n\t"
        
       "sti\n\t"
       "iret\n"
#else
       
       /* This is old code you could use if you wanted to chain the
	* interrupt.
	*/

       /* Restore the stuff we changed. */
       "popl %%fs\n\t"
       "popl %%ds\n\t"
       "popl %%eax\n\t"
       "popfl\n\t"
        
       /* Chain on to the next interrupt handler. */
       ".byte 0x2E\n\t"  /* %cs: prefix, for gas botch. */
       "ljmp _old_pm_timer_handler\n\t"
       "iret\n"  /* Not sure why this is necessary, but djgpp does it. */
#endif

       "_timer_handler_data_end:\n"
       ".text"
       
       : "=m" (cpu_state.interrupt_pending[M68K_TIMER_PRIORITY]),
         "=m" (cpu_state.interrupt_pending[M68K_WATCHDOG_PRIORITY]),
         "=m" (cpu_state.interrupt_status_changed)
       : "g" (INTERRUPT_STATUS_CHANGED));
}


/* Trick the C compiler so it knows about some of these asm symbols. */
extern char handle_timer_tick, handle_watchdog_tick;
extern char timer_handler_data_start, timer_handler_data_end;


uint32
fetch_elapsed_1024 (void)
{
  uint32 retval;

  if (rm_timer_handler_sel)
    retval = _farpeekl (rm_timer_handler_sel, ELAPSED_1024_OFFSET);
  else
    retval = 0;

  return retval;
}


void
set_elapsed_1024 (uint32 v)
{
  if (rm_timer_handler_sel)
    _farpokel (rm_timer_handler_sel, ELAPSED_1024_OFFSET, v);
}


static void
syncint_shutdown (void)
{
  BLOCK_VIRTUAL_INTERRUPTS_EXCURSION
    ({
      __dpmi_regs regs;

      /* Clear any pending interrupt. */
      syncint_post (0);

      __dpmi_set_protected_mode_interrupt_vector (WATCHDOG_INT_NUM,
						  &old_watchdog_handler);
      __dpmi_set_protected_mode_interrupt_vector (TIMER_INT_NUM,
						  &old_pm_timer_handler);
      __dpmi_set_real_mode_interrupt_vector (TIMER_INT_NUM,
					     &old_rm_timer_handler);
      
      /* Turn off the BIOS event wait, in case anyone else wants it. */
      dpmi_zero_regs (&regs);
      regs.x.ax = BIOS_CLEAR_EVENT_WAIT;
      __dpmi_int (0x15, &regs);


      /* THIS HAS BEEN DISABLED.
       * Freeing this memory causes Executor to crash on exit under
       * WinNT.  It's not clear why; we've already reset all the
       * interrupt vectors and no one should be using this memory once
       * rm_timer_handler_sel is zero.
       */
      if (0 && rm_timer_handler_sel != 0)
	{
	  uint16 sel;
	  sel = rm_timer_handler_sel;
	  rm_timer_handler_sel = 0; /* just in case of any race condition */
	  __dpmi_free_dos_memory (sel);
	}
    });
}


/* This routine starts int 0x70 going by turning on a dummy BIOS
 * event wait.
 */
static syn68k_addr_t
restart_timer (syn68k_addr_t interrupt_addr, void *unused)
{
  if (use_bios_timer_p)
    {
      __dpmi_regs regs;

      /* See if we've gotten two seconds worth of sluggish 1024 Hz clock
       * ticks in a row.  If so, disable that clock and just use the
       * 18 Hz clock.  This seems to be needed for WinNT.
       */
      if (!expect_slow_clock_p && slow_clock_count > 36)  /* 18 Hz * 2 == 36 */
	{
	  use_bios_timer_p = FALSE;
	  __dpmi_set_protected_mode_interrupt_vector (TIMER_INT_NUM,
						      &old_pm_timer_handler);
	  __dpmi_set_real_mode_interrupt_vector (TIMER_INT_NUM,
						 &old_rm_timer_handler);
	  warning_unexpected ("1024 Hz timer is unreliable.  Switching "
			      "to 18 Hz timer.  elapsed_1024 = %lu",
			      (unsigned long) fetch_elapsed_1024 ());
	}
      else
	{
	  /* Start the clock running by turning on a bogus event wait that
	   * we never want to finish.
	   */  
	  dpmi_zero_regs (&regs);
	  regs.x.ax = BIOS_SET_EVENT_WAIT;
	  regs.x.cx = 0x7FFF;
	  regs.x.dx = 0xFFFF;
	  regs.x.es = 0x40;    /* Use the MSB of the BIOS countdown address, */
	  regs.x.bx = 0x9C + 3;   /*  since setting that bit is harmless.    */
	  if (dpmi_int_check_carry (0x15, &regs))
	    {
	      /* It won't let us set the 1024 timer!  Let's not be rude and
	       * clear it; that may be causing problems under Windows.  We'll
	       * just let the 18.2 Hz watchdog limp along and keep trying to
	       * start the clock.
	       */
	    }
	}
    }

  return MAGIC_RTE_ADDRESS;
}


/* Sets a flag indicating whether a slow 1024 Hz clock is acceptable
 * (i.e. we shouldn't turn it off if it seems to be misbehaving).
 */
boolean_t
set_expect_slow_clock (boolean_t will_be_slow_p)
{
  boolean_t old;

  old = expect_slow_clock_p;
  if (expect_slow_clock_p != will_be_slow_p)
    {
      if (!will_be_slow_p)
	slow_clock_count = 0;
      expect_slow_clock_p = will_be_slow_p;
    }
  return old;
}


/* Sets up real-mode memory and handler used for timer stuff. */
static void
set_up_rm_timer_stuff (void)
{
  int sel, seg;

  seg = __dpmi_allocate_dos_memory ((&rm_int_70_handler_end
				     - &rm_int_70_handler_start + 15) / 16,
				    &sel);
  if (seg != -1)
    {
      rm_timer_handler_sel = sel;
      
      rm_int_70_chain_address = old_rm_timer_handler;
      rm_int_70_seg = seg;
      
      /* Copy the timer handler to DOS memory. */
      movedata (dos_pm_ds, (unsigned) &rm_int_70_handler_start,
		sel, 0,
		&rm_int_70_handler_end - &rm_int_70_handler_start);

      if (use_bios_timer_p)
	{
	  /* Make it so that the timer doesn't reflect to us from real
	   * mode, which would require a mode switch.  Sandmann thinks
	   * that setting the RM vector after we set the PM vector
	   * will cause RM interrupts not to get reflected to PM.
	   * We're doing this as a test because mode switches from RM to
	   * PM during disk accesses seem to cause trouble on some
	   * machines.
	   */
	  new_rm_timer_handler.segment = seg;
	  new_rm_timer_handler.offset16 = (&rm_int_70_handler_code_start
					   - &rm_int_70_handler_start);
	  __dpmi_set_real_mode_interrupt_vector (TIMER_INT_NUM,
						 &new_rm_timer_handler);
	}
    }
}


int
syncint_init (void)
{
  static char beenhere = FALSE;
  syn68k_addr_t watchdog_callback;

  /* Guard against getting called multiple times. */
  if (beenhere)
    return TRUE;
  beenhere = TRUE;

  /* Wire down our timer interrupt handler so it won't get paged out. */
  dpmi_lock_memory (&timer_handler_data_start,
		    &timer_handler_data_end - &timer_handler_data_start);
  dpmi_lock_memory (&rm_timer_handler_sel, sizeof rm_timer_handler_sel);
  dpmi_lock_memory (&old_watchdog_handler, sizeof old_watchdog_handler);
  dpmi_lock_memory (&old_pm_timer_handler, sizeof old_pm_timer_handler);
  dpmi_lock_memory (&old_rm_timer_handler, sizeof old_rm_timer_handler);
  dpmi_lock_memory ((void *)&use_bios_timer_p, sizeof use_bios_timer_p);
  dpmi_lock_memory ((void *)&slow_clock_count, sizeof slow_clock_count);
  dpmi_lock_memory ((void *)&watchdog_last_elapsed_1024,
		    sizeof watchdog_last_elapsed_1024);

  /* Set up the m68k trap vector for the watchdog interrupt. */
  watchdog_callback = callback_install (restart_timer, NULL);
  *(syn68k_addr_t *)SYN68K_TO_US(M68K_WATCHDOG_VECTOR * 4) = CL (watchdog_callback);

  /* Fetch the old interrupt handlers. */
  __dpmi_get_real_mode_interrupt_vector (TIMER_INT_NUM, &old_rm_timer_handler);
  __dpmi_get_protected_mode_interrupt_vector (TIMER_INT_NUM,
					      &old_pm_timer_handler);
  __dpmi_get_protected_mode_interrupt_vector (WATCHDOG_INT_NUM,
					      &old_watchdog_handler);

  /* Set up real-mode timer memory. */
  set_up_rm_timer_stuff ();

  if (use_bios_timer_p)
    {
      new_pm_timer_handler.offset32 = (unsigned long) &handle_timer_tick;
      new_pm_timer_handler.selector = dos_pm_cs;
      __dpmi_set_protected_mode_interrupt_vector (TIMER_INT_NUM,
						  &new_pm_timer_handler);
  
      /* Start the 1024 Hz clock running. */
      restart_timer (0, NULL);
    }
  
  /* Install the new watchdog interrupt handler. */
  new_watchdog_handler.offset32 = (unsigned long) &handle_watchdog_tick;
  new_watchdog_handler.selector = dos_pm_cs;
  __dpmi_set_protected_mode_interrupt_vector (WATCHDOG_INT_NUM,
					      &new_watchdog_handler);

  atexit (syncint_shutdown);
  return TRUE;
}


/* Posting a delay of 0 will clear any pending interrupt. */
void
syncint_post (unsigned long usecs)
{
  long new;

  if (rm_timer_handler_sel)
    {
      if (usecs == 0)
	new = 0x80000000;  /* magic value meaning timer is stopped. */
      else
	{
	  /* Convert usecs to 1024ths of a second, rounding up. */
	  new = (usecs * 16 + 15624) / 15625;/* 16 / 15625 == 1024 / 1000000 */
	  if (new <= 0)
	    new = 1;
	}

      _farpokel (rm_timer_handler_sel, REMAINING_1024_OFFSET, new);
    }
}

#else /* !MSDOS */


#if defined (SDL)

static Uint32
handle_sdltimer_tick (Uint32 n)
{
  //  fprintf (stderr, "T");
  //  fflush (stderr);
#if defined (CYGWIN32)
  win_queue(&cpu_state.interrupt_pending[M68K_EVENT_PRIORITY]);
#endif
  cpu_state.interrupt_pending[M68K_TIMER_PRIORITY] = TRUE;
  SET_INTERRUPT_STATUS (INTERRUPT_STATUS_CHANGED);
  //  SDL_mutexP (ROMlib_shouldbeawake_mutex);
  SDL_CondSignal (ROMlib_shouldbeawake_cond);
  //  SDL_mutexV (ROMlib_shouldbeawake_mutex);

#if !defined (SDL_TIMER_FIXED)
#warning returning 1 here to work around SDL timer bug(s)
  return 1;
#endif

  return 0;
}

#else

static void
handle_itimer_tick (int n)
{
  cpu_state.interrupt_pending[M68K_TIMER_PRIORITY] = TRUE;
  SET_INTERRUPT_STATUS (INTERRUPT_STATUS_CHANGED);
}

#endif

int
syncint_init (void)
{
#if defined (SDL)

  return (SDL_Init(SDL_INIT_TIMER) == 0);

#else /* !SDL */

#if !defined(USE_BSD_SIGNALS)

  struct sigaction s;

  s.sa_handler  = handle_itimer_tick;
  sigemptyset(&s.sa_mask);
  s.sa_flags    = 0;
  s.sa_restorer = NULL;
  return (sigaction (SIGALRM, &s, NULL) == 0);

#else
  struct sigvec s;

  s.sv_handler = handle_itimer_tick;
  s.sv_mask    = 0;
  s.sv_flags   = 0;
  return sigvec (SIGALRM, &s, NULL) == 0;
#endif

#endif
}


/* Posting a delay of 0 will clear any pending interrupt. */
void
syncint_post (unsigned long usecs)
{
#if defined (SDL)
  //  fprintf (stderr, "P(%ul)", usecs);
  //  fflush (stderr);
  SDL_SetTimer(usecs/1000, handle_sdltimer_tick);
#else
  struct itimerval t;

  t.it_value.tv_sec     = usecs / 1000000;
  t.it_value.tv_usec    = usecs % 1000000;
  t.it_interval.tv_sec  = 0;
  t.it_interval.tv_usec = 0;
  setitimer (ITIMER_REAL, &t, NULL);
#endif
}

#endif /* !MSDOS */

#endif /* SYN68K */
