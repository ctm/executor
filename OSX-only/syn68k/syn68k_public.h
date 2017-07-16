//
//  syn68k_public.h
//  CocoaExecutor
//
//  Created by C.W. Betts on 7/29/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#if !defined (_syn68k_public_h_)
#define _syn68k_public_h_

#include <stdint.h>
#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Decide whether we are big or little endian here.  Add more machines as
 * they are supported.
 */
#if !defined(BIGENDIAN) && !defined(LITTLEENDIAN)
# if defined(__BIG_ENDIAN__) || defined(m68k) || defined(mc68000) \
|| defined(sparc) || defined(powerpc) || defined (__ppc__)
#  define BIGENDIAN  /* ARDI naming convention, why rock the boat? */
# elif defined(__alpha) || defined(i860) || defined(vax) || defined(i386) \
|| defined(__x86_64)
#  define LITTLEENDIAN
# else
#  error "Unknown CPU type"
# endif
#endif


/* Define this for CPUs that require n byte data be aligned
 * on addresses congruent to zero modulo n.  The name "QUADALIGN"
 * is another ARDI convention, why rock the boat?
 */

/*
 * Alpha not quad aligned right now because that breaks Mat's code and the
 * alpha can get by, for now.
 */

#if !defined (QUADALIGN)
# if defined(sparc) || /* defined(__alpha) || */ defined (i860) || defined(mips)
#  define QUADALIGN
# elif !defined(m68k) && !defined(mc68000) && !defined(i386) \
&& !defined(vax) && !defined(__alpha) && !defined(powerpc) \
&& !defined (__ppc__) && !defined(__x86_64)
#  error Unknown CPU type
# endif
#endif


/* Turn this on if you want to enable destroying only blocks with checksum
 * mismatches.
 */
#define CHECKSUM_BLOCKS


/* Turn this on if you want syn68k to poll for interrupts, instead of
 * being interrupted by the real OS.
 */
#define SYNCHRONOUS_INTERRUPTS

/* Turn this on if you want to let the BIOS event wait manage the low
 * memory interrupt flag.
 */
#if 0 && defined (MSDOS) && defined (SYNCHRONOUS_INTERRUPTS)
# define USE_BIOS_TIMER
#endif

/* Portability typedefs for signed and unsigned n-bit numbers. */

/* TODO: just use the ones from stdint.h instead of our own */
#if !defined (INT_TYPES_TYPEDEFED)
typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
#define INT_TYPES_TYPEDEFED
#endif


/* Typedef for address in 68k space. */
typedef uint32 syn68k_addr_t;

/* Integral type big enough to hold a pointer.  Presumably we'll need to use
 * unsigned long long on machines with 64 bit pointers.
 */
typedef unsigned long ptr_sized_uint;

/* Because a 68k register can be referenced as a byte, word, or int32,
 * we represent them as a union of these types.  Although not guaranteed
 * to be portable, operations on appropriate byte- and word- union elements
 * should leave the other bytes unaffected (just as they do on the 68k).
 * on the 68k).
 */
#if defined (LITTLEENDIAN)
typedef union {
	struct { uint32 n; } ul;
	struct { int32 n; } sl;
	struct { uint16 n, hi; } uw;
	struct { int16 n, hi; } sw;
	struct { uint8 n, b1, b2, b3; } ub;
	struct { int8 n, b1, b2, b3; } sb;
} M68kReg;
#else
typedef union {
	struct { uint32 n; } ul;
	struct { int32 n; } sl;
	struct { uint16 hi, n; } uw;
	struct { int16 hi, n; } sw;
	struct { uint8 b3, b2, b1, n; } ub;
	struct { int8 b3, b2, b1, n; } sb;
} M68kReg;
#endif

/* Function type that handles callbacks (and therefore traps). */
typedef syn68k_addr_t (*callback_handler_t)(syn68k_addr_t, void *);

typedef struct {
	syn68k_addr_t callback_address;
	callback_handler_t func;
	void *arg;
} TrapHandlerInfo;


