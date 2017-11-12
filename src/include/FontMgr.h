#if !defined(_FONTMGR_H_)
#define _FONTMGR_H_

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: FontMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "QuickDraw.h"
namespace Executor
{
#define systemFont 0
#define applFont 1
#define newYork 2
#define geneva 3
#define monaco 4
#define venice 5
#define london 6
#define athens 7
#define sanFran 8
#define toronto 9
#define cairo 11
#define losAngeles 12

#if !defined(NEXT)
#define times 20
#else /* NEXT */
#define timesFont 20
#endif /* NEXT */

#define helvetica 21
#define courier 22
#define symbol 23
#define taliesin 24

#define commandMark 0x11
#define checkMark 0x12
#define diamondMark 0x13
#define appleMark 0x14

#define propFont ((INTEGER)0x9000)
#define prpFntH ((INTEGER)0x9001)
#define prpFntW ((INTEGER)0x9002)
#define prpFntHW ((INTEGER)0x9003)

#define fixedFont ((INTEGER)0xB000)
#define fxdFntH ((INTEGER)0xB001)
#define fxdFntW ((INTEGER)0xB002)
#define fxdFntHW ((INTEGER)0xB003)

#define fontWid ((INTEGER)0xACB0)

struct FMetricRec
{
    GUEST_STRUCT;
    GUEST<Fixed> ascent;
    GUEST<Fixed> descent;
    GUEST<Fixed> leading;
    GUEST<Fixed> widMax;
    GUEST<Handle> wTabHandle;
};

/* FontAssoc ffAssoc; */
/* WidTable ffWidthTab; */
/* StyleTable ffStyTab; */
/* KernTable ffKernTab; */
struct FamRec
{
    GUEST_STRUCT;
    GUEST<INTEGER> ffFlags;
    GUEST<INTEGER> ffFamID;
    GUEST<INTEGER> ffFirstChar;
    GUEST<INTEGER> ffLastChar;
    GUEST<INTEGER> ffAscent;
    GUEST<INTEGER> ffDescent;
    GUEST<INTEGER> ffLeading;
    GUEST<INTEGER> ffWidMax;
    GUEST<LONGINT> ffWTabOff;
    GUEST<LONGINT> ffKernOff;
    GUEST<LONGINT> ffStylOff;
    GUEST<INTEGER[9]> ffProperty;
    GUEST<INTEGER[2]> ffIntl;
    GUEST<INTEGER> ffVersion;
};

struct WidthTable
{
    GUEST_STRUCT;
    GUEST<Fixed[256]> tabData;
    GUEST<Handle> tabFont;
    GUEST<LONGINT> sExtra;
    GUEST<LONGINT> style;
    GUEST<INTEGER> fID;
    GUEST<INTEGER> fSize;
    GUEST<INTEGER> face;
    GUEST<INTEGER> device;
    GUEST<Point> inNumer;
    GUEST<Point> inDenom;
    GUEST<INTEGER> aFID;
    GUEST<Handle> fHand;
    GUEST<BOOLEAN> usedFam;
    GUEST<Byte> aFace;
    GUEST<INTEGER> vOutput;
    GUEST<INTEGER> hOutput;
    GUEST<INTEGER> vFactor;
    GUEST<INTEGER> hFactor;
    GUEST<INTEGER> aSize;
    GUEST<INTEGER> tabSize;
};

struct FMInput
{
    GUEST_STRUCT;
    GUEST<INTEGER> family;
    GUEST<INTEGER> size;
    GUEST<Style> face;
    GUEST<BOOLEAN> needBits;
    GUEST<INTEGER> device;
    GUEST<Point> numer;
    GUEST<Point> denom;
};

struct FMOutput
{
    GUEST_STRUCT;
    GUEST<INTEGER> errNum; /* 0x00 */
    GUEST<Handle> fontHandle; /* 0x02 */
    GUEST<Byte> bold; /* 0x06 */
    GUEST<Byte> italic; /* 0x07 */
    GUEST<Byte> ulOffset; /* 0x08 */
    GUEST<Byte> ulShadow; /* 0x09 */
    GUEST<Byte> ulThick; /* 0x0A */
    GUEST<Byte> shadow; /* 0x0B */
    GUEST<SignedByte> extra; /* 0x0C */
    GUEST<Byte> ascent; /* 0x0D */
    GUEST<Byte> descent; /* 0x0E */
    GUEST<Byte> widMax; /* 0x0F */
    GUEST<SignedByte> leading; /* 0x10 */
    GUEST<Byte> unused; /* 0x11 */
    GUEST<Point> numer; /* 0x12 */
    GUEST<Point> denom; /* 0x16 */
};

typedef FMOutput *FMOutPtr;

/* more stuff is usually appended here ... bitImage, locTable, owTable */
struct FontRec
{
    GUEST_STRUCT;
    GUEST<INTEGER> fontType;
    GUEST<INTEGER> firstChar;
    GUEST<INTEGER> lastChar;
    GUEST<INTEGER> widMax;
    GUEST<INTEGER> kernMax;
    GUEST<INTEGER> nDescent;
    GUEST<INTEGER> fRectWidth;
    GUEST<INTEGER> fRectHeight;
    GUEST<INTEGER> owTLoc;
    GUEST<INTEGER> ascent;
    GUEST<INTEGER> descent;
    GUEST<INTEGER> leading;
    GUEST<INTEGER> rowWords;
};

typedef FamRec *FamRecPtr;

typedef GUEST<FamRecPtr> *FamRecHandle;

typedef WidthTable *WidthTablePtr;

typedef GUEST<WidthTablePtr> *WidthTableHandle;

#if 0
#if !defined(JSwapFont_H)
extern GUEST<ProcPtr> 	JSwapFont_H;
extern GUEST<Handle> 	WidthListHand_H;
extern GUEST<Handle> 	ROMFont0_H;
extern INTEGER 	ApFontID;
extern FMInput 	ROMlib_myfmi;
extern FMOutput 	ROMlib_fmo;
extern Byte 	FScaleDisable;
extern GUEST<WidthTablePtr> WidthPtr_H;
extern GUEST<WidthTableHandle> WidthTabHandle_H;
extern LONGINT 	IntlSpec;
extern INTEGER 	SysFontFam;
extern INTEGER 	SysFontSiz;
extern GUEST<FamRecHandle> LastFOND_H;
extern INTEGER 	fondid;
extern Byte 	FractEnable;
#endif

#define JSwapFont (JSwapFont_H.p)
#define WidthListHand (WidthListHand_H.p)
#define ROMFont0 (ROMFont0_H.p)
#define WidthPtr (WidthPtr_H.p)
#define WidthTabHandle (WidthTabHandle_H.p)
#define LastFOND (LastFOND_H.p)
#endif

extern pascal trap void C_InitFonts(void);
extern pascal trap void P_InitFonts(void);
extern pascal trap void C_GetFontName(INTEGER fnum,
                                      StringPtr fnam);
extern pascal trap void P_GetFontName(INTEGER fnum,
                                      StringPtr fnam);
extern void ROMlib_GetFontName(LONGINT fnum, char *fnam);
extern pascal trap void C_GetFNum(StringPtr fnam,
                                  GUEST<INTEGER> *fnum);
extern pascal trap BOOLEAN C_RealFont(INTEGER fnum,
                                      INTEGER sz);
extern pascal trap BOOLEAN P_RealFont(INTEGER fnum,
                                      INTEGER sz);
extern pascal trap void C_SetFontLock(BOOLEAN lflag);
extern pascal trap void P_SetFontLock(BOOLEAN lflag);
extern pascal trap FMOutPtr C_FMSwapFont(FMInput *fmip);
extern pascal trap void C_FontMetrics(FMetricRec *metrp);
extern pascal trap void P_FontMetrics(FMetricRec *metrp);
extern pascal trap void C_SetFScaleDisable(
    BOOLEAN disable);
extern pascal trap void P_SetFScaleDisable(
    BOOLEAN disable);
extern pascal trap void C_SetFractEnable(BOOLEAN enable);
extern pascal trap void P_SetFractEnable(BOOLEAN enable);

extern pascal trap void C_SetOutlinePreferred(Boolean _outline_perferred_p);
extern pascal trap Boolean C_GetOutlinePreferred(void);
extern pascal trap Boolean C_IsOutline(Point numer, Point denom);
extern pascal trap OSErr C_OutlineMetrics(int16 byte_count, Ptr text,
                                          Point numer, Point denom,
                                          int16 *y_max, int16 *y_min,
                                          Fixed *aw_array, Fixed *lsb_array,
                                          Rect *bounds_array);

extern pascal trap void C_SetPreserveGlyph(Boolean preserve_glyph);
extern pascal trap Boolean C_GetPreserveGlyph(void);
extern pascal trap OSErr C_FlushFonts(void);
}
#endif /* _FONTMGR_H_ */
