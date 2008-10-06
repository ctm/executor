#include "host_bltmacros.h"
#include "i386_djgpp_version.h"

#if !defined (USE_PORTABLE_SRCBLT)

#define CODE_BUFFER_SIZE 16384

#define RGNSTOP 32767

/* Offsets into our function pointer table for this transfer mode. */
#define REPEAT_0_FUNC		0	/* Just a "ret" */
#define	REPEAT_1_FUNC		4
#define	REPEAT_2_FUNC		8
#define	REPEAT_3_FUNC		12
#define	REPEAT_4_FUNC		16
#define REPEAT_MANY_FUNC_MOD_0	20
#define REPEAT_MANY_FUNC_MOD_1	24
#define MASK_FUNC		28

.lcomm	y,4
.lcomm	next_y,4
.lcomm	x2_long,4
.lcomm	row_offset,4
.lcomm	code_start,4


#define OFFSET_ESI_EDI_BY_EAX(AFTER)					\
	cmpl	code_start,%edi;					\
	jnz	5f;							\
	addl	%eax,_srcblt_src_baseaddr;				\
	addl	%eax,_srcblt_dst_baseaddr;				\
	shll	$3,%eax;	/* bytes -> bits */			\
	subl	%eax,_srcblt_x_offset;					\
	OFFSET_EXTRA;							\
	sarl	$5,%eax;	/* bits -> longs */			\
	subl	%eax,x2_long;						\
	jmp	AFTER;							\
	.align	4,0x90;							\
5:	movw	$0xC681,(%edi);		/* addl $0xNNNNNN,%esi */	\
	movl	%eax,8(%edi);						\
	movw	$0xC781,6(%edi);	/* addl $0xNNNNNN,%edi */	\
	movl	%eax,2(%edi);						\
	addl	%eax,row_offset;					\
	addl	$12,%edi

/* This blts a bitmap that's already been canonicalized.
 * Can be called from C.
 */
	.text
	.align	4,0x90
	.globl	_srcblt_bitmap
_srcblt_bitmap:

	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	/* Set up %ebp for function table. */
	movl	_srcblt_stub_table,%ebp

	/* Make sure the D flag is clear. */
	cld

	/* Set up our region pointer and shift count. */
	movl	_srcblt_rgn_start,%esi
	movb	_srcblt_log2_bpp,%cl

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
	cmpl	$RGNSTOP,%eax
	je	Ldone_with_scanlines
	movl	%eax,y	

Lstart_scanline:
	/* Grab the first start X. */
	movswl	(%esi),%eax

	/* Clear the mask. */
	xorl	%edx,%edx

	/* Reset the code pointer. */
	movl	code_start,%edi

	/* Reset row_offset. */
	movl	%edx,row_offset

	/* Have we hit the end of this scanline? */
	cmpl	$RGNSTOP,%eax
	je	Lfetch_next_y

	/* No, so scale by bpp and apply horizontal offset. */
	shll	%cl,%eax
	addl	_srcblt_x_offset,%eax

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
	addl	_srcblt_x_offset,%ebx
	addl	$4,%esi
	pushl	%ebx	/* save stop bit. */
	sarl	$5,%ebx
	movl	%ebx,x2_long

	/* See if start and stop are in different longs. */
	subl	%eax,%ebx
	je	Lsame_long

Ldifferent_longs:
	sall	$2,%eax
	subl	row_offset,%eax
	jz	Lno_offset1
#define OFFSET_EXTRA
	OFFSET_ESI_EDI_BY_EAX (Lno_offset1)
Lno_offset1:

	cmpl	$-1,%edx	/* Solid mask? */
	jne	Lnot_solid_mask

Lsolid_mask:
	/* don't blt full boundary long separately. */
	movl	%ebx,%edx
	jmp	Lblt_contig

	.align	4,0x90
Lnot_solid_mask:
	/* save repeat count for later. */
	pushl	%ebx

	/* First, blt boundary cruft. */
	movl	MASK_FUNC(%ebp),%ecx
	call	*%ecx

	/* Then blt any contiguous longs. */
	popl	%edx		/* restore repeat count */

	/* Account for the long we just did.  If there's nothing else,
	 * skip the rest of this code.
	 */
	decl	%edx
	jz	Lxfer_done	

	/* skip boundary long we just did. */
	movw	$0xC683,(%edi)		/* addl $0x4,%esi ; addl $0x4,%edi */
	movl	$0x04C78304,2(%edi)
	addl	$6,%edi
	addl	$4,row_offset

Lblt_contig:
	cmpl	$4,%edx
	jle	Lsmall_repeat

	movl	%edx,%ecx
	andl	$1,%ecx

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
	movb	_srcblt_log2_bpp,%cl

Lsame_long:
	/* Restore stop bit, compute new mask, and fetch next start bit. */
	popl	%ebx
	andl	$31,%ebx
	movswl	(%esi),%eax
	shll	%cl,%eax
	addl	_srcblt_x_offset,%eax
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
	movl	MASK_FUNC(%ebp),%ecx
	subl	row_offset,%eax
	jz	Lno_offset2
