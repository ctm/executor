#if !defined(_RSYS_NEXT_H_)
#define _RSYS_NEXT_H_

/*
 * Copyright 1991 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#define SETUPA5     \
    LONGINT savea5; \
    savea5 = EM_A5; \
    EM_A5 = CL(guest_cast<LONGINT>(CurrentA5))

#define RESTOREA5 EM_A5 = savea5

#endif
