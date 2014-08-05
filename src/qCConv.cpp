/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qCConv[] =
		"$Id: qCConv.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"

using namespace Executor;
using namespace ByteSwap;

/* cmy and rgb color spaces are simply complements */

P2 (PUBLIC pascal trap, void, CMY2RGB,
    CMYColor *, cmy_color,
    RGBColor *, rgb_color)
{
  rgb_color->red   = ~cmy_color->cyan;
  rgb_color->green = ~cmy_color->magenta;
  rgb_color->blue  = ~cmy_color->yellow;
}
   
P2 (PUBLIC pascal trap, void, RGB2CMY,
    RGBColor *, rgb_color,
    CMYColor *, cmy_color)
{
  /* use `bar = ~foo' instead of `bar = BigEndianValue (MaxSmallFract - BigEndianValue (foo))'
     to compute the complement value */
  
  cmy_color->cyan    = ~rgb_color->red;
  cmy_color->magenta = ~rgb_color->green;
  cmy_color->yellow  = ~rgb_color->blue;
}

#define SF_MULT(x,y) (((x) * (y)) / 65535)
#define SF_DIV(x,y) (((x) * 65535) / (y))

#define C_TO_SF(c) ((c) * 65535)
#define SF_TO_C(sf) ((sf) / 65535)

#define ANGLE_TO_SF(angle) (C_TO_SF (angle) / 360)


static inline unsigned short
value (unsigned long n1, unsigned long n2, unsigned long hue)
{
  if (hue < ANGLE_TO_SF (60))
    return BigEndianValue (n1 + SF_MULT (n2 - n1,
			     SF_MULT (hue, C_TO_SF (6))));
  else if (hue < ANGLE_TO_SF (180))
    return BigEndianValue (n2);
  else if (hue < ANGLE_TO_SF (240))
    return BigEndianValue (n1 + SF_MULT (n2 - n1,
			     SF_MULT (ANGLE_TO_SF (240) - hue,
				      C_TO_SF (6))));
  else
    return BigEndianValue (n1);
}


P2 (PUBLIC pascal trap, void, HSL2RGB,
    HSLColor *, hsl_color,
    RGBColor *, rgb_color)
{
  if (hsl_color->saturation == CWC (0))
    {
      rgb_color->red   = hsl_color->lightness;
      rgb_color->green = hsl_color->lightness;
      rgb_color->blue  = hsl_color->lightness;
    }
  else
    {
      unsigned long m1, m2;

      /* the hue represents a angle in the range [0, 360) */
      unsigned long h = BigEndianValue (hsl_color->hue);
      unsigned long s = BigEndianValue (hsl_color->saturation);
      unsigned long l = BigEndianValue (hsl_color->lightness);
      
      if (l <= (MaxSmallFract / 2))
	m2 = SF_MULT (l, (C_TO_SF (1) + s));
      else
	m2 = l + s - SF_MULT (l, s);
      m1 = SF_MULT (l, C_TO_SF (2)) - m2;

      rgb_color->red   = value (m1, m2,
				(h >= ANGLE_TO_SF (240)) ? (h - ANGLE_TO_SF (240))
			                                 : (h + ANGLE_TO_SF (120)));
      rgb_color->green = value (m1, m2, h);
      rgb_color->blue  = value (m1, m2,
			        (h < ANGLE_TO_SF (120)) ? (h + ANGLE_TO_SF (240))
		                                        : (h - ANGLE_TO_SF (120)));
    }
}

P2 (PUBLIC pascal trap, void, RGB2HSL,
    RGBColor *, rgb_color,
    HSLColor *, hsl_color)
{
  unsigned long r = BigEndianValue (rgb_color->red);
  unsigned long g = BigEndianValue (rgb_color->green);
  unsigned long b = BigEndianValue (rgb_color->blue);
  
  unsigned long max = MAX (r, MAX (g, b));
  unsigned long min = MIN (r, MIN (g, b));

  unsigned long h;
  unsigned long s;
  unsigned long l = (min + max) / 2;

  if (min == max)
    {
      /* achromatic case, because all rgb color components
	 are equal */
      s = 0;
      /* the value of h in this case is meaningless, should
	 investigate what the Mac color conversion routines do */
      h = 0;
    }
  else
    {
      unsigned long delta = max - min;
      
      if (l <= (MaxSmallFract / 2))
	{
	  s = SF_DIV (delta, max + min);
	  gui_assert (s <= MaxSmallFract);
	}
      else
	{
	  s = SF_DIV (delta, C_TO_SF (2) - max - min);
	  gui_assert (s <= MaxSmallFract);
	}
      
      if (r == max)
	{
	  if (g >= b)
	    h = SF_MULT (SF_DIV (g - b, delta), ANGLE_TO_SF (60));
	  else
	    h = ANGLE_TO_SF (360) - SF_MULT (SF_DIV (b - g, delta), ANGLE_TO_SF (60));

	}
      else if (g == max)
	{
	  if (b >= r)
	    h = ANGLE_TO_SF (120) + SF_MULT (SF_DIV (b - r, delta), ANGLE_TO_SF (60));
	  else
	    h = ANGLE_TO_SF (120) - SF_MULT (SF_DIV (r - b, delta), ANGLE_TO_SF (60));
	}
      else if (b == max)
	{
	  if (r >= g)
	    h = ANGLE_TO_SF (240) + SF_MULT (SF_DIV (r - g, delta), ANGLE_TO_SF (60));
	  else
	    h = ANGLE_TO_SF (240) - SF_MULT (SF_DIV (g - r, delta), ANGLE_TO_SF (60));
	}
      else
	gui_fatal("r = 0x%lx, g = 0x%lx, b = 0x%lx", r, g, b);
    }

  hsl_color->hue        = BigEndianValue (h);
  hsl_color->saturation = BigEndianValue (s);
  hsl_color->lightness  = BigEndianValue (l);
}

