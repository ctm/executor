#if !defined(_AppleEvents_H_)
#define _AppleEvents_H_

/*
 * Copyright 1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "EventMgr.h"
#include "NotifyMgr.h"

namespace Executor
{
typedef int32_t AEEventClass;
typedef int32_t AEEventID;
typedef int32_t AEKeyword;
typedef ResType DescType;

typedef struct AEDesc
{
    GUEST_STRUCT;
    GUEST<DescType> descriptorType;
    GUEST<Handle> dataHandle;
} AEDesc;

/* ### hack, delete */
typedef AEDesc descriptor_t;

#define DESC_TYPE_X(desc) ((desc)->descriptorType)
#define DESC_DATA_X(desc) ((desc)->dataHandle)

#define DESC_TYPE(desc) (CL(DESC_TYPE_X(desc)))
#define DESC_DATA(desc) (MR(DESC_DATA_X(desc)))

typedef struct AEKeyDesc
{
    GUEST<AEKeyword> descKey;
    GUEST<AEDesc> descContent;
} AEKeyDesc;

typedef AEKeyDesc key_desc_t;

#define KEY_DESC_KEYWORD_X(keydesc) ((keydesc)->descKey)

#define KEY_DESC_KEYWORD(keydesc) (CL(KEY_DESC_KEYWORD_X(keydesc)))
#define KEY_DESC_CONTENT(keydesc) ((keydesc)->descContent)

typedef AEDesc AEAddressDesc;
typedef AEDesc AEDescList;
typedef AEDescList AERecord;
typedef AERecord AppleEvent;

typedef int32_t AESendMode;

enum
{
    _kAEReplyMask = (0x3),
};

enum
{
    kAENoReply = (0x1),
    kAEQueueReply = (0x2),
    kAEWaitReply = (0x3),
};

enum
{
    _kAEInteractMask = (0x30),
};

enum
{
    kAENeverInteract = (0x10),
    kAECanInteract = (0x20),
    kAEAlwaysInteract = (0x30),
};

enum
{
    kAECanSwitchLayer = (0x40),
    kAEDontReconnect = (0x80),
};
/* #define kAEWantReceipt	??? */

typedef int16_t AESendPriority;

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
    kAEDataArray,
    kAEPackedArray,
    kAEHandleArray,
    kAEDescArray,
    kAEKeyDescArray
};

typedef union AEArrayData {
    GUEST<int16_t> AEDataArray[1];
    int8 AEPackedArray[1];
    GUEST<Handle> AEHandleArray[1];
    AEDesc AEDescArray[1];
    AEKeyDesc AEKeyDescArray[1];
} AEArrayData;

typedef AEArrayData *AEArrayDataPointer;

typedef ProcPtr IdleProcPtr;
typedef ProcPtr EventFilterProcPtr;

typedef UPP<OSErr(AppleEvent *evt, AppleEvent *reply, int32_t refcon)> EventHandlerProcPtr;
typedef UPP<OSErr(DescType data_type, Ptr data, Size data_size, DescType to_type, int32_t refcon, AEDesc *desc_out)> CoercePtrProcPtr;
typedef UPP<OSErr(AEDesc *desc, DescType to_type, int32_t refcon, AEDesc *desc_out)> CoerceDescProcPtr;

/* #### internal */

typedef struct AE_hdlr
{
    GUEST_STRUCT;
    GUEST<void *> fn;
    GUEST<int32_t> refcon;
} AE_hdlr_t;

typedef struct AE_hdlr_selector
{
    GUEST_STRUCT;
    GUEST<int32_t> sel0;
    GUEST<int32_t> sel1;
} AE_hdlr_selector_t;

typedef struct AE_hdlr_table_elt
{
    GUEST_STRUCT;
    GUEST<int32_t> pad_1;
    GUEST<AE_hdlr_selector_t> selector;
    GUEST<AE_hdlr_t> hdlr;
    GUEST<int32_t> pad_2;
} AE_hdlr_table_elt_t;

#define AE_TABLE_ELTS(table) (HxX(table, elts))

#define AE_TABLE_N_ELTS_X(table) (HxX(table, n_elts))
#define AE_TABLE_N_ALLOCATED_BYTES_X(table) \
    (HxX(table, n_allocated_bytes))

