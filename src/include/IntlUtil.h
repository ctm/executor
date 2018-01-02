#if !defined(_INTLUTIL_H_)
#define _INTLUTIL_H_

#include "SANE.h"

/*
 * Copyright 1986, 1989, 1990, 1994 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
enum
{
    currSymLead = 16,
    currNegSym = 32,
    currTrailingZ = 64,
    currLeadingZ = 128,
};

enum
{
    mdy = 0,
    dmy = 1,
    ymd = 2,
};

enum
{
    dayLdingZ = 32,
    mntLdingZ = 64,
    century = 128,
};

enum
{
    secLeadingZ = 32,
    minLeadingZ = 64,
    hrLeadingZ = 128,
};

enum
{
    verUS = 0,
    verFrance = 1,
    verBritain = 2,
    verGermany = 3,
    verItaly = 4,
    verNetherlands = 5,
    verBelgiumLux = 6,
    verSweden = 7,
    verSpain = 8,
    verDenmark = 9,
    verPortugal = 10,
    verFrCanada = 11,
    verNorway = 12,
    verIsreal = 13,
    verJapan = 14,
    verAustralia = 15,
    verArabia = 16,
    verFinland = 17,
    verFrSwiss = 18,
    verGrSwiss = 19,
    verGreece = 20,
    verIceland = 21,
    verMalta = 22,
    verCyprus = 23,
    verTurkey = 24,
    verYugoslavia = 25,
};

struct Intl0Rec
{
    GUEST_STRUCT;
    GUEST<Byte> decimalPt;
    GUEST<Byte> thousSep;
    GUEST<Byte> listSep;
    GUEST<Byte> currSym1;
    GUEST<Byte> currSym2;
    GUEST<Byte> currSym3;
    GUEST<Byte> currFmt;
    GUEST<Byte> dateOrder;
    GUEST<Byte> shrtDateFmt;
    GUEST<Byte> dateSep;
    GUEST<Byte> timeCycle;
    GUEST<Byte> timeFmt;
    GUEST<LONGINT> mornStr;
    GUEST<LONGINT> eveStr;
    GUEST<Byte> timeSep;
    GUEST<Byte> time1Suff;
    GUEST<Byte> time2Suff;
    GUEST<Byte> time3Suff;
    GUEST<Byte> time4Suff;
    GUEST<Byte> time5Suff;
    GUEST<Byte> time6Suff;
    GUEST<Byte> time7Suff;
    GUEST<Byte> time8Suff;
    GUEST<Byte> metricSys;
    GUEST<INTEGER> intl0Vers;
};
typedef Intl0Rec *Intl0Ptr;

typedef GUEST<Intl0Ptr> *Intl0Hndl;

typedef Byte STRING15[16];

struct Intl1Rec
{
    GUEST_STRUCT;
    GUEST<STRING15[7]> days;
    GUEST<STRING15[12]> months;
    GUEST<Byte> suppressDay;
    GUEST<Byte> lngDateFmt;
    GUEST<Byte> dayLeading0;
    GUEST<Byte> abbrLen;
    GUEST<LONGINT> st0;
    GUEST<LONGINT> st1;
    GUEST<LONGINT> st2;
    GUEST<LONGINT> st3;
    GUEST<LONGINT> st4;
    GUEST<INTEGER> intl1Vers;
    GUEST<INTEGER> localRtn;
};
typedef Intl1Rec *Intl1Ptr;

typedef GUEST<Intl1Ptr> *Intl1Hndl;

typedef comp LongDateTime;

#if !defined(BINCOMPAT)

typedef enum { shortDate,
               longDate,
               abbrevDate } DateForm;

#else /* BINCOMPAT */

typedef SignedByte DateForm;
enum
{
    shortDate = 0,
    longDate = 1,
    abbrevDate = 2,
};

#endif /* BINCOMPAT */

/* DO NOT DELETE THIS LINE */

extern void C_IUDatePString(LONGINT date,
                                        DateForm form, StringPtr p, Handle h);
PASCAL_FUNCTION(IUDatePString);
extern Handle C_IUGetIntl(INTEGER id);
PASCAL_FUNCTION(IUGetIntl);

extern void C_IUDateString(LONGINT date,
                                       DateForm form, StringPtr p);
PASCAL_FUNCTION(IUDateString);
extern void C_IUTimePString(LONGINT date,
                                        BOOLEAN secs, StringPtr p, Handle h);
PASCAL_FUNCTION(IUTimePString);
extern void C_IUTimeString(LONGINT date,
                                       BOOLEAN secs, StringPtr p);
PASCAL_FUNCTION(IUTimeString);
extern BOOLEAN C_IUMetric(void);
PASCAL_FUNCTION(IUMetric);

extern void C_IUSetIntl(INTEGER rn,
                                    INTEGER id, Handle newh);
PASCAL_FUNCTION(IUSetIntl);

extern INTEGER C_IUMagString(Ptr ptr1, Ptr ptr2, INTEGER len1, INTEGER len2);
PASCAL_FUNCTION(IUMagString);

extern INTEGER IUCompString(StringPtr str1,
                            StringPtr str2);
extern INTEGER C_IUMagIDString(Ptr ptr1,
                                           Ptr ptr2, INTEGER len1, INTEGER len2);
PASCAL_FUNCTION(IUMagIDString);
extern INTEGER IUEqualString(StringPtr str1,
                             StringPtr str2);
extern void C_IUMystery(Ptr arg1, Ptr arg2,
                                    INTEGER arg3, INTEGER arg4);
PASCAL_FUNCTION(IUMystery);

extern void C_IULDateString(LongDateTime *datetimep,
                                        DateForm longflag, Str255 result, Handle intlhand);
PASCAL_FUNCTION(IULDateString);

extern void C_IULTimeString(LongDateTime *datetimep,
                                        BOOLEAN wantseconds, Str255 result, Handle intlhand);
PASCAL_FUNCTION(IULTimeString);

extern void C_IUClearCache(void);
PASCAL_FUNCTION(IUClearCache);

extern INTEGER C_IUMagPString(Ptr ptra, Ptr ptrb, INTEGER lena,
                                          INTEGER lenb, Handle itl2hand);
PASCAL_FUNCTION(IUMagPString);

extern INTEGER C_IUMagIDPString(Ptr ptra, Ptr ptrb, INTEGER lena,
                                            INTEGER lenb, Handle itl2hand);
PASCAL_FUNCTION(IUMagIDPString);

extern INTEGER C_IUScriptOrder(ScriptCode script1,
                                           ScriptCode script2);
PASCAL_FUNCTION(IUScriptOrder);

extern INTEGER C_IULangOrder(LangCode l1, LangCode l2);
PASCAL_FUNCTION(IULangOrder);

extern INTEGER C_IUTextOrder(Ptr ptra, Ptr ptrb, INTEGER lena,
                                         INTEGER lenb, ScriptCode scripta, ScriptCode bscript, LangCode langa,
                                         LangCode langb);
PASCAL_FUNCTION(IUTextOrder);

extern void C_IUGetItlTable(ScriptCode script, INTEGER tablecode,
                                        Handle *itlhandlep, LONGINT *offsetp, LONGINT *lengthp);
PASCAL_FUNCTION(IUGetItlTable);
}
#endif /* _INTLUTIL_H_ */
