/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/depthconv.h"
#include "rsys/cquick.h"

namespace Executor
{

/* This file contains all of the low-level depth conversion routines,
 * and works hand in hand with dcmaketables.c.  The routines in this
 * file can handle mapping any type of indirect or RGB pixel to any
 * type of indirect or RGB pixel.
 *
 * None of the routines in this file should ever be called directly;
 * instead, you call one of the table-creating routines in
 * dcmaketables.c, and that routine will return a function pointer
 * that points to the appropriate function in this file.
 *
 * All of the depth conversion functions share the same structure; the
 * key thing that differs is the actual loop that perform the
 * translation.  Therefore each depth conversion function is created
 * by instantating the generic depth conversion macro with appropriate
 * parameters.
 *
 * The most important such parameter is the "convert" macro.  This
 * supplies code for the inner loop that converts source pixels to
 * destination pixels.  Since this loop is time-critical, the generic
 * C versions of these inner loops can be overridden with inline
 * assembly versions; each generic C loop is wrapped in an "#ifndef"
 * so any preceding host-specific version will override it.  Currently
 * several of the most common inner loops have been written in x86
 * assembly.  These cases are especially time-critical since they are
 * frequently used to convert data into a format used by the host
 * windowing system.
 */

#define CONVERT_FUNC(func_name, convert, log2_out_ratio, abs_log2_out_ratio,        \
                     compute_add, table_type)                                       \
    void                                                                            \
    func_name(const void *raw_table,                                                \
              const uint8 *src_base, int src_row_bytes,                             \
              uint8 *dst_base, int dst_row_bytes,                                   \
              int top, int left, int bottom, int right)                             \
    {                                                                               \
        int byte_width, height;                                                     \
        int left_byte, right_byte, in_add, out_add;                                 \
        int log2_in_bpp, log2_out_bpp;                                              \
        IF_TABLE(const table_type *table);                                          \
                                                                                    \
        EXTRA_DECLS;                                                                \
                                                                                    \
        /* Grab log2_in_bpp from the supplied table. */                             \
        log2_in_bpp = *(const uint32_t *)raw_table;                                 \
        log2_out_bpp = log2_in_bpp + log2_out_ratio;                                \
                                                                                    \
        /* Line up incoming data on byte boundaries. */                             \
        left_byte = (left << log2_in_bpp) >> 3;                                     \
        right_byte = ((right << log2_in_bpp) + 7) >> 3;                             \
                                                                                    \
        /* Line up the outgoing data on byte boundaries. */                         \
        if(log2_out_ratio < 0)                                                      \
        {                                                                           \
            left_byte &= ~((1L << (abs_log2_out_ratio)) - 1);                       \
            right_byte = ((right_byte + ((1L << (abs_log2_out_ratio)) - 1))         \
                          & ~((1L << (abs_log2_out_ratio)) - 1));                   \
        }                                                                           \
                                                                                    \
        byte_width = right_byte - left_byte;                                        \
        height = bottom - top;                                                      \
                                                                                    \
        if(byte_width <= 0 || height <= 0)                                          \
            return;                                                                 \
                                                                                    \
        /* Compute the actual aligned base of the table. */                         \
        IF_TABLE(table = ((const table_type *)                                      \
                              _ALIGN_TABLE(raw_table, log2_in_bpp, log2_out_bpp))); \
                                                                                    \
        EXTRA_SETUP();                                                              \
                                                                                    \
        /* Compute the quantities to add as we march through each row. */           \
        in_add = compute_add(src_row_bytes, byte_width);                            \
        if(log2_out_ratio >= 0)                                                     \
            out_add = compute_add(dst_row_bytes,                                    \
                                  byte_width << (abs_log2_out_ratio));              \
        else                                                                        \
            out_add = compute_add(dst_row_bytes,                                    \
                                  byte_width >> (abs_log2_out_ratio));              \
                                                                                    \
        {                                                                           \
            INP_OUTP_DECL;                                                          \
                                                                                    \
            /* Compute pointers to the beginning of the transfer areas. */          \
            inp = src_base + (top * src_row_bytes) + left_byte;                     \
            if(log2_out_ratio >= 0)                                                 \
                outp = (dst_base + (top * dst_row_bytes)                            \
                        + (left_byte << (abs_log2_out_ratio)));                     \
            else                                                                    \
                outp = (dst_base + (top * dst_row_bytes)                            \
                        + (left_byte >> (abs_log2_out_ratio)));                     \
                                                                                    \
            for(; height > 0; height--)                                             \
            {                                                                       \
                convert();                                                          \
                inp += in_add;                                                      \
                outp += out_add;                                                    \
            }                                                                       \
        }                                                                           \
                                                                                    \
        EXTRA_CLEANUP();                                                            \
    }

#if defined(i386)

/* Override some of the common conversion cases with inline assembly
 * hand-scheduled for the Pentium.
 */

#define INP_OUTP_DECL                     \
    register const uint8 *inp asm("%si"); \
    register uint8 *outp asm("%di")

/* This can do unaligned movsl's, but it's not clear who would ever
 * call this anyway, so I'm not going to worry about it.
 */
