/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_fontIMVI[] =
	"$Id: fontIMVI.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#include "FontMgr.h"

using namespace Executor;

static boolean_t outline_preferred_p = FALSE;

P1 (PUBLIC pascal trap, void, SetOutlinePreferred,
    Boolean, _outline_preferred_p)
{
  outline_preferred_p = _outline_preferred_p;
}

P0 (PUBLIC pascal trap, Boolean, GetOutlinePreferred)
{
  return outline_preferred_p;
}

P2 (PUBLIC pascal trap, Boolean, IsOutline,
    Point, numer, Point, denom)
{
  return FALSE;
}

P9 (PUBLIC pascal trap, OSErr, OutlineMetrics,
    int16, byte_count, Ptr, text,
    Point, numer, Point, denom,
    int16 *, y_max, int16 *, y_min,
    Fixed *, aw_array, Fixed *, lsb_array,
    Rect *, bounds_array)
{
  warning_unimplemented (NULL_STRING);
  /* ### paramErr */
  return -50;
}

static boolean_t preserve_glyph_p = FALSE;

P1 (PUBLIC pascal trap, void, SetPreserveGlyph,
    Boolean, _preserve_glyph_p)
{
  preserve_glyph_p = _preserve_glyph_p;
}

P0 (PUBLIC pascal trap, Boolean, GetPreserveGlyph)
{
  return preserve_glyph_p;
}

P0 (PUBLIC pascal trap, OSErr, FlushFonts)
{
  warning_unimplemented (NULL_STRING);
  /* ### paramErr */
  return -50;
}
