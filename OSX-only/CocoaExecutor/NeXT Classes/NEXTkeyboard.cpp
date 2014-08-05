#include "rsys/common.h"
#define Point NeXT_Point
#include "IOKit/hidsystem/IOHIDTypes.h"
#include "IOKit/hidsystem/event_status_driver.h"
#undef Point
#include "rsys/keyboards.h"

#warning TODO: Update/replace with more modern code!

using namespace Executor;

keyboard_enum_t Executor::ROMlib_get_keyboard_type( void )
{
	NXEventHandle handle;
	NXEventSystemDevice	dev[NX_EVS_DEVICE_MAX];
	unsigned int cnt = NX_EVS_DEVICE_INFO_COUNT, i;
	int interface, id;
	keyboard_enum_t retval;
	
	if ( (handle = NXOpenEventStatus()) == 0 )
		return no_keyboard;
	NXEventSystemInfo( handle, NX_EVS_DEVICE_INFO, (int *)dev, &cnt );
	NXCloseEventStatus( handle );
	interface = -1;
	id = 0;
	for (i = 0; i < cnt/(sizeof (NXEventSystemDevice)/sizeof (int)); ++i) {
		if ( dev[i].dev_type == NX_EVS_DEVICE_TYPE_KEYBOARD ) {
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
