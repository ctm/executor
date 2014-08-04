#if !defined (__arch_x86_64_h__)
#define __arch_x86_64_h__

#define LITTLEENDIAN
#define SYN68K

/* TODO: only do these if the compiler supports it, check to see if we can
         get a better swap16 w/o using builtin_bswap32 */

#define swap16(v) ((uint16_t) (__builtin_bswap16 ((int16_t)(v))))

#define swap32(v)((uint32_t) __builtin_bswap32 ((int32_t)(v)))

typedef enum {
	ARCH_TYPE_I386,
	ARCH_TYPE_I486
} arch_type_t;

#define arch_type ARCH_TYPE_I486


#endif /* !defined (__arch_x86_64_h__) */
