/* Copyright 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "dpmicall.h"
#include "rsys/blockinterrupts.h"
#include "dosdisk.h" /* should go away. */

void dpmi_zero_regs(__dpmi_regs *regs)
{
    memset(regs, 0, sizeof *regs);
}

/* Same as __dpmi_int, but also returns -1 if the carry is set
 * after the __dpmi_int is performed.  Returns 0 on success.
 */
int dpmi_int_check_carry(int vector, __dpmi_regs *regs)
{
    int retval;

    retval = __dpmi_int(vector, regs);
    if(regs->x.flags & I386_CC_CARRY_MASK)
        retval = -1;

    return retval;
}

#if ERROR_SUPPORTED_P(ERROR_TRACE_INFO)

#define DUMP_PIECE(bufp, regsp, u, component)                            \
    do                                                                   \
    {                                                                    \
        sprintf(bufp + strlen(bufp),                                     \
                "%s = 0x%08lx\n", #component, (long)regsp->u.component); \
    } while(0)

static void
dump_regs(__dpmi_regs *regsp)
{
    if(ERROR_ENABLED_P(ERROR_TRACE_INFO))
    {
        char buf[1024];

        buf[0] = 0;
        DUMP_PIECE(buf, regsp, d, edi);
        DUMP_PIECE(buf, regsp, d, esi);
        DUMP_PIECE(buf, regsp, d, ebp);
        DUMP_PIECE(buf, regsp, d, res);
        DUMP_PIECE(buf, regsp, d, ebx);
        DUMP_PIECE(buf, regsp, d, edx);
        DUMP_PIECE(buf, regsp, d, ecx);
        DUMP_PIECE(buf, regsp, d, eax);
        DUMP_PIECE(buf, regsp, x, flags);
        DUMP_PIECE(buf, regsp, x, es);
        DUMP_PIECE(buf, regsp, x, ds);
        DUMP_PIECE(buf, regsp, x, fs);
        DUMP_PIECE(buf, regsp, x, gs);
        DUMP_PIECE(buf, regsp, x, ip);
        DUMP_PIECE(buf, regsp, x, cs);
        DUMP_PIECE(buf, regsp, x, sp);
        DUMP_PIECE(buf, regsp, x, ss);
        /*      warning_trace_info ("%s", buf); */
    }
}

/* Calls the specified function, logging the registers before and after.
 * Returns the value returned by the given function.
 */
static int
logging_dpmi_int_helper(int vector, __dpmi_regs *regsp, const char *label,
                        int (*int_func)(int, __dpmi_regs *))
{
    int retval;

#if ERROR_SUPPORTED_P(ERROR_TRACE_INFO)
    /* warning_trace_info ("INT 0x%02x (%s)", vector, label); */
    dump_regs(regsp);
#endif

    retval = (*int_func)(vector, regsp);

#if ERROR_SUPPORTED_P(ERROR_TRACE_INFO)
    /* warning_trace_info ("retval = %d", retval); */
    dump_regs(regsp);
#endif

    return retval;
}

/* Calls __dpmi_int, but logs the input and output registers. */
int logging_dpmi_int(int vector, __dpmi_regs *regsp, const char *label)
{
    int retval;
    retval = logging_dpmi_int_helper(vector, regsp, label, __dpmi_int);
    return retval;
}

/* Calls dpmi_int_check_carry, but logs the input and output registers. */
int logging_dpmi_int_check_carry(int vector, __dpmi_regs *regsp,
                                 const char *label)
{
    int retval;
    retval = logging_dpmi_int_helper(vector, regsp, label,
                                     dpmi_int_check_carry);
    return retval;
}

#endif /* ERROR_SUPPORTED_P (ERROR_TRACE_INFO) */
