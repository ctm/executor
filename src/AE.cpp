/* Copyright 1994, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* AppleEvents implementation */

#include "rsys/common.h"

#include "EventMgr.h"
#include "NotifyMgr.h"
#include "AppleEvents.h"
#include "MemoryMgr.h"
#include "ProcessMgr.h"
#include "OSEvent.h"
#include "OSUtil.h"
#include "ProcessMgr.h"
#include "AliasMgr.h"

#include "rsys/mman.h"
#include "rsys/flags.h"
#include "rsys/functions.impl.h"

namespace Executor
{
bool send_application_open_aevt_p;
}
using namespace Executor;

/* dispatching apple events */

OSErr Executor::C_AEProcessAppleEvent(EventRecord *evtrec)
{
    GUEST<AEEventID> event_id_s;
    GUEST<AEEventClass> event_class_s;
    GUEST<DescType> dummy_type;
    GUEST<Size> dummy_size;
    GUEST<TargetID> dummy_target_id;

    AppleEvent *evt = (AppleEvent *)alloca(sizeof *evt);

    GUEST<EventHandlerProcPtr> hdlr_s;
    EventHandlerProcPtr hdlr;
    Handle evt_data;
    GUEST<Size> evt_data_size;
    GUEST<int32_t> refcon_s;
    GUEST<int32_t> dummy_refcon;

    OSErr err;
    OSErr retval;

    /* our current buffer is empty */
    evt_data_size = CLC(0);
    err = AcceptHighLevelEvent(&dummy_target_id,
                               &dummy_refcon,
                               nullptr, &evt_data_size);
    if(err != bufferIsSmall)
        AE_RETURN_ERROR(errAEEventNotHandled);

    evt_data = NewHandle(CL(evt_data_size));
    if(MemError() != noErr)
        AE_RETURN_ERROR(MemError());

    {
        HLockGuard guard(evt_data);
        err = AcceptHighLevelEvent(&dummy_target_id,
                                   &dummy_refcon,
                                   STARH(evt_data), &evt_data_size);
    }

    if(err != noErr)
    {
        DisposHandle(evt_data);
        AE_RETURN_ERROR(errAEEventNotHandled);
    }

    DESC_TYPE_X(evt) = CLC(typeAppleEvent);
    DESC_DATA_X(evt) = RM(evt_data);

    err = AEGetAttributePtr(evt, keyEventClassAttr,
                            typeType, &dummy_type,
                            (Ptr)&event_class_s, sizeof event_class_s,
                            &dummy_size);
    if(err != noErr)
    {
        AEDisposeDesc(evt);
        AE_RETURN_ERROR(err);
    }
    AEEventClass event_class = CL(event_class_s);

    err = AEGetAttributePtr(evt, keyEventIDAttr,
                            typeType, &dummy_type,
                            (Ptr)&event_id_s, sizeof event_id_s, &dummy_size);
    if(err != noErr)
    {
        AEDisposeDesc(evt);
        AE_RETURN_ERROR(err);
    }
    AEEventID event_id = CL(event_id_s);

    err = AEGetEventHandler(event_class, event_id, &hdlr_s, &refcon_s, false);
    if(err != noErr)
    {
        err = AEGetEventHandler(event_class, event_id, &hdlr_s, &refcon_s, true);
        if(err != noErr)
        {
            AEDisposeDesc(evt);
            AE_RETURN_ERROR(err);
        }
    }
    hdlr = MR(hdlr_s);
    int32_t refcon = CL(refcon_s);

    {
        AppleEvent *reply = (AppleEvent *)alloca(sizeof *reply);
        AEDesc *target = (AEDesc *)alloca(sizeof *target);
        ProcessSerialNumber psn;

        GetCurrentProcess(&psn);

        err = AECreateDesc(typeProcessSerialNumber,
                           (Ptr)&psn, sizeof psn, target);

        /* create a reply apple event.  currently unused */
        err = AECreateAppleEvent(kCoreEventClass, kAEAnswer,
                                 target,
                                 /* dummy */ -1, /* dummy */ -1,
                                 reply);

        retval = hdlr(evt, reply, refcon);
        AEDisposeDesc(reply);
    }

    AEDisposeDesc(evt);
    AE_RETURN_ERROR(retval);
}

