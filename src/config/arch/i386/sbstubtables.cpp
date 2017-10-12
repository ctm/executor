/* Copyright 1995 - 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/srcblt.h"
#include "rsys/vdriver.h"
#include "src-blitters.h"


#if !defined (USE_PORTABLE_SRCBLT)

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_sbstubtables[] = "$Id: sbstubtables.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* NOTE:  xor is the only mode that cares about neither fg nor
 * bk colors, so there is no special fgbk version for it.
 */


#define NOSHIFT(OPNAME, SEGSUFF)				\
static const void *OPNAME##_noshift_##SEGSUFF##_table[8] =	\
{								\
  &srcblt_nop,							\
  &srcblt_##OPNAME##_noshift_1_##SEGSUFF,			\
  &srcblt_##OPNAME##_noshift_2_##SEGSUFF,			\
  &srcblt_##OPNAME##_noshift_3_##SEGSUFF,			\
  &srcblt_##OPNAME##_noshift_4_##SEGSUFF,			\
  &srcblt_##OPNAME##_noshift_many_mod_0_##SEGSUFF,		\
  &srcblt_##OPNAME##_noshift_many_mod_1_##SEGSUFF,		\
  &srcblt_##OPNAME##_noshift_mask_##SEGSUFF			\
}

NOSHIFT (copy,    noseg);
NOSHIFT (and,     noseg);
NOSHIFT (or,      noseg);
NOSHIFT (xor,     noseg);
NOSHIFT (notcopy, noseg);
NOSHIFT (notand,  noseg);
NOSHIFT (notor,   noseg);
NOSHIFT (notxor,  noseg);

#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
NOSHIFT (copy,    seg);
NOSHIFT (and,     seg);
NOSHIFT (or,      seg);
NOSHIFT (xor,     seg);
NOSHIFT (notcopy, seg);
NOSHIFT (notand,  seg);
NOSHIFT (notor,   seg);
NOSHIFT (notxor,  seg);
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */

#undef NOSHIFT


#define NOSHIFT_FGBK(OPNAME, SEGSUFF)				\
static const void *OPNAME##_noshift_fgbk_##SEGSUFF##_table[8] =	\
{								\
  &srcblt_nop,							\
  &srcblt_##OPNAME##_noshift_many_fgbk_##SEGSUFF,		\
  &srcblt_##OPNAME##_noshift_many_fgbk_##SEGSUFF,		\
  &srcblt_##OPNAME##_noshift_many_fgbk_##SEGSUFF,		\
  &srcblt_##OPNAME##_noshift_many_fgbk_##SEGSUFF,		\
  &srcblt_##OPNAME##_noshift_many_fgbk_##SEGSUFF,		\
  &srcblt_##OPNAME##_noshift_many_fgbk_##SEGSUFF,		\
  &srcblt_##OPNAME##_noshift_mask_fgbk_##SEGSUFF		\
}

NOSHIFT_FGBK (copy,    noseg);
NOSHIFT_FGBK (and,     noseg);
NOSHIFT_FGBK (or,      noseg);
NOSHIFT_FGBK (notcopy, noseg);
NOSHIFT_FGBK (notand,  noseg);
NOSHIFT_FGBK (notor,   noseg);

#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
NOSHIFT_FGBK (copy,    seg);
NOSHIFT_FGBK (and,     seg);
NOSHIFT_FGBK (or,      seg);
NOSHIFT_FGBK (notcopy, seg);
NOSHIFT_FGBK (notand,  seg);
NOSHIFT_FGBK (notor,   seg);
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */

#undef NOSHIFT_FGBK


#define SHIFT(OPNAME, CPU, SEGSUFF)					\
static const void *OPNAME##_shift_##CPU##_##SEGSUFF##_table[8] =	\
{									\
  &srcblt_nop,								\
  &srcblt_##OPNAME##_shift_1_##CPU##_##SEGSUFF,				\
  &srcblt_##OPNAME##_shift_many_mod_0_##CPU##_##SEGSUFF,		\
  &srcblt_##OPNAME##_shift_many_mod_1_##CPU##_##SEGSUFF,		\
  &srcblt_##OPNAME##_shift_many_mod_0_##CPU##_##SEGSUFF,		\
  &srcblt_##OPNAME##_shift_many_mod_0_##CPU##_##SEGSUFF,		\
  &srcblt_##OPNAME##_shift_many_mod_1_##CPU##_##SEGSUFF,		\
  &srcblt_##OPNAME##_shift_mask_##CPU##_##SEGSUFF			\
}

