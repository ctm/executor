/*
 * Copyright 1992 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 */

#if !defined(__GESTALT__)
#define __GESTALT__

namespace Executor
{

enum
{
    gestaltPhysicalRAMSize = T('r', 'a', 'm', ' '),
    gestaltAddressingModeAttr = T('a', 'd', 'd', 'r'),
    gestaltAliasMgrAttr = T('a', 'l', 'i', 's'),
    gestaltApplEventsAttr = T('e', 'v', 'n', 't'),
    gestaltAppleTalkVersion = T('a', 't', 'l', 'k'),
    gestaltAUXVersion = T('a', '/', 'u', 'x'),
    gestaltConnMgrAttr = T('c', 'o', 'n', 'n'),
    gestaltCRMAttr = T('c', 'r', 'm', ' '),
    gestaltCTBVersion = T('c', 't', 'b', 'v'),
    gestaltDBAccessMgrAttr = T('d', 'b', 'a', 'c'),
    gestaltDITLExtAttr = T('d', 'i', 't', 'l'),
    gestaltEasyAccessAttr = T('e', 'a', 's', 'y'),
    gestaltEditionMgrAttr = T('e', 'd', 't', 'n'),
    gestaltExtToolboxTable = T('x', 't', 't', 't'),
    gestaltFindFolderAttr = T('f', 'o', 'l', 'd'),
    gestaltFontMgrAttr = T('f', 'o', 'n', 't'),
    gestaltFPUType = T('f', 'p', 'u', ' '),
    gestaltFSAttr = T('f', 's', ' ', ' '),
    gestaltFXfrMgrAttr = T('f', 'x', 'f', 'r'),
    gestaltHardwareAttr = T('h', 'd', 'w', 'r'),
    gestaltHelpMgrAttr = T('h', 'e', 'l', 'p'),
    gestaltKeyboardType = T('k', 'b', 'd', ' '),
    gestaltLogicalPageSize = T('p', 'g', 's', 'z'),
    gestaltLogicalRAMSize = T('l', 'r', 'a', 'm'),
    gestaltLowMemorySize = T('l', 'm', 'e', 'm'),
    gestaltMiscAttr = T('m', 'i', 's', 'c'),
    gestaltMMUType = T('m', 'm', 'u', ' '),
    gestaltNotificatinMgrAttr = T('n', 'm', 'g', 'r'),
    gestaltNuBusConnectors = T('s', 'l', 't', 'c'),
    gestaltOSAttr = T('o', 's', ' ', ' '),
    gestaltOSTable = T('o', 's', 't', 't'),
    gestaltParityAttr = T('p', 'r', 't', 'y'),
    gestaltPopupAttr = T('p', 'o', 'p', '!'),
    gestaltPowerMgrAttr = T('p', 'o', 'w', 'r'),
    gestaltPPCToolboxAttr = T('p', 'p', 'c', ' '),
    gestaltProcessorType = T('p', 'r', 'o', 'c'),
    gestaltQuickdrawVersion = T('q', 'd', ' ', ' '),
    gestaltQuickdrawFeatures = T('q', 'd', 'r', 'w'),
    gestaltResourceMgrAttr = T('r', 's', 'r', 'c'),
    gestaltScriptCount = T('s', 'c', 'r', '#'),
    gestaltScriptMgrVersion = T('s', 'c', 'r', 'i'),
    gestaltSerialAttr = T('s', 'e', 'r', ' '),
    gestaltSoundAttr = T('s', 'n', 'd', ' '),
    gestaltStandardFileAttr = T('s', 't', 'd', 'f'),
    gestaltStdNBPAttr = T('n', 'l', 'u', 'p'),
    gestaltTermMgrAttr = T('t', 'e', 'r', 'm'),
    gestaltTextEditVersion = T('t', 'e', ' ', ' '),
    gestaltTimeMgrVersion = T('t', 'm', 'g', 'r'),
    gestaltToolboxTable = T('t', 'b', 'b', 't'),
    gestaltVersion = T('v', 'e', 'r', 's'),
    gestaltVMAttr = T('v', 'm', ' ', ' '),
    gestaltMachineIcon = T('m', 'i', 'c', 'n'),
    gestaltMachineType = T('m', 'a', 'c', 'h'),
    gestaltROMSize = T('r', 'o', 'm', ' '),
    gestaltROMVersion = T('r', 'o', 'm', 'v'),
    gestaltSystemVersion = T('s', 'y', 's', 'v'),

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

enum
{
    gestalt32BitAddressing = 0,
    gestalt32BitSysZone = 1,
    gestalt32BitCapable = 2,
};

/* gestaltHardwareAttr return values */
enum
{
    gestaltHasVIA1 = 0,
    gestaltHasVIA2 = 1,
    gestaltHasASC = 3,
    gestaltHasSSC = 4,
    gestaltHasSCI = 7,
};

enum
{
    gestaltEasyAccessOff = (1 << 0),
    gestalt68881 = 1,
    gestaltMacKbd = 1,
    gestalt68040MMu = 4,
    gestalt68000 = 1,
    gestalt68040 = 5,
};

enum
{
    gestaltOriginalQD = 0,
    gestalt8BitQD = 0x0100,
    gestalt32BitQD = 0x0200,
    gestalt32BitQD11 = 0x0210,
    gestalt32BitQD12 = 0x0220,
    gestalt32BitQD13 = 0x0230,
};

enum
{
    gestaltHasColor = 0,
    gestaltHasDeepGWorlds = 1,
    gestaltHasDirectPixMaps = 2,
    gestaltHasGrayishTextOr = 3,
};

enum
{
    gestaltTE1 = 1,
    gestaltTE2 = 2,
    gestaltTE3 = 3,
    gestaltTE4 = 4,
    gestaltTE5 = 5,
};

enum
{
    gestaltDITLExtPresent = 0,
};

enum
{
    gestaltStandardTimeMgr = 1,
    gestaltVMPresent = (1 << 0),
};

enum
{
    gestaltClassic = 1,
    gestaltMacXL = 2,
    gestaltMac512KE = 3,
    gestaltMacPlus = 4,
    gestaltMacSE = 5,
    gestaltMacII = 6,
    gestaltMacIIx = 7,
    gestaltMacIIcx = 8,
    gestaltMacSE30 = 9,
    gestaltPortable = 10,
    gestaltMacIIci = 11,
    gestaltMacIIfx = 13,
    gestaltMacClassic = 17,
    gestaltMacIIsi = 18,
    gestaltMacLC = 19,
};

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

extern OSErrRET Gestalt(OSType selector, GUEST<LONGINT> *responsep);
extern OSErrRET NewGestalt(OSType selector, ProcPtr selFunc);
extern OSErrRET ReplaceGestalt(OSType selector, ProcPtr selFunc,
                                    ProcPtr *oldSelFuncp);

extern OSErrRET C_GestaltTablesOnly(OSType selector,
                                         GUEST<LONGINT> *responsep);
PASCAL_FUNCTION(GestaltTablesOnly);
}
#endif