#if (defined(i386) || defined(m68k)) && !defined (NO_CCR_SPEEDUPS)
#define CCR_ELEMENT_8_BITS
#endif

#if defined (CCR_ELEMENT_8_BITS)
typedef uint8 CCRElement;  /* So we can setb, etc. them directly. */
#else /* !CCR_ELEMENT_8_BITS */
typedef int32 CCRElement;
#endif /* !CCR_ELEMENT_8_BITS */

typedef struct {
	syn68k_addr_t tag;
	const uint16 *code;
} jsr_stack_elt_t;

#define JSR_STACK_SIZE 8

typedef struct {
	M68kReg regs[16];   /* d0...d7 followed by a0...a7 */
	CCRElement ccnz, ccn, ccc, ccv, ccx;          /* 0 or non-0, NOT 0 or 1! */
#if defined (MINIMAL_CPU_STATE)
	uint8 filler[3]; /* So we can copy small cpu states w/out boundary cruft. */
#else /* !MINIMAL_CPU_STATE */
	char *amode_p, *reversed_amode_p;  /* Used in interpreter. */
#if !defined (USE_BIOS_TIMER) && defined (SYNCHRONOUS_INTERRUPTS)
	volatile int32 interrupt_status_changed; /* High bit set when interrupted. */
#endif
	jmp_buf *setjmp_buf;
	jsr_stack_elt_t jsr_stack[JSR_STACK_SIZE];
	unsigned jsr_stack_byte_index; /* # of **BYTES** into jsr stack for top. */
	volatile uint16 sr; /* Status register, except cc bits (low 5 bits) == 0. */
	syn68k_addr_t vbr;  /* Vector base register (in 68k address space). */
	uint32 cacr, caar;  /* Cache Control Register/Cache Address Register. */
	uint32 usp, msp, isp;  /* Various stack pointers.  Whichever one is
							* currently in use can only be found in a7;
							* the value here will be outdated. */
	volatile uint8 interrupt_pending[8];     /* 1 if interrupt pending. */
	volatile TrapHandlerInfo trap_handler_info[64];
#endif /* !MINIMAL_CPU_STATE */
} CPUState;


/* This lets us put syn68k.c's global register variable before the
 * inline functions in this file, since that is an error.
 */
#if defined (GLOBAL_REGISTER_DECLS)
GLOBAL_REGISTER_DECLS
#endif


#define EM_DREG(X) (cpu_state.regs[X].ul.n)
#define EM_AREG(X) (cpu_state.regs[8 + (X)].ul.n)
#define EM_D0 EM_DREG (0)
#define EM_D1 EM_DREG (1)
#define EM_D2 EM_DREG (2)
#define EM_D3 EM_DREG (3)
#define EM_D4 EM_DREG (4)
#define EM_D5 EM_DREG (5)
#define EM_D6 EM_DREG (6)
#define EM_D7 EM_DREG (7)
#define EM_A0 EM_AREG (0)
#define EM_A1 EM_AREG (1)
#define EM_A2 EM_AREG (2)
#define EM_A3 EM_AREG (3)
#define EM_A4 EM_AREG (4)
#define EM_A5 EM_AREG (5)
#define EM_A6 EM_AREG (6)
#define EM_A7 EM_AREG (7)
#define EM_FP EM_A6
#define EM_SP EM_A7

/* We need a range of "magic" addresses where we know legitimate 68k
 * code will be stored.  When code at those addresses is "executed" by
 * the 68k, we will instead take special actions.  We want those
 * addresses to be dereferenceable so we'll overlay them over a big
 * block of legitimate memory that can never contain m68k code.
 * We'll start a few bytes into the array in case anyone examines
 * memory before the array.
 */
extern uint16 callback_dummy_address_space[];
#define CALLBACK_SLOP 16
#define MAGIC_ADDRESS_BASE \
((syn68k_addr_t) US_TO_SYN68K((unsigned long) (&callback_dummy_address_space[CALLBACK_SLOP])))
#define MAGIC_EXIT_EMULATOR_ADDRESS (MAGIC_ADDRESS_BASE + 0)
#define MAGIC_RTE_ADDRESS           (MAGIC_ADDRESS_BASE + 2)

