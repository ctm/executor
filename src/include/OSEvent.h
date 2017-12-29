#if !defined(_OSEVENT_H_)
#define _OSEVENT_H_

#include "EventMgr.h"
#include "PPC.h"

#include "rsys/commonevt.h"

/*
 * Copyright 1986, 1989, 1990, 1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
enum
{
    evtNotEnb = 1,
};

struct EvQEl
{
    GUEST_STRUCT;
    GUEST<QElemPtr> qLink;
    GUEST<INTEGER> qType;
    GUEST<INTEGER> evtQWhat;
    GUEST<LONGINT> evtQMessage;
    GUEST<LONGINT> evtQWhen;
    GUEST<Point> evtQWhere;
    GUEST<INTEGER> evtQModifiers;
};

typedef EvQEl *EvQElPtr;

enum
{
    osEvt = 15,
    SUSPENDRESUMEBITS = 0x01000000,
    SUSPEND = (0 << 0),
    RESUME = (1 << 0),
    CONVERTCLIPBOARD = (1 << 1),
    mouseMovedMessage = 0xFA,
};

typedef enum {
    SZreserved0 = (1 << 15),
    SZacceptSuspendResumeEvents = (1 << 14),
    SZreserved1 = (1 << 13),
    SZcanBackground = (1 << 12),
    SZdoesActivateOnFGSwitch = (1 << 11),
    SZonlyBackground = (1 << 10),
    SZgetFrontClicks = (1 << 9),
    SZAcceptAppDiedEvents = (1 << 8),
    SZis32BitCompatible = (1 << 7),
    SZisHighLevelEventAware = (1 << 6),
    SZlocalAndRemoveHLEvents = (1 << 5),
    SZisStationeryAware = (1 << 4),
    SZuseTextEditServices = (1 << 3),
    SZreserved2 = (1 << 2),
    SZreserved3 = (1 << 1),
    SZreserved4 = (1 << 0)
} SZ_t;

/*
 * The first three fields are used to dereference mac memory.  The two
 * extra booleans are for our own use.
 */

struct SIZEResource
{
    GUEST_STRUCT;
    GUEST<int16_t> size_flags;
    GUEST<int32_t> preferred_size;
    GUEST<int32_t> minimum_size;
};

typedef struct size_info
{
    GUEST_STRUCT;
    int16_t size_flags;
    int32_t preferred_size;
    int32_t minimum_size;
    bool size_resource_present_p;
    bool application_p;
} size_info_t;

extern size_info_t size_info;

#if 0
#if !defined(Ticks_UL)
extern GUEST<ULONGINT> Ticks_UL;
extern INTEGER 	monkeylives;
extern INTEGER 	SysEvtMask;
extern QHdr 	EventQueue;
#endif

enum
{
    Ticks = (Ticks_UL.u),
};
#endif

extern void ROMlib_eventdep(void);
extern void insertcommonevent(char *xeventp, commonevent *comevtp);
extern void ROMlib_zapmap(LONGINT loc, LONGINT val);
extern trap OSErrRET PPostEvent(INTEGER evcode,
                                LONGINT evmsg, GUEST<EvQElPtr> *qelp);
extern OSErrRET ROMlib_PPostEvent(INTEGER evcode, LONGINT evmsg,
                                  GUEST<EvQElPtr> *qelp, LONGINT when, Point where, INTEGER butmods);
extern trap OSErrRET PostEvent(INTEGER evcode, LONGINT evmsg);
extern trap void FlushEvents(INTEGER evmask,
                             INTEGER stopmask);
extern trap BOOLEANRET GetOSEvent(INTEGER evmask, EventRecord *eventp);
extern trap BOOLEANRET OSEventAvail(INTEGER evmask,
                                    EventRecord *eventp);
extern trap void SetEventMask(INTEGER evmask);
extern QHdrPtr GetEvQHdr(void);

extern EvQEl *geteventelem(void);

typedef struct TargetID
{
    GUEST_STRUCT;
    GUEST<int32_t> sessionID;
    GUEST<PPCPortRec> name;
    GUEST<LocationNameRec> location;
    GUEST<PPCPortRec> recvrName;
} TargetIDPtr;

typedef struct HighLevelEventMsg
{
    GUEST_STRUCT;
    GUEST<int16_t> HighLevelEventMsgHeaderlength;
    GUEST<int16_t> version;
    GUEST<int32_t> reserved1;
    GUEST<EventRecord> theMsgEvent;
    GUEST<int32_t> userRefCon;
    GUEST<int32_t> postingOptions;
    GUEST<int32_t> msgLength;
} * HighLevelEventMsgPtr;

typedef ProcPtr GetSpecificFilterProcPtr;

enum
{
    noOutstandingHLE = (-607),
    bufferIsSmall = (-608),
};

extern pascal trap OSErr C_AcceptHighLevelEvent(TargetID *sender_id_return, GUEST<int32_t> *refcon_return, Ptr msg_buf, GUEST<int32_t> *msg_length_return);
PASCAL_FUNCTION(AcceptHighLevelEvent);
extern pascal trap Boolean C_GetSpecificHighLevelEvent(GetSpecificFilterProcPtr fn, Ptr data, OSErr *err_return);
PASCAL_FUNCTION(GetSpecificHighLevelEvent);
extern pascal trap OSErr C_PostHighLevelEvent(EventRecord *evt, Ptr receiver_id, int32_t refcon, Ptr msg_buf, int32_t msg_length, int32_t post_options);
PASCAL_FUNCTION(PostHighLevelEvent);

/* #### move to rsys/foo.h */
extern bool hle_get_event(EventRecord *evt, bool remflag);
extern void hle_init(void);
extern void hle_reinit(void);
extern void hle_reset(void);
}

#endif /* !_OSEVENT_H_ */
