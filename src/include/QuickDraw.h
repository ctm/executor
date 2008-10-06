#if !defined (_QUICKDRAW_H_)
#define _QUICKDRAW_H_

/*
 * Copyright 1986 - 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: QuickDraw.h 63 2004-12-24 18:19:43Z ctm $
 */


enum { grafSize = 206 }; /* number of bytes InitGraf needs */

#define srcCopy	0
#define srcOr	1
#define srcXor	2
#define srcBic	3
#define notSrcCopy	4
#define notSrcOr	5
#define notSrcXor	6
#define notSrcBic	7

#define patCopy	8
#define patOr	9
#define patXor	10
#define patBic	11
#define notPatCopy	12
#define notPatOr	13
#define notPatXor	14
#define notPatBic	15

#define grayishTextOr	49

#define hilite		50

#define blackColor	33
#define whiteColor	30
#define redColor	205
#define greenColor	341
#define blueColor	409
#define cyanColor	273
#define magentaColor	137
#define yellowColor	69

#define picLParen	0
#define picRParen	1

typedef enum { bold=1,    italic=2,    underline=4, outline=8,
	       shadow=16, condense=32, extend=64 } StyleItem;

#if 0
/*
 * The enum doesn't suffice since some compilers will complain if you
 * try do bitwise arithmetic with the members of an enum
 *
 * Unfortunately, we can't do the #defines below because there are structures
 * that have fields named "bold".
 */

#define bold		1
#define italic		2
#define underline	4
#define outline		8
#define shadow		16
#define condense	32
#define extend		64
#endif /* 0 */

typedef SignedByte Style;

typedef struct {
    INTEGER rgnSize	PACKED;
    Rect rgnBBox	PACKED;
} Region;

typedef Region *RgnPtr;
typedef struct { RgnPtr p PACKED_P; } HIDDEN_RgnPtr;
typedef HIDDEN_RgnPtr *RgnHandle;
typedef struct { RgnHandle p PACKED_P; } HIDDEN_RgnHandle;

typedef struct {
    Ptr baseAddr	PACKED_P;
    INTEGER rowBytes	PACKED;
    Rect bounds	PACKED;
} BitMap;

typedef Byte Pattern[8];
typedef INTEGER Bits16[16];

typedef struct {
    Bits16 data	PACKED;
    Bits16 mask	PACKED;
    Point hotSpot	PACKED;
} Cursor;

typedef Cursor *CursPtr;
typedef struct { CursPtr p PACKED_P; } HIDDEN_CursPtr;
typedef HIDDEN_CursPtr *CursHandle;

typedef SignedByte GrafVerb;
#define frame	0
#define paint	1
#define erase	2
#define invert	3
#define fill	4

typedef struct {
    INTEGER polySize	PACKED;
    Rect polyBBox	PACKED;
    Point polyPoints[1]	PACKED;
} Polygon;

typedef Polygon *PolyPtr;
typedef struct { PolyPtr p PACKED_P; } HIDDEN_PolyPtr;
typedef HIDDEN_PolyPtr *PolyHandle;

typedef struct {
    INTEGER ascent	PACKED;
    INTEGER descent	PACKED;
    INTEGER widMax	PACKED;
    INTEGER leading	PACKED;
} FontInfo;

typedef struct {
    pascal trap void (*textProc)(INTEGER bc, Ptr textb, Point num, Point den)	PACKED_P;
    pascal trap void (*lineProc)(Point drawto)	PACKED_P;
    pascal trap void (*rectProc)(GrafVerb verb, Rect *rp)	PACKED_P;
    pascal trap void (*rRectProc)(GrafVerb verb, Rect *rp, INTEGER ow,
								   INTEGER oh)	PACKED_P;
    pascal trap void (*ovalProc)(GrafVerb verb, Rect *rp)	PACKED_P;
    pascal trap void (*arcProc)(GrafVerb verb, Rect *rp, INTEGER ang,
								  INTEGER arc)	PACKED_P;
    pascal trap void (*polyProc)(GrafVerb verb, PolyHandle poly)	PACKED_P;
    pascal trap void (*rgnProc)(GrafVerb verb, RgnHandle rgn)	PACKED_P;
    pascal trap void (*bitsProc)(BitMap *srcb, Rect *srcr, Rect *dstr,
						  INTEGER mod, RgnHandle mask)	PACKED_P;
    pascal trap void (*commentProc)(INTEGER kind, INTEGER size, Handle data)	PACKED_P;
    pascal trap INTEGER  (*txMeasProc)(INTEGER bc, Ptr texta, Point *numer,
						 Point *denom, FontInfo *info)	PACKED_P;
    pascal trap void (*getPicProc)(Ptr data, INTEGER bc)	PACKED_P;
    pascal trap void (*putPicProc)(Ptr data, INTEGER bc)	PACKED_P;
} QDProcs;