#define CONVERT_COPY()                                                      \
    ({                                                                      \
        decltype(byte_width) byte_width_dregs_unused;                       \
                                                                            \
        asm volatile("cld\n\t"                                              \
                     "movl %%ecx,%%eax\n\t"                                 \
                     "shrl $2,%%ecx\n\t"                                    \
                     "andb $3,%%al\n\t"                                     \
                     "rep\n\t"                                              \
                     "movsl\n\t"                                            \
                     "movb %%al,%%cl\n\t" /* %ecx high 3 bytes == 0. */     \
                     "rep\n\t"                                              \
                     "movsb"                                                \
                     : "=S"(inp), "=D"(outp), "=c"(byte_width_dregs_unused) \
                     : "0"(inp), "1"(outp), "2"(byte_width)                 \
                     : "ax", "memory", "cc");                               \
    })

#define CONVERT_1_1()                                                        \
    /* This conversion loop has been heavily optimized for the Pentium       \
     * and 80486.  Pointers to the conversion table (which is aligned        \
     * % 256 bytes) are stored in %eax, %ebx, %ecx, and %edx.  The low byte  \
     * of each of these pointers is replaced with the input byte; the        \
     * resulting register value is a pointer to the appropriate table entry. \
     */                                                                      \
    ({                                                                       \
        int smashed_ax_unused;                                               \
                                                                             \
        asm volatile("pushl %%ebx\n\t"                                       \
                     "pushl %%ebp\n\t"                                       \
                     "movl %5,%%ebp\n\t"                                     \
                     "movl %%eax,%%ebx\n\t"                                  \
                     "testl $7,%%ebp\n\t"                                    \
                     "movl %%eax,%%ecx\n\t"                                  \
                     "movl %%eax,%%edx\n\t"                                  \
                     "jz 1f\n"                                               \
                                                                             \
                     /* Cleanup loop; handle cruft until we get an even      \
                      * multiple of 8 bytes left to process.                 \
                      */                                                     \
                     "5:\n\t"                                                \
                     "movb (%%esi),%%al\n\t"                                 \
                     "decl %%ebp\n\t"                                        \
                     "incl %%esi\n\t"                                        \
                     "movb (%%eax),%%al\n\t"                                 \
                     "testl $7,%%ebp\n\t"                                    \
                     "movb %%al,(%%edi)\n\t"                                 \
                     "leal 1(%%edi),%%edi\n"                                 \
                     "jnz 5b\n"                                              \
                                                                             \
                     "1:\n\t"                                                \
                     "shrl $3,%%ebp\n\t"                                     \
                     "jnz 2f\n\t"                                            \
                     "jmp 3f\n\t"                                            \
                                                                             \
                     /* Here's the main conversion loop.  We stagger         \
                      * byte fetches by 4 bytes to avoid cache bank          \
                      * conflicts which would prohibit Pentium               \
                      * pairability.  16 bytes of consecutive memory         \
                      * operations will probably stall the 80486             \
                      * prefetch queue, so I've tried to move some of        \
                      * the necessary register operations into the           \
                      * middle of the loop.                                  \
                      */                                                     \
                                                                             \
                     /* Read in four input bytes. */                         \
                     ".align 4,0x90\n"                                       \
                     "2:\n\t"                                                \
                     "movb (%%esi),%%al\n\t"                                 \
                     "movb 4(%%esi),%%bl\n\t"                                \
                     "movb 1(%%esi),%%cl\n\t"                                \
                     "movb 5(%%esi),%%dl\n\t"                                \
                                                                             \
                     /* Map them to the output bytes (we will probably       \
                      * have cache bank conflicts here since %%eax           \
                      * probably == %%ebx, etc).                             \
                      */                                                     \
                     "movb (%%eax),%%al\n\t"                                 \
                     "movb (%%ebx),%%bl\n\t"                                 \
                     "movb (%%ecx),%%cl\n\t"                                 \
                     "movb (%%edx),%%dl\n\t"                                 \
                                                                             \
                     /* Write them out to memory. */                         \
                     "decl %%ebp\n\t"                                        \
                     "movb %%al,(%%edi)\n\t"                                 \
                     "movb %%bl,4(%%edi)\n\t"                                \
                     "movb %%cl,1(%%edi)\n\t"                                \
                     "movb %%dl,5(%%edi)\n\t"                                \
                                                                             \
                     /* Read in four input bytes. */                         \
                     "leal 8(%%edi),%%edi\n\t" /* don't touch decl cc's. */  \
                     "movb 2(%%esi),%%al\n\t"                                \
                     "movb 6(%%esi),%%bl\n\t"                                \
                     "movb 3(%%esi),%%cl\n\t"                                \
                     "movb 7(%%esi),%%dl\n\t"                                \
                                                                             \
                     /* Map them to the output bytes. */                     \
                     "leal 8(%%esi),%%esi\n\t" /* don't touch decl cc's. */  \
                     "movb (%%eax),%%al\n\t"                                 \
                     "movb (%%ebx),%%bl\n\t"                                 \
                     "movb (%%ecx),%%cl\n\t"                                 \
                     "movb (%%edx),%%dl\n\t"                                 \
                                                                             \
                     /* Write them out to memory. */                         \
                     "movb %%al,-6(%%edi)\n\t"                               \
                     "movb %%bl,-2(%%edi)\n\t"                               \
                     "movb %%cl,-5(%%edi)\n\t"                               \
                     "movb %%dl,-1(%%edi)\n\t"                               \
                                                                             \
                     /* Loop while there's still more work to do. */         \
                     "jnz 2b\n"                                              \
                                                                             \
                     /* All done! */                                         \
                     "3:\n\t"                                                \
                     "popl %%ebp\n\t"                                        \
                     "popl %%ebx\n\t"                                        \
                     : "=S"(inp), "=D"(outp), "=a"(smashed_ax_unused)        \
                     : "0"(inp), "1"(outp), "g"(byte_width),                 \
                       "2"(table)                                            \
                     : "memory", "cx", "dx", "cc");                          \
    })

