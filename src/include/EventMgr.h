#if !defined (__EVENT__)
#define __EVENT__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: EventMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "QuickDraw.h"

#define nullEvent	0
#define mouseDown	1
#define mouseUp		2
#define keyDown		3
#define keyUp		4
#define autoKey		5
#define updateEvt	6
#define diskEvt		7
#define activateEvt	8
#define networkEvt	10
#define driverEvt	11
#define app1Evt		12
#define app2Evt		13
#define app3Evt		14
#define app4Evt		15
#define kHighLevelEvent	23

#define charCodeMask	0xFFL
#define keyCodeMask	0xFF00L

#define mDownMask	2
#define mUpMask		4
#define keyDownMask	8
#define keyUpMask	16
#define autoKeyMask	32
#define updateMask	64
#define diskMask	128
#define activMask	256
/* #define networkMask	1024 */
#define highLevelEventMask	1024
#define driverMask	2048
#define app1Mask	4096
#define app2Mask	8192
#define app3Mask	16384
#define app4Mask	(-32768)
#define everyEvent	(-1)

#define activeFlag	1
#define changeFlag	2
#define btnState	128
#define cmdKey		256
#define shiftKey	512
#define alphaLock	1024
#define optionKey	2048
#define ControlKey	4096	/* IM V-196 */

enum
{
  rightShiftKey   = 0x2000,
  rightOptionKey  = 0x4000,
  rightControlKey = 0x8000
};

typedef struct {
    INTEGER what	PACKED;
    LONGINT message	PACKED;
    LONGINT when	PACKED;
    Point where	LPACKED;
    INTEGER modifiers	PACKED;
} EventRecord;

#if !defined (KeyMap)
extern unsigned char 	KeyMap[16];
#endif

#endif /* __EVENT__ */
