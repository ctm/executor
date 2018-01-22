#if !defined(_PROCESSMGR_H_)
#define _PROCESSMGR_H_

#include "FileMgr.h"
#include "PPC.h"

/*
 * Copyright 1995, 1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
typedef INTEGER LaunchFlags;

struct ProcessSerialNumber
{
    GUEST_STRUCT;
    GUEST<uint32_t> highLongOfPSN;
    GUEST<uint32_t> lowLongOfPSN;
};

/* for now, anyway */

/*
 * ARDI implementation of AppParameters.  This has not been determined by
 * looking at the data Apple returns, so it will not play well with anything
 * that knows the format of this data structure.  It's added so we can
 * support the launching of non-Mac applications from Executor.  TODO: figure
 * out exactly what the mac uses
 */

typedef struct
{
    GUEST_STRUCT;
    GUEST<uint32_t> magic;
    GUEST<INTEGER> n_fsspec;
    FSSpec fsspec[0];
} ROMlib_AppParameters_t;

enum
{
    APP_PARAMS_MAGIC = 0xd6434a1b,
}; /* chosen from /dev/random */

typedef ROMlib_AppParameters_t *AppParametersPtr;

struct LaunchParamBlockRec
{
    GUEST_STRUCT;
    GUEST<LONGINT> reserved1;
    GUEST<INTEGER> reserved2;
    GUEST<INTEGER> launchBlockID;
    GUEST<LONGINT> launchEPBLength;
    GUEST<INTEGER> launchFileFlags;
    GUEST<LaunchFlags> launchControlFlags;
    GUEST<FSSpecPtr> launchAppSpec;
    GUEST<ProcessSerialNumber> launchProcessSN;
    GUEST<LONGINT> launchPreferredSize;
    GUEST<LONGINT> launchMinimumSize;
    GUEST<LONGINT> launchAvailableSize;
    GUEST<AppParametersPtr> launchAppParameters;
};

enum
{
    extendedBlock = 0x4C43
};
enum
{
    extendedBlockLen = sizeof(LaunchParamBlockRec) - 12
};
enum
{
    launchContinue = 0x4000
};

#define PSN_EQ_P(psn0, psn1)                      \
    ((psn0).highLongOfPSN == (psn1).highLongOfPSN \
     && (psn0).lowLongOfPSN == (psn1).lowLongOfPSN)

struct ProcessInfoRec
{
    GUEST_STRUCT;
    GUEST<uint32_t> processInfoLength;
    GUEST<StringPtr> processName;
    GUEST<ProcessSerialNumber> processNumber;
    GUEST<uint32_t> processType;
    GUEST<OSType> processSignature;
    GUEST<uint32_t> processMode;
    GUEST<Ptr> processLocation;
    GUEST<uint32_t> processSize;
    GUEST<uint32_t> processFreeMem;
    GUEST<ProcessSerialNumber> processLauncher;
    GUEST<uint32_t> processLaunchDate;
    GUEST<uint32_t> processActiveTime;
    GUEST<FSSpecPtr> processAppSpec;
};
typedef ProcessInfoRec *ProcessInfoPtr;

#define PROCESS_INFO_SERIAL_NUMBER(info) ((info)->processNumber)
#define PROCESS_INFO_LAUNCHER(info) ((info)->processLauncher)

#define PROCESS_INFO_LENGTH_X(info) ((info)->processInfoLength)
#define PROCESS_INFO_NAME_X(info) ((info)->processName)
#define PROCESS_INFO_TYPE_X(info) ((info)->processType)
#define PROCESS_INFO_SIGNATURE_X(info) ((info)->processSignature)
#define PROCESS_INFO_MODE_X(info) ((info)->processMode)
#define PROCESS_INFO_LOCATION_X(info) ((info)->processLocation)
#define PROCESS_INFO_SIZE_X(info) ((info)->processSize)
#define PROCESS_INFO_FREE_MEM_X(info) ((info)->processFreeMem)
#define PROCESS_INFO_LAUNCH_DATE_X(info) ((info)->processLaunchDate)
#define PROCESS_INFO_ACTIVE_TIME_X(info) ((info)->processActiveTime)

