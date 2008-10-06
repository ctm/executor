	.file "deintr.s"
.text
	.align 4
.globl	_dosevq_asm_begin
_dosevq_asm_begin:

# This computes BIOS keyflags in the style of int 16h, ah=12h, just by
# looking at the BIOS memory locations containing the modifier bits.	
_fetch_bios_keyflags:
	pushl %ds
	.byte 0x2E	# %cs prefix, for gas botch
	movw _dos_rm_selector,%ds
	xorl %eax,%eax
	movb 0x418,%ah
	movb %ah,%al
	andb $0x73,%ah
	shlb $5,%al
	andb $0x80,%al
	orb %al,%ah
	movb 0x496,%al
	andb $0x0C,%al
	orb %al,%ah
	movb 0x417,%al
	popl %ds
	ret	
			
.globl	_dosevq_kbd_int
_dosevq_kbd_int:
	# Save interrupt state
	pushfl
	pushal
	pushw %ds
	pushw %es
	pushw %fs
	pushw %gs

	# Set up %ds and %es for protected mode
	.byte 0x2E		# %cs prefix, for gas botch
	movw _dos_pm_interrupt_ds,%ds
	movw _dos_pm_interrupt_ds,%es	# No need for %cs here, %ds is OK

	# Fetch the current BIOS modifier flags and push them
	call _fetch_bios_keyflags
	pushl %eax
	
	# Fetch the key from the keyboard, and acknowledge it
	xorl %eax,%eax
	inb $0x60,%al
	pushl %eax
	inb $0x61,%al
	orb $0x80,%al
	outb %al,$0x61
	andb $0x7F,%al
	outb %al,$0x61

	# Push the event type (EVTYPE_RAWKEY)
	pushl $1

	# Enqueue the event	
	call _dosevq_enqueue
	addl $12,%esp

	# Restore interrupt state
	popw %gs
	popw %fs
	popw %es
	popw %ds
	popal
	nop	# Morten Welinder says this works around 80386 popal bug
	popfl

	# Jump down the chain
	.byte 0x2E	# %cs prefix, for gas botch
	ljmp _old_kbd_handler	# Contains selector:offset pair
	iret

	.align 4
.globl _dosevq_handle_mouse_callback
_dosevq_handle_mouse_callback:
	# Save interrupt state
	pushfl
	pushal
	pushw %ds
	pushw %es
	pushw %fs
	pushw %gs

	# Perform a "ret" for the caller by popping cs:ip from the
	# real mode stack pointer and sticking them in the cs:ip
	# fields of the register struct.  The normal way to pop the
	# ret address is to examine the stack via %ds:(%esi).
	# However, that seems to be broken under WinNT so we do it the
	# slightly harder way, by extracting out ss:sp from the
	# register struct and figuring out where the return address
	# is ourselves.  This isn't 100% the same because maybe %ds
	# (the theoretical selector for the real mode stack here)
	# doesn't refer to the same set of conventional memory as
	# dos_rm_selector.  Seems unlikely though.
	movzwl	%es:48(%edi),%eax	# fetch %ss
	movzwl	%es:46(%edi),%ebx	# fetch %sp
	shll	$4,%eax
	addl	%ebx,%eax
	.byte	0x2E			# %cs prefix, for gas botch
	movw	_dos_rm_selector,%ds
	movl	(%eax),%eax
	movl 	%eax,%es:42(%edi)		# jam ret address into cs:ip
	addw	$4,%es:46(%edi)		# increment sp

	# Push %di and %si
	movswl %es:(%edi),%eax
	pushl %eax
	movswl %es:4(%edi),%eax
	pushl %eax

	# Push %ax
	movzwl %es:28(%edi),%eax
	pushl %eax
	
	# Set up %ds and %es for protected mode
	.byte 0x2E		# %cs prefix, for gas botch
	movw _dos_pm_interrupt_ds,%ds
	movw _dos_pm_interrupt_ds,%es	# No need for %cs here, %ds is OK

	testb $6,%al	# See if the mouse button status changed
	jz 1f		# If not, don't bother computing key mod flags
	
	# Fetch the current BIOS modifier flags and push them
	call _fetch_bios_keyflags
1:	pushl %eax
	
	# Call the main mouse handler
	call _dosevq_handle_mouse
	addl $16,%esp
				
	# Restore state
	popw %gs
	popw %fs
	popw %es
	popw %ds
	popal
	nop	# Morten Welinder says this works around 80386 popal bug
	popfl
	
	iret

.globl	_dosevq_asm_end
_dosevq_asm_end:
	ret	# Avoid some ld weirdness Sandmann hinted at
