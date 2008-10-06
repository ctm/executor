#if !defined (_INTLUTIL_H_)
#define _INTLUTIL_H_

#include "SANE.h"

/*
 * Copyright 1986, 1989, 1990, 1994 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: IntlUtil.h 63 2004-12-24 18:19:43Z ctm $
 */

#define currSymLead	16
#define currNegSym	32
#define currTrailingZ	64
#define currLeadingZ	128

#define mdy		0
#define dmy		1
#define ymd		2

#define dayLdingZ	32
#define mntLdingZ	64
#define century		128

#define secLeadingZ	32
#define minLeadingZ	64
#define hrLeadingZ	128

#define verUS		0
#define verFrance	1
#define verBritain	2
#define verGermany	3
#define verItaly	4
#define verNetherlands	5
#define verBelgiumLux	6
#define verSweden	7
#define verSpain	8
#define verDenmark	9
#define verPortugal	10
#define verFrCanada	11
#define verNorway	12
#define verIsreal	13
#define verJapan	14
#define verAustralia	15
#define verArabia	16
#define verFinland	17
#define verFrSwiss	18
#define verGrSwiss	19
#define verGreece	20
#define verIceland	21
#define verMalta	22
#define verCyprus	23
#define verTurkey	24
#define verYugoslavia	25

typedef struct {
    Byte decimalPt	PACKED;
    Byte thousSep	PACKED;
    Byte listSep	PACKED;
    Byte currSym1	PACKED;
    Byte currSym2	PACKED;
    Byte currSym3	PACKED;
    Byte currFmt	PACKED;
    Byte dateOrder	PACKED;
    Byte shrtDateFmt	PACKED;
    Byte dateSep	PACKED;
    Byte timeCycle	PACKED;
    Byte timeFmt	PACKED;
    LONGINT mornStr	PACKED;
    LONGINT eveStr	PACKED;
    Byte timeSep	PACKED;
    Byte time1Suff	PACKED;
    Byte time2Suff	PACKED;
    Byte time3Suff	PACKED;
    Byte time4Suff	PACKED;
    Byte time5Suff	PACKED;
    Byte time6Suff	PACKED;
    Byte time7Suff	PACKED;
    Byte time8Suff	PACKED;
    Byte metricSys	PACKED;
    INTEGER intl0Vers	PACKED;
} Intl0Rec;
typedef Intl0Rec *Intl0Ptr;
typedef struct { Intl0Ptr p PACKED_P; } HIDDEN_Intl0Ptr;
typedef HIDDEN_Intl0Ptr *Intl0Hndl;

typedef Byte STRING15[16];

typedef struct {
    STRING15 days[7]	PACKED;
    STRING15 months[12]	PACKED;
    Byte suppressDay	PACKED;
    Byte lngDateFmt	PACKED;
    Byte dayLeading0	PACKED;
    Byte abbrLen	PACKED;
    LONGINT st0	PACKED;
    LONGINT st1	PACKED;
    LONGINT st2	PACKED;
    LONGINT st3	PACKED;
    LONGINT st4	PACKED;
    INTEGER intl1Vers	PACKED;
    INTEGER localRtn	PACKED;
} Intl1Rec;
typedef Intl1Rec *Intl1Ptr;
typedef struct { Intl1Ptr p PACKED_P; } HIDDEN_Intl1Ptr;
typedef HIDDEN_Intl1Ptr *Intl1Hndl;

typedef comp LongDateTime;

#if !defined (BINCOMPAT)

typedef enum { shortDate, longDate, abbrevDate } DateForm;

#else /* BINCOMPAT */

typedef SignedByte DateForm;
#define shortDate	0
#define longDate	1
#define abbrevDate	2

#endif /* BINCOMPAT */


/* DO NOT DELETE THIS LINE */

extern pascal trap void C_IUDatePString( LONGINT date, 
 DateForm form, StringPtr p, Handle h ); extern pascal trap void P_IUDatePString( LONGINT date, 
 DateForm form, StringPtr p, Handle h ); 
extern pascal trap Handle C_IUGetIntl( INTEGER id ); extern pascal trap Handle P_IUGetIntl( INTEGER id); 
extern pascal trap void C_IUDateString( LONGINT date, 
 DateForm form, StringPtr p ); extern pascal trap void P_IUDateString( LONGINT date, 
 DateForm form, StringPtr p ); 
extern pascal trap void C_IUTimePString( LONGINT date, 
 BOOLEAN secs, StringPtr p, Handle h ); extern pascal trap void P_IUTimePString( LONGINT date, 
 BOOLEAN secs, StringPtr p, Handle h ); 
extern pascal trap void C_IUTimeString( LONGINT date, 
 BOOLEAN secs, StringPtr p ); extern pascal trap void P_IUTimeString( LONGINT date, 
 BOOLEAN secs, StringPtr p ); 
extern pascal trap BOOLEAN C_IUMetric( void  ); extern pascal trap BOOLEAN P_IUMetric( void ); 
extern pascal trap void C_IUSetIntl( INTEGER rn, 
 INTEGER id, Handle newh ); extern pascal trap void P_IUSetIntl( INTEGER rn, 
 INTEGER id, Handle newh );

extern pascal trap INTEGER C_IUMagString (Ptr ptr1, Ptr ptr2, INTEGER len1, INTEGER len2);

extern INTEGER IUCompString( StringPtr str1, 
 StringPtr str2 ); 
extern pascal trap INTEGER C_IUMagIDString( Ptr ptr1, 
 Ptr ptr2, INTEGER len1, INTEGER len2 ); extern pascal trap INTEGER P_IUMagIDString( Ptr ptr1, 
 Ptr ptr2, INTEGER len1, INTEGER len2 ); 
extern INTEGER IUEqualString( StringPtr str1, 
 StringPtr str2 ); 
extern pascal trap void C_IUMystery( Ptr arg1, Ptr arg2, 
 INTEGER arg3, INTEGER arg4 ); extern pascal trap void P_IUMystery( Ptr arg1, Ptr arg2, 
 INTEGER arg3, INTEGER arg4 ); 

extern pascal trap void C_IULDateString(LongDateTime *datetimep,
			    DateForm longflag, Str255 result, Handle intlhand);

extern pascal trap void C_IULTimeString(LongDateTime *datetimep,
			  BOOLEAN wantseconds, Str255 result, Handle intlhand);

extern pascal trap void C_IUClearCache( void );

extern pascal trap INTEGER C_IUMagPString( Ptr ptra, Ptr ptrb, INTEGER lena,
						INTEGER lenb, Handle itl2hand);

extern pascal trap INTEGER C_IUMagIDPString( Ptr ptra, Ptr ptrb, INTEGER lena,
					        INTEGER lenb, Handle itl2hand);

extern pascal trap INTEGER C_IUScriptOrder( ScriptCode script1,
							   ScriptCode script2);

extern pascal trap INTEGER C_IULangOrder( LangCode l1, LangCode l2);

extern pascal trap INTEGER C_IUTextOrder( Ptr ptra, Ptr ptrb, INTEGER lena,
	INTEGER lenb, ScriptCode scripta, ScriptCode bscript, LangCode langa,
							       LangCode langb);

extern pascal trap void C_IUGetItlTable( ScriptCode script, INTEGER tablecode,
		       Handle *itlhandlep, LONGINT *offsetp, LONGINT *lengthp);

#endif /* _INTLUTIL_H_ */