/* Address bits that have meaning, for the CLEAN() macro, below. */
#define LEGAL_ADDRESS_BITS 0xFFFFFFFFUL

/* Here is a macro to "clean" an address; if you make it mask out the
 * nasty bits, you risk a noticeable performance downgrade.
 */
#if 1
# define CLEAN(addr) ((ptr_sized_uint)(addr))
#else
# define CLEAN(addr) (((ptr_sized_uint)(addr)) & LEGAL_ADDRESS_BITS)
#endif


/* TODO: see if get rid of need for SIZEOF_CHAR_P, since that's something
 that comes from config.h and we shouldn't require users of
 syn68k_public.h to require config.h */

#if !defined(SIZEOF_CHAR_P)

#if !defined(__x86_64)
# define SIZEOF_CHAR_P 4
#else
# define SIZEOF_CHAR_P 8
#endif

#endif


#if SIZEOF_CHAR_P == 4

extern uint32 ROMlib_offset;
#define SYN68K_TO_US(addr) ((uint16 *) ((unsigned long)addr + ROMlib_offset)) /* uint16 * only the default. */
#define US_TO_SYN68K(addr) (/*(syn68k_addr_t)*/(int32) (addr) - ROMlib_offset)

#elif SIZEOF_CHAR_P == 8

extern uint64 ROMlib_offset;
#define SYN68K_TO_US(addr) ((uint16 *) ((uint64)(uint32)addr + ROMlib_offset)) /* uint16 * only the default. */
#define US_TO_SYN68K(addr) ((uint32) ((long) (addr) - ROMlib_offset))

#else
#error "SIZEOF_CHAR_P unknown"
#endif

/* These macros should not be used within Syn68k, but are needed in
	ROMlib for translations of addresses where it's important to preserve
	the address 0 as 0 */

#define SYN68K_TO_US_CHECK0(addr) ({ typeof(addr) t; t = addr; t ? SYN68K_TO_US(t) : (uint16 *) 0; })
#define US_TO_SYN68K_CHECK0(addr) ({ typeof(addr) t; t = addr; t ? US_TO_SYN68K(t) : (int32) 0; })

/* Macros for byte swapping + specifying signedness.  On a big endian
 * machine, these macros are dummies and don't actually swap bytes.
 * You may wish to replace these with the most efficient inline assembly
 * you can get, as these macros are performance-critical.
 * The "_IFLE" suffix stands for "if little endian".
 */

/* Byte swapping macros. */
#define SWAPUB(val) ((uint8) (val))   /* Here for symmetry. */
#define SWAPSB(val) ((int8)  (val))

#if defined (CWV) && defined (CLV)
#define ROMlib_SwapShort(n)	CWV ((uint16) (n))
#define ROMlib_SwapLong(n)	CLV ((uint32) (n))
#define SWAPUW(n)		CWV ((uint16) (n))
#define SWAPUL(n)		CLV ((uint32) (n))
#elif defined(i386)

static inline uint16
ROMlib_SwapShort (unsigned short us) __attribute__ ((const));

static inline uint16
ROMlib_SwapShort (unsigned short us)
{
	uint16 retval;
	asm ("rorw $8,%w0" : "=r" (retval) : "0" (us) : "cc");
	return retval;
}

static inline uint32
ROMlib_SwapLong (uint32 ul) __attribute__ ((const));

static inline uint32
ROMlib_SwapLong (uint32 ul)
{
	uint32 retval;
#if defined (NeXT) || defined (USE_BSWAP) || defined (ALWAYS_ON_I486)
	asm ("bswap %0" : "=r" (retval) : "0" (ul));
#else
	asm ("rorw $8,%w0\n\t"
		 "rorl $16,%k0\n\t"
		 "rorw $8,%w0"
		 : "=r" (retval) : "0" (ul) : "cc");
#endif
	return retval;
}

# define SWAPUW(val) ROMlib_SwapShort(val)
# define SWAPUL(val) ROMlib_SwapLong(val)

#elif 0 && defined(__alpha)

