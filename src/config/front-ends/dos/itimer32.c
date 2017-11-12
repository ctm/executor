/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_itimer32[] = "$Id: itimer32.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include <target-config.h> /* See if we're using synchronous interrupts. */

#if defined(MSDOS) && !defined(SYNCHRONOUS_INTERRUPTS)

#include "rsys/itimer.h"
#include "rsys/int386.h"
#include "rsys/blockinterrupts.h"
#include "OSEvent.h"
#include "OSUtil.h"
#include "rsys/prefs.h"

#include <dos.h>
#include <go32.h>
#include <dpmi.h>

/* Reprogram clock to 60Hz? */

INTEGER ROMlib_ticks60 = 1;

/* Function to be called the next time the interval timer expires. */
static void (*itimer_func)(void) = NULL;
static long itimer_ticks = 0;

/* Boolean value; 1 while we have int 8 patched, else 0. */
static volatile int itimer_chained = 0;

static _go32_dpmi_seginfo old_handler;
static _go32_dpmi_seginfo new_handler;
BOOLEAN iret_wrapper_allocated = false;

#if 1
#define REPROGRAM_TIMER_0(ticks_per_sec)                                     \
    asm volatile("movb $0x36,%%al\n\t"                                       \
                 "outb %%al,$0x43\n\t"                                       \
                 "jmp 1f\n"                                                  \
                 "1:\tjmp 1f\n"                                              \
                 "1:\tjmp 1f\n"                                              \
                 "1:\tmovb %0,%%al\n\t"                                      \
                 "outb %%al,$0x40\n\t"                                       \
                 "jmp 1f\n"                                                  \
                 "1:\tjmp 1f\n"                                              \
                 "1:\tjmp 1f\n"                                              \
                 "1:\tmovb %1,%%al\n\t"                                      \
                 "outb %%al,$0x40\n\t"                                       \
                 :                                                           \
                 : "g"((unsigned)((65532 / ((ticks_per_sec) / 18.2)) + 0.5)  \
                       & 0xFF),                                              \
                   "g"(((unsigned)((65532 / ((ticks_per_sec) / 18.2)) + 0.5) \
                                                                             \
                        >> 8)                                                \
                       & 0xFF)                                               \
                 : "eax")

#define MICROSECONDS_PER_TICK          \
    (ROMlib_ticks60                    \
         ? (1000000L / 60) /* 60 Hz */ \
         : (1000000L * 10 / 182)) /* 18.2 Hz */
#endif

#if !defined(MICROSECONDS_PER_TICK)
#define MICROSECONDS_PER_TICK (1000000L * 10 / 182) /* 18.2 Hz */
#endif

static void unchain()
{
    real_int_state_t bi;

    if(itimer_chained)
    {
        bi = block_real_ints();
#if defined(REPROGRAM_TIMER_0)
        if(ROMlib_ticks60)
        {
            REPROGRAM_TIMER_0(18.2);
        }
#endif
        _go32_dpmi_set_protected_mode_interrupt_vector(8, &old_handler);
        itimer_chained = 0;

        if(iret_wrapper_allocated)
        {
            _go32_dpmi_free_iret_wrapper(&new_handler);
            iret_wrapper_allocated = false;
        }

        restore_real_ints(bi);
    }
}

typedef enum {
    syn_started = (1 << 0),
    int_pending = (1 << 1),
    in_int = (1 << 2),
    in_aline = (1 << 3),
    blocked = (1 << 4)
} syn_cpu_state_t;

#define STATE_MASK (syn_started | int_pending | in_int | in_aline | blocked)

PRIVATE syn_cpu_state_t syn_cpu_state = 0;

/*
 * NOTE: we ignore tzp.
 */

void protected_gettimeofday(struct timeval *tvp, struct timezone *tzp)
{
    static struct timeval last_known_time;
    static long last_known_clock;
    static unsigned long last_known_ticks;
    static char beenhere = false;
    long temp_clock;

    if(!beenhere)
    {
        gettimeofday(tvp, tzp);
        if(ROMlib_ticks60 && ROMlib_clock)
            last_known_ticks = CL(Ticks);
        else
            last_known_clock = clock();
        beenhere = true;
    }

    if(ROMlib_ticks60 && ROMlib_clock)
    {
        unsigned long usec_diff, t;

        /* Compute the change in Ticks since last time we were here. */
        t = CL(Ticks);
        usec_diff = (t - last_known_ticks) * 1000000 / 60;
        last_known_ticks = t;

        *tvp = last_known_time;
        tvp->tv_sec += usec_diff / 1000000;
        tvp->tv_usec += usec_diff % 1000000;
        tvp->tv_sec += tvp->tv_usec / 1000000;
        tvp->tv_usec %= 1000000;

        last_known_time = *tvp;
    }
    else
    {
        if(syn_cpu_state & in_int)
        {
            temp_clock = clock() - last_known_clock;
            *tvp = last_known_time;
            tvp->tv_sec += temp_clock / 1000000;
            tvp->tv_usec += temp_clock % 1000000;
            tvp->tv_sec += tvp->tv_usec / 1000000;
            tvp->tv_usec %= 1000000;
        }
        else
        {
            gettimeofday(tvp, tzp);
            last_known_clock = clock();
            last_known_time = *tvp;
        }
    }
}

