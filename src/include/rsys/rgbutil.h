#if !defined (_RGBUTIL_H_)
#define _RGBUTIL_H_


#include "QuickDraw.h"

namespace Executor {
/* forward decl */
struct rgb_spec;

/* This struct is used to perform efficient mapping from an RGB pixel
 * to an RGBColor.
 */
typedef struct
{
  int low_red_bit;
  int low_green_bit;
  int low_blue_bit;
  uint16 map[3][256]; /* Maps r,g,b to big endian INTEGER's. */
} rgb_map_t;

typedef void (*rgb_extract_func_t) (const struct rgb_spec *rgb_spec,
				    uint32 in,
				    RGBColor *out);

/* This struct describes the format of any RGB pixel. */
typedef struct rgb_spec
{
  int bpp;				/* Bits per pixel, either 16 or 32. */
  bool big_endian_p;		/* TRUE iff a Mac-format pixel.     */
  uint32 xor_mask;			/* Bits XOR'd after RGB assembled.  */
  int num_red_bits,   low_red_bit;	/* Size and position of red bits.   */
  int num_green_bits, low_green_bit;	/* Size and position of green bits. */
  int num_blue_bits,  low_blue_bit;	/* Size and position of blue bits.  */

  /* The following pixel values are in the same byte order specified
   * by the big_endian_p field.  If you write them out unchanged to
   * the appropriate PixMap, the Right Thing will happen.
   */
  uint32 black_pixel;			/* Tiled to 32 bits. 		    */
  uint32 white_pixel;			/* Tiled to 32 bits. 		    */
  uint32 pixel_bits_mask;		/* Tiled to 32 bits. 		    */
  
  uint32 seed_x;
  
  /* pixel to big endian rgbcolor converion tables/function */
  rgb_map_t map;
  rgb_extract_func_t pixel_to_rgbcolor;
  
  uint32 (*rgbcolor_to_pixel) (const struct rgb_spec *rgb_spec,
			       const RGBColor *color,
			       bool big_endian_rgbcolor_p);
} rgb_spec_t;

extern void rgbutil_init (void);

extern void make_rgb_spec (rgb_spec_t *rgb_spec,
			   int bpp, bool big_endian_p,
			   uint32 xor_mask,
			   int num_red_bits, int low_red_bit,
			   int num_green_bits, int low_green_bit,
			   int num_blue_bits, int low_blue_bit,
			   uint32 seed_x);
}
#endif /* !_RGBUTIL_H_ */
