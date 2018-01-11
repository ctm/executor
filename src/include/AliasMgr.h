
#if !defined(_ALIASMGR_H_)
#define _ALIASMGR_H_

#include "FileMgr.h"

namespace Executor
{
typedef ProcPtr AliasFilterProcPtr;
typedef Handle AliasHandle;
typedef int16_t AliasTypeInfo;

enum
{
    kSystemFolderType = FOURCC('m', 'a', 'c', 's'),
    kDesktopFolderType = FOURCC('d', 'e', 's', 'k'),
    kTrashFolderType = FOURCC('t', 'r', 's', 'h'),
    kWhereToEmptyTrashFolderType = FOURCC('e', 'm', 'p', 't'),
    kPrintMonitorDocsFolderType = FOURCC('p', 'r', 'n', 't'),
    kStartupFolderType = FOURCC('s', 't', 'r', 't'),
    kAppleMenuFolderType = FOURCC('a', 'm', 'n', 'u'),
    kControlPanelFolderType = FOURCC('c', 't', 'r', 'l'),
    kExtensionFolderType = FOURCC('e', 'x', 't', 'n'),
    kPreferencesFolderType = FOURCC('p', 'r', 'e', 'f'),
    kTemporaryFolderType = FOURCC('t', 'e', 'm', 'p'),
    kFontFolderType = FOURCC('f', 'o', 'n', 't'),
};

extern OSErr C_FindFolder(int16_t vRefNum, OSType folderType,
                                      Boolean createFolder,
                                      GUEST<int16_t> *foundVRefNum,
                                      GUEST<int32_t> *foundDirID);
PASCAL_FUNCTION(FindFolder);

extern OSErr C_NewAlias(FSSpecPtr fromFile, FSSpecPtr target,
                                    GUEST<AliasHandle> *alias);
PASCAL_FUNCTION(NewAlias);

extern OSErr C_NewAliasMinimal(FSSpecPtr target,
                                           GUEST<AliasHandle> *alias);
PASCAL_FUNCTION(NewAliasMinimal);

extern OSErr C_NewAliasMinimalFromFullPath(int16_t fullpathLength, Ptr fullpath,
                                                       Str32 zoneName, Str31 serverName, GUEST<AliasHandle> *alias);
PASCAL_FUNCTION(NewAliasMinimalFromFullPath);

extern OSErr C_UpdateAlias(FSSpecPtr fromFile, FSSpecPtr target,
                                       AliasHandle alias,
                                       Boolean *wasChanged);
PASCAL_FUNCTION(UpdateAlias);

extern OSErr C_ResolveAlias(FSSpecPtr fromFile,
                                        AliasHandle alias,
                                        FSSpecPtr target,
                                        Boolean *wasAliased);
PASCAL_FUNCTION(ResolveAlias);

extern OSErr C_ResolveAliasFile(FSSpecPtr theSpec,
                                            Boolean resolveAliasChains,
                                            Boolean *targetIsFolder,
                                            Boolean *wasAliased);
PASCAL_FUNCTION(ResolveAliasFile);

extern OSErr C_MatchAlias(FSSpecPtr fromFile, int32_t rulesMask,
                                      AliasHandle alias, int16_t *aliasCount,
                                      FSSpecArrayPtr aliasList,
                                      Boolean *needsUpdate,
                                      AliasFilterProcPtr aliasFilter,
                                      Ptr yourDataPtr);
PASCAL_FUNCTION(MatchAlias);
extern OSErr C_GetAliasInfo(AliasHandle alias,
                                        AliasTypeInfo index,
                                        Str63 theString);
PASCAL_FUNCTION(GetAliasInfo);
}

#endif
