/*
 * This program attempts to read a file from an HFS disk without using
 * the file manager; only the device manager.  HA!  Begun Feb-5-91.
 */

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "myhfs.h"

#define TESTSIZE    100

#define NELEM(x)    (sizeof(x) / sizeof(x[0]))

PRIVATE INTEGER dirindex = 0, fileindex = 0;

PRIVATE char *dirnames[] = {
    "\pdirectory0:",
    "\pdir1:",
    "\pd:",
    "\pd3:",
    "\pdi4:",
    "\pcliff's directory:",
    "\pdirt with 31 characters in name:",
    "\panother dirname:",
    "\pcliff's a weasel:",
    "\pI shouldn't:",
    "\phave to put these colons:",
    "\pin here:",
    "\pyet another dir:",
    "\pwill wonders never cease:",
#if 0
    "\pfifteen:",
    "\psixteen:",
    "\pseventeen:",
    "\peighteen:",
#endif
};

PRIVATE char *filenames[] = {
    "\pjerry",
    "\pbob",
    "\pmickey",
    "\pBrent",
    "\pPig-Pen",
    "\pKeith",
    "\pDonna",
    "\pbilly",
    "\pphil",
    "\pa",
    "\pab",
    "\pa relatively long name",
    "\pfile with 31 characters in name",
    "\p1",
    "\pl2",
    "\p123",
    "\p1234",
    "\pj2345",
    "\po23456",
    "\pz234567",
    "\pk2345678",
    "\py23456789",
    "\pe234567890",
    "\p12345678901",
    "\pn23456789012",
    "\pf234567890123",
    "\pt2345678901234",
    "\pi23456789012345",
    "\pu234567890123456",
    "\pv2345678901234567",
    "\pb23456789012345678",
    "\pd234567890123456789",
    "\pm2345678901234567890",
    "\pw23456789012345678901",
    "\pr234567890123456789012",
    "\pa2345678901234567890123",
    "\px23456789012345678901234",
    "\pg234567890123456789012345",
    "\ps2345678901234567890123456",
    "\pc23456789012345678901234567",
    "\pq234567890123456789012345678",
    "\ph2345678901234567890123456789",
    "\pp23456789012345678901234567890",
    "\pone",
    "\ptwo",
    "\pthree",
    "\pfour",
    "\pfive",
    "\psix",
    "\pseven",
    "\peight",
    "\pnine",
    "\pten",
    "\peleven",
    "\ptwelve",
};

extern OSErr myPBCreate   (ioParam    *pb, BOOLEAN async);
extern OSErr myPBDelete   (ioParam    *pb, BOOLEAN async);
extern OSErr myPBDirCreate(HFileParam *pb, BOOLEAN async);
extern OSErr myPBHDelete  (HFileParam *pb, BOOLEAN async);

PRIVATE void createnfiles(INTEGER level, StringPtr prefixp, INTEGER n)
{
    Str255 name;
    ioParam pb;
    HFileParam hp;
    unsigned char *munglocp;
    int namelen, i;
    OSErr err;
    
    str255assign(name, prefixp);
    munglocp = name + name[0];
    if (level == 0) {
	pb.ioNamePtr = name;
	pb.ioVRefNum = 0;
	for (i = n; --i >= 0;) {
	    str255assign(munglocp, (StringPtr) filenames[fileindex++ % NELEM(filenames)]);
	    namelen = *munglocp;
	    name[0] += namelen;
	    *munglocp = ':';
	    printf("cf %d\n", fileindex);
	    err = myPBCreate(&pb, FALSE);
	    if (err != noErr)
		DebugStr((StringPtr) "\ppbcreate failed");
	    name[0] -= namelen;
	}
    } else {
	--level;
	hp.ioNamePtr = name;
	hp.ioVRefNum = 0;
	for (i = n; --i >= 0;) {
	    str255assign(munglocp, (StringPtr) dirnames[dirindex++ % NELEM(dirnames)]);
	    namelen = *munglocp;
	    name[0] += namelen-1;
	    *munglocp = ':';
	    printf("cd %d\n", dirindex);
	    err = myPBDirCreate(&hp, FALSE);
	    if (err != noErr)
		DebugStr((StringPtr) "\ppbdirCreate failed");
	    name[0] += 1;
	    createnfiles(level, name, n);
	    name[0] -= namelen;
	}
    }
}

