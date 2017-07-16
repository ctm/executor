#if !defined(__RSYS_OSUTIL__)
#define __RSYS_OSUTIL__

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: osutil.h 63 2004-12-24 18:19:43Z ctm $
 */
namespace Executor {
extern long long ROMlib_long_long_secs (Executor::INTEGER year, Executor::INTEGER month,
					Executor::INTEGER day, Executor::INTEGER hour,
					Executor::INTEGER minute, Executor::INTEGER second);

extern void date_to_swapped_fields (long long mactime, Executor::INTEGER *yearp,
				    Executor::INTEGER *monthp, Executor::INTEGER *dayp,
				    Executor::INTEGER *hourp, Executor::INTEGER *minutep,
				    Executor::INTEGER *secondp, Executor::INTEGER *dayofweekp,
				    Executor::INTEGER *dayofyearp, Executor::INTEGER *weekofyearp);

extern void C_ROMlib_wakeup( void );

#define PARAMRAMMACNAME	"\010ParamRAM"
     
extern Executor::LONGINT ROMlib_GMTcorrect;	/* Correction for GMT to localtime */

#define U70MINUSM04	2082844800	/* 1/1/1970 - 1/1/1904 */
#define UNIXTIMETOMACTIME(x) ((x) - ROMlib_GMTcorrect + U70MINUSM04)
#define MACTIMETOLUNIXTIME(x) ((x) - U70MINUSM04)
#define MACTIMETOGUNIXTIME(x) ((x) + ROMlib_GMTcorrect - U70MINUSM04)

extern char ROMlib_phoneyrom[];
}
#endif /* ! efined(__RSYS_OSUTIL__) */
