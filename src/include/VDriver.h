#if !defined (__VDRIVER__)
#define __VDRIVER__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: VDriver.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "FileMgr.h"

typedef struct
{
  COMMONFSQUEUEDEFS	PACKED;
  INTEGER ioRefNum	PACKED;
  INTEGER csCode	PACKED;
  Ptr csParam		PACKED_P;
} VDParamBlock;

typedef VDParamBlock *VDParamBlockPtr;


typedef struct
{
  Ptr csTable		PACKED_P;
  INTEGER csStart	PACKED;
  INTEGER csCount	PACKED;
} VDEntryRecord;

typedef VDEntryRecord *VDEntRecPtr;


typedef struct
{
  Ptr csGTable	PACKED_P;
} VDGammaRecord;

typedef VDGammaRecord *VDGamRecPtr;


typedef struct
{
  INTEGER csMode	PACKED;
  LONGINT csData	PACKED;
  INTEGER csPage	PACKED;
  Ptr csBaseAddr	PACKED_P;
} VDPgInfo;

typedef VDPgInfo *VDPgInfoPtr;


typedef struct
{
  SignedByte flag	PACKED;
} VDFlagRec;

typedef VDFlagRec *VDFlagPtr;


typedef struct
{
  SignedByte spID;	PACKED;
} VDDefModeRec;

typedef VDDefModeRec *VDDefModePtr;


#endif /* !__VDRIVER__ */
