/* Copyright 1991 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ScriptMgr.h (DO NOT DELETE THIS LINE) */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_script[] =
	    "$Id: script.c 88 2005-05-25 03:59:37Z ctm $";
#endif

#include "rsys/common.h"
#include "QuickDraw.h"
#include "IntlUtil.h"
#include "ScriptMgr.h"
#include "MemoryMgr.h"
#include "ToolboxUtil.h"
#include "OSUtil.h"

#include "rsys/hook.h"
#include "rsys/quick.h"
#include "rsys/cquick.h"
#include "rsys/osutil.h"
#include "rsys/osevent.h"
#include "rsys/print.h"
#include "rsys/floatconv.h"
#include "rsys/string.h"
#include "rsys/mman.h"

#include <ctype.h>

using namespace Executor;

/*
 * NOTE: these are stubs to help me make FileMaker Pro go.
 */

P1(PUBLIC pascal trap, LONGINT, GetEnvirons, INTEGER, verb)
{
  LONGINT retval;

  switch (verb)
    {
    case smEnabled:
      /* powerpoint seems to require that at least one script is
         present */
      warning_unimplemented ("reporting script manager is enabled");
      /* we currently only have a single script */
      retval = 1;
      break;

    case smKCHRCache:
      retval = US_TO_SYN68K (ROMlib_kchr_ptr ());
      break;

    default:
      warning_unexpected ("unhandled selector `%d'", verb);
      retval = 0;
    }
  return retval;
}

P2(PUBLIC pascal trap, OSErr, SetEnvirons, INTEGER, verb, LONGINT, param)
{
#if defined (BINCOMPAT)
    ROMlib_hook(script_notsupported);
#endif /* BINCOMPAT */
    return smVerbNotFound;
}

P2(PUBLIC pascal trap, LONGINT, GetScript, INTEGER, script, INTEGER, verb)
{
  warning_unimplemented (NULL_STRING);
  return 0;
}

P3(PUBLIC pascal trap, OSErr, SetScript, INTEGER, script, INTEGER, verb,
							        LONGINT, param)
{
  warning_unimplemented (NULL_STRING);
  return smVerbNotFound;
}

P1(PUBLIC pascal trap, INTEGER, Font2Script, INTEGER, fontnum)
{
  warning_unimplemented (NULL_STRING);
  return 0;
}

/*
 * butchered Transliterate provided for Excel 3.0
 */

namespace Executor {
  PRIVATE char upper(char);
  PRIVATE char lower(char);
}

#define LOWERTOUPPEROFFSET 'A' - 'a'
A1(PRIVATE, char, upper, char, ch)
{
    if (ch >= 'a' && ch <= 'z')
#if 1
        return ch + LOWERTOUPPEROFFSET;
#else /* 0 */
	return ch & ~0x20;
#endif /* 0 */
    else
	return ch;
}

#define UPPERTOLOWEROFFSET ('a' - 'A')
A1(PRIVATE, char, lower, char, ch)
{
    if (ch >= 'A' && ch <= 'Z')
#if 1
        return ch + UPPERTOLOWEROFFSET;
#else /* 0 */
	return ch | 0x20;
#endif /* 0 */
    else
	return ch;
}

P4(PUBLIC pascal trap, INTEGER, Transliterate, Handle, srch, Handle, dsth,
					     INTEGER, target, LONGINT, srcmask)
{
    char *sp, *dp, *ep;

    sp = (char *) STARH(srch);
    dp = (char *) STARH(dsth);
    ep = sp + GetHandleSize(srch);
    if (target & smTransLower) {
	if (target & smTransUpper)
/*-->*/     return -1;
        while (sp < ep)
	    *dp++ = lower(*sp++);
    } else if (target & smTransUpper) {
        while (sp < ep)
	    *dp++ = upper(*sp++);
    } else {
        while (sp < ep)
	    *dp++ = *sp++;
    }
    return 0;
}