#define CONVERT_1_2()                                                   \
    ({                                                                  \
        int smashed_ax, smashed_cx;                                     \
                                                                        \
        asm volatile("pushl %%ebx\n\t"                                  \
                     "pushl %%ebp\n\t"                                  \
                     "xorl %%ebx,%%ebx\n\t"                             \
                     "xorl %%edx,%%edx\n\t"                             \
                                                                        \
                     "testb $3,%%cl\n\t"                                \
                     "jz 1f\n\t"                                        \
                     "jmp 5f\n\t"                                       \
                                                                        \
                     /* Cleanup loop; handle cruft until we get an even \
                      * multiple of 4 bytes left to process.            \
                      */                                                \
                     ".align 4,0x90\n"                                  \
                     "5:\n\t"                                           \
                     "movb (%%esi),%%bl\n\t"                            \
                     "decl %%ecx\n\t"                                   \
                     "incl %%esi\n\t"                                   \
                     "movl (%%eax,%%ebx,8),%%ebp\n\t"                   \
                     "testb $3,%%cl\n\t"                                \
                     "movw %%bp,(%%edi)\n\t"                            \
                     "leal 2(%%edi),%%edi\n\t"                          \
                     "jnz 5b\n"                                         \
                                                                        \
                     "1:\n\t"                                           \
                     "shrl $2,%%ecx\n\t"                                \
                     "jnz 2f\n\t"                                       \
                     "jmp 3f\n\t"                                       \
                                                                        \
                     ".align 4,0x90\n"                                  \
                     "2:\n\t"                                           \
                     "movb (%%esi),%%bl\n\t"                            \
                     "movb 1(%%esi),%%dl\n\t"                           \
                     "movl (%%eax,%%ebx,8),%%ebp\n\t"                   \
                     "movb 2(%%esi),%%bl\n\t"                           \
                     "orl  4(%%eax,%%edx,8),%%ebp\n\t"                  \
                     "movb 3(%%esi),%%dl\n\t"                           \
                     "movl %%ebp,(%%edi)\n\t"                           \
                     "movl (%%eax,%%ebx,8),%%ebp\n\t"                   \
                     "addl $4,%%esi\n\t"                                \
                     "orl  4(%%eax,%%edx,8),%%ebp\n\t"                  \
                     "movl %%ebp,4(%%edi)\n\t"                          \
                     "addl $8,%%edi\n\t"                                \
                     "decl %%ecx\n\t"                                   \
                     "jnz 2b\n"                                         \
                                                                        \
                     "3:\n\t"                                           \
                     "popl %%ebp\n\t"                                   \
                     "popl %%ebx\n\t"                                   \
                     : "=S"(inp), "=D"(outp), "=a"(smashed_ax),         \
                       "=c"(smashed_cx)                                 \
                     : "2"(table), "0"(inp), "1"(outp), "3"(byte_width) \
                     : "dx", "cc", "memory");                           \
    })

