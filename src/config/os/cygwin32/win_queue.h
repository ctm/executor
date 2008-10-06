#if !defined (__WIN_QUEUE_H__)
#define __WIN_QUEUE_H__

extern void win_queue (volatile uint8 *pendingp);
extern void set_timer_driven_events (boolean_t value);

#endif
