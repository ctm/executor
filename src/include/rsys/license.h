#if !defined(_RSYS_LICENSE_H_)
#define _RSYS_LICENSE_H_

/*
 * Copyright 1994 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: license.h 63 2004-12-24 18:19:43Z ctm $
 */

#include <DialogMgr.h>

enum { GENERIC_COMPLAINT_ID = -3000 };

typedef enum { no_license, old_license, register_only } which_license_t;

extern void dolicense ( void );
extern which_license_t ROMlib_dolicense;

extern INTEGER C_ROMlib_licensefilt(DialogPeek dp, EventRecord *evtp,
				  INTEGER *ith);
extern void ROMlib_writenameorgkey(char *name, char *org, char *key);
extern void protectus(long serialnumber, long max);

#endif
