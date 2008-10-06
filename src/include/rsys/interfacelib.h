#if !defined(_rsys_interfacelib_h_)
#define _rsys_interfacelib_h_

#include "rsys/cfm.h"

extern OSErr ROMlib_GetInterfaceLib (Str63 library, OSType arch,
				     LoadFlags loadflags, ConnectionID *cidp,
				     Ptr *mainaddrp, Str255 errName);


#endif