typedef QDProcs *QDProcsPtr;

typedef struct {
    INTEGER device	PACKED;
    BitMap portBits	PACKED;
    Rect portRect	PACKED;
    RgnHandle visRgn	PACKED_P;
    RgnHandle clipRgn	PACKED_P;
    Pattern bkPat	PACKED;
    Pattern fillPat	PACKED;
    Point pnLoc	PACKED;
    Point pnSize	PACKED;
    INTEGER pnMode	PACKED;
    Pattern pnPat	PACKED;
    INTEGER pnVis	PACKED;
    INTEGER txFont	PACKED;
    Style txFace	PACKED;
    Byte filler		PACKED;
    INTEGER txMode	PACKED;
    INTEGER txSize	PACKED;
    Fixed spExtra	PACKED;
    LONGINT fgColor	PACKED;
    LONGINT bkColor	PACKED;
    INTEGER colrBit	PACKED;
    INTEGER patStretch	PACKED;
    Handle picSave	PACKED_P;
    Handle rgnSave	PACKED_P;
    Handle polySave	PACKED_P;
    QDProcsPtr grafProcs	PACKED_P;
} GrafPort;

typedef GrafPort *GrafPtr;
typedef struct { GrafPtr p PACKED_P; } HIDDEN_GrafPtr;

typedef struct {
    INTEGER picSize	PACKED;
    Rect picFrame	PACKED;
} Picture;

typedef Picture *PicPtr;
typedef struct { PicPtr p PACKED_P; } HIDDEN_PicPtr;
typedef HIDDEN_PicPtr *PicHandle;

typedef struct {
    Point pnLoc	PACKED;
    Point pnSize	PACKED;
    INTEGER pnMode	PACKED;
    Pattern pnPat	PACKED;
} PenState;

/* IMV stuff is used when we parse Version 2 pictures, but the IMV calls
   are not supported in V1.0 */

typedef enum { blend=32,    addPin, addOver, subPin,
	       transparent, adMax,  subOver, adMin,
	       mask=64 } colormodes;

#define pHiliteBit	 0

#define defQDColors	127

typedef struct {
    unsigned short red		PACKED;
    unsigned short green	PACKED;
    unsigned short blue		PACKED;
} RGBColor;

typedef struct {
  SmallFract hue		PACKED;
  SmallFract saturation		PACKED;
  SmallFract value		PACKED;
} HSVColor;

typedef struct {
  SmallFract hue		PACKED;
  SmallFract saturation		PACKED;
  SmallFract lightness		PACKED;
} HSLColor;

typedef struct { 
  SmallFract cyan		PACKED;
  SmallFract magenta		PACKED;
  SmallFract yellow		PACKED;
} CMYColor;

typedef struct ColorSpec
{
  INTEGER	value	PACKED;
  RGBColor	rgb	PACKED;
} ColorSpec;

typedef ColorSpec cSpecArray[1];	/* can't use 0 */

typedef struct {
    LONGINT    ctSeed	PACKED;
    unsigned short    ctFlags	PACKED;
    INTEGER    ctSize	PACKED;
    cSpecArray ctTable	PACKED;
} ColorTable, *CTabPtr;

typedef struct { CTabPtr p PACKED_P; } HIDDEN_CTabPtr;
typedef HIDDEN_CTabPtr *CTabHandle;
typedef struct { CTabHandle p PACKED_P; } HIDDEN_CTabHandle;