PRIVATE void deletenfiles(INTEGER level, StringPtr prefixp, INTEGER n)
{
    Str255 name;
    ioParam pb;
    HFileParam hp;
    unsigned char *munglocp;
    int namelen, i;
    OSErr err;
    
    str255assign(name, prefixp);
    munglocp = name + name[0];
    if (level == 0) {
	pb.ioNamePtr = name;
	pb.ioVRefNum = 0;
	for (i = n; --i >= 0;) {
	    str255assign(munglocp, (StringPtr) filenames[fileindex++ % NELEM(filenames)]);
	    namelen = *munglocp;
	    name[0] += namelen;
	    *munglocp = ':';
	    printf("df %d\n", fileindex);
#if 0
	    if (fileindex == 109)
	        DebugStr("\pabout to do 109");
#endif
	    err = myPBDelete(&pb, FALSE);
	    if (err != noErr)
		DebugStr((StringPtr) "\pmyPBDelete failed");
	    name[0] -= namelen;
	}
    } else {
	--level;
	hp.ioNamePtr = name;
	hp.ioVRefNum = 0;
	for (i = n; --i >= 0;) {
	    str255assign(munglocp, (StringPtr) dirnames[dirindex++ % NELEM(dirnames)]);
	    namelen = *munglocp;
	    *munglocp = ':';
	    name[0] += namelen;
	    deletenfiles(level, name, n);
	    name[0] -= 1;
	    printf("dd %d\n", dirindex);
	    err = myPBHDelete(&hp, FALSE);
	    if (err != noErr)
		DebugStr((StringPtr) "\pmyPBHDelete failed");
	    name[0] -= namelen-1;
	}
    }
}

PUBLIC void main( void )
{
    ioParam pb;
#if 0
    HFileParam hp;
    char testbuffer[TESTSIZE];
#endif
    OSErr err;
#if 0
    volumeinfoHandle vh;
    Ptr p2;
    ulong *lp;
    cacheentry *cachep;
    
    err = SetVol("\pMyVol:", 0);
    err = getcache(&cachep, ((HVCB *)DefVCBPtr)->vcbXTRef, (ulong) 0,
								  GETCACHESAVE);
    err = err;
#endif

#if 0
    testfcb();
    return;
#endif

#if 0
    pb.ioNamePtr = (StringPtr) "\pmyvol:mail:jody";
    pb.ioPermssn = fsRdPerm;
    pb.ioMisc = 0;
    myPBOpen(&pb, FALSE);
    
    pb.ioPosMode = fsFromStart;
    pb.ioPosOffset = 0;
    pb.ioReqCount = TESTSIZE;
    pb.ioBuffer = testbuffer;
    myPBRead(&pb, FALSE);
    myPBClose(&pb, FALSE);
#endif

#if 0
    pb.ioNamePtr = (StringPtr) "\pMyVol:Cliff's file";
    err = PBCreate(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:Cliff's second file";
    err = PBCreate(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:Cliff's third file";
    err = PBCreate(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:Cliff's fourth file";
    err = PBCreate(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:Cliff's fifth file";
    err = PBCreate(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:";
    err = PBFlushVol(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:Cliff's file";
    err = myPBDelete(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:Cliff's second file";
    err = myPBDelete(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:Cliff's third file";
    err = myPBDelete(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:Cliff's fourth file";
    err = myPBDelete(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:Cliff's fifth file";
    err = myPBDelete(&pb, FALSE);
#endif
    
#if 0
    hp.ioNamePtr = (StringPtr) "\pMyVol:testdir1";
    hp.ioDirID = 600;
    hp.ioVRefNum = 0;
    err = PBDirCreate(&hp, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:";
    err = PBFlushVol(&pb, FALSE);
    err = myPBHDelete(&hp, FALSE);
#endif
    
#if 0
    pb.ioNamePtr = (StringPtr) "\pMyVol:testdir1:testdir1's file";
    pb.ioRefNum = 0;
    err = myPBCreate(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:testdir1:testdir1's second file";
    err = myPBCreate(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:testdir1:testdir1's third file";
    err = myPBCreate(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:testdir1:testdir1's fourth file";
    err = myPBCreate(&pb, FALSE);
    pb.ioNamePtr = (StringPtr) "\pMyVol:testdir1:testdir1's fifth file";
    err = myPBCreate(&pb, FALSE);
#endif

#if 1
    pb.ioNamePtr = (StringPtr) "\pMyVol:";
    err = PBFlushVol((ParmBlkPtr) &pb, FALSE);
    if (err != noErr)
	DebugStr((StringPtr) "\pPBFlushVol fails");
#endif
#if 1
#if 1
    createnfiles(3, (StringPtr) "\pMyVol:", 5);
#endif
#if 0
    pb.ioNamePtr = (StringPtr) "\pMyVol:";
    err = PBFlushVol((ParmBlkPtr) &pb, FALSE);
    if (err != noErr)
	DebugStr((StringPtr) "\pPBFlushVol fails");
#endif
#if 1
    dirindex = 0;
    fileindex = 0;
    deletenfiles(3, (StringPtr) "\pMyVol:", 5);
#endif
#if 0
    pb.ioNamePtr = (StringPtr) "\pMyVol:";
    err = PBFlushVol((ParmBlkPtr) &pb, FALSE);
    if (err != noErr)
	DebugStr((StringPtr) "\pPBFlushVol fails");
#endif
#endif
}