static inline unsigned short const
ROMlib_SwapShort (unsigned short us)
{
	unsigned short retval;
	
	asm("extbl   %1, 1, $2\n\t"		/* r2 = 0x0A */
		"sll     %1, 8, %0\n\t"		/* %0 = 0xB0 */
		"or	 %0, $2, %0"		/* %0 = 0xBA */
		
		: "=r" (retval) : "r" (us) : "$2");
	
	return retval;
}

static inline uint32 const
ROMlib_SwapLong (uint32 ul)
{
	uint32 retval;
	
	asm("extbl   %1, 3, $2\n\t"		/* r2 = 0x000A */
		"sll     %1, 24, $3\n\t"	/* r3 = 0xD000 */
		
		"srl     %1, 8, $5\n\t"		/* r5 = 0x0ABC */
		"and     %1, 65280, %0\n\t"	/* %0 = 0x00C0 */
		
		"or      $2, $3, $4\n\t"	/* r4 = 0xD00A */
		"and     $5, 65280, $6\n\t"	/* r6 = 0x00B0 */
		
		"sll     %0, 8, %0\n\t"		/* %0 = 0x0C00 */
		"or      $4, $6, $7\n\t"	/* r7 = 0xD0BA */
		
		"or      $7, %0, %0"		/* %0 = 0xDCBA */
		
		: "=r" (retval) : "r" (ul) : "$2", "$3", "$4", "$5", "$6", "$7");
	
	return retval;
}

# define SWAPUW(val) ROMlib_SwapShort(val)
# define SWAPUL(val) ROMlib_SwapLong(val)

#elif defined(LITTLEENDIAN)

#if defined(__GNUC__)
# define SWAPUW(val) ({ uint16 _v = (val); (uint16) ((_v >> 8) | (_v << 8)); })
# define SWAPUL(val)                                        \
({ uint32 _v = (val);                                       \
(uint32) ((_v >> 24) | (_v << 24) | ((_v >> 8) & 0xFF00) \
| ((_v & 0xFF00) << 8));                       \
})
#else

static uint16 _swapuw(uint16 v)
{
	return (v >> 8) | (v << 8);
}

#define SWAPUW(val) _swapuw(val)

static uint32 _swapul(uint32 v)
{
	return (v >> 24) | (v << 24) | ((v >> 8) & 0xFF00) | ((v & 0xFF00) << 8);
}

#define SWAPUL(val) _swapul(val)

#endif

#define ROMlib_SwapShort(val) SWAPUW(val)
#define ROMlib_SwapLong(val) SWAPUL(val)

#endif
#define SWAPSW(val) ((int16) SWAPUW (val))
#define SWAPSL(val) ((int32) SWAPUL (val))

#define SWAPUB_IFLE(n) ((uint8) (n))   /* Here for symmetry. */
#define SWAPSB_IFLE(n) ((int8)  (n))

#if defined (BIGENDIAN)
# define SWAPUW_IFLE(n) ((uint16) (n))
# define SWAPSW_IFLE(n) ((int16)  (n))
# define SWAPUL_IFLE(n) ((uint32) (n))
# define SWAPSL_IFLE(n) ((int32)  (n))
#else /* LITTLEENDIAN */
# define SWAPUW_IFLE(n) SWAPUW(n)
# define SWAPSW_IFLE(n) SWAPSW(n)
# define SWAPUL_IFLE(n) SWAPUL(n)
# define SWAPSL_IFLE(n) SWAPSL(n)
#endif


/* The "slow" variants are here to facilitate compile-time reduction of
 * swapped constants.  The non-slow versions may use inline assembly, which
 * the C compiler won't be able to deal with, but the slow versions are
 * guaranteed to use standard C operators.  You would only want to use the
 * slow versions for swapping constants.
 */
#define SLOW_SWAPUB(val) ((uint8) (val))   /* Here for symmetry. */
#define SLOW_SWAPSB(val) ((int8)  (val))

#if defined(__GNUC__)
#define SLOW_SWAPUW(val) \
({ uint16 _v = (val); (uint16) ((_v >> 8) | (_v << 8)); })
#else
#define SLOW_SWAPUW(v) ((uint16) (((v) >> 8) | ((v) << 8)))
#endif

