#if !defined(_DEPTHCONV_H_)
#define _DEPTHCONV_H_

#include "rsys/rgbutil.h"
#include "CQuickDraw.h"

namespace Executor
{
/* Public API */
typedef void (*depthconv_func_t)(const void *table,
                                 const uint8 *src_base, int src_row_bytes,
                                 uint8 *dst_base, int dst_row_bytes,
                                 int top, int left, int bottom, int right);

/* Each of these functions fills in TABLE_SPACE with some private tables,
 * returns the size of those tables in *TABLE_SIZE, and returns a pointer
 * to a function to perform the desired translation.  If TABLE_SPACE is
 * NULL, it is ignored.  You can make one call to the function with a NULL
 * TABLE_SPACE, note the table size, allocate space for it, and then
 * call it again with the real table space.  If TABLE_SIZE is NULL,
 * it is also ignored.
 */
extern depthconv_func_t
depthconv_make_raw_table(void *table_space, unsigned in_bpp,
                         unsigned out_bpp, uint32_t *table_size,
                         const uint32_t *mapping);
extern depthconv_func_t
depthconv_make_ind_to_ind_table(void *table_space, unsigned in_bpp,
                                unsigned out_bpp, uint32_t *table_size,
                                const ColorSpec *mapping);
extern depthconv_func_t
depthconv_make_ind_to_rgb_table(void *table_space, unsigned in_bpp,
                                uint32_t *table_size, const ColorSpec *mapping,
                                const rgb_spec_t *dst_rgb_spec);

extern depthconv_func_t
depthconv_make_rgb_to_ind_table(void *table_space, unsigned out_bpp,
                                uint32_t *table_size, CTabHandle mapping,
                                ITabHandle itab,
                                const rgb_spec_t *src_rgb_spec);

extern depthconv_func_t
depthconv_make_rgb_to_rgb_table(void *table_space, uint32_t *table_size,
                                const rgb_spec_t *src_rgb_spec,
                                const rgb_spec_t *dst_rgb_spec);

/* This is the largest possible table size.
 * The largest table is 1bpp -> 32bpp, which requires about 8K of space.
 * We add a little extra cruft to handle the stored log2_in_bpp as
 * well as excess to assure that we can align the array just the way
 * we want it.
 */
#define DEPTHCONV_MAX_TABLE_SIZE (sizeof(uint32_t) + 32 * 256 + 31)
#define DEPTHCONV_MAX_UINT32_TABLE_SIZE \
    ((DEPTHCONV_MAX_TABLE_SIZE + sizeof(uint32_t) - 1) / sizeof(uint32_t))

/* ------- END public API ------- */

/* Private data, shared between dcmaketables.c and dcconvert.c. */

/* These are INTERNAL types used to store lookup tables used for conversion. */
typedef uint8 depthconv_8_1_data_t[8][256];
typedef uint8 depthconv_4_1_data_t[4][256];
typedef uint8 depthconv_2_1_data_t[2][256];
typedef uint8 depthconv_1_1_data_t[256];
typedef uint16_t depthconv_1_2_data_t[256][4];
typedef uint32_t depthconv_1_4_data_t[256];
typedef uint32_t depthconv_1_8_data_t[256][2];
typedef uint32_t depthconv_1_16_data_t[256][4];
typedef uint32_t depthconv_1_32_data_t[256][8];

typedef struct
{
    uint32_t log2_in_bpp;
    const rgb_spec_t *src_rgb_spec;
    GUEST<CTabHandle> swapped_ctab;
    GUEST<ITabHandle> swapped_itab;
} depthconv_rgb_to_ind_data_t;

typedef struct
{
    uint32_t log2_in_bpp;
    const rgb_spec_t *src_rgb_spec;
    const rgb_spec_t *dst_rgb_spec;
} depthconv_rgb_to_rgb_data_t;

extern const int depthconv_ind_src_table_alignment[];
#define DEPTHCONV_TABLE_ALIGNMENT(log2_in_bpp, log2_out_bpp) \
    depthconv_ind_src_table_alignment[(log2_in_bpp) - (log2_out_bpp) + 5]
#define DEPTHCONV_ALIGN_TABLE(t, log2_in_bpp, log2_out_bpp)                  \
    ((void *)((((uintptr_t)(t))                                          \
               + sizeof(uint32_t)                                              \
               + (DEPTHCONV_TABLE_ALIGNMENT(log2_in_bpp, log2_out_bpp)) - 1) \
              & ~((DEPTHCONV_TABLE_ALIGNMENT(log2_in_bpp, log2_out_bpp)) - 1)))

#define DEPTHCONV_DECL(func_name)                                   \
    extern void func_name(const void *raw_table,                    \
                          const uint8 *src_base, int src_row_bytes, \
                          uint8 *dst_base, int dst_row_bytes,       \
                          int top, int left, int bottom, int right)

DEPTHCONV_DECL(depthconv_copy);
DEPTHCONV_DECL(depthconv_1_1);
DEPTHCONV_DECL(depthconv_1_2);
DEPTHCONV_DECL(depthconv_1_4);
DEPTHCONV_DECL(depthconv_1_8);
DEPTHCONV_DECL(depthconv_1_16);
DEPTHCONV_DECL(depthconv_1_32);
DEPTHCONV_DECL(depthconv_2_1);
DEPTHCONV_DECL(depthconv_4_1);
DEPTHCONV_DECL(depthconv_8_1);
DEPTHCONV_DECL(depthconv_16_1);
DEPTHCONV_DECL(depthconv_16_2);
DEPTHCONV_DECL(depthconv_16_4);
DEPTHCONV_DECL(depthconv_16_8);
DEPTHCONV_DECL(depthconv_16_16);
DEPTHCONV_DECL(depthconv_16_32);
DEPTHCONV_DECL(depthconv_32_1);
DEPTHCONV_DECL(depthconv_32_2);
DEPTHCONV_DECL(depthconv_32_4);
DEPTHCONV_DECL(depthconv_32_8);
DEPTHCONV_DECL(depthconv_32_16);
DEPTHCONV_DECL(depthconv_32_32);

#undef DEPTHCONV_DECL
}

#endif /* !_DEPTHCONV_H_ */
