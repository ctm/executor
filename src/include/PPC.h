#if !defined(_PPC_H_)
#define _PPC_H_

/* Copyright 1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: PPC.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor
{

typedef int16_t PPCPortKinds;
typedef int16_t PPCLocationKind;

typedef struct EntityName
{
    /* #### bogus */
} EntityName;

struct LocationNameRec
{
    GUEST_STRUCT;
    GUEST<PPCLocationKind> locationKindSelector;

    union {
        GUEST<EntityName> npbEntity;
        GUEST<Str32> npbType;
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
