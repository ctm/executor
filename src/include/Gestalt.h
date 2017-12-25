#if !defined(__GESTALT__)
#define __GESTALT__

namespace Executor
{
#define gestaltPhysicalRAMSize T('r', 'a', 'm', ' ')

#if !defined(USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)

/*
 * Copyright 1992 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#define gestaltAddressingModeAttr T('a', 'd', 'd', 'r')
#define gestaltAliasMgrAttr T('a', 'l', 'i', 's')
#define gestaltApplEventsAttr T('e', 'v', 'n', 't')
#define gestaltAppleTalkVersion T('a', 't', 'l', 'k')
#define gestaltAUXVersion T('a', '/', 'u', 'x')
#define gestaltConnMgrAttr T('c', 'o', 'n', 'n')
#define gestaltCRMAttr T('c', 'r', 'm', ' ')
#define gestaltCTBVersion T('c', 't', 'b', 'v')
#define gestaltDBAccessMgrAttr T('d', 'b', 'a', 'c')
#define gestaltDITLExtAttr T('d', 'i', 't', 'l')
#define gestaltEasyAccessAttr T('e', 'a', 's', 'y')
#define gestaltEditionMgrAttr T('e', 'd', 't', 'n')
#define gestaltExtToolboxTable T('x', 't', 't', 't')
#define gestaltFindFolderAttr T('f', 'o', 'l', 'd')
#define gestaltFontMgrAttr T('f', 'o', 'n', 't')
#define gestaltFPUType T('f', 'p', 'u', ' ')
#define gestaltFSAttr T('f', 's', ' ', ' ')
#define gestaltFXfrMgrAttr T('f', 'x', 'f', 'r')
#define gestaltHardwareAttr T('h', 'd', 'w', 'r')
#define gestaltHelpMgrAttr T('h', 'e', 'l', 'p')
#define gestaltKeyboardType T('k', 'b', 'd', ' ')
#define gestaltLogicalPageSize T('p', 'g', 's', 'z')
#define gestaltLogicalRAMSize T('l', 'r', 'a', 'm')
#define gestaltLowMemorySize T('l', 'm', 'e', 'm')
#define gestaltMiscAttr T('m', 'i', 's', 'c')
#define gestaltMMUType T('m', 'm', 'u', ' ')
#define gestaltNotificatinMgrAttr T('n', 'm', 'g', 'r')
#define gestaltNuBusConnectors T('s', 'l', 't', 'c')
#define gestaltOSAttr T('o', 's', ' ', ' ')
#define gestaltOSTable T('o', 's', 't', 't')
#define gestaltParityAttr T('p', 'r', 't', 'y')
#define gestaltPopupAttr T('p', 'o', 'p', '!')
#define gestaltPowerMgrAttr T('p', 'o', 'w', 'r')
#define gestaltPPCToolboxAttr T('p', 'p', 'c', ' ')
#define gestaltProcessorType T('p', 'r', 'o', 'c')
#define gestaltQuickdrawVersion T('q', 'd', ' ', ' ')
#define gestaltQuickdrawFeatures T('q', 'd', 'r', 'w')
#define gestaltResourceMgrAttr T('r', 's', 'r', 'c')
#define gestaltScriptCount T('s', 'c', 'r', '#')
#define gestaltScriptMgrVersion T('s', 'c', 'r', 'i')
#define gestaltSerialAttr T('s', 'e', 'r', ' ')
#define gestaltSoundAttr T('s', 'n', 'd', ' ')
#define gestaltStandardFileAttr T('s', 't', 'd', 'f')
#define gestaltStdNBPAttr T('n', 'l', 'u', 'p')
#define gestaltTermMgrAttr T('t', 'e', 'r', 'm')
#define gestaltTextEditVersion T('t', 'e', ' ', ' ')
#define gestaltTimeMgrVersion T('t', 'm', 'g', 'r')
#define gestaltToolboxTable T('t', 'b', 'b', 't')
#define gestaltVersion T('v', 'e', 'r', 's')
#define gestaltVMAttr T('v', 'm', ' ', ' ')
#define gestaltMachineIcon T('m', 'i', 'c', 'n')
#define gestaltMachineType T('m', 'a', 'c', 'h')
#define gestaltROMSize T('r', 'o', 'm', ' ')
#define gestaltROMVersion T('r', 'o', 'm', 'v')
#define gestaltSystemVersion T('s', 'y', 's', 'v')

enum
{
    gestaltNativeCPUtype = T('c', 'p', 'u', 't'),
    gestaltSysArchitecture = T('s', 'y', 's', 'a'),
};

enum
{
    gestaltMacQuadra610 = 53,
    gestaltCPU68040 = 4,

    gestaltCPU601 = 0x101,
    gestaltCPU603 = 0x103,
    gestaltCPU604 = 0x104,
    gestaltCPU603e = 0x106,
    gestaltCPU603ev = 0x107,
    gestaltCPU750 = 0x108, /* G3 */
    gestaltCPU604e = 0x109,
    gestaltCPU604ev = 0x10A,
    gestaltCPUG4 = 0x10C, /* determined by running test app on Sybil */

