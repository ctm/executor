/* Each host that uses this has to provide this file.  It must
 * only contain preprocessor directives and comments...no C code.
 * It can #define:
 *
 * VGA_SCREEN_NEEDS_FAR_PTR   : different selector may be required.
 */
#include "host_bltmacros.h"
#include "i386_djgpp_version.h"

#if !defined (USE_PORTABLE_PATBLT)

#define CODE_BUFFER_SIZE 16384 /* if you change this, be sure to change
				  the TOUCH_PAGES code below */

#define RGN_STOP 32767

/* Offsets into our function pointer table for this transfer mode. */
#define REPEAT_0_FUNC		0	/* Just a "ret" */
#define	REPEAT_1_FUNC		4
#define	REPEAT_2_FUNC		8
#define	REPEAT_3_FUNC		12
#define	REPEAT_4_FUNC		16
#define REPEAT_MANY_FUNC_MOD_0	20
#define REPEAT_MANY_FUNC_MOD_1	24
#define REPEAT_MANY_FUNC_MOD_2	28
#define REPEAT_MANY_FUNC_MOD_3	32
#define MASK_FUNC		36
#define SETUP_PAT_FUNC		40
#define INIT_EBP_FUNC		44

/* These are temp variables. */
.lcomm	y,4
.lcomm	next_y,4
.lcomm	x2_long,4
.lcomm	edi_offset,4
.lcomm	code_start,4
.lcomm	pattern_row_bytes,4
.lcomm	pattern_row_bytes_minus_1,4


/* This blts a pattern that's already been canonicalized.
 * Can be called from C.
 */
	.text
	.align	4,0x90
	.globl	_xdblt_canon_pattern
_xdblt_canon_pattern:
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	/* Set up %ebp for function table. */
	movl	_xdblt_stub_table,%ebp

	/* Make sure the D flag is clear. */
	cld

	/* Set up pattern_row_bytes, pattern_row_bytes_minus_1. */
	movl	$1,%eax
	movl	_xdblt_log2_pattern_row_bytes,%ecx
	shll	%cl,%eax
	movl	%eax,pattern_row_bytes
	decl	%eax
	movl	%eax,pattern_row_bytes_minus_1

	/* Set up our region pointer and shift count. */
	movl	_xdblt_rgn_start,%esi
	movb	_xdblt_log2_bpp,%cl

	/* Set up a code buffer on the stack, aligned % 32.  Granted,
	 * we only need to align % 16 to make the i486 happy, but
	 * we might as well line the code up with Pentium cache lines.
	 */
	leal	31-CODE_BUFFER_SIZE(%esp),%edi
	subl	$CODE_BUFFER_SIZE,%esp	/* Make room for code on the stack. */
#if defined (TOUCH_PAGES)
#if CODE_BUFFER_SIZE != 16384
#error someone changed CODE_BUFFER_SIZE
#endif		
	movb    %cl, CODE_BUFFER_SIZE-4096(%esp)
	movb    %cl, CODE_BUFFER_SIZE-8192(%esp)
	movb    %cl, CODE_BUFFER_SIZE-12288(%esp)
	movb    %cl, CODE_BUFFER_SIZE-16384(%esp)
#endif	
	andl	$~31,%edi		/* Align code % 32.		    */
	movl	%edi,code_start

	/* Fetch first y value (special regions store y's as big endian.) */
	lodsw
	rorw	$8,%ax
	cwtl
	cmpl	$RGN_STOP,%eax
	je	Ldone_with_scanlines
	movl	%eax,y	

