#ifndef _int386_h_
#define _int386_h_

#ifdef MSDOS

typedef union {
    struct
    {
        unsigned long eax, ebx, ecx, edx, esi, edi, ebp;
    } l;
    struct
    {
        unsigned short ax, ax_filler;
        unsigned short bx, bx_filler;
        unsigned short cx, cx_filler;
        unsigned short dx, dx_filler;
        unsigned short si, si_filler;
        unsigned short di, di_filler;
        unsigned short bp, bp_filler;
    } w;
    struct
    {
        unsigned char al, ah, ax_filler1, ax_filler2;
        unsigned char bl, bh, bx_filler1, bx_filler2;
        unsigned char cl, ch, cx_filler1, cx_filler2;
        unsigned char dl, dh, dx_filler1, dx_filler2;
        unsigned short si, si_filler;
        unsigned short di, di_filler;
        unsigned short bp, bp_filler;
    } b;
} i386_registers_t;

/* These functions take a set of registers as input and modify the registers
 * to reflect the return values of the interrupt.  The return value of the
 * function is 1 if the carry is set after the interrupt, else 0.
 */
extern int int10(i386_registers_t *regs);
extern int int13(i386_registers_t *regs);
extern int int21(i386_registers_t *regs);
extern int int33(i386_registers_t *regs);

#endif /* MSDOS */

#endif /* Not _int386_h_ */
