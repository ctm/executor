#if !defined (_SCRIPTMGR_H_)
#define _SCRIPTMGR_H_

/*
 * Copyright 1991 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: ScriptMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "SANE.h"
#include "IntlUtil.h"

typedef INTEGER String2DateStatus;

#define smRoman		0

#define smCharPunct	0
#define smCharAscii	1
#define smCharEuro	7

#define smPunctNormal	0x0000
#define smPunctNumber	0x0100
#define smPunctSymbol	0x0200
#define smPunctBlank	0x0300

#define smCharLeft	0x0000
#define smCharRight	0x2000

#define smCharLower	0x0000
#define smCharUpper	0x4000

#define smChar1byte	0x0000
#define smChar2byte	0x8000

#define smTransAscii	0
#define smTransNative	1
#define smTransLower	16384
#define smTransUpper	32768
#define smMaskAscii	1
#define smMaskNative	2
#define smMaskAll	-1

#define smScriptVersion	0
#define smScriptMunged	2
#define smScriptEnabled	4
#define smScriptRight	6
#define smScriptJust	8
#define smScriptRedraw	10
#define smScriptSysFond	12
#define smScriptAppFond	14
#define smScriptNumber	16
#define smScriptDate	18
#define smScriptSort	20
#define smScriptRsvd1	22
#define smScriptRsvd2	24
#define smScriptRsvd3	26
#define smScriptRsvd4	28
#define smScriptRsvd5	30
#define smScriptKeys	32
#define smScriptIcon	34
#define smScriptPrint	36
#define smScriptTrap	38
#define smScriptCreator	40
#define smScriptFile	42
#define smScriptName	44

#define smVersion	0
#define smMunged	2
#define smEnabled	4
#define smBiDirect	6
#define smFontForce	8
#define smIntlForce	10
#define smForced	12
#define smDefault	14
#define smPrint		16
#define smSysScript	18
#define smAppScript	20
#define smKeyScript	22
#define smSysRef	24
#define smKeyCache	26
#define smKeySwap	28

enum { smKCHRCache = 38 };

#define smVerbNotFound	(-1)

#define smBreakWord	0
#define smBreakChar	1
#define smBreakOverflow	2

/* TruncText return codes */
#define NotTruncated	0
#define Truncated	1
#define TruncErr	(-1)

typedef uint8 StyledLineBreakCode;

typedef struct DateCacheRec
{
  int16 hidden[256] 	PACKED;
} DateCacheRec, *DateCachePtr;

typedef struct LongDateRec
{
  int16 era	PACKED;
  int16 year	PACKED;
  int16 month	PACKED;
  int16 day	PACKED;
  int16 hour	PACKED;
  int16 minute	PACKED;
  int16 second	PACKED;
  int16 dayOfWeek	PACKED;
  int16 dayOfYear	PACKED;
  int16 weekOfYear	PACKED;
  int16 pm	PACKED;
  int16 res1	PACKED;
  int16 res2	PACKED;
  int16 res3	PACKED;
} LongDateRec, *LongDatePtr;

typedef INTEGER TruncCode;
typedef int16 JustStyleCode;

typedef int8 CharByteTable[256];

typedef int16 ScriptRunStatus; /* Not sure this is correct, since in IM
				  ScriptRunStatus is a record with two
				  Signed Bytes */

typedef INTEGER FormatStatus;

typedef struct
{
  Byte fLength LPACKED;
  Byte fVersion LPACKED;
  SignedByte data[253] LPACKED;
}
NumFormatStringRec;

typedef union
{
  CHAR a[2];
  INTEGER b;
}
WideChar;

typedef struct
{
  INTEGER size PACKED;
  WideChar data[10] PACKED;
}
WideCharArr;

typedef struct
{
  INTEGER version PACKED;
  WideChar data[31] PACKED;
  WideCharArr pePlus LPACKED;
  WideCharArr peMinus LPACKED;
  WideCharArr peMinusPlus LPACKED;
  WideCharArr altNumTable LPACKED;
  CHAR reserved[20] PACKED;
}
NumberParts;

typedef extended80 Extended80;

typedef short ToggleResults;

typedef unsigned char LongDateField;

typedef char DateDelta;

typedef struct
{
  long togFlags PACKED;
  ResType amChars PACKED;
  ResType pmChars PACKED;
  long reserved[4] PACKED;
}
TogglePB;


enum { smSystemScript = -1 };

