/* Copyright 1989, 1990, 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_iu[] =
		    "$Id: iu.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in IntlUtil.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "IntlUtil.h"
#include "OSUtil.h"
#include "ResourceMgr.h"
#include "MemoryMgr.h"
#include "BinaryDecimal.h"
#include "rsys/glue.h"
#include "rsys/resource.h"
#include "rsys/hook.h"
#include <ctype.h>

using namespace Executor;
using namespace ByteSwap;

/*
 * outl adds the characters in the four bytes of a longint to the place
 * pointed to by the pointer pointed to by opp, the pointer pointed to
 * by opp is incremented for each character that is added.
 */


namespace Executor {
  typedef struct {
    unsigned char primary;
    unsigned char secondary;
  } sorttype;
  
#if !defined (__STDC__)
  typedef	INTEGER (*locptype)();
#else /* __STDC__ */
  typedef	INTEGER (*locptype)( Ptr ptr, INTEGER len, sorttype *sp );
#endif /* __STDC__ */

  PRIVATE void outl(LONGINT, char**);
  PRIVATE void outs(StringPtr, INTEGER, char**);
  PRIVATE void outn(INTEGER, BOOLEAN, char**);
  PRIVATE void month(INTEGER, Intl0Ptr, char, char**);
  PRIVATE void day(INTEGER, Intl0Ptr, char, char**);
  PRIVATE void year(INTEGER, Intl0Ptr, char, char**);
  PRIVATE INTEGER defaultorder(unsigned char *, INTEGER,
							   sorttype *);
  PRIVATE INTEGER germanylocalization(unsigned char *, INTEGER,
									  sorttype*);
  PRIVATE INTEGER britainlocalization(unsigned char *, INTEGER,
									  sorttype *);
  PRIVATE INTEGER defaultlocalization(unsigned char *, INTEGER,
									  sorttype *);
  PRIVATE INTEGER iuhelper(Ptr, Ptr, INTEGER,
						   INTEGER, BOOLEAN);
}


A2(PRIVATE, void, outl, LONGINT, l, char **, opp)
{
    if (l & 0xFF000000) {
	*(*opp)++ = (l & 0xFF000000) >> 24;
	if (l & 0xFF0000) {
	    *(*opp)++ = (l & 0xFF0000) >> 16;
	    if (l & 0xFF00) {
		*(*opp)++ = (l & 0xFF00) >> 8;
		if (l & 0xFF)
		    *(*opp)++ = (l & 0xFF);
	    }
	}
    }
}

/*
 * outs is similar to outl, except it works with a string and has an additional
 * paramater that is used to specify that an exact number of characters are
 * transfered (used when doing abbreviations).
 */

/*
 * n is the number of characters to transfer, or 0 for entire string
 */

A3(PRIVATE, void, outs, StringPtr, p, INTEGER, n, char **, opp)
{
    int ntocopy;

    ntocopy = n ? n : p[0];	/* This can lead to copying NUL characters */
				/* which is prescribed in IMI-505 (ick) */
    BlockMove((Ptr) (p+1), (Ptr) *opp, (Size) ntocopy);
    *opp += ntocopy;
}

/*
 * outn is similar to outl and outs but it converts a number to a set
 * of characters and adds them to *opp.  If leading0 is TRUE and the number
 * is less than 10, a leading zero is output.
 */

A3(PRIVATE, void, outn, INTEGER, n, BOOLEAN, leading0, char **, opp)
{
    Str255 s;

    NumToString((LONGINT) n, s);
    if (leading0 && n < 10)
	*(*opp)++ = '0';
    BlockMove((Ptr) (s+1), (Ptr) *opp, (Size) s[0]);
    *opp += s[0];
}

/*
 * month is similar to the outx routines but is used to print the numerical
 * form of the month and, if non-null, a separator.
 */

A4(PRIVATE, void, month, INTEGER, n, Intl0Ptr, int0p, char, sep, char **, opp)
{
    outn(n, Cx(int0p->shrtDateFmt) & mntLdingZ, opp);
    if (sep)
	*(*opp)++ = sep;
}

/*
 * day is similar to month
 */

A4(PRIVATE, void, day, INTEGER, n, Intl0Ptr, int0p, char, sep, char **, opp)
{
    outn(n, Cx(int0p->shrtDateFmt) & dayLdingZ, opp);
    if (sep)
	*(*opp)++ = sep;
}

