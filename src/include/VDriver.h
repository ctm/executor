#if !defined (__VDRIVER__)
#define __VDRIVER__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: VDriver.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "FileMgr.h"

namespace Executor {
typedef struct PACKED
{
  COMMONFSQUEUEDEFS;
  INTEGER ioRefNum;
  INTEGER csCode;
  PACKED_MEMBER(Ptr, csParam);
} VDParamBlock;

typedef VDParamBlock *VDParamBlockPtr;


typedef struct PACKED
{
  PACKED_MEMBER(Ptr, csTable);
  INTEGER csStart;
  INTEGER csCount;
} VDEntryRecord;

typedef VDEntryRecord *VDEntRecPtr;


typedef struct PACKED
{
  PACKED_MEMBER(Ptr, csGTable);
} VDGammaRecord;

typedef VDGammaRecord *VDGamRecPtr;


typedef struct PACKED
{
  INTEGER csMode;
  LONGINT csData;
  INTEGER csPage;
  PACKED_MEMBER(Ptr, csBaseAddr);
} VDPgInfo;

typedef VDPgInfo *VDPgInfoPtr;


typedef struct PACKED
{
  SignedByte flag;
} VDFlagRec;

typedef VDFlagRec *VDFlagPtr;


typedef struct PACKED
{
  SignedByte spID;
} VDDefModeRec;

typedef VDDefModeRec *VDDefModePtr;
}

#endif /* !__VDRIVER__ */
