/* Copyright 1994 - 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_lowglobals_mem[] = "$Id: lowglobals-mem.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* defined in conjunction with `MMAP_LOW_GLOBALS' */

#include "rsys/common.h"
#include "rsys/memory_layout.h"

#define OP_MASK 0xFC000000
#define REG1_MASK 0x03E00000
#define REG1_SHIFT 21
#define REG2_MASK 0x001F0000
#define REG2_SHIFT 16

#define OP_LDQ_U 0x2C000000
#define OP_STQ_U 0x3C000000
#define OP_LDL 0xA0000000
#define OP_LDQ 0xA4000000
#define OP_STL 0xB0000000
#define OP_STQ 0xB4000000

#if !defined(NDEBUG)
static int seg_fault_count;
#endif /* !NDEBUG */

void seg_fault_handler(int signo, int code, struct sigcontext *scp)
{
    unsigned int instr;
    unsigned long addr;
    long *regaddr;

#if defined(NDEBUG)
    seg_fault_count++;
#endif /* !NDEBUG */

    instr = *(unsigned int *)scp->sc_pc;
    addr = ((short)instr
            + scp->sc_regs[(instr & REG2_MASK) >> REG2_SHIFT]
            + PAGE_ZERO_SHADOW_START);
    regaddr = &scp->sc_regs[(instr & REG1_MASK) >> REG1_SHIFT];

    switch(instr & OP_MASK)
    {
        case OP_LDQ_U:
            *regaddr = *(long *)(addr & 0xFFFFFFFFFFFFFFF8);
            break;
        case OP_STQ_U:
            *(long *)(addr & 0xFFFFFFFFFFFFFFF8) = *regaddr;
            break;
        case OP_LDL:
            *regaddr = *(int *)addr;
            break;
        case OP_LDQ:
            *regaddr = *(long *)addr;
            break;
        case OP_STL:
            *(int *)addr = *regaddr;
            break;
        case OP_STQ:
            *(long *)addr = *regaddr;
            break;
        default:
            fatal_error("unknown instruction `%x'", instr);
    }
    scp->sc_pc += 4;
    sigreturn(scp);
}

int mmap_lowglobals()
{
    caddr_t addr;
    struct sigaction action;
    sigset_t newmask;

    addr = mmap((caddr_t)PAGE_ZERO_SHADOW_START,
                PAGE_ZERO_SIZE,
                PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE, -1, 0);
    gui_assert(addr == (caddr_t)PAGE_ZERO_SHADOW_START);

    sigemptyset(&newmask);
    action.sa_handler = (void *)seg_fault_handler;
    action.sa_mask = newmask;
    action.sa_flags = 0;
    sigaction(SIGSEGV, &action, (struct sigaction *)0);
}
