#if !defined(_DOSMEM_H_)
#define _DOSMEM_H_

#define DOS_BUF_SIZE (32768 - 16)
#define DOS_STACK_TOP ((DOS_BUF_SIZE - 16) & ~3)
#define DOS_MIN_STACK_SPACE 512

extern uint16 dos_buf_segment;
extern uint16 dos_buf_selector;
extern uint16 dos_rm_selector;
extern uint16 dos_pm_ds;
extern uint16 dos_pm_interrupt_ds;
extern uint16 dos_pm_cs;

extern bool init_dos_memory(void);

#endif /* !_DOSMEM_H_ */
