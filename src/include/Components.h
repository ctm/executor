#if !defined(__COMPONENTS__)
#define __COMPONENTS__

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
typedef struct ComponentRecord
{
    LONGINT data[1];
} ComponentRecord;

typedef ComponentRecord *Component;

typedef struct ComponentInstanceRecord
{
    LONGINT data[1];
} ComponentInstanceRecord;

typedef ComponentInstanceRecord *ComponentInstance;
}
#endif
