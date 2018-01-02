
#if !defined(_RESOURCE_H_)
#define _RESOURCE_H_

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
/* resource attribute masks */
enum
{
    resSysHeap = 64,
    resPurgeable = 32,
    resLocked = 16,
    resProtected = 8,
    resPreload = 4,
    resChanged = 2,
};
enum
{
    resCompressed = 1
};

/* resource manager return codes */

enum
{
    CantDecompress = -186
};

enum
{
    resNotFound = (-192),
    resFNotFound = (-193),
    addResFailed = (-194),
    rmvResFailed = (-196),
};

/* IMIV */
enum
{
    resAttrErr = (-198),
    mapReadErr = (-199),
};

/* IMVI */

enum
{
    resourceInMemory = -188,
    inputOutOfBounds = -190
};

/* resource file attribute masks */
enum
{
    mapReadOnly = 128,
    mapCompact = 64,
    mapChanged = 32,
};

#if 0
#if !defined(TopMapHndl_H)
extern GUEST<Handle> 	TopMapHndl_H;
extern GUEST<Handle> 	SysMapHndl_H;
extern GUEST<ProcPtr> 	ResErrProc_H;
extern INTEGER 	SysMap;
extern INTEGER 	CurMap;
extern BOOLEAN 	ResLoad;
extern INTEGER 	ResErr;
extern Byte 	SysResName[20];
#endif

enum
{
    TopMapHndl = (TopMapHndl_H.p),
    SysMapHndl = (SysMapHndl_H.p),
    ResErrProc = (ResErrProc_H.p),
};
#endif

extern BOOLEAN ROMlib_InstallxDEF(ProcPtr thedef, ResType typ,
                                  INTEGER id);
extern pascal trap void C_SetResLoad(BOOLEAN load);
PASCAL_TRAP(SetResLoad, 0xA99B);

extern pascal trap INTEGER C_CountResources(ResType typ);
PASCAL_TRAP(CountResources, 0xA99C);

extern pascal trap INTEGER C_Count1Resources(
    ResType typ);
PASCAL_TRAP(Count1Resources, 0xA80D);
extern pascal trap Handle C_GetIndResource(ResType typ, INTEGER indx);
PASCAL_TRAP(GetIndResource, 0xA99D);

extern pascal trap Handle C_Get1IndResource(ResType typ,
                                            INTEGER i);
PASCAL_TRAP(Get1IndResource, 0xA80E);
extern pascal trap Handle C_GetResource(ResType typ, INTEGER id);
PASCAL_FUNCTION(GetResource);

extern pascal trap Handle C_Get1Resource(ResType typ,
                                         INTEGER id);
PASCAL_TRAP(Get1Resource, 0xA81F);
extern pascal trap Handle C_GetNamedResource(ResType typ, StringPtr nam);
PASCAL_TRAP(GetNamedResource, 0xA9A1);

extern pascal trap Handle C_Get1NamedResource(ResType typ,
                                              StringPtr s);
PASCAL_TRAP(Get1NamedResource, 0xA820);
extern pascal trap void C_LoadResource(Handle volatile res);
PASCAL_TRAP(LoadResource, 0xA9A2);

extern pascal trap void C_ReleaseResource(Handle res);
PASCAL_TRAP(ReleaseResource, 0xA9A3);

extern pascal trap void C_DetachResource(Handle res);
PASCAL_TRAP(DetachResource, 0xA992);

extern pascal trap INTEGER C_UniqueID(ResType typ);
PASCAL_TRAP(UniqueID, 0xA9C1);

extern pascal trap INTEGER C_Unique1ID(ResType typ);
PASCAL_TRAP(Unique1ID, 0xA810);

extern pascal trap void C_GetResInfo(Handle res, GUEST<INTEGER> *id1,
                                     GUEST<ResType> *typ, StringPtr name);
PASCAL_TRAP(GetResInfo, 0xA9A8);
extern pascal trap INTEGER C_GetResAttrs(Handle res);
PASCAL_TRAP(GetResAttrs, 0xA9A6);

extern LONGINT ROMlib_SizeResource(Handle res, BOOLEAN usehandle);
extern pascal trap LONGINT C_SizeResource(Handle res);
PASCAL_TRAP(SizeResource, 0xA9A5);

extern pascal trap INTEGER C_CountTypes(void);
PASCAL_TRAP(CountTypes, 0xA99E);