/*
 * year is similar to month and day
 */

A4(PRIVATE, void, year, INTEGER, n, Intl0Ptr, int0p, char, sep, char **, opp)
{
    outn(Cx(int0p->shrtDateFmt) & century ? n : n % 100, FALSE, opp);
    if (sep)
	*(*opp)++ = sep;
}

P4(PUBLIC pascal trap, void, IUDatePString, LONGINT, date,	/* IMI-505 */
				     DateForm, form, StringPtr, p, Handle, h)
{
    Intl1Ptr int1p;
    Intl0Ptr int0p;
    char *op;
    DateTimeRec dtr;
    int abbrev;

    if (!h)
      h = IUGetIntl (form == shortDate ? 0 : 1);

    Secs2Date(date, &dtr);
    op = (char *) p + 1;
    if (form == shortDate) {
	if (h && (int0p = (Intl0Ptr) STARH(h))) {
	    switch (Cx(int0p->dateOrder)) {
	    case mdy:
		month(BigEndianValue(dtr.month), int0p, Cx(int0p->dateSep), &op);
		day  (BigEndianValue(dtr.day),   int0p, Cx(int0p->dateSep), &op);
		year (BigEndianValue(dtr.year),  int0p, 0,              &op);
		break;
	    case dmy:
		day  (BigEndianValue(dtr.day),   int0p, Cx(int0p->dateSep), &op);
		month(BigEndianValue(dtr.month), int0p, Cx(int0p->dateSep), &op);
		year (BigEndianValue(dtr.year),  int0p, 0,              &op);
		break;
	    case ymd:
		year (BigEndianValue(dtr.year),  int0p, Cx(int0p->dateSep), &op);
		month(BigEndianValue(dtr.month), int0p, Cx(int0p->dateSep), &op);
		day  (BigEndianValue(dtr.day),   int0p, 0,              &op);
		break;
	    }
	}
    } else {
	if (h && (int1p = (Intl1Ptr) STARH(h))) {
	    abbrev = form == longDate ? 0 : Cx(int1p->abbrLen);
	    outl(Cx(int1p->st0), &op);
	    if (!Cx(int1p->suppressDay)) {
		outs(int1p->days[BigEndianValue(dtr.dayOfWeek)-1], abbrev, &op);
		outl(Cx(int1p->st1), &op);
	    }
	    if (Cx(int1p->lngDateFmt)) {
		outs(int1p->months[BigEndianValue(dtr.month)-1], abbrev, &op);
		outl(Cx(int1p->st2), &op);
		outn(BigEndianValue(dtr.day), Cx(int1p->dayLeading0), &op);
	    } else {
		outn(BigEndianValue(dtr.day), Cx(int1p->dayLeading0), &op);
		outl(Cx(int1p->st2), &op);
		outs(int1p->months[BigEndianValue(dtr.month)-1], abbrev, &op);
	    }
	    outl(Cx(int1p->st3), &op);
	    outn(BigEndianValue(dtr.year), FALSE, &op);
	    outl(Cx(int1p->st4), &op);
	}
    }
    p[0] = op - (char *) p - 1;
}

P1(PUBLIC pascal trap, Handle, IUGetIntl, INTEGER, id)	/* IMI-505 */
{
    INTEGER oldres;
    Handle retval;

    oldres = CurResFile();
    UseResFile(0);
    retval = ROMlib_getrestid(TICK("INTL"), id);
    UseResFile(oldres);

    if (!retval && id == 4)
      {
	static Handle h = 0;

	warning_unimplemented (NULL_STRING);
	if (!h)
	  h = NewHandleClear (100);
	retval = h;
      }

    /* force dates to include century, otherwise we can run into y2k
       problems with programs like Quicken 6.0 */

    if (id == 0 && retval)
      {
	Intl0Ptr int0p;

	int0p = (Intl0Ptr) STARH (retval);
	int0p->shrtDateFmt |= CBC(century);
      }

    return retval;
}

P3(PUBLIC pascal trap, void, IUDateString, LONGINT, date,	/* IMI-504 */
						DateForm, form, StringPtr, p)
{
    IUDatePString(date, form, p, IUGetIntl(form == shortDate ? 0 : 1));
}

