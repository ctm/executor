
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
#define resSysHeap 64
#define resPurgeable 32
#define resLocked 16
#define resProtected 8
#define resPreload 4
#define resChanged 2
enum
{
    resCompressed = 1
};

/* resource manager return codes */

enum
{
    CantDecompress = -186
};

#define resNotFound (-192)
#define resFNotFound (-193)
#define addResFailed (-194)
#define rmvResFailed (-196)

/* IMIV */
#define resAttrErr (-198)
#define mapReadErr (-199)

/* IMVI */

enum
{
    resourceInMemory = -188,
    inputOutOfBounds = -190
};

/* resource file attribute masks */
#define mapReadOnly 128
#define mapCompact 64
#define mapChanged 32

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

#define TopMapHndl (TopMapHndl_H.p)
#define SysMapHndl (SysMapHndl_H.p)
#define ResErrProc (ResErrProc_H.p)
#endif

extern BOOLEAN ROMlib_InstallxDEF(ProcPtr thedef, ResType typ,
                                  INTEGER id);
extern pascal trap void C_SetResLoad(BOOLEAN load);
extern pascal trap void P_SetResLoad(BOOLEAN load);
extern pascal trap INTEGER C_CountResources(ResType typ);
extern pascal trap INTEGER P_CountResources(ResType typ);
extern pascal trap INTEGER C_Count1Resources(
    ResType typ);
extern pascal trap INTEGER P_Count1Resources(
    ResType typ);
extern pascal trap Handle C_GetIndResource(ResType typ, INTEGER indx);
extern pascal trap Handle P_GetIndResource(ResType typ, INTEGER indx);
extern pascal trap Handle C_Get1IndResource(ResType typ,
                                            INTEGER i);
extern pascal trap Handle P_Get1IndResource(ResType typ,
                                            INTEGER i);
extern pascal trap Handle C_GetResource(ResType typ, INTEGER id);
extern pascal trap Handle P_GetResource(ResType typ, INTEGER id);
extern pascal trap Handle C_Get1Resource(ResType typ,
                                         INTEGER id);
extern pascal trap Handle P_Get1Resource(ResType typ,
                                         INTEGER id);
extern pascal trap Handle C_GetNamedResource(ResType typ, StringPtr nam);
extern pascal trap Handle P_GetNamedResource(ResType typ, StringPtr nam);
extern pascal trap Handle C_Get1NamedResource(ResType typ,
                                              StringPtr s);
extern pascal trap Handle P_Get1NamedResource(ResType typ,
                                              StringPtr s);
extern pascal trap void C_LoadResource(Handle volatile res);
extern pascal trap void P_LoadResource(Handle volatile res);
extern pascal trap void C_ReleaseResource(Handle res);
extern pascal trap void P_ReleaseResource(Handle res);
extern pascal trap void C_DetachResource(Handle res);
extern pascal trap void P_DetachResource(Handle res);
extern pascal trap INTEGER C_UniqueID(ResType typ);
extern pascal trap INTEGER P_UniqueID(ResType typ);
extern pascal trap INTEGER C_Unique1ID(ResType typ);
extern pascal trap INTEGER P_Unique1ID(ResType typ);
extern pascal trap void C_GetResInfo(Handle res, GUEST<INTEGER> *id1,
                                     GUEST<ResType> *typ, StringPtr name);
extern pascal trap INTEGER C_GetResAttrs(Handle res);
extern pascal trap INTEGER P_GetResAttrs(Handle res);
extern LONGINT ROMlib_SizeResource(Handle res, BOOLEAN usehandle);
extern pascal trap LONGINT C_SizeResource(Handle res);
extern pascal trap LONGINT P_SizeResource(Handle res);
extern pascal trap INTEGER C_CountTypes(void);
extern pascal trap INTEGER P_CountTypes(void);
extern pascal trap INTEGER C_Count1Types(void);
extern pascal trap INTEGER P_Count1Types(void);
extern pascal trap void C_GetIndType(GUEST<ResType> *typ, INTEGER indx);
extern pascal trap void P_GetIndType(GUEST<ResType> *typ, INTEGER indx);
extern pascal trap void C_Get1IndType(GUEST<ResType> *typ,
                                      INTEGER indx);
