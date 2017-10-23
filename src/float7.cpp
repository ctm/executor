/* Copyright 1990, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_float7[] =
	    "$Id: float7.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in SANE.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "SANE.h"
#include "rsys/float.h"
#include <ctype.h>

using namespace Executor;

#define LOWER(x)   ((x) | 0x20)     /* converts to lower case if x is A-Z */

#if defined (BINCOMPAT)

P3(PUBLIC pascal trap, void, ROMlib_Fdec2str, DecForm * volatile, sp2,
				   Decimal * volatile, sp, Decstr volatile, dp)

{
    int i, j, exponent, expsize, beforedecimal, afterdecimal;
    int digits, style;

#define MAXEXPSIZE 8		/* maybe 6 or 7, but just in case. */
#define MAXDECSTRLEN 80
    char backwardsexp[MAXEXPSIZE];

    warning_floating_point (NULL_STRING);
    digits = CW (sp2->digits);
    style = CW (sp2->style);

    switch(style & DECIMALTYPEMASK) {
	case FloatDecimal:
	    if (sp->sgn)
		dp[1] = '-';
	    else
		dp[1] = ' ';
	    dp[2] = sp->sig[1];
	    if ((sp->sig[0] > 1) || digits > 1) {
		dp[3] = '.';
		for ( i = 2 ; i <= sp->sig[0] ; i++)
		    dp[i+2] = sp->sig[i];
		while ( i < digits) {
		    dp[i + 2] = '0';
		    i++;
		}
	    } else {
		i = 1;
	    }
	    dp[i + 2] = 'e';
	    exponent = CW (sp->exp) + sp->sig[0] - 1;
	    if (exponent < 0) {
		dp[i+3] = '-';
		exponent = -exponent;
	    } else {
		dp[i+3] = '+';
	    }
	    expsize = 1;
	    backwardsexp[1] = (exponent % 10) + '0';
	    exponent /= 10;
	    while (exponent) {
		expsize++;
		backwardsexp[expsize] = (exponent % 10) + '0';
		exponent /= 10;
	    }
	    i += 3;
	    for (j = expsize ; j ; j--) {
		i++;
		dp[i] = backwardsexp[j];
	    }
	    dp[0] = i;
	    break;
	case FixedDecimal:
	    if (sp->sig[1] == '0') {
		beforedecimal = 0;
		sp->sig[0] = 0;
	    } else {
		beforedecimal = sp->sig[0] + CW (sp->exp);
	    }
	    afterdecimal = (-CW (sp->exp) > digits) ? -CW (sp->exp) : digits;
	    if (sp->sgn) {
		dp[1] = '-';
		i = 1;
	    } else
		i = 0;
	    if (beforedecimal <= 0) {
		i++;
		dp[i] = '0';
	    }
	    j = 1;
	    while ( (j <= sp->sig[0]) && (beforedecimal > 0)) {
		beforedecimal--;
		i++;
		dp[i] = sp->sig[j];
		j++;
	    }
	    while (beforedecimal > 0) {
		beforedecimal--;
		i++;
		dp[i] = '0';
	    }
	    if ( j <= sp->sig[0] ) {
		i++;
		dp[i] = '.';
	    }
	    afterdecimal = 0;
	    while ((beforedecimal < 0)
		   && (!digits || (afterdecimal < digits))) {
		afterdecimal++;
		beforedecimal++;
		i++;
		dp[i] = '0';
	    }
	    while ( j <= sp->sig[0] ) {
		afterdecimal++;
		i++;
		dp[i] = sp->sig[j];
		j++;
	    }
	    if (( afterdecimal == 0) && digits > 0) {
		i++;
		dp[i] = '.';
	    }
	    for ( ; afterdecimal < digits ; afterdecimal++){
		i++;
		dp[i] = '0';
	    }
	    dp[0] = i;
	    break;
#if !defined (NDEBUG)
	default:
	    gui_abort();
	    break;
#endif /* NDEBUG */
    }
    ;
}
    