#define CONVERT_1_4()                                                   \
    ({                                                                  \
        int smashed_ax, smashed_cx;                                     \
                                                                        \
        asm volatile("pushl %%ebx\n\t"                                  \
                     "pushl %%ebp\n\t"                                  \
                     "xorl %%ebx,%%ebx\n\t"                             \
                     "xorl %%edx,%%edx\n\t"                             \
                                                                        \
                     "testb $3,%%cl\n\t"                                \
                     "jz 1f\n\t"                                        \
                     "jmp 5f\n\t"                                       \
                                                                        \
                     /* Cleanup loop; handle cruft until we get an even \
                      * multiple of 4 bytes left to process.            \
                      */                                                \
                     ".align 4,0x90\n"                                  \
                     "5:\n\t"                                           \
                     "movb (%%esi),%%bl\n\t"                            \
                     "decl %%ecx\n\t"                                   \
                     "incl %%esi\n\t"                                   \
                     "movl (%%eax,%%ebx,4),%%ebp\n\t"                   \
                     "testb $3,%%cl\n\t"                                \
                     "movl %%ebp,(%%edi)\n\t"                           \
                     "leal 4(%%edi),%%edi\n\t"                          \
                     "jnz 5b\n"                                         \
                                                                        \
                     "1:\n\t"                                           \
                     "shrl $2,%%ecx\n\t"                                \
                     "jnz 2f\n\t"                                       \
                     "jmp 3f\n\t"                                       \
                                                                        \
                     ".align 4,0x90\n"                                  \
                     "2:\n\t"                                           \
                     "movb (%%esi),%%bl\n\t"                            \
                     "movb 1(%%esi),%%dl\n\t"                           \
                     "movl (%%eax,%%ebx,4),%%ebp\n\t"                   \
                     "movb 2(%%esi),%%bl\n\t"                           \
                     "movl %%ebp,(%%edi)\n\t"                           \
                     "movl (%%eax,%%edx,4),%%ebp\n\t"                   \
                     "movb 3(%%esi),%%dl\n\t"                           \
                     "movl %%ebp,4(%%edi)\n\t"                          \
                     "movl (%%eax,%%ebx,4),%%ebp\n\t"                   \
                     "addl $4,%%esi\n\t"                                \
                     "movl %%ebp,8(%%edi)\n\t"                          \
                     "movl (%%eax,%%edx,4),%%ebp\n\t"                   \
                     "movl %%ebp,12(%%edi)\n\t"                         \
                     "addl $16,%%edi\n\t"                               \
                     "decl %%ecx\n\t"                                   \
                     "jnz 2b\n"                                         \
                                                                        \
                     "3:\n\t"                                           \
                     "popl %%ebp\n\t"                                   \
                     "popl %%ebx\n\t"                                   \
                     : "=S"(inp), "=D"(outp), "=a"(smashed_ax),         \
                       "=c"(smashed_cx)                                 \
                     : "2"(table), "0"(inp), "1"(outp), "3"(byte_width) \
                     : "dx", "cc", "memory");                           \
    })

#define CONVERT_1_8()                                                   \
    ({                                                                  \
        int smashed_ax, smashed_cx;                                     \
                                                                        \
        asm volatile("pushl %%ebx\n\t"                                  \
                     "pushl %%ebp\n\t"                                  \
                     "xorl %%ebx,%%ebx\n\t"                             \
                     "xorl %%edx,%%edx\n\t"                             \
                                                                        \
                     "testb $3,%%cl\n\t"                                \
                     "jz 1f\n\t"                                        \
                     "jmp 5f\n\t"                                       \
                                                                        \
                     /* Cleanup loop; handle cruft until we get an even \
                      * multiple of 4 bytes left to process.            \
                      */                                                \
                     ".align 4,0x90\n"                                  \
                     "5:\n\t"                                           \
                     "movb (%%esi),%%bl\n\t"                            \
                     "decl %%ecx\n\t"                                   \
                     "incl %%esi\n\t"                                   \
                     "movl (%%eax,%%ebx,8),%%ebp\n\t"                   \
                     "movl %%ebp,(%%edi)\n\t"                           \
                     "movl 4(%%eax,%%ebx,8),%%ebp\n\t"                  \
                     "testb $3,%%cl\n\t"                                \
                     "movl %%ebp,4(%%edi)\n\t"                          \
                     "leal 8(%%edi),%%edi\n\t"                          \
                     "jnz 5b\n"                                         \
                                                                        \
                     "1:\n\t"                                           \
                     "shrl $2,%%ecx\n\t"                                \
                     "jnz 2f\n\t"                                       \
                     "jmp 3f\n\t"                                       \
                                                                        \
                     ".align 4,0x90\n"                                  \
                     "2:\n\t"                                           \
                     "movb (%%esi),%%bl\n\t"                            \
                     "movb 1(%%esi),%%dl\n\t"                           \
                                                                        \
                     "movl (%%eax,%%ebx,8),%%ebp\n\t"                   \
                     "movl %%ebp,(%%edi)\n\t"                           \
                     "movl 4(%%eax,%%ebx,8),%%ebp\n\t"                  \
                     "movl %%ebp,4(%%edi)\n\t"                          \
                                                                        \
                     "movb 2(%%esi),%%bl\n\t"                           \
                                                                        \
                     "movl (%%eax,%%edx,8),%%ebp\n\t"                   \
                     "movl %%ebp,8(%%edi)\n\t"                          \
                     "movl 4(%%eax,%%edx,8),%%ebp\n\t"                  \
                     "movl %%ebp,12(%%edi)\n\t"                         \
                                                                        \
                     "movb 3(%%esi),%%dl\n\t"                           \
                                                                        \
                     "movl (%%eax,%%ebx,8),%%ebp\n\t"                   \
                     "movl %%ebp,16(%%edi)\n\t"                         \
                     "movl 4(%%eax,%%ebx,8),%%ebp\n\t"                  \
                     "addl $4,%%esi\n\t"                                \
                     "movl %%ebp,20(%%edi)\n\t"                         \
                                                                        \
                     "movl (%%eax,%%edx,8),%%ebp\n\t"                   \
                     "movl %%ebp,24(%%edi)\n\t"                         \
                     "movl 4(%%eax,%%edx,8),%%ebp\n\t"                  \
                     "movl %%ebp,28(%%edi)\n\t"                         \
                                                                        \
                     "addl $32,%%edi\n\t"                               \
                     "decl %%ecx\n\t"                                   \
                     "jnz 2b\n"                                         \
                                                                        \
                     "3:\n\t"                                           \
                     "popl %%ebp\n\t"                                   \
                     "popl %%ebx\n\t"                                   \
                     : "=S"(inp), "=D"(outp), "=a"(smashed_ax),         \
                       "=c"(smashed_cx)                                 \
                     : "2"(table), "0"(inp), "1"(outp), "3"(byte_width) \
                     : "dx", "cc", "memory");                           \
    })

