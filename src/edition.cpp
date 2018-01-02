/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "EditionMgr.h"

using namespace Executor;

PUBLIC pascal trap OSErr Executor::C_InitEditionPack(INTEGER unused)
{
    warning_unimplemented("someone is calling the edition manager");
    return noErr;
}

PUBLIC pascal trap OSErr Executor::C_NewSection(EditionContainerSpecPtr container, FSSpecPtr section_doc, SectionType kind, int32_t section_id, UpdateMode initial_mode, SectionHandle *section_out)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_RegisterSection(FSSpecPtr section_doc, SectionHandle section, Boolean *alias_was_updated_p_out)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_UnRegisterSection(SectionHandle section)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_IsRegisteredSection(SectionHandle section)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_AssociateSection(SectionHandle section, FSSpecPtr new_section_doc)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_CreateEditionContainerFile(FSSpecPtr edition_file, OSType creator, ScriptCode edition_file_name_script)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_DeleteEditionContainerFile(FSSpecPtr edition_file)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_SetEditionFormatMark(EditionRefNum edition, FormatType format, int32_t mark)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_GetEditionFormatMark(EditionRefNum edition, FormatType format, int32_t *currentMark)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_OpenEdition(SectionHandle subscriber_section, EditionRefNum *ref_num)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_EditionHasFormat(EditionRefNum edition, FormatType format, Size *format_size)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_ReadEdition(EditionRefNum edition, FormatType format, Ptr buffer, Size buffer_size)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_OpenNewEdition(SectionHandle publisher_section, OSType creator, FSSpecPtr publisher_section_doc, EditionRefNum *ref_num)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_WriteEdition(EditionRefNum edition, FormatType format, Ptr buffer, Size buffer_size)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_CloseEdition(EditionRefNum edition, Boolean success_p)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_GetLastEditionContainerUsed(EditionContainerSpecPtr container)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_NewSubscriberDialog(NewSubscriberReplyPtr reply)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_NewPublisherDialog(NewSubscriberReplyPtr reply)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_SectionOptionsDialog(SectionOptionsReply *reply)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_NewSubscriberExpDialog(NewSubscriberReplyPtr reply, Point where, int16_t expnasion_ditl_res_id, ExpDialogHookProcPtr dialog_hook, ExpModalFilterProcPtr filter_hook, Ptr data)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_NewPublisherExpDialog(NewPublisherReplyPtr reply, Point where, int16_t expnasion_ditl_res_id, ExpDialogHookProcPtr dialog_hook, ExpModalFilterProcPtr filter_hook, Ptr data)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_SectionOptionsExpDialog(SectionOptionsReply *reply, Point where, int16_t expnasion_ditl_res_id, ExpDialogHookProcPtr dialog_hook, ExpModalFilterProcPtr filter_hook, Ptr data)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_GetEditionInfo(SectionHandle section, EditionInfoPtr edition_info)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_GoToPublisherSection(EditionContainerSpecPtr container)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_GetStandardFormats(EditionContainerSpecPtr container, FormatType *preview_format, Handle preview, Handle publisher_alias, Handle formats)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_GetEditionOpenerProc(EditionOpenerProcPtr *opener)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_SetEditionOpenerProc(EditionOpenerProcPtr opener)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_CallEditionOpenerProc(EditionOpenerVerb selector, EditionOpenerParamBlock *param_block, EditionOpenerProcPtr opener)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

PUBLIC pascal trap OSErr Executor::C_CallFormatIOProc(FormatIOVerb selector, FormatIOParamBlock *param_block, FormatIOProcPtr proc)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}
