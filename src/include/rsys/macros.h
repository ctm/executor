#if !defined(_MACROS_H_)
#define _MACROS_H_

#if !defined(MAX)
#define MAX(a, b) ({ decltype (a) _maxa = (a); \
		      decltype (b) _maxb = (b); \
		      _maxa > _maxb ? _maxa : _maxb; })
#endif

#if !defined(MIN)
#define MIN(a, b) ({ decltype (a) _mina = (a); \
		      decltype (b) _minb = (b); \
		      _mina < _minb ? _mina : _minb; })
#endif

#if !defined(ABS)
#define ABS(x) ({ decltype (x) _absx = (x);  (_absx < 0) ? -_absx : _absx; })
#endif

#if 0 /* This should be defined by target-os-config.h */
      /* if we define it here, then we get warning messages when we */
      /* include target-os-config.h after this file has been included */
      /* so it's better to just get it from there in the first place */

#if !defined(offsetof)
#define offsetof(t, f) ((int)&((t *)0)->f)
#endif

#endif

#if !defined(NELEM)
#define NELEM(s) (sizeof(s) / sizeof(s)[0])
#endif

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

#if !defined(U)
#define U(c) ((uint8)(c))
#endif

#if !defined(ALLOCABEGIN)
#define ALLOCABEGIN /* nothing */
#define ALLOCA(n) __builtin_alloca(n)
#define ALLOCAEND /* nothing */
#endif

#define str255assign(d, s) \
    (BlockMove((Ptr)(s), (Ptr)(d), (Size)((unsigned char)(s)[0]) + 1))

#define PATASSIGN(dest, src)                             \
    (((uint32 *)(dest))[0] = ((const uint32 *)(src))[0], \
     ((uint32 *)(dest))[1] = ((const uint32 *)(src))[1])

#define PATTERNS_EQUAL_P(p1, p2)                            \
    (((const uint32 *)(p1))[0] == ((const uint32 *)(p2))[0] \
     && ((const uint32 *)(p1))[1] == ((const uint32 *)(p2))[1])

#define T(a, b, c, d) ((((uint32)(uint8)(a)) << 24)   \
                       | (((uint32)(uint8)(b)) << 16) \
                       | (((uint32)(uint8)(c)) << 8)  \
                       | (((uint32)(uint8)(d)) << 0))

#define TICK(str) (((LONGINT)(unsigned char)str[0] << 24) | ((LONGINT)(unsigned char)str[1] << 16) | ((LONGINT)(unsigned char)str[2] << 8) | ((LONGINT)(unsigned char)str[3] << 0))

#if 0
#if !defined(LITTLEENDIAN)

#define TICKX(str) (TICK(str))

#else /* defined(LITTLEENDIAN) */

#define TICKX(str) (((LONGINT)(unsigned char)str[0] << 0) | ((LONGINT)(unsigned char)str[1] << 8) | ((LONGINT)(unsigned char)str[2] << 16) | ((LONGINT)(unsigned char)str[3] << 24))

#endif /* defined(LITTLEENDIAN) */
#else
#define TICKX(str) CLC(TICK(str))
#endif

#if defined(__CHECKER__)
#include <checker.h>

#define BLOCK(body) \
    do              \
    {               \
        body        \
    } while(false)

#define TRANSFER_ASSIGN(dst, src)                          \
    BLOCK({                                                \
        gui_assert(sizeof(dst) == sizeof(src));            \
        chkr_update_stack_seg;                             \
        chkr_memcpy_transfer(&(dst), &(src), sizeof(dst)); \
    })

#define memcpy_transfer(dst, src, size)       \
    BLOCK({                                   \
        chkr_update_stack_seg;                \
        chkr_memcpy_transfer(dst, src, size); \
    })

#define memmove_transfer(dst, src, size)       \
    BLOCK({                                    \
        chkr_update_stack_seg;                 \
        chkr_memmove_transfer(dst, src, size); \
    })

#define check_bitmap(bitmap, perm)                           \
    BLOCK({                                                  \
        chkr_update_stack_seg;                               \
        chkr_setup_stack_trace_info();                       \
        chkr_check_addr(BITMAP_BASEADDR(bitmap),             \
                        (RECT_HEIGHT(&BITMAP_BOUNDS(bitmap)) \
                         * BITMAP_ROWBYTES(bitmap)),         \
                        perm);                               \
    })

#else

/* dummy values, will be ignored */
#define CHKR_RO -1
#define CHKR_WO -1
#define CHKR_RW -1

#define TRANSFER_ASSIGN(dst, src) \
    ((dst) = (src))
#define memcpy_transfer(dst, src, size) \
    memcpy(dst, src, size);
#define memmove_transfer(dst, src, size) \
    memmove(dst, src, size);
#define check_bitmap(pixmap, perm)
#endif

#endif /* !_MACROS_H_ */