/*
 * NOTE: These are all recent additions, made just before 1.2.2 was frozen.
 *	 They haven't been tested much, if at all.  In addition, much of
 *	 the code below just tries to return something "reasonable", although
 *	 not necessarily correct!
 */

P0(PUBLIC pascal trap, INTEGER, FontScript)
{
  warning_unimplemented (NULL_STRING);
  return smRoman;
}

P0(PUBLIC pascal trap, INTEGER, IntlScript)
{
  warning_unimplemented (NULL_STRING);
  return smRoman;
}

P1(PUBLIC pascal trap, void, KeyScript, INTEGER, scriptcode)
{
  warning_unimplemented (NULL_STRING);
}

P2(PUBLIC pascal trap, INTEGER, CharType, Ptr, textbufp, INTEGER, offset)
{
    INTEGER retval;
    unsigned char c;
#if defined (BINCOMPAT)
    ROMlib_hook(script_notsupported);
#endif /* BINCOMPAT */

    retval = 0;
    c = textbufp[offset];
    if (!isalpha(c)) {
	retval |= smCharPunct;
	if (isspace(c))
	    retval |= smPunctBlank;
	else if (isdigit(c))
	    retval |= smPunctNumber;
	else if (ispunct(c))
	    retval |= smPunctSymbol;
    } else {
	retval |= smCharAscii;
	if (islower(c))
	    retval |= smCharLower;
	else
	    retval |= smCharUpper;
    }
    retval |= smCharLeft;
    retval |= smChar1byte;

    return retval;
}

P4 (PUBLIC pascal trap, void, MeasureJust, Ptr, textbufp, int16, length,
    int16, slop, Ptr, charlocs)
{
  if (slop)
    warning_unimplemented ("slop = %d", slop);
  MeasureText (length, textbufp, charlocs);
}

P7 (PUBLIC pascal trap, void, NMeasureJust, Ptr, text, int32, length,
    Fixed, slop, Ptr, charLocs, JustStyleCode, run_pos,
    Point, numer, Point, denom)
{
  GUEST<Point> numerx, denomx;
  
  warning_unimplemented ("slop = %d, run_pos = %d", slop, run_pos);
  
  numerx.v = CW (numer.v);
  numerx.h = CW (numer.h);
  denomx.v = CW (denom.v);
  denomx.h = CW (denom.h);
  
  xStdTxMeas (length, (uint8 *) text, &numerx, &denomx,
	      NULL, (GUEST<int16> *) charLocs);
}

P1 (PUBLIC pascal trap, Boolean, ParseTable,
    CharByteTable, table)
{
  memset (table, 0, sizeof (CharByteTable));
  return TRUE;
}

P2 (PUBLIC pascal trap, Boolean, FillParseTable,
    CharByteTable, table, ScriptCode, script)
{
  /* ### should we even look at `script' */
  memset (table, 0, sizeof (CharByteTable));
  return TRUE;
}

P3 (PUBLIC pascal trap, INTEGER, CharacterByteType, Ptr, textBuf,
    INTEGER, textOffset, ScriptCode, script)
{
  warning_unimplemented (NULL_STRING);
  /* Single-byte character */
  return 0;
}

P3 (PUBLIC pascal trap, INTEGER, CharacterType, Ptr, textbufp,
    INTEGER, offset, ScriptCode, script)
{
  warning_unimplemented (NULL_STRING);
  return CharType (textbufp, offset);
}

P5 (PUBLIC pascal trap, INTEGER, TransliterateText, Handle, srch, Handle, dsth,
    INTEGER, target, LONGINT, srcmask, ScriptCode, script)
{
  warning_unimplemented (NULL_STRING);
  return Transliterate (srch, dsth, target, srcmask);
}

