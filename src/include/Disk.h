#if !defined(__DISK__)
#define __DISK__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
enum
{
    firstDskErr = (-84),
    sectNFErr = (-81),
    seekErr = (-80),
    spdAdjErr = (-79),
    twoSideErr = (-78),
    initIWMErr = (-77),
    tk0BadErr = (-76),
    cantStepErr = (-75),
    wrUnderrun = (-74),
    badDBtSlp = (-73),
    badDCksum = (-72),
    noDtaMkErr = (-71),
    badBtSlpErr = (-70),
    badCksmErr = (-69),
    dataVerErr = (-68),
    noAdrMkErr = (-67),
    noNybErr = (-66),
    offLinErr = (-65),
    noDriveErr = (-64),
    lastDskErr = (-64),
};

struct DrvSts
{
    GUEST_STRUCT;
    GUEST<INTEGER> track;
    GUEST<SignedByte> writeProt;
    GUEST<SignedByte> diskInPlace;
    GUEST<SignedByte> installed;
    GUEST<SignedByte> sides;
    GUEST<QElemPtr> qLink;
    GUEST<INTEGER> qType;
    GUEST<INTEGER> dQDrive;
    GUEST<INTEGER> dQRefNum;
    GUEST<INTEGER> dQFSID;
    GUEST<SignedByte> twoSideFmt;
    GUEST<SignedByte> needsFlush;
    GUEST<INTEGER> diskErrs;
};

extern OSErr DiskEject(INTEGER rn);
extern OSErr SetTagBuffer(Ptr bp);
extern OSErr DriveStatus(INTEGER dn, DrvSts *statp);
}
#endif /* __DISK__ */
