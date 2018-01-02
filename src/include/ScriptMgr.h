#if !defined(_SCRIPTMGR_H_)
#define _SCRIPTMGR_H_

/*
 * Copyright 1991 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "SANE.h"
#include "IntlUtil.h"

namespace Executor
{
typedef INTEGER String2DateStatus;

enum
{
    smRoman = 0,
};

enum
{
    smCharPunct = 0,
    smCharAscii = 1,
    smCharEuro = 7,
};

enum
{
    smPunctNormal = 0x0000,
    smPunctNumber = 0x0100,
    smPunctSymbol = 0x0200,
    smPunctBlank = 0x0300,
};

enum
{
    smCharLeft = 0x0000,
    smCharRight = 0x2000,
};

enum
{
    smCharLower = 0x0000,
    smCharUpper = 0x4000,
};

enum
{
    smChar1byte = 0x0000,
    smChar2byte = 0x8000,
};

enum
{
    smTransAscii = 0,
    smTransNative = 1,
    smTransLower = 16384,
    smTransUpper = 32768,
    smMaskAscii = 1,
    smMaskNative = 2,
    smMaskAll = -1,
};

enum
{
    smScriptVersion = 0,
    smScriptMunged = 2,
    smScriptEnabled = 4,
    smScriptRight = 6,
    smScriptJust = 8,
    smScriptRedraw = 10,
    smScriptSysFond = 12,
    smScriptAppFond = 14,
    smScriptNumber = 16,
    smScriptDate = 18,
    smScriptSort = 20,
    smScriptRsvd1 = 22,
    smScriptRsvd2 = 24,
    smScriptRsvd3 = 26,
    smScriptRsvd4 = 28,
    smScriptRsvd5 = 30,
    smScriptKeys = 32,
    smScriptIcon = 34,
    smScriptPrint = 36,
    smScriptTrap = 38,
    smScriptCreator = 40,
    smScriptFile = 42,
    smScriptName = 44,
};

enum
{
    smVersion = 0,
    smMunged = 2,
    smEnabled = 4,
    smBiDirect = 6,
    smFontForce = 8,
    smIntlForce = 10,
    smForced = 12,
    smDefault = 14,
    smPrint = 16,
    smSysScript = 18,
    smAppScript = 20,
    smKeyScript = 22,
    smSysRef = 24,
    smKeyCache = 26,
    smKeySwap = 28,
};

enum
{
    smKCHRCache = 38
};

enum
{
    smVerbNotFound = (-1),
};

enum
{
    smBreakWord = 0,
    smBreakChar = 1,
    smBreakOverflow = 2,
};

/* TruncText return codes */
enum
{
    NotTruncated = 0,
    Truncated = 1,
    TruncErr = (-1),
};

typedef uint8 StyledLineBreakCode;

typedef struct DateCacheRec
{
    GUEST_STRUCT;
    GUEST<int16_t[256]> hidden;
} * DateCachePtr;

typedef struct LongDateRec
{
    GUEST_STRUCT;
    GUEST<int16_t> era;
    GUEST<int16_t> year;
    GUEST<int16_t> month;
    GUEST<int16_t> day;
    GUEST<int16_t> hour;
    GUEST<int16_t> minute;
    GUEST<int16_t> second;
    GUEST<int16_t> dayOfWeek;
    GUEST<int16_t> dayOfYear;
    GUEST<int16_t> weekOfYear;
    GUEST<int16_t> pm;
    GUEST<int16_t> res1;
    GUEST<int16_t> res2;
    GUEST<int16_t> res3;
} * LongDatePtr;

typedef INTEGER TruncCode;
typedef int16_t JustStyleCode;

typedef int8 CharByteTable[256];

typedef int16_t ScriptRunStatus; /* Not sure this is correct, since in IM
				  ScriptRunStatus is a record with two
				  Signed Bytes */

typedef INTEGER FormatStatus;

struct NumFormatStringRec
{
    GUEST_STRUCT;
    GUEST<Byte> fLength;
    GUEST<Byte> fVersion;
    GUEST<SignedByte[253]> data;
};

typedef union {
    char a[2];
    INTEGER b;
} WideChar;

typedef struct
{
    INTEGER size PACKED;
    WideChar data[10] PACKED;
} WideCharArr;

struct NumberParts
{
    GUEST_STRUCT;
    GUEST<INTEGER> version;
    GUEST<WideChar[31]> data;
    GUEST<WideCharArr> pePlus;
    GUEST<WideCharArr> peMinus;
    GUEST<WideCharArr> peMinusPlus;
    GUEST<WideCharArr> altNumTable;
    GUEST<char[20]> reserved;
};

typedef extended80 Extended80;

