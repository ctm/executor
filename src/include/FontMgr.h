#if !defined (_FONTMGR_H_)
#define _FONTMGR_H_

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: FontMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "QuickDraw.h"

#define systemFont	0
#define applFont	1
#define newYork		2
#define geneva		3
#define monaco		4
#define venice		5
#define london		6
#define athens		7
#define sanFran		8
#define toronto		9
#define cairo		11
#define losAngeles	12

#if !defined (NEXT)
#define times		20
#else /* NEXT */
#define timesFont	20
#endif /* NEXT */

#define helvetica	21
#define courier		22
#define symbol		23
#define taliesin	24

#define commandMark	0x11
#define checkMark	0x12
#define diamondMark	0x13
#define appleMark	0x14

#define propFont	((INTEGER) 0x9000)
#define prpFntH		((INTEGER) 0x9001)
#define prpFntW		((INTEGER) 0x9002)
#define prpFntHW	((INTEGER) 0x9003)

#define fixedFont	((INTEGER) 0xB000)
#define fxdFntH		((INTEGER) 0xB001)
#define fxdFntW		((INTEGER) 0xB002)
#define fxdFntHW	((INTEGER) 0xB003)

#define fontWid		((INTEGER) 0xACB0)

typedef struct PACKED {
  Fixed ascent;
  Fixed descent;
  Fixed leading;
  Fixed widMax;
  Handle wTabHandle	PACKED_P;
} FMetricRec;

typedef struct PACKED {
  INTEGER ffFlags;
  INTEGER ffFamID;
  INTEGER ffFirstChar;
  INTEGER ffLastChar;
  INTEGER ffAscent;
  INTEGER ffDescent;
  INTEGER ffLeading;
  INTEGER ffWidMax;
  LONGINT ffWTabOff;
  LONGINT ffKernOff;
  LONGINT ffStylOff;
  INTEGER ffProperty[9];
  INTEGER ffIntl[2];
  INTEGER ffVersion;
  /* FontAssoc ffAssoc; */
  /* WidTable ffWidthTab; */
  /* StyleTable ffStyTab; */
  /* KernTable ffKernTab; */
} FamRec;

typedef struct PACKED {
  Fixed tabData[256];
  Handle tabFont	PACKED_P;
  LONGINT sExtra;
  LONGINT style;
  INTEGER fID;
  INTEGER fSize;
  INTEGER face;
  INTEGER device;
  Point inNumer;
  Point inDenom;
  INTEGER aFID;
  Handle fHand	PACKED_P;
  BOOLEAN usedFam;
  Byte aFace;
  INTEGER vOutput;
  INTEGER hOutput;
  INTEGER vFactor;
  INTEGER hFactor;
  INTEGER aSize;
  INTEGER tabSize;
} WidthTable;


typedef struct PACKED {
  INTEGER family;
  INTEGER size;
  Style face;
  BOOLEAN needBits;
  INTEGER device;
  Point numer;
  Point denom;
} FMInput;

typedef struct PACKED {
    INTEGER errNum;	/* 0x00 */
    Handle fontHandle	PACKED_P;	/* 0x02 */
    Byte bold;		/* 0x06 */
    Byte italic;	/* 0x07 */
    Byte ulOffset;	/* 0x08 */
    Byte ulShadow;	/* 0x09 */
    Byte ulThick;	/* 0x0A */
    Byte shadow;	/* 0x0B */
    SignedByte extra;	/* 0x0C */
    Byte ascent;	/* 0x0D */
    Byte descent;	/* 0x0E */
    Byte widMax;	/* 0x0F */
    SignedByte leading;	/* 0x10 */
    Byte unused;	/* 0x11 */
    Point numer;	/* 0x12 */
    Point denom;	/* 0x16 */
} FMOutput;

typedef FMOutput *FMOutPtr;

typedef struct PACKED {
  INTEGER fontType;
  INTEGER firstChar;
  INTEGER lastChar;
  INTEGER widMax;
  INTEGER kernMax;
  INTEGER nDescent;
  INTEGER fRectWidth;
  INTEGER fRectHeight;
  INTEGER owTLoc;
  INTEGER ascent;
  INTEGER descent;
  INTEGER leading;
  INTEGER rowWords;
  /* more stuff is usually appended here ... bitImage, locTable, owTable */
} FontRec;

