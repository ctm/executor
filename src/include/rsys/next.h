#if !defined(_RSYS_NEXT_H_)
#define _RSYS_NEXT_H_

/*
 * Copyright 1991 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: next.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "rsys/smash.h"

/*
 * make sure there are no duplicate characters in INFO_START_STRING
 */

#define INFO_START_STRING "Run Paiot,-Svp."
/* 1234567890123456 */

#define SETUPA5     \
    LONGINT savea5; \
    savea5 = EM_A5; \
    EM_A5 = CL(guest_cast<LONGINT>(CurrentA5))

#define RESTOREA5 EM_A5 = savea5

#endif
