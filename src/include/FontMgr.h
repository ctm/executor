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

typedef struct {
    Fixed ascent	PACKED;
    Fixed descent	PACKED;
    Fixed leading	PACKED;
    Fixed widMax	PACKED;
    Handle wTabHandle	PACKED_P;
} FMetricRec;

typedef struct {
    INTEGER ffFlags	PACKED;
    INTEGER ffFamID	PACKED;
    INTEGER ffFirstChar	PACKED;
    INTEGER ffLastChar	PACKED;
    INTEGER ffAscent	PACKED;
    INTEGER ffDescent	PACKED;
    INTEGER ffLeading	PACKED;
    INTEGER ffWidMax	PACKED;
    LONGINT ffWTabOff	PACKED;
    LONGINT ffKernOff	PACKED;
    LONGINT ffStylOff	PACKED;
    INTEGER ffProperty[9]	PACKED;
    INTEGER ffIntl[2]	PACKED;
    INTEGER ffVersion	PACKED;
    /* FontAssoc ffAssoc	PACKED; */
    /* WidTable ffWidthTab	PACKED; */
    /* StyleTable ffStyTab	PACKED; */
    /* KernTable ffKernTab	PACKED; */
} FamRec;

typedef struct {
    Fixed tabData[256]	PACKED;
    Handle tabFont	PACKED_P;
    LONGINT sExtra	PACKED;
    LONGINT style	PACKED;
    INTEGER fID	PACKED;
    INTEGER fSize	PACKED;
    INTEGER face	PACKED;
    INTEGER device	PACKED;
    Point inNumer	LPACKED;
    Point inDenom	LPACKED;
    INTEGER aFID	PACKED;
    Handle fHand	PACKED_P;
    BOOLEAN usedFam	LPACKED;
    Byte aFace	LPACKED;
    INTEGER vOutput	PACKED;
    INTEGER hOutput	PACKED;
    INTEGER vFactor	PACKED;
    INTEGER hFactor	PACKED;
    INTEGER aSize	PACKED;
    INTEGER tabSize	PACKED;
} WidthTable;


typedef struct {
    INTEGER family	PACKED;
    INTEGER size	PACKED;
    Style face	LPACKED;
    BOOLEAN needBits	LPACKED;
    INTEGER device	PACKED;
    Point numer	LPACKED;
    Point denom	LPACKED;
} FMInput;

typedef struct {
    INTEGER errNum	PACKED;	/* 0x00 */
    Handle fontHandle	PACKED_P;	/* 0x02 */
    Byte bold	LPACKED;		/* 0x06 */
    Byte italic	LPACKED;	/* 0x07 */
    Byte ulOffset	LPACKED;	/* 0x08 */
    Byte ulShadow	LPACKED;	/* 0x09 */
    Byte ulThick	LPACKED;	/* 0x0A */
    Byte shadow	LPACKED;	/* 0x0B */
    SignedByte extra	LPACKED;	/* 0x0C */
    Byte ascent	LPACKED;	/* 0x0D */
    Byte descent	LPACKED;	/* 0x0E */
    Byte widMax	LPACKED;	/* 0x0F */
    SignedByte leading	LPACKED;	/* 0x10 */
    Byte unused	LPACKED;	/* 0x11 */
    Point numer	LPACKED;	/* 0x12 */
    Point denom	LPACKED;	/* 0x16 */
} FMOutput;

typedef FMOutput *FMOutPtr;

typedef struct {
    INTEGER fontType	PACKED;
    INTEGER firstChar	PACKED;
    INTEGER lastChar	PACKED;
    INTEGER widMax	PACKED;
    INTEGER kernMax	PACKED;
    INTEGER nDescent	PACKED;
    INTEGER fRectWidth	PACKED;
    INTEGER fRectHeight	PACKED;
    INTEGER owTLoc	PACKED;
    INTEGER ascent	PACKED;
    INTEGER descent	PACKED;
    INTEGER leading	PACKED;
    INTEGER rowWords	PACKED;
    /* more stuff is usually appended here ... bitImage	PACKED, locTable	PACKED, owTable */
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
