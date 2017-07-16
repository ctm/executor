#if !defined (__VRETRACE__)
#define __VRETRACE__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: VRetraceMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {

#define qErr	(-1)
#define vTypErr	(-2)

typedef struct PACKED {
  PACKED_MEMBER(QElemPtr, qLink);
  INTEGER qType;
  PACKED_MEMBER(ProcPtr, vblAddr);
  INTEGER vblCount;
  INTEGER vblPhase;
} VBLTask;
typedef VBLTask *VBLTaskPtr;

#if !defined (VBLQueue)
extern QHdr 	VBLQueue;
#endif

extern void ROMlib_clockonoff( LONGINT onoroff ); 
extern trap OSErrRET VInstall( VBLTaskPtr vtaskp ); 
extern trap OSErrRET VRemove( VBLTaskPtr vtaskp ); 
extern QHdrPtr GetVBLQHdr( void  ); 
extern trap OSErrRET SlotVInstall( VBLTaskPtr vtaskp, INTEGER slot );
extern trap OSErrRET SlotVRemove( VBLTaskPtr vtaskp, INTEGER slot );
}
#endif /* __VRETRACE__ */
