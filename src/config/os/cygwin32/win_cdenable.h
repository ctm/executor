/*
		cdenable.sys driver for Windows NT 4.0 and upwards
		11/15/1997 Lauri Pesonen
*/

// max read requests, if larger -> STATUS_INVALID_PARAMETER
#define CDENABLE_MAX_TRANSFER_SIZE (0x10000)

// A structure representing the instance information associated with
// a particular device
typedef struct _DEVICE_EXTENSION
{
    ULONG StateVariable;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

// Define the various device type values.  Note that values used by Microsoft
// Corporation are in the range 0-32767, and 32768-65535 are reserved for use
// by customers.
#define FILE_DEVICE_CDENABLE 0x00008301

// Target NT version, internal version
#define CDENABLE_CURRENT_VERSION 0x04000100

// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
#define CDENABLE_IOCTL_READ 0x830
#define CDENABLE_IOCTL_GET_VERSION 0x831

#define IOCTL_CDENABLE_READ CTL_CODE(FILE_DEVICE_CDENABLE, \
                                     CDENABLE_IOCTL_READ,  \
                                     METHOD_BUFFERED,      \
                                     FILE_ANY_ACCESS)
#define IOCTL_CDENABLE_GET_VERSION CTL_CODE(FILE_DEVICE_CDENABLE,       \
                                            CDENABLE_IOCTL_GET_VERSION, \
                                            METHOD_BUFFERED,            \
                                            FILE_ANY_ACCESS)
