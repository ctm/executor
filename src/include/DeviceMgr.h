#if !defined (__DEVICEMGR__)
#define __DEVICEMGR__

/*
 * Copyright 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: DeviceMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "WindowMgr.h"
#include "FileMgr.h"

/*
 * Note the structure below is similar to that presented on IM-188,
 * but I don't use offsets to the routines, but pointers to the routines
 * directly.  This means the size of the structure is larger than one
 * might expect.
 */

namespace Executor {
typedef struct {
    ProcPtr udrvrOpen;
    ProcPtr udrvrPrime;	/* read and write */
    ProcPtr udrvrCtl;	/* control and killio */
    ProcPtr udrvrStatus;
    ProcPtr udrvrClose;
    Str255  udrvrName;
} umacdriver, *umacdriverptr;

struct ramdriver : GuestStruct {
    GUEST< INTEGER> drvrFlags;
    GUEST< INTEGER> drvrDelay;
    GUEST< INTEGER> drvrEMask;
    GUEST< INTEGER> drvrMenu;
    GUEST< INTEGER> drvrOpen;
    GUEST< INTEGER> drvrPrime;
    GUEST< INTEGER> drvrCtl;
    GUEST< INTEGER> drvrStatus;
    GUEST< INTEGER> drvrClose;
    GUEST< char> drvrName;
};

typedef ramdriver *ramdriverptr;
MAKE_HIDDEN(ramdriverptr);
typedef HIDDEN_ramdriverptr *ramdriverhand;

typedef enum { Open, Prime, Ctl, Stat, Close } DriverRoutineType;

typedef struct DCtlEntry : GuestStruct {
    GUEST< umacdriverptr> dCtlDriver;    /* not just Ptr */
    GUEST< INTEGER> dCtlFlags;
    GUEST< QHdr> dCtlQHdr;
    GUEST< LONGINT> dCtlPosition;
    GUEST< Handle> dCtlStorage;
    GUEST< INTEGER> dCtlRefNum;
    GUEST< LONGINT> dCtlCurTicks;
    GUEST< WindowPtr> dCtlWindow;
    GUEST< INTEGER> dCtlDelay;
    GUEST< INTEGER> dCtlEMask;
    GUEST< INTEGER> dCtlMenu;
} *DCtlPtr;

MAKE_HIDDEN(DCtlPtr);
typedef HIDDEN_DCtlPtr *DCtlHandle;
MAKE_HIDDEN(DCtlHandle);
typedef HIDDEN_DCtlHandle *DCtlHandlePtr;
MAKE_HIDDEN(DCtlHandlePtr);

#define asyncTrpBit	(1 << 10)
#define noQueueBit	(1 <<  9)

#define NEEDTIMEBIT	(1 << 13)

#define aRdCmd		2
#define aWrCmd		3

#define killCode	1

#define NDEVICES	48

#define abortErr	(-27)
#define badUnitErr	(-21)
#define controlErr	(-17)
#define dInstErr	(-26)
#define dRemovErr	(-25)
#define notOpenErr	(-28)
#define openErr		(-23)
#define readErr		(-19)
#define statusErr	(-18)
#define unitEmptyErr	(-22)
#define writErr		(-20)

typedef struct {
    OSErr     (*open)();
    OSErr     (*prime)();
    OSErr     (*ctl)();
    OSErr     (*status)();
    OSErr     (*close)();
    StringPtr name;
    INTEGER   refnum;
} driverinfo;

/*
 * __ROMlib_otherdrivers is initialized to null, but can be used to allow
 * extra drivers to be called.  Have __ROMlib_otherdrivers point to an array
 * driverinfo (terminated with a null open field) before you call PBOpen
 * if you have additional drivers to use.
 */

extern driverinfo *__ROMlib_otherdrivers;

#if 0
#if !defined (UTableBase_H)
extern HIDDEN_DCtlHandlePtr UTableBase_H;
extern HIDDEN_Ptr 	VIA_H;
extern INTEGER UnitNtryCnt;
extern INTEGER 	UnitNtryCnt;
#endif

#define UTableBase	(UTableBase_H.p)
#define VIA		(VIA_H.p)
#endif

#if !defined (__STDC__)
extern OSErr PBControl(); 
extern OSErr PBStatus(); 
extern OSErr PBKillIO(); 
extern OSErr OpenDriver(); 
extern OSErr CloseDriver(); 
extern OSErr Control(); 
extern OSErr Status(); 
extern OSErr KillIO(); 
extern DCtlHandle GetDCtlEntry(); 
#else /* __STDC__ */
extern OSErr PBControl( ParmBlkPtr pbp, BOOLEAN a ); 
extern OSErr PBStatus( ParmBlkPtr pbp, BOOLEAN a ); 
extern OSErr PBKillIO( ParmBlkPtr pbp, BOOLEAN a ); 
extern OSErr OpenDriver( StringPtr name, INTEGER *rnp ); 
extern OSErr CloseDriver( INTEGER rn ); 
extern OSErr Control( INTEGER rn, INTEGER code, 
 Ptr param ); 
extern OSErr Status( INTEGER rn, INTEGER code, Ptr param ); 
extern OSErr KillIO( INTEGER rn ); 
extern DCtlHandle GetDCtlEntry( INTEGER rn ); 
#endif /* __STDC__ */
}
#endif /* __DEVICEMGR__ */