SHIFT (copy,    i486, noseg);
SHIFT (and,     i486, noseg);
SHIFT (or,      i486, noseg);
SHIFT (xor,     i486, noseg);
SHIFT (notcopy, i486, noseg);
SHIFT (notand,  i486, noseg);
SHIFT (notor,   i486, noseg);
SHIFT (notxor,  i486, noseg);

SHIFT (copy,    i386, noseg);
SHIFT (and,     i386, noseg);
SHIFT (or,      i386, noseg);
SHIFT (xor,     i386, noseg);
SHIFT (notcopy, i386, noseg);
SHIFT (notand,  i386, noseg);
SHIFT (notor,   i386, noseg);
SHIFT (notxor,  i386, noseg);

#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
SHIFT (copy,    i486, seg);
SHIFT (and,     i486, seg);
SHIFT (or,      i486, seg);
SHIFT (xor,     i486, seg);
SHIFT (notcopy, i486, seg);
SHIFT (notand,  i486, seg);
SHIFT (notor,   i486, seg);
SHIFT (notxor,  i486, seg);

SHIFT (copy,    i386, seg);
SHIFT (and,     i386, seg);
SHIFT (or,      i386, seg);
SHIFT (xor,     i386, seg);
SHIFT (notcopy, i386, seg);
SHIFT (notand,  i386, seg);
SHIFT (notor,   i386, seg);
SHIFT (notxor,  i386, seg);
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */


#undef SHIFT


#define SHIFT_FGBK(OPNAME, CPU, SEGSUFF)				\
static const void *OPNAME##_shift_fgbk_##CPU##_##SEGSUFF##_table[8] =	\
{									\
  &srcblt_nop,								\
  &srcblt_##OPNAME##_shift_many_fgbk_##CPU##_##SEGSUFF,			\
  &srcblt_##OPNAME##_shift_many_fgbk_##CPU##_##SEGSUFF,			\
  &srcblt_##OPNAME##_shift_many_fgbk_##CPU##_##SEGSUFF,			\
  &srcblt_##OPNAME##_shift_many_fgbk_##CPU##_##SEGSUFF,			\
  &srcblt_##OPNAME##_shift_many_fgbk_##CPU##_##SEGSUFF,			\
  &srcblt_##OPNAME##_shift_many_fgbk_##CPU##_##SEGSUFF,			\
  &srcblt_##OPNAME##_shift_mask_fgbk_##CPU##_##SEGSUFF			\
}

SHIFT_FGBK (copy,    i486, noseg);
SHIFT_FGBK (and,     i486, noseg);
SHIFT_FGBK (or,      i486, noseg);
SHIFT_FGBK (notcopy, i486, noseg);
SHIFT_FGBK (notand,  i486, noseg);
SHIFT_FGBK (notor,   i486, noseg);

SHIFT_FGBK (copy,    i386, noseg);
SHIFT_FGBK (and,     i386, noseg);
SHIFT_FGBK (or,      i386, noseg);
SHIFT_FGBK (notcopy, i386, noseg);
SHIFT_FGBK (notand,  i386, noseg);
SHIFT_FGBK (notor,   i386, noseg);

#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
SHIFT_FGBK (copy,    i486, seg);
SHIFT_FGBK (and,     i486, seg);
SHIFT_FGBK (or,      i486, seg);
SHIFT_FGBK (notcopy, i486, seg);
SHIFT_FGBK (notand,  i486, seg);
SHIFT_FGBK (notor,   i486, seg);

SHIFT_FGBK (copy,    i386, seg);
SHIFT_FGBK (and,     i386, seg);
SHIFT_FGBK (or,      i386, seg);
SHIFT_FGBK (notcopy, i386, seg);
SHIFT_FGBK (notand,  i386, seg);
SHIFT_FGBK (notor,   i386, seg);
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */

#undef SHIFT_FGBK


#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
# define ARRAY [2]
# define CURLY {
#else
# define ARRAY
# define CURLY
#endif

const void **srcblt_noshift_stubs ARRAY[8] = { CURLY
  copy_noshift_noseg_table, or_noshift_noseg_table,
  xor_noshift_noseg_table, notand_noshift_noseg_table,
  notcopy_noshift_noseg_table, notor_noshift_noseg_table,
  notxor_noshift_noseg_table, and_noshift_noseg_table
#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
}, {
  copy_noshift_seg_table, or_noshift_seg_table,
  xor_noshift_seg_table, notand_noshift_seg_table,
  notcopy_noshift_seg_table, notor_noshift_seg_table,
  notxor_noshift_seg_table, and_noshift_seg_table
}
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */
};

