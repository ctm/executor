#if !defined(__SERIAL__)
#define __SERIAL__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
enum
{
    baud300 = 380,
    baud600 = 189,
    baud1200 = 94,
    baud1800 = 62,
    baud2400 = 46,
    baud3600 = 30,
    baud4800 = 22,
    baud7200 = 14,
    baud9600 = 10,
    baud19200 = 4,
    baud57600 = 0,
};

enum
{
    stop10 = 16384,
    stop15 = (-32768),
    stop20 = (-16384),
};

enum
{
    noParity = 0,
    oddParity = 4096,
    evenParity = 12288,
};

enum
{
    data5 = 0,
    data6 = 2048,
    data7 = 1024,
    data8 = 3072,
};

enum
{
    swOverrunErr = 1,
    parityErr = 16,
    hwOverrunErr = 32,
    framingErr = 64,
};

enum
{
    ctsEvent = 32,
    breakEvent = 128,
};

enum
{
    xOffWasSent = 0x80,
};


typedef SignedByte SPortSel;
enum
{
    sPortA = 0,
    sPortB = 1,
};


struct SerShk
{
    GUEST_STRUCT;
    GUEST<Byte> fXOn;
    GUEST<Byte> fCTS;
    GUEST<Byte> xOn;
    GUEST<Byte> xOff;
    GUEST<Byte> errs;
    GUEST<Byte> evts;
    GUEST<Byte> fInX;
    GUEST<Byte> null;
};

struct SerStaRec
{
    GUEST_STRUCT;
    GUEST<Byte> cumErrs;
    GUEST<Byte> xOffSent;
    GUEST<Byte> rdPend;
    GUEST<Byte> wrPend;
    GUEST<Byte> ctsHold;
    GUEST<Byte> xOffHold;
    GUEST<Byte> dsrHold;    // unimplemented
    GUEST<Byte> modemStatus;// unimplemented
};

const char *const MODEMINAME = ".AIn";
const char *const MODEMONAME = ".AOut";
const char *const PRNTRINAME = ".AIn";
const char *const PRNTRONAME = ".AOut";

enum
{
    MODEMIRNUM = (-6),
    MODEMORNUM = (-7),
    PRNTRIRNUM = (-8),
    PRNTRORNUM = (-9),
};

extern OSErr RAMSDOpen(SPortSel port);
extern void RAMSDClose(SPortSel port);
extern OSErr SerReset(INTEGER rn, INTEGER config);
extern OSErr SerSetBuf(INTEGER rn, Ptr p, INTEGER len);
extern OSErr SerHShake(INTEGER rn, SerShk flags);
extern OSErr SerSetBrk(INTEGER rn);
extern OSErr SerClrBrk(INTEGER rn);
extern OSErr SerGetBuf(INTEGER rn, LONGINT *lp);
extern OSErr SerStatus(INTEGER rn, SerStaRec *serstap);
}

#endif /* __SERIAL__ */
