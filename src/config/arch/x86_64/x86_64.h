#if !defined (__arch_x86_64_h__)
#define __arch_x86_64_h__

#define LITTLEENDIAN
#define SYN68K

/* TODO: only do these if the compiler supports it, check to see if we can
         get a better swap16 w/o using builtin_bswap32 */

#define swap16(v) ((uint16_t) (__builtin_bswap32 ((int32_t) (v)) >> 16))

#define swap32(v)((uint32_t) __builtin_bswap32 ((int32_t)(v)))

#endif /* !defined (__arch_x86_64_h__) */
