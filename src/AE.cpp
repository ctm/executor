/* Copyright 1994, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_AE[] =
  "$Id: AE.c 63 2004-12-24 18:19:43Z ctm $";
#endif

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

namespace Executor {
boolean_t send_application_open_aevt_p;
  PUBLIC pascal OSErr C_EventHandlerTemplate(AppleEvent *evt, AppleEvent *reply,
											 int32 refcon);
  PUBLIC pascal OSErr C_CoercePtrTemplate(DescType data_type, Ptr data, Size data_size,
										  DescType to_type, int32 refcon, AEDesc * desc_out);
  
  PUBLIC pascal OSErr C_CoerceDescTemplate(AEDesc * desc, DescType to_type, int32 refcon,
										   AEDesc * desc_out);

}
using namespace Executor;


/* dispatching apple events */

P3 (PUBLIC pascal, OSErr, EventHandlerTemplate,
    AppleEvent *, evt, AppleEvent *, reply,
    int32, refcon)
{
  /* template for CTOP_... and PTOC_... flags generation */
  abort ();
#if !defined (LETGCCWAIL)
  return paramErr;
#endif
}

P1 (PUBLIC pascal trap, OSErr, AEProcessAppleEvent,
    EventRecord *, evtrec)
{
  GUEST<AEEventID> event_id_s;
  GUEST<AEEventClass> event_class_s;
  GUEST<DescType> dummy_type;
  GUEST<Size> dummy_size;
  GUEST<TargetID> dummy_target_id;
  
  AppleEvent *evt = (AppleEvent*)alloca (sizeof *evt);
  
  GUEST<EventHandlerProcPtr> hdlr_s;
  EventHandlerProcPtr hdlr;
  Handle evt_data;
  GUEST<Size> evt_data_size;
  GUEST<int32> refcon_s;
  GUEST<int32> dummy_refcon;
  
  OSErr err;
  OSErr retval;

  /* our current buffer is empty */
  evt_data_size = CLC(0);
  err = AcceptHighLevelEvent (&dummy_target_id,
			      &dummy_refcon,
			      nullptr, &evt_data_size);
  if (err != bufferIsSmall)
    AE_RETURN_ERROR (errAEEventNotHandled);
  
  evt_data = NewHandle (CL (evt_data_size));
  if (MemError () != noErr)
    AE_RETURN_ERROR (MemError ());
  
  LOCK_HANDLE_EXCURSION_1
    (evt_data,
     {
       err = AcceptHighLevelEvent (&dummy_target_id,
				   &dummy_refcon,
				   STARH (evt_data), &evt_data_size);
     });
  
  if (err != noErr)
    {
      DisposHandle (evt_data);
      AE_RETURN_ERROR (errAEEventNotHandled);
    }
  
  DESC_TYPE_X (evt) = CLC (typeAppleEvent);
  DESC_DATA_X (evt) = RM (evt_data);
  
  err = AEGetAttributePtr (evt, keyEventClassAttr,
			   typeType, &dummy_type,
			   (Ptr) &event_class_s, sizeof event_class_s,
			   &dummy_size);
  if (err != noErr)
    {
      AEDisposeDesc (evt);
      AE_RETURN_ERROR (err);
    }
  AEEventClass event_class = CL (event_class_s);
  
  err = AEGetAttributePtr (evt, keyEventIDAttr,
			   typeType, &dummy_type,
			   (Ptr) &event_id_s, sizeof event_id_s, &dummy_size);
  if (err != noErr)
    {
      AEDisposeDesc (evt);
      AE_RETURN_ERROR (err);
    }
  AEEventID event_id = CL (event_id_s);
  
  err = AEGetEventHandler (event_class, event_id, &hdlr_s, &refcon_s, FALSE);
  if (err != noErr)
    {
      err = AEGetEventHandler (event_class, event_id, &hdlr_s, &refcon_s, TRUE);
      if (err != noErr)
	{
	  AEDisposeDesc (evt);
	  AE_RETURN_ERROR (err);
	}
    }
  hdlr = MR (hdlr_s);
  int32 refcon = CL (refcon_s);
  
  {
    AppleEvent *reply = (AppleEvent*)alloca (sizeof *reply);
    AEDesc *target = (AEDesc*)alloca (sizeof *target);
    ProcessSerialNumber psn;
    
    GetCurrentProcess (&psn);
    
    err = AECreateDesc (typeProcessSerialNumber,
			(Ptr) &psn, sizeof psn, target);
    
    /* create a reply apple event.  currently unused */
    err = AECreateAppleEvent (kCoreEventClass, kAEAnswer,
			      target,
			      /* dummy */ -1, /* dummy */ -1,
			      reply);
    
    retval = CToPascalCall ((void*)hdlr,
			    CTOP_EventHandlerTemplate,
			    evt, reply, refcon);
    
    AEDisposeDesc (reply);
  }
  
  AEDisposeDesc (evt);
  AE_RETURN_ERROR (retval);
}

