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
REGISTER_TRAP2(ADBReInit, 0xA07B, void ());

extern OSErr ADBOp(Ptr data, ProcPtr procp, Ptr buffer, INTEGER command);

extern INTEGER CountADBs(void);
REGISTER_TRAP2(CountADBs, 0xA077, D0());
extern OSErr GetIndADB(ADBDataBlock *adbp, INTEGER index);
REGISTER_TRAP2(GetIndADB, 0xA078, D0(A0,D0));
extern OSErr GetADBInfo(ADBDataBlock *adbp, INTEGER address);
REGISTER_TRAP2(GetADBInfo, 0xA079, D0(A0,D0));
extern OSErr SetADBInfo(ADBSetInfoBlock *adbp, INTEGER address);
REGISTER_TRAP2(SetADBInfo, 0xA07A, D0(A0,D0));
}

#endif /* !_ADB_H_ */
