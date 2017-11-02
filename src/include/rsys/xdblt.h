#if !defined (_RSYS_XDBLT_H_)
#define _RSYS_XDBLT_H_

#include "QuickDraw.h"

#include "rsys/cquick.h"
#include "rsys/xdata.h"
#include "rsys/region.h"
#include "host_bltmacros.h"

#if !defined (ARCH_PROVIDES_RAW_PATBLT)
#define USE_PORTABLE_PATBLT
#endif

extern "C" {
extern void xdblt_canon_pattern (void)
#if !defined (USE_PORTABLE_PATBLT)
     asm ("_xdblt_canon_pattern")
#endif
     ;

extern char xdblt_nop asm ("_xdblt_nop");
extern char xdblt_short_init_ebp asm ("_xdblt_short_init_ebp");
extern char xdblt_tall_narrow_init_ebp asm ("_xdblt_tall_narrow_init_ebp");
extern char xdblt_tall_wide_init_ebp asm ("_xdblt_tall_wide_init_ebp");
}

extern const void *xdblt_nop_table[];

extern const void **xdblt_ones_stubs[5];
extern const void **xdblt_zeros_stubs[5];
extern const void **xdblt_short_narrow_stubs[5];
extern const void **xdblt_tall_narrow_stubs[5];
extern const void **xdblt_short_wide_stubs[5];
extern const void **xdblt_tall_wide_stubs[5];

namespace Executor {
extern bool xdblt_xdata_norgb_norotate (RgnHandle rh, int mode,
					     int pat_x_rotate_count,
					     int pat_y_rotate_count,
					     xdata_t *x,
					     PixMap *dst_bitmap);
extern bool xdblt_xdata_short_narrow (RgnHandle rh, int mode,
					   int pat_x_rotate_count,
					   int pat_y_rotate_count,
					   xdata_t *x,
					   PixMap *dst_bitmap);
extern bool xdblt_xdata_complex (RgnHandle rh, int mode,
				      int pat_x_rotate_count,
				      int pat_y_rotate_count,
				      xdata_t *x, PixMap *dst_bitmap);
extern bool xdblt_pattern (RgnHandle rh, int mode,
				int pat_x_rotate_count, int pat_y_rotate_count,
				const Pattern pattern, PixMap *dst,
				uint32 fg_color, uint32 bk_color);
}

extern "C" {
extern uint32 xdblt_pattern_value asm ("_xdblt_pattern_value");
extern uint32 xdblt_log2_pattern_row_bytes
	asm ("_xdblt_log2_pattern_row_bytes");
extern uint32 xdblt_pattern_height_minus_1
	asm ("_xdblt_pattern_height_minus_1");
extern uint32 *xdblt_pattern_baseaddr asm ("_xdblt_pattern_baseaddr");
extern uint32 *xdblt_pattern_end asm ("_xdblt_pattern_end");
extern uint32 xdblt_pattern_row_0 asm ("_xdblt_pattern_row_0");
extern const void **xdblt_stub_table asm ("_xdblt_stub_table");
extern uint32 xdblt_log2_bpp asm ("_xdblt_log2_bpp");
extern const Executor::INTEGER *xdblt_rgn_start asm ("_xdblt_rgn_start");
extern uint32 xdblt_x_offset asm ("_xdblt_x_offset");
extern uint32 *xdblt_dst_baseaddr asm ("_xdblt_dst_baseaddr");
extern uint32 xdblt_dst_row_bytes asm ("_xdblt_dst_row_bytes");
extern uint32 xdblt_insert_bits asm ("_xdblt_insert_bits");
extern const uint32 xdblt_mask_array[32] asm ("_xdblt_mask_array");
}

/* This must be a macro instead of a function because it sometimes
 * does an alloca.  This macro is used by both the pattern and source
 * blitters.
 */
#define SETUP_SPECIAL_RGN(rh, ptr_name)				\
do {								\
  RgnPtr r = STARH (rh);					\
  if (RGNP_SMALL_P (r))						\
    {								\
      phony_special_region[0] = r->rgnBBox.top.raw();		\
      phony_special_region[1] = CW (r->rgnBBox.left);		\
      phony_special_region[2] = CW (r->rgnBBox.right);		\
      phony_special_region[4] = r->rgnBBox.bottom.raw();	\
      ptr_name = phony_special_region;				\
    }								\
  else if (RGNP_SPECIAL_P (r))					\
    {								\
      /* Already a special region. */				\
      ptr_name = RGNP_DATA (r);					\
    }								\
  else								\
    {								\
      INTEGER *space;						\
      ptr_name = space = (INTEGER*)alloca (72 * 1024);			\
      nonspecial_rgn_to_special_rgn (RGNP_DATA (r), space);	\
    }								\
} while (0)

extern Executor::INTEGER phony_special_region[7];

typedef enum
{
  XDBLT_COPY,		/* == patCopy & 3 */
  XDBLT_OR,		/* == patOr & 3 */
  XDBLT_XOR,		/* == patXor & 3 */
  XDBLT_AND,		/* == notPatBic & 3 */
  XDBLT_INSERT		/* special internal mode */
} xdblt_transfer_mode_t;

#endif /* _RSYS_XDBLT_H_ */
