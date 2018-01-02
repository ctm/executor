/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in Disk.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "Disk.h"

using namespace Executor;

OSErr Executor::DiskEject(INTEGER rn)
{
    return paramErr;
}

OSErr Executor::SetTagBuffer(Ptr bp)
{
    return paramErr;
}

OSErr Executor::DriveStatus(INTEGER dn, DrvSts *statp)
{
    return paramErr;
}