P7 (PUBLIC pascal trap, OSErr, AESend,
    AppleEvent *, evt, AppleEvent *, reply,
    AESendMode, send_mode, AESendPriority, send_priority,
    int32, timeout, IdleProcPtr, idle_proc, EventFilterProcPtr, filter_proc)
{
  AEDesc *target = (AEDesc*)alloca (sizeof *target);
  GUEST<DescType> target_type_s, dummy_type;
  GUEST<Size> target_size_s, dummy_size;
  
  ProcessSerialNumber *target_psn  = (ProcessSerialNumber *)alloca (sizeof *target_psn);
  ProcessSerialNumber *current_psn = (ProcessSerialNumber *)alloca (sizeof *current_psn);
  
  AEEventID event_id;
  AEEventClass event_class;
  
  OSErr err;
  
  GetCurrentProcess (current_psn);
  err = AEGetAttributePtr (evt, keyAddressAttr,
			   typeWildCard, &target_type_s,
			   (Ptr) target_psn, sizeof *target_psn, &target_size_s);
  if (err != noErr)
    /* ### not sure what error we should return here */
    AE_RETURN_ERROR (errAEEventNotHandled);
  
  DescType target_type = CL (target_type_s);
  Size target_size = CL (target_size_s);
  
  if (err != noErr)
    AE_RETURN_ERROR (err);
  
  err = AEGetAttributePtr (evt, keyEventClassAttr,
			   typeType, &dummy_type,
			   (Ptr) &event_class, sizeof event_class,
			   &dummy_size);
  if (err != noErr)
    AE_RETURN_ERROR (err);
  
  err = AEGetAttributePtr (evt, keyEventIDAttr,
			   typeType, &dummy_type,
			   (Ptr) &event_id, sizeof event_id, &dummy_size);
  if (err != noErr)
    AE_RETURN_ERROR (err);
  
  if (target_type == keyProcessSerialNumber
      && target_size == sizeof *target_psn)
    {
      Boolean same_process_p;
      
      err = SameProcess (target_psn, current_psn, &same_process_p);
      if (err != noErr)
	AE_RETURN_ERROR (errAEEventNotHandled);
      
      if (same_process_p)
	{
	  /* we are sending the AppleEvent to ourselves */
	  
	  if ((send_mode & _kAEReplyMask) == kAEWaitReply)
	    {
	      /* ### ignoring timeout/receipt requests */
	      warning_unimplemented ("target kCurrentProcess && kAEWaitReply");
	      AE_RETURN_ERROR (errAEEventNotHandled);
	    }
	  
	  if ((send_mode & _kAEReplyMask) == kAEQueueReply)
	    {
	      warning_unimplemented ("can't handle reply requests yet");
	      AE_RETURN_ERROR (errAEEventNotHandled);
	    }
	  
	  {
	    EventRecord evt_rec;
	    Handle desc_data;
	    int desc_data_size;
	    GUEST<Point> bogo_event_id;
	    
	    desc_data = DESC_DATA (evt);
	    desc_data_size = GetHandleSize (desc_data);
	    
	    evt_rec.what      = CWC (kHighLevelEvent);
	    evt_rec.message   = event_class;
	    
	    *(uint32 *) &bogo_event_id = event_id;
	    evt_rec.where     = bogo_event_id;
	    
	    LOCK_HANDLE_EXCURSION_1
	      (desc_data,
	       {
		 err = PostHighLevelEvent (&evt_rec,
					   /* #### i dunno */
					   NULL, -1,
					   STARH (desc_data), desc_data_size,
					   -1);
	       });
	    
	    AE_RETURN_ERROR (err);
	  }
	}
      else
	{
	  /* since there isn't any other processs this could be
             intended for, panic */
	  warning_unimplemented ("AE target not ProcessSerialNumber");
	  AE_RETURN_ERROR ((send_mode & _kAEReplyMask) == kAEWaitReply ?
			   errAEEventNotHandled : noErr);
	}
    }
  else
    {
      /* target is not a psn, we can't deal with anything else at this
	 time */
      if (target_type == typeApplSignature)
	{
	  warning_unimplemented ("returning invalidConnection");
	  AE_RETURN_ERROR (invalidConnection);
	}

      warning_unimplemented ("bad AE target, not ProcessSerialNumber");
      AE_RETURN_ERROR ((send_mode & _kAEReplyMask) == kAEWaitReply
		       ? errAEUnknownAddressType : noErr);
    }
}

