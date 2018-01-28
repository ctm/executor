#if !defined(__VRETRACE__)
#define __VRETRACE__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{

enum
{
    qErr = (-1),
    vTypErr = (-2),
};

struct VBLTask
{
    GUEST_STRUCT;
    GUEST<QElemPtr> qLink;
    GUEST<INTEGER> qType;
    GUEST<ProcPtr> vblAddr;
    GUEST<INTEGER> vblCount;
    GUEST<INTEGER> vblPhase;
};
typedef VBLTask *VBLTaskPtr;

const LowMemGlobal<QHdr> VBLQueue { 0x160 }; // VRetraceMgr IMII-352 (true);
const LowMemGlobal<ProcPtr> JVBLTask { 0xD28 }; // VRetraceMgr IMV (false);

extern void ROMlib_clockonoff(LONGINT onoroff);

extern OSErr VInstall(VBLTaskPtr vtaskp);
REGISTER_TRAP2(VInstall, 0xA033, D0(A0));
extern OSErr VRemove(VBLTaskPtr vtaskp);
REGISTER_TRAP2(VRemove, 0xA034, D0(A0));

extern QHdrPtr GetVBLQHdr(void);
extern OSErr SlotVInstall(VBLTaskPtr vtaskp, INTEGER slot);
REGISTER_TRAP2(SlotVInstall, 0xA06F, D0(A0,D0));
extern OSErr SlotVRemove(VBLTaskPtr vtaskp, INTEGER slot);
REGISTER_TRAP2(SlotVRemove, 0xA070, D0(A0,D0));
}
#endif /* __VRETRACE__ */
