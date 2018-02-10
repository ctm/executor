#if !defined(__DISKINIT__)
#define __DISKINIT__

#include "QuickDraw.h"

#define MODULE_NAME DiskInit
#include <rsys/api-module.h>

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */
namespace Executor
{
DISPATCHER_TRAP(Pack2, 0xA9E9, StackW);

extern void C_DILoad(void);
PASCAL_SUBTRAP(DILoad, 0xA9E9, 0x0002, Pack2);

extern void C_DIUnload(void);
PASCAL_SUBTRAP(DIUnload, 0xA9E9, 0x0004, Pack2);

extern INTEGER C_DIBadMount(Point pt, LONGINT evtmess);
PASCAL_SUBTRAP(DIBadMount, 0xA9E9, 0x0000, Pack2);

extern INTEGER C_dibadmount(Point *ptp, LONGINT evtmess);

extern OSErr C_DIFormat(INTEGER dn);
PASCAL_SUBTRAP(DIFormat, 0xA9E9, 0x0006, Pack2);

extern OSErr C_DIVerify(INTEGER dn);
PASCAL_SUBTRAP(DIVerify, 0xA9E9, 0x0008, Pack2);

extern OSErr C_DIZero(INTEGER dn, StringPtr vname);
PASCAL_SUBTRAP(DIZero, 0xA9E9, 0x000A, Pack2);

}
#endif /* __DISKINIT__ */