P5(PUBLIC pascal trap, INTEGER, Pixel2Char, Ptr, textbufp, INTEGER, len,
			INTEGER, slop, INTEGER, pixwidth, BOOLEAN *, leftsidep)
{
  Point num, den;
  INTEGER retval;

  warning_unimplemented ("poorly implemented");
  
  num.h = 1;
  num.v = 1;
  den.h = 1;
  den.v = 1;
  retval = C_PixelToChar (textbufp, len, slop << 16, pixwidth << 16, leftsidep,
			  0, 0, num, den);
  return retval;
}

P5(PUBLIC pascal trap, INTEGER, Char2Pixel, Ptr, textbufp, INTEGER, len,
			       INTEGER, slop, INTEGER, offset, SignedByte, dir)
{
    INTEGER retval;
    Point num, den;

    warning_unimplemented ("poorly implemented");

    num.h = 1;
    num.v = 1;
    den.h = 1;
    den.v = 1;
    
    retval = C_CharToPixel (textbufp, len, slop << 16, offset, dir, 0,
			    num, den);
    return retval;

}

P6(PUBLIC pascal trap, void, FindWord, Ptr, textbufp, INTEGER, length,
	   INTEGER, offset, BOOLEAN, leftside, Ptr, breaks, GUEST<INTEGER> *, offsets)
{
  INTEGER start, stop;
  boolean_t chasing_spaces_p;
#if defined (BINCOMPAT)
    ROMlib_hook(script_notsupported);
#endif /* BINCOMPAT */

    if (!leftside)
      --offset;
    if (offset < 0)
      offset = 0;

    chasing_spaces_p = isspace (textbufp[offset]);
    for (start = offset;
	 start > 0 && !isspace (textbufp[start-1]) == !chasing_spaces_p;
	 --start)
      ;

    for (stop = offset;
	 stop < length && !isspace (textbufp[stop]) == !chasing_spaces_p;
	 ++stop)
      ;

    offsets[0] = CW(start);
    offsets[1] = CW(stop);
    offsets[2] = CWC(0); /* Testing on Brute shows we should zero this memory */
    offsets[3] = CWC(0);
    offsets[4] = CWC(0);
    offsets[5] = CWC(0);
    warning_unimplemented ("poorly implemented");
}

P4(PUBLIC pascal trap, void, HiliteText, Ptr, textbufp, INTEGER, firstoffset,
				     INTEGER, secondoffset, GUEST<INTEGER> *, offsets)
{
#if defined (BINCOMPAT)
    ROMlib_hook(script_notsupported);
#endif /* BINCOMPAT */
    offsets[0] = CW(firstoffset);
    offsets[1] = CW(secondoffset);
    offsets[2] = CWC(0);
    offsets[3] = CWC(0);
    offsets[4] = CWC(0);
    offsets[5] = CWC(0);
}

PRIVATE int16
count_spaces (Ptr textbufp, int16 length)
{
  int16 retval;

  retval = 0;
  while (length-- > 0)
    if (*textbufp++ == ' ')
      ++retval;

  return retval;
}

P3 (PUBLIC pascal trap, void, DrawJust, Ptr, textbufp,
    int16, length, int16, slop)
{
  GUEST<Fixed> save_sp_extra_x;
  int n_spaces;

  save_sp_extra_x = PORT_SP_EXTRA_X (thePort);
  n_spaces = count_spaces (textbufp, length);
  if (n_spaces)
    {
      Fixed extra;

      extra = CL (save_sp_extra_x) + FixRatio (slop, n_spaces);
      PORT_SP_EXTRA_X (thePort) = CL (extra);
    }
  DrawText (textbufp, 0, length);
  PORT_SP_EXTRA_X (thePort) = save_sp_extra_x;
}

PRIVATE int
snag_date_part (Ptr text, int *offsetp, LONGINT len)
{
  int retval;

  retval = 0;

  while (*offsetp < len && text[*offsetp] != '/')
    {
      retval = retval * 10 + text[*offsetp] - '0';
      ++*offsetp;
    }

  if (*offsetp < len && text[*offsetp] == '/')
    ++*offsetp;

  return retval;
}