#if !defined (TESysJust)
extern INTEGER 	TESysJust;
#endif

extern pascal trap LONGINT C_VisibleLength (Ptr textp, LONGINT len);

extern pascal trap void C_LongDate2Secs (LongDateRec *ldatep,
					 ULONGINT *secs_outp);

extern pascal trap void C_LongSecs2Date (ULONGINT *secs_inp,
					 LongDateRec *ldatep);

extern pascal trap LONGINT C_GetEnvirons( INTEGER verb ); extern pascal trap LONGINT P_GetEnvirons( INTEGER verb); 
extern pascal trap OSErr C_SetEnvirons( INTEGER verb, LONGINT param ); extern pascal trap OSErr P_SetEnvirons( INTEGER verb, LONGINT param); 
extern pascal trap LONGINT C_GetScript( INTEGER script, INTEGER verb ); extern pascal trap LONGINT P_GetScript( INTEGER script, INTEGER verb); 
extern pascal trap OSErr C_SetScript( INTEGER script, INTEGER verb, 
 LONGINT param ); extern pascal trap OSErr P_SetScript( INTEGER script, INTEGER verb, 
 LONGINT param ); 
extern pascal trap INTEGER C_Font2Script( INTEGER fontnum ); extern pascal trap INTEGER P_Font2Script( INTEGER fontnum); 
extern pascal trap INTEGER C_Transliterate( Handle srch, Handle dsth, 
 INTEGER target, LONGINT srcmask ); extern pascal trap INTEGER P_Transliterate( Handle srch, Handle dsth, 
 INTEGER target, LONGINT srcmask ); 
extern pascal trap INTEGER C_FontScript( void  ); extern pascal trap INTEGER P_FontScript( void ); 
extern pascal trap INTEGER C_IntlScript( void  ); extern pascal trap INTEGER P_IntlScript( void ); 
extern pascal trap void C_KeyScript( INTEGER scriptcode ); extern pascal trap void P_KeyScript( INTEGER scriptcode); 
extern pascal trap INTEGER C_CharType( Ptr textbufp, INTEGER offset ); extern pascal trap INTEGER P_CharType( Ptr textbufp, INTEGER offset); 
extern pascal trap void C_MeasureJust( Ptr textbufp, INTEGER length, 
 INTEGER slop, Ptr charlocs ); extern pascal trap void P_MeasureJust( Ptr textbufp, INTEGER length, 
 INTEGER slop, Ptr charlocs ); 
extern pascal trap INTEGER C_Pixel2Char( Ptr textbufp, INTEGER len, 
 INTEGER slop, INTEGER pixwidth, BOOLEAN *leftsidep ); extern pascal trap INTEGER P_Pixel2Char( Ptr textbufp, INTEGER len, 
 INTEGER slop, INTEGER pixwidth, BOOLEAN *leftsidep ); 
extern pascal trap INTEGER C_Char2Pixel( Ptr textbufp, INTEGER len, 
 INTEGER slop, INTEGER offset, SignedByte dir ); extern pascal trap INTEGER P_Char2Pixel( Ptr textbufp, INTEGER len, 
 INTEGER slop, INTEGER offset, SignedByte dir ); 
extern pascal trap void C_FindWord( Ptr textbufp, INTEGER length, 
 INTEGER offset, BOOLEAN leftside, Ptr breaks, INTEGER *offsets ); extern pascal trap void P_FindWord( Ptr textbufp, INTEGER length, 
 INTEGER offset, BOOLEAN leftside, Ptr breaks, INTEGER *offsets ); 
extern pascal trap void C_HiliteText( Ptr textbufp, INTEGER firstoffset, 
 INTEGER secondoffset, INTEGER *offsets ); extern pascal trap void P_HiliteText( Ptr textbufp, INTEGER firstoffset, 
 INTEGER secondoffset, INTEGER *offsets ); 
extern pascal trap void C_DrawJust( Ptr textbufp, INTEGER length, 
 INTEGER slop ); extern pascal trap void P_DrawJust( Ptr textbufp, INTEGER length, 
 INTEGER slop ); 
extern pascal trap String2DateStatus C_String2Time( Ptr textp, 
 LONGINT len, Ptr cachep, LONGINT *lenusedp, HIDDEN_Ptr *datetimep ); extern pascal trap String2DateStatus P_String2Time( Ptr textp, 
 LONGINT len, Ptr cachep, LONGINT *lenusedp, HIDDEN_Ptr *datetimep ); 
