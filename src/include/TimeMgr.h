#if !defined (__TIMEMGR__)
#define __TIMEMGR__

/*
 * Copyright 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: TimeMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
typedef struct PACKED {
  PACKED_MEMBER(QElemPtr, qLink);
  INTEGER	qType;
  PACKED_MEMBER(ProcPtr, tmAddr);
  LONGINT	tmCount;	/* I don't trust IMIV-301 */
} TMTask;


/* DO NOT DELETE THIS LINE */
#if !defined (__STDC__)
extern void InsTime(); 
extern void RmvTime(); 
extern void PrimeTime(); 
#else /* __STDC__ */
extern void InsTime( QElemPtr taskp ); 
extern void RmvTime( QElemPtr taskp ); 
extern void PrimeTime( QElemPtr taskp, LONGINT count ); 
#endif /* __STDC__ */
}
#endif /* __TIMEMGR__ */
