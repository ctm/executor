/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

CPUState cpu_state;

bool arch_init(void)
{
    memset(&cpu_state, 0, sizeof cpu_state);
    callback_init();
    return true;
}
