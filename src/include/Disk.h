#if !defined(__DISK__)
#define __DISK__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: Disk.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor
{
#define nsDrvErr (-56)
#define paramErr (-50)
#define wPrErr (-44)
#define firstDskErr (-84)
#define sectNFErr (-81)
#define seekErr (-80)
#define spdAdjErr (-79)
#define twoSideErr (-78)
#define initIWMErr (-77)
#define tk0BadErr (-76)
#define cantStepErr (-75)
#define wrUnderrun (-74)
#define badDBtSlp (-73)
#define badDCksum (-72)
#define noDtaMkErr (-71)
#define badBtSlpErr (-70)
#define badCksmErr (-69)
#define dataVerErr (-68)
#define noAdrMkErr (-67)
#define noNybErr (-66)
#define offLinErr (-65)
#define noDriveErr (-64)
#define lastDskErr (-64)

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

/* DO NOT DELETE THIS LINE */
#if !defined(__STDC__)
extern OSErr DiskEject();
extern OSErr SetTagBuffer();
extern OSErr DriveStatus();
#else /* __STDC__ */
extern OSErr DiskEject(INTEGER rn);
extern OSErr SetTagBuffer(Ptr bp);
extern OSErr DriveStatus(INTEGER dn, DrvSts *statp);
#endif /* __STDC__ */
}
#endif /* __DISK__ */
