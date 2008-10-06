#if !defined(__RSYS_OSUTIL__)
#define __RSYS_OSUTIL__

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: osutil.h 63 2004-12-24 18:19:43Z ctm $
 */

extern long long ROMlib_long_long_secs (INTEGER year, INTEGER month,
					INTEGER day, INTEGER hour,
					INTEGER minute, INTEGER second);

extern void date_to_swapped_fields (long long mactime, INTEGER *yearp,
				    INTEGER *monthp, INTEGER *dayp,
				    INTEGER *hourp, INTEGER *minutep,
				    INTEGER *secondp, INTEGER *dayofweekp,
				    INTEGER *dayofyearp, INTEGER *weekofyearp);

extern void C_ROMlib_wakeup( void );

#define PARAMRAMMACNAME	"\010ParamRAM"
     
extern LONGINT ROMlib_GMTcorrect;	/* Correction for GMT to localtime */

#define U70MINUSM04	2082844800	/* 1/1/1970 - 1/1/1904 */
#define UNIXTIMETOMACTIME(x) ((x) - ROMlib_GMTcorrect + U70MINUSM04)
#define MACTIMETOLUNIXTIME(x) ((x) - U70MINUSM04)
#define MACTIMETOGUNIXTIME(x) ((x) + ROMlib_GMTcorrect - U70MINUSM04)

extern char ROMlib_phoneyrom[];

#endif /* ! efined(__RSYS_OSUTIL__) */
