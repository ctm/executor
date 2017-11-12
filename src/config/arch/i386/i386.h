#if !defined(_ARCH_I386_H_)
#define _ARCH_I386_H_

#define LITTLEENDIAN

#define SYN68K

#if !defined(i386)
#define i386
#endif

/*
 * Technically, it's possible to run Linux on a 386.  It might even
 * be possible to do the same with Windows 95.  However, I don't think
 * we have to support such a thing.  I don't know that we can even compile
 * the GO32 (DOS) version of Executor anymore, but if we can, *perhaps* it
 * makes sense for that to run in a 386.
 */

#if !defined(GO32)
#define ALWAYS_ON_I486
#endif

#include "rsys/types.h"
#include "i386_djgpp_version.h"

#define SWAP16_FUNC_DEFN            \
    inline uint16                   \
    swap16(uint16 n)                \
    {                               \
        return (n >> 8) | (n << 8); \
    }

#if defined(ALWAYS_ON_I486)
#define SWAP32_FUNC_DEFN                                           \
    inline uint32                                                  \
    swap32(uint32 n)                                               \
    {                                                              \
        /* We can use bswap on the i486, but never on the i386. */ \
        __asm__("bswap %k0"                                        \
                : "=r"(n)                                          \
                : "0"(n));                                         \
        return n;                                                  \
    }
#else /* !ALWAYS_ON_I486 */
#define SWAP32_FUNC_DEFN           \
    inline uint32                  \
    swap32(uint32 n)               \
    {                              \
        __asm__("rorw $8,%w0\n\t"  \
                "rorl $16,%k0\n\t" \
                "rorw $8,%w0"      \
                : "=r"(n)          \
                : "0"(n)           \
                : "cc");           \
        return n;                  \
    }
#endif /* !ALWAYS_ON_I486 */

extern inline uint16 swap16(uint16 n) __attribute__((const, always_inline));
extern inline uint32 swap32(uint32 n) __attribute__((const, always_inline));

SWAP16_FUNC_DEFN
SWAP32_FUNC_DEFN

typedef enum {
    ARCH_TYPE_I386,
    ARCH_TYPE_I486
} arch_type_t;

#if defined(ALWAYS_ON_I486)
#define arch_type ARCH_TYPE_I486
#else
extern arch_type_t arch_type;
#endif

#define I386_CC_CARRY_MASK (1 << 0)
#define I386_CC_ZERO_MASK (1 << 6)
#define I386_CC_SIGN_MASK (1 << 7)
#define I386_CC_DIRECTION_MASK (1 << 10)
#define I386_CC_OVERFLOW_MASK (1 << 11)

/* Note that we have an i386 implementation of these routines. */

#if defined(MACOSX)
/*
 * When doing the Proof-of-concept port on Mac OS X, the tricked out
 * blitters were causing Executor to crash.  I haven't figured out
 * why yet, but most likely they can be rehabilitated.
 */
#else /* !defined(MACOSX) */
#define ARCH_PROVIDES_RAW_PATBLT
#define ARCH_PROVIDES_RAW_SRCBLT
#endif /* !defined(MACOSX) */

#endif /* !_ARCH_I386_H_ */
