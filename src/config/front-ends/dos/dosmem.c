/* Copyright 1995 - 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "dosmem.h"
#include "dpmilock.h"
#include "rsys/blockinterrupts.h"
#include <go32.h>
#include <dpmi.h>
#include <sys/exceptn.h>

/* Use this memory for temporary stack during calls to DOS memory.
 * DOS_STACK_TOP is the real mode offset you should use.
 */
uint16_t dos_buf_segment;
uint16_t dos_buf_selector;

/* This selector refers to DOS conventional memory. */
uint16_t dos_rm_selector;

/* This is our %ds selector.  We copy %ds to %ss right away in main. */
uint16_t dos_pm_ds;

/* This is an alias for %ds that will always be valid, even when
 * ctrl-break is hit.  We use this at interrupt time.
 */
uint16_t dos_pm_interrupt_ds;

/* This is our %cs selector. */
uint16_t dos_pm_cs;

/* Attempts to allocate a block of DOS memory of size SIZE which does
 * not span two 64K banks.  We need to make this guarantee so that calls
 * to floppy disk BIOS routines will work (DMA hardware requires
 * things not to cross 64K boundaries).  Returns true iff successful,
 * else false.
 */
static bool
alloc_single_bank_dos_block(int size, uint16_t *segp, uint16_t *selp)
{
    int selectors_to_free[16], i, j, sel, seg;
    int paragraphs;
    bool success_p;

    if(size > 64 * 1024)
        abort();

    paragraphs = (size + 15) / 16;

    /* Default values. */
    seg = sel = -1;
    success_p = false;

    /* Allocate a bunch of selectors until we happen to get one that
   * doesn't cross a 64K mark.
   */
    memset(selectors_to_free, -1, sizeof selectors_to_free);
    for(i = 0; i < NELEM(selectors_to_free); i++)
    {
        warning_trace_info(NULL_STRING);

        seg = __dpmi_allocate_dos_memory(paragraphs, &sel);

        /* Did we run out of memory? */
        if(seg == -1)
            break;

        /* Make sure the start and end addresses are in the same 64K
       * bank.  Since these are segment values, they represent the
       * linear address divided by 16.
       */
        if((seg >> 12) == ((seg + paragraphs - 1) >> 12))
        {
            success_p = true;
            warning_trace_info(NULL_STRING);
            break;
        }

        selectors_to_free[i] = sel;
        warning_trace_info(NULL_STRING);
    }

    /* Free up all the failed selectors. */
    for(j = 0; j < i; j++)
    {
        int s;
        s = selectors_to_free[j];
        gui_assert(s != -1 && (!success_p || s != sel));
        __dpmi_free_dos_memory(s);
    }

    /* Record the successful selector, if there is one. */
    if(!success_p)
        *segp = *selp = 0;
    else
    {
        gui_assert(seg != -1 && sel != -1);
        *segp = seg;
        *selp = sel;
    }

    return success_p;
}

bool init_dos_memory(void)
{
    asm("movw %%cs,%0"
        : "=g"(dos_pm_cs));
    asm("movw %%ds,%0"
        : "=g"(dos_pm_ds));
    dos_rm_selector = _go32_info_block.selector_for_linear_memory;
    dos_pm_interrupt_ds = __djgpp_ds_alias;

    /* Allocate a DOS buffer. */
    if(!alloc_single_bank_dos_block(DOS_BUF_SIZE, &dos_buf_segment,
                                    &dos_buf_selector))
        return false;

    /* Wire these down so we can use them at interrupt time. */
    dpmi_lock_memory(&dos_rm_selector, sizeof dos_rm_selector);
    dpmi_lock_memory(&dos_pm_ds, sizeof dos_pm_ds);
    dpmi_lock_memory(&dos_pm_interrupt_ds, sizeof dos_pm_interrupt_ds);
    dpmi_lock_memory(&dos_pm_cs, sizeof dos_pm_cs);

    /* NOTE: we can't free our DOS memory at atexit time since we use it
   * in other shutdown routines.
   */

    /* Let __dpmi_int (heavily used by libc) use our extra DOS stack
   * space, just to be safe.
   */
    __dpmi_int_ss = dos_buf_segment;
    __dpmi_int_sp = DOS_STACK_TOP;

    return true;
}