typedef struct {
    Ptr textProc	PACKED_P;
    Ptr lineProc	PACKED_P;
    Ptr rectProc	PACKED_P;
    Ptr rRectProc	PACKED_P;
    Ptr ovalProc	PACKED_P;
    Ptr arcProc		PACKED_P;
    Ptr polyProc	PACKED_P;
    Ptr rgnProc		PACKED_P;
    Ptr bitsProc	PACKED_P;
    Ptr commentProc	PACKED_P;
    Ptr txMeasProc	PACKED_P;
    Ptr getPicProc	PACKED_P;
    Ptr putPicProc	PACKED_P;
    Ptr opcodeProc	PACKED_P;
    Ptr newProc1Proc	PACKED_P;
    Ptr newProc2Proc	PACKED_P;
    Ptr newProc3Proc	PACKED_P;
    Ptr newProc4Proc	PACKED_P;
    Ptr newProc5Proc	PACKED_P;
    Ptr newProc6Proc	PACKED_P;
} CQDProcs, *CQDProcsPtr;

typedef struct {
    Ptr baseAddr	PACKED_P;
    INTEGER rowBytes	PACKED;
    Rect bounds	PACKED;
    INTEGER pmVersion	PACKED;
    INTEGER packType	PACKED;
    LONGINT packSize	PACKED;
    Fixed hRes	PACKED;
    Fixed vRes	PACKED;
    INTEGER pixelType	PACKED;
    INTEGER pixelSize	PACKED;
    INTEGER cmpCount	PACKED;
    INTEGER cmpSize	PACKED;
    LONGINT planeBytes	PACKED;
    CTabHandle pmTable	PACKED_P;
    LONGINT pmReserved	PACKED;
} PixMap, *PixMapPtr;
typedef struct { PixMapPtr p PACKED_P; } HIDDEN_PixMapPtr;
typedef HIDDEN_PixMapPtr *PixMapHandle;
typedef struct { PixMapHandle p PACKED_P; } HIDDEN_PixMapHandle;

enum pixmap_pixel_types
{
  chunky_pixel_type,
  chunky_planar_pixel_type,
  planar_pixel_type,
};

#define ROWMASK	0x1FFF

typedef struct {
    INTEGER patType	PACKED;
    PixMapHandle patMap	PACKED_P;
    Handle patData	PACKED_P;
    Handle patXData	PACKED_P;
    INTEGER patXValid	PACKED;
    Handle patXMap	PACKED_P;
    Pattern pat1Data	PACKED;
} PixPat, *PixPatPtr;
typedef struct { PixPatPtr p PACKED_P; } HIDDEN_PixPatPtr;
typedef HIDDEN_PixPatPtr *PixPatHandle;
typedef struct { PixPatHandle p PACKED_P; } HIDDEN_PixPatHandle;

typedef struct {
    INTEGER device	PACKED;
    PixMapHandle portPixMap	PACKED_P;
    INTEGER portVersion	PACKED;
    Handle grafVars	PACKED_P;
    INTEGER chExtra	PACKED;
    INTEGER pnLocHFrac	PACKED;
    Rect portRect	PACKED;
    RgnHandle visRgn	PACKED_P;
    RgnHandle clipRgn	PACKED_P;
    PixPatHandle bkPixPat	PACKED_P;
    RGBColor rgbFgColor	PACKED;
    RGBColor rgbBkColor	PACKED;
    Point pnLoc	PACKED;
    Point pnSize	PACKED;
    INTEGER pnMode	PACKED;
    PixPatHandle pnPixPat	PACKED_P;
    PixPatHandle fillPixPat	PACKED_P;
    INTEGER pnVis	PACKED;
    INTEGER txFont	PACKED;
    Style txFace	PACKED;
    Byte filler		PACKED;
    INTEGER txMode	PACKED;
    INTEGER txSize	PACKED;
    Fixed spExtra	PACKED;
    LONGINT fgColor	PACKED;
    LONGINT bkColor	PACKED;
    INTEGER colrBit	PACKED;
    INTEGER patStretch	PACKED;
    Handle picSave	PACKED_P;
    Handle rgnSave	PACKED_P;
    Handle polySave	PACKED_P;
    CQDProcsPtr grafProcs	PACKED_P;
} CGrafPort, *CGrafPtr;

