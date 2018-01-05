/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "EventMgr.h"
#include "OSEvent.h"
#include "ADB.h"
#include "rsys/adb.h"
#include "rsys/osevent.h"
#include "rsys/pstuff.h"

#include <stdarg.h>
/*
 * NOTE: most of the code in adb.c was originally written solely to support
 * apeiron (and potentially other games).  Apeiron patches the mouse ADB
 * service routines and makes a mental note of some of the data that it
 * finds in there.  Specifically it examines:
 *
 * a0@(1) bits 0-6 as a signed (bit 6 is sign bit) y delta of the mouse
 * a0@(2) bits 0-6 as a signed (bit 6 is sign bit) x delta of the mouse
 * a0@(1) bit 7 represents the current button state (0 = down)
 * a0@(2) bit 7 must be set
 *
 * We are playing very fast and loose with ADB below, specifically trying to
 * provide just enough information so Apeiron can detect mouse motion and
 * mouse presses and releases.  We realize that this code, especially
 * adb_apeiron_hack and the front-end code that calls it, have many problems:
 *
 *     We "support" the mouse, but not the keyboard
 *     We count on the installed driver not modifying the data
 *     We count on the installed driver not caring about other low-memory
 *         globals (i.e. we update them before the "driver" is called)
 *     The stub driver that we give them never does any work, so it can't
 *         be used to simulate mouse activity
 *     Probably tons more; the hack is so incomplete that it doesn't even
 *         make sense to think of all the ways it is incomplete
 */

using namespace Executor;

void
Executor::ADBReInit(void)
{
    warning_unimplemented(NULL_STRING);
}

OSErr
Executor::ADBOp(Ptr data, ProcPtr procp, Ptr buffer, INTEGER command)
{
    warning_unimplemented(NULL_STRING);
    return noErr;
}

INTEGER
Executor::CountADBs(void)
{
    warning_unimplemented(NULL_STRING);
    return 1;
}

enum
{
    SPOOFED_MOUSE_ADDR = 3
};

static GUEST<Ptr> adb_service_procp = nullptr; /* stored as though in lowglobal */
static Ptr adb_data_ptr = (Ptr)0x90ABCDEF;

void
Executor::C_adb_service_stub(void)
{
}

void
Executor::reset_adb_vector(void)
{
    adb_service_procp = nullptr;
}

OSErr
Executor::GetIndADB(ADBDataBlock *adbp, INTEGER index)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    if(index != 1)
        retval = -1;
    else
    {
        adbp->devType = CB(0); /* should check on Mac to see what mouse is */
        adbp->origADBAddr = CB(SPOOFED_MOUSE_ADDR);
        if(!adb_service_procp)
            adb_service_procp = RM((Ptr)P_adb_service_stub);
        adbp->dbServiceRtPtr = adb_service_procp;
        adbp->dbDataAreaAddr = RM(adb_data_ptr);
        retval = SPOOFED_MOUSE_ADDR;
    }
    return retval;
}

OSErr
Executor::GetADBInfo(ADBDataBlock *adbp, INTEGER address)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    if(address != SPOOFED_MOUSE_ADDR)
        retval = -1;
    else
        retval = GetIndADB(adbp, 1);
    return retval;
}

OSErr
Executor::SetADBInfo(ADBSetInfoBlock *adbp, INTEGER address)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    if(address != SPOOFED_MOUSE_ADDR)
        retval = -1;
    else
    {
        adb_service_procp = adbp->siServiceRtPtr;
        adb_data_ptr = MR(adbp->siDataAreaAddr);
        retval = noErr;
    }
    return retval;
}

static bool
adb_vector_is_not_our_own(void)
{
    return adb_service_procp && adb_service_procp != RM((Ptr)P_adb_service_stub);
}

static void
call_patched_adb_vector(char *message)
{
    uint32_t save_d0, save_a0;

    save_d0 = EM_D0;
    save_a0 = EM_A0;
    EM_D0 = SPOOFED_MOUSE_ADDR << 4; /* based on Apeiron's code */
    EM_A0 = US_TO_SYN68K(message);
    CALL_EMULATOR((syn68k_addr_t)CL(guest_cast<uint32_t>(adb_service_procp)));
    EM_D0 = save_d0;
    EM_A0 = save_a0;
}

static inline int
pin(int val, int min, int max)
{
    int retval;

    if(val < min)
        retval = min;
    else if(val > max)
        retval = max;
    else
        retval = val;
    return retval;
}

enum
{
    LENGTH_OFFSET = 0,
    Y_OFFSET = 1,
    X_OFFSET = 2,
    MOUSE_OFFSET = 1
};
enum
{
    BUTTON_UP_BIT = 0x80
};

/*
 * if deltas_p is true, dx and dy are supplied as arguments, otherwise they
 * have to be computed as deltas off the last known LM(MouseLocation).  Eventually
 * we should probably always have them be supplied as arguments since we
 * should probably make our mouse handling more true to real life.
 */

// NOTE: deltas_p should be a bool, but is passed as int,
//       because va_start requires it.

void
Executor::adb_apeiron_hack(int /*bool*/ deltas_p, ...)
{
    static bool been_here = false;
    static long old_x;
    static long old_y;
    long x, y;
    bool button_is_down;
    char message[3];

    x = CW(LM(MouseLocation).h);
    y = CW(LM(MouseLocation).v);
    button_is_down = !(ROMlib_mods & btnState);

    /* begin code for PegLeg */

    if(button_is_down)
        LM(MBState) = 0;
    else
        LM(MBState) = 0xFF;

    LM(MTemp).h = LM(MouseLocation).h;
    LM(MTemp).v = LM(MouseLocation).v;

    /* end code for PegLeg */

    LM(MouseLocation2) = LM(MouseLocation);

    if(!been_here)
    {
        old_x = x;
        old_y = y;
        been_here = true;
    }
    if(adb_vector_is_not_our_own())
    {
        int dx, dy;

        if(deltas_p)
        {
            va_list ap;

            va_start(ap, deltas_p);
            dx = va_arg(ap, int);
            dy = va_arg(ap, int);
            va_end(ap);
        }
        else
        {
            dx = x - old_x;
            dy = y - old_y;
        }

        do
        {
            int this_dx, this_dy;

            this_dx = pin(dx, -64, 63);
            this_dy = pin(dy, -64, 63);
            message[LENGTH_OFFSET] = sizeof(message) - 1;
            message[X_OFFSET] = (this_dx) | 0x80; /* Apeiron expects the high */
            message[Y_OFFSET] = (this_dy) | 0x80; /*   bit to be set.         */
            if(button_is_down)
                message[MOUSE_OFFSET] &= ~BUTTON_UP_BIT;
            else
                message[MOUSE_OFFSET] |= BUTTON_UP_BIT;
            call_patched_adb_vector(message);
            dx -= this_dx;
            dy -= this_dy;
        } while(dx || dy);
    }
    old_x = x;
    old_y = y;
}
