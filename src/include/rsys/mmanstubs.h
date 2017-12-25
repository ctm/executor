#if !defined(__RSYS_MMANSTUBS__)
#define __RSYS_MMANSTUBS__
/*
 * Copyright 1990, 1991 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

/* Stubs for calling of register based traps to the memory manager */

#if !defined(BINCOMPAT) || defined(SYN68K)
#error "Only valid for bincompat, on real 680x0 (not Syn68k)"
#endif

#include <rsys/mman_private.h>

extern OSErr MemErr;

#include "rsys/trapdefines.h"

#define TRAPREGS "a0", "d0"
#define JBSRREGS "a0", "a1", "d0", "d1", "d2"

#define NOINTTRAPS

#if !defined(NO_ROMLIB)

static inline void
InitApplZone(void)
{
#if defined(NOINTTRAPS)
    asm("jbsr _R_InitApplZone"
        :
        :
        : JBSRREGS);
#else
    asm(trap_R_InitApplZone
        :
        :
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
SetApplBase(Ptr sp)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_SetApplBase"
        :
        : "g"(sp)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_SetApplBase
        :
        : "g"(sp)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
InitZone(ProcPtr pGrowZone, INTEGER cMoreMasters, Ptr limitPtr,
         Zone *startPtr)
{
    pblock_t pblock;

    pblock.sp = startPtr;
    pblock.lp = limitPtr;
    pblock.mm = cMoreMasters;
    pblock.gz = pGrowZone;
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_InitZone"
        :
        : "g"(&pblock)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_InitZone
        :
        : "g"(&pblock)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
SetApplLimit(Ptr zoneLimit)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_SetApplLimit"
        :
        : "g"(zoneLimit)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_SetApplLimit
        :
        : "g"(zoneLimit)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
MaxApplZone(void)
{
#if defined(NOINTTRAPS)
    asm("jbsr _R_MaxApplZone"
        :
        :
        : JBSRREGS);
#else
    asm(trap_R_MaxApplZone
        :
        :
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
MoreMasters(void)
{
#if defined(NOINTTRAPS)
    asm("jbsr _R_MoreMasters"
        :
        :
        : JBSRREGS);
#else
    asm(trap_R_MoreMasters
        :
        :
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline THz
GetZone(void)
{
    register THz ret asm("a0");

#if defined(NOINTTRAPS)
    asm volatile("jbsr _R_GetZone"
                 : "=g"(ret)
                 :
                 : JBSRREGS);
#else
    asm volatile(trap_R_GetZone
                 : "=g"(ret)
                 :
                 : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
    return ret;
}

static inline void
SetZone(THz hz)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_SetZone"
        :
        : "g"(hz)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_SetZone
        :
        : "g"(hz)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline Handle
NewHandle(Size sz)
{
    register Handle ret asm("a0");

#if defined(NOINTTRAPS)
    asm volatile("movel %1, d0\n\t"
                 "moveq #0, d1\n\t"
                 "jbsr _R_NewHandle"
                 : "=g"(ret)
                 : "g"(sz)
                 : JBSRREGS);
#else
    asm volatile("movel %1, d0\n\t" trap_R_NewHandle
                 : "=g"(ret)
                 : "g"(sz)
                 : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
    return ret;
}

static inline void
DisposHandle(Handle h)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_DisposHandle"
        :
        : "g"(h)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_DisposHandle
        :
        : "g"(h)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline Size
GetHandleSize(Handle h)
{
    register LONGINT ret asm("d0");

#if defined(NOINTTRAPS)
    asm volatile("movel %1, a0\n\t"
                 "jbsr _R_GetHandleSize"
                 : "=g"(ret)
                 : "g"(h)
                 : JBSRREGS);
#else
    asm volatile("movel %1, a0\n\t" trap_R_GetHandleSize
                 : "=g"(ret)
                 : "g"(h)
                 : TRAPREGS);
#endif
    if(ret > 0)
    {
        MemErr = noErr;
        return ret;
    }
    else
    {
        asm("movew d0, %0"
            : "=g"(MemErr));
        return 0;
    }
}

static inline void
SetHandleSize(Handle h, Size sz)
{
#if defined(NOINTTRAPS)
    asm("movel %0, d0\n\t"
        "movel %1, a0\n\t"
        "jbsr _R_SetHandleSize"
        :
        : "g"(sz), "g"(h)
        : JBSRREGS);
#else
    asm("movel %0, d0\n\t"
        "movel %1, a0\n\t" trap_R_SetHandleSize
        :
        : "g"(sz), "g"(h)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline THz
HandleZone(Handle h)
{
    register THz ret asm("a0");

#if defined(NOINTTRAPS)
    asm volatile("movel %1, a0\n\t"
                 "jbsr _R_HandleZone"
                 : "=g"(ret)
                 : "g"(h)
                 : JBSRREGS);
#else
    asm volatile("movel %1, a0\n\t" trap_R_HandleZone
                 : "=g"(ret)
                 : "g"(h)
                 : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
    return ret;
}

static inline Handle
RecoverHandle(Ptr p)
{
    register Handle ret asm("a0");

#if defined(NOINTTRAPS)
    asm volatile("movel %1, a0\n\t"
                 "moveq #0, d1\n\t"
                 "jbsr _R_RecoverHandle"
                 : "=g"(ret)
                 : "g"(p)
                 : JBSRREGS);
#else
    asm volatile("movel %1, a0\n\t" trap_R_RecoverHandle
                 : "=g"(ret)
                 : "g"(p)
                 : "a0"); /* d0 is preserved.  See IMII-35 */
#endif
    return ret;
}

static inline void
ReallocHandle(Handle h, Size sz)
{
#if defined(NOINTTRAPS)
    asm("movel %0, d0\n\t"
        "movel %1, a0\n\t"
        "jbsr _R_ReallocHandle"
        :
        : "g"(sz), "g"(h)
        : JBSRREGS);
#else
    asm("movel %0, d0\n\t"
        "movel %1, a0\n\t" trap_R_ReallocHandle
        :
        : "g"(sz), "g"(h)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline Ptr
NewPtr(Size sz)
{
    register Ptr ret asm("a0");

#if defined(NOINTTRAPS)
    asm volatile("movel %1, d0\n\t"
                 "moveq #0, d1\n\t"
                 "jbsr _R_NewPtr"
                 : "=g"(ret)
                 : "g"(sz)
                 : JBSRREGS);
#else
    asm volatile("movel %1, d0\n\t" trap_R_NewPtr
                 : "=g"(ret)
                 : "g"(sz)
                 : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
    return ret;
}

static inline void
DisposPtr(Ptr p)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_DisposPtr"
        :
        : "g"(p)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_DisposPtr
        :
        : "g"(p)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline Size
GetPtrSize(Ptr p)
{
    register LONGINT ret asm("d0");

#if defined(NOINTTRAPS)
    asm volatile("movel %1, a0\n\t"
                 "jbsr _R_GetPtrSize"
                 : "=g"(ret)
                 : "g"(p)
                 : JBSRREGS);
#else
    asm volatile("movel %1, a0\n\t" trap_R_GetPtrSize
                 : "=g"(ret)
                 : "g"(p)
                 : TRAPREGS);
#endif
    if(ret < 0)
    {
        asm("movew d0, %0"
            : "=g"(MemErr));
        return 0;
    }
    else
    {
        MemErr = 0;
        return ret;
    }
}

static inline void
SetPtrSize(Ptr p, Size sz)
{
#if defined(NOINTTRAPS)
    asm("movel %0, d0\n\t"
        "movel %1, a0\n\t"
        "jbsr _R_SetPtrSize"
        :
        : "g"(sz), "g"(p)
        : JBSRREGS);
#else
    asm("movel %0, d0\n\t"
        "movel %1, a0\n\t" trap_R_SetPtrSize
        :
        : "g"(sz), "g"(p)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline THz
PtrZone(Ptr p)
{
    register THz ret asm("a0");

#if defined(NOINTTRAPS)
    asm volatile("movel %1, a0\n\t"
                 "jbsr _R_PtrZone"
                 : "=g"(ret)
                 : "g"(p)
                 : JBSRREGS);
#else
    asm volatile("movel %1, a0\n\t" trap_R_PtrZone
                 : "=g"(ret)
                 : "g"(p)
                 : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
    return ret;
}

static inline LONGINT
FreeMem(void)
{
    register LONGINT ret asm("d0");

#if defined(NOINTTRAPS)
    asm volatile("moveq #0, d1\n\t"
                 "jbsr _R_FreeMem"
                 : "=g"(ret)
                 :
                 : JBSRREGS);
#else
    asm volatile(trap_R_FreeMem
                 : "=g"(ret)
                 :
                 : TRAPREGS);
#endif
    return ret;
}

static inline Size
MaxMem(Size *growp)
{
    register Size ret asm("d0");
    register Size grow asm("a0");

#if defined(NOINTTRAPS)
    asm volatile("moveq #0, d1\n\t"
                 "jbsr _R_MaxMem"
                 : "=g"(ret), "=g"(grow)
                 :
                 : JBSRREGS);
#else
    asm volatile(trap_R_MaxMem
                 : "=g"(ret), "=g"(grow)
                 :
                 : TRAPREGS);
#endif
    *growp = grow;
    return ret;
}

/*
 * For Excel 3.0a's benefit (perhaps)
 */

#undef NOINTTRAPS

static inline Size
CompactMem(Size needed)
{
    register Size ret asm("d0");

#if defined(NOINTTRAPS)
    asm volatile("movel %1, d0\n\t"
                 "moveq #0, d1\n\t"
                 "jbsr _R_CompactMem"
                 : "=g"(ret)
                 : "g"(needed)
                 : JBSRREGS);
#else
    asm volatile("movel %1, d0\n\t" trap_R_CompactMem
                 : "=g"(ret)
                 : "g"(needed)
                 : TRAPREGS);
#endif
    return ret;
}

#define NOINTTRAPS

static inline void
ResrvMem(Size needed)
{
#if defined(NOINTTRAPS)
    asm("movel %0, d0\n\t"
        "moveq #0, d1\n\t"
        "jbsr _R_ResrvMem"
        :
        : "g"(needed)
        : JBSRREGS);
#else
    asm("movel %0, d0\n\t" trap_R_ResrvMem
        :
        : "g"(needed)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
PurgeMem(Size needed)
{
#if defined(NOINTTRAPS)
    asm("movel %0, d0\n\t"
        "moveq #0, d1\n\t"
        "jbsr _R_PurgeMem"
        :
        : "g"(needed)
        : JBSRREGS);
#else
    asm("movel %0, d0\n\t" trap_R_PurgeMem
        :
        : "g"(needed)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
EmptyHandle(Handle h)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_EmptyHandle"
        :
        : "g"(h)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_EmptyHandle
        :
        : "g"(h)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
HLock(Handle h)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_HLock"
        :
        : "g"(h)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_HLock
        :
        : "g"(h)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
HUnlock(Handle h)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_HUnlock"
        :
        : "g"(h)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_HUnlock
        :
        : "g"(h)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
HPurge(Handle h)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_HPurge"
        :
        : "g"(h)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_HPurge
        :
        : "g"(h)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
HNoPurge(Handle h)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_HNoPurge"
        :
        : "g"(h)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_HNoPurge
        :
        : "g"(h)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
SetGrowZone(ProcPtr gz)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_SetGrowZone"
        :
        : "g"(gz)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_SetGrowZone
        :
        : "g"(gz)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
BlockMove(Ptr src, Ptr dst, Size len)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "movel %1, a1\n\t"
        "movel %2, d0\n\t"
        "jbsr _R_BlockMove"
        :
        : "g"(src), "g"(dst), "g"(len)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t"
        "movel %1, a1\n\t"
        "movel %2, d0\n\t" trap_R_BlockMove
        :
        : "g"(src), "g"(dst), "g"(len)
        : TRAPREGS, "a1");
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
MoveHHi(Handle h)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_MoveHHi"
        :
        : "g"(h)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_MoveHHi
        :
        : "g"(h)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline LONGINT
MaxBlock(void)
{
    register LONGINT ret asm("d0");

#if defined(NOINTTRAPS)
    asm("moveq #0, d1\n\t"
        "jbsr _R_MaxBlock"
        : "=g"(ret)
        :
        : JBSRREGS);
#else
    asm(trap_R_MaxBlock
        : "=g"(ret)
        :
        : TRAPREGS);
#endif
    return ret;
}

static inline void
PurgeSpace(LONGINT *totalp, LONGINT *contigp)
{
    register LONGINT total asm("d0");
    register LONGINT contig asm("a0");

#if defined(NOINTTRAPS)
    asm("moveq #0, d1\n\t"
        "jbsr _R_PurgeSpace"
        : "=g"(total), "=g"(contig)
        :
        : JBSRREGS);
#else
    asm(trap_R_PurgeSpace
        : "=g"(total), "=g"(contig)
        :
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
    *totalp = total;
    *contigp = contig;
}

static inline LONGINT
StackSpace(void)
{
    register LONGINT ret asm("d0");

#if defined(NOINTTRAPS)
    asm("jbsr _R_StackSpace"
        : "=g"(ret)
        :
        : JBSRREGS);
#else
    asm(trap_R_StackSpace
        : "=g"(ret)
        :
        : TRAPREGS);
#endif
    return ret;
}

static inline Handle
NewEmptyHandle(void)
{
    register Handle ret asm("a0");

#if defined(NOINTTRAPS)
    asm("moveq #0, d1\n\t"
        "jbsr _R_NewEmptyHandle"
        : "=g"(ret)
        :
        : JBSRREGS);
#else
    asm(trap_R_NewEmptyHandle
        : "=g"(ret)
        :
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
    return ret;
}

static inline void
HSetRBit(Handle h)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_HSetRBit"
        :
        : "g"(h)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_HSetRBit
        :
        : "g"(h)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline void
HClrRBit(Handle h)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "jbsr _R_HClrRBit"
        :
        : "g"(h)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t" trap_R_HClrRBit
        :
        : "g"(h)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

static inline SignedByte
HGetState(Handle h)
{
    register SignedByte ret asm("d0");

#if defined(NOINTTRAPS)
    asm("movel %1, a0\n\t"
        "jbsr _R_HGetState"
        : "=g"(ret)
        : "g"(h)
        : JBSRREGS);
#else
    asm("movel %1, a0\n\t" trap_R_HGetState
        : "=g"(ret)
        : "g"(h)
        : TRAPREGS);
#endif
    return ret;
}

static inline void
HSetState(Handle h, SignedByte flags)
{
#if defined(NOINTTRAPS)
    asm("movel %0, a0\n\t"
        "moveb %1, d0\n\t"
        "jbsr _R_HSetState"
        :
        : "g"(h), "d"(flags)
        : JBSRREGS);
#else
    asm("movel %0, a0\n\t"
        "moveb %1, d0\n\t" trap_R_HSetState
        :
        : "g"(h), "d"(flags)
        : TRAPREGS);
#endif
    asm("movew d0, %0"
        : "=g"(MemErr));
}

#endif /* !NO_ROMLIB */

#endif !defined(__RSYS_MMANSTUBS__)
