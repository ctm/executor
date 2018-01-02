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

#if 0
#if !defined(VBLQueue)
extern QHdr 	VBLQueue;
#endif
#endif

extern void ROMlib_clockonoff(LONGINT onoroff);
extern OSErrRET VInstall(VBLTaskPtr vtaskp);
extern OSErrRET VRemove(VBLTaskPtr vtaskp);
extern QHdrPtr GetVBLQHdr(void);
extern OSErrRET SlotVInstall(VBLTaskPtr vtaskp, INTEGER slot);
extern OSErrRET SlotVRemove(VBLTaskPtr vtaskp, INTEGER slot);
}
#endif /* __VRETRACE__ */
