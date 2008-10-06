#if !defined (_syncint_h_)
#define _syncint_h_

#if defined (MSDOS)

#include <go32.h>
#include <dpmi.h>

extern uint32 fetch_elapsed_1024 (void);
extern void set_elapsed_1024 (uint32 v);
extern uint8 use_bios_timer_p;
extern boolean_t set_expect_slow_clock (boolean_t will_be_slow_p);

#define M68K_WATCHDOG_PRIORITY 7  /* unmaskable */
#define M68K_WATCHDOG_VECTOR (24 + M68K_WATCHDOG_PRIORITY)

#endif /* MSDOS */

#if defined (SDL)
#define SDL_DEFINE_INIT_ONLY

#include "SDL/SDL.h"
#include "SDL/SDL_timer.h"

#undef SDL_DEFINE_INIT_ONLY
#endif /* SDL */

extern int syncint_init (void);
extern void syncint_post (unsigned long usecs);

#endif /* !_syncint_h_ */
