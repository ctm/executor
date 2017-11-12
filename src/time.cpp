/* Copyright 1989, 1990, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_time[] = "$Id: time.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in TimeMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "OSUtil.h"
#include "TimeMgr.h"

#include "rsys/blockinterrupts.h"
#include "rsys/pstuff.h"
#include "rsys/osutil.h"
#include "rsys/vbl.h"
#include "rsys/time.h"
#include "rsys/syncint.h"
#include "rsys/hook.h"
#include "rsys/refresh.h"
#include "rsys/soundopts.h"

#if !defined(SYN68K)
#include "m68k-stack.h"
#endif

using namespace Executor;

#if defined(SYSV)
LONGINT
setitimer(which, value, ovalue) /* TODO */
    LONGINT which;
CONST struct itimerval *value;
struct itimerval *ovalue;
{
    return 0L;
}
#endif /* SYSV */

PUBLIC QHdr Executor::ROMlib_timehead;

/* Actual time at which Executor started running, GMT. */
PUBLIC struct timeval ROMlib_start_time;

#if defined(SYN68K)
/* Current state of virtual interrupt enabling. */
PUBLIC virtual_int_state_t Executor::_virtual_interrupts_blocked = false;
#endif

/* Msecs during last interrupt. */
PRIVATE unsigned long last_interrupt_msecs;

/* Msecs during next anticipated interrupt. */
PRIVATE unsigned long next_interrupt_msecs;

#if defined(MSDOS) && !defined(USE_BIOS_TIMER)

unsigned long
msecs_elapsed(void)
{
    static char beenhere = false;
    unsigned long e;

    if(!beenhere)
    {
        struct timezone tz;

        tz.tz_minuteswest = 0;
        tz.tz_dsttime = 0;
        gettimeofday(&ROMlib_start_time, &tz); /* GMT */
        set_elapsed_1024(0);

        beenhere = true;
    }

    /* Convert elapsed 1024ths of a second to elapsed milliseconds,
   * and try to avoid overflow.
   */
    e = fetch_elapsed_1024();
    return ((e >> 10) * 1000) + (((e & 1023) * 1000) >> 10);
}

#else /* !defined (MSDOS) || defined (USE_BIOS_TIMER) */

#if !defined(CYGWIN32)
unsigned long
msecs_elapsed()
{
    struct timeval t;
    struct timezone tz;
    unsigned long m;
    static unsigned long start_msecs;

    tz.tz_minuteswest = 0;
    tz.tz_dsttime = 0;
    gettimeofday(&t, &tz); /* GMT */
    m = (t.tv_sec * 1000 + t.tv_usec / 1000);
    if(start_msecs == 0)
    {
        start_msecs = m;
        ROMlib_start_time = t;
    }
    return m - start_msecs;
}
#endif

#endif /* !defined (MSDOS) || defined (USE_BIOS_TIMER) */

/*
 * catchalarm has been written with an eye toward not having errors accumulate.
 * In the UNIX world it is going to be hard to give great response time, but
 * it is unexcusable for repeated calls to catchalarm to distort the timing
 * of events that are far in the future.
 *
 * Unfortunately, unless we change the data structure that is in the linked
 * list, errors are bound to accumulate.  Each time we take an interrupt
 * our idea of how much time has passed could be off by as much as a
 * millisecond.  We have to take interrupts 60 times a second so we will tend
 * to drift a couple ticks each second.  If this proves unacceptable, the
 * datastructure will have to be expanded so we keep a timeval around that
 * tells us when to expire.
 */

#define REALLONGTIME 0x7FFFFFFF

#if defined(SYN68K)
A2(PUBLIC, syn68k_addr_t, catchalarm, syn68k_addr_t, interrupt_pc,
   void *, unused)
#else
A3(PRIVATE, LONGINT, catchalarm, LONGINT, volatile signo,
   LONGINT, volatile code, struct sigcontext *, volatile scp)
