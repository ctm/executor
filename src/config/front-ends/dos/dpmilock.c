/* Copyright 1995 - 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include <dpmi.h>
#include "dpmilock.h"
#include "dosmem.h"

void dpmi_lock_memory(void *start, unsigned long num_bytes)
{
    unsigned long base = 0;

    /* Get the linear base address. */
    if(__dpmi_get_segment_base_address(dos_pm_ds, &base) != -1)
    {
        __dpmi_meminfo mem;

        mem.handle = 0; /* Unused */
        mem.size = num_bytes;
        mem.address = (unsigned long)((char *)start + base);
        __dpmi_lock_linear_region(&mem);
    }
}
