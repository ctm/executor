#include "dosevq_defs.h"
#include "i386_djgpp_version.h"

	.file "deintr.S"
.text
	.align 4
.globl	_dosevq_asm_begin
_dosevq_asm_begin:

/* This computes BIOS keyflags in the style of int 16h, ah=12h, just by 
 * looking at the BIOS memory locations containing the modifier bits.
 * The bizarre %esi hack is there so gas doesn't misassemble
 * the 16-bit instantiation of this code.
 */

#define FETCH_BIOS_KEYFLAGS_CORE(esi_val, prefix)	\
	pushl %esi		;		\
	movl $esi_val,%esi	;		\
	prefix movb 0x18(%esi),%ah	;		\
	movb %ah,%al		;		\
	andb $0x73,%ah		;		\
	shlb $5,%al		;		\
	andb $0x80,%al		;		\
	orb %al,%ah		;		\
	prefix movb 0x96(%esi),%al	;		\
	andb $0x0C,%al		;		\
	orb %al,%ah		;		\
	prefix movb 0x17(%esi),%al	;		\
	popl %esi

_fetch_bios_keyflags:
	pushl %ds
	.byte 0x2E	/* %cs prefix, for gas botch */
	DATA16 movw _dos_rm_selector,%ds
	xorl %eax,%eax
	FETCH_BIOS_KEYFLAGS_CORE(0x400,)
	popl %ds
	ret	
			
.globl	_dosevq_kbd_int
_dosevq_kbd_int:
	/* Save interrupt state */
	pushfl
	pushal
	pushw %ds
	pushw %es
	pushw %fs
	pushw %gs

	/* Set up %ds and %es for protected mode */
	.byte 0x2E		/* %cs prefix, for gas botch */
	DATA16 movw _dos_pm_interrupt_ds,%ds
	DATA16 movw _dos_pm_interrupt_ds,%es	/* No need for %cs here, %ds is OK */

	/* Fetch the current BIOS modifier flags and push them */
	call _fetch_bios_keyflags
	pushl %eax
	
	/* Fetch the key from the keyboard, and acknowledge it */
	xorl %eax,%eax
	inb $0x60,%al
	pushl %eax
	inb $0x61,%al
	orb $0x80,%al
	outb %al,$0x61
	andb $0x7F,%al
	outb %al,$0x61

	/* Push the event type (EVTYPE_RAWKEY) */
	pushl $1

	/* Enqueue the event	 */
	call _dosevq_enqueue
	addl $12,%esp

	/* Restore interrupt state */
	popw %gs
	popw %fs
	popw %es
	popw %ds
	popal
	nop	/* Morten Welinder says this works around 80386 popal bug */
	popfl

	/* Jump down the chain */
	.byte 0x2E	/* %cs prefix, for gas botch */
	ljmp _old_kbd_handler	/* Contains selector:offset pair */
	iret

#define QUEUE_OFFSET (__dos_event_queue_start-_dos_mouse_stub)

	.globl	_dosevq_qelt
_dosevq_qelt:
	movl	4(%esp),%eax
	DATA16 movw	_mouse_handler_and_queue_sel,%fs
	.byte	0x64	/* %fs prefix, for gas botch. */
	movl	(QUEUE_OFFSET+12)(,%eax,4),%eax
	ret

.globl	_dosevq_set_qelt
_dosevq_set_qelt:
	movl	4(%esp),%eax
	movl	8(%esp),%ecx
	DATA16 movw	_mouse_handler_and_queue_sel,%fs
	.byte	0x64	/* %fs prefix, for gas botch. */
	movl	%ecx,(QUEUE_OFFSET+12)(,%eax,4)
	ret


	/* This code is not called directly; rather, it is copied
	 * to real mode and run there.
	 */
	DOT_ALIGN (2)
.globl _dos_mouse_stub
_dos_mouse_stub:
	.code16

	pushfl
	pushal
	pushw	%ds

	/* Move the proper segment value into %ds. */
	.byte	0xBB	/* movw $nnnn,%bx opcode */
