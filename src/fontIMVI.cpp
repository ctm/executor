/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "FontMgr.h"

using namespace Executor;

static bool outline_preferred_p = false;

PUBLIC pascal trap void Executor::C_SetOutlinePreferred(Boolean _outline_preferred_p)
{
    outline_preferred_p = _outline_preferred_p;
}

PUBLIC pascal trap Boolean Executor::C_GetOutlinePreferred()
{
    return outline_preferred_p;
}

PUBLIC pascal trap Boolean Executor::C_IsOutline(Point numer, Point denom)
{
    return false;
}

PUBLIC pascal trap OSErr Executor::C_OutlineMetrics(int16_t byte_count, Ptr text, Point numer, Point denom, int16_t * y_max, int16_t * y_min, Fixed * aw_array, Fixed * lsb_array, Rect * bounds_array)
{
    warning_unimplemented(NULL_STRING);
    /* ### paramErr */
    return -50;
}

static bool preserve_glyph_p = false;

PUBLIC pascal trap void Executor::C_SetPreserveGlyph(Boolean _preserve_glyph_p)
{
    preserve_glyph_p = _preserve_glyph_p;
}

PUBLIC pascal trap Boolean Executor::C_GetPreserveGlyph()
{
    return preserve_glyph_p;
}

PUBLIC pascal trap OSErr Executor::C_FlushFonts()
{
    warning_unimplemented(NULL_STRING);
    /* ### paramErr */
    return -50;
}
