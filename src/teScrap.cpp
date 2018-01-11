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

    m = GetScrap(MR(LM(TEScrpHandle)), TICK("TEXT"), &l);
    if(m < 0)
    {
        EmptyHandle(MR(LM(TEScrpHandle)));
        LM(TEScrpLength) = CWC(0);
    }
    else
        LM(TEScrpLength) = CW(m);
    return m < 0 ? m : noErr;
}

OSErr Executor::TEToScrap()
{
    int32_t m;

    HLockGuard guard(MR(LM(TEScrpHandle)));

    m = PutScrap(CW(LM(TEScrpLength)), TICK("TEXT"),
                 STARH(MR(LM(TEScrpHandle))));
    return m < 0 ? m : 0;
}

Handle Executor::TEScrapHandle()
{
    return MR(LM(TEScrpHandle));
}

int32_t Executor::TEGetScrapLen()
{
    return CW(LM(TEScrpLength));
}

void Executor::TESetScrapLen(int32_t ln)
{
    LM(TEScrpLength) = CW(ln);
}
