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
struct VDParamBlock
{
  GUEST_STRUCT;
  COMMONFSQUEUEDEFS;
  GUEST<INTEGER> ioRefNum;
  GUEST<INTEGER> csCode;
  GUEST<Ptr> csParam;
};

typedef VDParamBlock *VDParamBlockPtr;


struct VDEntryRecord {
    GUEST_STRUCT;
    GUEST< Ptr> csTable;
    GUEST< INTEGER> csStart;
    GUEST< INTEGER> csCount;
};

typedef VDEntryRecord *VDEntRecPtr;


struct VDGammaRecord { GUEST_STRUCT;
    GUEST< Ptr> csGTable;
};

typedef VDGammaRecord *VDGamRecPtr;


struct VDPgInfo { GUEST_STRUCT;
    GUEST< INTEGER> csMode;
    GUEST< LONGINT> csData;
    GUEST< INTEGER> csPage;
    GUEST< Ptr> csBaseAddr;
};

typedef VDPgInfo *VDPgInfoPtr;


struct VDFlagRec { GUEST_STRUCT;
    GUEST< SignedByte> flag;
};

typedef VDFlagRec *VDFlagPtr;


struct VDDefModeRec { GUEST_STRUCT;
    GUEST< SignedByte> spID;
};

typedef VDDefModeRec *VDDefModePtr;
}

#endif /* !__VDRIVER__ */
