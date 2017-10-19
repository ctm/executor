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

namespace Executor {
#define evtNotEnb	1

struct EvQEl { GUEST_STRUCT;
    GUEST< QElemPtr> qLink;
    GUEST< INTEGER> qType;
    GUEST< INTEGER> evtQWhat;
    GUEST< LONGINT> evtQMessage;
    GUEST< LONGINT> evtQWhen;
    GUEST< Point> evtQWhere;
    GUEST< INTEGER> evtQModifiers;
};

typedef EvQEl *EvQElPtr;
MAKE_HIDDEN(EvQElPtr);

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


/*
 * The first three fields are used to dereference mac memory.  The two
 * extra booleans are for our own use.
 */


    /* extra */
typedef struct size_info { GUEST_STRUCT;
    GUEST< int16> size_flags;
    GUEST< int32> preferred_size;
    GUEST< int32> minimum_size;
    GUEST< boolean_t> size_resource_present_p;
    GUEST< boolean_t> application_p;
} size_info_t;

extern size_info_t size_info;

#if 0
#if !defined (Ticks_UL)
extern HIDDEN_ULONGINT Ticks_UL;
extern INTEGER 	monkeylives;
extern INTEGER 	SysEvtMask;
extern QHdr 	EventQueue;
#endif

#define Ticks	(Ticks_UL.u)
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

typedef struct TargetID { GUEST_STRUCT;
    GUEST< int32> sessionID;
    GUEST< PPCPortRec> name;
    GUEST< LocationNameRec> location;
    GUEST< PPCPortRec> recvrName;
} TargetIDPtr;

typedef struct HighLevelEventMsg { GUEST_STRUCT;
    GUEST< int16> HighLevelEventMsgHeaderlength;
    GUEST< int16> version;
    GUEST< int32> reserved1;
    GUEST< EventRecord> theMsgEvent;
    GUEST< int32> userRefCon;
    GUEST< int32> postingOptions;
    GUEST< int32> msgLength;
} *HighLevelEventMsgPtr;

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
}

#endif /* !_OSEVENT_H_ */
