//
//  CFriendly.h
//  CocoaExecutor
//
//  Created by C.W. Betts on 7/30/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#ifndef CocoaExecutor_CFriendly_h
#define CocoaExecutor_CFriendly_h

#define getthecrc CB_getthecrc
short CB_getthecrc(ResType typ, long id1);
#define ROMlib_version_long CGet_ROMlib_version_long()
long CGet_ROMlib_version_long();
uint32 ROMlib_PrDrvrVers_Set(uint32);
#define CREATE_SYSTEM_VERSION(a, b, c) \
((((a) & 0xF) << 8) | (((b) & 0xF) << 4) | ((c) & 0xF))
#define ROMlib_delay CGet_ROMlib_delay()
int CGet_ROMlib_delay();
#define ROMlib_delayp CGet_ROMlib_delayp()
int *CGet_ROMlib_delayp();
#define ROMlib_refreshp CGet_ROMlib_refreshp()
int *CGet_ROMlib_refreshp();
#define ROMlib_add_to_gestalt_list CB_ROMlib_add_to_gestalt_list
void CB_ROMlib_add_to_gestalt_list(OSType selector, OSErr retval, uint32 new_value);

#endif
