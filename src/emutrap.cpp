/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include <stdarg.h>
#include "rsys/trapglue.h"
#include "rsys/mixed_mode.h"

using namespace Executor;

/*
 * NOTE: the code below is "mostly" portable.  It relies on all arguments
 *	 being passed in the same size (we don't ever pass doubles or
 *	 long longs) and it also relies on the ability to pick up a return
 *	 value as a LONGINT, even if it is a pointer return value.  The latter
 *	 definitely wouldn't work with some early eighties C compilers, but
 *	 to heck with them.
 */

/*
 * The first time this code was running on a PowerPC was on an old
 * version of Yellow Dog Linux.  The calling conventions were
 * different than what we get when compiling on Mac OS X.  As such,
 * there are some places where we check for the preprocessing macro
 * "powerpc" and are really concerned with the old calling convention.
 * There are othe times when we check for either powerpc or __ppc__ in
 * which case the calling convention shouldn't matter.
 *
 * TODO: switch to a modern version of the GNU build system and use it
 *       to determine calling convention issues.
 *
 * The Yellow Dog version was actually able to run a few PPC binaries,
 * including (IIRC) the demo versions of a couple of Adobe products
 * (Photoshop and Illustrator?).  However, none of the code fragment
 * manager or mixed-mode code has been tested in years and never under
 * Mac OS X.  In the incredibly unlikely circumstance that someone
 * wants to play with that code and has questions, I'll be happy to
 * answer them to the best of my ability, but it was a long time ago
 * and was short-lived, so my memory is a bit weak.
 *
 */

#if defined(powerpc) || defined(__ppc__)

static long
CToRoutineDescriptorCall(const RoutineDescriptor *p, unsigned long long magic,
                         va_list ap)
{
    uint32_t args[11];
    int n_args;
    ProcInfoType procinfo;
    UniversalProcPtr up;
    uint32_t *argsp;
    long retval;
    int retvaltype;

    procinfo = kCStackBased;

    argsp = args;
    up = (UniversalProcPtr)p;
    n_args = 0;
    retvaltype = magic & 7;

    switch(retvaltype) /* procinfo to take return value into consideration */
    {
        case 0:
            procinfo |= RESULT_SIZE(kNoByteCode);
            break;
        case 1:
            procinfo |= RESULT_SIZE(kOneByteCode);
            break;
        case 2:
            procinfo |= RESULT_SIZE(kTwoByteCode);
            break;
        case 3:
        case 4:
        case 5:
            procinfo |= RESULT_SIZE(kFourByteCode);
            break;
    }

    /* we can use this while loop because we don't need to know the return
     type, so we don't need the first 3 bits, and each time through the
     argument type is another 3 bits */

    while(magic >>= 3)
    {
        uint32_t arg;

        ++n_args;
        switch(magic & 7)
        {
            case 1:
            {
                arg = (uint8_t)va_arg(ap, unsigned long);
                arg = CB(arg);
                procinfo |= STACK_ROUTINE_PARAMETER(n_args, kOneByteCode);
            }
            break;
            case 2:
            {
                arg = (uint16_t)va_arg(ap, unsigned long);
                arg = CW(arg);
                procinfo |= STACK_ROUTINE_PARAMETER(n_args, kTwoByteCode);
            }
            break;
            case 3: /* point */
            {
                arg = (uint32_t)va_arg(ap, unsigned long);
#if defined(powerpc)
                arg = *(uint32_t *)arg;
#endif
                arg = (CW((uint16_t)arg) | (CW(arg >> 16) << 16));
                procinfo |= STACK_ROUTINE_PARAMETER(n_args, kFourByteCode);
            }
            break;
            case 4:
            {
                arg = (uint32_t)va_arg(ap, unsigned long);
                arg = CL(arg);
                procinfo |= STACK_ROUTINE_PARAMETER(n_args, kFourByteCode);
            }
            break;
            case 5:
            {
                arg = (uint32_t)va_arg(ap, unsigned long);
                arg = (uint32_t)SYN68K_TO_US_CHECK0_CHECKNEG1(arg);
                procinfo |= STACK_ROUTINE_PARAMETER(n_args, kFourByteCode);
            }
            break;
            default:
                warning_unexpected("%d", (int)magic & 7);
                arg = 0;
                break;
        }
        *argsp++ = arg;
    }

    switch(n_args)
    {
        case 0:
            retval = CallUniversalProc_from_native(up, procinfo);
            break;
        case 1:
            retval = CallUniversalProc_from_native(up, procinfo, args[0]);
            break;
        case 2:
            retval = CallUniversalProc_from_native(up, procinfo, args[0], args[1]);
            break;
        case 3:
            retval = CallUniversalProc_from_native(up, procinfo, args[0], args[1],
                                                   args[2]);
            break;
        case 4:
            retval = CallUniversalProc_from_native(up, procinfo, args[0], args[1],
                                                   args[2], args[3]);
            break;
        case 5:
            retval = CallUniversalProc_from_native(up, procinfo, args[0], args[1],
                                                   args[2], args[3], args[4]);
            break;
        case 6:
            retval = CallUniversalProc_from_native(up, procinfo, args[0], args[1],
                                                   args[2], args[3], args[4],
                                                   args[5]);
            break;
        case 7:
            retval = CallUniversalProc_from_native(up, procinfo, args[0], args[1],
                                                   args[2], args[3], args[4],
                                                   args[5], args[6]);
            break;
        case 8:
            retval = CallUniversalProc_from_native(up, procinfo, args[0], args[1],
                                                   args[2], args[3], args[4],
                                                   args[5], args[6], args[7]);
            break;
        case 9:
            retval = CallUniversalProc_from_native(up, procinfo, args[0], args[1],
                                                   args[2], args[3], args[4],
                                                   args[5], args[6], args[7],
                                                   args[8]);
            break;
        case 10:
            retval = CallUniversalProc_from_native(up, procinfo, args[0], args[1],
                                                   args[2], args[3], args[4],
                                                   args[5], args[6], args[7],
                                                   args[8], args[9]);
            break;
        case 11:
            retval = CallUniversalProc_from_native(up, procinfo, args[0], args[1],
                                                   args[2], args[3], args[4],
                                                   args[5], args[6], args[7],
                                                   args[8], args[9], args[10]);
            break;
#if !defined(LETGCCWAIL)
        default:
            retval = 0;
#endif
    }
    switch(retvaltype)
    {
        case 1:
            retval = CB(retval);
            break;
        case 2:
            retval = CW(retval);
            break;
        case 4:
            retval = CL(retval);
            break;
        case 5:
            retval = (long)SYN68K_TO_US_CHECK0_CHECKNEG1(retval);
            break;
    }
    return retval;
}

static bool
is_routine_descriptor_ptr(uint16_t *addr)
{
    bool retval;

    retval = (*addr == (uint16_t)CWC(MIXED_MODE_TRAP));
    return retval;
}
#endif
