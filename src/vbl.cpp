/* Copyright 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in VRetraceMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "OSUtil.h"
#include "TimeMgr.h"
#include "VRetraceMgr.h"
#include "DeviceMgr.h"
#include "SoundDvr.h"
#include "OSEvent.h"
#include "ToolboxEvent.h"

#include "rsys/vbl.h"
#include "rsys/time.h"
#include "rsys/hook.h"

#include "rsys/soundopts.h"
#include "rsys/prefs.h"

using namespace Executor;

typedef enum {
    CLOCKOFF = 0,
    CLOCKTEMPON,
    CLOCKON,
    CLOCKTEMPONSOUND,
    CLOCKONSOUND,
} clockstatus_t;

int Executor::ROMlib_clock = CLOCKOFF;

/*
 * ROMlib_vcatch is called by the time manager, hence it doesn't need to
 * block signals
 */

/*
 * NOTE: we put a shim in the VBL queue, because Prince of Persia
 *       has a bug in it that overlooks the first element in the queue
 *       when it's looking for itself!  We put in a magic ptr value we
 *       can recognize in case some bozo actually ever tries to call it.
 *       Note, however, that VInstall fills this in with a pointer to
 *       a m68k RTS instruction, just to be safe.
 */

VBLTask vblshim = { nullptr, CWC(vType), guest_cast<ProcPtr>(CLC(0xAEDCBA98)), CWC(0), CWC(0) };

static TMTask vbltm;

#define VBUSY (1 << 6)

void Executor::C_ROMlib_vcatch()
{
    VBLTaskPtr vp, next;
    ULONGINT old_ticks, new_ticks, ticks_elapsed;
    LONGINT msecs_to_next_tick;
    CCRElement saved_ccnz, saved_ccn, saved_ccc, saved_ccv, saved_ccx;
    M68kReg saved_regs[16];

    /* We save the 68k registers away.  This isn't necessary if we can
   * only be called from the time manager, since it has already saved
   * all of the registers.
   */
    memcpy(saved_regs, &cpu_state.regs, sizeof saved_regs);
    saved_ccnz = cpu_state.ccnz;
    saved_ccn = cpu_state.ccn;
    saved_ccc = cpu_state.ccc;
    saved_ccv = cpu_state.ccv;
    saved_ccx = cpu_state.ccx;

    EM_A7 = (EM_A7 - 32) & ~3; /* Might as well long-align it. */

    /* Save the old LM(Ticks) value & compute new value. */
    old_ticks = CL(LM(Ticks));
    new_ticks = C_TickCount();
    ticks_elapsed = new_ticks - old_ticks;

    /* Compute the milliseconds until LM(Ticks) changes again.
   * Here we rely on the fact that 1000 / 60 == 50 / 3, to avoid overflow.
   */
    msecs_to_next_tick = (((new_ticks + 1) * 50 + 2) / 3) - msecs_elapsed();
    if(msecs_to_next_tick <= 0)
        msecs_to_next_tick = 1;

    PrimeTime((QElemPtr)&vbltm, msecs_to_next_tick);

    for(vp = (VBLTaskPtr)MR(LM(VBLQueue).qHead); vp; vp = next)
    {
        INTEGER old_vbl_count, new_vbl_count;

        next = (VBLTaskPtr)MR(vp->qLink);

        /* Blow off the Prince of Persia shim. */
        if(vp == &vblshim)
            /*-->*/ continue;

        /* Account for possible missed ticks by possibly subtracting
       * off more than one tick from the VBL count. */
        old_vbl_count = CW(vp->vblCount);
        new_vbl_count = old_vbl_count - ticks_elapsed;
        if(old_vbl_count > 0 && new_vbl_count < 0)
            new_vbl_count = 0; /* Only compensate for zero crossings. */

        vp->vblCount = CW(new_vbl_count);
        if(new_vbl_count == 0)
        {
            LM(VBLQueue).qFlags.raw_or(CWC(VBUSY));
            ROMlib_hook(vbl_number);

            /* No need to save/restore syn68k regs here; we do that
	   * outside the loop.
	   */

            EM_A0 = US_TO_SYN68K_CHECK0(vp);
            EM_A1 = CL(guest_cast<LONGINT>(vp->vblAddr));

            CALL_EMULATOR((syn68k_addr_t)EM_A1);

            LM(VBLQueue).qFlags.raw_and(CWC(~VBUSY));
            if(vp->vblCount == CWC(0))
                Dequeue((QElemPtr)vp, &LM(VBLQueue));
        }
    }

    memcpy(&cpu_state.regs, saved_regs, sizeof saved_regs);
    cpu_state.ccnz = saved_ccnz;
    cpu_state.ccn = saved_ccn;
    cpu_state.ccc = saved_ccc;
    cpu_state.ccv = saved_ccv;
    cpu_state.ccx = saved_ccx;
}

static void
startclock(void)
{
    vbltm.tmAddr = RM((ProcPtr)&ROMlib_vcatch);
    InsTime((QElemPtr)&vbltm);
    PrimeTime((QElemPtr)&vbltm, 17);
    /* don't adjust ROMlib_clock; it could be temporary */
}

static void
stopclock(void)
{
    if(!LM(VBLQueue).qTail)
    {
        RmvTime((QElemPtr)&vbltm);
        ROMlib_clock = CLOCKOFF;
    }
    else
        ROMlib_clock = CLOCKTEMPON;
}

void Executor::ROMlib_clockonoff(LONGINT onoroff)
{
    if(onoroff)
    {
        if(ROMlib_clock == CLOCKOFF)
            startclock();
        ROMlib_clock = CLOCKON;
    }
    else
        stopclock();
}

OSErr Executor::SlotVInstall(VBLTaskPtr vtaskp, INTEGER slot)
{
    return VInstall(vtaskp);
}

OSErr Executor::VInstall(VBLTaskPtr vtaskp)
{
    static GUEST<uint16_t> m68k_rts = CWC(0x4E75); /* RTS */

    vtaskp->vblCount = CW(CW(vtaskp->vblCount) + CW(vtaskp->vblPhase));
    if(vtaskp->qType == CWC((INTEGER)vType))
    {
        if(!LM(VBLQueue).qHead)
        {
            vblshim.vblAddr = RM((ProcPtr)&m68k_rts); /* legal 68k code. */
            Enqueue((QElemPtr)&vblshim, &LM(VBLQueue));
        }
        Enqueue((QElemPtr)vtaskp, &LM(VBLQueue));
        if(ROMlib_clock == CLOCKOFF)
        {
            ROMlib_clock = CLOCKTEMPON;
            startclock();
        }
        /*-->*/ return noErr;
    }
    else
        return vTypErr;
}

OSErr Executor::SlotVRemove(VBLTaskPtr vtaskp, INTEGER slot)
{
    return VRemove(vtaskp);
}

OSErr Executor::VRemove(VBLTaskPtr vtaskp)
{
    if(vtaskp->qType == CWC((INTEGER)vType))
    {
        Dequeue((QElemPtr)vtaskp, &LM(VBLQueue));
        if((VBLTaskPtr)MR(LM(VBLQueue).qHead) == &vblshim
           && (VBLTaskPtr)MR(LM(VBLQueue).qTail) == &vblshim)
        {
            Dequeue((QElemPtr)&vblshim, &LM(VBLQueue));
            if(ROMlib_clock == CLOCKTEMPON)
                stopclock();
        }
        /*-->*/ return noErr;
    }
    else
        return vTypErr;
}

QHdrPtr Executor::GetVBLQHdr()
{
    return &LM(VBLQueue);
}
