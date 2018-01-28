/* Copyright 1994, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/everything.h"

#include "OSUtil.h"
#include "Gestalt.h"
#include "SysErr.h"

#include "rsys/trapglue.h"
#include "rsys/stdfile.h"
#include "rsys/tesave.h"
#include "rsys/resource.h"
#include "rsys/ctl.h"
#include "rsys/list.h"
#include "rsys/menu.h"
#include "rsys/wind.h"
#include "rsys/print.h"
#include "rsys/osutil.h"
#include "rsys/vbl.h"
#include "rsys/soundopts.h"
#include "rsys/refresh.h"
#include "rsys/gestalt.h"
#include "rsys/emustubs.h"
#include "rsys/executor.h"

using namespace Executor;

syn68k_addr_t Executor::tooltraptable[0x400]; /* Gets filled in at run time */
syn68k_addr_t Executor::ostraptable[0x100]; /* Gets filled in at run time */

#define _Pack9 _Unimplemented /* PPCBrowser */
#define _Pack10 _Unimplemented

#define _Pack13 _Unimplemented /* DB stuff */
#define _Pack1 _Unimplemented

#define _AddDrive _Unimplemented
#define _RDrvrInstall _Unimplemented
/* #define (void*)_IMVI_ReadXPRam (void*)_Unimplemented */
#define _IMVI_WriteXPRam _Unimplemented
#define _IMVI_MemoryDispatch _Unimplemented
/* #define (void*)_SlotManager (void*)_Unimplemented */
/* #define (void*)_SlotVInstall (void*)_Unimplemented */
/* #define (void*)_SlotVRemove (void*)_Unimplemented */
#define _AttachVBL _Unimplemented
#define _DoVBLTask _Unimplemented
#define _DTInstall _Unimplemented
#define _SIntRemove _Unimplemented
#define _InternalWait _Unimplemented
#define _SIntInstall _Unimplemented
#define _IMVI_IdleUpdate _Unimplemented
#define _IMVI_SlpQInstall _Unimplemented
#define _IMVI_DebugUtil _Unimplemented
#define _IMVI_DeferUserFn _Unimplemented
#define _IMVI_Translate24To32 _Unimplemented

#define _GetMaskTable _Unimplemented
#define _Debugger _Unimplemented

void Executor::C_unknown574(void)
{
}

