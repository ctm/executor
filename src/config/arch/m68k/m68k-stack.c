/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "m68k-stack.h"

uint32_t last_executor_stack_ptr;

#define INTERRUPT_STACK_SIZE (256 * 1024 - 16)
#define NUM_INTERRUPT_STACKS 2

typedef struct
{
    uint32_t *old_stack_variable;
    uint32_t old_stack_value;
    uint8 stack[INTERRUPT_STACK_SIZE];
} m68k_interrupt_stack_t;

static m68k_interrupt_stack_t interrupt_stack[NUM_INTERRUPT_STACKS];

static volatile int current_interrupt_stack = 0;

bool m68k_use_interrupt_stacks(void)
{
    m68k_interrupt_stack_t *m;
    int s;

    s = current_interrupt_stack;
    if(s + 2 > NUM_INTERRUPT_STACKS)
        return false;
    current_interrupt_stack = s + 2;

    m = &interrupt_stack[s];

    /* Switch Mac stack. */
    m[0].old_stack_variable = &EM_A7;
    m[0].old_stack_value = EM_A7;
    EM_A7 = (uint32_t)&m[0].stack[INTERRUPT_STACK_SIZE - 16];

    /* Switch Executor stack. */
    m[1].old_stack_variable = &last_executor_stack_ptr;
    m[1].old_stack_value = last_executor_stack_ptr;
    last_executor_stack_ptr
        = (uint32_t)&m[1].stack[INTERRUPT_STACK_SIZE - 16];

    return true;
}

void m68k_restore_stacks(void)
{
    m68k_interrupt_stack_t *m;
    int s;

    s = current_interrupt_stack - 2;
    if(s < 0)
        abort();
    else
    {
        m = &interrupt_stack[s];
        *(m[0].old_stack_variable) = m[0].old_stack_value;
        *(m[1].old_stack_variable) = m[1].old_stack_value;

        current_interrupt_stack = s;
    }
}