extern INTEGER GetSysFont( void  ); 
extern INTEGER GetAppFont( void  ); 
extern INTEGER GetMBarHeight( void  ); 
extern INTEGER GetSysJust( void  ); 
extern void SetSysJust( INTEGER just ); 
extern pascal trap OSErr C_InitDateCache( DateCachePtr theCache ); extern pascal trap OSErr P_InitDateCache( DateCachePtr theCache); 
extern pascal trap INTEGER C_CharByte( Ptr textBuf, INTEGER textOffset ); extern pascal trap INTEGER P_CharByte( Ptr textBuf, INTEGER textOffset);

extern pascal trap String2DateStatus C_String2Date (Ptr text, int32 length,
						    DateCachePtr cache,
						    int32 *length_used_ret,
						    LongDatePtr date_time);
extern pascal trap StyledLineBreakCode C_StyledLineBreak (Ptr textp, int32 length,
							  int32 text_start, int32 text_end,
							  int32 flags,
							  Fixed *text_width_fp,
							  int32 *text_offset);

extern pascal trap void C_NMeasureJust (Ptr text, int32 length,
					Fixed slop, Ptr charLocs,
					JustStyleCode run_pos,
					Point numer, Point denom);

extern pascal trap Boolean C_ParseTable (CharByteTable table);
extern pascal trap Boolean C_FillParseTable (CharByteTable table,
					     ScriptCode script);

extern pascal trap INTEGER C_ReplaceText (Handle base_text, Handle subst_text,
					  Str15 key);
extern pascal trap INTEGER C_TruncString (INTEGER width, Str255 string,
					  TruncCode code);

extern pascal trap FormatStatus C_StringToExtended
  (Str255 string, NumFormatStringRec *formatp,
   NumberParts *partsp, Extended80 *xp);

extern pascal trap FormatStatus C_ExtendedToString (Extended80 *xp,
  NumFormatStringRec *formatp, NumberParts *partsp, Str255 string);

extern pascal trap FormatStatus C_StringToFormatRec (Str255 in_string,
  NumberParts *partsp, NumFormatStringRec *out_string);

extern pascal trap ToggleResults C_ToggleDate (LongDateTime *lsecsp,
  LongDateField field, DateDelta delta, INTEGER ch, TogglePB *paramsp);

extern pascal trap Fixed C_PortionLine (
   Ptr textPtr,
   LONGINT textLen,
   JustStyleCode styleRunPosition,
   Point numer,
   Point denom);

extern pascal trap void C_DrawJustified (
   Ptr textPtr,
   LONGINT textLength,
   Fixed slop,
   JustStyleCode styleRunPosition,
   Point numer,
   Point denom);

extern pascal trap ScriptRunStatus C_FindScriptRun (
   Ptr textPtr,
   LONGINT textLen,
   LONGINT *lenUsedp);

extern pascal trap INTEGER C_PixelToChar (
   Ptr textBuf,
   LONGINT textLen,
   Fixed slop,
   Fixed pixelWidth,
   BOOLEAN *leadingEdgep,
   Fixed *widthRemainingp,
   JustStyleCode styleRunPosition,
   Point numer,
   Point denom);

extern pascal trap INTEGER C_CharToPixel (
   Ptr textBuf,
   LONGINT textLen,
   Fixed slop,
   LONGINT offset,
   INTEGER direction,
   JustStyleCode styleRunPosition,
   Point numer,
   Point denom);

extern pascal trap void C_LowercaseText (
   Ptr textp,
   INTEGER len,
   ScriptCode script);

extern pascal trap void C_UppercaseText (
   Ptr textp,
   INTEGER len,
   ScriptCode script);

extern pascal trap void C_StripDiacritics (
   Ptr textp,
   INTEGER len,
   ScriptCode script);

extern pascal trap void C_UppercaseStripDiacritics (
   Ptr textp,
   INTEGER len,
   ScriptCode script);

extern INTEGER C_CharacterByteType (Ptr textBuf, INTEGER textOffset,
				    ScriptCode script);

extern INTEGER C_CharacterType (Ptr textbufp, INTEGER offset,
				ScriptCode script);

extern INTEGER C_TransliterateText (Handle srch, Handle dsth, INTEGER target,
				    LONGINT srcmask, ScriptCode script);

#endif /* _SCRIPTMGR_H_ */
