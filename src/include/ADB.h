#if !defined(_ADB_H_)
#define _ADB_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
struct ADBDataBlock
{
    GUEST_STRUCT;
    GUEST<SignedByte> devType;
    GUEST<SignedByte> origADBAddr;
    GUEST<ProcPtr> dbServiceRtPtr;
    GUEST<Ptr> dbDataAreaAddr;
};

struct ADBSetInfoBlock
{
    GUEST_STRUCT;
    GUEST<ProcPtr> siServiceRtPtr;
    GUEST<Ptr> siDataAreaAddr;
};

const LowMemGlobal<Byte> KbdLast { 0x218 }; // QuickDraw IMV-367 (false);
const LowMemGlobal<Byte> KbdType { 0x21E }; // QuickDraw IMV-367 (false);

extern void ADBReInit(void);
extern OSErr ADBOp(Ptr data, ProcPtr procp, Ptr buffer, INTEGER command);
extern INTEGER CountADBs(void);
extern OSErr GetIndADB(ADBDataBlock *adbp, INTEGER index);
extern OSErr GetADBInfo(ADBDataBlock *adbp, INTEGER address);
extern OSErr SetADBInfo(ADBSetInfoBlock *adbp, INTEGER address);
}

#endif /* !_ADB_H_ */
