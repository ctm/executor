#if !defined(__SEGMENT__)
#define __SEGMENT__

/*
 * Copyright 1989 - 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "ExMacTypes.h"
#include <rsys/lowglobals.h>

#define MODULE_NAME SegmentLdr
#include <rsys/api-module.h>

namespace Executor
{
//extern _NORET_1_ void C_ExitToShell(void) _NORET_2_;
extern void C_ExitToShell(void);
PASCAL_TRAP(ExitToShell, 0xA9F4);

#if !defined(USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)

enum
{
    appOpen = 0,
    appPrint = 1,
};

struct AppFile
{
    GUEST_STRUCT;
    GUEST<INTEGER> vRefNum;
    GUEST<OSType> fType;
    GUEST<INTEGER> versNum;
    GUEST<Str255> fName;
};

enum
{
    hwParamErr = (-502),
};

const LowMemGlobal<Byte> loadtrap { 0x12D }; // SegmentLdr SysEqu.a (true-b);
const LowMemGlobal<Byte[16]> FinderName { 0x2E0 }; // SegmentLdr IMII-59 (true);
const LowMemGlobal<INTEGER> CurApRefNum { 0x900 }; // SegmentLdr IMII-58 (true);
/*
 * NOTE: IMIII says CurApName is 32 bytes LONGINT, but it looks to me like
 * it is really 34 bytes LONGINT.
 */
const LowMemGlobal<Byte[34]> CurApName { 0x910 }; // SegmentLdr IMII-58 (true);
const LowMemGlobal<INTEGER> CurJTOffset { 0x934 }; // SegmentLdr IMII-62 (true-b);
const LowMemGlobal<INTEGER> CurPageOption { 0x936 }; // SegmentLdr IMII-60 (true);
const LowMemGlobal<Handle> AppParmHandle { 0xAEC }; // SegmentLdr IMII-57 (true);

extern void C_FlushCodeCache(void);
PASCAL_TRAP(FlushCodeCache, 0xA0BD);

extern void HWPriv(LONGINT d0, LONGINT a0);
REGISTER_TRAP2(HWPriv, 0xA198, void(D0,A0));

extern char *ROMlib_undotdot(char *origp);
extern void CountAppFiles(GUEST<INTEGER> *messagep,
                          GUEST<INTEGER> *countp);
extern void GetAppFiles(INTEGER index, AppFile *filep);
extern void ClrAppFiles(INTEGER index);
extern void Launch(StringPtr appl, INTEGER vrefnum);
extern void Chain(StringPtr appl, INTEGER vrefnum);

extern void C_GetAppParms(StringPtr namep,
                                      GUEST<INTEGER> *rnp, GUEST<Handle> *aphandp);
PASCAL_TRAP(GetAppParms, 0xA9F5);

extern void C_UnloadSeg(Ptr addr);
PASCAL_TRAP(UnloadSeg, 0xA9F1);

extern void C_LoadSeg(INTEGER volatile segno);

#endif
}
#endif /* __SEGMENT__ */
