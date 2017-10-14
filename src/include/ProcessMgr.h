#if !defined(_PROCESSMGR_H_)
#define _PROCESSMGR_H_

#include "FileMgr.h"
#include "PPC.h"

/*
 * Copyright 1995, 1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: ProcessMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
typedef INTEGER LaunchFlags;

struct ProcessSerialNumber : GuestStruct {
    GUEST< uint32> highLongOfPSN;
    GUEST< uint32> lowLongOfPSN;
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
  uint32 magic;
  INTEGER n_fsspec;
  FSSpec fsspec[0];
}
ROMlib_AppParameters_t;

enum { APP_PARAMS_MAGIC = 0xd6434a1b, }; /* chosen from /dev/random */

typedef ROMlib_AppParameters_t *AppParametersPtr;

struct LaunchParamBlockRec : GuestStruct {
    GUEST< LONGINT> reserved1;
    GUEST< INTEGER> reserved2;
    GUEST< INTEGER> launchBlockID;
    GUEST< LONGINT> launchEPBLength;
    GUEST< INTEGER> launchFileFlags;
    GUEST< LaunchFlags> launchControlFlags;
    GUEST< FSSpecPtr> launchAppSpec;
    GUEST< ProcessSerialNumber> launchProcessSN;
    GUEST< LONGINT> launchPreferredSize;
    GUEST< LONGINT> launchMinimumSize;
    GUEST< LONGINT> launchAvailableSize;
    GUEST< AppParametersPtr> launchAppParameters;
};

enum { extendedBlock = 0x4C43 };
enum { extendedBlockLen = sizeof (LaunchParamBlockRec) - 12 };
enum { launchContinue = 0x4000 };


#define PSN_EQ_P(psn0, psn1)				\
  ((psn0).highLongOfPSN == (psn1).highLongOfPSN		\
   && (psn0).lowLongOfPSN == (psn1).lowLongOfPSN)

struct ProcessInfoRec : GuestStruct {
    GUEST< uint32> processInfoLength;
    GUEST< StringPtr> processName;
    GUEST< ProcessSerialNumber> processNumber;
    GUEST< uint32> processType;
    GUEST< OSType> processSignature;
    GUEST< uint32> processMode;
    GUEST< Ptr> processLocation;
    GUEST< uint32> processSize;
    GUEST< uint32> processFreeMem;
    GUEST< ProcessSerialNumber> processLauncher;
    GUEST< uint32> processLaunchDate;
    GUEST< uint32> processActiveTime;
    GUEST< FSSpecPtr> processAppSpec;
};
typedef ProcessInfoRec *ProcessInfoPtr;

#define PROCESS_INFO_SERIAL_NUMBER(info)	((info)->processNumber)
#define PROCESS_INFO_LAUNCHER(info)		((info)->processLauncher)

#define PROCESS_INFO_LENGTH_X(info)		((info)->processInfoLength)
#define PROCESS_INFO_NAME_X(info)		((info)->processName)
#define PROCESS_INFO_TYPE_X(info)		((info)->processType)
#define PROCESS_INFO_SIGNATURE_X(info)		((info)->processSignature)
#define PROCESS_INFO_MODE_X(info)		((info)->processMode)
#define PROCESS_INFO_LOCATION_X(info)		((info)->processLocation)
#define PROCESS_INFO_SIZE_X(info)		((info)->processSize)
#define PROCESS_INFO_FREE_MEM_X(info)		((info)->processFreeMem)
#define PROCESS_INFO_LAUNCH_DATE_X(info)	((info)->processLaunchDate)
#define PROCESS_INFO_ACTIVE_TIME_X(info)	((info)->processActiveTime)

#define PROCESS_INFO_LENGTH(info)	(CL (PROCESS_INFO_LENGTH_X (info)))
#define PROCESS_INFO_NAME(info)		(CL (PROCESS_INFO_NAME_X (info)))
#define PROCESS_INFO_TYPE(info)		(CL (PROCESS_INFO_TYPE_X (info)))
#define PROCESS_INFO_SIGNATURE(info)	(CL (PROCESS_INFO_SIGNATURE_X (info)))
#define PROCESS_INFO_MODE(info)		(CL (PROCESS_INFO_MODE_X (info)))
#define PROCESS_INFO_LOCATION(info)	(CL (PROCESS_INFO_LOCATION_X (info)))
#define PROCESS_INFO_SIZE(info)		(CL (PROCESS_INFO_SIZE_X (info)))
#define PROCESS_INFO_FREE_MEM(info)	(CL (PROCESS_INFO_FREE_MEM_X (info)))
#define PROCESS_INFO_LAUNCH_DATE(info)	(CL (PROCESS_INFO_LAUNCH_DATE_X (info)))
#define PROCESS_INFO_ACTIVE_TIME(info)	(CL (PROCESS_INFO_ACTIVE_TIME_X (info)))

/* flags for the `processMode' field of the `ProcessInformationRec'
   record */

#define modeDeskAccessory 		0x00020000
#define modeMultiLaunch			0x00010000
#define modeNeedSuspendResume		0x00004000
#define modeCanBackground		0x00001000
#define modeDoesActivateOnFGSwitch	0x00000800
#define modeOnlyBackground		0x00000400
#define modeGetFrontClicks		0x00000200
#define modeGetAppDiedMsg		0x00000100
#define mode32BitCompatible		0x00000080
#define modeHighLevelEventAware		0x00000040
#define modeLocalAndRemoteHLEvents	0x00000020
#define modeStationeryAware		0x00000010
#define modeUseTextEditServices		0x00000008

#define kNoProcess	(0)
#define kSystemProcess	(1)
#define kCurrentProcess	(2)

#define paramErr	(-50)
#define procNotFound	(-600)

extern void process_create (boolean_t desk_accessory_p,
			    uint32 type, uint32 signature);

extern pascal trap OSErr C_GetCurrentProcess
  (ProcessSerialNumber *serial_number);

extern pascal trap OSErr C_GetNextProcess (ProcessSerialNumber *serial_number);

extern pascal trap OSErr C_GetProcessInformation
  (ProcessSerialNumber *serial_number,
   ProcessInfoPtr info);

extern pascal trap OSErr C_SameProcess (ProcessSerialNumber *serial_number0,
					ProcessSerialNumber *serial_number1,
					Boolean *same_out);
extern pascal trap OSErr C_GetFrontProcess
  (ProcessSerialNumber *serial_number, void *dummy);

extern pascal trap OSErr C_SetFrontProcess
  (ProcessSerialNumber *serial_number);

extern pascal trap OSErr C_WakeUpProcess (ProcessSerialNumber *serial_number);

extern pascal trap OSErr C_GetProcessSerialNumberFromPortName
  (PPCPortPtr port_name,
   ProcessSerialNumber *serial_number);

extern pascal trap OSErr C_GetPortNameFromProcessSerialNumber
  (PPCPortPtr port_name,
   ProcessSerialNumber *serial_number);

extern OSErr NewLaunch( StringPtr appl, INTEGER vrefnum,
		      LaunchParamBlockRec *lpbp );
}
#endif
