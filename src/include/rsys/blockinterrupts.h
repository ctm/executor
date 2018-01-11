/*
 * Copyright 1992, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#if !defined(_BLOCKINTERRUPTS_H_)
#define _BLOCKINTERRUPTS_H_

#include "rsys/time.h"

namespace Executor
{
typedef uint8 virtual_int_state_t;
extern virtual_int_state_t _virtual_interrupts_blocked;

/* A virtual_int_state_t is a four bit value.  The low bit controls
 * whether Executor sees the interrupts as masked.  The other three
 * bits are the old interrupt mask from the 68k status register.
 */
#define block_virtual_ints()                               \
    ({                                                     \
        virtual_int_state_t _old_state;                    \
        _old_state = ((_virtual_interrupts_blocked & true) \
                      | ((cpu_state.sr >> 7) & 0xE));      \
        cpu_state.sr |= (7 << 8); /* int mask = 7 */       \
        _virtual_interrupts_blocked = true;                \
        _old_state;                                        \
    })
#define restore_virtual_ints(n)                                                    \
    do                                                                             \
    {                                                                              \
        virtual_int_state_t _new_int_state = (n);                                  \
        _virtual_interrupts_blocked = _new_int_state & 1;                          \
        cpu_state.sr = (cpu_state.sr & ~(7 << 8)) | ((_new_int_state & 0xE) << 7); \
        /* Now force a recheck, in case an interrupt really did come in            \
         * but was masked out so it got ignored.                                   \
         */                                                                        \
        cpu_state.interrupt_status_changed = INTERRUPT_STATUS_CHANGED;             \
    } while(0)

extern void do_virtual_interrupt(void);
#define check_virtual_interrupt()                    \
    do                                               \
    {                                                \
        if(!_virtual_interrupts_blocked)             \
        {                                            \
            if(INTERRUPT_PENDING())                  \
                do_virtual_interrupt();              \
        }                                            \
    } while(0)

class BlockVirtualInterruptsGuard
{
    virtual_int_state_t state;

public:
    BlockVirtualInterruptsGuard()
        : state(block_virtual_ints())
    {
    }
    ~BlockVirtualInterruptsGuard()
    {
        restore_virtual_ints(state);
    }
};
}

#endif /* !_BLOCKINTERRUPTS_H_ */
