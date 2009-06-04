
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

#define declare_subtypes(type)						\
typedef struct type ## Record type ## Record;				\
typedef type ## Record *type ## Ptr;					\
typedef struct { type ## Ptr p PACKED_P; } HIDDEN_ ## type ## Ptr;	\
typedef HIDDEN_ ## type ## Ptr * type ## Handle

#define declare_record_subtypes(type)					\
typedef struct type type;						\
typedef type *type ## Ptr;						\
typedef struct { type ## Ptr p PACKED_P; } HIDDEN_ ## type ## Ptr;	\
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

struct SectionRecord
{
  SignedByte version			LPACKED;
  SectionType kind			LPACKED;
  UpdateMode mode			PACKED;
  TimeStamp mdDate			PACKED;
  int32 sectionID			PACKED;
  int32 refCon				PACKED;
  AliasHandle alias			PACKED_P;

  int32 subPart				PACKED;
  /* ### Section */ Handle nextSection	PACKED_P;
  Handle controlBlock			PACKED_P;
  EditionRefNum refNum			PACKED;
};

declare_record_subtypes (Section);

struct EditionContainerSpec
{
  FSSpec theFile			LPACKED;
  ScriptCode theFileScript		PACKED;
  int32 thePart				PACKED;
  Str31 thePartName			LPACKED;
  ScriptCode thePartScript		PACKED;
};

typedef struct EditionContainerSpec EditionContainerSpec;
typedef EditionContainerSpec *EditionContainerSpecPtr;

struct EditionInfoRecord
{
  TimeStamp crDate			PACKED;
  TimeStamp mdDate			PACKED;
  OSType fdCreator			PACKED;
  OSType fdType				PACKED;
  EditionContainerSpec container	LPACKED;
};

typedef struct EditionInfoRecord EditionInfoRecord;
typedef EditionInfoRecord *EditionInfoPtr;

struct NewPublisherReply
{
  Boolean canceled			LPACKED;
  Boolean replacing			LPACKED;
  Boolean usePart			LPACKED;
  uint8 _filler;
  Handle preview			PACKED_P;
  FormatType previewFormat		LPACKED;
  EditionContainerSpec container	LPACKED;
};

typedef struct NewPublisherReply NewPublisherReply;
typedef NewPublisherReply *NewPublisherReplyPtr;

struct NewSubscriberReply
{
  Boolean canceled		       	LPACKED;
  SignedByte formatsMask		LPACKED;
  EditionContainerSpec container	LPACKED;
};

typedef struct NewSubscriberReply NewSubscriberReply;
typedef NewSubscriberReply *NewSubscriberReplyPtr;

struct SectionOptionsReply
{
  Boolean canceled			LPACKED;
  Boolean changed			LPACKED;
  SectionHandle sectionH		PACKED_P;
  ResType action			PACKED;
};

typedef struct SectionOptionsReply SectionOptionsReply;
typedef SectionOptionsReply *SectionOptionsReplyPtr;

typedef uint8 EditionOpenerVerb;

#define eoOpen		(0)
#define eoClose		(1)
#define eoOpenNew	(2)
#define eoCloseNew	(3)
#define eoCanSubscribe	(4)

struct EditionOpenerParamBlock
{
  EditionInfoRecord info		LPACKED;
  SectionHandle sectionH		PACKED_P;
  FSSpecPtr document			PACKED_P;
  OSType fdCreator			PACKED;
  int32 ioRefNum			PACKED;
  FormatIOProcPtr ioProc		PACKED_P;
  Boolean success			LPACKED;
  SignedByte formatsMask		LPACKED;
};

typedef struct EditionOpenerParamBlock EditionOpenerParamBlock;
typedef EditionOpenerParamBlock *EditionOpenerParamBlockPtr;

typedef uint8 FormatIOVerb;

#define ioHasFormat	(0)
#define ioReadFormat	(1)
#define ioNewFormat	(2)
#define ioWtriteFormat	(3)

struct FormatIOParamBlock
{
  int32 ioRefNum		PACKED;
  FormatType format		LPACKED;
  int32 formatIndex		PACKED;
  int32 offset			PACKED;
  Ptr buffPtr			PACKED_P;
  int32 buffLen			PACKED;
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

#endif /* _EDITIONMGR_H_ */