extern pascal trap void P_Get1IndType(GUEST<ResType> *typ,
                                      INTEGER indx);
extern pascal trap LONGINT C_MaxSizeRsrc(Handle h);
extern pascal trap LONGINT P_MaxSizeRsrc(Handle h);
extern pascal trap LONGINT C_RsrcMapEntry(Handle h);
extern pascal trap LONGINT P_RsrcMapEntry(Handle h);
extern pascal trap Handle C_RGetResource(ResType typ, INTEGER id);
extern pascal trap Handle P_RGetResource(ResType typ, INTEGER id);
extern pascal trap INTEGER C_InitResources(void);
extern pascal trap INTEGER P_InitResources(void);
extern pascal trap void C_RsrcZoneInit(void);
extern pascal trap void P_RsrcZoneInit(void);
extern pascal trap INTEGER C_ResError(void);
extern pascal trap INTEGER P_ResError(void);
extern pascal trap INTEGER C_GetResFileAttrs(INTEGER rn);
extern pascal trap INTEGER P_GetResFileAttrs(INTEGER rn);
extern pascal trap void C_SetResFileAttrs(INTEGER rn, INTEGER attrs);
extern pascal trap void P_SetResFileAttrs(INTEGER rn, INTEGER attrs);
extern pascal trap void C_SetResInfo(Handle res, INTEGER id,
                                     StringPtr name);
extern pascal trap void P_SetResInfo(Handle res, INTEGER id,
                                     StringPtr name);
extern pascal trap void C_SetResAttrs(Handle res, INTEGER attrs);
extern pascal trap void P_SetResAttrs(Handle res, INTEGER attrs);
extern pascal trap void C_ChangedResource(Handle res);
extern pascal trap void P_ChangedResource(Handle res);
extern pascal trap void C_AddResource(Handle data, ResType typ,
                                      INTEGER id, StringPtr name);
extern pascal trap void P_AddResource(Handle data, ResType typ,
                                      INTEGER id, StringPtr name);
extern pascal trap void C_RmveResource(Handle res);
extern pascal trap void P_RmveResource(Handle res);
extern pascal trap void C_UpdateResFile(INTEGER rn);
extern pascal trap void P_UpdateResFile(INTEGER rn);
extern pascal trap void C_WriteResource(Handle res);
extern pascal trap void P_WriteResource(Handle res);
extern pascal trap void C_SetResPurge(BOOLEAN install);
extern pascal trap void P_SetResPurge(BOOLEAN install);
extern pascal trap void C_CreateResFile(StringPtr fn);
extern pascal trap void P_CreateResFile(StringPtr fn);
extern pascal trap INTEGER C_OpenRFPerm(StringPtr fn,
                                        INTEGER vref, Byte perm);
extern pascal trap INTEGER P_OpenRFPerm(StringPtr fn,
                                        INTEGER vref, Byte perm);
extern pascal trap INTEGER C_OpenResFile(StringPtr fn);
extern pascal trap INTEGER P_OpenResFile(StringPtr fn);
extern pascal trap void C_CloseResFile(INTEGER rn);
extern pascal trap void P_CloseResFile(INTEGER rn);
extern pascal trap INTEGER C_CurResFile(void);
extern pascal trap INTEGER P_CurResFile(void);
extern pascal trap INTEGER C_HomeResFile(Handle res);
extern pascal trap INTEGER P_HomeResFile(Handle res);
extern pascal trap void C_UseResFile(INTEGER rn);
extern pascal trap void P_UseResFile(INTEGER rn);

extern pascal trap void C_ReadPartialResource(Handle resource,
                                              int32_t offset,
                                              Ptr buffer, int32_t count);
extern pascal trap void C_WritePartialResource(Handle resource,
                                               int32_t offset,
                                               Ptr buffer, int32_t count);
extern pascal trap void C_SetResourceSize(Handle resource, int32_t size);

extern pascal Handle C_GetNextFOND(Handle fondHandle);
}

#endif /* _RESOURCE_H_ */
