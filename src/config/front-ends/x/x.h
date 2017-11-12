/* define `X' the x front-end */
#if !defined(X)
#define X
#endif /* !X */

extern syn68k_addr_t post_pending_x_events(syn68k_addr_t interrupt_addr,
                                           void *unused);
extern bool x_event_pending_p(void);
extern void WeOwnScrapX(void);
extern void ROMlib_set_use_scancodes(bool val);
