#if !defined(_ARCH_M68K_H_)
#define _ARCH_M68K_H_

#define BIGENDIAN

#if !defined(mc68000)
#define mc68000
#endif

#if defined(SYN68K)
#error "m68k should not be using SYN68K!"
#endif

#if defined(LITTLEENDIAN)
#error "m68k is not little endian!"
#endif

/* We don't want our CPU state to have anything more than regs and cc bits. */
#define MINIMAL_CPU_STATE

extern void m68k_call(unsigned long addr);
#define CALL_EMULATOR(addr) m68k_call(addr)

#endif /* _ARCH_M68K_H_ */