#define AE_TABLE_N_ELTS(table) \
    (CL(AE_TABLE_N_ELTS_X(table)))
#define AE_TABLE_N_ALLOCATED_BYTES(table) \
    (CL(AE_TABLE_N_ALLOCATED_BYTES_X(table)))

typedef struct AE_hdlr_table
{
    GUEST_STRUCT;
    GUEST<int32_t> pad_1;
    GUEST<int32_t> n_allocated_bytes;
    GUEST<int32_t> n_elts;
    GUEST<int32_t[10]> pad_2;
    GUEST<AE_hdlr_table_elt_t> elts[0];
} AE_hdlr_table_t;

typedef AE_hdlr_table_t *AE_hdlr_table_ptr;

typedef GUEST<AE_hdlr_table_ptr> *AE_hdlr_table_h;

/* points to a 32byte handle of unknown contents (at least,
     sometimes) */
typedef struct AE_zone_tables
{
    GUEST_STRUCT;
    GUEST<AE_hdlr_table_h> event_hdlr_table;
    GUEST<AE_hdlr_table_h> coercion_hdlr_table;
    GUEST<AE_hdlr_table_h> special_hdlr_table;
    GUEST<char[28]> pad_1;
    GUEST<char[4]> unknown_appl_value;
    GUEST<char[8]> pad_2;
    GUEST<Handle> unknown_sys_handle;
} AE_zone_tables_t;

typedef AE_zone_tables_t *AE_zone_tables_ptr;

typedef GUEST<AE_zone_tables_ptr> *AE_zone_tables_h;

/* offset of `appl_zone_tables' is 340; handle to a `struct tables' */

/* offset of `system_zone_tables' is 380; handle to a `struct tables' */

typedef struct
{
    GUEST_STRUCT;
    GUEST<char[340]> pad_1;
    GUEST<AE_zone_tables_h> appl_zone_tables;
    GUEST<char[36]> pad_2;
    GUEST<AE_zone_tables_h> system_zone_tables;
    GUEST<char[212]> pad_3;
} AE_info_t;

typedef AE_info_t *AE_info_ptr;

extern OSErr C__AE_hdlr_table_alloc(int32_t, int32_t, int32_t, int8,
                                                GUEST<AE_hdlr_table_h> *);
PASCAL_SUBTRAP(_AE_hdlr_table_alloc, 0xA816, Pack8);
extern OSErr C__AE_hdlr_delete(AE_hdlr_table_h, int32_t,
                                           AE_hdlr_selector_t *);
PASCAL_SUBTRAP(_AE_hdlr_delete, 0xA816, Pack8);
extern OSErr C__AE_hdlr_lookup(AE_hdlr_table_h, int32_t,
                                           AE_hdlr_selector_t *,
                                           AE_hdlr_t *);
PASCAL_SUBTRAP(_AE_hdlr_lookup, 0xA816, Pack8);
extern OSErr C__AE_hdlr_install(AE_hdlr_table_h, int32_t,
                                            AE_hdlr_selector_t *,
                                            AE_hdlr_t *);
PASCAL_SUBTRAP(_AE_hdlr_install, 0xA816, Pack8);

/* private */

extern bool send_application_open_aevt_p;
extern bool application_accepts_open_app_aevt_p;

/*  error codes */

#define AE_RETURN_ERROR(error)                         \
    do {                                               \
        OSErr _error_ = (error);                       \
                                                       \
        if(_error_ != noErr)                           \
            warning_unexpected("error `%d'", _error_); \
        return _error_;                                \
    } while(0)

enum
{
    invalidConnection = (-609),
};

enum
{
    errAECoercionFail = (-1700),
    errAEDescNotFound = (-1701),
    errAEWrongDataType = (-1703),
    errAENotAEDesc = (-1704),
};

enum
{
    errAEEventNotHandled = (-1708),
    errAEUnknownAddressType = (-1716),
};

enum
{
    errAEHandlerNotFound = (-1717),
    errAEIllegalIndex = (-1719),
};

/* types */

