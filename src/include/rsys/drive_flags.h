#if !defined(__rsys_drive_flags__)
#define __rsys_drive_flags__

namespace Executor {
typedef unsigned char drive_flags_t;
enum {
  DRIVE_FLAGS_LOCKED = 1,
  DRIVE_FLAGS_FIXED = 2,
  DRIVE_FLAGS_FLOPPY = 4
};
}

#endif /* !defined(__rsys_drive_flags__) */