typedef struct { CGrafPtr p PACKED_P; } HIDDEN_CGrafPtr;

typedef struct {
    INTEGER crsrType	PACKED;
    PixMapHandle crsrMap	PACKED_P;
    Handle crsrData	PACKED_P;
    Handle crsrXData	PACKED_P;
    INTEGER crsrXValid	PACKED;
    Handle crsrXHandle	PACKED_P;
    Bits16 crsr1Data	PACKED;
    Bits16 crsrMask	PACKED;
    Point crsrHotSpot	PACKED;
    LONGINT crsrXTable	PACKED;
    LONGINT crsrID	PACKED;
} CCrsr, *CCrsrPtr;
typedef struct { CCrsrPtr p PACKED_P; } HIDDEN_CCrsrPtr;
typedef HIDDEN_CCrsrPtr *CCrsrHandle;

typedef struct {
    uint16 red	PACKED;
    uint16 green	PACKED;
    uint16 blue	PACKED;
    int32 matchData	PACKED;
} MatchRec;

typedef struct { HIDDEN_GrafPtr *p PACKED_P; } HIDDEN_GrafPtr_Ptr;
typedef struct { Byte *p PACKED_P; } HIDDEN_BytePtr;

#define thePort		(STARH(STARH((HIDDEN_GrafPtr_Ptr *) (long) SYN68K_TO_US(a5))))
#define thePortX	((*STARH((HIDDEN_GrafPtr_Ptr *) (long) SYN68K_TO_US(a5))).p)
#define white		(STARH((HIDDEN_BytePtr *)(long) SYN68K_TO_US(a5)) -8)
#define black		(STARH((HIDDEN_BytePtr *)(long) SYN68K_TO_US(a5)) -16)
#define gray		(STARH((HIDDEN_BytePtr *)(long) SYN68K_TO_US(a5)) -24)
#define ltGray		(STARH((HIDDEN_BytePtr *)(long) SYN68K_TO_US(a5)) -32)
#define dkGray		(STARH((HIDDEN_BytePtr *)(long) SYN68K_TO_US(a5)) -40)
#define arrowX		(* (Cursor  *) (STARH((HIDDEN_BytePtr *)(long) SYN68K_TO_US(a5))-108))
#define screenBitsX	(* (BitMap  *) (STARH((HIDDEN_BytePtr *)(long) SYN68K_TO_US(a5))-122))
#define randSeed	CL(* (LONGINT *) (STARH((HIDDEN_BytePtr *)(long) SYN68K_TO_US(a5))-126))
#define randSeedX	((* (LONGINT *) (STARH((HIDDEN_BytePtr *)(long) SYN68K_TO_US(a5))-126)))

#if !defined (RndSeed_L)
extern HIDDEN_LONGINT RndSeed_L;
extern Byte HiliteMode;	/* not really supported in ROMlib-V1.0 */
extern RGBColor HiliteRGB;	/* not really supported in ROMlib-V1.0 */
extern HIDDEN_ProcPtr 	JInitCrsr_H;

extern HIDDEN_ProcPtr JHideCursor_H;
extern HIDDEN_ProcPtr JShowCursor_H;
extern HIDDEN_ProcPtr JShieldCursor_H;
extern HIDDEN_ProcPtr JSetCrsr_H;
extern HIDDEN_ProcPtr JCrsrObscure_H;
extern HIDDEN_ProcPtr JUnknown574_H;

extern HIDDEN_Ptr 	ScrnBase_H;
extern HIDDEN_ProcPtr	JCrsrTask_H;
extern HIDDEN_Ptr	Key1Trans_H;
extern HIDDEN_Ptr	Key2Trans_H;
extern INTEGER 	ScrVRes;
extern INTEGER 	ScrHRes;
extern INTEGER 	ScreenRow;
extern Point 	MouseLocation;
extern Point	MouseLocation2;
extern BOOLEAN 	CrsrVis;
extern Byte 	CrsrBusy;
extern INTEGER 	CrsrState;
extern LONGINT 	mousemask;
extern LONGINT 	mouseoffset;
extern Byte 	HiliteMode;
#endif