#define COMPUTE_ADD_COPY(row_bytes, byte_width) ((row_bytes) - (byte_width))
#define COMPUTE_ADD_1_1(row_bytes, byte_width) ((row_bytes) - (byte_width))
#define COMPUTE_ADD_1_2(row_bytes, byte_width) ((row_bytes) - (byte_width))
#define COMPUTE_ADD_1_4(row_bytes, byte_width) ((row_bytes) - (byte_width))
#define COMPUTE_ADD_1_8(row_bytes, byte_width) ((row_bytes) - (byte_width))

#endif /* i386 */

#if !defined(INP_OUTP_DECL)
#define INP_OUTP_DECL \
    const uint8 *inp; \
    uint8 *outp
#endif /* !defined (INP_OUTP_DECL) */

#if !defined(CONVERT_COPY)
#define CONVERT_COPY() memcpy(outp, inp, byte_width)
#define COMPUTE_ADD_COPY(row_bytes, byte_width) (row_bytes)
#endif /* !CONVERT_COPY */

#if !defined(CONVERT_1_1)
#define CONVERT_1_1()                        \
    do                                       \
    {                                        \
        int w;                               \
        for(w = byte_width - 1; w >= 0; w--) \
            outp[w] = (*table)[inp[w]];      \
    } while(0)
#define COMPUTE_ADD_1_1(row_bytes, byte_width) (row_bytes)
#endif /* !CONVERT_1_1 */

#if !defined(CONVERT_1_2)
#define CONVERT_1_2()                                    \
    do                                                   \
    {                                                    \
        int w;                                           \
        for(w = byte_width - 1; w >= 0; w--)             \
            ((uint16_t *)outp)[w] = (*table)[inp[w]][0]; \
    } while(0)
#define COMPUTE_ADD_1_2(row_bytes, byte_width) (row_bytes)
#endif /* !CONVERT_1_2 */

#if !defined(CONVERT_1_4)
#define CONVERT_1_4()                                 \
    do                                                \
    {                                                 \
        int w;                                        \
        for(w = byte_width - 1; w >= 0; w--)          \
            ((uint32_t *)outp)[w] = (*table)[inp[w]]; \
    } while(0)
#define COMPUTE_ADD_1_4(row_bytes, byte_width) (row_bytes)
#endif /* !CONVERT_1_4 */

#if !defined(CONVERT_1_8)
#define CONVERT_1_8()                                        \
    do                                                       \
    {                                                        \
        int w;                                               \
        for(w = byte_width - 1; w >= 0; w--)                 \
        {                                                    \
            unsigned in = inp[w];                            \
            ((uint32_t *)outp)[w * 2] = (*table)[in][0];     \
            ((uint32_t *)outp)[w * 2 + 1] = (*table)[in][1]; \
        }                                                    \
    } while(0)
#define COMPUTE_ADD_1_8(row_bytes, byte_width) (row_bytes)
#endif /* !CONVERT_1_8 */

#if !defined(CONVERT_1_16)
#define CONVERT_1_16()                                 \
    do                                                 \
    {                                                  \
        int w;                                         \
        uint32_t *dp;                                  \
                                                       \
        dp = (uint32_t *)&outp[(byte_width - 1) * 16]; \
        for(w = byte_width - 1; w >= 0; dp -= 4, w--)  \
        {                                              \
            const uint32_t *sp;                        \
            sp = &(*table)[inp[w]][0];                 \
            dp[0] = sp[0];                             \
            dp[1] = sp[1];                             \
            dp[2] = sp[2];                             \
            dp[3] = sp[3];                             \
        }                                              \
    } while(0)
#define COMPUTE_ADD_1_16(row_bytes, byte_width) (row_bytes)
#endif /* !CONVERT_1_16 */

#if !defined(CONVERT_1_32)
#define CONVERT_1_32()                                 \
    do                                                 \
    {                                                  \
        int w;                                         \
        uint32_t *dp;                                  \
                                                       \
        dp = (uint32_t *)&outp[(byte_width - 1) * 32]; \
        for(w = byte_width - 1; w >= 0; dp -= 8, w--)  \
        {                                              \
            const uint32_t *sp;                        \
            sp = &(*table)[inp[w]][0];                 \
            dp[0] = sp[0];                             \
            dp[1] = sp[1];                             \
            dp[2] = sp[2];                             \
            dp[3] = sp[3];                             \
            dp[4] = sp[4];                             \
            dp[5] = sp[5];                             \
            dp[6] = sp[6];                             \
            dp[7] = sp[7];                             \
        }                                              \
    } while(0)
