#if !defined (_MEMORY_MGR_H_)
#define _MEMORY_MGR_H_

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: MemoryMgr.h 63 2004-12-24 18:19:43Z ctm $
 */


#define memFullErr	(-108)
#define memLockedErr	(-117)
#define memPurErr	(-112)
#define memWZErr	(-111)
enum { memAZErr	 = -113 };
#define nilHandleErr	(-109)

#define memROZErr	(-99)
#define memAdrErr	(-110)
#define memAZErr	(-113)
#define memPCErr	(-114)
#define memBCErr	(-115)
#define memSCErr	(-116)

typedef struct PACKED Zone
{
  PACKED_MEMBER(Ptr, bkLim);
  PACKED_MEMBER(Ptr, purgePtr);
  PACKED_MEMBER(Ptr, hFstFree);
  LONGINT zcbFree;
  PACKED_MEMBER(ProcPtr, gzProc);
  INTEGER moreMast;
  INTEGER flags;
  INTEGER cntRel;
  INTEGER maxRel;
  INTEGER cntNRel;
  INTEGER maxNRel;
  INTEGER cntEmpty;
  INTEGER cntHandles;
  LONGINT minCBFree;
  PACKED_MEMBER(ProcPtr, purgeProc);
  PACKED_MEMBER(Ptr, sparePtr);
  PACKED_MEMBER(Ptr, allocPtr);
  INTEGER heapData;
} Zone;
typedef Zone *THz;

MAKE_HIDDEN(THz);

#if !defined (MemErr)
extern int16 MemErr;
extern HIDDEN_Ptr 	MemTop_H;
extern HIDDEN_Ptr 	BufPtr_H;
extern HIDDEN_Ptr 	HeapEnd_H;
extern HIDDEN_THz 	TheZone_H;
extern HIDDEN_Ptr 	ApplLimit_H;
extern HIDDEN_THz 	SysZone_H;
extern HIDDEN_THz 	ApplZone_H;
extern HIDDEN_Ptr 	ROMBase_H;
extern HIDDEN_Ptr 	heapcheck_H;
extern HIDDEN_Handle 	GZRootHnd_H;
extern HIDDEN_ProcPtr 	IAZNotify_H;
extern HIDDEN_Ptr 	CurrentA5_H;
extern HIDDEN_Ptr 	CurStackBase_H;
extern Byte 	Scratch20[20];
extern LONGINT 	Lo3Bytes;
extern LONGINT 	MinStack;
extern LONGINT 	DefltStack;
extern Byte 	ToolScratch[8];
extern Byte 	Scratch8[8];
extern LONGINT 	OneOne;
extern LONGINT 	MinusOne;
extern Byte 	ApplScratch[12];
#endif

#define MemTop	(MemTop_H.p)
#define BufPtr	(BufPtr_H.p)
#define HeapEnd	(HeapEnd_H.p)
#define TheZone	(TheZone_H.p)
#define ApplLimit	(ApplLimit_H.p)
#define SysZone		(SysZone_H.p)
#define ApplZone	(ApplZone_H.p)
#define ROMBase		(ROMBase_H.p)
#define heapcheck	(heapcheck_H.p)
#define GZRootHnd	(GZRootHnd_H.p)
#define IAZNotify	(IAZNotify_H.p)
#define CurrentA5	(CurrentA5_H.p)
#define CurStackBase	(CurStackBase_H.p)

/* traps which can have a `sys' or `clear' bit set */

#define NewEmptyHandle()	(_NewEmptyHandle_flags (FALSE))
#define NewEmptyHandleSys()	(_NewEmptyHandle_flags (TRUE))
extern Handle _NewEmptyHandle_flags (boolean_t sys_p);

#define NewHandle(size)		(_NewHandle_flags (size, FALSE, FALSE))
#define NewHandleSys(size)	(_NewHandle_flags (size, TRUE, FALSE))
#define NewHandleClear(size)	(_NewHandle_flags (size, FALSE, TRUE))
#define NewHandleSysClear(size)	(_NewHandle_flags (size, TRUE, TRUE))
extern Handle _NewHandle_flags (Size size, boolean_t sys_p, boolean_t clear_p);

#define RecoverHandle(ptr)	(_RecoverHandle_flags (ptr, FALSE))
#define RecoverHandleSys(ptr)	(_RecoverHandle_flags (ptr, TRUE))
extern Handle _RecoverHandle_flags (Ptr p, boolean_t sys_p);

#define NewPtr(size)		(_NewPtr_flags (size, FALSE, FALSE))
#define NewPtrSys(size)		(_NewPtr_flags (size, TRUE, FALSE))
#define NewPtrClear(size)	(_NewPtr_flags (size, FALSE, TRUE))
#define NewPtrSysClear(size)	(_NewPtr_flags (size, TRUE, TRUE))
extern Ptr _NewPtr_flags (Size size, boolean_t sys_p, boolean_t clear_p);

