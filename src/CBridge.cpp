//
//  CBridge.cpp
//  CocoaExecutor
//
//  Created by C.W. Betts on 7/30/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#include "rsys/common.h"
#include "rsys/options.h"
#include "rsys/prefs.h"
#include "rsys/crc.h"
#include "rsys/parse.h"
#include "rsys/parseopt.h"
#include "rsys/gestalt.h"
#include "rsys/launch.h"
#include "rsys/print.h"
#include "rsys/version.h"

using namespace Executor;

extern "C" {
#include "CFriendly.h"
}

#undef getthecrc
#undef ROMlib_version_long
#undef ROMlib_delay
#undef ROMlib_add_to_gestalt_list

short CB_getthecrc(ResType typ, long id1)
{
	return getthecrc(typ, id1);
}

long CGet_ROMlib_version_long()
{
	return ROMlib_version_long;
}

uint32 ROMlib_PrDrvrVers_Set(uint32 toSet)
{
	ROMlib_PrDrvrVers = toSet;
	return ROMlib_PrDrvrVers;
}

int CGet_ROMlib_delay()
{
	return ROMlib_delay;
}

int *CGet_ROMlib_delayp()
{
	return &ROMlib_delay;
}

int *CGet_ROMlib_refreshp()
{
	return &ROMlib_refresh;
}

void CB_ROMlib_add_to_gestalt_list(OSType selector, OSErr retval, uint32 new_value)
{
	ROMlib_add_to_gestalt_list(selector, retval, new_value);
}