#define typeFSS (FOURCC('f', 's', 's', ' '))

#define typeAEList (FOURCC('l', 'i', 's', 't'))
#define typeAERecord (FOURCC('r', 'e', 'c', 'o'))
#define typeAppleEvent (FOURCC('a', 'e', 'v', 't'))
#define typeProcessSerialNumber (FOURCC('p', 's', 'n', ' '))
#define typeNull (FOURCC('n', 'u', 'l', 'l'))
#define typeApplSignature (FOURCC('s', 'i', 'g', 'n'))

#define typeType (FOURCC('t', 'y', 'p', 'e'))
#define typeWildCard (FOURCC('*', '*', '*', '*'))
#define typeAlias (FOURCC('a', 'l', 'i', 's'))

#define keyAddressAttr (FOURCC('a', 'd', 'd', 'r'))
#define keyEventClassAttr (FOURCC('e', 'v', 'c', 'l'))
#define keyEventIDAttr (FOURCC('e', 'v', 'i', 'd'))
#define keyProcessSerialNumber (FOURCC('p', 's', 'n', ' '))

#define keyDirectObject (FOURCC('-', '-', '-', '-'))

#define kCoreEventClass (FOURCC('a', 'e', 'v', 't'))
#define kAEOpenApplication (FOURCC('o', 'a', 'p', 'p'))
#define kAEOpenDocuments (FOURCC('o', 'd', 'o', 'c'))
#define kAEPrintDocuments (FOURCC('p', 'd', 'o', 'c'))
#define kAEAnswer (FOURCC('a', 'n', 's', 'r'))
#define kAEQuitApplication (FOURCC('q', 'u', 'i', 't'))

#define keySelectProc (FOURCC('s', 'e', 'l', 'h'))

/* #### OSL internal */
extern syn68k_addr_t /*ProcPtr*/ AE_OSL_select_fn;

const LowMemGlobal<AE_info_ptr> AE_info { 0x2B6 }; // AppleEvents AEGizmo (true);

/* prototypes go here */

extern OSErr C_AEGetCoercionHandler(DescType from_type, DescType to_type,
                                                GUEST<CoerceDescProcPtr> *hdlr_out, GUEST<int32_t> *refcon_out,
                                                GUEST<Boolean> *from_type_is_desc_p_out, Boolean system_handler_p);
PASCAL_SUBTRAP(AEGetCoercionHandler, 0xA816, Pack8);

extern OSErr C_AECreateDesc(DescType type,
                                        Ptr data, Size data_size,
                                        AEDesc *desc_out);
PASCAL_SUBTRAP(AECreateDesc, 0xA816, Pack8);
extern OSErr C_AEDisposeDesc(AEDesc *desc);
PASCAL_SUBTRAP(AEDisposeDesc, 0xA816, Pack8);

extern OSErr C_AECoerceDesc(AEDesc *desc, DescType result_type,
                                        AEDesc *desc_out);
PASCAL_SUBTRAP(AECoerceDesc, 0xA816, Pack8);

extern OSErr C_AEGetKeyPtr(AERecord *record, AEKeyword keyword,
                                       DescType desired_type, GUEST<DescType> *type_out,
                                       Ptr data, Size max_size, GUEST<Size> *size_out);
PASCAL_SUBTRAP(AEGetKeyPtr, 0xA816, Pack8);

extern OSErr C_AEGetKeyDesc(AERecord *record, AEKeyword keyword,
                                        DescType desired_type, AEDesc *desc_out);
PASCAL_SUBTRAP(AEGetKeyDesc, 0xA816, Pack8);

extern OSErr C_AEPutKeyPtr(AERecord *record, AEKeyword keyword,
                                       DescType type, Ptr data, Size data_size);
PASCAL_SUBTRAP(AEPutKeyPtr, 0xA816, Pack8);
extern OSErr C_AEPutKeyDesc(AERecord *record, AEKeyword keyword,
                                        AEDesc *desc);
PASCAL_SUBTRAP(AEPutKeyDesc, 0xA816, Pack8);

/*
extern OSErr C_AEDeleteParam(AppleEvent *evt, AEKeyword keyword);
PASCAL_SUBTRAP(AEDeleteParam, 0xA816, Pack8);
*/