P5(PUBLIC pascal trap, void, ROMlib_Fxstr2dec, Decstr volatile, sp2,
	  INTEGER * volatile, sp, Decimal * volatile, dp2, Byte * volatile, dp,
							    INTEGER, lastchar)
{
    int index, expsgn, implicitexp;
    
    index = CW_RAW (*sp);
    warning_floating_point ("xstr2dec(\"%.*s\")",
			    lastchar - index + 1, (const char *) sp2 + index);
    while (((sp2[index] == ' ')||(sp2[index] == '\t')) && (index <= lastchar))
	index++;
    if (sp2[index] == '-') {
	dp2->sgn = 1;
	index++;
    } else if (sp2[index] == '+') {
	dp2->sgn = 0;
	index++;
    } else
	dp2->sgn = 0;
	
    dp2->sig[0] = 0;
    while (isdigit(sp2[index]) && (index <= lastchar)) {
	dp2->sig[0]++;
	dp2->sig[dp2->sig[0]] = sp2[index];
	index++;
    }
    implicitexp = dp2->sig[0];
    if (sp2[index] == '.') {
	index++;
	while (isdigit(sp2[index]) && (index <= lastchar)) {
	    dp2->sig[0]++;
	    dp2->sig[dp2->sig[0]] = sp2[index];
	    index++;
	}
    }
    if (!dp2->sig[0]){
	dp2->sig[0] = 0;
	dp2->sig[1] = 'N';
/*-->*/ goto abortlookahead;  /* should I use a break or return instead? */
    }
    *sp = CW_RAW (index);		/* The base is legit.  Check exponent. */
    dp2->exp =  CWC (0);
    if (LOWER(sp2[index]) == 'e') {
	index++;
	if (sp2[index] == '-') {
	    expsgn = 1;
	    index++;
	} else if (sp2[index] == '+') {
	    expsgn = 0;
	    index++;
	} else
	    expsgn = 0;
	if (!isdigit(sp2[index])) {
/*-->*/     goto abortlookahead;  /* should I use a break or return instead? */
        }
	while (isdigit(sp2[index]) && (index <= lastchar)) {
	    INTEGER newexp = CW (dp2->exp);
	    newexp *= 10;
	    newexp += sp2[index] - '0';
	    dp2->exp = CW (newexp);
	    index++;
	}
	if (expsgn)
	  dp2->exp = CW (-1 * CW (dp2->exp));
    }
    *sp = CW_RAW (index);
abortlookahead:
    dp2->exp = CW (CW (dp2->exp) + implicitexp - dp2->sig[0]);
    *dp = CB (!sp2[index] || (index > lastchar));
    while (dp2->sig[0] > 1 && dp2->sig[1] == '0')	/* gunch leading */
	memmove(dp2->sig+1, dp2->sig+2, --dp2->sig[0]);	/* zeros */

    warning_floating_point ("xstr2dec returning %s%.*s * 10**%d",
			    dp2->sgn ? "-" : "",
			    dp2->sig[0], dp2->sig + 1, CW (dp2->exp));
}


P4(PUBLIC pascal trap, void, ROMlib_Fcstr2dec, Decstr volatile, sp2,
	  INTEGER * volatile, sp, Decimal * volatile, dp2, Byte * volatile, dp)
/*
 * NOTE: sp2 is really supposed to be a Decstr, but it is going to
 * be used as a C *char or a Pascal string depending upon the
 * function.
 */

