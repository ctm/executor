#if !defined (_ADB_H_)
#define _ADB_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: ADB.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
struct ADBDataBlock : GuestStruct {
    GUEST< SignedByte> devType;
    GUEST< SignedByte> origADBAddr;
    GUEST< Ptr> dbServiceRtPtr;
    GUEST< Ptr> dbDataAreaAddr;
};

struct ADBSetInfoBlock : GuestStruct {
    GUEST< Ptr> siServiceRtPtr;
    GUEST< Ptr> siDataAreaAddr;
};

extern void ADBReInit (void);
extern OSErr ADBOp (Ptr data, ProcPtr procp, Ptr buffer, INTEGER command);
extern INTEGER CountADBs (void);
extern OSErr GetIndADB (ADBDataBlock *adbp, INTEGER index);
extern OSErr GetADBInfo (ADBDataBlock *adbp, INTEGER address);
extern OSErr SetADBInfo (ADBSetInfoBlock *adbp, INTEGER address);
}

#endif /* !_ADB_H_ */