extern pascal trap INTEGER C_Count1Types(void);
PASCAL_TRAP(Count1Types, 0xA81C);

extern pascal trap void C_GetIndType(GUEST<ResType> *typ, INTEGER indx);
PASCAL_TRAP(GetIndType, 0xA99F);

extern pascal trap void C_Get1IndType(GUEST<ResType> *typ,
                                      INTEGER indx);
PASCAL_TRAP(Get1IndType, 0xA80F);
extern pascal trap LONGINT C_MaxSizeRsrc(Handle h);
PASCAL_TRAP(MaxSizeRsrc, 0xA821);

extern pascal trap LONGINT C_RsrcMapEntry(Handle h);
PASCAL_TRAP(RsrcMapEntry, 0xA9C5);

extern pascal trap Handle C_RGetResource(ResType typ, INTEGER id);
PASCAL_TRAP(RGetResource, 0xA80C);

extern pascal trap INTEGER C_InitResources(void);
PASCAL_TRAP(InitResources, 0xA995);

extern pascal trap void C_RsrcZoneInit(void);
PASCAL_TRAP(RsrcZoneInit, 0xA996);

extern pascal trap INTEGER C_ResError(void);
PASCAL_TRAP(ResError, 0xA9AF);

extern pascal trap INTEGER C_GetResFileAttrs(INTEGER rn);
PASCAL_TRAP(GetResFileAttrs, 0xA9F6);

extern pascal trap void C_SetResFileAttrs(INTEGER rn, INTEGER attrs);
PASCAL_TRAP(SetResFileAttrs, 0xA9F7);

extern pascal trap void C_SetResInfo(Handle res, INTEGER id,
                                     StringPtr name);
PASCAL_TRAP(SetResInfo, 0xA9A9);
extern pascal trap void C_SetResAttrs(Handle res, INTEGER attrs);
PASCAL_TRAP(SetResAttrs, 0xA9A7);

extern pascal trap void C_ChangedResource(Handle res);
PASCAL_TRAP(ChangedResource, 0xA9AA);

extern pascal trap void C_AddResource(Handle data, ResType typ,
                                      INTEGER id, StringPtr name);
PASCAL_TRAP(AddResource, 0xA9AB);
extern pascal trap void C_RmveResource(Handle res);
PASCAL_TRAP(RmveResource, 0xA9AD);

extern pascal trap void C_UpdateResFile(INTEGER rn);
PASCAL_TRAP(UpdateResFile, 0xA999);

extern pascal trap void C_WriteResource(Handle res);
PASCAL_TRAP(WriteResource, 0xA9B0);

extern pascal trap void C_SetResPurge(BOOLEAN install);
PASCAL_TRAP(SetResPurge, 0xA993);

extern pascal trap void C_CreateResFile(StringPtr fn);
PASCAL_TRAP(CreateResFile, 0xA9B1);

extern pascal trap INTEGER C_OpenRFPerm(StringPtr fn,
                                        INTEGER vref, Byte perm);
PASCAL_TRAP(OpenRFPerm, 0xA9C4);
extern pascal trap INTEGER C_OpenResFile(StringPtr fn);
PASCAL_TRAP(OpenResFile, 0xA997);

extern pascal trap void C_CloseResFile(INTEGER rn);
PASCAL_TRAP(CloseResFile, 0xA99A);

extern pascal trap INTEGER C_CurResFile(void);
PASCAL_TRAP(CurResFile, 0xA994);

extern pascal trap INTEGER C_HomeResFile(Handle res);
PASCAL_TRAP(HomeResFile, 0xA9A4);

extern pascal trap void C_UseResFile(INTEGER rn);
PASCAL_TRAP(UseResFile, 0xA998);

extern pascal trap void C_ReadPartialResource(Handle resource,
                                              int32_t offset,
                                              Ptr buffer, int32_t count);
PASCAL_FUNCTION(ReadPartialResource);
extern pascal trap void C_WritePartialResource(Handle resource,
                                               int32_t offset,
                                               Ptr buffer, int32_t count);
PASCAL_FUNCTION(WritePartialResource);
extern pascal trap void C_SetResourceSize(Handle resource, int32_t size);
PASCAL_FUNCTION(SetResourceSize);

extern pascal Handle C_GetNextFOND(Handle fondHandle);
}

#endif /* _RESOURCE_H_ */