P2 (PUBLIC pascal trap, void, HSV2RGB,
    HSVColor *, hsv_color,
    RGBColor *, rgb_color)
{
  if (hsv_color->saturation == CWC (0))
    {
      rgb_color->red   = hsv_color->value;
      rgb_color->green = hsv_color->value;
      rgb_color->blue  = hsv_color->value;
    }
  else
    {
      /* the hue represents a angle in the range [0, 360) */
      unsigned long h = BigEndianValue (hsv_color->hue);
      unsigned long s = BigEndianValue (hsv_color->saturation);
      unsigned long v = BigEndianValue (hsv_color->value);

      /* one of the six color verticies of the hex cone, [0, 6) */
      unsigned sextant = SF_TO_C (h * 6);
      /* `fractional' portion of h, in the range [0,1] */
      /* unsigned long f = 6 * (h % ANGLE_TO_SF (60)); */
      unsigned long f = 6 * (h - ANGLE_TO_SF (sextant * 60));

      unsigned long p = SF_MULT (v, C_TO_SF (1) - s);
      unsigned long q = SF_MULT (v, C_TO_SF (1) - SF_MULT (s, f));
      unsigned long t = SF_MULT (v, (C_TO_SF (1)
				     - SF_MULT (s, C_TO_SF (1) - f)));

      switch (sextant)
	{
	case 0:
	  rgb_color->red   = BigEndianValue (v);
	  rgb_color->green = BigEndianValue (t);
	  rgb_color->blue  = BigEndianValue (p);
	  break;
	case 1:
	  rgb_color->red   = BigEndianValue (q);
	  rgb_color->green = BigEndianValue (v);
	  rgb_color->blue  = BigEndianValue (p);
	  break;
	case 2:
	  rgb_color->red   = BigEndianValue (p);
	  rgb_color->green = BigEndianValue (v);
	  rgb_color->blue  = BigEndianValue (t);
	  break;
	case 3:
	  rgb_color->red   = BigEndianValue (p);
	  rgb_color->green = BigEndianValue (q);
	  rgb_color->blue  = BigEndianValue (v);
	  break;
	case 4:
	  rgb_color->red   = BigEndianValue (t);
	  rgb_color->green = BigEndianValue (p);
	  rgb_color->blue  = BigEndianValue (v);
	  break;
	case 5:
	case 6:
	  rgb_color->red   = BigEndianValue (v);
	  rgb_color->green = BigEndianValue (p);
	  rgb_color->blue  = BigEndianValue (q);
	  break;
	default:
	  gui_fatal ("sextant = %d", sextant);
	}
    }
}

P2 (PUBLIC pascal trap, void, RGB2HSV,
    RGBColor *, rgb_color,
    HSVColor *, hsv_color)
{
  unsigned long r = BigEndianValue (rgb_color->red);
  unsigned long g = BigEndianValue (rgb_color->green);
  unsigned long b = BigEndianValue (rgb_color->blue);
  
  unsigned long max = MAX (r, MAX (g, b));
  unsigned long min = MIN (r, MIN (g, b));

  unsigned long h;
  unsigned long s;
  unsigned long v = max;

  if (max != 0)
    {
      s = SF_DIV (max - min, max);
      gui_assert (s <= MaxSmallFract);
    }
  else
    s = 0;
  
  if (s == 0)
    {
      /* the value of h in this case is meaningless, should
	 investigate what the Mac color conversion routines do */
      h = 0;
    }
  else
    {
      unsigned long delta = max - min;
      
      if (r == max)
	{
	  if (g >= b)
	    h = SF_MULT (SF_DIV (g - b, delta), ANGLE_TO_SF (60));
	  else
	    h = ANGLE_TO_SF (360) - SF_MULT (SF_DIV (b - g, delta), ANGLE_TO_SF (60));

	}
      else if (g == max)
	{
	  if (b >= r)
	    h = ANGLE_TO_SF (120) + SF_MULT (SF_DIV (b - r, delta), ANGLE_TO_SF (60));
	  else
	    h = ANGLE_TO_SF (120) - SF_MULT (SF_DIV (r - b, delta), ANGLE_TO_SF (60));
	}
      else if (b == max)
	{
	  if (r >= g)
	    h = ANGLE_TO_SF (240) + SF_MULT (SF_DIV (r - g, delta), ANGLE_TO_SF (60));
	  else
	    h = ANGLE_TO_SF (240) - SF_MULT (SF_DIV (g - r, delta), ANGLE_TO_SF (60));
	}
      else
	gui_fatal ("r = 0x%lx, g = 0x%lx, b = 0x%lx", r, g, b);
    }
  
  hsv_color->hue        = BigEndianValue (h);
  hsv_color->saturation = BigEndianValue (s);
  hsv_color->value      = BigEndianValue (v);
}

P1 (PUBLIC pascal trap, SmallFract, Fix2SmallFract,
    Fixed, f)
{
  return f & 0xFFFF;
}

P1 (PUBLIC pascal trap, Fixed, SmallFract2Fix,
    SmallFract, sf)
{
  return sf;
}
