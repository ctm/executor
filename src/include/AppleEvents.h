#if !defined(_AppleEvents_H_)
#define _AppleEvents_H_

/*
 * Copyright 1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: AppleEvents.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "EventMgr.h"
#include "NotifyMgr.h"

namespace Executor {
typedef int32 AEEventClass;
typedef int32 AEEventID;
typedef int32 AEKeyword;
typedef ResType DescType;

typedef struct AEDesc
{
  GUEST_STRUCT;
  GUEST<DescType> descriptorType;
  GUEST<Handle> dataHandle;
} AEDesc;

/* ### hack, delete */
typedef AEDesc descriptor_t;

#define DESC_TYPE_X(desc)		((desc)->descriptorType)
#define DESC_DATA_X(desc)		((desc)->dataHandle)

#define DESC_TYPE(desc)			(CL (DESC_TYPE_X (desc)))
#define DESC_DATA(desc)			(MR (DESC_DATA_X (desc)))

typedef struct AEKeyDesc
{
  GUEST<AEKeyword> descKey;
  GUEST<AEDesc> descContent;
} AEKeyDesc;

typedef AEKeyDesc key_desc_t;

#define KEY_DESC_KEYWORD_X(keydesc)	((keydesc)->descKey)

#define KEY_DESC_KEYWORD(keydesc)	(CL (KEY_DESC_KEYWORD_X (keydesc)))
#define KEY_DESC_CONTENT(keydesc)	((keydesc)->descContent)

typedef AEDesc AEAddressDesc;
typedef AEDesc AEDescList;
typedef AEDescList AERecord;
typedef AERecord AppleEvent;

typedef int32 AESendMode;

#define _kAEReplyMask		(0x3)

#define kAENoReply		(0x1)
#define kAEQueueReply		(0x2)
#define kAEWaitReply		(0x3)

#define _kAEInteractMask	(0x30)

#define kAENeverInteract	(0x10)
#define kAECanInteract		(0x20)
#define kAEAlwaysInteract	(0x30)

#define kAECanSwitchLayer	(0x40)
#define kAEDontReconnect	(0x80)
/* #define kAEWantReceipt	??? */

typedef int16 AESendPriority;

typedef uint8 AEEventSource;
typedef uint8 AEInteractionAllowed;
typedef uint8 AEArrayType;

enum
{
  kAEInteractWithSelf = 0,
  kAEInteractWithLocal = 1,
  kAEInteractWithAll = 2,
};

enum
{
  kAEUnknownSource = 0,
  kAEDirectCall = 1,
  kAESameProcess = 2,
  kAELocalProcess = 3,
  kAERemoteProcess = 4,
};

enum
{
  kAEDataArray, kAEPackedArray, kAEHandleArray, kAEDescArray,
  kAEKeyDescArray
};

typedef union AEArrayData
{
  GUEST<int16> AEDataArray[1];
  int8 AEPackedArray[1];
  GUEST<Handle> AEHandleArray[1];
  AEDesc AEDescArray[1];
  AEKeyDesc AEKeyDescArray[1];
} AEArrayData;

typedef AEArrayData *AEArrayDataPointer;

typedef ProcPtr EventHandlerProcPtr;
typedef ProcPtr IdleProcPtr;
typedef ProcPtr EventFilterProcPtr;

/* #### internal */

typedef struct AE_hdlr { GUEST_STRUCT;
    GUEST< ProcPtr> fn;
    GUEST< int32> refcon;
} AE_hdlr_t;

typedef struct AE_hdlr_selector { GUEST_STRUCT;
    GUEST< int32> sel0;
    GUEST< int32> sel1;
} AE_hdlr_selector_t;



typedef struct AE_hdlr_table_elt { GUEST_STRUCT;
    GUEST< int32> pad_1;
    GUEST< AE_hdlr_selector_t> selector;
    GUEST< AE_hdlr_t> hdlr;
    GUEST< int32> pad_2;
} AE_hdlr_table_elt_t;