.globl _dos_mouse_stub_segment
_dos_mouse_stub_segment:
	.byte	0x34, 0x12
	movw	%bx,%ds

	/* Move a pointer to the queue structure into %ebx. */
	movl	$(__dos_event_queue_start - _dos_mouse_stub),%ebx

	/* Did the mouse move? */
	testb	$MOUSE_MOTION_MASK,%al

	DATA32 je	check_button_pressed

	/* Compute mouse deltas. */
	ADDR32 movw	-4(%ebx),%cx
	ADDR32 movw	%si,-4(%ebx)
	subw	%cx,%si
	ADDR32 movw	-2(%ebx),%cx
	ADDR32 movw	%di,-2(%ebx)
	subw	%cx,%di

	/* Offset currently known mouse position. */
	ADDR32 addw	%si,4(%ebx)
	ADDR32 addw	%di,6(%ebx)

	/* Note that the mouse has moved. */
	ADDR32 movb	$1,8(%ebx)
	
/* Enqueues an event whose `type' and `which' are in %dx.  %bx pts to queue. */
#define ENQUEUE_DX							\
	pushl	%eax	;						\
									\
	/* Compute the keyflags. */					\
	pushw	%ds	;						\
	movw	$0x40,%ax ;						\
	movw	%ax,%ds	;						\
	xorw	%ax,%ax	;						\
	FETCH_BIOS_KEYFLAGS_CORE(0,ADDR32)	;				\
	popw	%ds	;						\
									\
	/* Jam keyflags into the high two bytes of %edx. */		\
	rorl	$16,%edx	;					\
	movw	%ax,%dx		;					\
	rorl	$16,%edx	;					\
									\
	ADDR32 movzwl	(%ebx),%eax	; /* qhead */				\
	movw	%ax,%di		; /* next_qhead */			\
	incw	%di		;					\
	andw	$(DOSEVQ_QUEUE_SIZE-1),%di	;			\
	ADDR32 cmpw	%di,2(%ebx) ;	/* compare next_qhead to qtail */	\
	DATA32 je	5f	;	/* queue full! */			\
									\
	/* Queue has room, so stick the event in the queue. */		\
	ADDR32 movl	%edx,12(%ebx,%eax,4)	;				\
	ADDR32 movw	%di,(%ebx) ;	/* update qhead */			\
									\
	/* Note that an event has been posted. */			\
	ADDR32 movb	$1,9(%ebx) ;						\
5:									\
	popl	%eax

check_button_pressed:
	testb	$MOUSE_LEFT_PRESSED_MASK,%al
	DATA32 je	check_button_released
	movw	$(EVTYPE_MOUSE_DOWN | (1 << 8)),%dx
	ENQUEUE_DX

check_button_released:	
	testb	$MOUSE_LEFT_RELEASED_MASK,%al
	DATA32 je	done
	movw	$(EVTYPE_MOUSE_UP | (0 << 8)),%dx
	ENQUEUE_DX

done:
	popw	%ds
	popal
	nop	/* Morten Welinder says this works around 80386 popal bug */
	popfl
	.byte	0xCB	/* retf (using `lret' prepends a bad prefix byte) */
	
	
	DOT_ALIGN (2)
/* These two variables must immediately precede the queue in this order! */
old_mickey_x:	.word	0
old_mickey_y:	.word	0
.globl __dos_event_queue_start
__dos_event_queue_start:
	.word	0,0	/* qhead, qtail */
	.word	0,0	/* mouse_dx, mouse_dy */
	.long	0	/* interrupt_pending_mask */
end_of_stub_stuff:
	.code32

.globl _dos_mouse_stub_bytes_to_copy
_dos_mouse_stub_bytes_to_copy:
	.long	(end_of_stub_stuff - _dos_mouse_stub)
		
.globl	_dosevq_asm_end
_dosevq_asm_end:
	ret	/* Avoid some ld weirdness Sandmann hinted at */
