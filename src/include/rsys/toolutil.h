#if !defined(__RSYS_TOOLUTIL__)
#define __RSYS_TOOLUTIL__

#include "SANE.h"

extern StringHandle ROMlib_phoney_name_string;

extern void unpack_int16_bits (HIDDEN_Ptr *sp, HIDDEN_Ptr *dp, INTEGER len);
extern trap void R_Fix2X (void *dummyretpc, Fixed x, extended80 *ret);
extern trap void R_Frac2X(void *dummyretpc, Fract x, extended80 *ret);

#endif /* !defined(__RSYS_TOOLUTIL__) */