P4(PUBLIC pascal trap, void, IUTimePString, LONGINT, date,	/* IMI-505 */
				      BOOLEAN, secs, StringPtr, p, Handle, h)
{
    Intl0Ptr int0p;
    char *op;
    DateTimeRec dtr;
    char *ip, *ep;

    if (!h)
      h = IUGetIntl (0);

    op = (char *) p + 1;
    if (h && (int0p = (Intl0Ptr) STARH(h))) {
	Secs2Date(date, &dtr);
        if (int0p->timeCycle)
            outn((BigEndianValue(dtr.hour) % 12) == 0 ? 12 : BigEndianValue(dtr.hour) % 12, 
                 Cx(int0p->timeFmt) & hrLeadingZ, &op);
        else
            outn(BigEndianValue(dtr.hour), Cx(int0p->timeFmt) & hrLeadingZ, &op);
	*op++ = int0p->timeSep;
	outn(BigEndianValue(dtr.minute), Cx(int0p->timeFmt) & minLeadingZ, &op);
	if (secs) {
	    *op++ = int0p->timeSep;
	    outn(BigEndianValue(dtr.second), Cx(int0p->timeFmt) & secLeadingZ, &op);
	}
/* IMI-499 is misleading about the timenSuff fields.  The first four are
   used for AM, the second four for PM.  Yes, that's dumb.  But that's how
   the Mac works.  Sigh. */
	if (!int0p->timeCycle && BigEndianValue(dtr.hour) < 12)
	    for (ip = (char *) &int0p->time1Suff, ep = ip + 4;
							      ip != ep && *ip;)
		*op++ = *ip++;
        else if (!int0p->timeCycle) /* dtr.hour >= 12 */
            for (ip = (char *) &int0p->time5Suff, ep = ip + 4;
							      ip != ep && *ip;)
                *op++ = *ip++;
	else if (BigEndianValue(dtr.hour) < 12)
	    outl(Cx(int0p->mornStr), &op);
	else
	    outl(Cx(int0p->eveStr), &op);
    }
    p[0] = op - (char *) p - 1;
}

P3(PUBLIC pascal trap, void, IUTimeString, LONGINT, date,	/* IMI-505 */
						 BOOLEAN, secs, StringPtr, p)
{
    IUTimePString(date, secs, p, IUGetIntl(0));
}

P0(PUBLIC pascal trap, BOOLEAN, IUMetric)	/* IMI-505 */
{
    Handle h;

    h = IUGetIntl(0);
    return h ? ((Intl0Ptr) STARH(h))->metricSys : FALSE;
}

P3(PUBLIC pascal trap, void, IUSetIntl, INTEGER, rn,		/* IMI-506 */
						   INTEGER, id, Handle, newh)
{
    INTEGER oldcurmap;
    Handle h;

    oldcurmap = Cx(CurMap);
    UseResFile(rn);
    if (ResErr == noErr) {
	h = IUGetIntl(id);
	if (h && HomeResFile(h) == rn) {
	    if (id == 0)
		* (Intl0Ptr) STARH(h) = * (Intl0Ptr) STARH(newh);
	    else
		* (Intl1Ptr) STARH(h) = * (Intl1Ptr) STARH(newh);
	    ChangedResource(h);
	} else
	    AddResource(newh, TICK("INTL"), id, (StringPtr) 0);
    }
    UseResFile(oldcurmap);
}