#define RndSeed	(RndSeed_L.l)
#define JInitCrsr	(JInitCrsr_H.p)
#define JHideCursor	(JHideCursor_H.p)
#define JShowCursor	(JShowCursor_H.p)
#define JShieldCursor	(JShieldCursor_H.p)
#define JSetCrsr	(JSetCrsr_H.p)
#define JCrsrObscure	(JCrsrObscure_H.p)
#define JUnknown574	(JUnknown574_H.p)
#define JCrsrTask	(JCrsrTask_H.p)
#define ScrnBase	(ScrnBase_H.p)
#define Key1Trans	(Key1Trans_H.p)
#define Key2Trans	(Key2Trans_H.p)

extern trap void C_CopyBits (BitMap *src_bitmap, BitMap *dst_bitmap,
				    const Rect *src_rect, const Rect *dst_rect,
				    INTEGER mode, RgnHandle mask);

extern trap void C_ScrollRect( Rect *rp, INTEGER dh, INTEGER dv, 
 RgnHandle updatergn );
extern pascal trap void C_ForeColor( LONGINT c ); 
extern pascal trap void C_BackColor( LONGINT c ); 
extern pascal trap void C_ColorBit( INTEGER b ); 
extern pascal trap void C_SetCursor( Cursor *cp ); 
extern pascal trap void C_InitCursor( void  ); 
extern pascal trap void C_HideCursor( void  ); 
extern pascal trap void C_ShowCursor( void  ); 
extern pascal trap void C_ObscureCursor( void  ); 
extern pascal trap void C_ShieldCursor( Rect *rp, Point p ); 
extern pascal trap void C_InitGraf( Ptr gp ); 
extern pascal trap void C_SetPort( GrafPtr p ); 
extern pascal trap void C_InitPort( GrafPtr p ); 
extern pascal trap void C_OpenPort( GrafPtr p ); 
extern pascal trap void C_ClosePort( GrafPtr p ); 
extern pascal trap void C_GetPort( HIDDEN_GrafPtr *pp ); 
extern pascal trap void C_GrafDevice( INTEGER d ); 
extern pascal trap void C_SetPortBits( BitMap *bm ); 
extern pascal trap void C_PortSize( INTEGER w, INTEGER h ); 
extern pascal trap void C_MovePortTo( INTEGER lg, INTEGER tg ); 
extern pascal trap void C_SetOrigin( INTEGER h, INTEGER v ); 
extern pascal trap void C_SetClip( RgnHandle r ); 
extern pascal trap void C_GetClip( RgnHandle r ); 
extern pascal trap void C_ClipRect( Rect *r ); 
extern pascal trap void C_BackPat( Pattern pp ); 
extern pascal trap void C_SeedFill( Ptr srcp, Ptr dstp, 
 INTEGER srcr, INTEGER dstr, INTEGER height, INTEGER width, 
 INTEGER seedh, INTEGER seedv );
extern pascal trap void C_CalcMask( Ptr srcp, Ptr dstp, 
 INTEGER srcr, INTEGER dstr, INTEGER height, INTEGER width ); 
extern pascal trap void C_CopyMask( BitMap *srcbp, 
 BitMap *mskbp, BitMap *dstbp, Rect *srcrp, Rect *
 mskrp, Rect *dstrp ); 
extern a0trap INTEGER *GetMaskTable( void  ); 
extern pascal trap void C_OpColor( RGBColor *colorp ); 
extern pascal trap void C_RGBBackColor( RGBColor *colorp ); 
extern pascal trap void C_PenPixPat( PixPatHandle ph ); 
extern pascal trap void C_HiliteColor( RGBColor *colorp ); 
extern pascal trap void C_CharExtra( Fixed Extra ); 
extern pascal trap void C_BackPixPat( PixPatHandle ph ); 
extern pascal trap void C_RGBForeColor( RGBColor *colorp ); 
extern pascal trap void C_MakeRGBPat( PixPatHandle ph, 
 RGBColor *colorp ); 
