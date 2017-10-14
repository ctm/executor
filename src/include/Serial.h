#if !defined (__SERIAL__)
#define __SERIAL__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: Serial.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
#if defined (USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)
#define __Byte uint8
#define __SignedByte int8
#define __OSErr int32
#define __Ptr void *
#else
#define __Byte Byte
#define __SignedByte SignedByte
#define __OSErr OSErr
#define __Ptr Ptr
#endif

#define baud300		380
#define baud600		189
#define baud1200	 94
#define baud1800	 62
#define baud2400	 46
#define baud3600	 30
#define baud4800	 22
#define baud7200	 14
#define baud9600	 10
#define baud19200	  4
#define baud57600	  0

#define stop10	16384
#define stop15	(-32768)
#define stop20	(-16384)

#define noParity	0
#define oddParity	4096
#define evenParity	12288

#define data5	0
#define data6	2048
#define data7	1024
#define data8	3072

#define swOverrunErr	1
#define parityErr	16
#define hwOverrunErr	32
#define framingErr	64

#define ctsEvent	32
#define breakEvent	128

#define xOffWasSent	0x80

#if !defined (BINCOMPAT)

typedef enum { sPortA, sPortB } SPortSel;

#else /* BINCOMPAT */

typedef __SignedByte SPortSel;
#define sPortA 0
#define sPortB 1

#endif /* BINCOMPAT */

struct SerShk : GuestStruct {
    GUEST< __Byte> fXOn;
    GUEST< __Byte> fCTS;
    GUEST< __Byte> xOn;
    GUEST< __Byte> xOff;
    GUEST< __Byte> errs;
    GUEST< __Byte> evts;
    GUEST< __Byte> fInX;
    GUEST< __Byte> null;
};

struct SerStaRec : GuestStruct {
    GUEST< __Byte> cumErrs;
    GUEST< __Byte> xOffSent;
    GUEST< __Byte> rdPend;
    GUEST< __Byte> wrPend;
    GUEST< __Byte> ctsHold;
    GUEST< __Byte> xOffHold;
};

#define MODEMINAME	".AIn"
#define MODEMONAME	".AOut"
#define PRNTRINAME	".AIn"
#define PRNTRONAME	".AOut"
#define MODEMIRNUM	(-6)
#define MODEMORNUM	(-7)
#define PRNTRIRNUM	(-8)
#define PRNTRORNUM	(-9)


/* DO NOT DELETE THIS LINE */
#if !defined (__STDC__)
extern __OSErr RAMSDOpen(); 
extern void RAMSDClose(); 
extern __OSErr SerReset(); 
extern __OSErr SerSetBuf(); 
extern __OSErr SerHShake(); 
extern __OSErr SerSetBrk(); 
extern __OSErr SerClrBrk(); 
extern __OSErr SerGetBuf(); 
extern __OSErr SerStatus(); 
#else /* __STDC__ */
extern __OSErr RAMSDOpen( SPortSel port ); 
extern void RAMSDClose( SPortSel port ); 
extern __OSErr SerReset( INTEGER rn, INTEGER config ); 
extern __OSErr SerSetBuf( INTEGER rn, __Ptr p, INTEGER len ); 
extern __OSErr SerHShake( INTEGER rn, SerShk flags ); 
extern __OSErr SerSetBrk( INTEGER rn ); 
extern __OSErr SerClrBrk( INTEGER rn ); 
extern __OSErr SerGetBuf( INTEGER rn, LONGINT *lp ); 
extern __OSErr SerStatus( INTEGER rn, SerStaRec *serstap ); 
#endif /* __STDC__ */

#if defined (USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)
#undef __Byte
#undef __SignedByte
#undef __OSErr
#undef __Ptr
#endif
}

#endif /* __SERIAL__ */