#define AE_TABLE_ELTS(table)			(HxX (table, elts))

#define AE_TABLE_N_ELTS_X(table)		(HxX (table, n_elts))
#define AE_TABLE_N_ALLOCATED_BYTES_X(table)	\
  (HxX (table, n_allocated_bytes))

#define AE_TABLE_N_ELTS(table)			\
  (CL (AE_TABLE_N_ELTS_X (table)))
#define AE_TABLE_N_ALLOCATED_BYTES(table)	\
  (CL (AE_TABLE_N_ALLOCATED_BYTES_X (table)))




typedef struct AE_hdlr_table { GUEST_STRUCT;
    GUEST< int32> pad_1;
    GUEST< int32> n_allocated_bytes;
    GUEST< int32> n_elts;
    GUEST< int32[10]> pad_2;
    GUEST< AE_hdlr_table_elt_t[0]> elts;
} AE_hdlr_table_t;

typedef AE_hdlr_table_t *AE_hdlr_table_ptr;

typedef GUEST<AE_hdlr_table_ptr> *AE_hdlr_table_h;





    /* points to a 32byte handle of unknown contents (at least,
     sometimes) */
typedef struct AE_zone_tables { GUEST_STRUCT;
    GUEST< AE_hdlr_table_h> event_hdlr_table;
    GUEST< AE_hdlr_table_h> coercion_hdlr_table;
    GUEST< AE_hdlr_table_h> special_hdlr_table;
    GUEST< char[28]> pad_1;
    GUEST< char[4]> unknown_appl_value;
    GUEST< char[8]> pad_2;
    GUEST< Handle> unknown_sys_handle;
} AE_zone_tables_t;

typedef AE_zone_tables_t *AE_zone_tables_ptr;

typedef GUEST<AE_zone_tables_ptr> *AE_zone_tables_h;


    /* offset of `appl_zone_tables' is 340; handle to a `struct tables' */


    /* offset of `system_zone_tables' is 380; handle to a `struct tables' */

typedef struct { GUEST_STRUCT;
    GUEST< char[340]> pad_1;
    GUEST< AE_zone_tables_h> appl_zone_tables;
    GUEST< char[36]> pad_2;
    GUEST< AE_zone_tables_h> system_zone_tables;
    GUEST< char[212]> pad_3;
} AE_info_t;

typedef AE_info_t *AE_info_ptr;


extern pascal trap OSErr C__AE_hdlr_table_alloc (int32, int32, int32, int8,
						 GUEST<AE_hdlr_table_h> *);
extern pascal trap OSErr C__AE_hdlr_delete (AE_hdlr_table_h, int32,
					    AE_hdlr_selector_t *);
extern pascal trap OSErr C__AE_hdlr_lookup (AE_hdlr_table_h, int32,
					    AE_hdlr_selector_t *,
					    AE_hdlr_t *);
extern pascal trap OSErr C__AE_hdlr_install (AE_hdlr_table_h, int32,
					     AE_hdlr_selector_t *,
					     AE_hdlr_t *);

/* private */

extern bool send_application_open_aevt_p;
extern bool application_accepts_open_app_aevt_p;

/*  error codes */

#define AE_RETURN_ERROR(error)						      \
  ({									      \
    OSErr _error_ = (error);						      \
									      \
    if (_error_ != noErr)						      \
      warning_unexpected ("error `%d'", _error_);			      \
    return _error_;							      \
  })

#define paramErr		(-50)

#define invalidConnection (-609)

#define errAECoercionFail	(-1700)
#define errAEDescNotFound	(-1701)
#define errAEWrongDataType	(-1703)
#define errAENotAEDesc		(-1704)

#define errAEEventNotHandled	(-1708)
#define errAEUnknownAddressType	(-1716)

#define errAEHandlerNotFound	(-1717)
#define errAEIllegalIndex	(-1719)


/* types */

#define typeFSS			(T ('f', 's', 's', ' '))

