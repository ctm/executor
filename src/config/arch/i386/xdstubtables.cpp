/* Copyright 1995 - 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/xdblt.h"
#include "pat-blitters.h"

#if !defined(USE_PORTABLE_PATBLT)

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_xdstubtables[] = "$Id: xdstubtables.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* For completely useless transfer modes (e.g. ORing with zero). */
const void *xdblt_nop_table[12] = {
    &xdblt_nop,
    &xdblt_nop,
    &xdblt_nop,
    &xdblt_nop,
    &xdblt_nop,
    &xdblt_nop,
    &xdblt_nop,
    &xdblt_nop,
    &xdblt_nop,
    &xdblt_nop,
    &xdblt_nop,
    &xdblt_nop,
};

#define SHORT_NARROW_COPY_TABLE(mask)      \
    {                                      \
        &xdblt_nop,                        \
            &xdblt_copy_short_narrow_1,    \
            &xdblt_copy_short_narrow_2,    \
            &xdblt_copy_short_narrow_3,    \
            &xdblt_copy_short_narrow_4,    \
            &xdblt_copy_short_narrow_many, \
            &xdblt_copy_short_narrow_many, \
            &xdblt_copy_short_narrow_many, \
            &xdblt_copy_short_narrow_many, \
            mask,                          \
            &xdblt_nop,                    \
            &xdblt_short_init_ebp,         \
    }

static const void *zeros_copy[12]
    = SHORT_NARROW_COPY_TABLE(&xdblt_copy_zeros_short_narrow_mask);
static const void *ones_copy[12]
    = SHORT_NARROW_COPY_TABLE(&xdblt_copy_ones_short_narrow_mask);
static const void *short_narrow_copy[12]
    = SHORT_NARROW_COPY_TABLE(&xdblt_copy_short_narrow_mask);

#undef SHORT_NARROW_COPY_TABLE

#define SHORT_NARROW_TABLE(op, mask)               \
    {                                              \
        &xdblt_nop,                                \
            &xdblt_##op##_short_narrow_1,          \
            &xdblt_##op##_short_narrow_2,          \
            &xdblt_##op##_short_narrow_3,          \
            &xdblt_##op##_short_narrow_4,          \
            &xdblt_##op##_short_narrow_many_mod_0, \
            &xdblt_##op##_short_narrow_many_mod_1, \
            &xdblt_##op##_short_narrow_many_mod_2, \
            &xdblt_##op##_short_narrow_many_mod_3, \
            mask,                                  \
            &xdblt_nop,                            \
            &xdblt_short_init_ebp,                 \
    }

static const void *short_narrow_or[12]
    = SHORT_NARROW_TABLE(or, &xdblt_or_short_narrow_mask);

static const void *ones_xor[12]
    = SHORT_NARROW_TABLE (xor, &xdblt_xor_ones_short_narrow_mask);
static const void *short_narrow_xor[12]
    = SHORT_NARROW_TABLE (xor, &xdblt_xor_short_narrow_mask);

static const void *short_narrow_and[12]
    = SHORT_NARROW_TABLE(and, &xdblt_and_short_narrow_mask);

#undef SHORT_NARROW_TABLE

static const void *short_narrow_insert[12] = {
    &xdblt_nop,
    &xdblt_insert_short_narrow_1,
    &xdblt_insert_short_narrow_many,
    &xdblt_insert_short_narrow_many,
    &xdblt_insert_short_narrow_many,
    &xdblt_insert_short_narrow_many,
    &xdblt_insert_short_narrow_many,
    &xdblt_insert_short_narrow_many,
    &xdblt_insert_short_narrow_many,
    &xdblt_insert_short_narrow_mask,
    &xdblt_nop,
    &xdblt_short_init_ebp,
};

/* Offsets into our function pointer table for this transfer mode. */
#define REPEAT_0_FUNC 0 /* Just a "ret" */
#define REPEAT_1_FUNC 4
#define REPEAT_2_FUNC 8
#define REPEAT_3_FUNC 12
#define REPEAT_4_FUNC 16
#define REPEAT_MANY_FUNC_MOD_0 20
#define REPEAT_MANY_FUNC_MOD_1 24
#define REPEAT_MANY_FUNC_MOD_2 28
#define REPEAT_MANY_FUNC_MOD_3 32
#define MASK_FUNC 36
#define SETUP_PAT_FUNC 40
#define INIT_EBP_FUNC 44

static const void *tall_narrow_copy[12] = {
    &xdblt_nop,
    &xdblt_copy_tall_narrow_1,
    &xdblt_copy_tall_narrow_2,
    &xdblt_copy_tall_narrow_3,
    &xdblt_copy_tall_narrow_4,
    &xdblt_copy_tall_narrow_many,
    &xdblt_copy_tall_narrow_many,
    &xdblt_copy_tall_narrow_many,
    &xdblt_copy_tall_narrow_many,
    &xdblt_copy_tall_narrow_mask,
    &xdblt_tall_narrow_setup_pat,
    &xdblt_tall_narrow_init_ebp,
};