extern OSErr C_AEDeleteAttribute(AppleEvent *evt,
                                             AEKeyword keyword);
PASCAL_SUBTRAP(AEDeleteAttribute, 0xA816, Pack8);

extern OSErr C_AESizeOfKeyDesc(AERecord *record, AEKeyword keyword,
                                           GUEST<DescType> *type_out, GUEST<Size> *size_out);
PASCAL_SUBTRAP(AESizeOfKeyDesc, 0xA816, Pack8);

extern OSErr C_AESetInteractionAllowed(AEInteractionAllowed level);
PASCAL_SUBTRAP(AESetInteractionAllowed, 0xA816, Pack8);

extern OSErr C_AEResetTimer(AppleEvent *evt);
PASCAL_SUBTRAP(AEResetTimer, 0xA816, Pack8);

extern OSErr C_AEGetTheCurrentEvent(AppleEvent *return_evt);
PASCAL_SUBTRAP(AEGetTheCurrentEvent, 0xA816, Pack8);
extern OSErr C_AESetTheCurrentEvent(AppleEvent *evt);
PASCAL_SUBTRAP(AESetTheCurrentEvent, 0xA816, Pack8);
extern OSErr C_AESuspendTheCurrentEvent(AppleEvent *evt);
PASCAL_SUBTRAP(AESuspendTheCurrentEvent, 0xA816, Pack8);

extern OSErr C_AEResumeTheCurrentEvent(AppleEvent *evt, AppleEvent *reply,
                                                   EventHandlerProcPtr dispatcher,
                                                   int32_t refcon);
PASCAL_SUBTRAP(AEResumeTheCurrentEvent, 0xA816, Pack8);

/*
extern OSErr C_AEProcessEvent(EventRecord *evt);
PASCAL_FUNCTION(AEProcessEvent);
*/

extern OSErr C_AEGetInteractionAllowed(AEInteractionAllowed *return_level);
PASCAL_SUBTRAP(AEGetInteractionAllowed, 0xA816, Pack8);

extern OSErr C_AEDuplicateDesc(AEDesc *src, AEDesc *dst);
PASCAL_SUBTRAP(AEDuplicateDesc, 0xA816, Pack8);

extern OSErr C_AECountItems(AEDescList *list, GUEST<int32_t> *count_out);
PASCAL_SUBTRAP(AECountItems, 0xA816, Pack8);

extern OSErr C_AEDeleteItem(AEDescList *list, int32_t index);
PASCAL_SUBTRAP(AEDeleteItem, 0xA816, Pack8);

extern OSErr C_AEDeleteKeyDesc(AERecord *record, AEKeyword keyword);
PASCAL_SUBTRAP(AEDeleteKeyDesc, 0xA816, Pack8);

extern OSErr C_AEInstallSpecialHandler(AEKeyword function_class,
                                                   EventHandlerProcPtr hdlr,
                                                   Boolean system_handler_p);
PASCAL_SUBTRAP(AEInstallSpecialHandler, 0xA816, Pack8);

extern OSErr C_AERemoveSpecialHandler(AEKeyword function_class,
                                                  EventHandlerProcPtr hdlr,
                                                  Boolean system_handler_p);
PASCAL_SUBTRAP(AERemoveSpecialHandler, 0xA816, Pack8);
extern OSErr C_AEGetSpecialHandler(AEKeyword function_class,
                                               GUEST<EventHandlerProcPtr> *hdlr_out,
                                               Boolean system_handler_p);
PASCAL_SUBTRAP(AEGetSpecialHandler, 0xA816, Pack8);

extern OSErr C_AESend(AppleEvent *evt, AppleEvent *reply,
                                  AESendMode send_mode, AESendPriority send_priority,
                                  int32_t timeout, IdleProcPtr idle_proc,
                                  EventFilterProcPtr filter_proc);
PASCAL_SUBTRAP(AESend, 0xA816, Pack8);

extern OSErr C_AECoercePtr(DescType data_type, Ptr data, Size data_size,
                                       DescType result_type, AEDesc *desc_out);
PASCAL_SUBTRAP(AECoercePtr, 0xA816, Pack8);