#define typeAEList		(T ('l', 'i', 's', 't'))
#define typeAERecord		(T ('r', 'e', 'c', 'o'))
#define typeAppleEvent		(T ('a', 'e', 'v', 't'))
#define typeProcessSerialNumber	(T ('p', 's', 'n', ' '))
#define typeNull		(T ('n', 'u', 'l', 'l'))
#define typeApplSignature      (T ('s', 'i', 'g', 'n'))

#define typeType		(T ('t', 'y', 'p', 'e'))
#define typeWildCard		(T ('*', '*', '*', '*'))
#define typeAlias		(T ('a', 'l', 'i', 's'))

#define keyAddressAttr		(T ('a', 'd', 'd', 'r'))
#define keyEventClassAttr	(T ('e', 'v', 'c', 'l'))
#define keyEventIDAttr		(T ('e', 'v', 'i', 'd'))
#define keyProcessSerialNumber	(T ('p', 's', 'n', ' '))

#define keyDirectObject		(T ('-', '-', '-', '-'))

#define kCoreEventClass		(T ('a', 'e', 'v', 't'))
#define kAEOpenApplication	(T ('o', 'a', 'p', 'p'))
#define kAEOpenDocuments	(T ('o', 'd', 'o', 'c'))
#define kAEPrintDocuments	(T ('p', 'd', 'o', 'c'))
#define kAEAnswer		(T ('a', 'n', 's', 'r'))
#define kAEQuitApplication	(T ('q', 'u', 'i', 't'))

#define keySelectProc		(T ('s', 'e', 'l', 'h'))

/* #### OSL internal */
extern syn68k_addr_t/*ProcPtr*/ AE_OSL_select_fn;

/* prototypes go here */

extern pascal trap OSErr C_AEGetCoercionHandler
  (DescType from_type, DescType to_type,
   GUEST<ProcPtr> *hdlr_out, GUEST<int32> *refcon_out,
   GUEST<Boolean> *from_type_is_desc_p_out, Boolean system_handler_p);

extern pascal trap OSErr C_AECreateDesc (DescType type,
					 Ptr data, Size data_size,
					 AEDesc *desc_out);
extern pascal trap OSErr C_AEDisposeDesc (AEDesc *desc);

extern pascal trap OSErr C_AECreateDesc (DescType type,
					 Ptr data, Size data_size,
					 AEDesc *desc_out);

extern pascal trap OSErr C_AECoerceDesc (AEDesc *desc, DescType result_type,
					 AEDesc *desc_out);

extern pascal trap OSErr C_AEGetKeyPtr (AERecord *record, AEKeyword keyword,
					DescType desired_type, GUEST<DescType> *type_out,
					Ptr data, Size max_size, GUEST<Size> *size_out);

extern pascal trap OSErr C_AEGetKeyDesc (AERecord *record, AEKeyword keyword,
					 DescType desired_type, AEDesc *desc_out);

extern pascal trap OSErr C_AEPutKeyPtr (AERecord *record, AEKeyword keyword,
					DescType type, Ptr data, Size data_size);
extern pascal trap OSErr C_AEPutKeyDesc (AERecord *record, AEKeyword keyword,
					 AEDesc *desc);

extern pascal trap OSErr C_AEDeleteParam (AppleEvent *evt, AEKeyword keyword);

extern pascal trap OSErr C_AEDeleteAttribute (AppleEvent *evt,
					      AEKeyword keyword);

extern pascal trap OSErr C_AESizeOfKeyDesc (AERecord *record, AEKeyword keyword,
					    GUEST<DescType> *type_out, GUEST<Size> *size_out);

extern pascal trap OSErr C_AESetInteractionAllowed (AEInteractionAllowed level);

extern pascal trap OSErr C_AEResetTimer (AppleEvent *evt);

