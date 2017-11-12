/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_m68k_destroy[] = "$Id: m68k-destroy.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

unsigned long
destroy_blocks(syn68k_addr_t low_m68k_address, uint32 num_bytes)
{
    /* Tell NEXTSTEP to totally flush the caches. */
    asm("trap #2");
    return 1;
}

unsigned long
destroy_blocks_with_checksum_mismatch(syn68k_addr_t low_m68k_address,
                                      uint32 num_bytes)
{
    return destroy_blocks(low_m68k_address, num_bytes);
}
