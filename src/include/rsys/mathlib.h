#if !defined(_rsys_mathlib_h_)
#define _rsys_mathlib_h_

#include "rsys/cfm.h"

extern OSErr ROMlib_GetMathLib (Str63 library, OSType arch,
				LoadFlags loadflags, ConnectionID *cidp,
				Ptr *mainaddrp, Str255 errName);


#endif