Lstart_scanline:
	/* Grab the first start X. */
	movswl	(%esi),%eax

	/* Clear the mask. */
	xorl	%edx,%edx

	/* Reset edi_offset. */
	movl	%edx,edi_offset

	/* Reset the code pointer. */
	movl	code_start,%edi

	/* Have we hit the end of this scanline? */
	cmpl	$RGN_STOP,%eax
	je	Lfetch_next_y

	/* No, so scale by bpp and apply horizontal offset. */
	shll	%cl,%eax
	addl	_xdblt_x_offset,%eax

	/* Fetch the initial boundary mask. */
	movl	%eax,%ebx
	andl	$31,%ebx
	sarl	$5,%eax
	movl	_xdblt_mask_array(,%ebx,4),%edx
	jmp	Lnext_stop

	.align	4,0x90
Lstill_same_long:
	andl	$31,%eax
	xorl	_xdblt_mask_array(,%eax,4),%edx
	movl	%ebx,%eax

Lnext_stop:	
	/* Fetch the next stop X. */
	movswl	2(%esi),%ebx
	shll	%cl,%ebx
	addl	_xdblt_x_offset,%ebx
	addl	$4,%esi
	pushl	%ebx	/* save stop bit. */
	sarl	$5,%ebx
	movl	%ebx,x2_long

	/* See if start and stop are in different longs. */
	subl	%eax,%ebx
	je	Lsame_long

Ldifferent_longs:
	sall	$2,%eax
	cmpl	$-1,%edx	/* Solid mask? */
	jne	Lnot_solid_mask

Lsolid_mask:
	/* don't blt full boundary long separately. */
	movl	%ebx,%edx
	subl	edi_offset,%eax
	jmp	Lblt_contig

	.align	4,0x90
Lnot_solid_mask:
	/* save repeat count for later. */
	pushl	%ebx

	/* First, blt boundary cruft. */
	movl	_xdblt_pattern_value,%ebx    /* Only legit for narrow short pat's. */
	subl	edi_offset,%eax
	movl	MASK_FUNC(%ebp),%ecx
	call	*%ecx

	/* Then blt any contiguous longs. */
	popl	%edx		/* restore repeat count */
	addl	$4,%eax		/* skip boundary long we just did. */
	decl	%edx		/* account for the boundary long we did. */

Lblt_contig:
	cmpl	$4,%edx
	movl	_xdblt_pattern_value,%ebx    /* Only legit for narrow short pat's. */
	jle	Lsmall_repeat

	movl	%edx,%ecx
	andl	$3,%ecx

	/* Call the routine to transfer many longs. */
	movl	REPEAT_MANY_FUNC_MOD_0(%ebp,%ecx,4),%ecx
	pushl	$Lxfer_done	/* Phony call. */
	jmp	*%ecx

	.align	4,0x90
Lsmall_repeat:
	movl	REPEAT_0_FUNC(%ebp,%edx,4),%ecx
	call	*%ecx
Lxfer_done:

	/* Start over. */
	movl	$-1,%edx

	/* Put log2_bpp back in %cl (it got smashed). */
	movb	_xdblt_log2_bpp,%cl

Lsame_long:
	/* Restore stop bit, compute new mask, and fetch next start bit. */
	popl	%ebx
	andl	$31,%ebx
	movswl	(%esi),%eax
	shll	%cl,%eax
	addl	_xdblt_x_offset,%eax
	xorl	_xdblt_mask_array(,%ebx,4),%edx
	movl	%eax,%ebx
	sarl	$5,%ebx
	cmpl	x2_long,%ebx
	je	Lstill_same_long

	/* If our mask is zero, don't touch the extra memory, because
	 * it might be out of bounds.
	 */
	testl	%edx,%edx
	je	Lno_boundary_cruft

	/* Blt boundary cruft. */
	movl	x2_long,%ebx
	pushl	%eax
	leal	(,%ebx,4),%eax
	movl	_xdblt_pattern_value,%ebx    /* Only legit for narrow short pat's. */
	movl	MASK_FUNC(%ebp),%ecx
	subl	edi_offset,%eax
	call	*%ecx
	popl	%eax
	movb	_xdblt_log2_bpp,%cl