static void *fsroutines[][2] = {
    { /* 0xA000 */ (void *)PBOpen, (void *)PBHOpen /*  0 */ },
#define OPENTRAP 0
#define OPENINDEX 0

    { /* 0xA001 */ (void *)PBClose, (void *)PBClose /*  1 */ },
#define CLOSETRAP 1
#define CLOSEINDEX 1

    { /* 0xA002 */ (void *)PBRead, (void *)PBRead /*  2 */ },
#define READTRAP 2
#define READINDEX 2

    { /* 0xA003 */ (void *)PBWrite, (void *)PBWrite /*  3 */ },
#define WRITETRAP 3
#define WRITEINDEX 3

    { /* 0xA004 */ (void *)PBControl, (void *)PBControl /*  4 */ },
#define CONTROLTRAP 4
#define CONTROLINDEX 4

    { /* 0xA005 */ (void *)PBStatus, (void *)PBStatus /*  5 */ },
#define STATUSTRAP 5
#define STATUSINDEX 5

    { /* 0xA006 */ (void *)PBKillIO, (void *)PBKillIO /*  6 */ },
#define KILLIOTRAP 6
#define KILLIOINDEX 6

    { /* 0xA007 */ (void *)PBGetVInfo, (void *)PBHGetVInfo /*  7 */ },
#define GETVINFOTRAP 7
#define GETVINFOINDEX 7

    { /* 0xA008 */ (void *)PBCreate, (void *)PBHCreate /*  8 */ },
#define CREATETRAP 8
#define CREATEINDEX 8

    { /* 0xA009 */ (void *)PBDelete, (void *)PBHDelete /*  9 */ },
#define DELETETRAP 9
#define DELETEINDEX 9

    { /* 0xA00A */ (void *)PBOpenRF, (void *)PBHOpenRF /* 10 */ },
#define OPENRFTRAP 0xA
#define OPENRFINDEX 10

    { /* 0xA00B */ (void *)PBRename, (void *)PBHRename /* 11 */ },
#define RENAMETRAP 0xB
#define RENAMEINDEX 11

    { /* 0xA00C */ (void *)PBGetFInfo, (void *)PBHGetFInfo /* 12 */ },
#define GETFINFOTRAP 0xC
#define GETFINFOINDEX 12

    { /* 0xA00D */ (void *)PBSetFInfo, (void *)PBHSetFInfo /* 13 */ },
#define SETFINFOTRAP 0xD
#define SETFINFOINDEX 13

    { /* 0xA00E */ (void *)PBUnmountVol, (void *)PBUnmountVol /* 14 */ },
#define UNMOUNTVOLTRAP 0xE
#define UNMOUNTVOLINDEX 14

    { /* 0xA00F */ (void *)PBMountVol, (void *)PBMountVol /* 15 */ },
#define MOUNTVOLTRAP 0xF
#define MOUNTVOLINDEX 15

    { /* 0xA010 */ (void *)PBAllocate, (void *)PBAllocate /* 16 */ },
#define ALLOCATETRAP 0x10
#define ALLOCATEINDEX 16

    { /* 0xA011 */ (void *)PBGetEOF, (void *)PBGetEOF /* 17 */ },
#define GETEOFTRAP 0x11
#define GETEOFINDEX 17

    { /* 0xA012 */ (void *)PBSetEOF, (void *)PBSetEOF /* 18 */ },
#define SETEOFTRAP 0x12
#define SETEOFINDEX 18

    { /* 0xA013 */ (void *)PBFlushVol, (void *)PBFlushVol /* 19 */ },
#define FLUSHVOLTRAP 0x13
#define FLUSHVOLINDEX 19

    { /* 0xA014 */ (void *)PBGetVol, (void *)PBHGetVol /* 20 */ },
#define GETVOLTRAP 0x14
#define GETVOLINDEX 20

    { /* 0xA015 */ (void *)PBSetVol, (void *)PBHSetVol /* 21 */ },
#define SETVOLTRAP 0x15
#define SETVOLINDEX 21

    { /* 0xA017 */ (void *)PBEject, (void *)PBEject /* 22 */ },
#define EJECTTRAP 0x17
#define EJECTINDEX 22

    { /* 0xA018 */ (void *)PBGetFPos, (void *)PBGetFPos /* 23 */ },
#define GETFPOSTRAP 0x18
#define GETFPOSINDEX 23

    { /* 0xA035 */ (void *)PBOffLine, (void *)PBOffLine /* 24 */ },
#define OFFLINETRAP 0x35
#define OFFLINEINDEX 24

    { /* 0xA041 */ (void *)PBSetFLock, (void *)PBHSetFLock /* 25 */ },
#define SETFLOCKTRAP 0x41
#define SETFLOCKINDEX 25

    { /* 0xA042 */ (void *)PBRstFLock, (void *)PBHRstFLock /* 26 */ },
#define RSTFLOCKTRAP 0x42
#define RSTFLOCKINDEX 26

    { /* 0xA043 */ (void *)PBSetFVers, (void *)PBSetFVers /* 27 */ },
#define SETFVERSTRAP 0x43
#define SETFVERSINDEX 27

    { /* 0xA044 */ (void *)PBSetFPos, (void *)PBSetFPos /* 28 */ },
#define SETFPOSTRAP 0x44
#define SETFPOSINDEX 28

    { /* 0xA045 */ (void *)PBFlushFile, (void *)PBFlushFile /* 29 */ },
#define FLUSHFILETRAP 0x45
#define FLUSHFILEINDEX 29

};