#define COMPUTE_ADD_1_32(row_bytes, byte_width) (row_bytes)
#endif /* !CONVERT_1_32 */

#if !defined(CONVERT_2_1)
#define CONVERT_2_1()                                  \
    do                                                 \
    {                                                  \
        int w;                                         \
        for(w = (byte_width / 2) - 1; w >= 0; w--)     \
            outp[w] = ((*table)[0][inp[w * 2]]         \
                       | (*table)[1][inp[w * 2 + 1]]); \
    } while(0)
#define COMPUTE_ADD_2_1(row_bytes, byte_width) (row_bytes)
#endif /* !CONVERT_2_1 */

#if !defined(CONVERT_4_1)
#define CONVERT_4_1()                                  \
    do                                                 \
    {                                                  \
        int w;                                         \
        for(w = (byte_width / 4) - 1; w >= 0; w--)     \
            outp[w] = ((*table)[0][inp[w * 4]]         \
                       | (*table)[1][inp[w * 4 + 1]]   \
                       | (*table)[2][inp[w * 4 + 2]]   \
                       | (*table)[3][inp[w * 4 + 3]]); \
    } while(0)
#define COMPUTE_ADD_4_1(row_bytes, byte_width) (row_bytes)
#endif /* !CONVERT_4_1 */

#if !defined(CONVERT_8_1)
#define CONVERT_8_1()                                  \
    do                                                 \
    {                                                  \
        int w;                                         \
        for(w = (byte_width / 8) - 1; w >= 0; w--)     \
            outp[w] = ((*table)[0][inp[w * 8]]         \
                       | (*table)[1][inp[w * 8 + 1]]   \
                       | (*table)[2][inp[w * 8 + 2]]   \
                       | (*table)[3][inp[w * 8 + 3]]   \
                       | (*table)[4][inp[w * 8 + 4]]   \
                       | (*table)[5][inp[w * 8 + 5]]   \
                       | (*table)[6][inp[w * 8 + 6]]   \
                       | (*table)[7][inp[w * 8 + 7]]); \
    } while(0)
#define COMPUTE_ADD_8_1(row_bytes, byte_width) (row_bytes)
#endif /* !CONVERT_8_1 */

#define CONVERT_RGB_IND(in_type, out_bpp)                              \
    do                                                                 \
    {                                                                  \
        int w, shift;                                                  \
        uint8 v;                                                       \
                                                                       \
        shift = 8 - (out_bpp);                                         \
        v = 0;                                                         \
                                                                       \
        for(w = byte_width; w > 0; w -= sizeof(in_type))               \
        {                                                              \
            RGBColor r;                                                \
            in_type in;                                                \
                                                                       \
            in = *(const in_type *)inp;                                \
            inp += sizeof(in_type);                                    \
            (*src_rgb_spec->pixel_to_rgbcolor)(src_rgb_spec, in, &r);  \
            v |= (Color2Index(&r) & ((1L << (out_bpp)) - 1)) << shift; \
            shift -= (out_bpp);                                        \
            if(shift < 0)                                              \
            {                                                          \
                *outp++ = v;                                           \
                shift = 8 - (out_bpp);                                 \
                v = 0;                                                 \
            }                                                          \
        }                                                              \
    } while(0)

#if !defined(CONVERT_16_1)
#define CONVERT_16_1() CONVERT_RGB_IND(uint16_t, 1)
#define COMPUTE_ADD_16_1(row_bytes, byte_width) ((row_bytes) - (byte_width))
#endif /* !CONVERT_16_1 */

#if !defined(CONVERT_16_2)
#define CONVERT_16_2() CONVERT_RGB_IND(uint16_t, 2)
#define COMPUTE_ADD_16_2(row_bytes, byte_width) ((row_bytes) - (byte_width))
#endif /* !CONVERT_16_2 */

#if !defined(CONVERT_16_4)
#define CONVERT_16_4() CONVERT_RGB_IND(uint16_t, 4)
#define COMPUTE_ADD_16_4(row_bytes, byte_width) ((row_bytes) - (byte_width))
#endif /* !CONVERT_16_4 */

#if !defined(CONVERT_16_8)
#define CONVERT_16_8() CONVERT_RGB_IND(uint16_t, 8)
#define COMPUTE_ADD_16_8(row_bytes, byte_width) ((row_bytes) - (byte_width))
#endif /* !CONVERT_16_8 */

#if !defined(CONVERT_32_1)
#define CONVERT_32_1() CONVERT_RGB_IND(uint32_t, 1)
#define COMPUTE_ADD_32_1(row_bytes, byte_width) ((row_bytes) - (byte_width))
#endif /* !CONVERT_32_1 */

#if !defined(CONVERT_32_2)
#define CONVERT_32_2() CONVERT_RGB_IND(uint32_t, 2)
#define COMPUTE_ADD_32_2(row_bytes, byte_width) ((row_bytes) - (byte_width))
#endif /* !CONVERT_32_2 */