Lno_boundary_cruft:
	/* See if we've hit the end of the scanline. */
	cmpw	$RGN_STOP,(%esi)
	je	Lfetch_next_y
	movl	%eax,%ebx
	andl	$31,%ebx
	sarl	$5,%eax
	movl	_xdblt_mask_array(,%ebx,4),%edx
	jmp	Lnext_stop

	.align	4,0x90
Lfetch_next_y:
	/* Fetch the starting y value for the next scanline.  When we
	 * get here, %esi points to the RGN_STOP for the last scanline,
	 * so we have to look two bytes farther to find the next y
	 * value.  Special regions store y's as big endian.
	 */
	movw	2(%esi),%dx
	addl	$4,%esi
	rorw	$8,%dx
	movswl	%dx,%edx
	cmpl	$RGN_STOP,%edx
	je	Ldone_with_scanlines

	/* Save away the next y value. */
	movl	%edx,next_y

	/* If this is an empty scanline, don't loop, just move on. */
	cmpl	code_start,%edi
	je	Ldone_looping

	/* Compute the number of rows to blt. */
	subl	y,%edx		
	jle	Ldone_looping

	/* Output code to advance %edi to the next row. */
	movl	_xdblt_dst_row_bytes,%eax
	movw	$0xC781,(%edi)	/* addl $XXXXXXXX,%edi */
	subl	edi_offset,%eax
	movl	%eax,2(%edi)	/* write out addl constant. */
	addl	$6,%edi

	/* Output code to advance %ebp (the pointer to the beginning
	 * of the current row) and set up %eax (the pattern value for the
	 * current row, for "narrow" patterns).  These may not be needed
	 * by some transfer modes; for those, this function just does a
	 * "ret".
	 */
	movl	SETUP_PAT_FUNC(%ebp),%ecx	
	call	*%ecx

	/* Output code to decl %edx ; jnz loop ; ret */
	movl	code_start,%eax	   	
	movl	$0x00850F4A,(%edi)	/* decl %edx ; jnz XXXXXXXX 	     */
	subl	%edi,%eax		/* jnz offset relative to end of jnz */
	subl	$7,%eax			/* jnz end at %edi+7		     */
	movb	$0xC3,7(%edi)		/* ret opcode			     */
	movl	%eax,3(%edi)		/* output jnz offset		     */
	/* Don't bother to advance %edi here. */

	movl	y,%edi
	imull	_xdblt_dst_row_bytes,%edi
	addl	_xdblt_dst_baseaddr,%edi

	/* Save our function table array ptr and load the appropriate
	 * pattern pointer into %ebp.  For short patterns %ebp may just
	 * point to `_xdblt_pattern_value'.
	 */
	movl	INIT_EBP_FUNC(%ebp),%ecx
	call	*%ecx

	/* Set up %ecx for the call. */
	movl	code_start,%ecx

	/* Move the pattern value into %eax (this value may go unused;
	 * it's not a legitimate value for wide or tall patterns).
	 */
	movl	(%ebp),%eax

#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
	/* Under DOS and friends, the screen may not be accessible
	 * with the default %ds.  The code generated by the pattern
	 * blitter expects the screen to be referenced by %ds/%es, and
	 * does _NOT_ use any other runtime variables stored in the
	 * normal %ds.  Instead, when it needs information from the
	 * %ds address space it uses %ebp, which implicitly uses %ss.
	 * This way we avoid all segment overrides and the associated
	 * performance penalties.
	 */
	pushl	%es
	pushl	%ds
	DATA16 movw	_xdblt_dst_selector,%es
	DATA16 movw	_xdblt_dst_selector,%ds
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */

	/* Actually call the scanline blitting loop. */
	call	*%ecx

#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
	/* Restore proper selector values. */
	popl	%ds
	popl	%es
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */

	/* Put log2_bpp back in %cl (it got smashed). */
	movb	_xdblt_log2_bpp,%cl

	/* Restore our function table array. */
	movl	_xdblt_stub_table,%ebp