#define SLOW_SWAPSW(val) ((int16) SLOW_SWAPUW (val))

#if defined(__GNUC__)
#define SLOW_SWAPUL(val)                                    \
({ uint32 _v = (val);                                       \
(uint32) ((_v >> 24) | (_v << 24) | ((_v >> 8) & 0xFF00) \
| ((_v & 0xFF00) << 8));                       \
})
#else
#define SLOW_SWAPUL(v) ((uint32) (((v) >> 24) | \
((v) << 24) | \
(((v) >> 8) & 0xFF00) | \
(((v) & 0xFF00) << 8)))
#endif

#define SLOW_SWAPSL(val) ((int32) SLOW_SWAPUL (val))

#define SLOW_SWAPUB_IFLE(n) ((uint8) (n))   /* Here for symmetry. */
#define SLOW_SWAPSB_IFLE(n) ((int8)  (n))

#if defined (BIGENDIAN)
# define SLOW_SWAPUW_IFLE(n) ((uint16) (n))
# define SLOW_SWAPSW_IFLE(n) ((int16)  (n))
# define SLOW_SWAPUL_IFLE(n) ((uint32) (n))
# define SLOW_SWAPSL_IFLE(n) ((int32)  (n))
#else /* LITTLEENDIAN */
# define SLOW_SWAPUW_IFLE(n) SLOW_SWAPUW(n)
# define SLOW_SWAPSW_IFLE(n) SLOW_SWAPSW(n)
# define SLOW_SWAPUL_IFLE(n) SLOW_SWAPUL(n)
# define SLOW_SWAPSL_IFLE(n) SLOW_SWAPSL(n)
#endif


/* Handy memory access macros.  These implicity refer to the 68k addr space
 except for the _US variants that refer to the native address space.*/

/* Memory reads. */
#define READUB(addr) (*(const uint8 *) SYN68K_TO_US (CLEAN (addr)))
#define READUW(addr) SWAPUW_IFLE (*(const uint16 *)SYN68K_TO_US (CLEAN (addr)))

#if defined (BIGENDIAN) && defined (QUADALIGN)
# define READUL(addr)                              \
({ const uint16 *_p = SYN68K_TO_US (CLEAN (addr)); \
(uint32) ((_p[0] << 16) | _p[1]);               \
})
# define READUL_US(addr)			\
({ const uint16 *_p = CLEAN (addr);		\
(uint32) ((_p[0] << 16) | _p[1]);		\
})
#elif defined (BIGENDIAN) && !defined (QUADALIGN)
# define READUL(addr) (*(uint32 *) SYN68K_TO_US (CLEAN (addr)))
# define READUL_US(addr) (*(uint32 *) CLEAN (addr))
#elif !defined (BIGENDIAN) && defined (QUADALIGN)

