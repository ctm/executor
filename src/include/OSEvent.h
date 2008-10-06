#if !defined (_OSEVENT_H_)
#define _OSEVENT_H_

#include "EventMgr.h"
#include "PPC.h"

#include "rsys/commonevt.h"

/*
 * Copyright 1986, 1989, 1990, 1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: OSEvent.h 63 2004-12-24 18:19:43Z ctm $
 */

#define evtNotEnb	1

typedef struct {
    QElemPtr qLink	PACKED_P;
    INTEGER qType	PACKED;
    INTEGER evtQWhat	PACKED;
    LONGINT evtQMessage	PACKED;
    LONGINT evtQWhen	PACKED;
    Point evtQWhere	PACKED;
    INTEGER evtQModifiers	PACKED;
} EvQEl;

typedef EvQEl *EvQElPtr;
typedef struct { EvQElPtr p PACKED_P; } HIDDEN_EvQElPtr;

#define osEvt   15
#define SUSPENDRESUMEBITS       0x01000000
#define SUSPEND                 (0 << 0)
#define RESUME                  (1 << 0)
#define CONVERTCLIPBOARD        (1 << 1)
#define mouseMovedMessage	0xFA

typedef enum
{
  SZreserved0			= (1 << 15),
  SZacceptSuspendResumeEvents	= (1 << 14),
  SZreserved1			= (1 << 13),
  SZcanBackground		= (1 << 12),
  SZdoesActivateOnFGSwitch	= (1 << 11),
  SZonlyBackground		= (1 << 10),
  SZgetFrontClicks		= (1 <<  9),
  SZAcceptAppDiedEvents		= (1 <<  8),
  SZis32BitCompatible		= (1 <<  7),
  SZisHighLevelEventAware	= (1 <<  6),
  SZlocalAndRemoveHLEvents	= (1 <<  5),
  SZisStationeryAware		= (1 <<  4),
  SZuseTextEditServices		= (1 <<  3),
  SZreserved2			= (1 <<  2),
  SZreserved3			= (1 <<  1),
  SZreserved4			= (1 <<  0)
} SZ_t;

typedef struct size_info
{
  int16 size_flags		PACKED;
  int32 preferred_size		PACKED;
  int32 minimum_size		PACKED;
  
  /* extra */
  boolean_t size_resource_present_p;
  boolean_t application_p;
} size_info_t;

extern size_info_t size_info;

#if !defined (Ticks_UL)
extern HIDDEN_ULONGINT Ticks_UL;
extern INTEGER 	monkeylives;
extern INTEGER 	SysEvtMask;
extern QHdr 	EventQueue;
#endif

#if !defined (SHORT_TICKS)

#define Ticks	(Ticks_UL.u)
#define CRACKER_ZERO (0)

#else

#if !defined (SHORT_TICKS_SHIFT)
#define SHORT_TICKS_SHIFT 0
#endif

#define Ticks ((ULONGINT) (*(unsigned long *)SYN68K_TO_US(0x16c)) \
	       & ((1<<(16+SHORT_TICKS_SHIFT))-1))

/* This macro evaluates to *(short *) 0x16a, but the assembly code
 * generated varies depending on what line the macro is invoked!
 */

#define CRACKER_ZERO 				\
(CW(*(short *) ({ volatile long a = __LINE__ * 17;	\
       *&a + ((SYN68K_TO_US(0x16a)) - __LINE__ * 17); })) >> SHORT_TICKS_SHIFT)

#endif


extern void ROMlib_eventdep( void  ); 
extern void insertcommonevent( char *xeventp, commonevent *comevtp ); 
extern void ROMlib_zapmap( LONGINT loc, LONGINT val ); 
extern trap OSErrRET PPostEvent( INTEGER evcode, 
 LONGINT evmsg, HIDDEN_EvQElPtr *qelp ); 
extern OSErrRET ROMlib_PPostEvent( INTEGER evcode, LONGINT evmsg, 
 HIDDEN_EvQElPtr *qelp, LONGINT when, Point where, INTEGER butmods ); 
extern trap OSErrRET PostEvent( INTEGER evcode, LONGINT evmsg ); 
extern trap void FlushEvents( INTEGER evmask, 
 INTEGER stopmask ); 
extern trap BOOLEANRET GetOSEvent( INTEGER evmask, EventRecord *eventp ); 
extern trap BOOLEANRET OSEventAvail( INTEGER evmask, 
 EventRecord *eventp ); 
extern trap void SetEventMask( INTEGER evmask ); 
extern QHdrPtr GetEvQHdr( void  );

extern EvQEl *geteventelem (void);

typedef struct TargetID
{
  int32 sessionID		PACKED;
  PPCPortRec name		PACKED;
  LocationNameRec location	PACKED;
  PPCPortRec recvrName		PACKED;
} TargetID, TargetIDPtr;

typedef struct HighLevelEventMsg
{
  int16 HighLevelEventMsgHeaderlength	PACKED;
  int16 version				PACKED;
  int32 reserved1			PACKED;
  EventRecord theMsgEvent		PACKED;
  int32 userRefCon			PACKED;
  int32 postingOptions			PACKED;
  int32 msgLength			PACKED;
} HighLevelEventMsg, *HighLevelEventMsgPtr; 

typedef ProcPtr GetSpecificFilterProcPtr;

#define noOutstandingHLE	(-607)
#define bufferIsSmall		(-608)

extern pascal trap OSErr C_AcceptHighLevelEvent (TargetID *sender_id_return, int32 *refcon_return, Ptr msg_buf, int32 *msg_length_return);
extern pascal trap Boolean C_GetSpecificHighLevelEvent (GetSpecificFilterProcPtr fn, Ptr data, OSErr * err_return);
extern pascal trap OSErr C_PostHighLevelEvent (EventRecord *evt, Ptr receiver_id, int32 refcon, Ptr msg_buf, int32 msg_length, int32 post_options);

/* #### move to rsys/foo.h */
extern boolean_t hle_get_event (EventRecord *evt, boolean_t remflag);
extern void hle_init (void);
extern void hle_reinit (void);
extern void hle_reset (void);

#endif /* !_OSEVENT_H_ */
