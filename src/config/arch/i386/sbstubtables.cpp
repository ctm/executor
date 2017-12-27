/* Copyright 1995 - 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/srcblt.h"
#include "rsys/vdriver.h"
#include "src-blitters.h"

#if !defined(USE_PORTABLE_SRCBLT)

/* NOTE:  xor is the only mode that cares about neither fg nor
 * bk colors, so there is no special fgbk version for it.
 */

#define NOSHIFT(OPNAME, SEGSUFF)                                 \
    static const void *OPNAME##_noshift_##SEGSUFF##_table[8] = { \
        &srcblt_nop,                                             \
        &srcblt_##OPNAME##_noshift_1_##SEGSUFF,                  \
        &srcblt_##OPNAME##_noshift_2_##SEGSUFF,                  \
        &srcblt_##OPNAME##_noshift_3_##SEGSUFF,                  \
        &srcblt_##OPNAME##_noshift_4_##SEGSUFF,                  \
        &srcblt_##OPNAME##_noshift_many_mod_0_##SEGSUFF,         \
        &srcblt_##OPNAME##_noshift_many_mod_1_##SEGSUFF,         \
        &srcblt_##OPNAME##_noshift_mask_##SEGSUFF                \
    }

NOSHIFT(copy, noseg);
NOSHIFT(and, noseg);
NOSHIFT(or, noseg);
NOSHIFT (xor, noseg);
NOSHIFT(notcopy, noseg);
NOSHIFT(notand, noseg);
NOSHIFT(notor, noseg);
NOSHIFT(notxor, noseg);

#undef NOSHIFT

#define NOSHIFT_FGBK(OPNAME, SEGSUFF)                                 \
    static const void *OPNAME##_noshift_fgbk_##SEGSUFF##_table[8] = { \
        &srcblt_nop,                                                  \
        &srcblt_##OPNAME##_noshift_many_fgbk_##SEGSUFF,               \
        &srcblt_##OPNAME##_noshift_many_fgbk_##SEGSUFF,               \
        &srcblt_##OPNAME##_noshift_many_fgbk_##SEGSUFF,               \
        &srcblt_##OPNAME##_noshift_many_fgbk_##SEGSUFF,               \
        &srcblt_##OPNAME##_noshift_many_fgbk_##SEGSUFF,               \
        &srcblt_##OPNAME##_noshift_many_fgbk_##SEGSUFF,               \
        &srcblt_##OPNAME##_noshift_mask_fgbk_##SEGSUFF                \
    }

NOSHIFT_FGBK(copy, noseg);
NOSHIFT_FGBK(and, noseg);
NOSHIFT_FGBK(or, noseg);
NOSHIFT_FGBK(notcopy, noseg);
NOSHIFT_FGBK(notand, noseg);
NOSHIFT_FGBK(notor, noseg);

#undef NOSHIFT_FGBK

#define SHIFT(OPNAME, CPU, SEGSUFF)                                    \
    static const void *OPNAME##_shift_##CPU##_##SEGSUFF##_table[8] = { \
        &srcblt_nop,                                                   \
        &srcblt_##OPNAME##_shift_1_##CPU##_##SEGSUFF,                  \
        &srcblt_##OPNAME##_shift_many_mod_0_##CPU##_##SEGSUFF,         \
        &srcblt_##OPNAME##_shift_many_mod_1_##CPU##_##SEGSUFF,         \
        &srcblt_##OPNAME##_shift_many_mod_0_##CPU##_##SEGSUFF,         \
        &srcblt_##OPNAME##_shift_many_mod_0_##CPU##_##SEGSUFF,         \
        &srcblt_##OPNAME##_shift_many_mod_1_##CPU##_##SEGSUFF,         \
        &srcblt_##OPNAME##_shift_mask_##CPU##_##SEGSUFF                \
    }

SHIFT(copy, i486, noseg);
SHIFT(and, i486, noseg);
SHIFT(or, i486, noseg);
SHIFT (xor, i486, noseg);
SHIFT(notcopy, i486, noseg);
SHIFT(notand, i486, noseg);
SHIFT(notor, i486, noseg);
SHIFT(notxor, i486, noseg);

SHIFT(copy, i386, noseg);
SHIFT(and, i386, noseg);
SHIFT(or, i386, noseg);
SHIFT (xor, i386, noseg);
SHIFT(notcopy, i386, noseg);
SHIFT(notand, i386, noseg);
SHIFT(notor, i386, noseg);
SHIFT(notxor, i386, noseg);

#undef SHIFT

