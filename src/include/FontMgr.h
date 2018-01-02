#if !defined(_FONTMGR_H_)
#define _FONTMGR_H_

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "QuickDraw.h"
namespace Executor
{
enum
{
    systemFont = 0,
    applFont = 1,
    newYork = 2,
    geneva = 3,
    monaco = 4,
    venice = 5,
    london = 6,
    athens = 7,
    sanFran = 8,
    toronto = 9,
    cairo = 11,
    losAngeles = 12,
};

#if !defined(NEXT)
enum
{
    times = 20,
};
#else /* NEXT */
enum
{
    timesFont = 20,
};
#endif /* NEXT */

enum
{
    helvetica = 21,
    courier = 22,
    symbol = 23,
    taliesin = 24,
};

enum
{
    commandMark = 0x11,
    checkMark = 0x12,
    diamondMark = 0x13,
    appleMark = 0x14,
};

enum
{
    propFont = ((INTEGER)0x9000),
    prpFntH = ((INTEGER)0x9001),
    prpFntW = ((INTEGER)0x9002),
    prpFntHW = ((INTEGER)0x9003),
};

enum
{
    fixedFont = ((INTEGER)0xB000),
    fxdFntH = ((INTEGER)0xB001),
    fxdFntW = ((INTEGER)0xB002),
    fxdFntHW = ((INTEGER)0xB003),
};

enum
{
    fontWid = ((INTEGER)0xACB0),
};

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

enum
{
    JSwapFont = (JSwapFont_H.p),
    WidthListHand = (WidthListHand_H.p),
    ROMFont0 = (ROMFont0_H.p),
    WidthPtr = (WidthPtr_H.p),
    WidthTabHandle = (WidthTabHandle_H.p),
    LastFOND = (LastFOND_H.p),
};
#endif

extern pascal trap void C_InitFonts(void);
PASCAL_TRAP(InitFonts, 0xA8FE);

extern pascal trap void C_GetFontName(INTEGER fnum,
                                      StringPtr fnam);
PASCAL_TRAP(GetFontName, 0xA8FF);
extern void ROMlib_GetFontName(LONGINT fnum, char *fnam);
extern pascal trap void C_GetFNum(StringPtr fnam,
                                  GUEST<INTEGER> *fnum);
PASCAL_TRAP(GetFNum, 0xA900);
extern pascal trap BOOLEAN C_RealFont(INTEGER fnum,
                                      INTEGER sz);
PASCAL_TRAP(RealFont, 0xA902);
extern pascal trap void C_SetFontLock(BOOLEAN lflag);
PASCAL_TRAP(SetFontLock, 0xA903);

extern pascal trap FMOutPtr C_FMSwapFont(FMInput *fmip);
PASCAL_TRAP(FMSwapFont, 0xA901);
extern pascal trap void C_FontMetrics(FMetricRec *metrp);
PASCAL_TRAP(FontMetrics, 0xA835);

extern pascal trap void C_SetFScaleDisable(
    BOOLEAN disable);
PASCAL_TRAP(SetFScaleDisable, 0xA834);
extern pascal trap void C_SetFractEnable(BOOLEAN enable);
PASCAL_TRAP(SetFractEnable, 0xA814);

extern pascal trap void C_SetOutlinePreferred(Boolean _outline_perferred_p);
PASCAL_FUNCTION(SetOutlinePreferred);
extern pascal trap Boolean C_GetOutlinePreferred(void);
PASCAL_FUNCTION(GetOutlinePreferred);
extern pascal trap Boolean C_IsOutline(Point numer, Point denom);
PASCAL_FUNCTION(IsOutline);
extern pascal trap OSErr C_OutlineMetrics(int16_t byte_count, Ptr text,
                                          Point numer, Point denom,
                                          int16_t *y_max, int16_t *y_min,
                                          Fixed *aw_array, Fixed *lsb_array,
                                          Rect *bounds_array);
PASCAL_FUNCTION(OutlineMetrics);

extern pascal trap void C_SetPreserveGlyph(Boolean preserve_glyph);
PASCAL_FUNCTION(SetPreserveGlyph);
extern pascal trap Boolean C_GetPreserveGlyph(void);
PASCAL_FUNCTION(GetPreserveGlyph);
extern pascal trap OSErr C_FlushFonts(void);
PASCAL_FUNCTION(FlushFonts);
}
#endif /* _FONTMGR_H_ */