#if !defined(CONVERT_32_4)
#define CONVERT_32_4() CONVERT_RGB_IND(uint32_t, 4)
#define COMPUTE_ADD_32_4(row_bytes, byte_width) ((row_bytes) - (byte_width))
#endif /* !CONVERT_32_4 */

#if !defined(CONVERT_32_8)
#define CONVERT_32_8() CONVERT_RGB_IND(uint32_t, 8)
#define COMPUTE_ADD_32_8(row_bytes, byte_width) ((row_bytes) - (byte_width))
#endif /* !CONVERT_32_8 */

#define CONVERT_RGB_RGB(in_type, out_type)                                \
    do                                                                    \
    {                                                                     \
        int w;                                                            \
                                                                          \
        for(w = byte_width; w > 0; w -= sizeof(in_type))                  \
        {                                                                 \
            RGBColor r;                                                   \
            out_type v;                                                   \
            in_type in;                                                   \
                                                                          \
            in = *(const in_type *)inp;                                   \
            inp += sizeof(in_type);                                       \
                                                                          \
            if(in == cache_in)                                            \
                v = cache_out;                                            \
            else                                                          \
            {                                                             \
                /* Translate src RGB to canonical form. */                \
                (*src_rgb_spec->pixel_to_rgbcolor)(src_rgb_spec, in, &r); \
                                                                          \
                /* Translate canonical form to dest pixel. */             \
                v = (*dst_rgb_spec->rgbcolor_to_pixel)(dst_rgb_spec, &r,  \
                                                       true);             \
                                                                          \
                /* Record the last RGB translated, for speed. */          \
                cache_in = in;                                            \
                cache_out = v;                                            \
            }                                                             \
                                                                          \
            *(out_type *)outp = v;                                        \
            outp += sizeof(out_type);                                     \
        }                                                                 \
    } while(0)

#if !defined(CONVERT_16_16)
#define CONVERT_16_16() CONVERT_RGB_RGB(uint16_t, uint16_t)
#define COMPUTE_ADD_16_16(row_bytes, byte_width) ((row_bytes) - (byte_width))
#endif /* !CONVERT_16_16 */

#if !defined(CONVERT_16_32)
#define CONVERT_16_32() CONVERT_RGB_RGB(uint16_t, uint32_t)
#define COMPUTE_ADD_16_32(row_bytes, byte_width) ((row_bytes) - (byte_width))
#endif /* !CONVERT_16_32 */

#if !defined(CONVERT_32_16)
#define CONVERT_32_16() CONVERT_RGB_RGB(uint32_t, uint16_t)
#define COMPUTE_ADD_32_16(row_bytes, byte_width) ((row_bytes) - (byte_width))
#endif /* !CONVERT_32_16 */

#if !defined(CONVERT_32_32)
#define CONVERT_32_32() CONVERT_RGB_RGB(uint32_t, uint32_t)
#define COMPUTE_ADD_32_32(row_bytes, byte_width) ((row_bytes) - (byte_width))
#endif /* !CONVERT_32_32 */

#define _ALIGN_TABLE(t, in, out) DEPTHCONV_ALIGN_TABLE(t, in, out)
#define EXTRA_DECLS
#define EXTRA_SETUP()
#define EXTRA_CLEANUP()

/* Handle bpp nondecreasing cases. */
#define IF_TABLE(x)
CONVERT_FUNC(depthconv_copy, CONVERT_COPY, 0, 0, COMPUTE_ADD_COPY, void)
#undef IF_TABLE
#define IF_TABLE(x) x
CONVERT_FUNC(depthconv_1_1, CONVERT_1_1, 0, 0, COMPUTE_ADD_1_1,
             depthconv_1_1_data_t)
CONVERT_FUNC(depthconv_1_2, CONVERT_1_2, 1, 1, COMPUTE_ADD_1_2,
             depthconv_1_2_data_t)
CONVERT_FUNC(depthconv_1_4, CONVERT_1_4, 2, 2, COMPUTE_ADD_1_4,
             depthconv_1_4_data_t)
CONVERT_FUNC(depthconv_1_8, CONVERT_1_8, 3, 3, COMPUTE_ADD_1_8,
             depthconv_1_8_data_t)
CONVERT_FUNC(depthconv_1_16, CONVERT_1_16, 4, 4, COMPUTE_ADD_1_16,
             depthconv_1_16_data_t)
CONVERT_FUNC(depthconv_1_32, CONVERT_1_32, 5, 5, COMPUTE_ADD_1_32,
             depthconv_1_32_data_t)

/* Handle bpp decreasing cases. */
CONVERT_FUNC(depthconv_2_1, CONVERT_2_1, -1, 1, COMPUTE_ADD_2_1,
             depthconv_2_1_data_t)
CONVERT_FUNC(depthconv_4_1, CONVERT_4_1, -2, 2, COMPUTE_ADD_4_1,
             depthconv_4_1_data_t)
CONVERT_FUNC(depthconv_8_1, CONVERT_8_1, -3, 3, COMPUTE_ADD_8_1,
             depthconv_8_1_data_t)

