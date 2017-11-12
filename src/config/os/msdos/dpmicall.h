#if !defined(_DPMICALL_H_)
#define _DPMICALL_H_

#include <dpmi.h>

extern void dpmi_zero_regs(__dpmi_regs *regs);
extern int dpmi_int_check_carry(int vector, __dpmi_regs *regs);

#if ERROR_SUPPORTED_P(ERROR_TRACE_INFO)

extern int logging_dpmi_int(int vector, __dpmi_regs *regs, const char *label);
extern int logging_dpmi_int_check_carry(int vector, __dpmi_regs *regsp,
                                        const char *label);

#else /* !ERROR_SUPPORTED_P (ERROR_TRACE_INFO) */

#define logging_dpmi_int(v, r, l) __dpmi_int(v, r)
#define logging_dpmi_int_check_carry(v, r, l) dpmi_int_check_carry(v, r)

#endif /* !ERROR_SUPPORTED_P (ERROR_TRACE_INFO) */

#endif /* _DPMICALL_H_ */
