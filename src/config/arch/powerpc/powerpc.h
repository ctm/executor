#if !defined (_ARCH_POWERPC_H_)
#define _ARCH_POWERPC_H_

#define BIGENDIAN

#define SYN68K

#if !defined (POWERPC)
#define POWERPC
#endif

#include "rsys/types.h"

extern uint32 ppc_call (uint32 toc, uint32 (*func)(uint32), uint32 arg);

/*
 * Using __ppc__ to detect that we're going to have problems with the
 * Code Fragment Manager is a "short-term" (heh!) hack to allow us to
 * build the Mac OS X PPC version before switching to the GNU Build
 * system where we can either use configure to figure out if cfm.c
 * will build or have the free time (right!) to figure out what needs
 * to be done to get cfm.c "working" again.
 */

#if defined(__ppc__)
# define CFM_PROBLEMS
#endif


#endif /* !_ARCH_POWERPC_H_ */
