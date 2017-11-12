#if !defined(_M68K_STACK_H_)
#define _M68K_STACK_H_

extern bool m68k_use_interrupt_stacks(void);
extern void m68k_restore_stacks(void);

extern uint32 last_executor_stack_ptr;

#endif /* _M68K_STACK_H_ */
