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

OSErr Executor::TEFromScrap()
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

OSErr Executor::TEToScrap()
{
    int32_t m;

    HLockGuard guard(MR(TEScrpHandle));

    m = PutScrap(CW(TEScrpLength), TICK("TEXT"),
                 STARH(MR(TEScrpHandle)));
    return m < 0 ? m : 0;
}

Handle Executor::TEScrapHandle()
{
    return MR(TEScrpHandle);
}

int32_t Executor::TEGetScrapLen()
{
    return CW(TEScrpLength);
}

void Executor::TESetScrapLen(int32_t ln)
{
    TEScrpLength = CW(ln);
}
