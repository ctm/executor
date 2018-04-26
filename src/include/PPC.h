#if !defined(_PPC_H_)
#define _PPC_H_

/* Copyright 1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{

typedef int16_t PPCPortKinds;
typedef int16_t PPCLocationKind;

typedef struct EntityName
{
  Str32 objStr;
  Str32 typeStr;
  Str32 zoneStr;
} EntityName;

struct PPCXTIAddress
{
    GUEST_STRUCT;
    int16_t fAddressType;
    uint8_t fAddress[96];
};
struct PPCAddrRec
{
    GUEST_STRUCT;
    uint8_t Reserved[3];
    uint8_t xtiAddrLen;
    PPCXTIAddress xtiAddr;
};

struct LocationNameRec
{
    GUEST_STRUCT;
    GUEST<PPCLocationKind> locationKindSelector;

    union {
        GUEST<EntityName> npbEntity;
        GUEST<Str32> npbType;
        GUEST<PPCAddrRec> xtiType;
    } u;
};

typedef struct PPCPortRec
{
    GUEST_STRUCT;
    GUEST<ScriptCode> nameScript;
    GUEST<Str32> name;

    GUEST<PPCPortKinds> portKindsSelector;

    union {
        GUEST<Str32> portTypeStr;
        struct
        {
            GUEST<OSType> creator;
            GUEST<OSType> type;
        } port;
    } u;
} * PPCPortPtr;
}

#endif /* !_PPC_H_ */
