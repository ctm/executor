#if !defined(_SERIAL_H_)
#define _SERIAL_H_

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
struct sersetbuf_t
{
    GUEST_STRUCT;
    GUEST<Ptr> p;
    GUEST<INTEGER> i;
};

extern OSErr ROMlib_serialopen(ParmBlkPtr pbp, DCtlPtr dcp);
extern OSErr ROMlib_serialprime(ParmBlkPtr pbp, DCtlPtr dcp);
extern OSErr ROMlib_serialctl(ParmBlkPtr pbp, DCtlPtr dcp);
extern OSErr ROMlib_serialstatus(ParmBlkPtr pbp, DCtlPtr dcp);
extern OSErr ROMlib_serialclose(ParmBlkPtr pbp, DCtlPtr dcp);
}
#endif
