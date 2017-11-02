/* Copyright 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_timer[] = "$Id: timer.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Set up a 60 hz timer interrupt using the multimedia timers */
/* Are these used anymore? */

#include "rsys/common.h"
#include "rsys/blockinterrupts.h"

#include "timer.h"

/* Timer handler */
static void __attribute__ ((stdcall)) 
  MTimer_Intr(uint32 uID, uint32 uMsg, uint32 dwUser, uint32 dw1, uint32 dw2)
{
  /* Eventually we may want to add some intelligent projection here
   * and adjust the value of the timer to match the real-time circumstances.
   * (QueryPerformanceCounter and friends)
   */

  /* Virtual interrupt */
  if ( ! _virtual_interrupts_blocked )
    {
      cpu_state.interrupt_pending[M68K_TIMER_PRIORITY] = true;
      SET_INTERRUPT_STATUS (INTERRUPT_STATUS_CHANGED);
    }
}

static uint32 Timer32;

/* Init */
int Timer32_Init(void)
{
  /* Allow 1 ms of drift so we don't chew the CPU with the timer */
  Timer32 = timeSetEvent(1000/60, 1, MTimer_Intr, 0, TIME_PERIODIC);
  return (Timer32 ? true : false);
}

/* Finish -- this needs to be called for cleanup */
void Timer32_End(void)
{
  timeKillEvent(Timer32);
}