Ldone_looping:
	/* Move next_y into the current y, and advance to the next scanline. */
	movl	next_y,%eax
	movl	%eax,y
	jmp	Lstart_scanline	

	.align	4,0x90
Ldone_with_scanlines:
	addl	$CODE_BUFFER_SIZE,%esp

	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp

	ret

	.globl	_xdblt_nop
_xdblt_nop:
	ret

	.align	4,0x90
	.globl	_xdblt_short_init_ebp
_xdblt_short_init_ebp:	
	movl	_xdblt_pattern_baseaddr,%ebp
	ret

	.align	4,0x90
	.globl	_xdblt_tall_narrow_init_ebp
_xdblt_tall_narrow_init_ebp:
	movl	y,%ebp
	addl	_xdblt_pattern_row_0,%ebp
	andl	_xdblt_pattern_height_minus_1,%ebp
	shll	$2,%ebp
	addl	_xdblt_pattern_baseaddr,%ebp
	ret

	.align	4,0x90
	.globl	_xdblt_tall_wide_init_ebp
_xdblt_tall_wide_init_ebp:
	movl	y,%ebp
	movl	_xdblt_log2_pattern_row_bytes,%ecx
	addl	_xdblt_pattern_row_0,%ebp
	andl	_xdblt_pattern_height_minus_1,%ebp
	shll	%cl,%ebp
	addl	_xdblt_pattern_baseaddr,%ebp
	ret

#if 0
	.align	4,0x90
_xdblt_copy_32:
	movl	%eax,0(%edi)
_xdblt_copy_31:
	movl	%eax,4(%edi)
_xdblt_copy_30:
	movl	%eax,8(%edi)
_xdblt_copy_29:
	movl	%eax,12(%edi)
_xdblt_copy_28:
	movl	%eax,16(%edi)
_xdblt_copy_27:
	movl	%eax,20(%edi)
_xdblt_copy_26:
	movl	%eax,24(%edi)
_xdblt_copy_25:
	movl	%eax,28(%edi)
_xdblt_copy_24:
	movl	%eax,32(%edi)
_xdblt_copy_23:
	movl	%eax,36(%edi)
_xdblt_copy_22:
	movl	%eax,40(%edi)
_xdblt_copy_21:
	movl	%eax,44(%edi)
_xdblt_copy_20:
	movl	%eax,48(%edi)
_xdblt_copy_19:
	movl	%eax,52(%edi)
_xdblt_copy_18:
	movl	%eax,56(%edi)
_xdblt_copy_17:
	movl	%eax,60(%edi)
_xdblt_copy_16:
	movl	%eax,64(%edi)
_xdblt_copy_15:
	movl	%eax,68(%edi)
_xdblt_copy_14:
	movl	%eax,72(%edi)
_xdblt_copy_13:
	movl	%eax,76(%edi)
_xdblt_copy_12:
	movl	%eax,80(%edi)
_xdblt_copy_11:
	movl	%eax,84(%edi)
_xdblt_copy_10:
	movl	%eax,88(%edi)
_xdblt_copy_9:
	movl	%eax,92(%edi)
_xdblt_copy_8:
	movl	%eax,96(%edi)
_xdblt_copy_7:
	movl	%eax,100(%edi)
_xdblt_copy_6:
	movl	%eax,104(%edi)
_xdblt_copy_5:
	movl	%eax,108(%edi)
_xdblt_copy_4:
	movl	%eax,112(%edi)
_xdblt_copy_3:
	movl	%eax,116(%edi)
_xdblt_copy_2:
	movl	%eax,120(%edi)
_xdblt_copy_1:
	movl	%eax,124(%edi)

	addl	$128,%edi
	decl	%ecx
	jnz	_xdblt_copy_32
	ret
#endif

#include "pat-blitters.s"

#endif /* !USE_PORTABLE_PATBLT */