extern pascal trap void C_DisposPixPat( PixPatHandle pph ); 
extern pascal trap INTEGER C_Random( void  ); 
extern pascal trap BOOLEAN C_GetPixel( INTEGER h, INTEGER v ); 
extern pascal trap void C_StuffHex( register Ptr p, StringPtr s ); 
extern pascal trap void C_ScalePt( Point *pt, Rect *srcr, Rect *dstr ); 
extern pascal trap void C_MapPt( Point *pt, Rect *srcr, Rect *dstr ); 
extern pascal trap void C_MapRect( Rect *r, Rect *srcr, Rect *dstr ); 
extern pascal trap void C_MapRgn( RgnHandle rh, Rect *srcr, Rect *dstr ); 
extern pascal trap void C_MapPoly( PolyHandle poly, Rect *srcr, 
 Rect *dstr ); 
extern pascal trap void C_HidePen( void  ); 
extern pascal trap void C_ShowPen( void  ); 
extern pascal trap void C_GetPen( Point *ptp ); 
extern pascal trap void C_GetPenState( PenState *ps ); 
extern pascal trap void C_SetPenState( PenState *ps ); 
extern pascal trap void C_PenSize( INTEGER w, INTEGER h ); 
extern pascal trap void C_PenMode( INTEGER m ); 
extern pascal trap void C_PenPat( Pattern pp ); 
extern pascal trap void C_PenNormal( void  ); 
extern pascal trap void C_MoveTo( INTEGER h, INTEGER v ); 
extern pascal trap void C_Move( INTEGER dh, INTEGER dv ); 
extern pascal trap void C_LineTo( INTEGER h, INTEGER v ); 
extern pascal trap void C_Line( INTEGER dh, INTEGER dv ); 
extern pascal trap void C_DrawPicture( PicHandle pic, Rect *destrp ); 
extern pascal trap PicHandle C_OpenPicture( Rect *pf ); 
extern pascal trap void C_ClosePicture( void  ); 
extern pascal trap void C_PicComment( INTEGER kind, INTEGER size, 
 Handle hand ); 
extern pascal trap void C_ReadComment( INTEGER kind, INTEGER size, 
 Handle hand ); 
extern pascal trap void C_KillPicture( PicHandle pic ); 
extern pascal trap void C_AddPt( Point src, Point *dst ); 
extern pascal trap void C_SubPt( Point src, Point *dst ); 
extern pascal trap void C_SetPt( Point *pt, INTEGER h, INTEGER v ); 
extern pascal trap BOOLEAN C_EqualPt( Point p1, Point p2 ); 
extern pascal trap void C_LocalToGlobal( Point *pt ); 
extern pascal trap void C_GlobalToLocal( Point *pt ); 
extern pascal trap PolyHandle C_OpenPoly( void  ); 
extern pascal trap void C_ClosePoly( void  ); 
extern pascal trap void C_KillPoly( PolyHandle poly ); 
extern pascal trap void C_OffsetPoly( PolyHandle poly, 
 INTEGER dh, INTEGER dv ); 
extern pascal trap void C_SetRect( Rect *r, INTEGER left, INTEGER top, 
 INTEGER right, INTEGER bottom ); 
extern pascal trap void C_OffsetRect( Rect *r, INTEGER dh, INTEGER dv ); 
extern pascal trap void C_InsetRect( Rect *r, INTEGER dh, INTEGER dv ); 




extern pascal trap BOOLEAN C_EmptyRect( Rect *r ); 
extern pascal trap BOOLEAN C_SectRect (const Rect *s1, const Rect *s2, Rect *dest);
extern pascal trap void C_UnionRect( Rect *s1, Rect *s2, Rect *dest ); 
extern pascal trap BOOLEAN C_PtInRect( Point p, Rect *r ); 
extern pascal trap void C_Pt2Rect( Point p1, Point p2, Rect *dest ); 
extern pascal trap void C_PtToAngle( Rect *rp, Point p, INTEGER *angle ); 
extern pascal trap BOOLEAN C_EqualRect( const Rect *r1, const Rect *r2 ); 
extern pascal trap RgnHandle C_NewRgn( void  ); 
extern pascal trap void C_OpenRgn( void  ); 
extern pascal trap void C_CopyRgn( RgnHandle s, RgnHandle d ); 
extern pascal trap void C_CloseRgn( RgnHandle rh ); 
extern pascal trap void C_DisposeRgn( RgnHandle rh ); 
extern pascal trap void C_SetEmptyRgn( RgnHandle rh ); 
extern pascal trap void C_SetRectRgn( RgnHandle rh, INTEGER left, 
 INTEGER top, INTEGER right, INTEGER bottom ); 
