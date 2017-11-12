/* 
 * Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "syswm_vars.h"
#include "syswm_map.h"

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"

#include "sdlscrap.h"

#if defined(CYGWIN32)
#include "win_clip.h"
#endif

/* System dependent variables */
#if defined(__unix__)
/* * */
Display *SDL_Display;
Window SDL_Window;

static int screen_width;
static int screen_height;

int os_current_screen_width(void)
{
    return screen_width;
}

int os_current_screen_height(void)
{
    return screen_height;
}

#elif defined(_WIN32)

#include "win_screen.h"

HWND SDL_Window;

#endif /* OS */

/* Initialize the system dependent variables */
PUBLIC int
sdl_syswm_init(void)
{
    int retval;
    SDL_SysWMinfo info;

    /* Grab the window manager specific information */
    SDL_VERSION(&info.version);
    if(SDL_GetWMInfo(&info) >= 0)
    {
#if defined(__unix__)
/* * */
#if SDL_MAJOR_VERSION == 0 && SDL_MINOR_VERSION < 9
        SDL_Display = info.display;
        SDL_Window = info.window;
#else
        SDL_Display = info.info.x11.display;
        SDL_Window = info.info.x11.window;
#endif
        {
            Screen *screen;

            screen = DefaultScreenOfDisplay(SDL_Display);
            screen_width = WidthOfScreen(screen);
            screen_height = HeightOfScreen(screen);
        }

#elif defined(_WIN32)
        /* * */
        SDL_Window = info.window;

#endif /* OS */
        retval = 0;
    }
    else
    {
        retval = -1;
    }
    return (retval);
}

#if defined(linux)

/* Handle system dependent events */
PUBLIC int
sdl_syswm_event(const SDL_Event *event)
{
    int retval;

#if SDL_MAJOR_VERSION == 0 && SDL_MINOR_VERSION < 9
    switch(event->syswm.msg->xevent.type)
#else
    switch(event->syswm.msg->event.xevent.type)
#endif
    {
        case SelectionRequest:
            export_scrap(event);
            break;
        default:
            break;
    }
    retval = 1; /* NOTE: this looks wrong ... but that's how Sam wrote it */

    return retval;
}

#elif defined(CYGWIN32)

/* Handle system dependent events */
PUBLIC int
sdl_syswm_event(const SDL_Event *event)
{
    int retval;

    retval = 0;
    switch(event->syswm.msg->msg)
    {
        case WM_SYSCOMMAND:
            if(event->syswm.msg->wParam == SC_MAXIMIZE)
            {
                ROMlib_recenter_window();
                retval = 1;
            }
            break;
        case WM_RENDERFORMAT:
            if(event->syswm.msg->wParam == CF_DIB)
                write_pict_as_dib_to_clipboard();
            else if(event->syswm.msg->wParam == ROMlib_executor_format(TICK("PICT")))
                write_pict_as_pict_to_clipboard();
            break;
        case WM_RENDERALLFORMATS:
            write_pict_as_pict_to_clipboard();
            write_pict_as_dib_to_clipboard();
            break;
        default:
            break;
    }

    return retval;
}

#else

/* Handle system dependent events */
PUBLIC int
sdl_syswm_event(const SDL_Event *event)
{
    int retval;

    retval = 0;
    return retval;
}
#endif