{
    warning_floating_point (NULL_STRING);
#if 1
    C_ROMlib_Fxstr2dec(sp2, sp, dp2, dp, 32767);
#else /* 0 */
    int index, implicitexp, expsgn;

#error No one has put byte swapping code in me yet!

    index = *sp;
    while ((sp2[index] == ' ') || (sp2[index] == '\t'))
	index++;
    if (sp2[index] == '-') {
	dp2->sgn = 1;
	index++;
    } else if (sp2[index] == '+') {
	dp2->sgn = 0;
	index++;
    } else
	dp2->sgn = 0;
	
    dp2->sig[0] = 0;
    while (isdigit(sp2[index])) {
	dp2->sig[0]++;
	dp2->sig[dp2->sig[0]] = sp2[index];
	index++;
	}
    implicitexp = dp2->sig[0];
    if (sp2[index] == '.') {
	index++;
	while (isdigit(sp2[index])) {
	    dp2->sig[0]++;
	    dp2->sig[dp2->sig[0]] = sp2[index];
	    index++;
	    }
	}
    if (!dp2->sig[0]){
	dp2->sig[0] = 0;
	dp2->sig[1] = 'N';
/*-->*/ goto abortlookahead;  /* should I use a break or return instead? */
        }
    *sp = index;		/* The base is legit.  Check exponent. */
    dp2->exp =  0;
    if (LOWER(sp2[index]) == 'e') {
	index++;
	if (sp2[index] == '-') {
	    expsgn = 1;
	    index++;
	} else if (sp2[index] == '+') {
	    expsgn = 0;
	    index++;
	} else
	    expsgn = 0;
	if (!isdigit(sp2[index])) {
/*-->*/     goto abortlookahead;  /* should I use a break or return instead? */
        }
	while (isdigit(sp2[index])) {
	    dp2->exp *= 10;
	    dp2->exp += sp2[index] - '0';
	    index++;
	    }
	}
    *sp = index;
abortlookahead:
    dp2->exp += implicitexp - dp2->sig[0];
    *dp = !sp2[index];
#endif /* 0 */
}


P4(PUBLIC pascal trap, void, ROMlib_Fpstr2dec, Decstr volatile, sp2,
	  INTEGER * volatile, sp, Decimal * volatile, dp2, Byte * volatile, dp)
/*
 * NOTE: sp2 is really supposed to be a Decstr, but it is going to
 * be used as a C *char or a Pascal string depending upon the
 * function.
 */

{
    warning_floating_point (NULL_STRING);
#if 1
    C_ROMlib_Fxstr2dec(sp2, sp, dp2, dp, sp2[0]);
#else /* 0 */
    int index, implicitexp, expsgn, lastchar;

#error No one has put byte swapping code in me yet!
    
    index = *sp;
    lastchar = index + sp2[0];
    while (((sp2[index] == ' ')||(sp2[index] == '\t')) && (index <= lastchar))
	index++;
    if (sp2[index] == '-') {
	dp2->sgn = 1;
	index++;
    } else if (sp2[index] == '+') {
	dp2->sgn = 0;
	index++;
    } else
	dp2->sgn = 0;
	
    dp2->sig[0] = 0;
    while (isdigit(sp2[index]) && (index <= lastchar)) {
	dp2->sig[0]++;
	dp2->sig[dp2->sig[0]] = sp2[index];
	index++;
	}
    implicitexp = dp2->sig[0] - 1;
    if (sp2[index] == '.') {
	index++;
	while (isdigit(sp2[index]) && (index <= lastchar)) {
	    dp2->sig[0]++;
	    dp2->sig[dp2->sig[0]] = sp2[index];
	    index++;
	    }
	}
    if (!dp2->sig[0]){
	dp2->sig[0] = 0;
	dp2->sig[1] = 'N';
/*-->*/ goto abortlookahead;  /* should I use a break or return instead? */
        }
    *sp = index;		/* The base is legit.  Check exponent. */
    dp2->exp =  0;
    if (LOWER(sp2[index]) == 'e') {
	index++;
	if (sp2[index] == '-') {
	    expsgn = 1;
	    index++;
	} else if (sp2[index] == '+') {
	    expsgn = 0;
	    index++;
	} else
	    expsgn = 0;
	if (!isdigit(sp2[index])) {
/*-->*/     goto abortlookahead;  /* should I use a break or return instead? */
        }
	while (isdigit(sp2[index]) && (index <= lastchar)) {
	    dp2->exp *= 10;
	    dp2->exp += sp2[index] - '0';
	    index++;
	    }
	}
    *sp = index;
abortlookahead:
    dp2->exp += implicitexp;
    *dp = (index > lastchar);
#endif /* 0 */
}

#endif /* BINCOMPAT */
