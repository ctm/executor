#if !defined(_RSYS_GESTALT_H_)
#define _RSYS_GESTALT_H_

namespace Executor
{
#if defined(USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)
typedef uint32_t OSType;
typedef uint32_t OSErr;
#endif

enum
{
    DONGLE_GESTALT = 0xb7d20e84,
};

extern void replace_physgestalt_selector(OSType selector, uint32_t new_value);
extern void ROMlib_clear_gestalt_list(void);
extern void ROMlib_add_to_gestalt_list(OSType selector, OSErr retval,
                                       uint32_t new_value);

/* GhostScript DLL version number */
enum
{
    gestaltGhostScriptVersion = FOURCC('g', 'o', 's', 't')
};

#if !defined(USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)
/*
 * Copyright 1996-1999 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

/* Executor Version as string */
enum
{
    gestaltExecutorVersionString = FOURCC('x', 'q', 't', 'r')
};

/* screen size */
enum
{
    gestaltScreenSize = FOURCC('s', 'c', 'r', 'n')
};

enum
{
    gestaltPHYSICAL = FOURCC('M', 'U', 'L', '8')
};

enum
{
    physicalUndefSelectorErr = -15551
};

extern OSErrRET C_PhysicalGestalt(OSType selector, GUEST<LONGINT> *responsep);
PASCAL_FUNCTION(PhysicalGestalt);

extern void gestalt_set_system_version(uint32_t version);
extern void gestalt_set_memory_size(uint32_t size);
extern void gestalt_set_cpu_type(uint32_t type);

#endif
}
#endif /* !_RSYS_GESTALT_H_ */