#endif
{
    ULONGINT diff;
    TMTask *qp;
    LONGINT min;
    LONGINT tm_count;
    M68kReg saved_regs[16];
    CCRElement saved_ccnz, saved_ccn, saved_ccc, saved_ccv, saved_ccx;
    unsigned long now_msecs;

    /* We save the 68k registers and cc bits away. */
    memcpy(saved_regs, &cpu_state.regs, sizeof saved_regs);
    saved_ccnz = cpu_state.ccnz;
    saved_ccn = cpu_state.ccn;
    saved_ccc = cpu_state.ccc;
    saved_ccv = cpu_state.ccv;
    saved_ccx = cpu_state.ccx;

#if defined(SYN68K)

    /* There's no reason to think we need to decrement A7 by 32;
   * it's just a paranoid thing to do.
   */
    EM_A7 = (EM_A7 - 32) & ~3; /* Might as well long-align it. */

#else /* !SYN68K */

    /* Since we don't know which stack we were on when interrupted,
   * switch stacks as appropriate.  If all stacks are busy,
   * there's not much we can do other than return.  
   */
    if(!m68k_use_interrupt_stacks())
        return 0;

#endif /* !SYN68K */

    /* Loop while it's still time to do stuff sitting in the queue. */
    do
    {
        unsigned long msecs;

        msecs = msecs_elapsed();
        diff = msecs - last_interrupt_msecs;
        last_interrupt_msecs = msecs;

        for(qp = (TMTask *)MR(ROMlib_timehead.qHead);
            qp;
            qp = (TMTask *)MR(qp->qLink))
        {
            tm_count = CL(qp->tmCount);

            if(tm_count > 0)
            {
                tm_count -= diff;
                qp->tmCount = CL(tm_count);
                if(tm_count <= 0)
                {
                    ProcPtr tm_addr;
                    ROMlib_hook(time_number);

                    tm_addr = MR(qp->tmAddr);
                    if(tm_addr == (ProcPtr)P_ROMlib_wakeup)
                        C_ROMlib_wakeup();
                    else if(tm_addr == (ProcPtr)P_ROMlib_vcatch)
                        C_ROMlib_vcatch();
                    else if(tm_addr == (ProcPtr)P_handle_refresh)
                        C_handle_refresh();
                    else if(tm_addr == (ProcPtr)P_sound_timer_handler)
                        C_sound_timer_handler();
                    else if(tm_addr)
                    {
                        /* No need to save and restore regs here; we
		       * save and restore all of them outside this
		       * loop. */
                        EM_A0 = US_TO_SYN68K(tm_addr);
                        EM_A1 = US_TO_SYN68K(qp);

                        CALL_EMULATOR((syn68k_addr_t)EM_A0);
                    }
                }
            }
        }

        /* Find the next imminent timer event in the queue. */
        min = REALLONGTIME;
        for(qp = (TMTask *)MR(ROMlib_timehead.qHead);
            qp;
            qp = (TMTask *)MR(qp->qLink))
        {
            tm_count = CL(qp->tmCount);
            if(tm_count > 0 && tm_count < min)
                min = tm_count;
        }

        /* Fetch the current time and move the nearest event even closer
       * to compensate for time spent in this procedure.
       */
        now_msecs = msecs_elapsed();
        if(min < REALLONGTIME)
            min -= now_msecs - last_interrupt_msecs;
    } while(min <= 0);

    if(min < REALLONGTIME)
    {
/* If there's anything left in the queue, set up another
       * timer interrupt to come in at the appropriate time.
       */

#if defined(SYN68K)
        syncint_post(min * 1000);
#else /* !SYN68K */
        struct itimerval t;

        t.it_value.tv_sec = min / 1000;
        t.it_value.tv_usec = (min * 1000) % 1000000;
        t.it_interval.tv_sec = 0;
        t.it_interval.tv_usec = 0;
        setitimer(ITIMER_REAL, &t, (struct itimerval *)0);
#endif /* !SYN68K */

        next_interrupt_msecs = now_msecs + min;
    }
    else
    {
        /* Note that there's no interrupt queued up. */
        next_interrupt_msecs = 0;
    }

#if !defined(SYN68K)
    m68k_restore_stacks();
#endif

    memcpy(&cpu_state.regs, saved_regs, sizeof saved_regs);
    cpu_state.ccnz = saved_ccnz;
    cpu_state.ccn = saved_ccn;
    cpu_state.ccc = saved_ccc;
    cpu_state.ccv = saved_ccv;
    cpu_state.ccx = saved_ccx;

#if defined(SYN68K)
    return MAGIC_RTE_ADDRESS;
#else
    return 0;
#endif
}

namespace Executor
{
PRIVATE void ROMlib_PrimeTime(QElemPtr, LONGINT);
}

A2(PRIVATE, void, ROMlib_PrimeTime, QElemPtr, taskp, LONGINT, count)
{
    static char beenhere = false;
    LONGINT msecs_until_next;
    virtual_int_state_t block;
    unsigned long now_msecs;

/*
 * We introduce this fudge factor because we are nervous that our time
 * calculations will mess up and result in a negative number under Executor
 * when on a real Mac the number wouldn't be negative.  This is paranoia,
 * but small timing stuff just isn't going to work properly under Executor
 * anyway.
 */

#define FUDGE_FACTOR (-30)

    if(count < FUDGE_FACTOR)
        count = -count / 1000; /* IM-Processes 3-20 */

    if(count <= 0)
        count = 1;

    block = block_virtual_ints();

    now_msecs = msecs_elapsed();
    if(!beenhere)
    {
        last_interrupt_msecs = now_msecs; /* actually there haven't been any */
        msecs_until_next = 0x7FFF0000; /* Arbitrary large value. */
        next_interrupt_msecs = now_msecs + msecs_until_next;
        beenhere = true;

#if !defined(SYN68K)
        signal(SIGALRM, (void *)catchalarm);
#endif
    }
    else
    {
        msecs_until_next = next_interrupt_msecs - now_msecs;
    }

    /* catchalarm works by subtracting off the msecs _since the last
   * interrupt_ from each entry in the queue.  Since we might be
   * getting posted sometime between interrupts, we have to compensate
   * by adding the time since the last interrupt to our count.  That
   * way the extra time subtracted off during the catchalarm will
   * exactly match the extra time we added here.
   */
    ((TMTask *)taskp)->tmCount = CL(count + now_msecs - last_interrupt_msecs);

    if(count < msecs_until_next || msecs_until_next <= 0)
    {
#if defined(SYN68K)
        syncint_post(count * 1000);
#else /* !SYN68K */
        struct itimerval t;
        t.it_value.tv_sec = count / 1000;
        t.it_value.tv_usec = (count * 1000) % 1000000;
        t.it_interval.tv_sec = 0;
        t.it_interval.tv_usec = 0;
        setitimer(ITIMER_REAL, &t, (struct itimerval *)0);
#endif /* !SYN68K */

        next_interrupt_msecs = now_msecs + count;
    }

    restore_virtual_ints(block);
}

A1(PUBLIC, void, InsTime, QElemPtr, taskp)
{
    ((TMTask *)taskp)->tmCount = CLC(-1);
    Enqueue(taskp, &ROMlib_timehead);
}

A1(PUBLIC, void, RmvTime, QElemPtr, taskp)
{
    Dequeue(taskp, &ROMlib_timehead);
}

A2(PUBLIC, void, PrimeTime, QElemPtr, taskp, LONGINT, count)
{
    ROMlib_PrimeTime(taskp, count);
}