typedef short ToggleResults;

typedef unsigned char LongDateField;

typedef char DateDelta;

struct TogglePB
{
    GUEST_STRUCT;
    GUEST<int32_t> togFlags;
    GUEST<ResType> amChars;
    GUEST<ResType> pmChars;
    GUEST<int32_t[4]> reserved;
};

enum
{
    smSystemScript = -1
};

#if !defined(TESysJust)
extern INTEGER TESysJust;
#endif

extern pascal trap LONGINT C_VisibleLength(Ptr textp, LONGINT len);
PASCAL_FUNCTION(VisibleLength);

extern pascal trap void C_LongDate2Secs(LongDateRec *ldatep,
                                        GUEST<ULONGINT> *secs_outp);
PASCAL_FUNCTION(LongDate2Secs);

extern pascal trap void C_LongSecs2Date(GUEST<ULONGINT> *secs_inp,
                                        LongDateRec *ldatep);
PASCAL_FUNCTION(LongSecs2Date);

extern pascal trap LONGINT C_GetEnvirons(INTEGER verb);
PASCAL_FUNCTION(GetEnvirons);

extern pascal trap OSErr C_SetEnvirons(INTEGER verb, LONGINT param);
PASCAL_FUNCTION(SetEnvirons);

extern pascal trap LONGINT C_GetScript(INTEGER script, INTEGER verb);
PASCAL_FUNCTION(GetScript);

extern pascal trap OSErr C_SetScript(INTEGER script, INTEGER verb,
                                     LONGINT param);
PASCAL_FUNCTION(SetScript);
extern pascal trap INTEGER C_Font2Script(INTEGER fontnum);
PASCAL_FUNCTION(Font2Script);

extern pascal trap INTEGER C_Transliterate(Handle srch, Handle dsth,
                                           INTEGER target, LONGINT srcmask);
PASCAL_FUNCTION(Transliterate);
extern pascal trap INTEGER C_FontScript(void);
PASCAL_FUNCTION(FontScript);

extern pascal trap INTEGER C_IntlScript(void);
PASCAL_FUNCTION(IntlScript);

extern pascal trap void C_KeyScript(INTEGER scriptcode);
PASCAL_FUNCTION(KeyScript);

extern pascal trap INTEGER C_CharType(Ptr textbufp, INTEGER offset);
PASCAL_FUNCTION(CharType);

extern pascal trap void C_MeasureJust(Ptr textbufp, INTEGER length,
                                      INTEGER slop, Ptr charlocs);
PASCAL_FUNCTION(MeasureJust);
extern pascal trap INTEGER C_Pixel2Char(Ptr textbufp, INTEGER len,
                                        INTEGER slop, INTEGER pixwidth, BOOLEAN *leftsidep);
PASCAL_FUNCTION(Pixel2Char);
extern pascal trap INTEGER C_Char2Pixel(Ptr textbufp, INTEGER len,
                                        INTEGER slop, INTEGER offset, SignedByte dir);
PASCAL_FUNCTION(Char2Pixel);
extern pascal trap void C_FindWord(Ptr textbufp, INTEGER length,
                                   INTEGER offset, BOOLEAN leftside, Ptr breaks, GUEST<INTEGER> *offsets);
PASCAL_FUNCTION(FindWord);
extern pascal trap void C_HiliteText(Ptr textbufp, INTEGER firstoffset,
                                     INTEGER secondoffset, GUEST<INTEGER> *offsets);
PASCAL_FUNCTION(HiliteText);
extern pascal trap void C_DrawJust(Ptr textbufp, INTEGER length,
                                   INTEGER slop);
PASCAL_FUNCTION(DrawJust);
extern pascal trap String2DateStatus C_String2Time(Ptr textp,
                                                   LONGINT len, Ptr cachep, GUEST<LONGINT> *lenusedp, GUEST<Ptr> *datetimep);
PASCAL_FUNCTION(String2Time);
extern INTEGER GetSysFont(void);
extern INTEGER GetAppFont(void);
extern INTEGER GetMBarHeight(void);
extern INTEGER GetSysJust(void);
extern void SetSysJust(INTEGER just);
extern pascal trap OSErr C_InitDateCache(DateCachePtr theCache);
PASCAL_FUNCTION(InitDateCache);

extern pascal trap INTEGER C_CharByte(Ptr textBuf, INTEGER textOffset);
PASCAL_FUNCTION(CharByte);

extern pascal trap String2DateStatus C_String2Date(Ptr text, int32_t length,
                                                   DateCachePtr cache,
                                                   GUEST<int32_t> *length_used_ret,
                                                   LongDatePtr date_time);
