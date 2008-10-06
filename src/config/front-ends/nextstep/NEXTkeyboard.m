#include "rsys/common.h"
#define Point NeXT_Point
#import <drivers/event_status_driver.h>
#undef Point
#include "rsys/keyboards.h"

keyboard_enum_t ROMlib_get_keyboard_type( void )
{
	NXEventHandle handle;
	NXEventSystemDevice	dev[NX_EVS_DEVICE_MAX];
	unsigned int cnt, i;
	int interface, id, retval;
	
	if ( (handle = NXOpenEventStatus()) == NULL )
/*-->*/		return NO;
	cnt = NX_EVS_DEVICE_INFO_COUNT;
	NXEventSystemInfo( handle, NX_EVS_DEVICE_INFO, (int *)dev, &cnt );
	NXCloseEventStatus( handle );
	interface = -1;
	id = 0;
	for (i = 0; i < cnt/(sizeof (NXEventSystemDevice)/sizeof (int)); ++i)
	{
		if ( dev[i].dev_type == NX_EVS_DEVICE_TYPE_KEYBOARD )
		{
			interface = dev[i].interface;
			id = dev[i].id;
			break;
		}
	}
	switch (interface) {
	default:
	    retval = default_keyboard;
	    break;
	case NX_EVS_DEVICE_INTERFACE_ADB:
	    retval = adb_keyboard;
	    break;
	case NX_EVS_DEVICE_INTERFACE_ACE:
	    retval = pc_keyboard;
	    break;
	}
	return retval;
}
