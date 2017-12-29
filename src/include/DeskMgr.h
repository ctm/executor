#if !defined(__DESKMGR__)
#define __DESKMGR__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

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

#if !defined(SEvtEnb)
extern Byte SEvtEnb;
#endif

/* DO NOT DELETE THIS LINE */
extern pascal trap INTEGER C_OpenDeskAcc(
    Str255 acc);
PASCAL_TRAP(OpenDeskAcc, 0xA9B6);
extern pascal trap void C_CloseDeskAcc(INTEGER rn);
PASCAL_TRAP(CloseDeskAcc, 0xA9B7);

extern pascal trap void C_SystemClick(EventRecord *evp, WindowPtr wp);
PASCAL_TRAP(SystemClick, 0xA9B3);

extern pascal trap BOOLEAN C_SystemEdit(INTEGER editcmd);
PASCAL_TRAP(SystemEdit, 0xA9C2);

extern pascal trap void C_SystemTask(void);
PASCAL_TRAP(SystemTask, 0xA9B4);

extern pascal trap BOOLEAN C_SystemEvent(EventRecord *evp);
PASCAL_TRAP(SystemEvent, 0xA9B2);

extern pascal trap void C_SystemMenu(LONGINT menu);
PASCAL_TRAP(SystemMenu, 0xA9B5);

}
#endif /* __DESKMGR__ */