PRIVATE sorttype highsortvalues[] = {
    { 'A',  0x02 }, { 'A',  0x04 }, { 'C',  0x01 }, { 'E',  0x01 },
    { 'N',  0x01 }, { 'O',  0x01 }, { 'U',  0x01 }, { 'A',  0x81 },
    { 'A',  0x82 }, { 'A',  0x83 }, { 'A',  0x84 }, { 'A',  0x85 },
    { 'A',  0x86 }, { 'C',  0x81 }, { 'E',  0x81 }, { 'E',  0x82 },
    { 'E',  0x83 }, { 'E',  0x84 }, { 'I',  0x81 }, { 'I',  0x82 },
    { 'I',  0x83 }, { 'I',  0x84 }, { 'N',  0x81 }, { 'O',  0x81 },
    { 'O',  0x82 }, { 'O',  0x83 }, { 'O',  0x84 }, { 'O',  0x85 },
    { 'U',  0x81 }, { 'U',  0x82 }, { 'U',  0x83 }, { 'U',  0x84 },
    { 0xA0, 0x00 }, { 0xA1, 0x00 }, { 0xA2, 0x00 }, { 0xA3, 0x00 },
    { 0xA4, 0x00 }, { 0xA5, 0x00 }, { 0xA6, 0x00 }, { 0xA7, 0x00 },
    { 0xA8, 0x00 }, { 0xA9, 0x00 }, { 0xAA, 0x00 }, { 0xAB, 0x00 },
    { 0xAC, 0x00 }, { 0xAD, 0x00 }, { 0xAE, 0x00 }, { 'O',  0x03 },
    { 0xB0, 0x00 }, { 0xB1, 0x00 }, { 0xB2, 0x00 }, { 0xB3, 0x00 },
    { 0xB4, 0x00 }, { 0xB5, 0x00 }, { 0xB6, 0x00 }, { 0xB7, 0x00 },
    { 0xB8, 0x00 }, { 0xB9, 0x00 }, { 0xBA, 0x00 }, { 0xBB, 0x00 },
    { 0xBC, 0x00 }, { 0xBD, 0x00 }, { 0xAE, 0x01 }, { 'O',  0x86 },
    { 0xC0, 0x00 }, { 0xC1, 0x00 }, { 0xC2, 0x00 }, { 0xC3, 0x00 },
    { 0xC4, 0x00 }, { 0xC5, 0x00 }, { 0xC6, 0x00 }, { 0x22, 0x01 },
    { 0x22, 0x02 }, { 0xC9, 0x00 }, { 0x20, 0x01 }, { 'A',  0x01 },
    { 'A',  0x03 }, { 'O',  0x02 }, { 0xAE, 0x02 }, { 0xAE, 0x03 },
    { 0xD0, 0x00 }, { 0xD1, 0x00 }, { 0x22, 0x03 }, { 0x22, 0x04 },
    { 0x27, 0x01 }, { 0x27, 0x02 }, { 0xD6, 0x00 }, { 0xD7, 0x00 },
    { 'Y',  0x00 },
};

A3(PRIVATE, INTEGER, defaultorder, unsigned char *, cp, INTEGER, len,
				    sorttype *, rp)	/* i.e. U.S. order */
{
    int index;
    int c;

    c = *cp;
    if (c & 0x80) {
	index = (c & 0x7F);
	if (index < (int) NELEM(highsortvalues))
	    *rp = highsortvalues[index];
	else {
	    rp->primary   = c;
	    rp->secondary = 0;
	}
    } else {
	if (islower(c)) {
	    rp->primary   = toupper(c);
	    rp->secondary = 0x80;
	} else {
	    rp->primary   = c;
	    rp->secondary = 0;
	}
    }
    return 1;
}

