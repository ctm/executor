#if !defined(__DESKMGR__)
#define __DESKMGR__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
#define undoCmd 0
#define cutCmd 2
#define copyCmd 3
#define pasteCmd 4
#define clearCmd 5

#define accEvent 64
#define accRun 65
#define accMenu 67
#define accUndo 68

#if !defined(SEvtEnb)
extern Byte SEvtEnb;
#endif

/* DO NOT DELETE THIS LINE */
extern pascal trap INTEGER C_OpenDeskAcc(
    Str255 acc);
extern pascal trap void C_CloseDeskAcc(INTEGER rn);

extern pascal trap void C_SystemClick(EventRecord *evp, WindowPtr wp);

extern pascal trap BOOLEAN C_SystemEdit(INTEGER editcmd);

extern pascal trap void C_SystemTask(void);

extern pascal trap BOOLEAN C_SystemEvent(EventRecord *evp);

extern pascal trap void C_SystemMenu(LONGINT menu);

}
#endif /* __DESKMGR__ */
