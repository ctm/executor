#if !defined (_M68KINT_H_)
#define _M68KINT_H_

/* These are the m68k interrupts we "patch into" to get asynchronous
 * information from the system.
 */

#define M68K_EVENT_PRIORITY 2
#define M68K_EVENT_VECTOR (24 + M68K_EVENT_PRIORITY)

#define M68K_MOUSE_MOVED_PRIORITY 3
#define M68K_MOUSE_MOVED_VECTOR (24 + M68K_MOUSE_MOVED_PRIORITY)

#define M68K_TIMER_PRIORITY 4
#define M68K_TIMER_VECTOR (24 + M68K_TIMER_PRIORITY)

#define M68K_SOUND_PRIORITY 5
#define M68K_SOUND_VECTOR (24 + M68K_SOUND_PRIORITY)

#define M68K_WATCHDOG_PRIORITY 7  /* unmaskable */
#define M68K_WATCHDOG_VECTOR (24 + M68K_WATCHDOG_PRIORITY)

#endif /* !_M68KINT_H_ */