#if defined(__GNUC__)
# define READUL(addr)                                               \
({ const uint8 *_p = (const uint8 *) SYN68K_TO_US (CLEAN (addr));   \
(uint32) ((_p[0] << 24) | (_p[1] << 16) | (_p[2] << 8) | _p[3]); \
})
# define READUL_US(addr)						\
({ const uint8 *_p = (const uint8 *) CLEAN (addr);			\
(uint32) ((_p[0] << 24) | (_p[1] << 16) | (_p[2] << 8) | _p[3]);	\
})
#else
static uint32 _readul(syn68k_addr_t addr)
{
	const uint8 *p;
	
	p = (const uint8 *) SYN68K_TO_US (CLEAN (addr));
	return (uint32) ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

#define READUL(addr)	_readul(addr)
static uint32 _readul_us(syn68k_addr_t addr)
{
	const uint8 *p;
	
	p = (const uint8 *) CLEAN (addr);
	return (uint32) ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

#define READUL_US(addr)	_readul_us(addr)
#endif


#else /* !defined (BIGENDIAN) && !defined (QUADALIGN) */
# define READUL(addr) SWAPUL (*(uint32 *) SYN68K_TO_US (CLEAN (addr)))
# define READUL_US(addr) SWAPUL (*(uint32 *) CLEAN (addr))
#endif
#define READSB(addr) ((int8)  READUB (addr))
#define READSW(addr) ((int16) READUW (addr))
#define READSL(addr) ((int32) READUL (addr))

/* Memory writes. */
#define WRITEUB(addr, val) (*(uint8 *) SYN68K_TO_US (CLEAN (addr)) = (val))

#if defined (BIGENDIAN)
# define WRITEUW(addr, val) (*(uint16 *) SYN68K_TO_US (CLEAN (addr)) = (val))
#else
# define WRITEUW(addr, val) \
(*(uint16 *) SYN68K_TO_US (CLEAN (addr)) = SWAPUW (val))
#endif

#if defined (BIGENDIAN) && defined (QUADALIGN)
# define WRITEUL(addr, val)                               \
do { uint32 _v = (val);                                   \
uint16 *_p = (uint16 *) SYN68K_TO_US (CLEAN (addr)); \
_p[0] = _v >> 16;                                    \
_p[1] = _v;                                          \
} while (0)
#elif defined (BIGENDIAN) && !defined (QUADALIGN)
# define WRITEUL(addr, val) (*(uint32 *) (SYN68K_TO_US (CLEAN (addr))) = (val))
#elif !defined (BIGENDIAN) && defined (QUADALIGN)
# define WRITEUL(addr, val)                             \
do { uint32 _v = (val);                                 \
uint8 *_p = (uint8 *) SYN68K_TO_US (CLEAN (addr)); \
_p[0] = _v >> 24;                                  \
_p[1] = _v >> 16;                                  \
_p[2] = _v >> 8;                                   \
_p[3] = _v;                                        \
} while (0)
#else /* !defined (BIGENDIAN) && !defined (QUADALIGN) */
# define WRITEUL(addr, val) \
(*(uint32 *) SYN68K_TO_US (CLEAN (addr)) = SWAPUL (val))
#endif

#define WRITESB(addr, val)   WRITEUB ((addr), (uint8)  (val))
#define WRITESW(addr, val)   WRITEUW ((addr), (uint16) (val))
#define WRITESL(addr, val)   WRITEUL ((addr), (uint32) (val))

/* Pop instructions. */
#define POPUB()       (EM_A7 += 2, READUB (EM_A7 - 2))
#define POPUW()       (EM_A7 += 2, READUW (EM_A7 - 2))
#define POPUL()       (EM_A7 += 4, READUL (EM_A7 - 4))
#define POPSB()       ((int8)  POPUB ())
#define POPSW()       ((int16) POPUW ())
#define POPSL()       ((int32) POPUL ())
#define POPADDR()     POPUL()

/* Push instructions */
#define PUSHUB(val)    WRITEUB (EM_A7 -= 2, (val))
#define PUSHUW(val)    WRITEUW (EM_A7 -= 2, (val))
#define PUSHUL(val)    WRITEUL (EM_A7 -= 4, (val))
#define PUSHSB(val)    PUSHUB ((uint8)  (val))
#define PUSHSW(val)    PUSHUW ((uint16) (val))
#define PUSHSL(val)    PUSHUL ((uint32) (val))
#define PUSHADDR(val)  PUSHUL (val)

#if !defined (CALL_EMULATOR)
#define CALL_EMULATOR(addr)                \
do {PUSHADDR (MAGIC_EXIT_EMULATOR_ADDRESS); \
(interpret_code (hash_lookup_code_and_create_if_needed (addr)));} while (0)
#endif

/* Global struct describing the CPU state. */
extern CPUState cpu_state;


/* I hate to put this here, but we want our interrupt polling to
 * be extremely fast so we can check for interrupts anywhere in
 * executor that might be time consuming.
 */
#if defined (SYNCHRONOUS_INTERRUPTS)

#define M68K_TIMER_PRIORITY 4
#define M68K_TIMER_VECTOR (24 + M68K_TIMER_PRIORITY)

#define INTERRUPT_STATUS_CHANGED    (-1)
#define INTERRUPT_STATUS_UNCHANGED  0x7FFFFFFF

#if defined (USE_BIOS_TIMER)

extern uint16 dos_memory_selector;
extern uint32 dos_interrupt_flag_addr;

# ifdef SYN68K_C

/* If we're in syn68k.c, we know that %fs holds the dos mem selector value. */
#  define FETCH_INTERRUPT_STATUS()		\
({						\
int32 n;					\
asm ("movl %%fs:(%1),%0"			\
: "=r" (n)				\
: "r" (dos_interrupt_flag_addr));	\
n;						\
})
#  define SET_INTERRUPT_STATUS(n)			\
asm ("movl %0,%%fs:(%1)"				\
: : "g" (n), "r" (dos_interrupt_flag_addr))
# else  /* !SYN68K_C */

/* Temporarily use %fs to reference DOS memory. */
#  define FETCH_INTERRUPT_STATUS()					\
({									\
int32 n;								\
asm ("pushl %%fs\n\t"							\
"movw %2,%%fs\n\t"						\
"movl %%fs:(%1),%0\n\t"						\
"popl %%fs"							\
: "=r" (n)							\
: "r" (dos_interrupt_flag_addr), "g" (dos_memory_selector));	\
n;									\
})
#  define SET_INTERRUPT_STATUS(n)			\
asm ("pushl %%fs\n\t"					\
"movw %2,%%fs\n\t"				\
"movl %0,%%fs:(%1)\n\t"				\
"popl %%fs"					\
: : "g" (n), "r" (dos_interrupt_flag_addr),	\
"g" (dos_memory_selector))
# endif /* !SYN68K_C */

#else /* !USE_BIOS_TIMER */
#define FETCH_INTERRUPT_STATUS() cpu_state.interrupt_status_changed
#define SET_INTERRUPT_STATUS(n) \
((void) (cpu_state.interrupt_status_changed = (n)))

#endif /* !USE_BIOS_TIMER */

#define INTERRUPT_PENDING() (FETCH_INTERRUPT_STATUS () < 0)

#endif /* SYNCHRONOUS_INTERRUPTS */


/* Functions to be called from outside the emulator. */
extern void initialize_68k_emulator (void (*while_busy)(int), int native_p,
									 uint32 trap_vector_storage[64],
									 uint32 dos_int_flag_addr);

#if defined (SYNCHRONOUS_INTERRUPTS)
extern void interrupt_generate (unsigned priority);
extern void interrupt_note_if_present (void);
/* called from `host_interrupt_status_changed' assembly stub in
 host-native.c */
extern syn68k_addr_t interrupt_process_any_pending (syn68k_addr_t pc)
asm ("_interrupt_process_any_pending");
#endif /* SYNCHRONOUS_INTERRUPTS */

extern void interpret_code (const uint16 *code);
/* called from asm; hence the need for the asm label, see
 `host_interrupt_status_changed' stub asm in host-native.c */
extern const uint16 *hash_lookup_code_and_create_if_needed (syn68k_addr_t adr)
asm ("_hash_lookup_code_and_create_if_needed");

extern unsigned long destroy_blocks (syn68k_addr_t low_m68k_address,
									 uint32 num_bytes);
extern syn68k_addr_t callback_install (callback_handler_t func,
									   void *arbitrary_argument);
extern void callback_remove (syn68k_addr_t m68k_address);
extern void trap_install_handler (unsigned trap_number,
								  callback_handler_t func,
								  void *arbitrary_argument);
extern void trap_remove_handler (unsigned trap_number);
extern void *callback_argument (syn68k_addr_t callback_address);
extern callback_handler_t callback_function (syn68k_addr_t callback_address);
extern void dump_profile (const char *file);

#if defined (CHECKSUM_BLOCKS)
extern unsigned long destroy_blocks_with_checksum_mismatch
(syn68k_addr_t low_m68k_address, uint32 num_bytes);
#endif

extern void m68kaddr (const uint16 *pc);

#ifdef __cplusplus
}
#endif

#endif /* Not _syn68k_public_h_ */