extern pascal trap OSErr C_AEGetTheCurrentEvent (AppleEvent *return_evt);
extern pascal trap OSErr C_AESetTheCurrentEvent (AppleEvent *evt);
extern pascal trap OSErr C_AESuspendTheCurrentEvent (AppleEvent *evt);

extern pascal trap OSErr C_AEResumeTheCurrentEvent (AppleEvent *evt, AppleEvent *reply,
						    EventHandlerProcPtr dispatcher,
						    int32 refcon);
extern pascal trap OSErr C_AEProcessEvent (EventRecord *evt);

extern pascal trap OSErr C_AEGetInteractionAllowed (AEInteractionAllowed *return_level);

extern pascal trap OSErr C_AEDuplicateDesc (AEDesc *src, AEDesc *dst);

extern pascal trap OSErr C_AECountItems (AEDescList *list, GUEST<int32> *count_out);

extern pascal trap OSErr C_AEDeleteItem (AEDescList *list, int32 index);

extern pascal trap OSErr C_AEDeleteKeyDesc (AERecord *record, AEKeyword keyword);

extern pascal trap OSErr C_AEInstallSpecialHandler (AEKeyword function_class,
						    ProcPtr hdlr,
						    Boolean system_handler_p);

extern pascal trap OSErr C_AERemoveSpecialHandler (AEKeyword function_class,
						   ProcPtr hdlr,
						   Boolean system_handler_p);
extern pascal trap OSErr C_AEGetSpecialHandler (AEKeyword function_class,
						GUEST<ProcPtr> *hdlr_out,
						Boolean system_handler_p);

extern pascal trap OSErr C_AESend (AppleEvent *evt, AppleEvent *reply,
				   AESendMode send_mode, AESendPriority send_priority,
				   int32 timeout, IdleProcPtr idle_proc,
				   EventFilterProcPtr filter_proc);

extern pascal trap OSErr C_AECoercePtr (DescType data_type, Ptr data, Size data_size,
					DescType result_type, AEDesc *desc_out);

extern pascal trap OSErr C_AEGetEventHandler (AEEventClass event_class,
					      AEEventID event_id,
					      GUEST<EventHandlerProcPtr> *hdlr, GUEST<int32> *refcon,
					      Boolean system_handler_p);

extern pascal trap OSErr C_AERemoveEventHandler (AEEventClass event_class,
						 AEEventID event_id,
						 EventHandlerProcPtr hdlr,
						 Boolean system_handler_p);

extern pascal trap OSErr C_AESetInteractionAllowed (AEInteractionAllowed level);

extern pascal trap OSErr C_AEProcessAppleEvent (EventRecord *evt);

extern pascal trap OSErr C_AEPutDesc (AEDescList *list, int32 index,
				      AEDesc *desc);


extern pascal trap OSErr C_AEPutAttributePtr (AppleEvent *evt, AEKeyword keyword,
					      DescType type, Ptr data, Size size);

extern pascal trap OSErr C_AEPutAttributeDesc (AppleEvent *evt, AEKeyword keyword,
					       AEDesc *desc);

extern pascal trap OSErr C_AEGetNthPtr (AEDescList *list, int32 index,
					DescType desired_type, GUEST<AEKeyword> *keyword_out,
					GUEST<DescType> *type_out,
					Ptr data, int32 max_size, GUEST<int32> *size_out);

extern pascal trap OSErr C_AEGetAttributePtr (AppleEvent *evt, AEKeyword keyword,
					      DescType desired_type, GUEST<DescType> *type_out,
					      Ptr data, Size max_size, GUEST<Size> *size_out);

extern pascal trap OSErr C_AEGetArray (AEDescList *list,
				       AEArrayType array_type,
				       AEArrayDataPointer array_ptr, Size max_size,
				       GUEST<DescType> *return_item_type,
				       GUEST<Size> *return_item_size,
				       GUEST<int32> *return_item_count);

extern pascal trap OSErr C_AECreateAppleEvent
  (AEEventClass event_class, AEEventID event_id,
   AEAddressDesc *target, int16 return_id, int32 transaction_id, AppleEvent *evt);

