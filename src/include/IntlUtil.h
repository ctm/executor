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

typedef struct PACKED {
  Byte decimalPt;
  Byte thousSep;
  Byte listSep;
  Byte currSym1;
  Byte currSym2;
  Byte currSym3;
  Byte currFmt;
  Byte dateOrder;
  Byte shrtDateFmt;
  Byte dateSep;
  Byte timeCycle;
  Byte timeFmt;
  LONGINT mornStr;
  LONGINT eveStr;
  Byte timeSep;
  Byte time1Suff;
  Byte time2Suff;
  Byte time3Suff;
  Byte time4Suff;
  Byte time5Suff;
  Byte time6Suff;
  Byte time7Suff;
  Byte time8Suff;
  Byte metricSys;
  INTEGER intl0Vers;
} Intl0Rec;
typedef Intl0Rec *Intl0Ptr;
MAKE_HIDDEN(Intl0Ptr);
typedef HIDDEN_Intl0Ptr *Intl0Hndl;

typedef Byte STRING15[16];

typedef struct PACKED {
  STRING15 days[7];
  STRING15 months[12];
  Byte suppressDay;
  Byte lngDateFmt;
  Byte dayLeading0;
  Byte abbrLen;
  LONGINT st0;
  LONGINT st1;
  LONGINT st2;
  LONGINT st3;
  LONGINT st4;
  INTEGER intl1Vers;
  INTEGER localRtn;
} Intl1Rec;
typedef Intl1Rec *Intl1Ptr;
MAKE_HIDDEN(Intl1Ptr);
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
