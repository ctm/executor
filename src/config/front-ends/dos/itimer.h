#if !defined (_itimer_h_)
#define _itimer_h_

extern void itimer_init (void);
extern void itimer_deinit (void);
extern int itimer_set (unsigned long usecs, void (*func) (void));
extern void itimer_clear (void);

extern void ROMlib_blockdostimer( void );
extern void ROMlib_unblockdostimer( void );

extern INTEGER ROMlib_ticks60;

#endif /* Not _itimer_h_ */