const void **srcblt_noshift_fgbk_stubs ARRAY[8] = { CURLY
  copy_noshift_fgbk_noseg_table, or_noshift_fgbk_noseg_table,
  xor_noshift_noseg_table, notand_noshift_fgbk_noseg_table,
  notcopy_noshift_fgbk_noseg_table, notor_noshift_fgbk_noseg_table,
  notxor_noshift_noseg_table, and_noshift_fgbk_noseg_table
#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
}, {
  copy_noshift_fgbk_seg_table, or_noshift_fgbk_seg_table,
  xor_noshift_seg_table, notand_noshift_fgbk_seg_table,
  notcopy_noshift_fgbk_seg_table, notor_noshift_fgbk_seg_table,
  notxor_noshift_seg_table, and_noshift_fgbk_seg_table
}
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */
};

const void **srcblt_shift_i486_stubs ARRAY[8] = { CURLY
  copy_shift_i486_noseg_table, or_shift_i486_noseg_table,
  xor_shift_i486_noseg_table, notand_shift_i486_noseg_table,
  notcopy_shift_i486_noseg_table, notor_shift_i486_noseg_table,
  notxor_shift_i486_noseg_table, and_shift_i486_noseg_table
#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
}, {
  copy_shift_i486_seg_table, or_shift_i486_seg_table,
  xor_shift_i486_seg_table, notand_shift_i486_seg_table,
  notcopy_shift_i486_seg_table, notor_shift_i486_seg_table,
  notxor_shift_i486_seg_table, and_shift_i486_seg_table
}
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */
};

const void **srcblt_shift_i386_stubs ARRAY[8] = { CURLY
  copy_shift_i386_noseg_table, or_shift_i386_noseg_table,
  xor_shift_i386_noseg_table, notand_shift_i386_noseg_table,
  notcopy_shift_i386_noseg_table, notor_shift_i386_noseg_table,
  notxor_shift_i386_noseg_table, and_shift_i386_noseg_table
#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
}, {
  copy_shift_i386_seg_table, or_shift_i386_seg_table,
  xor_shift_i386_seg_table, notand_shift_i386_seg_table,
  notcopy_shift_i386_seg_table, notor_shift_i386_seg_table,
  notxor_shift_i386_seg_table, and_shift_i386_seg_table
}
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */
};

const void **srcblt_shift_fgbk_i486_stubs ARRAY[8] = { CURLY
  copy_shift_fgbk_i486_noseg_table, or_shift_fgbk_i486_noseg_table,
  xor_shift_i486_noseg_table, notand_shift_fgbk_i486_noseg_table,
  notcopy_shift_fgbk_i486_noseg_table, notor_shift_fgbk_i486_noseg_table,
  notxor_shift_i486_noseg_table, and_shift_fgbk_i486_noseg_table
#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
}, {
  copy_shift_fgbk_i486_seg_table, or_shift_fgbk_i486_seg_table,
  xor_shift_i486_seg_table, notand_shift_fgbk_i486_seg_table,
  notcopy_shift_fgbk_i486_seg_table, notor_shift_fgbk_i486_seg_table,
  notxor_shift_i486_seg_table, and_shift_fgbk_i486_seg_table
}
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */
};

const void **srcblt_shift_fgbk_i386_stubs ARRAY[8] = { CURLY
  copy_shift_fgbk_i386_noseg_table, or_shift_fgbk_i386_noseg_table,
  xor_shift_i386_noseg_table, notand_shift_fgbk_i386_noseg_table,
  notcopy_shift_fgbk_i386_noseg_table, notor_shift_fgbk_i386_noseg_table,
  notxor_shift_i386_noseg_table, and_shift_fgbk_i386_noseg_table
#if defined (VGA_SCREEN_NEEDS_FAR_PTR)
}, {
  copy_shift_fgbk_i386_seg_table, or_shift_fgbk_i386_seg_table,
  xor_shift_i386_seg_table, notand_shift_fgbk_i386_seg_table,
  notcopy_shift_fgbk_i386_seg_table, notor_shift_fgbk_i386_seg_table,
  notxor_shift_i386_seg_table, and_shift_fgbk_i386_seg_table
}
#endif /* VGA_SCREEN_NEEDS_FAR_PTR */
};

#endif /* !USE_PORTABLE_SRCBLT */
