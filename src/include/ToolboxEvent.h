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
extern pascal trap LONGINT C_KeyTrans(Ptr mapp, unsigned short code,
                                      LONGINT *state);
extern pascal trap BOOLEAN C_GetNextEvent(INTEGER em,
                                          EventRecord *evt);
extern pascal trap BOOLEAN C_WaitNextEvent(INTEGER mask,
                                           EventRecord *evp, LONGINT sleep, RgnHandle mousergn);
extern pascal trap BOOLEAN C_EventAvail(INTEGER em, EventRecord *evt);

extern pascal trap void C_GetMouse(GUEST<Point> *p);
extern pascal trap BOOLEAN C_Button(void);

extern pascal trap BOOLEAN C_StillDown(void);

extern pascal trap BOOLEAN C_WaitMouseUp(void);

extern pascal trap void C_GetKeys(unsigned char *keys);

extern pascal trap LONGINT C_TickCount(void);

extern LONGINT GetDblTime(void);
extern LONGINT GetCaretTime(void);
}
#endif /* __TOOLEVENT__ */
