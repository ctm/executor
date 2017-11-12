#if !defined(_WIN32)

/*
 * Yuck -- even though we're SDL based, we need to know about X, since if we
 * don't call XInitThreads() we can get hammered as we process the scrap in
 * the SDL callback.
 */

#include <X11/Xlib.h>
#include "sdlX.h"

void ROMlib_XInitThreads(void)
{
    XInitThreads();
}

#endif
