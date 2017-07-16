#if !defined(_RSYS_OPTIONS_H_)
#define _RSYS_OPTIONS_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: options.h 63 2004-12-24 18:19:43Z ctm $
 */

#ifdef __cplusplus
using namespace Executor;
extern "C" {
#endif
typedef struct {
    LONGINT first;
    LONGINT second;
} pair_t;

extern pair_t ROMlib_ScreenSize;
extern pair_t ROMlib_MacSize;
extern pair_t ROMlib_ScreenLocation;
extern int32 ROMlib_options;
extern char *ROMlib_WindowName;
extern char *ROMlib_Comments;
extern int ROMlib_desired_bpp;

#define ROMLIB_NOCLOCK_BIT		(1 <<  0)
#define ROMLIB_DEBUG_BIT		(1 <<  1)
#define ROMLIB_REFRESH_BIT		(1 <<  2)
#define ROMLIB_DIRTY_VARIANT_BIT	(1 <<  3)
/*
 * NOTE: Don't change ROMLIB_STRIPADDRESSHACK_BIT.  It is assumed to be 1 << 4
 * in __StripAddress in stubs.s
 */
#define ROMLIB_STRIPADDRESSHACK_BIT	(1 <<  4)
#define ROMLIB_BLIT_OFTEN_BIT		(1 <<  5)
#define ROMLIB_BLIT_OS_BIT		(1 <<  6)
#define ROMLIB_BLIT_TRAP_BIT		(1 <<  7)
#define ROMLIB_PRETENDSOUND_BIT		(1 <<  8)
#define ROMLIB_PASSPOSTSCRIPT_BIT	(1 <<  9)
#define ROMLIB_NEWLINETOCR_BIT		(1 << 10)
#define ROMLIB_DIRECTDISKACCESS_BIT	(1 << 11)

#define ROMLIB_ACCELERATED_BIT		(1 << 12)
#define ROMLIB_SOUNDOFF_BIT		(1 << 13)
#define ROMLIB_SOUNDON_BIT		(1 << 14)
#define ROMLIB_NOWARN32_BIT		(1 << 15)
#define ROMLIB_FLUSHOFTEN_BIT		(1 << 16)

enum
{
  ROMLIB_PRETEND_HELP_BIT    = (1 << 17),
  ROMLIB_PRETEND_EDITION_BIT = (1 << 18),
  ROMLIB_PRETEND_SCRIPT_BIT  = (1 << 19),
  ROMLIB_PRETEND_ALIAS_BIT   = (1 << 20),

  ROMLIB_TEXT_DISABLE_BIT    = (1 << 21),
  ROMLIB_NOPREFS_BIT         = (1 << 22),
  ROMLIB_NOLOWER_BIT         = (1 << 23),
  ROMLIB_PRINTING_HACK_BIT   = (1 << 24),
  ROMLIB_DISPOSHANDLE_HACK_BIT = (1<<25),
  ROMLIB_NOSUSPEND_BIT       = (1 << 26),
  ROMLIB_CLOSE_IS_QUIT_BIT   = (1 << 27),
  ROMLIB_RECT_SCREEN_BIT     = (1 << 28),
};

#define INITIALPAIRVALUE	(-1000)
#ifdef __cplusplus
}
#endif
#endif