enum { longDateFound = 1, dateTimeNotFound = 0x8400 };

P5(PUBLIC pascal trap, String2DateStatus, String2Time, Ptr, textp,
       LONGINT, len, Ptr, cachep, GUEST<LONGINT> *, lenusedp, GUEST<Ptr> *, datetimep)
{
  warning_unimplemented (NULL_STRING);
  *lenusedp = CLC (0);
  return (String2DateStatus) dateTimeNotFound;
}

PRIVATE void
this_date_rec(DateTimeRec *p)
{
  GUEST<ULONGINT> now;

  GetDateTime (&now);
  Secs2Date (CL (now), p);
}

PRIVATE int
this_century (void)
{
  DateTimeRec d;
  int retval;

  this_date_rec (&d);
  retval = CW (d.year) / 100 * 100;
  return retval;
}

PRIVATE int
this_millennium (void)
{
  int retval;

  retval = this_century () / 1000 * 1000;
  return retval;
}

P5 (PUBLIC pascal trap, String2DateStatus, String2Date,
    Ptr, text, int32, length, DateCachePtr, cache,
    GUEST<int32> *, length_used_ret, LongDatePtr, date_time)
{
  String2DateStatus retval;

  if (length <= 10 && (text[1] == '/' || text[2] == '/'))
    {
      int offset, month, day, year;
      int offset_save, year_length;
      
      offset = 0;
      month = snag_date_part (text, &offset, length);
      day = snag_date_part (text, &offset, length);

      offset_save = offset;
      year = snag_date_part (text, &offset, length);
      year_length = offset - offset_save;

      if (year_length == 3)
	year += this_millennium ();
      else if (year_length < 3)
	year += this_century ();

      *length_used_ret = CL (offset);

      /* not clear what we should do with other fields, some should probably
	 be zeroed */

      date_time->year = CW (year);
      date_time->month = CW (month);
      date_time->day = CW (day);
      retval = longDateFound;
    }
  else
    {
      *length_used_ret = CLC (0);
      warning_unexpected (NULL_STRING);
      retval = (String2DateStatus) dateTimeNotFound;
    }
  warning_unimplemented (NULL_STRING);
  return retval;
}

P7 (PUBLIC pascal trap, StyledLineBreakCode, StyledLineBreak,
    Ptr, textp, int32, length,
    int32, text_start, int32, text_end, int32, flags,
    GUEST<Fixed> *, text_width_fp, GUEST<int32> *, text_offset)
{
  char *text = (char *) textp;
  /* the index into `text' that began the last word, which is where we
     want to break if the current word extends past the end of the
     current line */
  int last_word_break = -1;
  char current_char;
  int current_index;
  int text_width;
  int width = 0;
  
  /* ### are we losing information here? */
  text_width = Fix2Long (CL (*text_width_fp));
  
  for (current_index = text_start, current_char = text[current_index];
       current_index < text_end;
       current_index ++, current_char = text[current_index])
    {
      /* ### do we do this? */
      if (current_char == '\r')
	{
	  *text_offset = CL (current_index + 1);
	  return smBreakWord;
	}
	  
      if (current_index > text_start
	  && current_char != ' '
	  && text[current_index - 1] == ' ')
	{
	  last_word_break = current_index - 1;
	}
      
      width += CharWidth (current_char);
      
      if (width > text_width)
	{
	  /* we got our char */
	  if (last_word_break == -1)
	    {
	      /* d'oh, we are on the first word */
	      if (*text_offset)
		{
		  /* beginning of the line, break here */
		  *text_offset = CL (current_index - 1);
		  return smBreakChar;
		}
	      *text_offset = CL (current_index - 1);
	      return smBreakWord;
	    }
	  else
	    {
	      *text_offset = CL (last_word_break);
	      return smBreakWord;
	    }
	}
    }
  /* if we got here, that means the run did not extend past the end of
     the current line */
  *text_width_fp = CL (Long2Fix (text_width - width));
  *text_offset = CL (current_index);
  return smBreakOverflow;
}