static void do_itimer(void)
{
    asm("cld"); /* Broken djgpp libs assume d is always clear. */
    while((syn_cpu_state & STATE_MASK) == (syn_started | int_pending))
    {
        syn_cpu_state &= ~int_pending;
        syn_cpu_state |= in_int;
        itimer_func();
        syn_cpu_state &= ~in_int;
    }
}

static void tic_handler(void)
{
    real_int_state_t bi;
#if defined(REPROGRAM_TIMER_0)
    static int usecs_elapsed = 0;
#endif

    bi = block_real_ints();
    if(--itimer_ticks == 0)
    { /* TODO: take syn_cpu_state into consider */
        if(ROMlib_ticks60 && ROMlib_clock)
        {
            unsigned long t;

            /* Increment Ticks and Time. */
            t = CL(Ticks) + 1;
            Ticks = CL(t);
            Time = CL(UNIXTIMETOMACTIME(ROMlib_startup) + t / 60);
        }

        syn_cpu_state |= int_pending;
        do_itimer();
    }

#if defined(REPROGRAM_TIMER_0)
    if(ROMlib_ticks60)
    {
        /* See if we should forward this tick to the normal 18.2 Hz handler. */
        usecs_elapsed += MICROSECONDS_PER_TICK;
        if(usecs_elapsed >= (1000000 * 10 / 182))
        {
            usecs_elapsed -= 1000000 * 10 / 182;
            asm volatile("cli\n\t"
                         "pushfl\n\t"
                         "lcall %0"
                         :
                         : "m"(*&old_handler.pm_offset));
        }
        else
        {
            /* Issue end-of-interrupt command. */
            asm volatile("cli\n\t"
                         "outb %b0,$0x20"
                         :
                         : "a"(0x20));
        }
    }
#endif
    restore_real_ints(bi);
}

short ROMlib_disable_dos_timer = 0;

/* This allows you to specify a given function to be called in the specified
 * number of microseconds.  If an itimer was already installed, it is removed
 * and replaced with the new information.  Passing an argument of "0" for
 * usecs will remove any pending interrupt.
 */
int itimer_set(unsigned long usecs, void (*func)(void))
{
    real_int_state_t bi;

    if(!ROMlib_disable_dos_timer)
    {
        bi = block_real_ints();
        itimer_func = func;
        itimer_ticks = func == NULL ? -1 : usecs / MICROSECONDS_PER_TICK;
        if(!itimer_ticks)
            itimer_ticks = 1;
        if(usecs)
        {
            if(!itimer_chained)
            {
                struct timeval tv;
                struct timezone tz;

                /* Initialize starting time. */
                protected_gettimeofday(&tv, &tz);

                _go32_dpmi_get_protected_mode_interrupt_vector(8,
                                                               &old_handler);
                new_handler.pm_offset = (int)tic_handler;
                new_handler.pm_selector = _go32_my_cs();

                if(
#if !defined(REPROGRAM_TIMER_0)
                    0 &&
#endif
                    ROMlib_ticks60)
                {
                    if(_go32_dpmi_allocate_iret_wrapper(&new_handler))
                    {
                        gui_fatal("Unable to allocate iret wrapper "
                                  "for the clock.");
                    }
                    iret_wrapper_allocated = true;
                    _go32_dpmi_set_protected_mode_interrupt_vector(8,
                                                                   &new_handler);
                }
                else
                    _go32_dpmi_chain_protected_mode_interrupt_vector(8, &new_handler);

                if(ROMlib_ticks60)
                {
                    REPROGRAM_TIMER_0(60.0);
                }
                else
                {
                    REPROGRAM_TIMER_0(18.2);
                }

                itimer_chained = 1;
                atexit(unchain);
            }
        }
        restore_real_ints(bi);
    }
    return 1;
}

void ROMlib_blockdostimer(void)
{
    syn_cpu_state |= blocked;
}

void ROMlib_unblockdostimer(void)
{
    real_int_state_t bi;

    bi = block_real_ints();
    syn_cpu_state &= ~blocked;
    do_itimer();
    restore_real_ints(bi);
}

void syn_begin(void)
{
    real_int_state_t bi;

    bi = block_real_ints();
    syn_cpu_state |= syn_started;
    do_itimer();
    restore_real_ints(bi);
}

void aline_begin(void)
{
    syn_cpu_state |= in_aline;
}

void aline_end(void)
{
    real_int_state_t bi;

    bi = block_real_ints();
    syn_cpu_state &= ~in_aline;
    do_itimer();
    restore_real_ints(bi);
}
#endif /* defined(MSDOS) && !defined(SYNCHRONOUS_INTERRUPTS) */
