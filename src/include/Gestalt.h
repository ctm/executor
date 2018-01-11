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
    gestaltPhysicalRAMSize = FOURCC('r', 'a', 'm', ' '),
    gestaltAddressingModeAttr = FOURCC('a', 'd', 'd', 'r'),
    gestaltAliasMgrAttr = FOURCC('a', 'l', 'i', 's'),
    gestaltApplEventsAttr = FOURCC('e', 'v', 'n', 't'),
    gestaltAppleTalkVersion = FOURCC('a', 't', 'l', 'k'),
    gestaltAUXVersion = FOURCC('a', '/', 'u', 'x'),
    gestaltConnMgrAttr = FOURCC('c', 'o', 'n', 'n'),
    gestaltCRMAttr = FOURCC('c', 'r', 'm', ' '),
    gestaltCTBVersion = FOURCC('c', 't', 'b', 'v'),
    gestaltDBAccessMgrAttr = FOURCC('d', 'b', 'a', 'c'),
    gestaltDITLExtAttr = FOURCC('d', 'i', 't', 'l'),
    gestaltEasyAccessAttr = FOURCC('e', 'a', 's', 'y'),
    gestaltEditionMgrAttr = FOURCC('e', 'd', 't', 'n'),
    gestaltExtToolboxTable = FOURCC('x', 't', 't', 't'),
    gestaltFindFolderAttr = FOURCC('f', 'o', 'l', 'd'),
    gestaltFontMgrAttr = FOURCC('f', 'o', 'n', 't'),
    gestaltFPUType = FOURCC('f', 'p', 'u', ' '),
    gestaltFSAttr = FOURCC('f', 's', ' ', ' '),
    gestaltFXfrMgrAttr = FOURCC('f', 'x', 'f', 'r'),
    gestaltHardwareAttr = FOURCC('h', 'd', 'w', 'r'),
    gestaltHelpMgrAttr = FOURCC('h', 'e', 'l', 'p'),
    gestaltKeyboardType = FOURCC('k', 'b', 'd', ' '),
    gestaltLogicalPageSize = FOURCC('p', 'g', 's', 'z'),
    gestaltLogicalRAMSize = FOURCC('l', 'r', 'a', 'm'),
    gestaltLowMemorySize = FOURCC('l', 'm', 'e', 'm'),
    gestaltMiscAttr = FOURCC('m', 'i', 's', 'c'),
    gestaltMMUType = FOURCC('m', 'm', 'u', ' '),
    gestaltNotificatinMgrAttr = FOURCC('n', 'm', 'g', 'r'),
    gestaltNuBusConnectors = FOURCC('s', 'l', 't', 'c'),
    gestaltOSAttr = FOURCC('o', 's', ' ', ' '),
    gestaltOSTable = FOURCC('o', 's', 't', 't'),
    gestaltParityAttr = FOURCC('p', 'r', 't', 'y'),
    gestaltPopupAttr = FOURCC('p', 'o', 'p', '!'),
    gestaltPowerMgrAttr = FOURCC('p', 'o', 'w', 'r'),
    gestaltPPCToolboxAttr = FOURCC('p', 'p', 'c', ' '),
    gestaltProcessorType = FOURCC('p', 'r', 'o', 'c'),
    gestaltQuickdrawVersion = FOURCC('q', 'd', ' ', ' '),
    gestaltQuickdrawFeatures = FOURCC('q', 'd', 'r', 'w'),
    gestaltResourceMgrAttr = FOURCC('r', 's', 'r', 'c'),
    gestaltScriptCount = FOURCC('s', 'c', 'r', '#'),
    gestaltScriptMgrVersion = FOURCC('s', 'c', 'r', 'i'),
    gestaltSerialAttr = FOURCC('s', 'e', 'r', ' '),
    gestaltSoundAttr = FOURCC('s', 'n', 'd', ' '),
    gestaltStandardFileAttr = FOURCC('s', 't', 'd', 'f'),
    gestaltStdNBPAttr = FOURCC('n', 'l', 'u', 'p'),
    gestaltTermMgrAttr = FOURCC('t', 'e', 'r', 'm'),
    gestaltTextEditVersion = FOURCC('t', 'e', ' ', ' '),
    gestaltTimeMgrVersion = FOURCC('t', 'm', 'g', 'r'),
    gestaltToolboxTable = FOURCC('t', 'b', 'b', 't'),
    gestaltVersion = FOURCC('v', 'e', 'r', 's'),
    gestaltVMAttr = FOURCC('v', 'm', ' ', ' '),
    gestaltMachineIcon = FOURCC('m', 'i', 'c', 'n'),
    gestaltMachineType = FOURCC('m', 'a', 'c', 'h'),
    gestaltROMSize = FOURCC('r', 'o', 'm', ' '),
    gestaltROMVersion = FOURCC('r', 'o', 'm', 'v'),
    gestaltSystemVersion = FOURCC('s', 'y', 's', 'v'),

    gestaltNativeCPUtype = FOURCC('c', 'p', 'u', 't'),
    gestaltSysArchitecture = FOURCC('s', 'y', 's', 'a'),
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
