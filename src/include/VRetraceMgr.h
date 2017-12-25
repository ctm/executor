#if !defined(__VRETRACE__)
#define __VRETRACE__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{

#define qErr (-1)
#define vTypErr (-2)

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
extern trap OSErrRET VInstall(VBLTaskPtr vtaskp);
extern trap OSErrRET VRemove(VBLTaskPtr vtaskp);
extern QHdrPtr GetVBLQHdr(void);
extern trap OSErrRET SlotVInstall(VBLTaskPtr vtaskp, INTEGER slot);
extern trap OSErrRET SlotVRemove(VBLTaskPtr vtaskp, INTEGER slot);
}
#endif /* __VRETRACE__ */