extern pascal trap void C_RectRgn( RgnHandle rh, Rect *rect ); 
extern pascal trap void C_OffsetRgn( RgnHandle rh, INTEGER dh, 
 INTEGER dv ); 
extern pascal trap BOOLEAN C_PtInRgn( Point p, RgnHandle rh ); 
extern pascal trap void C_InsetRgn( RgnHandle rh, INTEGER dh, INTEGER dv ); 
extern pascal trap void C_SectRgn( RgnHandle s1, RgnHandle s2, 
 RgnHandle dest ); 
extern pascal trap void C_UnionRgn( RgnHandle s1, RgnHandle s2, 
 RgnHandle dest ); 
extern pascal trap void C_DiffRgn( RgnHandle s1, RgnHandle s2, 
 RgnHandle dest ); 
extern pascal trap void C_XorRgn( RgnHandle s1, RgnHandle s2, 
 RgnHandle dest ); 
extern pascal trap BOOLEAN C_RectInRgn( Rect *rp, 
 RgnHandle rh ); 
extern pascal trap BOOLEAN C_EqualRgn( RgnHandle r1, RgnHandle r2 ); 
extern pascal trap BOOLEAN C_EmptyRgn( RgnHandle rh ); 
extern void ROMlib_printrgn( RgnHandle h ); 
extern void ROMlib_printpairs( INTEGER *p, LONGINT n ); 
extern pascal trap void C_FrameRect( Rect *r ); 
extern pascal trap void C_PaintRect( Rect *r ); 
extern pascal trap void C_EraseRect( Rect *r ); 
extern pascal trap void C_InvertRect( Rect *r ); 
extern pascal trap void C_FillRect( Rect *r, Pattern pat ); 
extern pascal trap void C_FrameOval( Rect *r ); 
extern pascal trap void C_PaintOval( Rect *r ); 
extern pascal trap void C_EraseOval( Rect *r ); 
extern pascal trap void C_InvertOval( Rect *r ); 
extern pascal trap void C_FillOval( Rect *r, Pattern pat ); 
extern pascal trap void C_FrameRoundRect( Rect *r, INTEGER ow, 
 INTEGER oh ); 
extern pascal trap void C_PaintRoundRect( Rect *r, INTEGER ow, 
 INTEGER oh ); 
extern pascal trap void C_EraseRoundRect( Rect *r, INTEGER ow, 
 INTEGER oh ); 
extern pascal trap void C_InvertRoundRect( Rect *r, INTEGER ow, 
 INTEGER oh ); 
extern pascal trap void C_FillRoundRect( Rect *r, INTEGER ow, 
 INTEGER oh, Pattern pat ); 
extern pascal trap void C_FrameArc( Rect *r, INTEGER start, 
 INTEGER angle ); 
extern pascal trap void C_PaintArc( Rect *r, INTEGER start, 
 INTEGER angle ); 
extern pascal trap void C_EraseArc( Rect *r, INTEGER start, 
 INTEGER angle ); 
extern pascal trap void C_InvertArc( Rect *r, INTEGER start, 
 INTEGER angle ); 
extern pascal trap void C_FillArc( Rect *r, INTEGER start, 
 INTEGER angle, Pattern pat ); 
extern pascal trap void C_FrameRgn( RgnHandle rh ); 
extern pascal trap void C_PaintRgn( RgnHandle rh ); 
extern pascal trap void C_EraseRgn( RgnHandle rh ); 
extern pascal trap void C_InvertRgn( RgnHandle rh ); 
extern pascal trap void C_FillRgn( RgnHandle rh, Pattern pat ); 
extern pascal trap void C_FramePoly( PolyHandle poly ); 
extern pascal trap void C_PaintPoly( PolyHandle poly ); 
extern pascal trap void C_ErasePoly( PolyHandle poly ); 
extern pascal trap void C_InvertPoly( PolyHandle poly ); 
extern pascal trap void C_FillPoly( PolyHandle poly, Pattern pat ); 
extern pascal trap void C_SetStdProcs( QDProcs *procs ); 
extern pascal trap void C_StdArc( GrafVerb verb, Rect *r, 
 INTEGER starta, INTEGER arca ); 