osstuff_t Executor::osstuff[0x100] = {
    { 0, (void *)PBOpen },
    { 0, (void *)PBClose },
    { 0, (void *)PBRead },
    { 0, (void *)PBWrite },
    { 0, (void *)PBControl },
    { 0, (void *)PBStatus },
    { 0, (void *)PBKillIO },
    { 0, (void *)PBGetVInfo },
    { 0, (void *)PBCreate },
    { 0, (void *)PBDelete },
    { 0, (void *)PBOpenRF },
    { 0, (void *)PBRename },
    { 0, (void *)PBGetFInfo },
    { 0, (void *)PBSetFInfo },
    { 0, (void *)PBUnmountVol },
    { 0, (void *)PBMountVol },
    { 0, (void *)PBAllocate },
    { 0, (void *)PBGetEOF },
    { 0, (void *)PBSetEOF },
    { 0, (void *)PBFlushVol },
    { 0, (void *)PBGetVol },
    { 0, (void *)PBSetVol },
    { 0, (void *)_FInitQueue },
    { 0, (void *)PBEject },
    { 0, (void *)PBGetFPos },
    { 0, (void *)_InitZone },
 {0,0},//   { 0, (void *)_GetZone },
 {0,0},//   { 0, (void *)_SetZone },
    { 0, (void *)_FreeMem },
    { 0, (void *)_MaxMem },
  {0,0},//  { 0, (void *)_NewPtr },
 {0,0},//   { 0, (void *)_DisposPtr },
{0,0},//    { 0, (void *)_SetPtrSize },
{0,0},//    { 0, (void *)_GetPtrSize },
    { 0, (void *)_NewHandle },
{0,0},//    { 0, (void *)_DisposHandle },
{0,0},//    { 0, (void *)_SetHandleSize },
{0,0},//    { 0, (void *)_GetHandleSize },
{0,0},//    { 0, (void *)_HandleZone },
{0,0},//    { 0, (void *)_ReallocHandle },
    { 0, (void *)_RecoverHandle },
{0,0},//    { 0, (void *)_HLock },
{0,0},//    { 0, (void *)_HUnlock },
{0,0},//    { 0, (void *)_EmptyHandle },
{0,0},//    { 0, (void *)_InitApplZone },
{0,0},//    { 0, (void *)_SetApplLimit },
    { 0, (void *)_BlockMove },
    { 0, (void *)_PostEvent },
{0,0},//    { 0, (void *)_OSEventAvail },
{0,0},//    { 0, (void *)_GetOSEvent },
{0,0},//    { 0, (void *)_FlushEvents },
{0,0},//    { 0, (void *)_VInstall },
{0,0},//    { 0, (void *)_VRemove },
    { 0, (void *)PBOffLine },
{0,0},//    { 0, (void *)_MoreMasters },
    { 0, (void *)_Unimplemented },
{0,0},//    { 0, (void *)_WriteParam },
{0,0},//    { 0, (void *)_ReadDateTime },
{0,0},//    { 0, (void *)_SetDateTime },
{0,0},//    { 0, (void *)_Delay },
    { 0, (void *)_EqualString },
    { 0, (void *)_DrvrInstall },
    { 0, (void *)_DrvrRemove },
    { 0, (void *)_InitUtil },
    { 0, (void *)_ResrvMem },
    { 0, (void *)PBSetFLock },
    { 0, (void *)PBRstFLock },
    { 0, (void *)PBSetFVers },
    { 0, (void *)PBSetFPos },
    { 0, (void *)PBFlushFile },
    { 0, (void *)_GetTrapAddress },
    { 0, (void *)_SetTrapAddress },
{0,0},//    { 0, (void *)_PtrZone },
{0,0},//    { 0, (void *)_HPurge },
{0,0},//    { 0, (void *)_HNoPurge },
{0,0},//    { 0, (void *)_SetGrowZone },
    { 0, (void *)_CompactMem },
    { 0, (void *)_PurgeMem },
    { 0, (void *)_AddDrive },
    { 0, (void *)_RDrvrInstall },
    { 0, (void *)_RelString },
    { 0, (void *)_IMVI_ReadXPRam },
    { 0, (void *)_IMVI_WriteXPRam },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_UprString },
    { 0, (void *)_StripAddress },
    { 0, (void *)_IMVI_LowerText },
{0,0},//    { 0, (void *)_SetApplBase },
{0,0},//    { 0, (void *)_InsTime },
{0,0},//    { 0, (void *)_RmvTime },
{0,0},//    { 0, (void *)_PrimeTime },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_IMVI_MemoryDispatch },
    { 0, (void *)_SwapMMUMode },
{0,0},//    { 0, (void *)_NMInstall },
{0,0},//    { 0, (void *)_NMRemove },
    { 0, (void *)_HFSDispatch },
    { 0, (void *)_MaxBlock },
    { 0, (void *)_PurgeSpace },
{0,0},//    { 0, (void *)_MaxApplZone },
{0,0},//    { 0, (void *)_MoveHHi },
{0,0},//    { 0, (void *)_StackSpace },
    { 0, (void *)_NewEmptyHandle },
{0,0},//    { 0, (void *)_HSetRBit },
{0,0},//    { 0, (void *)_HClrRBit },
{0,0},//    { 0, (void *)_HGetState },
{0,0},//    { 0, (void *)_HSetState },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_SlotManager },
{0,0},//    { 0, (void *)_SlotVInstall },
{0,0},//    { 0, (void *)_SlotVRemove },
    { 0, (void *)_AttachVBL },
    { 0, (void *)_DoVBLTask },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_DTInstall },
    { 0, (void *)_SIntRemove },
{0,0},//    { 0, (void *)_CountADBs },
{0,0},//    { 0, (void *)_GetIndADB },
{0,0},//    { 0, (void *)_GetADBInfo },
{0,0},//    { 0, (void *)_SetADBInfo },
{0,0},//    { 0, (void *)_ADBReInit },
    { 0, (void *)_ADBOp },
    { 0, (void *)_GetDefaultStartup },
    { 0, (void *)_SetDefaultStartup },
    { 0, (void *)_InternalWait },
    { 0, (void *)_GetVideoDefault },
    { 0, (void *)_SetVideoDefault },
    { 0, (void *)_SIntInstall },
    { 0, (void *)_SetOSDefault },
    { 0, (void *)_GetOSDefault },
    { 0, (void *)_IMVI_IdleUpdate },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_IMVI_SlpQInstall },
    { 0, (void *)_CommToolboxDispatch },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_IMVI_DebugUtil },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_IMVI_DeferUserFn },
{0,0},//    { 0, (void *)_SysEnvirons },
    { 0, (void *)_IMVI_Translate24To32 },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Microseconds },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
{0,0},//    { 0, (void *)_HWPriv },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Gestalt },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
{0,0},//    { 0, (void *)_flushcache },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_IMVI_PPC },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_ResourceStub },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
    { 0, (void *)_Unimplemented },
};

