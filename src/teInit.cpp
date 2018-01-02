/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in TextEdit.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "WindowMgr.h"
#include "ControlMgr.h"
#include "ToolboxUtil.h"
#include "FontMgr.h"
#include "TextEdit.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/mman.h"
#include "rsys/tesave.h"

using namespace Executor;

PUBLIC pascal trap void Executor::C_TEInit()
{
    TEScrpHandle = RM(NewHandle(0));
    TEScrpLength = CWC(0);
    TEDoText = RM((ProcPtr)P_ROMlib_dotext);
}

/* This code just does "moveql #1,d0 ; rts".  We use it because
 * DNA strider chains to the old clikLoop handler.
 */
static GUEST<uint16_t> default_clik_loop[2] = { CWC(0x7001), CWC(0x4E75) };

PUBLIC pascal trap TEHandle Executor::C_TENew(Rect *dst, Rect *view)
{
    TEHandle teh;
    FontInfo finfo;
    GUEST<Handle> hText;
    GUEST<tehiddenh> temptehiddenh;
    GUEST<int16_t> *tehlinestarts;
    int te_size;

    te_size = ((sizeof(TERec)
                - sizeof TE_LINE_STARTS(teh))
               + 4 * sizeof *TE_LINE_STARTS(teh));
    teh = (TEHandle)NewHandle(te_size);

    hText = RM(NewHandle(0));
    GetFontInfo(&finfo);
    /* zero the te record */
    memset(STARH(teh), 0, te_size);
    /* ### find out what to assign to the `selRect', `selPoint'
     and `clikStuff' fields */
    HASSIGN_15(teh,
               destRect, *dst,
               viewRect, *view,
               lineHeight, CW(CW(finfo.ascent)
                              + CW(finfo.descent)
                              + CW(finfo.leading)),
               fontAscent, finfo.ascent,
               active, CWC(false),
               caretState, CWC(caret_invis),
               just, CWC(teFlushDefault),
               crOnly, CWC(1),
               clikLoop, RM((ProcPtr)&default_clik_loop[0]),
               inPort, thePortX,
               txFont, PORT_TX_FONT_X(thePort),
               txFace, PORT_TX_FACE(thePort),
               txMode, PORT_TX_MODE_X(thePort),
               txSize, PORT_TX_SIZE_X(thePort),
               hText, hText);

    tehlinestarts = HxX(teh, lineStarts);
    tehlinestarts[0] = 0;
    tehlinestarts[1] = 0; /* this one is only for mix & match w/mac */

    temptehiddenh = RM((tehiddenh)NewHandle(sizeof(tehidden)));
    /* don't merge with line above */
    TEHIDDENHX(teh) = temptehiddenh;
    memset(STARH(TEHIDDENH(teh)), 0, sizeof(tehidden));

    TE_SLAM(teh);

    return teh;
}

PUBLIC pascal trap void Executor::C_TEDispose(TEHandle teh)
{
    DisposHandle((Handle)TEHIDDENH(teh));
    DisposHandle(TE_HTEXT(teh));
    DisposHandle((Handle)teh);
}