P1 (PUBLIC pascal trap, OSErr, AESuspendTheCurrentEvent,
    AppleEvent *, evt)
{
  warning_unimplemented (NULL_STRING);
  AE_RETURN_ERROR (noErr);
}

P4 (PUBLIC pascal trap, OSErr, AEResumeTheCurrentEvent,
    AppleEvent *, evt, AppleEvent *, reply,
    EventHandlerProcPtr, dispatcher,
    int32, refcon)
{
  warning_unimplemented (NULL_STRING);
  AE_RETURN_ERROR (noErr);
}

P1 (PUBLIC pascal trap, OSErr, AEGetTheCurrentEvent,
    AppleEvent *, return_evt)
{
  warning_unimplemented (NULL_STRING);
  AE_RETURN_ERROR (noErr);
}

P1 (PUBLIC pascal trap, OSErr, AESetTheCurrentEvent,
    AppleEvent *, evt)
{
  warning_unimplemented (NULL_STRING);
  AE_RETURN_ERROR (noErr);
}

/* array spew */

P7 (PUBLIC pascal trap, OSErr, AEGetArray,
    AEDescList *, list,
    AEArrayType, array_type,
    AEArrayDataPointer, array_ptr, Size, max_size,
    GUEST<DescType> *, return_item_type,
    GUEST<Size> *, return_item_size, GUEST<int32> *, return_item_count)
{
  warning_unimplemented (NULL_STRING);
  AE_RETURN_ERROR (noErr);
}

P6 (PUBLIC pascal trap, OSErr, AEPutArray,
    AEDescList *, list, AEArrayType, type,
    AEArrayDataPointer, array_data, DescType, item_type,
    Size, item_size, int32, item_count)
{
  warning_unimplemented (NULL_STRING);
  AE_RETURN_ERROR (noErr);
}

/* user interaction settings */

AEInteractionAllowed interaction_level = kAEInteractWithSelf;

P1 (PUBLIC pascal trap, OSErr, AESetInteractionAllowed,
    AEInteractionAllowed, level)
{
  /* do nothing; since all levels are a superset of
     kAEInteractWithSelf, which is the only interaction currently
     available */
  
  interaction_level = level;
  AE_RETURN_ERROR (noErr);
}

P1 (PUBLIC pascal trap, OSErr, AEGetInteractionAllowed,
    AEInteractionAllowed *, level_out)
{
  *level_out = interaction_level;
  AE_RETURN_ERROR (noErr);
}

P3 (PUBLIC pascal trap, OSErr, AEInteractWithUser,
    int32, timeout, NMRecPtr, nm_req,
    IdleProcPtr, idle_proc)
{
  warning_unimplemented (NULL_STRING);
  AE_RETURN_ERROR (noErr);
}


P1 (PUBLIC pascal trap, OSErr, AEResetTimer,
    AppleEvent *, evt)
{
  warning_unimplemented (NULL_STRING);
  AE_RETURN_ERROR (noErr);
}

/* suspending and resuming AE handling */

/* type coersion */

P6 (PUBLIC pascal, OSErr, CoercePtrTemplate,
    DescType, data_type, Ptr, data, Size, data_size,
    DescType, to_type, int32, refcon, AEDesc *, desc_out)
{
  /* template for CTOP_... and PTOC_... flags generation */
  abort ();
#if !defined (LETGCCWAIL)
  return paramErr;
#endif
}

P4 (PUBLIC pascal, OSErr, CoerceDescTemplate,
    AEDesc *, desc, DescType, to_type, int32, refcon,
    AEDesc *, desc_out)
{
  /* template for CTOP_... and PTOC_... flags generation */
  abort ();
#if !defined (LETGCCWAIL)
  return paramErr;
#endif
}

