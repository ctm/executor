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
// ### Struct needs manual conversion to GUEST<...>
//   COMMONFSQUEUEDEFS;
typedef struct PACKED
{
  COMMONFSQUEUEDEFS;
  INTEGER ioRefNum;
  INTEGER csCode;
  PACKED_MEMBER(Ptr, csParam);
} VDParamBlock;

typedef VDParamBlock *VDParamBlockPtr;


struct VDEntryRecord : GuestStruct {
    GUEST< Ptr> csTable;
    GUEST< INTEGER> csStart;
    GUEST< INTEGER> csCount;
};

typedef VDEntryRecord *VDEntRecPtr;


struct VDGammaRecord : GuestStruct {
    GUEST< Ptr> csGTable;
};

typedef VDGammaRecord *VDGamRecPtr;


struct VDPgInfo : GuestStruct {
    GUEST< INTEGER> csMode;
    GUEST< LONGINT> csData;
    GUEST< INTEGER> csPage;
    GUEST< Ptr> csBaseAddr;
};

typedef VDPgInfo *VDPgInfoPtr;


struct VDFlagRec : GuestStruct {
    GUEST< SignedByte> flag;
};

typedef VDFlagRec *VDFlagPtr;


struct VDDefModeRec : GuestStruct {
    GUEST< SignedByte> spID;
};

typedef VDDefModeRec *VDDefModePtr;
}

#endif /* !__VDRIVER__ */
