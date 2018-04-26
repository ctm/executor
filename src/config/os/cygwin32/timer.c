/* Copyright 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Set up a 60 hz timer interrupt using the multimedia timers */
/* Are these used anymore? */

#include "rsys/common.h"
#include "rsys/blockinterrupts.h"

#include "timer.h"

/* Timer handler */
static void __attribute__((stdcall))
MTimer_Intr(uint32_t uID, uint32_t uMsg, uint32_t dwUser, uint32_t dw1, uint32_t dw2)
{
    /* Eventually we may want to add some intelligent projection here
   * and adjust the value of the timer to match the real-time circumstances.
   * (QueryPerformanceCounter and friends)
   */

    /* Virtual interrupt */
    if(!_virtual_interrupts_blocked)
    {
        interrupt_generate(M68K_TIMER_PRIORITY);
    }
}

static uint32_t Timer32;

/* Init */
int Timer32_Init(void)
{
    /* Allow 1 ms of drift so we don't chew the CPU with the timer */
    Timer32 = timeSetEvent(1000 / 60, 1, MTimer_Intr, 0, TIME_PERIODIC);
    return (Timer32 ? true : false);
}

/* Finish -- this needs to be called for cleanup */
void Timer32_End(void)
{
    timeKillEvent(Timer32);
}
