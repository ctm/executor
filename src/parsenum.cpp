/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_parsenum[] = "$Id: parsenum.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/parsenum.h"
#include <ctype.h>

using namespace Executor;
using namespace std;

/* If the given string has a 'k', 'K', 'm', or 'M' suffix, returns the
 * log base 2 corresponding to that size (either 10 or 20) and removes
 * the suffix from the string.  If no suffix is found, returns 0.
 */
static int
fetch_and_remove_shift_suffix(char *num)
{
    char *end;
    int shift, last;

    end = &num[strlen(num) - 1];
    last = *end;
    if(islower(last)) /* Check not actually needed under ANSI C. */
        last = toupper(last);

    if(last == 'K')
        shift = 10; /* 1K == 1 << 10 */
    else if(last == 'M')
        shift = 20; /* 1M == 1 << 20 */
    else
        shift = 0;

    if(shift)
        *end = '\0';

    return shift;
}

/* Parses the given input string as a number of base strlen(digits)
 * with the specified digits.  Returns by reference the parsed value
 * as (*vp / *divisorp), which may be fractional.  All letters in the
 * digits string must be uppercase.  All input characters are
 * converted to upper case.
 */
static bool
parse_base_number(const char *s, const char *digits, long long *vp,
                  long long *divisorp)
{
    long long v, divisor;
    bool found_dot_p;
    size_t radix;

    /* Make sure there's at least one digit. */
    if(s[0] == '\0')
        return false;

    radix = strlen(digits);

    for(v = 0, divisor = 1, found_dot_p = false; *s; s++)
    {
        const char *p;
        int n;

        n = toupper(*s);
        if(n == '.')
        {
            if(found_dot_p) /* Only one decimal point allowed. */
                return false;
            found_dot_p = true;
        }
        else
        {
            p = strchr(digits, n);
            if(p == NULL)
                return false;
            if(v > 0x7FFFFFFFFFFFFFFFLL / radix /* Check overflow. */
               || divisor > 0x7FFFFFFFFFFFFFFFLL / radix)
                return false;
            v = (v * radix) + (p - digits);
            if(found_dot_p)
                divisor *= radix;
        }
    }

    *vp = v;
    *divisorp = divisor;
    return true;
}

/* Parse a number specified in a flexible format.  Returns true iff
 * the parse was successful and the result didn't exceed 32 bits,
 * else false.  Returns by reference the parsed value in *val.
 *
 * Each number may have an optional preceding "+" or "-" sign, which
 * do the obvious thing.
 * 
 * Base numbers may be specified in several ways:
 *
 * "123"	; decimal number
 * "146.5"	; floating point value
 * "0x12AC"	; hexadecimal number
 * "0x12AC.12A"	; hexadecimal floating point
 * "0b010010"	; binary number
 * "0b010.10"	; binary floating point
 *
 * I decided not to allow octal because that might confuse
 * unsophisticated users who accidentally precede a number with a
 * leading zero.
 *
 * The number may be followed by an optional case-insensitive suffix:
 *
 * "K" or "k"	; multiply the value by 1024 (1K)
 * "M" or "m"	; multiply the value by 1024*1024 (1M)
 *
 * Floating point results will be rounded toward zero before they are
 * returned.  The rounding happens *after* any multiplication suffix
 * is applied, so "1.5K" is exactly equivalent to "1536".
 *
 * The truncated result can then be rounded up to the next highest
 * (farther from zero) multiple of a specified number.  If you don't
 * want rounding, just pass "1" for round_up_to_multiple_of.
 */
bool Executor::parse_number(string orig_num, int32 *val,
                            unsigned round_up_to_multiple_of)
{
    long long orig_raw_val, raw_val, div;
    int shift_factor;
    bool negate_p, parsed_p;
    int32 v;
    char *num;

    /* Default value to be safe. */
    *val = 0;

    /* Empty string is an error.  We check this here to simplify checks later. */
    if(orig_num == "" || orig_num[0] == '\0')
        return false;

    num = (char *)alloca(orig_num.length() + 1);
    strcpy(num, orig_num.c_str());

    /* Check for leading + or -, and advance past it. */
    negate_p = (num[0] == '-');
    num += (num[0] == '-' || num[0] == '+');

    /* Look for a trailing suffix. */
    shift_factor = fetch_and_remove_shift_suffix(num);

    /* Figure out which parsing function to use. */
    if(num[0] == '0' && num[1] == 'x')
        parsed_p = parse_base_number(num + 2, "0123456789ABCDEF", &raw_val, &div);
    else if(num[0] == '0' && num[1] == 'b')
        parsed_p = parse_base_number(num + 2, "01", &raw_val, &div);
    else
        parsed_p = parse_base_number(num, "0123456789", &raw_val, &div);

    /* Make sure we parsed successfully. */
    if(!parsed_p)
        return false;

    /* Compute the resulting number by applying the scaling factors. */
    orig_raw_val = raw_val;
    raw_val <<= shift_factor;
    if(raw_val >> shift_factor != orig_raw_val) /* exceeded precision? */
        return false;
    raw_val /= div;

    /* Make sure we didn't exceed the precision of the result. */
    v = (int32)raw_val;
    if(v != raw_val)
        return false;

    /* Round up if so specified. */
    if(round_up_to_multiple_of > 1)
    {
        v += round_up_to_multiple_of - 1;
        v -= v % round_up_to_multiple_of;
    }

    *val = negate_p ? -v : v;

    return true;
}