P3 (PUBLIC pascal trap, INTEGER, ReplaceText,
    Handle, base_text, Handle, subst_text,
    Str15, key)
{
  INTEGER retval;

  warning_unimplemented ("not tested much");

  retval = 0;
  LOCK_HANDLE_EXCURSION_1
    (subst_text, 
     {
       Ptr p;
       INTEGER len;
       LONGINT offset;
       LONGINT l;

       p = (Ptr) STARH (subst_text);
       len = GetHandleSize (subst_text);
       offset = 0;
       while (retval >= 0 &&
	      (l = Munger (base_text,
			   offset, (Ptr) key+1, key[0], NULL, 1)) >= 0)
	 {
	   offset = Munger (base_text,
			    l, (Ptr) key+1, key[0], p, len);
	   if (offset < 0)
	     retval = offset;
	   else
	     ++retval;
	 }
     });

  return retval;
}

/* FormatStr2X is now StringToExtended */
P4 (PUBLIC pascal trap, FormatStatus, StringToExtended,  /* TTS TODO */
   Str255, string, 
   NumFormatStringRec *, formatp,
   NumberParts *, partsp,
   Extended80 *, xp)
{
  FormatStatus retval;
  double d;
  char buf[256];

  memcpy (buf, string+1, string[0]);
  buf[string[0]] = 0;
  sscanf (buf, "%lg", &d);
  ieee_to_x80 ((ieee_t) d, xp);
  warning_unimplemented (NULL_STRING);
  retval = noErr;
  return retval;
}

P4 (PUBLIC pascal trap, FormatStatus, ExtendedToString,  /* TTS TODO */
    Extended80 *, xp,
    NumFormatStringRec *, formatp,
    NumberParts *, partsp,
    Str255, string)
{
  ieee_t val;
  FormatStatus retval;
  char buf[256];

  val = x80_to_ieee (xp);
#if !defined (CYGWIN32)
  sprintf (buf, "%Lg", val);
#else
#warning may lose bits of precision here
  sprintf (buf, "%g", (double) val);
#endif
  str255_from_c_string (string ,buf);
  warning_unimplemented (NULL_STRING);
  retval = noErr;
  return retval;
}

P3 (PUBLIC pascal trap, FormatStatus, StringToFormatRec, /* TTS TODO */
    Str255, in_string,
    NumberParts *, partsp,
    NumFormatStringRec *, out_string)
{
  FormatStatus retval;

  warning_unimplemented (NULL_STRING);
  retval = 0;
  return retval;
}

P5 (PUBLIC pascal trap, ToggleResults, ToggleDate,  /* TTS TODO */
    LongDateTime *, lsecsp,
    LongDateField, field,
    DateDelta, delta,
    INTEGER, ch,
    TogglePB *, paramsp)
{
  ToggleResults retval;

  warning_unimplemented (NULL_STRING);
  retval = 0;
  return retval;
}

P3 (PUBLIC pascal trap, INTEGER, TruncString,  /* TTS TODO */
    INTEGER, width, Str255, string,
    TruncCode, code)
{
  warning_unimplemented (NULL_STRING);
  
  /* ### claim we didn't have to truncate the string */
  return Truncated;
}

P2 (PUBLIC pascal trap, LONGINT, VisibleLength, Ptr, textp, LONGINT, len)
{
  warning_unimplemented ("poorly implemented -- what about other white space");

  warning_trace_info ("%.*s", (int) len, textp);
  while (len > 0 && textp[len-1] == ' ')
    --len;
  return len;
}