/* Handle RGB -> RGB cases. */
#undef _ALIGN_TABLE
#define _ALIGN_TABLE(t, in, out) (t)

#undef EXTRA_SETUP
#undef EXTRA_CLEANUP

#define EXTRA_SETUP()                          \
    do                                         \
    {                                          \
        src_rgb_spec = table->src_rgb_spec;    \
        dst_rgb_spec = table->dst_rgb_spec;    \
        cache_in = src_rgb_spec->white_pixel;  \
        cache_out = dst_rgb_spec->white_pixel; \
    } while(0)

#define EXTRA_CLEANUP()

#define SRC_DST_RGB_SPEC_DECLS      \
    const rgb_spec_t *src_rgb_spec; \
    const rgb_spec_t *dst_rgb_spec

#undef EXTRA_DECLS
#define EXTRA_DECLS     \
    uint16_t cache_in;  \
    uint16_t cache_out; \
    SRC_DST_RGB_SPEC_DECLS
CONVERT_FUNC(depthconv_16_16, CONVERT_16_16, 0, 0, COMPUTE_ADD_16_16,
             depthconv_rgb_to_rgb_data_t)

#undef EXTRA_DECLS
#define EXTRA_DECLS     \
    uint16_t cache_in;  \
    uint32_t cache_out; \
    SRC_DST_RGB_SPEC_DECLS
CONVERT_FUNC(depthconv_16_32, CONVERT_16_32, 1, 1, COMPUTE_ADD_16_32,
             depthconv_rgb_to_rgb_data_t)

#undef EXTRA_DECLS
#define EXTRA_DECLS     \
    uint32_t cache_in;  \
    uint16_t cache_out; \
    SRC_DST_RGB_SPEC_DECLS
CONVERT_FUNC(depthconv_32_16, CONVERT_32_16, -1, 1, COMPUTE_ADD_32_16,
             depthconv_rgb_to_rgb_data_t)

#undef EXTRA_DECLS
#define EXTRA_DECLS     \
    uint32_t cache_in;  \
    uint32_t cache_out; \
    SRC_DST_RGB_SPEC_DECLS
CONVERT_FUNC(depthconv_32_32, CONVERT_32_32, 0, 0, COMPUTE_ADD_32_32,
             depthconv_rgb_to_rgb_data_t)

/* Handle RGB -> indirect cases. */
#undef EXTRA_DECLS
#undef EXTRA_SETUP
#undef EXTRA_CLEANUP
#undef SRC_DST_RGB_SPEC_DECLS

#define EXTRA_DECLS                   \
    GDHandle gdev;                    \
    PixMapHandle gd_pmap;             \
    GUEST<CTabHandle> gdev_ctab_save; \
    GUEST<ITabHandle> gdev_itab_save; \
    const rgb_spec_t *src_rgb_spec

#define EXTRA_SETUP()                                  \
    do                                                 \
    {                                                  \
        gdev = MR(TheGDevice);                         \
        gd_pmap = GD_PMAP(gdev);                       \
        gdev_ctab_save = PIXMAP_TABLE_X(gd_pmap);      \
        gdev_itab_save = GD_ITABLE_X(gdev);            \
        PIXMAP_TABLE_X(gd_pmap) = table->swapped_ctab; \
        GD_ITABLE_X(gdev) = table->swapped_itab;       \
        src_rgb_spec = table->src_rgb_spec;            \
    } while(0)

#define EXTRA_CLEANUP()                           \
    do                                            \
    {                                             \
        PIXMAP_TABLE_X(gd_pmap) = gdev_ctab_save; \
        GD_ITABLE_X(gdev) = gdev_itab_save;       \
    } while(0)

CONVERT_FUNC(depthconv_16_1, CONVERT_16_1, -4, 4, COMPUTE_ADD_16_1,
             depthconv_rgb_to_ind_data_t)
CONVERT_FUNC(depthconv_16_2, CONVERT_16_2, -3, 3, COMPUTE_ADD_16_2,
             depthconv_rgb_to_ind_data_t)
CONVERT_FUNC(depthconv_16_4, CONVERT_16_4, -2, 2, COMPUTE_ADD_16_4,
             depthconv_rgb_to_ind_data_t)
CONVERT_FUNC(depthconv_16_8, CONVERT_16_8, -1, 1, COMPUTE_ADD_16_8,
             depthconv_rgb_to_ind_data_t)

CONVERT_FUNC(depthconv_32_1, CONVERT_32_1, -5, 5, COMPUTE_ADD_32_1,
             depthconv_rgb_to_ind_data_t)
CONVERT_FUNC(depthconv_32_2, CONVERT_32_2, -4, 4, COMPUTE_ADD_32_2,
             depthconv_rgb_to_ind_data_t)
CONVERT_FUNC(depthconv_32_4, CONVERT_32_4, -3, 3, COMPUTE_ADD_32_4,
             depthconv_rgb_to_ind_data_t)
CONVERT_FUNC(depthconv_32_8, CONVERT_32_8, -2, 2, COMPUTE_ADD_32_8,
             depthconv_rgb_to_ind_data_t)
}
