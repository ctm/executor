#if !defined(__TOOLEVENT__)
#define __TOOLEVENT__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "EventMgr.h"
namespace Executor
{
#if 0
#if !defined(KeyThresh)
extern INTEGER 	KeyThresh;
extern INTEGER 	KeyRepThresh;
extern LONGINT 	DoubleTime;
extern LONGINT 	CaretTime;
extern Byte 	ScrDmpEnb;
#endif
#endif

extern void ROMlib_alarmoffmbar(void);
extern LONGINT C_KeyTrans(Ptr mapp, unsigned short code,
                                      LONGINT *state);
PASCAL_TRAP(KeyTrans, 0xA9C3);
extern BOOLEAN C_GetNextEvent(INTEGER em,
                                          EventRecord *evt);
PASCAL_TRAP(GetNextEvent, 0xA970);
extern BOOLEAN C_WaitNextEvent(INTEGER mask,
                                           EventRecord *evp, LONGINT sleep, RgnHandle mousergn);
PASCAL_TRAP(WaitNextEvent, 0xA860);
extern BOOLEAN C_EventAvail(INTEGER em, EventRecord *evt);
PASCAL_TRAP(EventAvail, 0xA971);

extern void C_GetMouse(GUEST<Point> *p);
PASCAL_TRAP(GetMouse, 0xA972);
extern BOOLEAN C_Button(void);
PASCAL_TRAP(Button, 0xA974);

extern BOOLEAN C_StillDown(void);
PASCAL_TRAP(StillDown, 0xA973);

extern BOOLEAN C_WaitMouseUp(void);
PASCAL_TRAP(WaitMouseUp, 0xA977);

extern void C_GetKeys(unsigned char *keys);
PASCAL_TRAP(GetKeys, 0xA976);

extern LONGINT C_TickCount(void);
PASCAL_TRAP(TickCount, 0xA975);

extern LONGINT GetDblTime(void);
extern LONGINT GetCaretTime(void);
}
#endif /* __TOOLEVENT__ */