void Executor::filltables(void)
{
    int i, fsindex;
    syn68k_addr_t unimpl = 0;

    unimpl = callback_install((callback_handler_t)(void *)_Unimplemented,
                              nullptr);
    
    for(i = 0; i < (int)NELEM(ostraptable); ++i)
    {
        switch(i)
        {
            case OPENTRAP:
                fsindex = OPENINDEX;
                break;
            case CLOSETRAP:
                fsindex = CLOSEINDEX;
                break;
            case READTRAP:
                fsindex = READINDEX;
                break;
            case WRITETRAP:
                fsindex = WRITEINDEX;
                break;
            case CONTROLTRAP:
                fsindex = CONTROLINDEX;
                break;
            case STATUSTRAP:
                fsindex = STATUSINDEX;
                break;
            case KILLIOTRAP:
                fsindex = KILLIOINDEX;
                break;
            case GETVINFOTRAP:
                fsindex = GETVINFOINDEX;
                break;
            case CREATETRAP:
                fsindex = CREATEINDEX;
                break;
            case DELETETRAP:
                fsindex = DELETEINDEX;
                break;
            case OPENRFTRAP:
                fsindex = OPENRFINDEX;
                break;
            case RENAMETRAP:
                fsindex = RENAMEINDEX;
                break;
            case GETFINFOTRAP:
                fsindex = GETFINFOINDEX;
                break;
            case SETFINFOTRAP:
                fsindex = SETFINFOINDEX;
                break;
            case UNMOUNTVOLTRAP:
                fsindex = UNMOUNTVOLINDEX;
                break;
            case MOUNTVOLTRAP:
                fsindex = MOUNTVOLINDEX;
                break;
            case ALLOCATETRAP:
                fsindex = ALLOCATEINDEX;
                break;
            case GETEOFTRAP:
                fsindex = GETEOFINDEX;
                break;
            case SETEOFTRAP:
                fsindex = SETEOFINDEX;
                break;
            case FLUSHVOLTRAP:
                fsindex = FLUSHVOLINDEX;
                break;
            case GETVOLTRAP:
                fsindex = GETVOLINDEX;
                break;
            case SETVOLTRAP:
                fsindex = SETVOLINDEX;
                break;
            case EJECTTRAP:
                fsindex = EJECTINDEX;
                break;
            case GETFPOSTRAP:
                fsindex = GETFPOSINDEX;
                break;
            case OFFLINETRAP:
                fsindex = OFFLINEINDEX;
                break;
            case SETFLOCKTRAP:
                fsindex = SETFLOCKINDEX;
                break;
            case RSTFLOCKTRAP:
                fsindex = RSTFLOCKINDEX;
                break;
            case SETFVERSTRAP:
                fsindex = SETFVERSINDEX;
                break;
            case SETFPOSTRAP:
                fsindex = SETFPOSINDEX;
                break;
            case FLUSHFILETRAP:
                fsindex = FLUSHFILEINDEX;
                break;
            default:
                fsindex = static_cast<ULONGINT>(-1);
                break;
        };

        if(fsindex == static_cast<ULONGINT>(-1))
            ostraptable[i] = osstuff[i].orig = (osstuff[i].func == (void *)_Unimplemented) ? unimpl
                                                                                           : callback_install((callback_handler_t)osstuff[i].func,
                                                                                                              osstuff[i].func);
        else
        {
            osstuff[i].func = (void *)_HFSRoutines;
            ostraptable[i] = osstuff[i].orig = callback_install((callback_handler_t)_HFSRoutines,
                                                                fsroutines[fsindex]);
        }
    }
}
