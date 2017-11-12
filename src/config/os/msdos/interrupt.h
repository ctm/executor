#if !defined(_MSDOS_INTERRUPT_H_)
#define _MSDOS_INTERRUPT_H_

extern void msdos_register_enable_disable_funcs(void (*enable_func)(void), void (*disable_func)(void));
extern void msdos_disable_interrupts(void);
extern void msdos_enable_interrupts(void);

#endif /* !_MSDOS_INTERRUPT_H_ */
