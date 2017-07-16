#if !defined (_RSYS_XDATA_H_)
#define _RSYS_XDATA_H_


#include "rsys/rgbutil.h"
#include "rsys/cquick.h"

namespace Executor {
typedef struct
{
  uint16 x_rot_count;	/* Number of pixels x-rotated.		     */
  uint16 y_rot_count;	/* Number of rows y-rotated.		     */
  uint8 src_flipped_p;	/* Are the mode_bits already flipped?	     */
} xdata_xfer_spec_t;

#define XDATA_INVALID_XFER_SPEC (-1)

#define XDATA_MAGIC_COOKIE (0xf9edbe3f)

typedef struct _xdata_t
{
  int log2_bpp;			/* Log base 2 of bits per pixel [0, 5].	     */
  int row_bytes;		/* Bytes per row, power of 2, >= 4.	     */
  int log2_row_bytes;		/* Log base 2 of row_bytes.		     */
  int row_bits_minus_1;		/* row_bytes * 8 - 1.			     */
  int height_minus_1;		/* Number of rows in this pattern, minus 1.  */
  int byte_size;		/* Total bytes taken by pattern.	     */
  LONGINT ctab_seed_x;		/* Big endian color table seed.		     */
  const rgb_spec_t *rgb_spec;	/* Only non-NULL for log2_bpp >= 4.	     */
  
  uint32 magic_cookie;
  
  /* This function will do the actual blit. */
  boolean_t (*blt_func) (RgnHandle rh, int mode,
			 int pat_x_rotate_count, int pat_y_rotate_count,
			 struct _xdata_t *x, PixMap *dst_bitmap);

  /* The following fields indicate the current transformation of the
   * pat_bits relative to their initial state.   These fields are only
   * used for tall or wide patterns.
   */
  int pat_x_rot;		/* X rotation in bits (from canonical).      */
  uint32 pat_flip_mask;		/* XOR mask applied to bits, from canonical. */

  /* Exactly one of these two holds the actual bits for the pattern. */
  uint32 pat_value;		/* For short, narrow patterns.		     */
  uint32 *pat_bits;		/* For tall and/or wide patterns (else NULL).*/

  Ptr raw_pat_bits_mem;		/* Might not == pat_bits; for DisposPtr.     */

  const void ***stub_table_for_mode;   /* Host-specific table. */
} xdata_t;


typedef xdata_t *xdata_ptr;
MAKE_HIDDEN(xdata_ptr);
typedef HIDDEN_xdata_ptr *xdata_handle_t;

extern boolean_t update_xdata_if_needed (xdata_handle_t x, PixPat *pixpat,
					 PixMapPtr dest);
extern xdata_handle_t xdata_for_pixpat_with_space (PixPat *pixpat,
						   PixMapPtr dest,
						   xdata_handle_t x);
#define xdata_for_pixpat(p, d) \
xdata_for_pixpat_with_space (p, d, \
			     (xdata_handle_t) NewHandle (sizeof (xdata_t)))
extern xdata_handle_t xdata_for_pattern (const Pattern pattern,
					 PixMapPtr target);
extern void xdata_free (xdata_handle_t x);
}
#endif /* !defined (_RSYS_XDATA_H_) */
