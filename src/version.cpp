/* Copyright 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_version[] =
		"$Id: version.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/version.h"
#include "rsys/gestalt.h"
#include "rsys/prefs.h"

#include "ResourceMgr.h"
#include "MemoryMgr.h"

using namespace Executor;

/* A simple version number like "1.99q" */
const char ROMlib_executor_version[] = EXECUTOR_VERSION;

/* A descriptive string like "Executor 1.99q DEMO" */
const char *ROMlib_executor_full_name = "Executor " EXECUTOR_VERSION
#if defined (RELEASE_DEMO)
" DEMO"
#elif defined (RELEASE_INTERNAL)
" DEBUG"
#endif
#if defined (EXPERIMENTAL)
" EXPERIMENTAL"
#endif
;

#if defined (CYGWIN32) || defined (MSDOS)

/*
 * No snprintf, so we just ignore the length.
 * This is very ugly, but the strings we're crafting won't overflow the
 * 1024 byte buffer we're using.  ICK.
 */

#define snprintf(str, len, fmt, args...) sprintf (str, fmt , args)
#endif

#define UPDATE_RES(typ, id, fmt, args...)		\
({							\
  int len;						\
  char str[1024];					\
  GUEST<INTEGER> save_map;					\
  Handle h;						\
  OSType _typ;						\
							\
  _typ = (typ);						\
  len = snprintf (str, sizeof str, fmt , args);		\
  save_map = CurMap;					\
  CurMap = SysMap;					\
  h = C_Get1Resource (_typ, (id));			\
  CurMap = save_map;					\
  LoadResource (h);					\
  HUnlock (h); /* safe to do -- app not running */	\
  SetHandleSize (h, len);				\
  memcpy (STARH (h), str, len);				\
  if (_typ == TICK ("STR "))				\
    *(char *) STARH (h) = len-1;			\
  else if (_typ == TICK ("vers"))			\
    ((char *) STARH (h))[12] = len-13;			\
})

#define UPDATE_VERS(major, minor, rev, vers, fmt, args...)	\
({								\
  int _major;							\
  int _minor;							\
  int _rev;							\
								\
  _major = (major);						\
  _minor = (minor);						\
  _rev = (rev);							\
								\
  UPDATE_RES (TICK("vers"), vers, "%c%c\x80%c%c%c\x5%d.%d.%d"	\
	      fmt , _major, ((_minor << 4)| _rev), 0, 0, 0 ,	\
	      _major, _minor, _rev , args);			\
})

PUBLIC void
ROMlib_set_system_version (uint32 version)
{
  static uint32 old_version = -1;

  if (version != old_version)
    {
      int major, minor, rev;
      enum { MINOR_MASK = 0xF, REV_MASK = 0xF };

      system_version = version;
      SysVersion = CW (version);

      major = version >> 8;
      minor = (version >> 4) & MINOR_MASK;
      rev   = (version     ) & REV_MASK;
      gestalt_set_system_version (version);

      UPDATE_RES (TICK("STR "), 0, "XMacintosh System Software, "
		  "version %d.%d\r\r\r\xa9 ARDI 1986-2000\r"
		  "All rights reserved.", major, minor);

      UPDATE_VERS (major, minor, rev, 1, "X%d.%d.%d \xa9 ARDI 1986-00",
		   major, minor, rev);

      UPDATE_VERS (major, minor, rev, 2, "XSystem %d.%d Version %d.%d.%d",
		   major, minor, major, minor, rev);

      old_version = version;
    }
}