typedef FamRec *FamRecPtr;
typedef struct { FamRecPtr p PACKED_P; } HIDDEN_FamRecPtr;
typedef HIDDEN_FamRecPtr *FamRecHandle;
typedef struct { FamRecHandle p PACKED_P; } HIDDEN_FamRecHandle;

typedef WidthTable *WidthTablePtr;
typedef struct { WidthTablePtr p PACKED_P; } HIDDEN_WidthTablePtr;
typedef HIDDEN_WidthTablePtr *WidthTableHandle;
typedef struct { WidthTableHandle p PACKED_P; } HIDDEN_WidthTableHandle;

#if !defined (JSwapFont_H)
extern HIDDEN_ProcPtr 	JSwapFont_H;
extern HIDDEN_Handle 	WidthListHand_H;
extern HIDDEN_Handle 	ROMFont0_H;
extern INTEGER 	ApFontID;
extern FMInput 	ROMlib_myfmi;
extern FMOutput 	ROMlib_fmo;
extern Byte 	FScaleDisable;
extern HIDDEN_WidthTablePtr WidthPtr_H;
extern HIDDEN_WidthTableHandle WidthTabHandle_H;
extern LONGINT 	IntlSpec;
extern INTEGER 	SysFontFam;
extern INTEGER 	SysFontSiz;
extern HIDDEN_FamRecHandle LastFOND_H;
extern INTEGER 	fondid;
extern Byte 	FractEnable;
#endif

#define JSwapFont	(JSwapFont_H.p)
#define WidthListHand	(WidthListHand_H.p)
#define ROMFont0	(ROMFont0_H.p)
#define WidthPtr	(WidthPtr_H.p)
#define WidthTabHandle	(WidthTabHandle_H.p)
#define LastFOND	(LastFOND_H.p)

extern pascal trap void C_InitFonts( void  ); extern pascal trap void P_InitFonts( void ); 
extern pascal trap void C_GetFontName( INTEGER fnum, 
 StringPtr fnam ); extern pascal trap void P_GetFontName( INTEGER fnum, 
 StringPtr fnam ); 
extern void ROMlib_GetFontName( LONGINT fnum, char *fnam ); 
extern pascal trap void C_GetFNum( StringPtr fnam, 
 INTEGER *fnum ); extern pascal trap void P_GetFNum( StringPtr fnam, 
 INTEGER *fnum ); 
extern pascal trap BOOLEAN C_RealFont( INTEGER fnum, 
 INTEGER sz ); extern pascal trap BOOLEAN P_RealFont( INTEGER fnum, 
 INTEGER sz ); 
extern pascal trap void C_SetFontLock( BOOLEAN lflag ); extern pascal trap void P_SetFontLock( BOOLEAN lflag); 
extern pascal trap FMOutPtr C_FMSwapFont( FMInput *fmip );
extern pascal trap void C_FontMetrics( FMetricRec *metrp ); extern pascal trap void P_FontMetrics( FMetricRec *metrp); 
extern pascal trap void C_SetFScaleDisable( 
 BOOLEAN disable ); extern pascal trap void P_SetFScaleDisable( 
 BOOLEAN disable ); 
extern pascal trap void C_SetFractEnable( BOOLEAN enable ); extern pascal trap void P_SetFractEnable( BOOLEAN enable);

extern pascal trap void C_SetOutlinePreferred (Boolean _outline_perferred_p);
extern pascal trap Boolean C_GetOutlinePreferred (void);
extern pascal trap Boolean C_IsOutline (Point numer, Point denom);
extern pascal trap OSErr C_OutlineMetrics (int16 byte_count, Ptr text,
					   Point numer, Point denom,
					   int16 *y_max, int16 *y_min,
					   Fixed *aw_array, Fixed *lsb_array,
					   Rect *bounds_array);

extern pascal trap void C_SetPreserveGlyph (Boolean preserve_glyph);
extern pascal trap Boolean C_GetPreserveGlyph (void);
extern pascal trap OSErr C_FlushFonts (void);

#endif /* _FONTMGR_H_ */
