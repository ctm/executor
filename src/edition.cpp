/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "EditionMgr.h"

using namespace Executor;

P1(PUBLIC pascal trap, OSErr, InitEditionPack, INTEGER, unused)
{
    warning_unimplemented("someone is calling the edition manager");
    return noErr;
}

P6(PUBLIC pascal trap, OSErr, NewSection,
   EditionContainerSpecPtr, container,
   FSSpecPtr, section_doc, SectionType, kind, int32_t, section_id,
   UpdateMode, initial_mode, SectionHandle *, section_out)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P3(PUBLIC pascal trap, OSErr, RegisterSection,
   FSSpecPtr, section_doc, SectionHandle, section,
   Boolean *, alias_was_updated_p_out)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P1(PUBLIC pascal trap, OSErr, UnRegisterSection,
   SectionHandle, section)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P1(PUBLIC pascal trap, OSErr, IsRegisteredSection,
   SectionHandle, section)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P2(PUBLIC pascal trap, OSErr, AssociateSection,
   SectionHandle, section, FSSpecPtr, new_section_doc)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P3(PUBLIC pascal trap, OSErr, CreateEditionContainerFile,
   FSSpecPtr, edition_file,
   OSType, creator, ScriptCode, edition_file_name_script)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P1(PUBLIC pascal trap, OSErr, DeleteEditionContainerFile,
   FSSpecPtr, edition_file)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P3(PUBLIC pascal trap, OSErr, SetEditionFormatMark,
   EditionRefNum, edition, FormatType, format,
   int32_t, mark)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P3(PUBLIC pascal trap, OSErr, GetEditionFormatMark,
   EditionRefNum, edition, FormatType, format,
   int32_t *, currentMark)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P2(PUBLIC pascal trap, OSErr, OpenEdition,
   SectionHandle, subscriber_section,
   EditionRefNum *, ref_num)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P3(PUBLIC pascal trap, OSErr, EditionHasFormat,
   EditionRefNum, edition, FormatType, format,
   Size *, format_size)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P4(PUBLIC pascal trap, OSErr, ReadEdition,
   EditionRefNum, edition, FormatType, format,
   Ptr, buffer, Size, buffer_size)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P4(PUBLIC pascal trap, OSErr, OpenNewEdition,
   SectionHandle, publisher_section, OSType, creator,
   FSSpecPtr, publisher_section_doc, EditionRefNum *, ref_num)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P4(PUBLIC pascal trap, OSErr, WriteEdition,
   EditionRefNum, edition, FormatType, format,
   Ptr, buffer, Size, buffer_size)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P2(PUBLIC pascal trap, OSErr, CloseEdition,
   EditionRefNum, edition, Boolean, success_p)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P1(PUBLIC pascal trap, OSErr, GetLastEditionContainerUsed,
   EditionContainerSpecPtr, container)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P1(PUBLIC pascal trap, OSErr, NewSubscriberDialog,
   NewSubscriberReplyPtr, reply)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P1(PUBLIC pascal trap, OSErr, NewPublisherDialog,
   NewSubscriberReplyPtr, reply)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P1(PUBLIC pascal trap, OSErr, SectionOptionsDialog,
   SectionOptionsReply *, reply)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P6(PUBLIC pascal trap, OSErr, NewSubscriberExpDialog,
   NewSubscriberReplyPtr, reply,
   Point, where, int16_t, expnasion_ditl_res_id,
   ExpDialogHookProcPtr, dialog_hook,
   ExpModalFilterProcPtr, filter_hook,
   Ptr, data)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P6(PUBLIC pascal trap, OSErr, NewPublisherExpDialog,
   NewPublisherReplyPtr, reply,
   Point, where, int16_t, expnasion_ditl_res_id,
   ExpDialogHookProcPtr, dialog_hook,
   ExpModalFilterProcPtr, filter_hook,
   Ptr, data)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P6(PUBLIC pascal trap, OSErr, SectionOptionsExpDialog,
   SectionOptionsReply *, reply,
   Point, where, int16_t, expnasion_ditl_res_id,
   ExpDialogHookProcPtr, dialog_hook,
   ExpModalFilterProcPtr, filter_hook,
   Ptr, data)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P2(PUBLIC pascal trap, OSErr, GetEditionInfo,
   SectionHandle, section, EditionInfoPtr, edition_info)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P1(PUBLIC pascal trap, OSErr, GoToPublisherSection,
   EditionContainerSpecPtr, container)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P5(PUBLIC pascal trap, OSErr, GetStandardFormats,
   EditionContainerSpecPtr, container,
   FormatType *, preview_format,
   Handle, preview, Handle, publisher_alias, Handle, formats)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P1(PUBLIC pascal trap, OSErr, GetEditionOpenerProc,
   EditionOpenerProcPtr *, opener)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P1(PUBLIC pascal trap, OSErr, SetEditionOpenerProc,
   EditionOpenerProcPtr, opener)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P3(PUBLIC pascal trap, OSErr, CallEditionOpenerProc,
   EditionOpenerVerb, selector,
   EditionOpenerParamBlock *, param_block,
   EditionOpenerProcPtr, opener)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

P3(PUBLIC pascal trap, OSErr, CallFormatIOProc,
   FormatIOVerb, selector,
   FormatIOParamBlock *, param_block,
   FormatIOProcPtr, proc)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}
