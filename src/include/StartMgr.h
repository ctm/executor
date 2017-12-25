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

#if !defined(CPUFlag)
extern Byte CPUFlag;
#endif
}

#endif
