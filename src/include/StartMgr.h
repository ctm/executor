#if !defined(__STARTMGR__)
#define __STARTMGR__

/*
 * Copyright 1994 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
typedef union {
    struct
    {
        SignedByte sdExtDevID;
        SignedByte sdPartition;
        SignedByte sdSlotNum;
        SignedByte sdSRsrcID;
    } slotDev;
    struct
    {
        SignedByte sdReserved1;
        SignedByte sdReserved2;
        INTEGER sdRefNum;
    } scsiDev;
} DefStartRec, *DefStartPtr;

typedef struct
{
    SignedByte sdSlot;
    SignedByte sdSResource;
} DefVideoRec, *DefVideoPtr;

typedef struct
{
    SignedByte sdReserved;
    SignedByte sdOSType;
} DefOSRec, *DefOSPtr;

const LowMemGlobal<Byte> CPUFlag { 0x12F }; // StartMgr IMV-348 (true-b);
const LowMemGlobal<INTEGER> TimeDBRA { 0xD00 }; // StartMgr IMV (false);
const LowMemGlobal<INTEGER> TimeSCCDB { 0xD02 }; // StartMgr IMV (false);
const LowMemGlobal<INTEGER> TimeSCSIDB { 0xDA6 }; // StartMgr IMV (false);
}

#endif
