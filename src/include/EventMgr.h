#if !defined(__EVENT__)
#define __EVENT__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "QuickDraw.h"

namespace Executor
{
enum
{
    nullEvent = 0,
    mouseDown = 1,
    mouseUp = 2,
    keyDown = 3,
    keyUp = 4,
    autoKey = 5,
    updateEvt = 6,
    diskEvt = 7,
    activateEvt = 8,
    networkEvt = 10,
    driverEvt = 11,
    app1Evt = 12,
    app2Evt = 13,
    app3Evt = 14,
    app4Evt = 15,
    kHighLevelEvent = 23,
};

enum
{
    charCodeMask = 0xFFL,
    keyCodeMask = 0xFF00L,
};

enum
{
    mDownMask = 2,
    mUpMask = 4,
    keyDownMask = 8,
    keyUpMask = 16,
    autoKeyMask = 32,
    updateMask = 64,
    diskMask = 128,
    activMask = 256,
};
/* #define networkMask	1024 */
enum
{
    highLevelEventMask = 1024,
    driverMask = 2048,
    app1Mask = 4096,
    app2Mask = 8192,
    app3Mask = 16384,
    app4Mask = (-32768),
    everyEvent = (-1),
};

enum
{
    activeFlag = 1,
    changeFlag = 2,
    btnState = 128,
    cmdKey = 256,
    shiftKey = 512,
    alphaLock = 1024,
    optionKey = 2048,
};
#define ControlKey 4096 /* IM V-196 */

enum
{
    rightShiftKey = 0x2000,
    rightOptionKey = 0x4000,
    rightControlKey = 0x8000
};

struct EventRecord
{
    GUEST_STRUCT;
    GUEST<INTEGER> what;
    GUEST<LONGINT> message;
    GUEST<LONGINT> when;
    GUEST<Point> where;
    GUEST<INTEGER> modifiers;
};

const LowMemGlobal<unsigned char[16]> KeyMap { 0x174 }; // EventMgr SysEqu.a (true-b);
/* was LONGINT KeypadMap[2]; */

}

#endif /* __EVENT__ */
