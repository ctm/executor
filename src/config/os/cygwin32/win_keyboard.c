/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_win_keyboard[] = "$Id: win_keyboard.c 63 2004-12-24 18:19:43Z ctm $";
#endif

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
