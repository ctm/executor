#if !defined (__OSUTIL__)
#define __OSUTIL__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: OSUtil.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "VRetraceMgr.h"
#include "FileMgr.h"
#include "OSEvent.h"

namespace Executor {
#define macXLMachine	0
#define macMachine	1
#define UNIXMachine	1127

#define clkRdErr	(-85)
#define clkWrErr	(-86)
#define memFullErr	(-108)
#define memWZErr	(-111)
#define nilHandleErr	(-109)
#define prInitErr	(-88)
#define prWrErr		(-87)

typedef struct PACKED {
  Byte valid;
  Byte aTalkA;
  Byte aTalkB;
  Byte config;
  INTEGER portA;
  INTEGER portB;
  LONGINT alarm;
  INTEGER font;
  INTEGER kbdPrint;
  INTEGER volClik;
  INTEGER misc;
} SysParmType;
typedef SysParmType *SysPPtr;

typedef enum { dummyType, vType, ioQType, drvQType, evType, fsQType } QTypes;

union __qe {
    VBLTask vblQElem;
    ParamBlockRec ioQElem;
    DrvQEl drvQElem;
    EvQEl evQElem;
    VCB vcbQElem;
};
typedef union __qe QElem;

typedef struct PACKED {
  INTEGER year;
  INTEGER month;
  INTEGER day;
  INTEGER hour;
  INTEGER minute;
  INTEGER second;
  INTEGER dayOfWeek;
} DateTimeRec;

typedef struct PACKED {
  INTEGER environsVersion;
  INTEGER machineType;
  INTEGER systemVersion;
  INTEGER processor;
  BOOLEAN hasFPU;
  BOOLEAN hasColorQD;
  INTEGER keyBoardType;
  INTEGER atDrvrVersNum;
  INTEGER sysVRefNum;
} SysEnvRec, *SysEnvRecPtr;

#define SYSRECVNUM	2

/* sysEnv machine types */
#define envMachUnknown	0
#define env512KE	1
#define envMacPlus	2
#define envSE		3
#define envMacII	4
#define envMac		-1
#define envXL		-2

#define envCPUUnknown	0
#define env68000	1
#define env68020	3
#define env68030	4
#define env68040	5

#define envUnknownKbd	0
#define envMacKbd	1
#define envMacAndPad	2
#define envMacPlusKbd	3
#define envAExtendKbd	4
#define envStandADBKbd	5

#define envBadVers	(-5501)
#define envVersTooBig	(-5502)

#define OSTrap	0
#define ToolTrap	1


#if !defined (SysVersion)
extern INTEGER 	SysVersion;
extern Byte 	SPValid;
extern Byte 	SPATalkA;
extern Byte 	SPATalkB;
extern Byte 	SPConfig;
extern INTEGER 	SPPortA;
extern INTEGER 	SPPortB;
extern LONGINT 	SPAlarm;
extern INTEGER 	SPFont;
extern Byte 	SPKbd;
extern Byte 	SPPrint;
extern Byte 	SPVolCtl;
extern Byte 	SPClikCaret;
extern Byte 	SPMisc2;
extern ULONGINT  Time;
extern Byte	MMU32Bit;
extern Byte MMUType;
extern Byte KbdType;
#endif

#if !defined (__STDC__)
extern OSErrRET HandToHand(); 
extern OSErrRET PtrToHand(); 
extern OSErrRET PtrToXHand(); 
extern OSErrRET HandAndHand(); 
extern OSErrRET PtrAndHand(); 
extern LONGINT ROMlib_RelString(); 
extern INTEGERRET RelString(); 
extern BOOLEANRET EqualString(); 
extern void ROMlib_UprString(); 
extern void UprString(); 
extern void GetDateTime(); 
extern OSErrRET ReadDateTime(); 
extern OSErrRET SetDateTime(); 
extern void Date2Secs(); 
extern void Secs2Date(); 
extern void GetTime(); 
extern void SetTime(); 
extern OSErrRET InitUtil(); 
extern SysPPtr GetSysPPtr(); 
extern OSErrRET WriteParam(); 
extern void Enqueue(); 
extern OSErrRET Dequeue(); 
extern LONGINT GetTrapAddress(); 
extern LONGINT NGetTrapAddress(); 
extern void SetTrapAddress(); 
extern void Delay(); 
extern void SysBeep(); 
extern void Environs(); 
extern OSErrRET SysEnvirons(); 
extern void Restart(); 
extern void SetUpA5(); 
extern void RestoreA5(); 
extern void GetMMUMode(); 
extern void SwapMMUMode(); 
extern LONGINT StripAddress(); 
#else /* __STDC__ */
extern trap OSErrRET HandToHand( HIDDEN_Handle *h ); 
extern trap OSErrRET PtrToHand( Ptr p, HIDDEN_Handle *h, LONGINT s ); 
extern trap OSErrRET PtrToXHand( Ptr p, Handle h, LONGINT s ); 
extern trap OSErrRET HandAndHand( Handle h1, Handle h2 ); 
extern trap OSErrRET PtrAndHand( Ptr p, Handle h, LONGINT s1 ); 
extern LONGINT ROMlib_RelString( unsigned char *s1, unsigned char *s2, 
 BOOLEAN casesig, BOOLEAN diacsig, LONGINT d0 ); 
extern trap INTEGERRET RelString( StringPtr s1, StringPtr s2, 
 BOOLEAN casesig, BOOLEAN diacsig ); 
extern trap BOOLEANRET EqualString( StringPtr s1, StringPtr s2, 
 BOOLEAN casesig, BOOLEAN diacsig ); 
extern void ROMlib_UprString( StringPtr s, BOOLEAN diac, INTEGER len ); 
extern trap void UprString( StringPtr s, BOOLEAN diac ); 
extern void GetDateTime( LONGINT *mactimepointer ); 
extern trap OSErrRET ReadDateTime( LONGINT *secs ); 
extern trap OSErrRET SetDateTime( LONGINT mactime ); 
extern trap void Date2Secs( DateTimeRec *d, LONGINT *s ); 
extern trap void Secs2Date( LONGINT mactime, DateTimeRec *d ); 
extern void GetTime( DateTimeRec *d ); 
extern void SetTime( DateTimeRec *d ); 
extern trap OSErrRET InitUtil( void  ); 
extern SysPPtr GetSysPPtr( void  ); 
extern trap OSErrRET WriteParam( void  ); 
extern trap void Enqueue( QElemPtr e, QHdrPtr h ); 
extern trap OSErrRET Dequeue( QElemPtr e, QHdrPtr h ); 
extern LONGINT GetTrapAddress( INTEGER n ); 
extern LONGINT NGetTrapAddress( INTEGER n, INTEGER ttype ); 
extern void SetTrapAddress( LONGINT addr, 
 INTEGER n ); 
extern trap void Delay( LONGINT n, LONGINT *ftp ); 
extern pascal trap void C_SysBeep( INTEGER i ); extern pascal trap void P_SysBeep( INTEGER i); 
extern trap void Environs( INTEGER *rom, INTEGER *machine ); 
extern trap OSErrRET SysEnvirons( INTEGER vers, SysEnvRecPtr p ); 
extern void Restart( void  ); 
extern void SetUpA5( void  ); 
extern void RestoreA5( void  );
#undef GetMMUMode
#undef SwapMMUMode
extern void GetMMUMode( INTEGER *ip );
extern void SwapMMUMode( Byte *bp ); 
extern LONGINT StripAddress( LONGINT l ); 
#endif /* __STDC__ */
}
#endif /* __OSUTIL__ */