P2 (PUBLIC pascal trap, void, LongDate2Secs, LongDateRec *, ldatep,
    GUEST<ULONGINT> *, secs_outp)
{
  long long secs;
  LONGINT high, low;
  INTEGER hour;

  hour = CW (ldatep->hour);
  if (ldatep->pm && hour < 12)
    hour += 12;
    
  secs = ROMlib_long_long_secs (CW (ldatep->year), CW (ldatep->month),
				CW (ldatep->day), hour,
				CW (ldatep->minute), CW (ldatep->second));
  high = secs >> 32;
  low = secs;
  secs_outp[0] = CL (high);
  secs_outp[1] = CL (low);
}

P2 (PUBLIC pascal trap, void, LongSecs2Date, GUEST<ULONGINT> *, secs_inp,
    LongDateRec *, ldatep)

{
  long long secs;
  INTEGER pm;
  
  secs = ((long long) CL (secs_inp[0]) << 32) | CL (secs_inp[1]);
  date_to_swapped_fields (secs, &ldatep->year, &ldatep->month, &ldatep->day,
			  &ldatep->hour, &ldatep->minute, &ldatep->second,
			  &ldatep->dayOfWeek, &ldatep->dayOfYear,
			  &ldatep->weekOfYear);

  pm = (CW (ldatep->hour) > 12) ? 1 : 0;
  ldatep->pm = CW (pm);
}

/*
 * NOTE: At least some of these need to be implemented if we want
 *	 Resolve to work
 */


#if 0
ParseTable

PortionText

FindScriptRun

IsSpecialFont

RawPrinterValues

NPixel2Char

NChar2Pixel

NDrawJust

NPortionText

ReplaceText

TruncText

TrunString

NFindWord

ValidDate

FormatStr2X

FormatX2Str

Format2Str

Str2Format

ToggleDate

LongSecs2Date

LongDate2Secs

IntlTokenize

GetFormatOrder
#endif

A0(PUBLIC, INTEGER, GetAppFont)
{
    return CW (ApFontID);
}

#if 0
/*
 * NOTE: the following is stuff I typed in before noticing that these are
 *       provided by glue.  Hence they will be necessary if we ever support
 *	 ROMlib again but are not needed for Executor.
 */

A0(PUBLIC, INTEGER, GetDefFontSize)
{
  return 0;
}

A0(PUBLIC, INTEGER, GetSysFont)
{
    return 0;
}

A0(PUBLIC, INTEGER, GetMBarHeight)
{
    return 0;
}

A0(PUBLIC, INTEGER, GetSysJust)
{
    return 0;
}

A1(PUBLIC, void, SetSysJust, INTEGER, just)
{
}
#endif

P1 (PUBLIC pascal trap, OSErr, InitDateCache, DateCachePtr, cache)  /* TTS TODO */
{
  warning_unimplemented (NULL_STRING);
  return noErr;
}

P2(PUBLIC pascal trap, INTEGER, CharByte, Ptr, textBuf, INTEGER, textOffset)
{
  warning_unimplemented (NULL_STRING);
  /* Single-byte character */
  return 0;
}

P5(PUBLIC pascal trap, Fixed, PortionLine,
   Ptr, textPtr,
   LONGINT, textLen,
   JustStyleCode, styleRunPosition,
   Point, numer,
   Point, denom)
{
  Fixed retval;

  warning_unimplemented (NULL_STRING);
  retval = 0x10000;
  return retval;
}

P6(PUBLIC pascal trap, void, DrawJustified,
   Ptr, textPtr,
   LONGINT, textLength,
   Fixed, slop,
   JustStyleCode, styleRunPosition,
   Point, numer,
   Point, denom)
{
  GUEST<Point> swapped_numer;
  GUEST<Point> swapped_denom;
  
  warning_unimplemented ("poorly implemented");
  swapped_numer.h = CW (numer.h);
  swapped_numer.v = CW (numer.v);
  swapped_denom.h = CW (denom.h);
  swapped_denom.v = CW (denom.v);
  text_helper (textLength, textPtr, &swapped_numer, &swapped_denom, 0, 0,
	       text_helper_draw);
}

