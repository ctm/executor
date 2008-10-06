
#if !defined (_ALIASMGR_H_)
#define _ALIASMGR_H_

#include "FileMgr.h"

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

extern pascal trap OSErr C_FindFolder (int16 vRefNum, OSType folderType,
				       Boolean createFolder,
				       int16 *foundVRefNum,
				       int32 *foundDirID);

extern pascal trap OSErr C_NewAlias (FSSpecPtr fromFile, FSSpecPtr target,
				     AliasHandle *alias);

extern pascal trap OSErr C_NewAliasMinimal (FSSpecPtr target,
					    AliasHandle *alias);

extern pascal trap OSErr C_NewAliasMinimalFromFullPath
  (int16 fullpathLength, Ptr fullpath,
   Str32 zoneName, Str31 serverName, AliasHandle *alias);

extern pascal trap OSErr C_UpdateAlias (FSSpecPtr fromFile, FSSpecPtr target,
					AliasHandle alias,
					Boolean *wasChanged);

extern pascal trap OSErr C_ResolveAlias (FSSpecPtr fromFile,
					 AliasHandle alias,
					 FSSpecPtr target,
					 Boolean *wasAliased);

extern pascal trap OSErr C_ResolveAliasFile (FSSpecPtr theSpec,
					     Boolean resolveAliasChains,
					     Boolean *targetIsFolder,
					     Boolean *wasAliased);

extern pascal trap OSErr C_MatchAlias (FSSpecPtr fromFile, int32 rulesMask,
				       AliasHandle alias, int16 *aliasCount,
				       FSSpecArrayPtr aliasList,
				       Boolean *needsUpdate,
				       AliasFilterProcPtr aliasFilter,
				       Ptr yourDataPtr);
extern pascal trap OSErr C_GetAliasInfo (AliasHandle alias,
					 AliasTypeInfo index,
					 Str63 theString);

#endif
