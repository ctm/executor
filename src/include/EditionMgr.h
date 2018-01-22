
#if !defined(_EDITIONMGR_H_)
#define _EDITIONMGR_H_

/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "IntlUtil.h"
#include "AliasMgr.h"

namespace Executor
{
#define declare_subtypes(type)                \
    typedef struct type##Record type##Record; \
    typedef type##Record *type##Ptr;          \
    typedef GUEST<type##Ptr> *type##Handle

#define declare_record_subtypes(type) \
    typedef struct type type;         \
    typedef type *type##Ptr;          \
    typedef GUEST<type##Ptr> *type##Handle

typedef int32_t TimeStamp;
typedef Handle EditionRefNum;
typedef int16_t UpdateMode;
typedef SignedByte SectionType;
typedef char FormatType[4];

typedef ProcPtr ExpDialogHookProcPtr;
typedef ProcPtr ExpModalFilterProcPtr;
typedef ProcPtr FormatIOProcPtr;
typedef ProcPtr EditionOpenerProcPtr;

struct SectionRecord
{
    GUEST_STRUCT;
    GUEST<SignedByte> version;
    GUEST<SectionType> kind;
    GUEST<UpdateMode> mode;
    GUEST<TimeStamp> mdDate;
    GUEST<int32_t> sectionID;
    GUEST<int32_t> refCon;
    GUEST<AliasHandle> alias;
    GUEST<int32_t> subPart;
    GUEST<Handle> nextSection; /* ### Section */
    GUEST<Handle> controlBlock;
    GUEST<EditionRefNum> refNum;
};

declare_record_subtypes(Section);

struct EditionContainerSpec
{
    GUEST_STRUCT;
    GUEST<FSSpec> theFile;
    GUEST<ScriptCode> theFileScript;
    GUEST<int32_t> thePart;
    GUEST<Str31> thePartName;
    GUEST<ScriptCode> thePartScript;
};

typedef struct EditionContainerSpec EditionContainerSpec;
typedef EditionContainerSpec *EditionContainerSpecPtr;

struct EditionInfoRecord
{
    GUEST_STRUCT;
    GUEST<TimeStamp> crDate;
    GUEST<TimeStamp> mdDate;
    GUEST<OSType> fdCreator;
    GUEST<OSType> fdType;
    GUEST<EditionContainerSpec> container;
};

typedef struct EditionInfoRecord EditionInfoRecord;
typedef EditionInfoRecord *EditionInfoPtr;

struct NewPublisherReply
{
    GUEST_STRUCT;
    GUEST<Boolean> canceled;
    GUEST<Boolean> replacing;
    GUEST<Boolean> usePart;
    GUEST<uint8> _filler;
    GUEST<Handle> preview;
    GUEST<FormatType> previewFormat;
    GUEST<EditionContainerSpec> container;
};

typedef struct NewPublisherReply NewPublisherReply;
typedef NewPublisherReply *NewPublisherReplyPtr;

struct NewSubscriberReply
{
    GUEST_STRUCT;
    GUEST<Boolean> canceled;
    GUEST<SignedByte> formatsMask;
    GUEST<EditionContainerSpec> container;
};

typedef struct NewSubscriberReply NewSubscriberReply;
typedef NewSubscriberReply *NewSubscriberReplyPtr;

struct SectionOptionsReply
{
    GUEST_STRUCT;
    GUEST<Boolean> canceled;
    GUEST<Boolean> changed;
    GUEST<SectionHandle> sectionH;
    GUEST<ResType> action;
};

typedef struct SectionOptionsReply SectionOptionsReply;
typedef SectionOptionsReply *SectionOptionsReplyPtr;

typedef uint8 EditionOpenerVerb;

enum
{
    eoOpen = (0),
    eoClose = (1),
    eoOpenNew = (2),
    eoCloseNew = (3),
    eoCanSubscribe = (4),
};

struct EditionOpenerParamBlock
{
    GUEST_STRUCT;
    GUEST<EditionInfoRecord> info;
    GUEST<SectionHandle> sectionH;
    GUEST<FSSpecPtr> document;
    GUEST<OSType> fdCreator;
    GUEST<int32_t> ioRefNum;
    GUEST<FormatIOProcPtr> ioProc;
    GUEST<Boolean> success;
    GUEST<SignedByte> formatsMask;
};

typedef struct EditionOpenerParamBlock EditionOpenerParamBlock;
typedef EditionOpenerParamBlock *EditionOpenerParamBlockPtr;

typedef uint8 FormatIOVerb;

enum
{
    ioHasFormat = (0),
    ioReadFormat = (1),
    ioNewFormat = (2),
    ioWtriteFormat = (3),
};

struct FormatIOParamBlock
{
    GUEST_STRUCT;
    GUEST<int32_t> ioRefNum;
    GUEST<FormatType> format;
    GUEST<int32_t> formatIndex;
    GUEST<int32_t> offset;
    GUEST<Ptr> buffPtr;
    GUEST<int32_t> buffLen;
};

typedef struct FormatIOParamBlock FormatIOParamBlock;
typedef FormatIOParamBlock *FormatIOParamBlockPtr;

enum
{
    flLckedErr = (-45),
    fBusyErr = (-47),
    noTypeErr = (-102),
    userCanceledErr = (-128),
    editionMgrInitErr = (-450),
    badSectionErr = (-451),
    notRegisteredSectionErr = (-452),
    badSubPartErr = (-454),
    multiplePubliserWrn = (-460),
    containerNotFoundWrn = (-461),
    notThePublisherWrn = (-463),
};

extern OSErr C_InitEditionPackVersion(INTEGER unused);
PASCAL_SUBTRAP(InitEditionPackVersion, 0xA82D, Pack11);
extern OSErr C_NewSection(EditionContainerSpecPtr container,
                                      FSSpecPtr section_doc,
                                      SectionType kind, int32_t section_id,
                                      UpdateMode initial_mode,
                                      SectionHandle *section_out);
PASCAL_SUBTRAP(NewSection, 0xA82D, Pack11);
extern OSErr C_RegisterSection(FSSpecPtr section_doc,
                                           SectionHandle section,
                                           Boolean *alias_was_updated_p_out);
PASCAL_SUBTRAP(RegisterSection, 0xA82D, Pack11);
extern OSErr C_UnRegisterSection(SectionHandle section);
PASCAL_SUBTRAP(UnRegisterSection, 0xA82D, Pack11);
extern OSErr C_IsRegisteredSection(SectionHandle section);
PASCAL_SUBTRAP(IsRegisteredSection, 0xA82D, Pack11);
extern OSErr C_AssociateSection(SectionHandle section,
                                            FSSpecPtr new_section_doc);
PASCAL_SUBTRAP(AssociateSection, 0xA82D, Pack11);
extern OSErr C_CreateEditionContainerFile(FSSpecPtr edition_file, OSType creator,
                                                      ScriptCode edition_file_name_script);
PASCAL_SUBTRAP(CreateEditionContainerFile, 0xA82D, Pack11);
extern OSErr C_DeleteEditionContainerFile(FSSpecPtr edition_file);
PASCAL_SUBTRAP(DeleteEditionContainerFile, 0xA82D, Pack11);

extern OSErr C_SetEditionFormatMark(EditionRefNum edition,
                                                FormatType format,
                                                int32_t mark);
PASCAL_SUBTRAP(SetEditionFormatMark, 0xA82D, Pack11);

extern OSErr C_GetEditionFormatMark(EditionRefNum edition,
                                                FormatType format,
                                                int32_t *currentMark);
PASCAL_SUBTRAP(GetEditionFormatMark, 0xA82D, Pack11);

extern OSErr C_OpenEdition(SectionHandle subscriber_section,
                                       EditionRefNum *ref_num);
PASCAL_SUBTRAP(OpenEdition, 0xA82D, Pack11);

extern OSErr C_EditionHasFormat(EditionRefNum edition,
                                            FormatType format,
                                            Size *format_size);
PASCAL_SUBTRAP(EditionHasFormat, 0xA82D, Pack11);
extern OSErr C_ReadEdition(EditionRefNum edition,
                                       FormatType format,
                                       Ptr buffer, Size buffer_size);
PASCAL_SUBTRAP(ReadEdition, 0xA82D, Pack11);

extern OSErr C_OpenNewEdition(SectionHandle publisher_section,
                                          OSType creator,
                                          FSSpecPtr publisher_section_doc,
                                          EditionRefNum *ref_num);
PASCAL_SUBTRAP(OpenNewEdition, 0xA82D, Pack11);

extern OSErr C_WriteEdition(EditionRefNum edition, FormatType format,
                                        Ptr buffer, Size buffer_size);
PASCAL_SUBTRAP(WriteEdition, 0xA82D, Pack11);

extern OSErr C_CloseEdition(EditionRefNum edition, Boolean success_p);
PASCAL_SUBTRAP(CloseEdition, 0xA82D, Pack11);

extern OSErr C_GetLastEditionContainerUsed(EditionContainerSpecPtr container);
PASCAL_SUBTRAP(GetLastEditionContainerUsed, 0xA82D, Pack11);

extern OSErr C_NewSubscriberDialog(NewSubscriberReplyPtr reply);
PASCAL_SUBTRAP(NewSubscriberDialog, 0xA82D, Pack11);
extern OSErr C_NewPublisherDialog(NewSubscriberReplyPtr reply);
PASCAL_SUBTRAP(NewPublisherDialog, 0xA82D, Pack11);
extern OSErr C_SectionOptionsDialog(SectionOptionsReply *reply);
PASCAL_SUBTRAP(SectionOptionsDialog, 0xA82D, Pack11);

extern OSErr C_NewSubscriberExpDialog(NewSubscriberReplyPtr reply, Point where,
                                                  int16_t expnasion_ditl_res_id,
                                                  ExpDialogHookProcPtr dialog_hook,
                                                  ExpModalFilterProcPtr filter_hook,
                                                  Ptr data);
PASCAL_SUBTRAP(NewSubscriberExpDialog, 0xA82D, Pack11);

extern OSErr C_NewPublisherExpDialog(NewPublisherReplyPtr reply, Point where,
                                                 int16_t expnasion_ditl_res_id,
                                                 ExpDialogHookProcPtr dialog_hook,
                                                 ExpModalFilterProcPtr filter_hook,
                                                 Ptr data);
PASCAL_SUBTRAP(NewPublisherExpDialog, 0xA82D, Pack11);

extern OSErr C_SectionOptionsExpDialog(SectionOptionsReply *reply,
                                                   Point where, int16_t expnasion_ditl_res_id,
                                                   ExpDialogHookProcPtr dialog_hook,
                                                   ExpModalFilterProcPtr filter_hook,
                                                   Ptr data);
PASCAL_SUBTRAP(SectionOptionsExpDialog, 0xA82D, Pack11);

extern OSErr C_GetEditionInfo(SectionHandle section,
                                          EditionInfoPtr edition_info);
PASCAL_SUBTRAP(GetEditionInfo, 0xA82D, Pack11);

extern OSErr C_GoToPublisherSection(EditionContainerSpecPtr container);
PASCAL_SUBTRAP(GoToPublisherSection, 0xA82D, Pack11);

extern OSErr C_GetStandardFormats(EditionContainerSpecPtr container,
                                              FormatType *preview_format,
                                              Handle preview,
                                              Handle publisher_alias,
                                              Handle formats);
PASCAL_SUBTRAP(GetStandardFormats, 0xA82D, Pack11);
extern OSErr C_GetEditionOpenerProc(EditionOpenerProcPtr *opener);
PASCAL_SUBTRAP(GetEditionOpenerProc, 0xA82D, Pack11);
extern OSErr C_SetEditionOpenerProc(EditionOpenerProcPtr opener);
PASCAL_SUBTRAP(SetEditionOpenerProc, 0xA82D, Pack11);

extern OSErr C_CallEditionOpenerProc(EditionOpenerVerb selector,
                                                 EditionOpenerParamBlock *param_block,
                                                 EditionOpenerProcPtr opener);
PASCAL_SUBTRAP(CallEditionOpenerProc, 0xA82D, Pack11);

extern OSErr C_CallFormatIOProc(FormatIOVerb selector,
                                            FormatIOParamBlock *param_block,
                                            FormatIOProcPtr proc);
PASCAL_SUBTRAP(CallFormatIOProc, 0xA82D, Pack11);
}

#endif /* _EDITIONMGR_H_ */
