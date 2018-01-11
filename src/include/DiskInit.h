#if !defined(__DISKINIT__)
#define __DISKINIT__

#include "QuickDraw.h"

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */
namespace Executor
{
extern void C_DILoad(void);
extern void C_DIUnload(void);
extern INTEGER C_DIBadMount(Point pt, LONGINT evtmess);
extern INTEGER C_dibadmount(Point *ptp, LONGINT evtmess);
extern OSErr C_DIFormat(INTEGER dn);
extern OSErr C_DIVerify(INTEGER dn);
extern OSErr C_DIZero(INTEGER dn, StringPtr vname);
}
#endif /* __DISKINIT__ */
