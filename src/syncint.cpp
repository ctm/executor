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

using namespace Executor;

#if defined (SDL)

static Uint32
handle_sdltimer_tick (Uint32 n)
{
	//  fprintf (stderr, "T");
	//  fflush (stderr);
#if defined (CYGWIN32)
	win_queue(&cpu_state.interrupt_pending[M68K_EVENT_PRIORITY]);
#endif
	cpu_state.interrupt_pending[M68K_TIMER_PRIORITY] = true;
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
  cpu_state.interrupt_pending[M68K_TIMER_PRIORITY] = true;
  SET_INTERRUPT_STATUS (INTERRUPT_STATUS_CHANGED);
}

#endif

int
Executor::syncint_init (void)
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
Executor::syncint_post (unsigned long usecs)
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

#endif /* SYN68K */
