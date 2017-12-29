#if !defined(__PACKAGE__)
#define __PACKAGE__

/*
 * Copyright 1986, 1989, 1990, 1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
#define dskInit 2
#define stdFile 3
#define flPoint 4
#define trFunc 5
#define intUtil 6
#define bdConv 7

#if 0
#if !defined(AppPacks_H)
extern GUEST<Handle> AppPacks_H[8];
#endif
#endif

extern pascal trap void C_InitPack(INTEGER packid);

extern pascal trap void C_InitAllPacks(void);

}
#endif /* __PACKAGE__ */