A3(PRIVATE, INTEGER, germanylocalization, unsigned char *, cp, INTEGER, len,
								sorttype *, rp)
{
    INTEGER retval;

    (void) defaultorder(cp, len, rp);
    retval = 1;
    switch (*cp) {
    case 'A':
	if (len > 1 && cp[1] == 'E') {	/* A E */
	    retval = 2;
	    rp->secondary = 0x02;
	}
	break;
    case 0x80:	/* A-umlaut */
	rp->secondary = 0x03;
	break;
    case 0xAE:	/* AE */
	rp->secondary = 0x04;
	break;
    case 0xCB:	/* A~ */
	rp->secondary = 0x05;
	break;
    case 0x81:	/* A-jot */
	rp->secondary = 0x06;
	break;

    case 'a':
	if (len > 1 && cp[1] == 'e') {	/* a e */
	    retval = 2;
	    rp->secondary = 0x84;
	}
	break;
    case 0x8A:	/* a-umlaut */
	rp->secondary = 0x85;
	break;
    case 0xBE:	/* ae */
	rp->secondary = 0x86;
	break;
    case 0x8B:	/* a~ */
	rp->secondary = 0x87;
	break;
    case 0x8C:	/* a-jot */
	rp->secondary = 0x88;
	break;

    case 'O':
	if (len > 1 && cp[1] == 'E') {	/* O E */
	    retval = 2;
	    rp->secondary = 0x01;
	}
	break;
    case 0x85:	/* O-umlaut */
	rp->secondary = 0x02;
	break;
    case 0xCE:	/* OE */
	rp->secondary = 0x03;
	break;
    case 0xCD:	/* O~ */
	rp->secondary = 0x04;
	break;
    case 0xAF:	/* O-slash */
	rp->secondary = 0x05;
	break;

    case 'o':
	if (len > 1 && cp[1] == 'e') {	/* o e */
	    retval = 2;
	    rp->secondary = 0x84;
	}
	break;
    case 0x9A:	/* o-umlaut */
	rp->secondary = 0x85;
	break;
    case 0xCF:	/* oe */
	rp->secondary = 0x86;
	break;
    case 0x9B:	/* o~ */
	rp->secondary = 0x87;
	break;
    case 0xBF:	/* o-slash */
	rp->secondary = 0x88;
	break;

    case 'U':
	if (len > 1 && cp[1] == 'E') {	/* U E */
	    retval = 2;
	    rp->secondary = 0x01;
	}
	break;
    case 0x86:	/* U-umlaut */
	rp->secondary = 0x02;
	break;

    case 'u':
	if (len > 1 && cp[1] == 'e') {	/* u e */
	    retval = 2;
	    rp->secondary = 0x84;
	}
	break;
    case 0x9F:	/* u-umlaut */
	rp->secondary = 0x85;
	break;

    case 's':
	if (len > 1 && cp[1] == 's') {	/* s s */
	    retval = 2;
	    rp->primary = 0xA7;
	    rp->secondary = 0x00;
	}
	break;
    case 0xA7:	/* Beta? */
	rp->secondary = 0x01;
	break;
    }

    return retval;
}

A3(PRIVATE, INTEGER, britainlocalization, unsigned char *, cp, INTEGER, len,
								sorttype *, rp)
{
    (void) defaultorder(cp, len, rp);
    if (rp->primary >= 0x23 && rp->primary <= 0xA3) {	/* shift around so */
	if (rp->primary == 0xA3)     /* real pound sign comes before sharp */
	    rp->primary = 0x23;
	else
	    rp->primary++;
    }
    return 1;
}

A3(PRIVATE, INTEGER, defaultlocalization, unsigned char *, cp, INTEGER, len,
								sorttype *, rp)
{
    return 0;	/* never consume anything */
}

/*
 * NOTE: currently we can only use natively compiled localization routines
 *	 because we don't envoke syn68k below.  However, since we don't know
 *	 the exact format of localization routines, we don't care, for now.
 */


A5(PRIVATE, INTEGER, iuhelper, Ptr, ptr1, Ptr, ptr2, INTEGER, len1,
					    INTEGER, len2, BOOLEAN, ignoresec)
{
    Intl0Hndl h;
    locptype locp;
    sorttype sort1, sort2;
    INTEGER ret1, ret2;
    int usesec = 0;

#if !defined (LETGCCWAIL)
    locp = 0;
#endif /* LETGCCWAIL */

    if ((h = (Intl0Hndl) IUGetIntl(0)) && (*h).p) {
	switch ((Hx(h, intl0Vers) >> 8) & 0xFF) {
	case verBritain:
	    locp = (locptype) britainlocalization;
	    break;
	case verGermany:
	    locp = (locptype) germanylocalization;
	    break;
	default:
	    locp = (locptype) defaultlocalization;
	    break;
	}
    } else {
	gui_assert(0);
    }
    while (len1 > 0 && len2 > 0) {
	if (!(ret1 = (*locp)(ptr1, len1, &sort1)))
	    ret1 = defaultorder((unsigned char *) ptr1, len1, &sort1);
	if (!(ret2 = (*locp)(ptr2, len2, &sort2)))
	    ret2 = defaultorder((unsigned char *) ptr2, len2, &sort2);
        if (sort1.primary == sort2.primary) {
            if (!ignoresec && !usesec && sort1.secondary != sort2.secondary) {
                if (sort1.secondary < sort2.secondary)
                    usesec = -1;
                else
                    usesec = 1;
            }
            len1 -= ret1;
            ptr1 += ret1;
            len2 -= ret2;
            ptr2 += ret2;
        } else if (sort1.primary < sort2.primary) {
            len1 = 0;
            len2 = 1;
        } else {
            len1 = 1;
            len2 = 0;
        }
    }
    if (len1 == len2)
        return ignoresec ? 0 : usesec;
    else
        return (len1 < len2 ? -1 : 1);
}

