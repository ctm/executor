/*
 * Copyright 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: glue.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "MemoryMgr.h"

#define CFROMP(cp, pp)							\
    (BlockMove((Ptr) pp+1, (Ptr) cp, (Size) (unsigned char) pp[0]),	\
     cp[(unsigned char) pp[0]] = 0,			    		\
     cp)

#if !defined (__STDC__)
extern StringPtr ROMlib_PFROMC();
#else /* __STDC__ */
extern StringPtr ROMlib_PFROMC(StringPtr pp, char *cp, Size len);
#endif /* __STDC__ */

#define PASCALSTR(x, len) \
    (len = strlen(x), ROMlib_PFROMC(alloca(len+1), x, len))