P5 (PUBLIC pascal trap, OSErr, AECoercePtr,
    DescType, data_type,
    Ptr, data, Size, data_size,
    DescType, result_type, AEDesc *, desc_out)
{
  GUEST<ProcPtr> coercion_hdlr_s;
  GUEST<int32> refcon_s;
  Boolean is_desc_hdlr_p;
  OSErr err;
  
  if (result_type == typeWildCard
      || result_type == data_type)
    {
      AECreateDesc (data_type, data, data_size, desc_out);
      AE_RETURN_ERROR (noErr);
    }
  
  err = AEGetCoercionHandler (data_type, result_type,
			      &coercion_hdlr_s, &refcon_s,
			      &is_desc_hdlr_p,
			      FALSE);
  if (err != noErr)
    {
      err = AEGetCoercionHandler (data_type, result_type,
				  &coercion_hdlr_s, &refcon_s,
				  &is_desc_hdlr_p,
				  TRUE);
      if (err != noErr)
	AE_RETURN_ERROR (errAECoercionFail);
    }
  
  /* swap things to a normal state */
  ProcPtr coercion_hdlr = MR (coercion_hdlr_s);
  int32 refcon = CL (refcon_s);
  
  if (is_desc_hdlr_p)
    {
      descriptor_t *desc = (descriptor_t*)alloca (sizeof *desc);
      
      err = AECreateDesc (data_type, data, data_size, desc);
      if (err != noErr)
	return memFullErr;

      err = CToPascalCall ((void*)coercion_hdlr, PTOC_CoerceDescTemplate,
			   desc, result_type, refcon, desc_out);
      
      AEDisposeDesc (desc);
    }
  else
    {
      err = CToPascalCall ((void*)coercion_hdlr, PTOC_CoercePtrTemplate,
			   data_type, data, data_size, result_type,
			   refcon, desc_out);
    }
  
  if (err != noErr)
    AE_RETURN_ERROR (errAECoercionFail);
  
  AE_RETURN_ERROR (noErr);
}

#define AEGetParamDesc AEGetKeyDesc

PRIVATE OSErr
parse_evt (const AppleEvent *evtp, AEDesc *desc_out)
{
  OSErr retval;
  AEDesc d;

  /*
   * The cast to AppleEvent * of evtp is so the compiler won't complain
   * about our discarding const.  The problem is AEGetParamDesc doesn't the
   * thing pointed to by its first argument, but the prototype doesn't show
   * this.
   */

  retval = AEGetParamDesc ((AppleEvent *) evtp,
			   keyDirectObject, typeAEList, &d);
  if (retval == noErr)
    {
      LONGINT n;
      GUEST<LONGINT> n_s;

      retval = AECountItems (&d, &n_s);
      n = CL (n_s);
      if (retval == noErr)
	{
	  Handle h;
	  AppParametersPtr p;

	  h = NewHandle (sizeof *p + n * sizeof (FSSpec));
	  if (!h || !(p = (typeof (p)) STARH (h)))
	    retval = MemError();
	  else
	    {
	      LONGINT l;

	      p->magic = CLC (APP_PARAMS_MAGIC);
	      p->n_fsspec = CW (n);
	      for (l = 1; retval == noErr && l <= n; ++l)
		{
		  AEDesc d2;
		  GUEST<AEKeyword> keyword;

		  retval = AEGetNthDesc (&d, 1L, typeAlias, &keyword, &d2);
		  if (retval == noErr)
		    {
		      AliasHandle ah;
		      Boolean wasChanged;

		      ah = (AliasHandle) MR (d2.dataHandle);
		      retval = ResolveAlias (NULL, ah, &p->fsspec[l-1],
					     &wasChanged);
		    }
		}
	      if (retval != noErr)
		DisposHandle (h);
	      else
		{
		  desc_out->descriptorType = CLC(TICK("appa"));
		  desc_out->dataHandle = RM (h);
		}
	    }
	}
    }
  return retval;
}

