//
//  Syn68k.c
//  CocoaExecutor
//
//  Created by C.W. Betts on 7/31/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#include <stdio.h>
#include "syn68k_public.h"

// "When in doubt, stub it out."
//    --Ryan C. Gordon, Steam Dev Days 2014

CPUState cpu_state = {0};

void initialize_68k_emulator (void (*while_busy)(int), int native_p,
									 uint32 trap_vector_storage[64],
									 uint32 dos_int_flag_addr)
{
	
}

#if defined (SYNCHRONOUS_INTERRUPTS)
void interrupt_generate (unsigned priority)
{
	
}

void interrupt_note_if_present (void)
{
	
}

syn68k_addr_t interrupt_process_any_pending (syn68k_addr_t pc)
{
	return 0;
}
#endif /* SYNCHRONOUS_INTERRUPTS */

void interpret_code (const uint16 *code)
{
	
}

const uint16 *hash_lookup_code_and_create_if_needed (syn68k_addr_t adr)
{
	return NULL;
}

unsigned long destroy_blocks (syn68k_addr_t low_m68k_address,
									 uint32 num_bytes)
{
	return 0;
}

syn68k_addr_t callback_install (callback_handler_t func,
									   void *arbitrary_argument)
{
	return 0;
}

void callback_remove (syn68k_addr_t m68k_address)
{
	
}

void trap_install_handler (unsigned trap_number,
								  callback_handler_t func,
								  void *arbitrary_argument)
{
	
}

void trap_remove_handler (unsigned trap_number)
{
	
}

void *callback_argument (syn68k_addr_t callback_address)
{
	return NULL;
}

callback_handler_t callback_function (syn68k_addr_t callback_address)
{
	return 0;
}

extern void dump_profile (const char *file)
{
	
}

uint16 callback_dummy_address_space[CALLBACK_SLOP] = {0};


#if SIZEOF_CHAR_P == 4
uint32 ROMlib_offset = 0;
#elif SIZEOF_CHAR_P == 8
uint64 ROMlib_offset = 0;
#endif

#if defined (CHECKSUM_BLOCKS)
unsigned long destroy_blocks_with_checksum_mismatch
(syn68k_addr_t low_m68k_address, uint32 num_bytes)
{
	return 0;
}
#endif

void m68kaddr (const uint16 *pc)
{
	
}
