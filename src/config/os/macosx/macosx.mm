#define _block_h_
#include <Cocoa/Cocoa.h>

static bool inited = false;
static bool dock = false;
void macosx_hide_menu_bar(int mouseY)
{
    // Apple says that if you hide the menu bar, you *have* to hide the dock
    // as well. Otherwise, you get an assertion failure.
    // I don't understand why they insist on that.
    // Executor would have no problem with the Dock, but we have our own
    // menu bar, so the menu bar has to be hidden.
    // As a hack, we'll switch between Autohidden dock and menu bar and
    // truly hidden dock and menu bar depending on the mouse Y coordinate.
    // If the mouse is near the menu bar, we set things to "hide" so the
    // menu bar doesn't pop up.
    // If the mouse is elsewhere, we set things to "autohide" so the user
    // can access the dock.

    if(!inited || (mouseY < 20 && dock))
    {
        [NSApp setPresentationOptions: NSApplicationPresentationHideMenuBar
                | NSApplicationPresentationHideDock];
        dock = false;
    }
    else if(!inited || (mouseY > 30 && !dock))
    {
        [NSApp setPresentationOptions: NSApplicationPresentationAutoHideMenuBar
            | NSApplicationPresentationAutoHideDock];
        dock = true;
    }
    inited = true;
}