#undef OFFSET_EXTRA
#define OFFSET_EXTRA subl %eax,(%esp)
	OFFSET_ESI_EDI_BY_EAX (Lno_offset2)
Lno_offset2:
	call	*%ecx
	popl	%eax
	movb	_srcblt_log2_bpp,%cl

Lno_boundary_cruft:
	/* See if we've hit the end of the scanline. */
	cmpw	$RGNSTOP,(%esi)
	je	Lfetch_next_y
	movl	%eax,%ebx
	andl	$31,%ebx
	sarl	$5,%eax
	movl	_xdblt_mask_array(,%ebx,4),%edx
	jmp	Lnext_stop

	.align	4,0x90
Lfetch_next_y:
	/* Fetch the starting y value for the next scanline.  When we
	 * get here, %esi points to the RGNSTOP for the last scanline,
	 * so we have to look two bytes farther to find the next y
	 * value.  Special regions store y's as big endian.
	 */
	movw	2(%esi),%dx
	addl	$4,%esi
	rorw	$8,%dx
	movswl	%dx,%edx
	cmpl	$RGNSTOP,%edx
	je	Ldone_with_scanlines

	/* Save away the next y value. */
	movl	%edx,next_y

	/* If this is an empty scanline, don't loop, just move on. */
	cmpl	code_start,%edi
	je	Ldone_looping

	/* Compute the number of rows to blt. */
	subl	y,%edx		
	jle	Ldone_looping

	/* Output code to advance %esi and %edi to the next row. */
	movl	_srcblt_src_row_bytes,%eax
	cmpl	$0,_srcblt_reverse_scanlines_p
	jz	Lno_reverse_1
	negl	%eax
Lno_reverse_1:
	subl	row_offset,%eax
	jz	Lesi_done
	movw	$0xC681,(%edi)	/* addl $XXXXXXXX,%esi */
	movl	%eax,2(%edi)	/* write out %esi addl constant. */
	addl	$6,%edi
Lesi_done:
	movl	_srcblt_dst_row_bytes,%eax
	cmpl	$0,_srcblt_reverse_scanlines_p
	jz	Lno_reverse_2
	negl	%eax
Lno_reverse_2:
	subl	row_offset,%eax
	jz	Ledi_done
	movw	$0xC781,(%edi)	/* addl $XXXXXXXX,%edi */
	movl	%eax,2(%edi)	/* write out %edi addl constant. */
	addl	$6,%edi
Ledi_done:

	/* Output code to decl %edx ; jnz loop ; ret */
	movl	code_start,%eax	   	
	subl	%edi,%eax		/* jnz offset relative to end of jnz */
	movl	$0x00850F4A,(%edi)	/* decl %edx ; jnz XXXXXXXX 	     */
	subl	$7,%eax			/* jnz end at %edi+7		     */
	movb	$0xC3,7(%edi)		/* ret opcode			     */
	movl	%eax,3(%edi)		/* output jnz offset		     */
	/* Don't bother to advance %edi here. */
	
	/* Load the scanline starts into %esi and %edi. */
	cmpl	$0,_srcblt_reverse_scanlines_p
	jz	Lno_reverse
	movl	next_y,%edi
	decl	%edi
	jmp	Ledi_set_up
Lno_reverse:
	movl	y,%edi
Ledi_set_up:
	pushl	%esi	/* Save away the rgn ptr */
	movl	%edi,%esi
	imull	_srcblt_dst_row_bytes,%edi
	imull	_srcblt_src_row_bytes,%esi
	addl	_srcblt_dst_baseaddr,%edi
	addl	_srcblt_src_baseaddr,%esi

	/* Call the scanline blitting loop. */
	movl	code_start,%ebp

	/* Load shift offset. */
	movl	_srcblt_shift_offset,%ecx

#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
	/* Under DOS and friends, the screen may not be accessible
	 * with the default %ds/%es.  The code generated by the pattern
	 * blitter expects the source bits to be reference by %ds,
	 * and the dest bitmap to be referenced by %es.
	 */
	pushl	%es
	pushl	%ds
	DATA16 movw	_srcblt_dst_selector,%es
	DATA16 movw	_srcblt_src_selector,%ds
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */

	call	*%ebp

#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
	/* Restore proper selector values. */
	popl	%ds
	popl	%es
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */

	/* Put log2_bpp back in %cl (it got smashed). */
	movb	_srcblt_log2_bpp,%cl

	/* Restore our function table array. */
	movl	_srcblt_stub_table,%ebp

	/* Restore the region ptr. */
	popl	%esi

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

	.globl	_srcblt_nop
_srcblt_nop:
	ret


#include "src-blitters.s"

#endif /* !USE_PORTABLE_SRCBLT */
