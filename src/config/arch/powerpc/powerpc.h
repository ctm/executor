#if !defined (_ARCH_POWERPC_H_)
#define _ARCH_POWERPC_H_

#define BIGENDIAN

#define SYN68K

#if !defined (POWERPC)
#define POWERPC
#endif

#include "rsys/types.h"

extern uint32 ppc_call (uint32 toc, uint32 (*func)(uint32), uint32 arg);


#endif /* !_ARCH_POWERPC_H_ */