#define PROCESS_INFO_LENGTH(info) (CL(PROCESS_INFO_LENGTH_X(info)))
#define PROCESS_INFO_NAME(info) (CL(PROCESS_INFO_NAME_X(info)))
#define PROCESS_INFO_TYPE(info) (CL(PROCESS_INFO_TYPE_X(info)))
#define PROCESS_INFO_SIGNATURE(info) (CL(PROCESS_INFO_SIGNATURE_X(info)))
#define PROCESS_INFO_MODE(info) (CL(PROCESS_INFO_MODE_X(info)))
#define PROCESS_INFO_LOCATION(info) (CL(PROCESS_INFO_LOCATION_X(info)))
#define PROCESS_INFO_SIZE(info) (CL(PROCESS_INFO_SIZE_X(info)))
#define PROCESS_INFO_FREE_MEM(info) (CL(PROCESS_INFO_FREE_MEM_X(info)))
#define PROCESS_INFO_LAUNCH_DATE(info) (CL(PROCESS_INFO_LAUNCH_DATE_X(info)))
#define PROCESS_INFO_ACTIVE_TIME(info) (CL(PROCESS_INFO_ACTIVE_TIME_X(info)))

/* flags for the `processMode' field of the `ProcessInformationRec'
   record */

enum
{
    modeDeskAccessory = 0x00020000,
    modeMultiLaunch = 0x00010000,
    modeNeedSuspendResume = 0x00004000,
    modeCanBackground = 0x00001000,
    modeDoesActivateOnFGSwitch = 0x00000800,
    modeOnlyBackground = 0x00000400,
    modeGetFrontClicks = 0x00000200,
    modeGetAppDiedMsg = 0x00000100,
    mode32BitCompatible = 0x00000080,
    modeHighLevelEventAware = 0x00000040,
    modeLocalAndRemoteHLEvents = 0x00000020,
    modeStationeryAware = 0x00000010,
    modeUseTextEditServices = 0x00000008,
};

enum
{
    kNoProcess = (0),
    kSystemProcess = (1),
    kCurrentProcess = (2),
};

enum
{
    procNotFound = (-600),
};

extern void process_create(bool desk_accessory_p,
                           uint32_t type, uint32_t signature);

extern OSErr C_GetCurrentProcess(ProcessSerialNumber *serial_number);
PASCAL_SUBTRAP(GetCurrentProcess, 0xA88F, 0x0037, OSDispatch);

extern OSErr C_GetNextProcess(ProcessSerialNumber *serial_number);
PASCAL_SUBTRAP(GetNextProcess, 0xA88F, 0x0038, OSDispatch);

extern OSErr C_GetProcessInformation(ProcessSerialNumber *serial_number,
                                                 ProcessInfoPtr info);
PASCAL_SUBTRAP(GetProcessInformation, 0xA88F, 0x003A, OSDispatch);

extern OSErr C_SameProcess(ProcessSerialNumber *serial_number0,
                                       ProcessSerialNumber *serial_number1,
                                       Boolean *same_out);
PASCAL_SUBTRAP(SameProcess, 0xA88F, 0x003D, OSDispatch);
extern OSErr C_GetFrontProcess(int32_t dummy, ProcessSerialNumber *serial_number);
PASCAL_SUBTRAP(GetFrontProcess, 0xA88F, 0x0039, OSDispatch);

extern OSErr C_SetFrontProcess(ProcessSerialNumber *serial_number);
PASCAL_SUBTRAP(SetFrontProcess, 0xA88F, 0x003B, OSDispatch);

extern OSErr C_WakeUpProcess(ProcessSerialNumber *serial_number);
PASCAL_SUBTRAP(WakeUpProcess, 0xA88F, 0x003C, OSDispatch);

extern OSErr C_GetProcessSerialNumberFromPortName(PPCPortPtr port_name,
                                                              ProcessSerialNumber *serial_number);
PASCAL_SUBTRAP(GetProcessSerialNumberFromPortName, 0xA88F, 0x0035, OSDispatch);

extern OSErr C_GetPortNameFromProcessSerialNumber(PPCPortPtr port_name,
                                                              ProcessSerialNumber *serial_number);
PASCAL_SUBTRAP(GetPortNameFromProcessSerialNumber, 0xA88F, 0x0046, OSDispatch);

extern OSErr NewLaunch(StringPtr appl, INTEGER vrefnum,
                       LaunchParamBlockRec *lpbp);
}
#endif