PASCAL_FUNCTION(String2Date);
extern pascal trap StyledLineBreakCode C_StyledLineBreak(Ptr textp, int32_t length,
                                                         int32_t text_start, int32_t text_end,
                                                         int32_t flags,
                                                         GUEST<Fixed> *text_width_fp,
                                                         GUEST<int32_t> *text_offset);
PASCAL_FUNCTION(StyledLineBreak);

extern pascal trap void C_NMeasureJust(Ptr text, int32_t length,
                                       Fixed slop, Ptr charLocs,
                                       JustStyleCode run_pos,
                                       Point numer, Point denom);
PASCAL_FUNCTION(NMeasureJust);

extern pascal trap Boolean C_ParseTable(CharByteTable table);
PASCAL_FUNCTION(ParseTable);
extern pascal trap Boolean C_FillParseTable(CharByteTable table,
                                            ScriptCode script);
PASCAL_FUNCTION(FillParseTable);

extern pascal trap INTEGER C_ReplaceText(Handle base_text, Handle subst_text,
                                         Str15 key);
PASCAL_FUNCTION(ReplaceText);
extern pascal trap INTEGER C_TruncString(INTEGER width, Str255 string,
                                         TruncCode code);
PASCAL_FUNCTION(TruncString);

extern pascal trap FormatStatus C_StringToExtended(Str255 string, NumFormatStringRec *formatp,
                                                   NumberParts *partsp, Extended80 *xp);
PASCAL_FUNCTION(StringToExtended);

extern pascal trap FormatStatus C_ExtendedToString(Extended80 *xp,
                                                   NumFormatStringRec *formatp, NumberParts *partsp, Str255 string);
PASCAL_FUNCTION(ExtendedToString);

extern pascal trap FormatStatus C_StringToFormatRec(Str255 in_string,
                                                    NumberParts *partsp, NumFormatStringRec *out_string);
PASCAL_FUNCTION(StringToFormatRec);

extern pascal trap ToggleResults C_ToggleDate(LongDateTime *lsecsp,
                                              LongDateField field, DateDelta delta, INTEGER ch, TogglePB *paramsp);
PASCAL_FUNCTION(ToggleDate);

extern pascal trap Fixed C_PortionLine(
    Ptr textPtr,
    LONGINT textLen,
    JustStyleCode styleRunPosition,
    Point numer,
    Point denom);
PASCAL_FUNCTION(PortionLine);

extern pascal trap void C_DrawJustified(
    Ptr textPtr,
    LONGINT textLength,
    Fixed slop,
    JustStyleCode styleRunPosition,
    Point numer,
    Point denom);
PASCAL_FUNCTION(DrawJustified);

extern pascal trap ScriptRunStatus C_FindScriptRun(
    Ptr textPtr,
    LONGINT textLen,
    GUEST<LONGINT> *lenUsedp);
PASCAL_FUNCTION(FindScriptRun);

extern pascal trap INTEGER C_PixelToChar(
    Ptr textBuf,
    LONGINT textLen,
    Fixed slop,
    Fixed pixelWidth,
    BOOLEAN *leadingEdgep,
    GUEST<Fixed> *widthRemainingp,
    JustStyleCode styleRunPosition,
    Point numer,
    Point denom);
PASCAL_FUNCTION(PixelToChar);

extern pascal trap INTEGER C_CharToPixel(
    Ptr textBuf,
    LONGINT textLen,
    Fixed slop,
    LONGINT offset,
    INTEGER direction,
    JustStyleCode styleRunPosition,
    Point numer,
    Point denom);
PASCAL_FUNCTION(CharToPixel);

extern pascal trap void C_LowercaseText(
    Ptr textp,
    INTEGER len,
    ScriptCode script);
PASCAL_FUNCTION(LowercaseText);

extern pascal trap void C_UppercaseText(
    Ptr textp,
    INTEGER len,
    ScriptCode script);
PASCAL_FUNCTION(UppercaseText);

extern pascal trap void C_StripDiacritics(
    Ptr textp,
    INTEGER len,
    ScriptCode script);
PASCAL_FUNCTION(StripDiacritics);

extern pascal trap void C_UppercaseStripDiacritics(
    Ptr textp,
    INTEGER len,
    ScriptCode script);
PASCAL_FUNCTION(UppercaseStripDiacritics);

extern INTEGER C_CharacterByteType(Ptr textBuf, INTEGER textOffset,
                                   ScriptCode script);

extern INTEGER C_CharacterType(Ptr textbufp, INTEGER offset,
                               ScriptCode script);

extern INTEGER C_TransliterateText(Handle srch, Handle dsth, INTEGER target,
                                   LONGINT srcmask, ScriptCode script);
}

#endif /* _SCRIPTMGR_H_ */
