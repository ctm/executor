/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/arch.h"

#if !defined(ALWAYS_ON_I486)
arch_type_t arch_type;
#endif

bool Executor::arch_init(void)
{
#if !defined(ALWAYS_ON_I486)
    uint32_t scratch1, scratch2;
    uint8_t i486_p;

    /* Adapted from _Assembly Language: For Real Programmers Only_ p. 561.
   * This code determines if we are on an i486 or higher.  We care because
   * those chips have the "bswap" instruction for byte swapping.
   */
    asm("pushfl\n\t"
        "pushfl\n\t"
        "popl %0\n\t"
        "xorl $0x40000,%0\n\t"
        "pushl %0\n\t"
        "popfl\n\t"
        "pushfl\n\t"
        "popl %1\n\t"
        "cmpl %0,%1\n\t"
        "jnz 1f\n\t"
        "xorl $0x40000,%0\n\t"
        "pushl %0\n\t"
        "popfl\n\t"
        "pushfl\n\t"
        "popl %1\n\t"
        "cmpl %0,%1\n"
        "1:\n\t"
        "setz %b2\n\t"
        "popfl"
        : "=r"(scratch1), "=r"(scratch2), "=abcd"(i486_p));

    arch_type = (i486_p ? ARCH_TYPE_I486 : ARCH_TYPE_I386);
#endif /* !ALWAYS_ON_I486 */

    return true;
}