P3 (PUBLIC pascal trap, OSErr, AECoerceDesc,
    AEDesc *, desc, DescType, result_type, AEDesc *, desc_out)
{
  GUEST<ProcPtr> coercion_hdlr_s;
  GUEST<int32> refcon_s;
  ProcPtr coercion_hdlr;
  int32 refcon;
  GUEST<Boolean> is_desc_hdlr_p;
  DescType desc_type;
  OSErr err;
  
  desc_type = DESC_TYPE (desc);

  if (desc_type == TICK("aevt") && result_type == TICK("appa"))
    {
      err = parse_evt (desc, desc_out);
      AE_RETURN_ERROR (err);
    }
  
  if (result_type == typeWildCard
      || result_type == desc_type)
    {
      AEDuplicateDesc (desc, desc_out);
      AE_RETURN_ERROR (noErr);
    }
  
  err = AEGetCoercionHandler (desc_type, result_type,
			      &coercion_hdlr_s, &refcon_s,
			      &is_desc_hdlr_p,
			      FALSE);
  if (err != noErr)
    {
      err = AEGetCoercionHandler (desc_type, result_type,
				  &coercion_hdlr_s, &refcon_s,
				  &is_desc_hdlr_p,
				  TRUE);
      if (err != noErr)
	goto fail;
    }
  
  /* swap things to a normal state */
  coercion_hdlr = MR (coercion_hdlr_s);
  refcon = CL (refcon_s);

  if (is_desc_hdlr_p)
    {
      err = CToPascalCall((void*)coercion_hdlr, PTOC_CoerceDescTemplate,
			   desc, result_type, refcon, desc_out);
    }
  else
    {
      Handle desc_data;
      
      desc_data = DESC_DATA (desc);
      
      LOCK_HANDLE_EXCURSION_1
	(desc_data,
	 {
	   err = CToPascalCall((void*)coercion_hdlr, PTOC_CoercePtrTemplate,
				desc_type, STARH (desc_data),
				GetHandleSize (desc_data),
				result_type, refcon, desc_out);
	 });
    }
  
  if (err != noErr)
    goto fail;
  
  AE_RETURN_ERROR (noErr);

 fail:
  desc_out->descriptorType = CLC (typeNull);
  desc_out->dataHandle = nullptr;
  AE_RETURN_ERROR (errAECoercionFail);
}

P1 (PUBLIC pascal trap, OSErr, AEManagerInfo, GUEST<LONGINT> *, resultp)
{
  OSErr retval;

  *resultp = CL(0);
  retval = noErr;

  return retval;
}

/* stubs added by Cliff after reading documentation on Apple's web site */

P1 (PUBLIC pascal trap, OSErr, AEDisposeToken,
    AEDesc *, theToken)
{
  warning_unimplemented (NULL_STRING);
  return noErr;
}

P3 (PUBLIC pascal trap, OSErr, AEREesolve,
    AEDesc *, objectSpecifier,
    INTEGER, callbackFlags,
    AEDesc *, theToken)
{
  warning_unimplemented (NULL_STRING);
  return noErr;
}

P4 (PUBLIC pascal trap, OSErr, AERemoveObjectAccessor,
    DescType, desiredClass,
    DescType, containerType,
    ProcPtr, theAccessor,
    BOOLEAN, isSysHandler)
{
  warning_unimplemented (NULL_STRING);
  return noErr;
}

P5 (PUBLIC pascal trap, OSErr, AEInstallObjectAccessor,
    DescType, desiredClass,
    DescType, containerType,
    ProcPtr, theAccessor,
    LONGINT, refcon,
    BOOLEAN, isSysHandler)
{
  warning_unimplemented (NULL_STRING);
  return noErr;
}

P5 (PUBLIC pascal trap, OSErr, AEGetObjectAccessor,
    DescType, desiredClass,
    DescType, containerType,
    ProcPtr *, theAccessor,
    LONGINT *, accessorRefcon,
    BOOLEAN, isSysHandler)
{
  warning_unimplemented (NULL_STRING);
  return noErr;
}

P6 (PUBLIC pascal trap, OSErr, AECallObjectAccessor,
    DescType, desiredClass,
    AEDesc *, containerToken,
    DescType, containerClass,
    DescType, keyForm,
    AEDesc *, keyData,
    AEDesc *, theToken)
{
  warning_unimplemented (NULL_STRING);
  return noErr;
}

P7 (PUBLIC pascal trap, OSErr, AESetObjectCallbacks,
    ProcPtr, myCompareProc,
    ProcPtr, myCountProc,
    ProcPtr, myDisposeTokenProc,
    ProcPtr, myGetMarkTokenProc,
    ProcPtr, myMarkProc,
    ProcPtr, myAdjustMarksProc,
    ProcPtr, myGetErrDescProc)
{
  warning_unimplemented (NULL_STRING);
  return noErr;
}

