
#if !defined (_EDITIONMGR_H_)
#define _EDITIONMGR_H_

/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: EditionMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "IntlUtil.h"
#include "AliasMgr.h"


namespace Executor {
#define declare_subtypes(type)						\
typedef struct type ## Record type ## Record;				\
typedef type ## Record *type ## Ptr;					\
MAKE_HIDDEN(type ## Ptr);                                               \
typedef HIDDEN_ ## type ## Ptr * type ## Handle

#define declare_record_subtypes(type)					\
typedef struct type type;						\
typedef type *type ## Ptr;						\
MAKE_HIDDEN(type ## Ptr);                                               \
typedef HIDDEN_ ## type ## Ptr * type ## Handle

typedef int32 TimeStamp;
typedef Handle EditionRefNum;
typedef int16 UpdateMode;
typedef SignedByte SectionType;
typedef char FormatType[4];

typedef ProcPtr ExpDialogHookProcPtr;
typedef ProcPtr ExpModalFilterProcPtr;
typedef ProcPtr FormatIOProcPtr;
typedef ProcPtr EditionOpenerProcPtr;

struct PACKED SectionRecord
{
  SignedByte version;
  SectionType kind;
  UpdateMode mode;
  TimeStamp mdDate;
  int32 sectionID;
  int32 refCon;
  PACKED_MEMBER(AliasHandle, alias);

  int32 subPart;
  PACKED_MEMBER(/* ### Section */ Handle, nextSection);
  PACKED_MEMBER(Handle, controlBlock);
  EditionRefNum refNum;
};

declare_record_subtypes (Section);

struct PACKED EditionContainerSpec
{
  FSSpec theFile;
  ScriptCode theFileScript;
  int32 thePart;
  Str31 thePartName;
  ScriptCode thePartScript;
};

typedef struct EditionContainerSpec EditionContainerSpec;
typedef EditionContainerSpec *EditionContainerSpecPtr;

struct PACKED EditionInfoRecord
{
  TimeStamp crDate;
  TimeStamp mdDate;
  OSType fdCreator;
  OSType fdType;
  EditionContainerSpec container;
};

typedef struct EditionInfoRecord EditionInfoRecord;
typedef EditionInfoRecord *EditionInfoPtr;

struct PACKED NewPublisherReply
{
  Boolean canceled;
  Boolean replacing;
  Boolean usePart;
  uint8 _filler;
  PACKED_MEMBER(Handle, preview);
  FormatType previewFormat;
  EditionContainerSpec container;
};

typedef struct NewPublisherReply NewPublisherReply;
typedef NewPublisherReply *NewPublisherReplyPtr;

struct PACKED NewSubscriberReply
{
  Boolean canceled;
  SignedByte formatsMask;
  EditionContainerSpec container;
};

typedef struct NewSubscriberReply NewSubscriberReply;
typedef NewSubscriberReply *NewSubscriberReplyPtr;

struct PACKED SectionOptionsReply
{
  Boolean canceled;
  Boolean changed;
  PACKED_MEMBER(SectionHandle, sectionH);
  ResType action;
};

typedef struct SectionOptionsReply SectionOptionsReply;
typedef SectionOptionsReply *SectionOptionsReplyPtr;

typedef uint8 EditionOpenerVerb;

#define eoOpen		(0)
#define eoClose		(1)
#define eoOpenNew	(2)
#define eoCloseNew	(3)
#define eoCanSubscribe	(4)

struct PACKED EditionOpenerParamBlock
{
  EditionInfoRecord info;
  PACKED_MEMBER(SectionHandle, sectionH);
  PACKED_MEMBER(FSSpecPtr, document);
  OSType fdCreator;
  int32 ioRefNum;
  PACKED_MEMBER(FormatIOProcPtr, ioProc);
  Boolean success;
  SignedByte formatsMask;
};

typedef struct EditionOpenerParamBlock EditionOpenerParamBlock;
typedef EditionOpenerParamBlock *EditionOpenerParamBlockPtr;

typedef uint8 FormatIOVerb;

#define ioHasFormat	(0)
#define ioReadFormat	(1)
#define ioNewFormat	(2)
#define ioWtriteFormat	(3)

struct PACKED FormatIOParamBlock
{
  int32 ioRefNum;
  FormatType format;
  int32 formatIndex;
  int32 offset;
  PACKED_MEMBER(Ptr, buffPtr);
  int32 buffLen;
};

typedef struct FormatIOParamBlock FormatIOParamBlock;
typedef FormatIOParamBlock *FormatIOParamBlockPtr;

#define noErr			0
#define abortErr		(-27)
#define eofErr			(-39)
#define fnfErr			(-43)
#define flLckedErr		(-45)
#define fBusyErr		(-47)
#define paramErr		(-50)
#define rfNumErr		(-51)
#define permErr			(-54)
#define wrPermErr		(-61)
#define noTypeErr		(-102)
#define memFullErr		(-108)
#define userCanceledErr		(-128)
#define editionMgrInitErr	(-450)
#define badSectionErr		(-451)
#define notRegisteredSectionErr	(-452)
#define badSubPartErr		(-454)
#define multiplePubliserWrn	(-460)
#define containerNotFoundWrn	(-461)
#define notThePublisherWrn	(-463)

extern pascal trap OSErr C_InitEditionPack (INTEGER unused);
extern pascal trap OSErr C_NewSection (EditionContainerSpecPtr container,
				       FSSpecPtr section_doc,
				       SectionType kind,  int32 section_id,
				       UpdateMode initial_mode,
				       SectionHandle *section_out);
extern pascal trap OSErr C_RegisterSection (FSSpecPtr section_doc,
					    SectionHandle section,
					    Boolean *alias_was_updated_p_out);
extern pascal trap OSErr C_UnRegisterSection (SectionHandle section);
extern pascal trap OSErr C_IsRegisteredSection (SectionHandle section);
extern pascal trap OSErr C_AssociateSection (SectionHandle section,
					     FSSpecPtr new_section_doc);
extern pascal trap OSErr C_CreateEditionContainerFile
  (FSSpecPtr edition_file, OSType creator,
   ScriptCode edition_file_name_script);
extern pascal trap OSErr C_DeleteEditionContainerFile (FSSpecPtr edition_file);

extern pascal trap OSErr C_SetEditionFormatMark (EditionRefNum edition,
						 FormatType format,
						 int32 mark);

extern pascal trap OSErr C_GetEditionFormatMark (EditionRefNum edition,
						 FormatType format,
						 int32 *currentMark);

extern pascal trap OSErr C_OpenEdition (SectionHandle subscriber_section,
					EditionRefNum *ref_num);

extern pascal trap OSErr C_EditionHasFormat (EditionRefNum edition,
					     FormatType format,
					     Size *format_size);
extern pascal trap OSErr C_ReadEdition (EditionRefNum edition,
					FormatType format,
					Ptr buffer, Size buffer_size);

extern pascal trap OSErr C_OpenNewEdition (SectionHandle publisher_section,
					   OSType creator,
					   FSSpecPtr publisher_section_doc,
					   EditionRefNum *ref_num);

extern pascal trap OSErr C_WriteEdition (EditionRefNum edition, FormatType format,
					 Ptr buffer, Size buffer_size);

extern pascal trap OSErr C_CloseEdition (EditionRefNum edition, Boolean success_p);

extern pascal trap OSErr C_GetLastEditionContainerUsed
  (EditionContainerSpecPtr container);

extern pascal trap OSErr C_NewSubscriberDialog (NewSubscriberReplyPtr reply);
extern pascal trap OSErr C_NewPublisherDialog (NewSubscriberReplyPtr reply);
extern pascal trap OSErr C_SectionOptionsDialog (SectionOptionsReply *reply);

extern pascal trap OSErr C_NewSubscriberExpDialog
  (NewSubscriberReplyPtr reply, Point where,
   int16 expnasion_ditl_res_id,
   ExpDialogHookProcPtr dialog_hook,
   ExpModalFilterProcPtr filter_hook,
   Ptr data);

extern pascal trap OSErr C_NewPublisherExpDialog
  (NewPublisherReplyPtr reply, Point where,
   int16 expnasion_ditl_res_id,
   ExpDialogHookProcPtr dialog_hook,
   ExpModalFilterProcPtr filter_hook,
   Ptr data);

extern pascal trap OSErr C_SectionOptionsExpDialog
  (SectionOptionsReply *reply,
   Point where, int16 expnasion_ditl_res_id,
   ExpDialogHookProcPtr dialog_hook,
   ExpModalFilterProcPtr filter_hook,
   Ptr data);

extern pascal trap OSErr C_GetEditionInfo (SectionHandle section,
					   EditionInfoPtr edition_info);

extern pascal trap OSErr C_GoToPublisherSection
  (EditionContainerSpecPtr container);

extern pascal trap OSErr C_GetStandardFormats (EditionContainerSpecPtr container,
					       FormatType *preview_format,
					       Handle preview,
					       Handle publisher_alias,
					       Handle formats);
extern pascal trap OSErr C_GetEditionOpenerProc (EditionOpenerProcPtr *opener);
extern pascal trap OSErr C_SetEditionOpenerProc (EditionOpenerProcPtr opener);

extern pascal trap OSErr C_CallEditionOpenerProc
  (EditionOpenerVerb selector,
   EditionOpenerParamBlock *param_block,
   EditionOpenerProcPtr opener);

extern pascal trap OSErr C_CallFormatIOProc
  (FormatIOVerb selector,
   FormatIOParamBlock *param_block,
   FormatIOProcPtr proc);
}

#endif /* _EDITIONMGR_H_ */
