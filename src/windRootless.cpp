#include <rsys/wind.h>
#include <rsys/vdriver.h>
#include "MenuMgr.h"

#include <iostream>
#include <vector>

using namespace Executor;

static std::vector<Rect> rootlessMenus;

// #define LOG_ROOTLESS

void Executor::ROMlib_rootless_update(RgnHandle extra)
{
#ifdef LOG_ROOTLESS
    std::cout << "ROMlib_rootless_update\n";
#endif
#ifdef VDRIVER_ROOTLESS
    RgnHandle rgn = NewRgn();
    SetRectRgn(rgn, 0,0, CW(screenBitsX.bounds.right), CW(LM(MBarHeight)));

    for(WindowPeek wp = MR(LM(WindowList)); wp; wp = WINDOW_NEXT_WINDOW(wp))
    {
        if(WINDOW_VISIBLE_X(wp))
        {
            UnionRgn(rgn, WINDOW_STRUCT_REGION(wp), rgn);
        }
    }
    RgnHandle tmp = NewRgn();
    for(Rect r : rootlessMenus)
    {   // N.B: do not declare "r" as Rect&,
        // as we aren't allowed to pass pointers into vectors(host heap)
        // as parameters to toolbox functions
        RectRgn(tmp, &r);
        UnionRgn(rgn, tmp, rgn);
    }
    if(extra)
        UnionRgn(rgn, extra, rgn);

    vdriver_set_rootless_region(rgn);
    DisposeRgn(tmp);
    DisposeRgn(rgn);
#endif
}

void Executor::ROMlib_rootless_openmenu(Rect r)
{
#ifdef LOG_ROOTLESS
    std::cout << "ROMlib_rootless_openmenu\n";
#endif
#ifdef VDRIVER_ROOTLESS
    r.left   = CW(CW(r.left  ) - 1);
    r.top    = CW(CW(r.top   ) - 1);
    r.right  = CW(CW(r.right ) + 2);
    r.bottom = CW(CW(r.bottom) + 2);
    rootlessMenus.push_back(r);
    ROMlib_rootless_update();
#endif
}
void Executor::ROMlib_rootless_closemenu()
{
#ifdef LOG_ROOTLESS
    std::cout << "ROMlib_rootless_closemenu\n";
#endif
#ifdef VDRIVER_ROOTLESS
    if(rootlessMenus.empty())
        return;
    rootlessMenus.pop_back();
    ROMlib_rootless_update();
#endif
}

bool Executor::ROMlib_rootless_drawdesk(RgnHandle desk)
{
#ifdef VDRIVER_ROOTLESS
    EraseRgn(desk);
    return true;
#else
    return false;
#endif
}