#define FreeMem()		(_FreeMem_flags (FALSE))
#define FreeMemSys()		(_FreeMem_flags (TRUE))
extern int32 _FreeMem_flags (boolean_t sys_p);

#define MaxMem(growp)		(_MaxMem_flags (growp, FALSE))
#define MaxMemSys(growp)	(_MaxMem_flags (growp, TRUE))
extern Size _MaxMem_flags (Size *growp, boolean_t sys_p);

#define CompactMem(needed)	(_CompactMem_flags (needed, FALSE))
#define CompactMemSys(needed)	(_CompactMem_flags (needed, TRUE))
extern Size _CompactMem_flags (Size sizeneeded, boolean_t sys_p);

#define ResrvMem(needed)	(_ResrvMem_flags (needed, FALSE))
#define ResrvMemSys(needed)	(_ResrvMem_flags (needed, TRUE))
extern void _ResrvMem_flags (Size needed, boolean_t sys_p);

#define PurgeMem(needed)	(_PurgeMem_flags (needed, FALSE))
#define PurgeMemSys(needed)	(_PurgeMem_flags (needed, TRUE))
extern void _PurgeMem_flags (Size needed, boolean_t sys_p);

#define MaxBlock()		(_MaxBlock_flags (FALSE))
#define MaxBlockSys()		(_MaxBlock_flags (TRUE))
extern Size _MaxBlock_flags (boolean_t sys_p);

#define PurgeSpace(totalp, congtigp)		\
  (_PurgeSpace_flags (totalp, contigp, FALSE))
#define PurgeSpaceSys(totalp, congtigp)		\
  (_PurgeSpace_flags (totalp, contigp, TRUE))
extern void _PurgeSpace_flags (Size *totalp, Size *contigp, boolean_t sys_p); 

/* ### cliff bogofunc; should go away */
extern void ROMlib_installhandle (Handle sh, Handle dh);

typedef enum
{
  offset_no_change,	/* for when we call InitZone a second time */
  offset_none,		/* use page 0 for page 0 */
  offset_8k,		/* offset is a fixed 8192 */
  offset_big,		/* offset is to big syszone+applzone block */
} offset_enum;

extern void ROMlib_InitZones (offset_enum which);
extern OSErr MemError (void);

extern SignedByte HGetState (Handle h);
extern void HSetState (Handle h, SignedByte flags);
extern void HLock (Handle h);
extern void HUnlock (Handle h);
extern void HPurge (Handle h);
extern void HNoPurge (Handle h);
extern void HSetRBit (Handle h);
extern void HClrRBit (Handle h);
extern void InitApplZone (void);
extern void SetApplBase (Ptr newbase);
extern void MoreMasters (void);
extern void InitZone (ProcPtr pGrowZone, int16 cMoreMasters, 
		      Ptr limitPtr, THz startPtr);
extern THz GetZone (void);
extern void SetZone (THz hz);
extern void DisposHandle (Handle h); 
extern Size GetHandleSize (Handle h); 
extern void SetHandleSize (Handle h, Size newsize); 
extern THz HandleZone (Handle h); 
extern void ReallocHandle (Handle h, Size size); 
extern void DisposPtr (Ptr p); 
extern Size GetPtrSize (Ptr p); 
extern void SetPtrSize (Ptr p, Size newsize); 
extern THz PtrZone (Ptr p); 
extern void BlockMove (Ptr src, Ptr dst, Size cnt); 
extern void BlockMoveData (Ptr src, Ptr dst, Size cnt); 
extern void MaxApplZone (void); 
extern void MoveHHi (Handle h); 
extern void SetApplLimit (Ptr newlimit); 
extern void SetGrowZone (ProcPtr newgz); 
extern void EmptyHandle (Handle h); 
extern THz SystemZone (void); 
extern THz ApplicZone (void); 
extern Size StackSpace (void); 

/* temporary memory functions; see tempmem.c */
extern pascal trap int32 C_TempFreeMem (void);
extern pascal trap Size C_TempMaxMem (Size *grow);
extern pascal trap Ptr C_TempTopMem (void);
extern pascal trap Handle C_TempNewHandle (Size logical_size, OSErr *result_code);
extern pascal trap void C_TempHLock (Handle h, OSErr *result_code);
extern pascal trap void C_TempHUnlock (Handle h, OSErr *result_code);
extern pascal trap void C_TempDisposeHandle (Handle h, OSErr *result_code);

#endif /* _MEMORY_MGR_H_ */
