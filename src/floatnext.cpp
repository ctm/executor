/* Copyright 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "SANE.h"
#include "rsys/float.h"
#include "rsys/floatconv.h"

using namespace Executor;

/* Subtracts one from the given multi-byte big endian unsigned number. */
static void
mp_decrement_big_endian(uint8 *bytes, int num_bytes)
{
    bool borrow_p;
    uint8 *b;

    borrow_p = true;
    for(b = bytes + num_bytes; borrow_p && b > bytes;)
    {
        const uint8 v = *--b;
        borrow_p = (v == 0);
        *b = v - 1;
    }
}

/* Adds one to the given multi-byte big endian unsigned number. */
static void
mp_increment_big_endian(uint8 *bytes, int num_bytes)
{
    bool carry_p;
    uint8 *b;

    carry_p = true;
    for(b = bytes + num_bytes; carry_p && b > bytes;)
    {
        /* Equivalent to "carry_p = !++*--b;"  Heh. */
        const uint8 v = *--b + 1;
        carry_p = (v == 0);
        *b = v;
    }
}

/* Replaces x with the nearest number to x in the direction of y.
 *
 * This implementation is not exactly right, but it should be good
 * enough for Executor 2.  In particular, it doesn't handle
 * denormalized numbers well and it doesn't signal exceptions.
 *
 * Note that most SANE traps replace the dst operand with the
 * result.  This one replaces the src operand (really!)
 */
void Executor::C_ROMlib_FnextX(uint8 *x, uint8 *y, unsigned short sel)
{
    CCRElement saved_ccc, saved_ccn, saved_ccv, saved_ccnz, saved_ccx;
    INTEGER x_class, y_class, x_class_swapped, y_class_swapped;
    x80_t x80_x;
    int byte_size;
    int x_sign, y_sign;
    bool normalize_x80_p;
    ieee_t xv, yv;

    saved_ccnz = cpu_state.ccnz;
    saved_ccn = cpu_state.ccn;
    saved_ccc = cpu_state.ccc;
    saved_ccv = cpu_state.ccv;
    saved_ccx = cpu_state.ccx;

    /* Determine the classes of both X and Y. */
    ROMlib_Fclassx(x, &x_class_swapped, sel);
    x_class = CW_RAW(x_class_swapped);
    ROMlib_Fclassx(y, &y_class_swapped, sel);
    y_class = CW_RAW(y_class_swapped);

    normalize_x80_p = false; /* default, avoid gcc warnings. */

    switch(sel & OPCODE_MASK)
    {
        case FX_OPERAND:
            xv = x80_to_ieee((const x80_t *)x);
            yv = x80_to_ieee((const x80_t *)y);
            normalize_x80_p = ((x[2] & 0x80) != 0);
            byte_size = 10;
            break;
        case FD_OPERAND:
            xv = f64_to_ieee((const f64_t *)x);
            yv = f64_to_ieee((const f64_t *)y);
            byte_size = 8;
            break;
        case FS_OPERAND:
            xv = f32_to_ieee((const f32_t *)x);
            yv = f32_to_ieee((const f32_t *)y);
            byte_size = 4;
            break;
        case FI_OPERAND:
            xv = CW_RAW(*(short *)x);
            yv = CW_RAW(*(short *)y);
            byte_size = 2;
            break;
        case FL_OPERAND:
            xv = CL_RAW(*(long *)x);
            yv = CL_RAW(*(long *)y);
            byte_size = 4;
            break;
        case FC_OPERAND:
            xv = comp_to_ieee((const comp_t *)x);
            yv = comp_to_ieee((const comp_t *)y);
            byte_size = 8;
            break;
        default:
            gui_abort();
            xv = yv = 0; /* avoid gcc warnings */
            goto done;
    }

    /* If either number is a NaN, the result is a NaN.  Technically it
   * should be a quiet NaN and we should signal if a signaling
   * NaN made it here, but we don't do that yet.
   */
    if(y_class == SNaN || y_class == QNaN)
    {
        memcpy(x, y, byte_size);
        goto done;
    }
    if(x_class == SNaN || x_class == QNaN)
        goto done;

    /* Compare x and y. */
    ROMlib_Fx2X(x, &x80_x, sel & OPCODE_MASK);
    ROMlib_Fcmpx(y, &x80_x, sel & OPCODE_MASK);

    /* If x == y, just return x unchanged. */
    if(!cpu_state.ccnz)
        goto done;

    /* Fetch the sign of x and y. */
    x_sign = x[0] & 0x80;
    y_sign = y[0] & 0x80;

    if(x_class == ZeroNum)
    {
        memset(x, 0, byte_size);
        x[0] = y_sign;
        x[byte_size - 1] |= 1; /* smallest normalized number. */
    }
    else /* x != 0 */
    {
        if(!cpu_state.ccn ^ !x_sign)
            mp_increment_big_endian(x, byte_size);
        else
        {
            if(normalize_x80_p)
                x[2] &= 0x7F; /* make sure borrow goes through normalize bit */
            mp_decrement_big_endian(x, byte_size);
        }

        /* Restore the sign in case of carry. */
        x[0] = (x[0] & 0x7F) | x_sign;
    }

    /* Restore normalize bit, which may have gotten carried through or
   * borrowed through, etc.
   */
    if(normalize_x80_p)
        x[2] |= 0x80;

done:

#if ERROR_SUPPORTED_P(ERROR_FLOATING_POINT)
{
    ieee_t result;

    switch(sel & OPCODE_MASK)
    {
        case FX_OPERAND:
            result = x80_to_ieee((const x80_t *)x);
            break;
        case FD_OPERAND:
            result = f64_to_ieee((const f64_t *)x);
            break;
        case FS_OPERAND:
            result = f32_to_ieee((const f32_t *)x);
            break;
        case FI_OPERAND:
            result = CW_RAW(*(short *)x);
            break;
        case FL_OPERAND:
            result = CL_RAW(*(long *)x);
            break;
        case FC_OPERAND:
            result = comp_to_ieee((const comp_t *)x);
            break;
        default:
            gui_abort();
            result = 0;
    }

    warning_floating_point("nextafter(%.30f, %.30f) == %.30f",
                           (double)xv, (double)yv, (double)result);
}
#endif /* ERROR_SUPPORTED_P (ERROR_FLOATING_POINT) */

    cpu_state.ccnz = saved_ccnz;
    cpu_state.ccn = saved_ccn;
    cpu_state.ccc = saved_ccc;
    cpu_state.ccv = saved_ccv;
    cpu_state.ccx = saved_ccx;
}
