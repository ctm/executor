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

typedef struct size_info    /* executor-only struct */
{
    int16_t size_flags;
    int32_t preferred_size;
    int32_t minimum_size;
    bool size_resource_present_p;
    bool application_p;
} size_info_t;

extern size_info_t size_info;

const LowMemGlobal<INTEGER> monkeylives { 0x100 }; // OSEvent SysEqu.a (true-b);
const LowMemGlobal<INTEGER> SysEvtMask { 0x144 }; // OSEvent IMII-70 (true);
const LowMemGlobal<QHdr> EventQueue { 0x14A }; // OSEvent IMII-71 (true);
const LowMemGlobal<ULONGINT> Ticks { 0x16A }; // OSEvent IMI-260 (true);
const LowMemGlobal<Byte> MBState { 0x172 }; // EventMgr PegLeg (True-b);
const LowMemGlobal<Point> MTemp { 0x828 }; // QuickDraw PegLeg (True-b);
const LowMemGlobal<Point> MouseLocation { 0x82C }; // QuickDraw Vamp (true);
const LowMemGlobal<Point> MouseLocation2 { 0x830 }; // QuickDraw MacAttack (true);

extern void ROMlib_eventdep(void);
extern void insertcommonevent(char *xeventp, commonevent *comevtp);
extern void ROMlib_zapmap(LONGINT loc, LONGINT val);
extern OSErrRET PPostEvent(INTEGER evcode,
                                LONGINT evmsg, GUEST<EvQElPtr> *qelp);
extern OSErrRET ROMlib_PPostEvent(INTEGER evcode, LONGINT evmsg,
                                  GUEST<EvQElPtr> *qelp, LONGINT when, Point where, INTEGER butmods);
extern OSErrRET PostEvent(INTEGER evcode, LONGINT evmsg);
extern void FlushEvents(INTEGER evmask,
                             INTEGER stopmask);
extern BOOLEANRET GetOSEvent(INTEGER evmask, EventRecord *eventp);
extern BOOLEANRET OSEventAvail(INTEGER evmask,
                                    EventRecord *eventp);
extern void SetEventMask(INTEGER evmask);
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

typedef UPP<Boolean(Ptr, HighLevelEventMsgPtr, TargetID *)> GetSpecificFilterProcPtr;

enum
{
    noOutstandingHLE = (-607),
    bufferIsSmall = (-608),
};

extern OSErr C_AcceptHighLevelEvent(TargetID *sender_id_return, GUEST<int32_t> *refcon_return, Ptr msg_buf, GUEST<int32_t> *msg_length_return);
PASCAL_SUBTRAP(AcceptHighLevelEvent, 0xA88F, OSDispatch);
extern Boolean C_GetSpecificHighLevelEvent(GetSpecificFilterProcPtr fn, Ptr data, OSErr *err_return);
PASCAL_SUBTRAP(GetSpecificHighLevelEvent, 0xA88F, OSDispatch);
extern OSErr C_PostHighLevelEvent(EventRecord *evt, Ptr receiver_id, int32_t refcon, Ptr msg_buf, int32_t msg_length, int32_t post_options);
PASCAL_SUBTRAP(PostHighLevelEvent, 0xA88F, OSDispatch);

/* #### move to rsys/foo.h */
extern bool hle_get_event(EventRecord *evt, bool remflag);
extern void hle_init(void);
extern void hle_reinit(void);
extern void hle_reset(void);
}

#endif /* !_OSEVENT_H_ */
