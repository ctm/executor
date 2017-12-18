/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in TextEdit.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "TextEdit.h"
#include "MemoryMgr.h"
#include "ScrapMgr.h"

#include "rsys/cquick.h"
#include "rsys/mman.h"

using namespace Executor;

A0(PUBLIC, OSErr, TEFromScrap)
{
    GUEST<int32_t> l;
    int32_t m;

    m = GetScrap(MR(TEScrpHandle), TICK("TEXT"), &l);
    if(m < 0)
    {
        EmptyHandle(MR(TEScrpHandle));
        TEScrpLength = CWC(0);
    }
    else
        TEScrpLength = CW(m);
    return m < 0 ? m : noErr;
}

A0(PUBLIC, OSErr, TEToScrap)
{
    int32_t m;

    HLockGuard guard(MR(TEScrpHandle));

    m = PutScrap(CW(TEScrpLength), TICK("TEXT"),
                 STARH(MR(TEScrpHandle)));
    return m < 0 ? m : 0;
}

A0(PUBLIC, Handle, TEScrapHandle)
{
    return MR(TEScrpHandle);
}

A0(PUBLIC, int32_t, TEGetScrapLen)
{
    return CW(TEScrpLength);
}

A1(PUBLIC, void, TESetScrapLen, int32_t, ln)
{
    TEScrpLength = CW(ln);
}
