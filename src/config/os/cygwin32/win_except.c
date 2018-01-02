/* Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"

#include "SegmentLdr.h"
#include "win_except.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

#if defined(SDL)
#include <SDL/SDL.h>
#include "rsys/segment.h"
#include "rsys/launch.h"
#endif

#if defined(__WIN32__) && (__GNUC__ == 2 && __GNUC_MINOR__ < 95)

typedef LONG (*LPTOP_LEVEL_EXCEPTION_FILTER)(struct _EXCEPTION_POINTERS *ExceptionInfo);

LPTOP_LEVEL_EXCEPTION_FILTER STDCALL
SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER
                                lpTopLevelExceptionFilter);

static LONG
my_fault_proc(struct _EXCEPTION_POINTERS *ExceptionInfo)
#else

static CALLBACK LONG my_fault_proc(LPEXCEPTION_POINTERS unused)
#endif
{
    ++ROMlib_uaf;
    if(ROMlib_uaf > 1)
    {
        if(ROMlib_uaf <= 2)
            uninstall_exception_handler();
        exit(1);
    }

#if defined(SDL)
    if(ROMlib_fullscreen_p)
    {
        SDL_Quit();
        ROMlib_exit = true;
    }
#endif
    MessageBox(NULL, "Unexpected Application Failure",
               "Application Failure", MB_OK);

    C_ExitToShell();
}

static LPTOP_LEVEL_EXCEPTION_FILTER old_filter;

void install_exception_handler(void)
{
    old_filter = SetUnhandledExceptionFilter(my_fault_proc);
}

void uninstall_exception_handler(void)
{
    int i;
    static int fatal_signals[] = {
        SIGSEGV,
#ifdef SIGBUS
        SIGBUS,
#endif
#ifdef SIGFPE
        SIGFPE,
#endif
#ifdef SIGQUIT
        SIGQUIT,
#endif
#ifdef SIGPIPE
        SIGPIPE,
#endif
    };

    SetUnhandledExceptionFilter(old_filter);
    /* Set a handler for any fatal signal not already handled */
    for(i = 0; i < (int)NELEM(fatal_signals); ++i)
        signal(fatal_signals[i], SIG_DFL);
}
