
#include "QuickDraw.h"

namespace Executor {

typedef struct pixel_image
{
  Rect bounds;
  
  PixMapHandle bits[2];
  PixMapHandle x_bits[2];
  
  /* -1 indicates `x_bits' is not valid, otherwise this field contains
     the color table of the `bits' from which is was made */
  int x_bits_valid[2];
} pixel_image_t;

#define IMAGE_BITS(image, color_p)		((image)->bits[(color_p)])
#define IMAGE_X_BITS(image, color_p)		((image)->x_bits[(color_p)])
#define IMAGE_X_BITS_VALID(image, color_p) \
  ((image)->x_bits_valid[(color_p)])

typedef struct image_bits_desc
{
  unsigned char *raw_bits;
  int row_bytes;
  int bpp;
} image_bits_desc_t;

typedef struct pixel_image_desc
{
  Rect bounds;
  image_bits_desc_t bits[5];
} pixel_image_desc_t;

extern pixel_image_t *image_init (pixel_image_desc_t *image_desc);
extern void image_copy (pixel_image_t *image, int color_p /* visual */,
			Rect *dst_rect, int mode);
extern void image_validate_x_bits (pixel_image_t *image,
				   int color_p /* visual */);
extern void image_update_ctab (pixel_image_t *image,
			       const RGBColor *new_colors, int max_color);
extern void image_inits (void);
}