P4(PUBLIC pascal trap, INTEGER, IUMagString, Ptr, ptr1,	/* IMI-506 */
				     Ptr, ptr2, INTEGER, len1, INTEGER, len2)
{
    return iuhelper(ptr1, ptr2, len1, len2, FALSE);
}

A2(PUBLIC, INTEGER, IUCompString, StringPtr, str1,
					StringPtr, str2)	/* IMI-506 */
{
    return IUMagString((Ptr) (str1+1), (Ptr) (str2+1), str1[0], str2[0]);
}

P4(PUBLIC pascal trap, INTEGER, IUMagIDString, Ptr, ptr1,	/* IMI-507 */
				     Ptr, ptr2, INTEGER, len1, INTEGER, len2)
{
    return iuhelper(ptr1, ptr2, len1, len2, TRUE);
}

A2(PUBLIC, INTEGER, IUEqualString, StringPtr, str1,		/* IMI-506 */
						    StringPtr, str2)
{
    return IUMagIDString((Ptr) (str1+1), (Ptr) (str2+1), str1[0], str2[0]);
}

P4(PUBLIC pascal trap, /* INTEGER */ void, IUMystery, Ptr, arg1, Ptr, arg2,
						  INTEGER, arg3, INTEGER, arg4)
{
    /* Microsoft Excel calls Pack6 with a selector of 18.
       This is being done on the Mac+.
       Neither, Inside Macintosh, nor any of the .h and .a files help.
       This is here just to pop the arguments in a desperate hope that
       nothing important is being done anyway.
       */
}

/* NOTE: none of the below are done yet */
/* #warning A bunch of IU routines are not implemented yet */

P4(PUBLIC pascal trap, void, IULDateString, LongDateTime *, datetimep,
			  DateForm, longflag, Str255, result, Handle, intlhand)
{
  warning_unimplemented (NULL_STRING);
  ROMlib_hook(iu_unimplementednumber);
}

P4(PUBLIC pascal trap, void, IULTimeString, LongDateTime *, datetimep,
			BOOLEAN, wantseconds, Str255, result, Handle, intlhand)
{
  warning_unimplemented (NULL_STRING);
  ROMlib_hook(iu_unimplementednumber);
}

P0(PUBLIC pascal trap, void, IUClearCache)
{
}

P5(PUBLIC pascal trap, INTEGER, IUMagPString, Ptr, ptra, Ptr, ptrb,
				INTEGER, lena, INTEGER, lenb, Handle, itl2hand)
{
  warning_unimplemented (NULL_STRING);
  ROMlib_hook(iu_unimplementednumber);
  return 0;
}

P5(PUBLIC pascal trap, INTEGER, IUMagIDPString, Ptr, ptra, Ptr, ptrb,
				INTEGER, lena, INTEGER, lenb, Handle, itl2hand)
{
  warning_unimplemented (NULL_STRING);
  ROMlib_hook(iu_unimplementednumber);
  return 0;
}

P2(PUBLIC pascal trap, INTEGER, IUScriptOrder, ScriptCode, script1,
						           ScriptCode, script2)
{
  warning_unimplemented (NULL_STRING);
  ROMlib_hook(iu_unimplementednumber);
  return 0;
}

P2(PUBLIC pascal trap, INTEGER, IULangOrder, LangCode, l1, LangCode, l2)
{
  warning_unimplemented (NULL_STRING);
  ROMlib_hook(iu_unimplementednumber);
  return 0;
}

P8(PUBLIC pascal trap, INTEGER, IUTextOrder, Ptr, ptra, Ptr, ptrb,
			INTEGER, lena, INTEGER, lenb, ScriptCode, scripta,
		         ScriptCode, bscript, LangCode, langa, LangCode, langb)
{
  warning_unimplemented (NULL_STRING);
  ROMlib_hook(iu_unimplementednumber);
  return 0;
}

P5(PUBLIC pascal trap, void, IUGetItlTable, ScriptCode, script,
	    INTEGER, tablecode, Handle *, itlhandlep, LONGINT *, offsetp,
							    LONGINT *, lengthp)
{
  warning_unimplemented (NULL_STRING);
  ROMlib_hook(iu_unimplementednumber);
}
