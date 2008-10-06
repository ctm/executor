#include "rsys/common.h"

#import <sound/sound.h>

#ifndef OPENSTEP

#import <soundkit/soundkit.h>
#import <appkit/Application.h>

#else /* OPENSTEP */

#import <SoundKit/Sound.h>
#import <AppKit/NSApplication.h>

#endif /* OPENSTEP */

#include "rsys/soundopts.h"
#include "MemoryMgr.h"
#include "rsys/next.h"

#if defined(mc68000) && (!defined(NX_CURRENT_COMPILER_RELEASE) || \
		(NX_CURRENT_COMPILER_RELEASE < NX_COMPILER_RELEASE_3_0))
#include <cthreads.h>
#else
#include <mach/cthreads.h>
#endif

#include "rsys/syn68k_public.h"

/*
 * NOTE: this is a simplistic implementation that doesn't use double
 *	 buffering.
 */

void ROMlib_callcompletion( void *chanp )
{
}

void ROMlib_outbuffer( char *buf, LONGINT nsamples, LONGINT rate, void *chanp)
{
}
