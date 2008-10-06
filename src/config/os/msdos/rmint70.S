#include "i386_djgpp_version.h"

/* We copy this stub to real mode to handle INT 70h, the 1024 Hz timer. */

	.data
	.code16
.globl _rm_int_70_handler_start
_rm_int_70_handler_start:

	/* WARNING: these two must appear first, in this order,
	 * or syncint.c will do the wrong thing.
	 */
.globl	_rm_elapsed_1024_placeholder
_rm_elapsed_1024_placeholder:
	.long	0
.globl	_rm_remaining_1024_placeholder
_rm_remaining_1024_placeholder:
	.long	0

.globl _rm_int_70_handler_code_start
_rm_int_70_handler_code_start:
	pushfl
	pushl	%eax
	pushl	%ebx
	pushw	%ds

	.byte	0xB8		/* movw $nnnn,%ax */
.globl _rm_int_70_seg
_rm_int_70_seg:
	.byte	0,0
	movw	%ax,%ds

	xorl	%eax,%eax
	ADDR32 incl	(%eax)
	ADDR32 movl	4(%eax),%ebx		/* remaining_1024 */
	decl	%ebx
	js	1f			/* stop once <= 0. */
	ADDR32 movl	%ebx,4(%eax)
1:	popw	%ds
	popl	%ebx
	popl	%eax
	popfl
	
	.byte	0x66,0x68	/* pushl */
.globl _rm_int_70_chain_address
_rm_int_70_chain_address:
	.long	0
	.byte	0xCB	/* retf (using `lret' prepends a bad prefix byte) */

	.align	4,0x90
.globl _rm_int_70_handler_end
_rm_int_70_handler_end:
	nop
	.code32