OSErr Executor::C_AESend(AppleEvent *evt, AppleEvent *reply,
                         AESendMode send_mode, AESendPriority send_priority,
                         int32_t timeout, IdleProcPtr idle_proc,
                         EventFilterProcPtr filter_proc)
{
    AEDesc *target = (AEDesc *)alloca(sizeof *target);
    GUEST<DescType> target_type_s, dummy_type;
    GUEST<Size> target_size_s, dummy_size;

    ProcessSerialNumber *target_psn = (ProcessSerialNumber *)alloca(sizeof *target_psn);
    ProcessSerialNumber *current_psn = (ProcessSerialNumber *)alloca(sizeof *current_psn);

    AEEventID event_id;
    GUEST<AEEventClass> event_class;

    OSErr err;

    GetCurrentProcess(current_psn);
    err = AEGetAttributePtr(evt, keyAddressAttr,
                            typeWildCard, &target_type_s,
                            (Ptr)target_psn, sizeof *target_psn, &target_size_s);
    if(err != noErr)
        /* ### not sure what error we should return here */
        AE_RETURN_ERROR(errAEEventNotHandled);

    DescType target_type = CL(target_type_s);
    Size target_size = CL(target_size_s);

    if(err != noErr)
        AE_RETURN_ERROR(err);

    err = AEGetAttributePtr(evt, keyEventClassAttr,
                            typeType, &dummy_type,
                            (Ptr)&event_class, sizeof event_class,
                            &dummy_size);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    err = AEGetAttributePtr(evt, keyEventIDAttr,
                            typeType, &dummy_type,
                            (Ptr)&event_id, sizeof event_id, &dummy_size);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    if(target_type == keyProcessSerialNumber
       && target_size == sizeof *target_psn)
    {
        Boolean same_process_p;

        err = SameProcess(target_psn, current_psn, &same_process_p);
        if(err != noErr)
            AE_RETURN_ERROR(errAEEventNotHandled);

        if(same_process_p)
        {
            /* we are sending the AppleEvent to ourselves */

            if((send_mode & _kAEReplyMask) == kAEWaitReply)
            {
                /* ### ignoring timeout/receipt requests */
                warning_unimplemented("target kCurrentProcess && kAEWaitReply");
                AE_RETURN_ERROR(errAEEventNotHandled);
            }

            if((send_mode & _kAEReplyMask) == kAEQueueReply)
            {
                warning_unimplemented("can't handle reply requests yet");
                AE_RETURN_ERROR(errAEEventNotHandled);
            }

            {
                EventRecord evt_rec;
                Handle desc_data;
                int desc_data_size;
                GUEST<Point> bogo_event_id;

                desc_data = DESC_DATA(evt);
                desc_data_size = GetHandleSize(desc_data);

                evt_rec.what = CWC(kHighLevelEvent);
                evt_rec.message = event_class;

                *(uint32_t *)&bogo_event_id = event_id;
                evt_rec.where = bogo_event_id;

                {
                    HLockGuard guard(desc_data);
                    err = PostHighLevelEvent(&evt_rec,
                                             /* #### i dunno */
                                             NULL, -1,
                                             STARH(desc_data), desc_data_size,
                                             -1);
                }

                AE_RETURN_ERROR(err);
            }
        }
        else
        {
            /* since there isn't any other processs this could be
             intended for, panic */
            warning_unimplemented("AE target not ProcessSerialNumber");
            AE_RETURN_ERROR((send_mode & _kAEReplyMask) == kAEWaitReply ? errAEEventNotHandled : noErr);
        }
    }
    else
    {
        /* target is not a psn, we can't deal with anything else at this
	 time */
        if(target_type == typeApplSignature)
        {
            warning_unimplemented("returning invalidConnection");
            AE_RETURN_ERROR(invalidConnection);
        }

        warning_unimplemented("bad AE target, not ProcessSerialNumber");
        AE_RETURN_ERROR((send_mode & _kAEReplyMask) == kAEWaitReply
                            ? errAEUnknownAddressType
                            : noErr);
    }
}

