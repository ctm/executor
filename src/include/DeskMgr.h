#if !defined(__DESKMGR__)
#define __DESKMGR__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 */

#include <rsys/lowglobals.h>
#include "ExMacTypes.h"
#include "EventMgr.h"
#include "WindowMgr.h"

#define MODULE_NAME DeskMgr
#include <rsys/api-module.h>

namespace Executor
{
enum
{
    undoCmd = 0,
    cutCmd = 2,
    copyCmd = 3,
    pasteCmd = 4,
    clearCmd = 5,
};

enum
{
    accEvent = 64,
    accRun = 65,
    accMenu = 67,
    accUndo = 68,
};

const LowMemGlobal<Byte> SEvtEnb { 0x15C }; // DeskMgr IMI-443 (false);

/* DO NOT DELETE THIS LINE */
extern INTEGER C_OpenDeskAcc(
    Str255 acc);
PASCAL_TRAP(OpenDeskAcc, 0xA9B6);
extern void C_CloseDeskAcc(INTEGER rn);
PASCAL_TRAP(CloseDeskAcc, 0xA9B7);

extern void C_SystemClick(EventRecord *evp, WindowPtr wp);
PASCAL_TRAP(SystemClick, 0xA9B3);

extern BOOLEAN C_SystemEdit(INTEGER editcmd);
PASCAL_TRAP(SystemEdit, 0xA9C2);

extern void C_SystemTask(void);
PASCAL_TRAP(SystemTask, 0xA9B4);

extern BOOLEAN C_SystemEvent(EventRecord *evp);
PASCAL_TRAP(SystemEvent, 0xA9B2);

extern void C_SystemMenu(LONGINT menu);
PASCAL_TRAP(SystemMenu, 0xA9B5);
}
#endif /* __DESKMGR__ */
