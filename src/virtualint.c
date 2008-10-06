/* Copyright 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_virtualint[] =
		    "$Id: virtualint.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#if defined (SYN68K)

#include "rsys/blockinterrupts.h"

void
do_virtual_interrupt (void)
{
  syn68k_addr_t pc;
#if defined (MSDOS)
  volatile unsigned short saved_es;
  
  asm volatile ("movw %%es,%0\n\t"
		"pushl %%ds\n\t"
		"popl %%es\n\t"
		"cld"
		: "=m" (saved_es));
#endif  

  pc = interrupt_process_any_pending (MAGIC_EXIT_EMULATOR_ADDRESS);
  if (pc != MAGIC_EXIT_EMULATOR_ADDRESS)
    {
      interpret_code (hash_lookup_code_and_create_if_needed (pc));
    }
 
#if defined (MSDOS)
  asm volatile ("movw %0,%%es"
		: : "m" (saved_es));
#endif
}

#endif /* SYN68K */
