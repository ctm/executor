/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_disk[] = "$Id: disk.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in Disk.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "Disk.h"

using namespace Executor;

A1(PUBLIC, OSErr, DiskEject, INTEGER, rn)
{
    return paramErr;
}

A1(PUBLIC, OSErr, SetTagBuffer, Ptr, bp)
{
    return paramErr;
}

A2(PUBLIC, OSErr, DriveStatus, INTEGER, dn, DrvSts *, statp)
{
    return paramErr;
}