#define SHIFT_FGBK(OPNAME, CPU, SEGSUFF)                                    \
    static const void *OPNAME##_shift_fgbk_##CPU##_##SEGSUFF##_table[8] = { \
        &srcblt_nop,                                                        \
        &srcblt_##OPNAME##_shift_many_fgbk_##CPU##_##SEGSUFF,               \
        &srcblt_##OPNAME##_shift_many_fgbk_##CPU##_##SEGSUFF,               \
        &srcblt_##OPNAME##_shift_many_fgbk_##CPU##_##SEGSUFF,               \
        &srcblt_##OPNAME##_shift_many_fgbk_##CPU##_##SEGSUFF,               \
        &srcblt_##OPNAME##_shift_many_fgbk_##CPU##_##SEGSUFF,               \
        &srcblt_##OPNAME##_shift_many_fgbk_##CPU##_##SEGSUFF,               \
        &srcblt_##OPNAME##_shift_mask_fgbk_##CPU##_##SEGSUFF                \
    }

SHIFT_FGBK(copy, i486, noseg);
SHIFT_FGBK(and, i486, noseg);
SHIFT_FGBK(or, i486, noseg);
SHIFT_FGBK(notcopy, i486, noseg);
SHIFT_FGBK(notand, i486, noseg);
SHIFT_FGBK(notor, i486, noseg);

SHIFT_FGBK(copy, i386, noseg);
SHIFT_FGBK(and, i386, noseg);
SHIFT_FGBK(or, i386, noseg);
SHIFT_FGBK(notcopy, i386, noseg);
SHIFT_FGBK(notand, i386, noseg);
SHIFT_FGBK(notor, i386, noseg);

#undef SHIFT_FGBK

#define ARRAY
#define CURLY
#

const void **srcblt_noshift_stubs ARRAY[8] = { CURLY
                                                   copy_noshift_noseg_table,
                                               or_noshift_noseg_table,
                                               xor_noshift_noseg_table, notand_noshift_noseg_table,
                                               notcopy_noshift_noseg_table, notor_noshift_noseg_table,
                                               notxor_noshift_noseg_table, and_noshift_noseg_table
}
;

const void **srcblt_noshift_fgbk_stubs ARRAY[8] = { CURLY
                                                        copy_noshift_fgbk_noseg_table,
                                                    or_noshift_fgbk_noseg_table,
                                                    xor_noshift_noseg_table, notand_noshift_fgbk_noseg_table,
                                                    notcopy_noshift_fgbk_noseg_table, notor_noshift_fgbk_noseg_table,
                                                    notxor_noshift_noseg_table, and_noshift_fgbk_noseg_table
}
;

const void **srcblt_shift_i486_stubs ARRAY[8] = { CURLY
                                                      copy_shift_i486_noseg_table,
                                                  or_shift_i486_noseg_table,
                                                  xor_shift_i486_noseg_table, notand_shift_i486_noseg_table,
                                                  notcopy_shift_i486_noseg_table, notor_shift_i486_noseg_table,
                                                  notxor_shift_i486_noseg_table, and_shift_i486_noseg_table
}
;

const void **srcblt_shift_i386_stubs ARRAY[8] = { CURLY
                                                      copy_shift_i386_noseg_table,
                                                  or_shift_i386_noseg_table,
                                                  xor_shift_i386_noseg_table, notand_shift_i386_noseg_table,
                                                  notcopy_shift_i386_noseg_table, notor_shift_i386_noseg_table,
                                                  notxor_shift_i386_noseg_table, and_shift_i386_noseg_table
}
;

const void **srcblt_shift_fgbk_i486_stubs ARRAY[8] = { CURLY
                                                           copy_shift_fgbk_i486_noseg_table,
                                                       or_shift_fgbk_i486_noseg_table,
                                                       xor_shift_i486_noseg_table, notand_shift_fgbk_i486_noseg_table,
                                                       notcopy_shift_fgbk_i486_noseg_table, notor_shift_fgbk_i486_noseg_table,
                                                       notxor_shift_i486_noseg_table, and_shift_fgbk_i486_noseg_table
}
;

const void **srcblt_shift_fgbk_i386_stubs ARRAY[8] = { CURLY
                                                           copy_shift_fgbk_i386_noseg_table,
                                                       or_shift_fgbk_i386_noseg_table,
                                                       xor_shift_i386_noseg_table, notand_shift_fgbk_i386_noseg_table,
                                                       notcopy_shift_fgbk_i386_noseg_table, notor_shift_fgbk_i386_noseg_table,
                                                       notxor_shift_i386_noseg_table, and_shift_fgbk_i386_noseg_table
}
;

#endif /* !USE_PORTABLE_SRCBLT */
