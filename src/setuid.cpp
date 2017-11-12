/* Copyright 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/*
 * This file is necessary on systems where uid_t is a short.  The
 * problem with just calling setuid ourselves is that we'd like
 * to do it from source files that may include .h files that say
 * to pass a short to setuid.  Of course -mshort really allows that
 * to be done, which isn't really what the software is expecting.
 */

#include "rsys/common.h"
#include "rsys/setuid.h"

using namespace Executor;

void ROMlib_setuid(int uid)
{
#if !defined(MSDOS) && !defined(CYGWIN32)
    setuid(uid);
#endif
}

void ROMlib_seteuid(int uid)
{
#if !defined(MSDOS) && !defined(CYGWIN32)
    seteuid(uid);
#endif
}
