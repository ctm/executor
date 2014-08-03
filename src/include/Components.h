#if !defined (__COMPONENTS__)
#define __COMPONENTS__

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: Components.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
typedef struct ComponentRecord
{
  LONGINT data[1];
}
ComponentRecord;

typedef ComponentRecord *Component;

typedef struct ComponentInstanceRecord
{
  LONGINT data[1];
}
ComponentInstanceRecord;

typedef ComponentInstanceRecord *ComponentInstance;
}
#endif
