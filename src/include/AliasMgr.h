
#if !defined (_ALIASMGR_H_)
#define _ALIASMGR_H_

#include "FileMgr.h"

namespace Executor {
typedef ProcPtr AliasFilterProcPtr;
typedef Handle AliasHandle;
typedef int16 AliasTypeInfo;

enum
{
  kSystemFolderType		= T('m', 'a', 'c', 's'),
  kDesktopFolderType		= T('d', 'e', 's', 'k'),
  kTrashFolderType		= T('t', 'r', 's', 'h'),
  kWhereToEmptyTrashFolderType	= T('e', 'm', 'p', 't'),
  kPrintMonitorDocsFolderType	= T('p', 'r', 'n', 't'),
  kStartupFolderType		= T('s', 't', 'r', 't'),
  kAppleMenuFolderType		= T('a', 'm', 'n', 'u'),
  kControlPanelFolderType	= T('c', 't', 'r', 'l'),
  kExtensionFolderType		= T('e', 'x', 't', 'n'),
  kPreferencesFolderType	= T('p', 'r', 'e', 'f'),
  kTemporaryFolderType		= T('t', 'e', 'm', 'p'),
  kFontFolderType		= T('f', 'o', 'n', 't'),
};

extern pascal trap Executor::OSErr C_FindFolder (int16 vRefNum, Executor::OSType folderType,
				       Executor::Boolean createFolder,
				       int16 *foundVRefNum,
				       int32 *foundDirID);

extern pascal trap Executor::OSErr C_NewAlias (Executor::FSSpecPtr fromFile, Executor::FSSpecPtr target,
				     Executor::AliasHandle *alias);

extern pascal trap Executor::OSErr C_NewAliasMinimal (Executor::FSSpecPtr target,
					    Executor::AliasHandle *alias);

extern pascal trap Executor::OSErr C_NewAliasMinimalFromFullPath
  (int16 fullpathLength, Executor::Ptr fullpath,
   Executor::Str32 zoneName, Executor::Str31 serverName, Executor::AliasHandle *alias);

extern pascal trap Executor::OSErr C_UpdateAlias (Executor::FSSpecPtr fromFile, Executor::FSSpecPtr target,
					Executor::AliasHandle alias,
					Executor::Boolean *wasChanged);

extern pascal trap Executor::OSErr C_ResolveAlias (Executor::FSSpecPtr fromFile,
					 Executor::AliasHandle alias,
					 Executor::FSSpecPtr target,
					 Executor::Boolean *wasAliased);

extern pascal trap Executor::OSErr C_ResolveAliasFile (Executor::FSSpecPtr theSpec,
					     Executor::Boolean resolveAliasChains,
					     Executor::Boolean *targetIsFolder,
					     Executor::Boolean *wasAliased);

extern pascal trap Executor::OSErr C_MatchAlias (Executor::FSSpecPtr fromFile, int32 rulesMask,
				       Executor::AliasHandle alias, int16 *aliasCount,
				       Executor::FSSpecArrayPtr aliasList,
				       Executor::Boolean *needsUpdate,
				       Executor::AliasFilterProcPtr aliasFilter,
				       Executor::Ptr yourDataPtr);
extern pascal trap Executor::OSErr C_GetAliasInfo (Executor::AliasHandle alias,
					 Executor::AliasTypeInfo index,
					 Executor::Str63 theString);
}

#endif
