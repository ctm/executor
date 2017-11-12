#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES
#include "rsys/common.h"

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "sdlquit.h"
#include "rsys/options.h"
#include "rsys/toolevent.h"

#if defined(_WIN32)

PRIVATE int
os_specific_really_quit(void)
{
    /* This should really be an internal ROMlib dialog */
    int retval;
    int reply;
    char *mess;
    char *name;

/* would be nice to use snprintf here, but mingw32 doesn't provide it */
#define FORMAT_STR "Terminate %s?"

    name = ROMlib_WindowName ? ROMlib_WindowName : "application";
    mess = alloca(sizeof FORMAT_STR + strlen(name));
    sprintf(mess, FORMAT_STR, name);

    reply = MessageBox(NULL, mess, "Terminate?", MB_OKCANCEL);

    retval = reply == IDOK;
    return retval;
}

#else

PRIVATE int
os_specific_really_quit(void)
{
    int retval;

    retval = true;
    return retval;
}

#endif

/* Query the user as to whether we should really quit */
int sdl_really_quit(void)
{
    int retval;

    if(!(ROMlib_options & ROMLIB_CLOSE_IS_QUIT_BIT))
        retval = os_specific_really_quit();
    else
    {
        ROMlib_send_quit();
        retval = false;
    }

    return retval;
}
