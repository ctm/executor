#if !defined(__DEVICEMGR__)
#define __DEVICEMGR__

/*
 * Copyright 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "WindowMgr.h"
#include "FileMgr.h"

/*
 * Note the structure below is similar to that presented on IM-188,
 * but I don't use offsets to the routines, but pointers to the routines
 * directly.  This means the size of the structure is larger than one
 * might expect.
 */

namespace Executor
{
typedef struct
{
    GUEST<ProcPtr> udrvrOpen;
    GUEST<ProcPtr> udrvrPrime; /* read and write */
    GUEST<ProcPtr> udrvrCtl; /* control and killio */
    GUEST<ProcPtr> udrvrStatus;
    GUEST<ProcPtr> udrvrClose;
    Str255 udrvrName;
} umacdriver, *umacdriverptr;

struct ramdriver
{
    GUEST_STRUCT;
    GUEST<INTEGER> drvrFlags;
    GUEST<INTEGER> drvrDelay;
    GUEST<INTEGER> drvrEMask;
    GUEST<INTEGER> drvrMenu;
    GUEST<INTEGER> drvrOpen;
    GUEST<INTEGER> drvrPrime;
    GUEST<INTEGER> drvrCtl;
    GUEST<INTEGER> drvrStatus;
    GUEST<INTEGER> drvrClose;
    GUEST<char> drvrName;
};

typedef ramdriver *ramdriverptr;

typedef GUEST<ramdriverptr> *ramdriverhand;

typedef enum { Open,
               Prime,
               Ctl,
               Stat,
               Close } DriverRoutineType;

typedef struct DCtlEntry
{
    GUEST_STRUCT;
    GUEST<umacdriverptr> dCtlDriver; /* not just Ptr */
    GUEST<INTEGER> dCtlFlags;
    GUEST<QHdr> dCtlQHdr;
    GUEST<LONGINT> dCtlPosition;
    GUEST<Handle> dCtlStorage;
    GUEST<INTEGER> dCtlRefNum;
    GUEST<LONGINT> dCtlCurTicks;
    GUEST<WindowPtr> dCtlWindow;
    GUEST<INTEGER> dCtlDelay;
    GUEST<INTEGER> dCtlEMask;
    GUEST<INTEGER> dCtlMenu;
} * DCtlPtr;

typedef GUEST<DCtlPtr> *DCtlHandle;

typedef GUEST<DCtlHandle> *DCtlHandlePtr;

enum
{
    asyncTrpBit = (1 << 10),
    noQueueBit = (1 << 9),
};

enum
{
    NEEDTIMEBIT = (1 << 13),
};

enum
{
    aRdCmd = 2,
    aWrCmd = 3,
};

enum
{
    killCode = 1,
};

enum
{
    NDEVICES = 48,
};

enum
{
    abortErr = (-27),
    badUnitErr = (-21),
    controlErr = (-17),
    dInstErr = (-26),
    dRemovErr = (-25),
    notOpenErr = (-28),
    openErr = (-23),
    readErr = (-19),
    statusErr = (-18),
    unitEmptyErr = (-22),
    writErr = (-20),
};

typedef struct
{
    OSErr (*open)();
    OSErr (*prime)();
    OSErr (*ctl)();
    OSErr (*status)();
    OSErr (*close)();
    StringPtr name;
    INTEGER refnum;
} driverinfo;

const LowMemGlobal<DCtlHandlePtr> UTableBase { 0x11C }; // DeviceMgr IMII-192 (false);
const LowMemGlobal<ProcPtr[8]> Lvl1DT { 0x192 }; // DeviceMgr IMII-197 (false);
const LowMemGlobal<ProcPtr[8]> Lvl2DT { 0x1B2 }; // DeviceMgr IMII-198 (false);
const LowMemGlobal<INTEGER> UnitNtryCnt { 0x1D2 }; // DeviceMgr ThinkC (true-b);
const LowMemGlobal<Ptr> VIA { 0x1D4 }; // DeviceMgr IMIII-39 (true-b);
const LowMemGlobal<Ptr> SCCRd { 0x1D8 }; // DeviceMgr IMII-199 (false);
const LowMemGlobal<Ptr> SCCWr { 0x1DC }; // DeviceMgr IMII-199 (false);
const LowMemGlobal<Ptr> IWM { 0x1E0 }; // DeviceMgr ThinkC (false);
const LowMemGlobal<ProcPtr[4]> ExtStsDT { 0x2BE }; // DeviceMgr IMII-199 (false);
const LowMemGlobal<Ptr> JFetch { 0x8F4 }; // DeviceMgr IMII-194 (false);
const LowMemGlobal<Ptr> JStash { 0x8F8 }; // DeviceMgr IMII-195 (false);
const LowMemGlobal<Ptr> JIODone { 0x8FC }; // DeviceMgr IMII-195 (false);

/*
 * __ROMlib_otherdrivers is initialized to null, but can be used to allow
 * extra drivers to be called.  Have __ROMlib_otherdrivers point to an array
 * driverinfo (terminated with a null open field) before you call PBOpen
 * if you have additional drivers to use.
 */

extern driverinfo *__ROMlib_otherdrivers;

extern OSErr PBControl(ParmBlkPtr pbp, BOOLEAN a);
extern OSErr PBStatus(ParmBlkPtr pbp, BOOLEAN a);
extern OSErr PBKillIO(ParmBlkPtr pbp, BOOLEAN a);

FILE_TRAP(PBControl, 0xA004);
FILE_TRAP(PBStatus, 0xA005);
FILE_TRAP(PBKillIO, 0xA006);

extern OSErr OpenDriver(StringPtr name, GUEST<INTEGER> *rnp);
extern OSErr CloseDriver(INTEGER rn);
extern OSErr Control(INTEGER rn, INTEGER code,
                     Ptr param);
extern OSErr Status(INTEGER rn, INTEGER code, Ptr param);
extern OSErr KillIO(INTEGER rn);
extern DCtlHandle GetDCtlEntry(INTEGER rn);
}
#endif /* __DEVICEMGR__ */
