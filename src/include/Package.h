#if !defined(__PACKAGE__)
#define __PACKAGE__

/*
 * Copyright 1986, 1989, 1990, 1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
enum
{
    dskInit = 2,
    stdFile = 3,
    flPoint = 4,
    trFunc = 5,
    intUtil = 6,
    bdConv = 7,
};

#if 0
#if !defined(AppPacks_H)
extern GUEST<Handle> AppPacks_H[8];
#endif
#endif

extern void C_InitPack(INTEGER packid);
PASCAL_TRAP(InitPack, 0xA9E5);

extern void C_InitAllPacks(void);
PASCAL_TRAP(InitAllPacks, 0xA9E6);
}
#endif /* __PACKAGE__ */
