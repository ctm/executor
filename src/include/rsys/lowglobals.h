#if !defined(_LOWGLOBALS_H_)
#define _LOWGLOBALS_H_

/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
template<class T>
struct LowMemGlobal
{
    uint32_t address;
};

template<class T>
inline GUEST<T>& LM(LowMemGlobal<T> lm)
{
    return *(GUEST<T>*)(ROMlib_offset + lm.address);
}

// Declarations of low memory globals which haven't been put anywhere else
// yet follow. Whenever a low memory global clearly belongs to one manager/module,
// put it should live in the appropriate header.
// There is also a complete list of low memory globals in globals.cpp,
// which is not used but might be nice to have.

const LowMemGlobal<Ptr> nilhandle { 0x00 }; // rsys/misc MADEUP (true-b);
/*
 * NOTE: MacWrite starts writing longwords at location 0x80 for TRAPs
 */
const LowMemGlobal<LONGINT[10]> trapvectors { 0x80 }; // rsys/misc WHOKNOWS (true-b);
const LowMemGlobal<Ptr> dodusesit { 0xE4 }; // rsys/misc WHOKNOWS (true-b);
/*
 * Hypercard does a movel to this location.
 */
const LowMemGlobal<LONGINT> hyperlong { 0x1AA }; // rsys/misc WHOKNOWS (true-b);
/*
 * NOTE: mathones is a LONGINT that Mathematica looks at that contains -1
 * on a Mac+
 */
const LowMemGlobal<LONGINT> mathones { 0x282 }; // rsys/misc WHOKNOWS (true-b);
/*
 * NOTE: Theoretically ROM85 is mentioned in IMV, but I don't know where.
 * On a Mac+ the value 0x7FFF is stored there.
 * tim: It is at least on page IMV-328.
 */
const LowMemGlobal<INTEGER> ROM85 { 0x28E }; // MacTypes IMV-328 (true-b);
const LowMemGlobal<LONGINT> BufTgFNum { 0x2FC }; // DiskDvr IMII-212 (false);
const LowMemGlobal<INTEGER> BufTgFFlg { 0x300 }; // DiskDvr IMII-212 (false);
const LowMemGlobal<INTEGER> BufTgFBkNum { 0x302 }; // DiskDvr IMII-212 (false);
const LowMemGlobal<LONGINT> BufTgDate { 0x304 }; // DiskDvr IMII-212 (false);


const LowMemGlobal<INTEGER> MCLKPCmiss1 { 0x3A0 }; // MacLinkPC badaccess (true-b);
const LowMemGlobal<INTEGER> MCLKPCmiss2 { 0x3A6 }; // MacLinkPC badaccess (true-b);

/*
 * JFLUSH is a guess from disassembling some of Excel 3.0
 */
const LowMemGlobal<ProcPtr> JFLUSH { 0x6F4 }; // idunno guess (true-b);
const LowMemGlobal<ProcPtr> JResUnknown1 { 0x700 }; // idunno resedit (true-b);
const LowMemGlobal<ProcPtr> JResUnknown2 { 0x714 }; // idunno resedit (true-b);

/*
 * NOTE: The graphing program looks for a -1 in 0x952
 */
const LowMemGlobal<INTEGER> graphlooksat { 0x952 }; // rsys/misc WHOKNOWS (true-b);
/*
 * NOTE: MacWrite stores a copy of the trap address for LoadSeg in 954
 */
const LowMemGlobal<LONGINT> macwritespace { 0x954 }; // rsys/misc WHOKNOWS (true-b);
const LowMemGlobal<INTEGER> DSErrCode { 0xAF0 }; // MacTypes IMII-362 (true);
const LowMemGlobal<INTEGER> SCSIFlags { 0xB22 }; // uknown Private.a (true-b);
const LowMemGlobal<LONGINT> LastSPExtra { 0xB4C }; // rsys/misc WHOKNOWS (true-b);
const LowMemGlobal<LONGINT> lastlowglobal { 0x2000 }; // rsys/misc MadeUp (true-b);
}

#endif
