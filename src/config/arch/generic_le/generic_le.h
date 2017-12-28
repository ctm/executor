#if !defined(__arch_x86_64_h__)
#define __arch_x86_64_h__

#define LITTLEENDIAN
#define SYN68K

/* TODO: only do these if the compiler supports it, check to see if we can
         get a better swap16 w/o using builtin_bswap32 */

#ifdef _MSC_VER
#include <stdlib.h>

#define swap16(v) _byteswap_ushort(v)
#define swap32(v) _byteswap_ulong(v)
#else
#define swap16(v) ((uint16_t)__builtin_bswap16((uint32_t)(v)))
#define swap32(v) ((uint32_t)__builtin_bswap32((int32_t)(v)))
#endif

#endif /* !defined (__arch_x86_64_h__) */
