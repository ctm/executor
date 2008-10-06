#if !defined (_ARCH_ALPHA_H_)
#define _ARCH_ALPHA_H_

#include <netinet/in.h>

#define LITTLEENDIAN

#define SYN68K

#if !defined (ALPHA)
# define ALPHA
#endif

#include "rsys/types.h"


#define SWAP16_FUNC_DEFN				\
uint16 const						\
swap16 (uint16 n)					\
{							\
  return (n >> 8) | (n << 8);				\
}

#define SWAP32_FUNC_DEFN				\
uint32 const						\
swap32 (uint32 n)					\
{							\
  return htonl(n);					\
}

extern __inline__ SWAP16_FUNC_DEFN
extern __inline__ SWAP32_FUNC_DEFN

#endif /* !_ARCH_ALPHA_H_ */
