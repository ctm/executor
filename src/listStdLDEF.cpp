/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "ListMgr.h"
#include "MemoryMgr.h"

namespace Executor
{
PRIVATE void draw(BOOLEAN, Rect *, INTEGER, INTEGER, ListHandle);
void C_ldef0(INTEGER, BOOLEAN, Rect *, Cell, INTEGER, INTEGER, ListHandle);
}

A5(PRIVATE, void, draw, BOOLEAN, sel, Rect *, rect, INTEGER, doff,
   INTEGER, dl, ListHandle, list)
{
    GrafPtr savePort;

    TRAPBEGIN();
    savePort = thePort;
    SetPort(HxP(list, port));
    EraseRect(rect);
    MoveTo(CW(rect->left) + Hx(list, indent.h), CW(rect->top) + Hx(list, indent.v));
    HLock((Handle)HxP(list, cells));
    DrawText((Ptr)STARH(HxP(list, cells)) + doff, 0, dl);
    HUnlock((Handle)HxP(list, cells));
    if(sel)
        InvertRect(rect);
    SetPort(savePort);
    TRAPEND();
}

P7(PUBLIC pascal, void, ldef0, INTEGER, msg, BOOLEAN, sel, Rect *, rect,
   Cell, cell, INTEGER, doff, INTEGER, dl,
   ListHandle, list) /* IMIV-276 */
{
    GrafPtr savePort;
    FontInfo fi;

    switch(msg)
    {
        case lInitMsg:
            savePort = thePort;
            SetPort(HxP(list, port));
            GetFontInfo(&fi);
            HxX(list, indent.h) = CWC(5);
            HxX(list, indent.v) = fi.ascent;
            SetPort(savePort);
            break;
        case lDrawMsg:
            draw(sel, rect, doff, dl, list);
            break;
        case lHiliteMsg:
            savePort = thePort;
            SetPort(HxP(list, port));
            InvertRect(rect);
            SetPort(savePort);
            break;
        case lCloseMsg: /* nothing special to do */
            break;
        default: /* weirdness */
            break;
    }
}