extern OSErr C_AEGetEventHandler(AEEventClass event_class,
                                             AEEventID event_id,
                                             GUEST<EventHandlerProcPtr> *hdlr, GUEST<int32_t> *refcon,
                                             Boolean system_handler_p);
PASCAL_SUBTRAP(AEGetEventHandler, 0xA816, Pack8);

extern OSErr C_AERemoveEventHandler(AEEventClass event_class,
                                                AEEventID event_id,
                                                EventHandlerProcPtr hdlr,
                                                Boolean system_handler_p);
PASCAL_SUBTRAP(AERemoveEventHandler, 0xA816, Pack8);

extern OSErr C_AEProcessAppleEvent(EventRecord *evt);
PASCAL_SUBTRAP(AEProcessAppleEvent, 0xA816, Pack8);

extern OSErr C_AEPutDesc(AEDescList *list, int32_t index,
                                     AEDesc *desc);
PASCAL_SUBTRAP(AEPutDesc, 0xA816, Pack8);

extern OSErr C_AEPutAttributePtr(AppleEvent *evt, AEKeyword keyword,
                                             DescType type, Ptr data, Size size);
PASCAL_SUBTRAP(AEPutAttributePtr, 0xA816, Pack8);

extern OSErr C_AEPutAttributeDesc(AppleEvent *evt, AEKeyword keyword,
                                              AEDesc *desc);
PASCAL_SUBTRAP(AEPutAttributeDesc, 0xA816, Pack8);

extern OSErr C_AEGetNthPtr(AEDescList *list, int32_t index,
                                       DescType desired_type, GUEST<AEKeyword> *keyword_out,
                                       GUEST<DescType> *type_out,
                                       Ptr data, int32_t max_size, GUEST<int32_t> *size_out);
PASCAL_SUBTRAP(AEGetNthPtr, 0xA816, Pack8);

extern OSErr C_AEGetAttributePtr(AppleEvent *evt, AEKeyword keyword,
                                             DescType desired_type, GUEST<DescType> *type_out,
                                             Ptr data, Size max_size, GUEST<Size> *size_out);
PASCAL_SUBTRAP(AEGetAttributePtr, 0xA816, Pack8);

extern OSErr C_AEGetArray(AEDescList *list,
                                      AEArrayType array_type,
                                      AEArrayDataPointer array_ptr, Size max_size,
                                      GUEST<DescType> *return_item_type,
                                      GUEST<Size> *return_item_size,
                                      GUEST<int32_t> *return_item_count);
PASCAL_SUBTRAP(AEGetArray, 0xA816, Pack8);

extern OSErr C_AECreateAppleEvent(AEEventClass event_class, AEEventID event_id,
                                              AEAddressDesc *target, int16_t return_id, int32_t transaction_id, AppleEvent *evt);
PASCAL_SUBTRAP(AECreateAppleEvent, 0xA816, Pack8);

extern OSErr C_AEInstallCoercionHandler(DescType from_type, DescType to_type,
                                                    CoerceDescProcPtr hdlr, int32_t refcon, Boolean from_type_is_desc_p, Boolean system_handler_p);
PASCAL_SUBTRAP(AEInstallCoercionHandler, 0xA816, Pack8);

extern OSErr C_AEInstallEventHandler(AEEventClass event_class, AEEventID event_id,
                                                 EventHandlerProcPtr hdlr, int32_t refcon, Boolean system_handler_p);
PASCAL_SUBTRAP(AEInstallEventHandler, 0xA816, Pack8);

extern OSErr C_AERemoveCoercionHandler(DescType from_type, DescType to_type,
                                                   CoerceDescProcPtr hdlr, Boolean system_handler_p);
PASCAL_SUBTRAP(AERemoveCoercionHandler, 0xA816, Pack8);

extern OSErr C_AEPutArray(AEDescList *list, AEArrayType type,
                                      AEArrayDataPointer array_data,
                                      DescType item_type,
                                      Size item_size, int32_t item_count);
PASCAL_SUBTRAP(AEPutArray, 0xA816, Pack8);