    gestaltNoMMU = 0,
    gestalt68k = 1,
    gestaltPowerPC = 2,
};

#define gestalt32BitAddressing 0
#define gestalt32BitSysZone 1
#define gestalt32BitCapable 2

/* gestaltHardwareAttr return values */
#define gestaltHasVIA1 0
#define gestaltHasVIA2 1
#define gestaltHasASC 3
#define gestaltHasSSC 4
#define gestaltHasSCI 7

#define gestaltEasyAccessOff (1 << 0)
#define gestalt68881 1
#define gestaltMacKbd 1
#define gestalt68040MMu 4
#define gestalt68000 1
#define gestalt68040 5

#define gestaltOriginalQD 0
#define gestalt8BitQD 0x0100
#define gestalt32BitQD 0x0200
#define gestalt32BitQD11 0x0210
#define gestalt32BitQD12 0x0220
#define gestalt32BitQD13 0x0230

#define gestaltHasColor 0
#define gestaltHasDeepGWorlds 1
#define gestaltHasDirectPixMaps 2
#define gestaltHasGrayishTextOr 3

#define gestaltTE1 1
#define gestaltTE2 2
#define gestaltTE3 3
#define gestaltTE4 4
#define gestaltTE5 5

#define gestaltDITLExtPresent 0

#define gestaltStandardTimeMgr 1
#define gestaltVMPresent (1 << 0)

#define gestaltClassic 1
#define gestaltMacXL 2
#define gestaltMac512KE 3
#define gestaltMacPlus 4
#define gestaltMacSE 5
#define gestaltMacII 6
#define gestaltMacIIx 7
#define gestaltMacIIcx 8
#define gestaltMacSE30 9
#define gestaltPortable 10
#define gestaltMacIIci 11
#define gestaltMacIIfx 13
#define gestaltMacClassic 17
#define gestaltMacIIsi 18
#define gestaltMacLC 19

enum
{
    gestaltHasFSSpecCalls = (1 << 1)
};
enum
{
    gestaltStandardFile58 = (1 << 0)
};

enum
{
    gestaltUndefSelectorErr = -5551,
    gestaltUnknownErr = -5550,
    gestaltDupSelectorErr = -5552,
    gestaltLocationErr = -5553,
};

extern trap OSErrRET Gestalt(OSType selector, GUEST<LONGINT> *responsep);
extern trap OSErrRET NewGestalt(OSType selector, ProcPtr selFunc);
extern trap OSErrRET ReplaceGestalt(OSType selector, ProcPtr selFunc,
                                    ProcPtr *oldSelFuncp);

extern trap OSErrRET C_GestaltTablesOnly(OSType selector,
                                         GUEST<LONGINT> *responsep);

#endif
}
#endif
