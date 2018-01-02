#if !defined(__SEGMENT__)
#define __SEGMENT__

/*
 * Copyright 1989 - 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "rsys/noreturn.h"

namespace Executor
{
extern _NORET_1_ void C_ExitToShell(void) _NORET_2_;
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

#if 0
#if !defined(AppParmHandle_H)
extern GUEST<Handle> 	AppParmHandle_H;
extern Byte 	loadtrap;
extern Byte 	FinderName[16];
extern INTEGER 	CurApRefNum;
extern Byte 	CurApName[34];
extern INTEGER 	CurJTOffset;
extern INTEGER 	CurPageOption;
#endif

enum
{
    AppParmHandle = (AppParmHandle_H.p),
};
#endif

extern void flushcache(void);

extern void HWPriv(LONGINT d0, LONGINT a0);
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
PASCAL_FUNCTION(LoadSeg);

#endif
}
#endif /* __SEGMENT__ */