extern OSErr C_AECreateList(Ptr list_elt_prefix, Size list_elt_prefix_size,
                                        Boolean is_record_p, AEDescList *list_out);
PASCAL_SUBTRAP(AECreateList, 0xA816, Pack8);

extern OSErr C_AEGetAttributeDesc(AppleEvent *evt, AEKeyword keyword,
                                              DescType desired_type, AEDesc *desc_out);
PASCAL_SUBTRAP(AEGetAttributeDesc, 0xA816, Pack8);

extern OSErr C_AESizeOfAttribute(AppleEvent *evt, AEKeyword keyword,
                                             GUEST<DescType> *type_out, GUEST<Size> *size_out);
PASCAL_SUBTRAP(AESizeOfAttribute, 0xA816, Pack8);

extern OSErr C_AEGetNthDesc(AEDescList *list, int32_t index,
                                        DescType desired_type, GUEST<AEKeyword> *keyword_out,
                                        AEDesc *desc_out);
PASCAL_SUBTRAP(AEGetNthDesc, 0xA816, Pack8);

extern OSErr C_AESizeOfNthItem(AEDescList *list, int32_t index,
                                           GUEST<DescType> *type_out, GUEST<Size> *size_out);
PASCAL_SUBTRAP(AESizeOfNthItem, 0xA816, Pack8);

extern OSErr C_AEPutPtr(AEDescList *list, int32_t index, DescType type,
                                    Ptr data, Size data_size);
PASCAL_SUBTRAP(AEPutPtr, 0xA816, Pack8);

extern OSErr C_AEInteractWithUser(int32_t timeout, NMRecPtr nm_req,
                                              IdleProcPtr idle_proc);
PASCAL_SUBTRAP(AEInteractWithUser, 0xA816, Pack8);

extern void AE_init(void);
extern void AE_reinit(void);

extern OSErr C_AEManagerInfo(GUEST<LONGINT> *resultp);
PASCAL_SUBTRAP(AEManagerInfo, 0xA816, Pack8);

extern OSErr C_AEDisposeToken(AEDesc *theToken);
PASCAL_SUBTRAP(AEDisposeToken, 0xA816, Pack8);
extern OSErr C_AEResolve(AEDesc *objectSpecifier,
                                      INTEGER callbackFlags,
                                      AEDesc *theToken);
PASCAL_SUBTRAP(AEResolve, 0xA816, Pack8);
extern OSErr C_AERemoveObjectAccessor(DescType desiredClass,
                                                  DescType containerType,
                                                  ProcPtr theAccessor,
                                                  BOOLEAN isSysHandler);
PASCAL_SUBTRAP(AERemoveObjectAccessor, 0xA816, Pack8);
extern OSErr C_AEInstallObjectAccessor(DescType desiredClass,
                                                   DescType containerType,
                                                   ProcPtr theAccessor,
                                                   LONGINT refcon,
                                                   BOOLEAN isSysHandler);
PASCAL_SUBTRAP(AEInstallObjectAccessor, 0xA816, Pack8);
extern OSErr C_AEGetObjectAccessor(DescType desiredClass,
                                               DescType containerType,
                                               ProcPtr *theAccessor,
                                               LONGINT *accessorRefcon,
                                               BOOLEAN isSysHandler);
PASCAL_SUBTRAP(AEGetObjectAccessor, 0xA816, Pack8);
extern OSErr C_AECallObjectAccessor(DescType desiredClass,
                                                AEDesc *containerToken,
                                                DescType containerClass,
                                                DescType keyForm,
                                                AEDesc *keyData,
                                                AEDesc *theToken);
PASCAL_SUBTRAP(AECallObjectAccessor, 0xA816, Pack8);
extern OSErr C_AESetObjectCallbacks(ProcPtr myCompareProc,
                                                ProcPtr myCountProc,
                                                ProcPtr myDisposeTokenProc,
                                                ProcPtr myGetMarkTokenProc,
                                                ProcPtr myMarkProc,
                                                ProcPtr myAdjustMarksProc,
                                                ProcPtr myGetErrDescProc);
PASCAL_SUBTRAP(AESetObjectCallbacks, 0xA816, Pack8);
}

#endif /* ! _AppleEvents_H_ */