extern pascal trap void C_StdBits (BitMap *srcbmp,
				   const Rect *srcrp, const Rect *dstrp,
				   INTEGER mode, RgnHandle mask);
extern void StdBitsPicSaveFlag (BitMap *srcbmp,
				const Rect *srcrp, const Rect *dstrp,
				INTEGER mode, RgnHandle mask, BOOLEAN savepic);


extern void ROMlib_printsegs( INTEGER *ip ); 
extern pascal trap void C_StdLine( Point p ); 
extern pascal trap void C_StdOval( GrafVerb v, Rect *rp ); 
extern pascal trap void C_StdComment( INTEGER kind, INTEGER size, 
 Handle hand ); 
extern pascal trap void C_StdGetPic( Ptr dp, INTEGER bc ); 
extern pascal trap void C_StdPutPic( Ptr sp, INTEGER bc ); 
extern pascal trap void C_StdPoly( GrafVerb verb, PolyHandle ph ); 
extern pascal trap void C_StdRRect( GrafVerb verb, Rect *r, 
 INTEGER width, INTEGER height ); 
extern pascal trap void C_StdRect( GrafVerb v, Rect *rp ); 
extern pascal trap void C_StdRgn( GrafVerb verb, RgnHandle rgn ); 
extern pascal trap void C_StdText( INTEGER n, Ptr textbufp, 
 Point num, Point den ); 
extern pascal trap INTEGER C_StdTxMeas( INTEGER n, Ptr p, 
 Point *nump, Point *denp, FontInfo *finfop ); 
extern INTEGER ROMlib_StdTxMeas( LONGINT n, Ptr p, 
 Point *nump, Point *denp, FontInfo *finfop ); 
extern pascal trap void C_MeasureText( INTEGER n, Ptr text, 
 Ptr chars ); 
extern pascal trap void C_TextFont( INTEGER f ); 
extern pascal trap void C_TextFace( INTEGER thef ); 
extern pascal trap void C_TextMode( INTEGER m ); 
extern pascal trap void C_TextSize( INTEGER s ); 
extern pascal trap void C_SpaceExtra( Fixed e ); 
extern pascal trap void C_DrawChar( CHAR thec ); 
extern pascal trap void C_DrawString( StringPtr s ); 
extern pascal trap void C_DrawText( Ptr tb, INTEGER fb, INTEGER bc ); 
extern pascal trap INTEGER C_CharWidth( CHAR thec ); 
extern pascal trap INTEGER C_StringWidth( StringPtr s ); 
extern pascal trap INTEGER C_TextWidth( Ptr tb, INTEGER fb, INTEGER bc ); 
extern pascal trap void C_GetFontInfo( FontInfo *ip ); 

extern pascal trap void C_GetCPixel( INTEGER h, INTEGER v, RGBColor *colorp ); 
extern pascal trap void C_SetCPixel( INTEGER h, INTEGER v,
						       RGBColor *colorp);

extern pascal trap void C_SeedCFill( BitMap *srcbp, BitMap *dstbp,
                Rect *srcrp, Rect *dstrp, INTEGER seedh, INTEGER seedv,
				       ProcPtr matchprocp, LONGINT matchdata);

extern pascal trap void C_CalcCMask( BitMap *srcbp, BitMap *dstbp,
    Rect *srcrp, Rect *dstrp, RGBColor *seedrgbp, ProcPtr matchprocp,
							    LONGINT matchdata);
extern pascal trap void C_IMVI_CopyDeepMask (
    BitMap *srcBits,
    BitMap *maskBits,
    BitMap *dstBits,
    Rect *srcRect,
    Rect *maskRect,
    Rect *dstRect,
    INTEGER mode,
    RgnHandle maskRgn);

#endif /* _QUICKDRAW_H_ */
