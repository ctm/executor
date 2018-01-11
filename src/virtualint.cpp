/* Copyright 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/blockinterrupts.h"
using namespace Executor;

void Executor::do_virtual_interrupt(void)
{
    syn68k_addr_t pc;

    pc = interrupt_process_any_pending(MAGIC_EXIT_EMULATOR_ADDRESS);
    if(pc != MAGIC_EXIT_EMULATOR_ADDRESS)
    {
        interpret_code(hash_lookup_code_and_create_if_needed(pc));
    }
}