P3(PUBLIC pascal trap, ScriptRunStatus, FindScriptRun,
   Ptr, textPtr,
   LONGINT, textLen,
   GUEST<LONGINT> *, lenUsedp)
{
  warning_unimplemented (NULL_STRING);
  *lenUsedp = CLC (1);
  return 0;
}

P9(PUBLIC pascal trap, INTEGER, PixelToChar,
   Ptr, textBuf,
   LONGINT, textLen,
   Fixed, slop,
   Fixed, pixelWidth,
   BOOLEAN *, leadingEdgep,
   GUEST<Fixed> *, widthRemainingp,
   JustStyleCode, styleRunPosition,
   Point, numer,
   Point, denom)
{
  GUEST<INTEGER> *locs;
  INTEGER retval, i;
  GUEST<Point> swapped_numer;
  GUEST<Point> swapped_denom;
  INTEGER int_pix_width;

  warning_unimplemented ("poorly implemented");

  locs = (GUEST<INTEGER>*)alloca( sizeof(INTEGER) * (textLen + 1));

  swapped_numer.h = CW (numer.h);
  swapped_numer.v = CW (numer.v);
  swapped_denom.h = CW (denom.h);
  swapped_denom.v = CW (denom.v);

  int_pix_width = pixelWidth >> 16;

  text_helper (textLen, textBuf, &swapped_numer, &swapped_denom, 0, locs,
	       text_helper_measure);

  /* NOTE: we could distribute slop here, or we could adjust text_helper
     to account for slop (probably better in the long run).  Right now,
     we do neither.  Ick. */

  if (int_pix_width >= CW(locs[textLen]))
    {
      retval = textLen;
      *leadingEdgep = FALSE;
      *widthRemainingp = CL( pixelWidth - (CW(locs[textLen]) << 16) );
    }
  else
    {
      *widthRemainingp = CLC (-1);
      for (i = 0; int_pix_width > CW(locs[i]); ++i)
	;
      if ((i > 0) &&
	  ((int_pix_width - CW(locs[i-1])) > (CW(locs[i]) - int_pix_width)))
	{
	  retval = i - 1;
	  *leadingEdgep = FALSE;
	}
      else
	{
	  retval = i;
	  *leadingEdgep = TRUE;
	}
    }
  return retval;
}

P8(PUBLIC pascal trap, INTEGER, CharToPixel,
   Ptr, textBuf,
   LONGINT, textLen,
   Fixed, slop,
   LONGINT, offset,
   INTEGER, direction,
   JustStyleCode, styleRunPosition,
   Point, numer,
   Point, denom)
{
  INTEGER retval;
  GUEST<Point> swapped_numer, swapped_denom;

  warning_unimplemented ("poorly implemented");

  swapped_numer.h = CW (numer.h);
  swapped_numer.v = CW (numer.v);
  swapped_denom.h = CW (denom.h);
  swapped_denom.v = CW (denom.v);
  retval = text_helper (offset, textBuf, &swapped_numer, &swapped_denom,
			0, 0, text_helper_measure);
  retval += (slop / textLen) >> 16;
  return retval;
}

P3(PUBLIC pascal trap, void, LowercaseText,
   Ptr, textp,
   INTEGER, len,
   ScriptCode, script)
{
  warning_unimplemented ("poorly implemented");
}

P3(PUBLIC pascal trap, void, UppercaseText,
   Ptr, textp,
   INTEGER, len,
   ScriptCode, script)
{
  ROMlib_UprString ((StringPtr) textp, FALSE, len);
}

P3(PUBLIC pascal trap, void, StripDiacritics,
   Ptr, textp,
   INTEGER, len,
   ScriptCode, script)
{
  warning_unimplemented ("poorly implemented");
}

P3(PUBLIC pascal trap, void, UppercaseStripDiacritics,
   Ptr, textp,
   INTEGER, len,
   ScriptCode, script)
{
  ROMlib_UprString ((StringPtr) textp, TRUE, len);
}