#define TALL_NARROW(op)                         \
    static const void *tall_narrow_##op[12] = { \
        &xdblt_nop,                             \
        &xdblt_##op##_tall_narrow_1,            \
        &xdblt_##op##_tall_narrow_2,            \
        &xdblt_##op##_tall_narrow_3,            \
        &xdblt_##op##_tall_narrow_4,            \
        &xdblt_##op##_tall_narrow_many_mod_0,   \
        &xdblt_##op##_tall_narrow_many_mod_1,   \
        &xdblt_##op##_tall_narrow_many_mod_2,   \
        &xdblt_##op##_tall_narrow_many_mod_3,   \
        &xdblt_##op##_tall_narrow_mask,         \
        &xdblt_tall_narrow_setup_pat,           \
        &xdblt_tall_narrow_init_ebp,            \
    }

TALL_NARROW(or);
TALL_NARROW (xor);
TALL_NARROW(and);

#undef TALL_NARROW

static const void *tall_narrow_insert[12] = {
    &xdblt_nop,
    &xdblt_insert_tall_narrow_1,
    &xdblt_insert_tall_narrow_many,
    &xdblt_insert_tall_narrow_many,
    &xdblt_insert_tall_narrow_many,
    &xdblt_insert_tall_narrow_many,
    &xdblt_insert_tall_narrow_many,
    &xdblt_insert_tall_narrow_many,
    &xdblt_insert_tall_narrow_many,
    &xdblt_insert_tall_narrow_mask,
    &xdblt_tall_narrow_setup_pat,
    &xdblt_tall_narrow_init_ebp,
};

#define WIDE(name, op, setup, init)        \
    static const void *name##_##op[12] = { \
        &xdblt_nop,                        \
        &xdblt_##op##_wide_1,              \
        &xdblt_##op##_wide_2,              \
        &xdblt_##op##_wide_3,              \
        &xdblt_##op##_wide_4,              \
        &xdblt_##op##_wide_many,           \
        &xdblt_##op##_wide_many,           \
        &xdblt_##op##_wide_many,           \
        &xdblt_##op##_wide_many,           \
        &xdblt_##op##_wide_mask,           \
        setup,                             \
        init,                              \
    }

WIDE(short_wide, copy, &xdblt_nop, &xdblt_short_init_ebp);
WIDE(short_wide, or, &xdblt_nop, &xdblt_short_init_ebp);
WIDE(short_wide, xor, &xdblt_nop, &xdblt_short_init_ebp);
WIDE(short_wide, and, &xdblt_nop, &xdblt_short_init_ebp);

WIDE(tall_wide, copy, &xdblt_tall_wide_setup_pat, &xdblt_tall_wide_init_ebp);
WIDE(tall_wide, or, &xdblt_tall_wide_setup_pat, &xdblt_tall_wide_init_ebp);
WIDE(tall_wide, xor, &xdblt_tall_wide_setup_pat, &xdblt_tall_wide_init_ebp);
WIDE(tall_wide, and, &xdblt_tall_wide_setup_pat, &xdblt_tall_wide_init_ebp);

#undef WIDE

static const void *short_wide_insert[12] = {
    &xdblt_nop,
    &xdblt_insert_wide_1,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_mask,
    &xdblt_nop,
    &xdblt_short_init_ebp,
};

static const void *tall_wide_insert[12] = {
    &xdblt_nop,
    &xdblt_insert_wide_1,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_many,
    &xdblt_insert_wide_mask,
    &xdblt_tall_wide_setup_pat,
    &xdblt_tall_wide_init_ebp,
};

const void **xdblt_ones_stubs[5] = {
    ones_copy, ones_copy, ones_xor, xdblt_nop_table, short_narrow_insert
};

const void **xdblt_zeros_stubs[5] = {
    zeros_copy, xdblt_nop_table, xdblt_nop_table, zeros_copy,
    short_narrow_insert
};

const void **xdblt_short_narrow_stubs[5] = {
    short_narrow_copy, short_narrow_or, short_narrow_xor, short_narrow_and,
    short_narrow_insert
};

const void **xdblt_tall_narrow_stubs[5] = {
    tall_narrow_copy, tall_narrow_or, tall_narrow_xor, tall_narrow_and,
    tall_narrow_insert
};

const void **xdblt_short_wide_stubs[5] = {
    short_wide_copy, short_wide_or, short_wide_xor, short_wide_and,
    short_wide_insert
};

const void **xdblt_tall_wide_stubs[5] = {
    tall_wide_copy, tall_wide_or, tall_wide_xor, tall_wide_and, tall_wide_insert
};

#endif /* !USE_PORTABLE_PATBLT */
