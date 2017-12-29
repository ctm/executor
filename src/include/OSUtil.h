#if !defined(__OSUTIL__)
#define __OSUTIL__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "VRetraceMgr.h"
#include "FileMgr.h"
#include "OSEvent.h"

namespace Executor
{
enum
{
    macXLMachine = 0,
    macMachine = 1,
    UNIXMachine = 1127,
};

enum
{
    clkRdErr = (-85),
    clkWrErr = (-86),
    prInitErr = (-88),
    prWrErr = (-87),
};

struct SysParmType
{
    GUEST_STRUCT;
    GUEST<Byte> valid;
    GUEST<Byte> aTalkA;
    GUEST<Byte> aTalkB;
    GUEST<Byte> config;
    GUEST<INTEGER> portA;
    GUEST<INTEGER> portB;
    GUEST<LONGINT> alarm;
    GUEST<INTEGER> font;
    GUEST<INTEGER> kbdPrint;
    GUEST<INTEGER> volClik;
    GUEST<INTEGER> misc;
};
typedef SysParmType *SysPPtr;

typedef enum { dummyType,
               vType,
               ioQType,
               drvQType,
               evType,
               fsQType } QTypes;

union __qe {
    VBLTask vblQElem;
    ParamBlockRec ioQElem;
    DrvQEl drvQElem;
    EvQEl evQElem;
    VCB vcbQElem;
};
typedef union __qe QElem;

struct DateTimeRec
{
    GUEST_STRUCT;
    GUEST<INTEGER> year;
    GUEST<INTEGER> month;
    GUEST<INTEGER> day;
    GUEST<INTEGER> hour;
    GUEST<INTEGER> minute;
    GUEST<INTEGER> second;
    GUEST<INTEGER> dayOfWeek;
};

typedef struct SysEnvRec
{
    GUEST_STRUCT;
    GUEST<INTEGER> environsVersion;
    GUEST<INTEGER> machineType;
    GUEST<INTEGER> systemVersion;
    GUEST<INTEGER> processor;
    GUEST<BOOLEAN> hasFPU;
    GUEST<BOOLEAN> hasColorQD;
    GUEST<INTEGER> keyBoardType;
    GUEST<INTEGER> atDrvrVersNum;
    GUEST<INTEGER> sysVRefNum;
} * SysEnvRecPtr;

enum
{
    SYSRECVNUM = 2,
};

/* sysEnv machine types */
enum
{
    envMachUnknown = 0,
    env512KE = 1,
    envMacPlus = 2,
    envSE = 3,
    envMacII = 4,
    envMac = -1,
    envXL = -2,
};

enum
{
    envCPUUnknown = 0,
    env68000 = 1,
    env68020 = 3,
    env68030 = 4,
    env68040 = 5,
};

enum
{
    envUnknownKbd = 0,
    envMacKbd = 1,
    envMacAndPad = 2,
    envMacPlusKbd = 3,
    envAExtendKbd = 4,
    envStandADBKbd = 5,
};

enum
{
    envBadVers = (-5501),
    envVersTooBig = (-5502),
};

enum
{
    OSTrap = 0,
    ToolTrap = 1,
};

#if !defined(SysVersion)
extern INTEGER SysVersion;
extern Byte SPValid;
extern Byte SPATalkA;
extern Byte SPATalkB;
extern Byte SPConfig;
extern INTEGER SPPortA;
extern INTEGER SPPortB;
extern LONGINT SPAlarm;
extern INTEGER SPFont;
extern Byte SPKbd;
extern Byte SPPrint;
extern Byte SPVolCtl;
extern Byte SPClikCaret;
extern Byte SPMisc2;
extern ULONGINT Time;
extern Byte MMU32Bit;
extern Byte MMUType;
extern Byte KbdType;
#endif

extern trap OSErrRET HandToHand(Handle *h);
extern trap OSErrRET PtrToHand(Ptr p, Handle *h, LONGINT s);
extern trap OSErrRET PtrToXHand(Ptr p, Handle h, LONGINT s);
extern trap OSErrRET HandAndHand(Handle h1, Handle h2);
extern trap OSErrRET PtrAndHand(Ptr p, Handle h, LONGINT s1);
extern LONGINT ROMlib_RelString(unsigned char *s1, unsigned char *s2,
                                BOOLEAN casesig, BOOLEAN diacsig, LONGINT d0);
extern trap INTEGERRET RelString(StringPtr s1, StringPtr s2,
                                 BOOLEAN casesig, BOOLEAN diacsig);
extern trap BOOLEANRET EqualString(StringPtr s1, StringPtr s2,
                                   BOOLEAN casesig, BOOLEAN diacsig);
extern void ROMlib_UprString(StringPtr s, BOOLEAN diac, INTEGER len);
extern trap void UprString(StringPtr s, BOOLEAN diac);
extern void GetDateTime(GUEST<ULONGINT> *mactimepointer);
extern trap OSErrRET ReadDateTime(GUEST<ULONGINT> *secs);
extern trap OSErrRET SetDateTime(ULONGINT mactime);
extern trap void Date2Secs(DateTimeRec *d, ULONGINT *s);
extern trap void Secs2Date(ULONGINT mactime, DateTimeRec *d);
extern void GetTime(DateTimeRec *d);
extern void SetTime(DateTimeRec *d);
extern trap OSErrRET InitUtil(void);
extern SysPPtr GetSysPPtr(void);
extern trap OSErrRET WriteParam(void);
extern trap void Enqueue(QElemPtr e, QHdrPtr h);
extern trap OSErrRET Dequeue(QElemPtr e, QHdrPtr h);
extern LONGINT GetTrapAddress(INTEGER n);
extern LONGINT NGetTrapAddress(INTEGER n, INTEGER ttype);
extern void SetTrapAddress(LONGINT addr,
                           INTEGER n);
extern trap void Delay(LONGINT n, LONGINT *ftp);
extern pascal trap void C_SysBeep(INTEGER i);
PASCAL_TRAP(SysBeep, 0xA9C8);

extern trap void Environs(GUEST<INTEGER> *rom, GUEST<INTEGER> *machine);
extern trap OSErrRET SysEnvirons(INTEGER vers, SysEnvRecPtr p);
extern void Restart(void);
extern void SetUpA5(void);
extern void RestoreA5(void);
#undef GetMMUMode
#undef SwapMMUMode
extern void GetMMUMode(GUEST<INTEGER> *ip);
extern void SwapMMUMode(Byte *bp);
extern LONGINT StripAddress(LONGINT l);

extern pascal trap void C_DebugStr(StringPtr p);
PASCAL_TRAP(DebugStr, 0xABFF);
}
#endif /* __OSUTIL__ */
