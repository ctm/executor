/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"
#include "rsys/error.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "win_keyboard.h"

PUBLIC void
ROMlib_set_caps_lock_off(void)
{
    if(GetKeyState(VK_CAPITAL) & 1)
    {
        char state[256];

        GetKeyboardState(state);
        state[VK_CAPITAL] = 0;
        SetKeyboardState(state);
    }
}
