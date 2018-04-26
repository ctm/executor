#if !defined(__TIMEMGR__)
#define __TIMEMGR__

/*
 * Copyright 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 */

#include "ExMacTypes.h"

#define MODULE_NAME TimeMgr
#include <rsys/api-module.h>

namespace Executor
{
struct TMTask
{
    GUEST_STRUCT;
    GUEST<QElemPtr> qLink;
    GUEST<INTEGER> qType;
    GUEST<ProcPtr> tmAddr;
    GUEST<LONGINT> tmCount; /* I don't trust IMIV-301 */
};

extern void InsTime(QElemPtr taskp);
REGISTER_TRAP2(InsTime, 0xA058, void(A0), ClearD0);
extern void RmvTime(QElemPtr taskp);
REGISTER_TRAP2(RmvTime, 0xA059, void(A0), ClearD0);
extern void PrimeTime(QElemPtr taskp, LONGINT count);
REGISTER_TRAP2(PrimeTime, 0xA05A, void(A0,D0), ClearD0);
}
#endif /* __TIMEMGR__ */