extern pascal trap OSErr C_AEInstallCoercionHandler
  (DescType from_type, DescType to_type,
   ProcPtr hdlr, int32 refcon, Boolean from_type_is_desc_p, Boolean system_handler_p);

extern pascal trap OSErr C_AEInstallEventHandler
  (AEEventClass event_class,  AEEventID event_id,
   EventHandlerProcPtr hdlr, int32 refcon, Boolean system_handler_p);

extern pascal trap OSErr C_AERemoveCoercionHandler
  (DescType from_type, DescType to_type,
   ProcPtr hdlr, Boolean system_handler_p);

extern pascal trap OSErr C_AEPutArray (AEDescList *list, AEArrayType type,
				       AEArrayDataPointer array_data,
				       DescType item_type,
				       Size item_size, int32 item_count);

extern pascal trap OSErr C_AECreateList (Ptr list_elt_prefix, Size list_elt_prefix_size,
					 Boolean is_record_p, AEDescList *list_out);

extern pascal trap OSErr C_AEGetAttributeDesc (AppleEvent *evt, AEKeyword keyword,
					       DescType desired_type, AEDesc *desc_out);

extern pascal trap OSErr C_AESizeOfAttribute (AppleEvent *evt, AEKeyword keyword,
					      GUEST<DescType> *type_out, GUEST<Size> *size_out);

extern pascal trap OSErr C_AEGetNthDesc (AEDescList *list, int32 index,
					 DescType desired_type, GUEST<AEKeyword> *keyword_out,
					 AEDesc *desc_out);

extern pascal trap OSErr C_AESizeOfNthItem (AEDescList *list, int32 index,
					    GUEST<DescType> *type_out, GUEST<Size> *size_out);

extern pascal trap OSErr C_AEPutPtr (AEDescList *list, int32 index, DescType type,
				     Ptr data, Size data_size);

extern pascal trap OSErr C_AEInteractWithUser (int32 timeout, NMRecPtr nm_req,
					       IdleProcPtr idle_proc);

extern void AE_init (void);
extern void AE_reinit (void);

extern pascal trap OSErr C_AEManagerInfo (GUEST<LONGINT> *resultp);

#if 0
#if !defined (AE_info_H)
extern GUEST<AE_info_ptr> AE_info_H;
#endif

#define AE_info (AE_info_H.p)
#endif

extern pascal trap OSErr C_AEDisposeToken (AEDesc *theToken);
extern pascal trap OSErr C_AEREesolve (AEDesc *objectSpecifier,
				       INTEGER callbackFlags,
				       AEDesc *theToken);
extern pascal trap OSErr C_AERemoveObjectAccessor (DescType desiredClass,
						   DescType containerType,
						   ProcPtr theAccessor,
						   BOOLEAN isSysHandler);
extern pascal trap OSErr C_AEInstallObjectAccessor (DescType desiredClass,
						    DescType containerType,
						    ProcPtr theAccessor,
						    LONGINT refcon,
						    BOOLEAN isSysHandler);
extern pascal trap OSErr C_AEGetObjectAccessor (DescType desiredClass,
						DescType containerType,
						ProcPtr *theAccessor,
						LONGINT *accessorRefcon,
						BOOLEAN isSysHandler);
extern pascal trap OSErr C_AECallObjectAccessor (DescType desiredClass,
						  AEDesc *containerToken,
						  DescType containerClass,
						  DescType keyForm,
						  AEDesc *keyData,
						  AEDesc *theToken);
extern pascal trap OSErr C_AESetObjectCallbacks (ProcPtr myCompareProc,
						 ProcPtr myCountProc,
						 ProcPtr myDisposeTokenProc,
						 ProcPtr myGetMarkTokenProc,
						 ProcPtr myMarkProc,
						 ProcPtr myAdjustMarksProc,
						 ProcPtr myGetErrDescProc);
}

#endif /* ! _AppleEvents_H_ */
