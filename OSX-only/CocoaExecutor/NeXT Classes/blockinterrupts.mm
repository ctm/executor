#include "rsys/common.h"
#include "rsys/mactype.h"
#include <mach/mach.h>
#include <mach/thread_status.h>
#include <pthread.h>
#include <cthreads.h>
#include <signal.h>
#include <sys/time.h>
#include <assert.h>
#include "rsys/blockinterrupts.h"

static mutex_t m;
static condition_t c;

static thread_t parentthread;
static cthread_t parentcthread;

#if 0
static int done;

static void callsomeroutine( void (*routinetocallp)( void * ), void *arg )
{
    (*routinetocallp)( arg );
/*    mutex_lock(m); necessary? */
    done = 1;
/*     mutex_unlock(m); necessary? */
    condition_signal(c);
    for (;;)
	cthread_yield();
    gui_assert(0);
}

void ROMlib_callcompletion( void *chanp )
{
    struct NeXT_thread_state_regs regs, newregs;
    struct NeXT_thread_state_68882 s68882;
    struct NeXT_thread_state_user_reg ureg;
    static unsigned int regcount    = sizeof(regs),
    			s68882count = sizeof(s68882),
			uregcount   = sizeof(ureg);

    thread_suspend(parentthread);
    cthread_abort(parentcthread);
    thread_get_state(parentthread, NeXT_THREAD_STATE_REGS,
						     (int *) &regs, &regcount);
    thread_get_state(parentthread, NeXT_THREAD_STATE_68882,
						(int *) &s68882, &s68882count);
    thread_get_state(parentthread, NeXT_THREAD_STATE_USER_REG,
						    (int *) &ureg, &uregcount);
    newregs = regs;
    newregs.pc = (long) callsomeroutine;


/* TODO	set up stack with args... */

    thread_set_state(parentthread, NeXT_THREAD_STATE_REGS,
						   (int *) &newregs, regcount);
    mutex_lock(m);
    done = 0;
    thread_resume(parentthread);
    while (!done)
	    condition_wait(c, m);
    mutex_unlock(m);
    thread_suspend(parentthread);
    cthread_abort(parentcthread);	/* necessary! */
    thread_set_state(parentthread, NeXT_THREAD_STATE_REGS,
						      (int *) &regs, regcount);
    thread_set_state(parentthread, NeXT_THREAD_STATE_68882,
					         (int *) &s68882, s68882count);
    thread_set_state(parentthread, NeXT_THREAD_STATE_USER_REG,
						     (int *) &ureg, uregcount);
    thread_resume(parentthread);
}
#endif

static long blockcount = 0;

#error "This stuff has succumbed to bitrot; see new virtual interrupt stuff"
void blockinterrupts_init( void )
{
    m = mutex_alloc();
    c = condition_alloc();

    parentthread = thread_self();
    parentcthread = cthread_self();
}

void blockinterrupts (blockinterrupts_t *blockp)
{
    *blockp = sigblock(sigmask(SIGALRM));	/* must precede mutex_lock */
    mutex_lock(m);
    ++blockcount;
    mutex_unlock(m);
}

void unblockinterrupts (const blockinterrupts_t *blockp)
{
    mutex_lock(m);
    if (--blockcount < 0)
	blockcount = 0;
    mutex_unlock(m);
    sigsetmask(*blockp);	/* must be after mutex_unlock */
    condition_signal(c);
}

void requestinterrupts( void )
{
    mutex_lock(m);
    while (blockcount > 0)
	condition_wait(c, m);
    blockcount = 1;
    mutex_unlock(m);
}

void unrequestinterrupts( void )
{
    mutex_lock(m);
    if (--blockcount < 0)
	blockcount = 0;
    mutex_unlock(m);
    condition_signal(c);
}
