/* Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"

#include <windows.h>

static bool timer_driven_events_p = true;

PUBLIC void
win_queue(volatile uint8 *pendingp)
{
    if(timer_driven_events_p)
        *pendingp = true;
}

PUBLIC void
set_timer_driven_events(bool value)
{
    timer_driven_events_p = value;
}