OSErr Executor::C_AESuspendTheCurrentEvent(AppleEvent *evt)
{
    warning_unimplemented(NULL_STRING);
    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AEResumeTheCurrentEvent(
    AppleEvent *evt, AppleEvent *reply, EventHandlerProcPtr dispatcher,
    int32_t refcon)
{
    warning_unimplemented(NULL_STRING);
    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AEGetTheCurrentEvent(AppleEvent *return_evt)
{
    warning_unimplemented(NULL_STRING);
    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AESetTheCurrentEvent(AppleEvent *evt)
{
    warning_unimplemented(NULL_STRING);
    AE_RETURN_ERROR(noErr);
}

/* array spew */

OSErr Executor::C_AEGetArray(AEDescList *list, AEArrayType array_type,
                             AEArrayDataPointer array_ptr, Size max_size,
                             GUEST<DescType> *return_item_type,
                             GUEST<Size> *return_item_size,
                             GUEST<int32_t> *return_item_count)
{
    warning_unimplemented(NULL_STRING);
    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AEPutArray(AEDescList *list, AEArrayType type,
                             AEArrayDataPointer array_data, DescType item_type,
                             Size item_size, int32_t item_count)
{
    warning_unimplemented(NULL_STRING);
    AE_RETURN_ERROR(noErr);
}

/* user interaction settings */

AEInteractionAllowed interaction_level = kAEInteractWithSelf;

OSErr Executor::C_AESetInteractionAllowed(AEInteractionAllowed level)
{
    /* do nothing; since all levels are a superset of
     kAEInteractWithSelf, which is the only interaction currently
     available */

    interaction_level = level;
    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AEGetInteractionAllowed(AEInteractionAllowed *level_out)
{
    *level_out = interaction_level;
    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AEInteractWithUser(int32_t timeout, NMRecPtr nm_req,
                                     IdleProcPtr idle_proc)
{
    warning_unimplemented(NULL_STRING);
    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AEResetTimer(AppleEvent *evt)
{
    warning_unimplemented(NULL_STRING);
    AE_RETURN_ERROR(noErr);
}

/* suspending and resuming AE handling */

/* type coersion */

OSErr Executor::C_AECoercePtr(DescType data_type, Ptr data, Size data_size,
                              DescType result_type, AEDesc *desc_out)
{
    GUEST<CoerceDescProcPtr> coercion_hdlr_s;
    GUEST<int32_t> refcon_s;
    Boolean is_desc_hdlr_p;
    OSErr err;

    if(result_type == typeWildCard
       || result_type == data_type)
    {
        AECreateDesc(data_type, data, data_size, desc_out);
        AE_RETURN_ERROR(noErr);
    }

    err = AEGetCoercionHandler(data_type, result_type,
                               &coercion_hdlr_s, &refcon_s,
                               &is_desc_hdlr_p,
                               false);
    if(err != noErr)
    {
        err = AEGetCoercionHandler(data_type, result_type,
                                   &coercion_hdlr_s, &refcon_s,
                                   &is_desc_hdlr_p,
                                   true);
        if(err != noErr)
            AE_RETURN_ERROR(errAECoercionFail);
    }

    /* swap things to a normal state */
    CoerceDescProcPtr coercion_hdlr = MR(coercion_hdlr_s);
    int32_t refcon = CL(refcon_s);

    if(is_desc_hdlr_p)
    {
        descriptor_t *desc = (descriptor_t *)alloca(sizeof *desc);

        err = AECreateDesc(data_type, data, data_size, desc);
        if(err != noErr)
            return memFullErr;

        err = coercion_hdlr(desc, result_type, refcon, desc_out);

        AEDisposeDesc(desc);
    }
    else
    {
        CoercePtrProcPtr ptr_coercion_hdlr((void *)coercion_hdlr);
        err = ptr_coercion_hdlr(data_type, data, data_size, result_type,
                                refcon, desc_out);
    }

    if(err != noErr)
        AE_RETURN_ERROR(errAECoercionFail);

    AE_RETURN_ERROR(noErr);
}

#define AEGetParamDesc AEGetKeyDesc

static OSErr
parse_evt(const AppleEvent *evtp, AEDesc *desc_out)
{
    OSErr retval;
    AEDesc d;

    /*
   * The cast to AppleEvent * of evtp is so the compiler won't complain
   * about our discarding const.  The problem is AEGetParamDesc doesn't the
   * thing pointed to by its first argument, but the prototype doesn't show
   * this.
   */

    retval = AEGetParamDesc((AppleEvent *)evtp,
                            keyDirectObject, typeAEList, &d);
    if(retval == noErr)
    {
        LONGINT n;
        GUEST<LONGINT> n_s;

        retval = AECountItems(&d, &n_s);
        n = CL(n_s);
        if(retval == noErr)
        {
            Handle h;
            AppParametersPtr p;

            h = NewHandle(sizeof *p + n * sizeof(FSSpec));
            if(!h || !(p = (decltype(p))STARH(h)))
                retval = MemError();
            else
            {
                LONGINT l;

                p->magic = CLC(APP_PARAMS_MAGIC);
                p->n_fsspec = CW(n);
                for(l = 1; retval == noErr && l <= n; ++l)
                {
                    AEDesc d2;
                    GUEST<AEKeyword> keyword;

                    retval = AEGetNthDesc(&d, 1L, typeAlias, &keyword, &d2);
                    if(retval == noErr)
                    {
                        AliasHandle ah;
                        Boolean wasChanged;

                        ah = (AliasHandle)MR(d2.dataHandle);
                        retval = ResolveAlias(NULL, ah, &p->fsspec[l - 1],
                                              &wasChanged);
                    }
                }
                if(retval != noErr)
                    DisposHandle(h);
                else
                {
                    desc_out->descriptorType = CLC(TICK("appa"));
                    desc_out->dataHandle = RM(h);
                }
            }
        }
    }
    return retval;
}

OSErr Executor::C_AECoerceDesc(AEDesc *desc, DescType result_type,
                               AEDesc *desc_out)
{
    GUEST<CoerceDescProcPtr> coercion_hdlr_s;
    GUEST<int32_t> refcon_s;
    CoerceDescProcPtr coercion_hdlr;
    int32_t refcon;
    GUEST<Boolean> is_desc_hdlr_p;
    DescType desc_type;
    OSErr err;

    desc_type = DESC_TYPE(desc);

    if(desc_type == TICK("aevt") && result_type == TICK("appa"))
    {
        err = parse_evt(desc, desc_out);
        AE_RETURN_ERROR(err);
    }

    if(result_type == typeWildCard
       || result_type == desc_type)
    {
        AEDuplicateDesc(desc, desc_out);
        AE_RETURN_ERROR(noErr);
    }

    err = AEGetCoercionHandler(desc_type, result_type,
                               &coercion_hdlr_s, &refcon_s,
                               &is_desc_hdlr_p,
                               false);
    if(err != noErr)
    {
        err = AEGetCoercionHandler(desc_type, result_type,
                                   &coercion_hdlr_s, &refcon_s,
                                   &is_desc_hdlr_p,
                                   true);
        if(err != noErr)
            goto fail;
    }

    /* swap things to a normal state */
    coercion_hdlr = MR(coercion_hdlr_s);
    refcon = CL(refcon_s);

    if(is_desc_hdlr_p)
    {
        err = coercion_hdlr(desc, result_type, refcon, desc_out);
    }
    else
    {
        Handle desc_data;

        desc_data = DESC_DATA(desc);

        {
            HLockGuard guard(desc_data);
            CoercePtrProcPtr ptr_coercion_hdlr((void *)coercion_hdlr);
            err = ptr_coercion_hdlr(desc_type, STARH(desc_data),
                                    GetHandleSize(desc_data),
                                    result_type, refcon, desc_out);
        }
    }

    if(err != noErr)
        goto fail;

    AE_RETURN_ERROR(noErr);

fail:
    desc_out->descriptorType = CLC(typeNull);
    desc_out->dataHandle = nullptr;
    AE_RETURN_ERROR(errAECoercionFail);
}

OSErr Executor::C_AEManagerInfo(GUEST<LONGINT> *resultp)
{
    OSErr retval;

    *resultp = CL(0);
    retval = noErr;

    return retval;
}

/* stubs added by Cliff after reading documentation on Apple's web site */

OSErr Executor::C_AEDisposeToken(AEDesc *theToken)
{
    warning_unimplemented(NULL_STRING);
    return noErr;
}

OSErr Executor::C_AEResolve(AEDesc *objectSpecifier, INTEGER callbackFlags,
                             AEDesc *theToken)
{
    warning_unimplemented(NULL_STRING);
    return noErr;
}

OSErr Executor::C_AERemoveObjectAccessor(
    DescType desiredClass, DescType containerType, ProcPtr theAccessor,
    BOOLEAN isSysHandler)
{
    warning_unimplemented(NULL_STRING);
    return noErr;
}

OSErr Executor::C_AEInstallObjectAccessor(
    DescType desiredClass, DescType containerType, ProcPtr theAccessor,
    LONGINT refcon, BOOLEAN isSysHandler)
{
    warning_unimplemented(NULL_STRING);
    return noErr;
}

OSErr Executor::C_AEGetObjectAccessor(DescType desiredClass,
                                      DescType containerType,
                                      ProcPtr *theAccessor,
                                      LONGINT *accessorRefcon,
                                      BOOLEAN isSysHandler)
{
    warning_unimplemented(NULL_STRING);
    return noErr;
}

OSErr Executor::C_AECallObjectAccessor(DescType desiredClass,
                                       AEDesc *containerToken,
                                       DescType containerClass,
                                       DescType keyForm, AEDesc *keyData,
                                       AEDesc *theToken)
{
    warning_unimplemented(NULL_STRING);
    return noErr;
}

OSErr Executor::C_AESetObjectCallbacks(ProcPtr myCompareProc,
                                       ProcPtr myCountProc,
                                       ProcPtr myDisposeTokenProc,
                                       ProcPtr myGetMarkTokenProc,
                                       ProcPtr myMarkProc,
                                       ProcPtr myAdjustMarksProc,
                                       ProcPtr myGetErrDescProc)
{
    warning_unimplemented(NULL_STRING);
    return noErr;
}
