#if !defined (_QUICKDRAW_H_)
#define _QUICKDRAW_H_

/*
 * Copyright 1986 - 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: QuickDraw.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {

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

enum {
	blackColor = 33,
	whiteColor = 30,
	redColor = 205,
	greenColor = 341,
	blueColor = 409,
	cyanColor = 273,
	magentaColor = 137,
	yellowColor = 69
};

#define picLParen	0
#define picRParen	1

typedef enum {
	bold = 1,
	italic = 2,
	underline = 4,
	outline = 8,
	shadow = 16,
	condense = 32,
	extend = 64
} StyleItem;

typedef SignedByte Style;

struct Region { GUEST_STRUCT;
    GUEST< INTEGER> rgnSize;
    GUEST< Rect> rgnBBox;
};

typedef Region *RgnPtr;

typedef GUEST<RgnPtr> *RgnHandle;


struct BitMap {
    GUEST_STRUCT;
    GUEST< Ptr> baseAddr;
    GUEST< INTEGER> rowBytes;
    GUEST< Rect> bounds;
};

typedef Byte Pattern[8];
typedef INTEGER Bits16[16];

struct Cursor { GUEST_STRUCT;
    GUEST< Bits16> data;
    GUEST< Bits16> mask;
    GUEST< Point> hotSpot;
};

typedef Cursor *CursPtr;

typedef GUEST<CursPtr> *CursHandle;

typedef SignedByte GrafVerb;
enum {
  frame = 0,
  paint = 1,
  erase = 2,
  invert = 3,
  fill = 4
};

struct Polygon { GUEST_STRUCT;
    GUEST< INTEGER> polySize;
    GUEST< Rect> polyBBox;
    GUEST< Point[1]> polyPoints;
};

typedef Polygon *PolyPtr;

typedef GUEST<PolyPtr> *PolyHandle;

struct FontInfo { GUEST_STRUCT;
    GUEST< INTEGER> ascent;
    GUEST< INTEGER> descent;
    GUEST< INTEGER> widMax;
    GUEST< INTEGER> leading;
};

typedef pascal trap void (*textProc_t)(INTEGER bc, Ptr textb, Point num, Point den);
typedef pascal trap void (*lineProc_t)(Point drawto);
typedef pascal trap void (*rectProc_t)(GrafVerb verb, Rect *rp);
typedef pascal trap void (*rRectProc_t)(GrafVerb verb, Rect *rp, INTEGER ow,
                                      INTEGER oh);
typedef pascal trap void (*ovalProc_t)(GrafVerb verb, Rect *rp);
typedef pascal trap void (*arcProc_t)(GrafVerb verb, Rect *rp, INTEGER ang,
                                    INTEGER arc);
typedef pascal trap void (*polyProc_t)(GrafVerb verb, PolyHandle poly);
typedef pascal trap void (*rgnProc_t)(GrafVerb verb, RgnHandle rgn);
typedef pascal trap void (*bitsProc_t)(BitMap *srcb, Rect *srcr, Rect *dstr,
                                     INTEGER mod, RgnHandle mask);
typedef pascal trap void (*commentProc_t)(INTEGER kind, INTEGER size, Handle data);
typedef pascal trap INTEGER  (*txMeasProc_t)(INTEGER bc, Ptr texta, Point *numer,
                                           Point *denom, FontInfo *info);
typedef pascal trap void (*getPicProc_t)(Ptr data, INTEGER bc);
typedef pascal trap void (*putPicProc_t)(Ptr data, INTEGER bc);

struct QDProcs { GUEST_STRUCT;
    GUEST< textProc_t> textProc;
    GUEST< lineProc_t> lineProc;
    GUEST< rectProc_t> rectProc;
    GUEST< rRectProc_t> rRectProc;
    GUEST< ovalProc_t> ovalProc;
    GUEST< arcProc_t> arcProc;
    GUEST< polyProc_t> polyProc;
    GUEST< rgnProc_t> rgnProc;
    GUEST< bitsProc_t> bitsProc;
    GUEST< commentProc_t> commentProc;
    GUEST< txMeasProc_t> txMeasProc;
    GUEST< getPicProc_t> getPicProc;
    GUEST< putPicProc_t> putPicProc;
};

typedef QDProcs *QDProcsPtr;

struct GrafPort { GUEST_STRUCT;
    GUEST< INTEGER> device;
    GUEST< BitMap> portBits;
    GUEST< Rect> portRect;
    GUEST< RgnHandle> visRgn;
    GUEST< RgnHandle> clipRgn;
    GUEST< Pattern> bkPat;
    GUEST< Pattern> fillPat;
    GUEST< Point> pnLoc;
    GUEST< Point> pnSize;
    GUEST< INTEGER> pnMode;
    GUEST< Pattern> pnPat;
    GUEST< INTEGER> pnVis;
    GUEST< INTEGER> txFont;
    GUEST< Style> txFace;
    GUEST< Byte> filler;
    GUEST< INTEGER> txMode;
    GUEST< INTEGER> txSize;
    GUEST< Fixed> spExtra;
    GUEST< LONGINT> fgColor;
    GUEST< LONGINT> bkColor;
    GUEST< INTEGER> colrBit;
    GUEST< INTEGER> patStretch;
    GUEST< Handle> picSave;
    GUEST< Handle> rgnSave;
    GUEST< Handle> polySave;
    GUEST< QDProcsPtr> grafProcs;
};

typedef GrafPort *GrafPtr;


struct Picture { GUEST_STRUCT;
    GUEST< INTEGER> picSize;
    GUEST< Rect> picFrame;
};

typedef Picture *PicPtr;

typedef GUEST<PicPtr> *PicHandle;

struct PenState { GUEST_STRUCT;
    GUEST< Point> pnLoc;
    GUEST< Point> pnSize;
    GUEST< INTEGER> pnMode;
    GUEST< Pattern> pnPat;
};

/* IMV stuff is used when we parse Version 2 pictures, but the IMV calls
   are not supported in V1.0 */

typedef enum { blend=32,    addPin, addOver, subPin,
	       transparent, adMax,  subOver, adMin,
	       mask=64 } colormodes;

#define pHiliteBit	 0

#define defQDColors	127

struct RGBColor { GUEST_STRUCT;
    GUEST< unsigned short> red;
    GUEST< unsigned short> green;
    GUEST< unsigned short> blue;
};

struct HSVColor { GUEST_STRUCT;
    GUEST< SmallFract> hue;
    GUEST< SmallFract> saturation;
    GUEST< SmallFract> value;
};

struct HSLColor { GUEST_STRUCT;
    GUEST< SmallFract> hue;
    GUEST< SmallFract> saturation;
    GUEST< SmallFract> lightness;
};

struct CMYColor { GUEST_STRUCT;
    GUEST< SmallFract> cyan;
    GUEST< SmallFract> magenta;
    GUEST< SmallFract> yellow;
};

struct ColorSpec { GUEST_STRUCT;
    GUEST< INTEGER> value;
    GUEST< RGBColor> rgb;
};

struct NativeRGBColor { 
        unsigned short red;
        unsigned short green;
        unsigned short blue;
};
struct NativeColorSpec {
        INTEGER value;
        NativeRGBColor rgb;
};    

typedef ColorSpec cSpecArray[1];	/* can't use 0 */

typedef struct ColorTable { GUEST_STRUCT;
    GUEST< LONGINT> ctSeed;
    GUEST< unsigned short> ctFlags;
    GUEST< INTEGER> ctSize;
    GUEST< cSpecArray> ctTable;
} *CTabPtr;


typedef GUEST<CTabPtr> *CTabHandle;


typedef struct CQDProcs { GUEST_STRUCT;
    GUEST< Ptr> textProc;
    GUEST< Ptr> lineProc;
    GUEST< Ptr> rectProc;
    GUEST< Ptr> rRectProc;
    GUEST< Ptr> ovalProc;
    GUEST< Ptr> arcProc;
    GUEST< Ptr> polyProc;
    GUEST< Ptr> rgnProc;
    GUEST< Ptr> bitsProc;
    GUEST< Ptr> commentProc;
    GUEST< Ptr> txMeasProc;
    GUEST< Ptr> getPicProc;
    GUEST< Ptr> putPicProc;
    GUEST< Ptr> opcodeProc;
    GUEST< Ptr> newProc1Proc;
    GUEST< Ptr> newProc2Proc;
    GUEST< Ptr> newProc3Proc;
    GUEST< Ptr> newProc4Proc;
    GUEST< Ptr> newProc5Proc;
    GUEST< Ptr> newProc6Proc;
} *CQDProcsPtr;

typedef struct PixMap { GUEST_STRUCT;
    GUEST< Ptr> baseAddr;
    GUEST< INTEGER> rowBytes;
    GUEST< Rect> bounds;
    GUEST< INTEGER> pmVersion;
    GUEST< INTEGER> packType;
    GUEST< LONGINT> packSize;
    GUEST< Fixed> hRes;
    GUEST< Fixed> vRes;
    GUEST< INTEGER> pixelType;
    GUEST< INTEGER> pixelSize;
    GUEST< INTEGER> cmpCount;
    GUEST< INTEGER> cmpSize;
    GUEST< LONGINT> planeBytes;
    GUEST< CTabHandle> pmTable;
    GUEST< LONGINT> pmReserved;
} *PixMapPtr;

typedef GUEST<PixMapPtr> *PixMapHandle;


enum pixmap_pixel_types
{
  chunky_pixel_type,
  chunky_planar_pixel_type,
  planar_pixel_type,
};

#define ROWMASK	0x1FFF

typedef struct PixPat { GUEST_STRUCT;
    GUEST< INTEGER> patType;
    GUEST< PixMapHandle> patMap;
    GUEST< Handle> patData;
    GUEST< Handle> patXData;
    GUEST< INTEGER> patXValid;
    GUEST< Handle> patXMap;
    GUEST< Pattern> pat1Data;
} *PixPatPtr;

typedef GUEST<PixPatPtr> *PixPatHandle;


typedef struct CGrafPort { GUEST_STRUCT;
    GUEST< INTEGER> device;
    GUEST< PixMapHandle> portPixMap;
    GUEST< INTEGER> portVersion;
    GUEST< Handle> grafVars;
    GUEST< INTEGER> chExtra;
    GUEST< INTEGER> pnLocHFrac;
    GUEST< Rect> portRect;
    GUEST< RgnHandle> visRgn;
    GUEST< RgnHandle> clipRgn;
    GUEST< PixPatHandle> bkPixPat;
    GUEST< RGBColor> rgbFgColor;
    GUEST< RGBColor> rgbBkColor;
    GUEST< Point> pnLoc;
    GUEST< Point> pnSize;
    GUEST< INTEGER> pnMode;
    GUEST< PixPatHandle> pnPixPat;
    GUEST< PixPatHandle> fillPixPat;
    GUEST< INTEGER> pnVis;
    GUEST< INTEGER> txFont;
    GUEST< Style> txFace;
    GUEST< Byte> filler;
    GUEST< INTEGER> txMode;
    GUEST< INTEGER> txSize;
    GUEST< Fixed> spExtra;
    GUEST< LONGINT> fgColor;
    GUEST< LONGINT> bkColor;
    GUEST< INTEGER> colrBit;
    GUEST< INTEGER> patStretch;
    GUEST< Handle> picSave;
    GUEST< Handle> rgnSave;
    GUEST< Handle> polySave;
    GUEST< CQDProcsPtr> grafProcs;
} *CGrafPtr;



typedef struct CCrsr { GUEST_STRUCT;
    GUEST< INTEGER> crsrType;
    GUEST< PixMapHandle> crsrMap;
    GUEST< Handle> crsrData;
    GUEST< Handle> crsrXData;
    GUEST< INTEGER> crsrXValid;
    GUEST< Handle> crsrXHandle;
    GUEST< Bits16> crsr1Data;
    GUEST< Bits16> crsrMask;
    GUEST< Point> crsrHotSpot;
    GUEST< LONGINT> crsrXTable;
    GUEST< LONGINT> crsrID;
} *CCrsrPtr;

typedef GUEST<CCrsrPtr> *CCrsrHandle;

struct MatchRec { GUEST_STRUCT;
    GUEST< uint16> red;
    GUEST< uint16> green;
    GUEST< uint16> blue;
    GUEST< int32> matchData;
};

typedef Byte *BytePtr;


#define thePort		(STARH(STARH((GUEST<GUEST<GrafPtr>*> *) (long) SYN68K_TO_US(a5))))
#define thePortX	((*STARH((GUEST<GUEST<GrafPtr>*> *) (long) SYN68K_TO_US(a5))))
#define white		(STARH((GUEST<BytePtr> *)(long) SYN68K_TO_US(a5)) -8)
#define black		(STARH((GUEST<BytePtr> *)(long) SYN68K_TO_US(a5)) -16)
#define gray		(STARH((GUEST<BytePtr> *)(long) SYN68K_TO_US(a5)) -24)
#define ltGray		(STARH((GUEST<BytePtr> *)(long) SYN68K_TO_US(a5)) -32)
#define dkGray		(STARH((GUEST<BytePtr> *)(long) SYN68K_TO_US(a5)) -40)
#define arrowX		(* (Cursor  *) (STARH((GUEST<BytePtr> *)(long) SYN68K_TO_US(a5))-108))
#define screenBitsX	(* (BitMap  *) (STARH((GUEST<BytePtr> *)(long) SYN68K_TO_US(a5))-122))
#define randSeed	CL(* (GUEST<LONGINT> *) (STARH((GUEST<BytePtr> *)(long) SYN68K_TO_US(a5))-126))
#define randSeedX	((* (GUEST<LONGINT> *) (STARH((GUEST<BytePtr> *)(long) SYN68K_TO_US(a5))-126)))

#if 0
extern GUEST<LONGINT> RndSeed_L;
extern Byte HiliteMode;	/* not really supported in ROMlib-V1.0 */
extern RGBColor HiliteRGB;	/* not really supported in ROMlib-V1.0 */
extern GUEST<ProcPtr> 	JInitCrsr_H;

extern GUEST<ProcPtr> JHideCursor_H;
extern GUEST<ProcPtr> JShowCursor_H;
extern GUEST<ProcPtr> JShieldCursor_H;
extern GUEST<ProcPtr> JSetCrsr_H;
extern GUEST<ProcPtr> JCrsrObscure_H;
extern GUEST<ProcPtr> JUnknown574_H;

extern GUEST<Ptr> 	ScrnBase_H;
extern GUEST<ProcPtr>	JCrsrTask_H;
extern GUEST<Ptr>	Key1Trans_H;
extern GUEST<Ptr>	Key2Trans_H;
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

#if 0
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
#endif

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
extern pascal trap void C_GetPort( GUEST<GrafPtr> *pp ); 
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
extern pascal trap void C_ScalePt( GUEST<Point> *pt, Rect *srcr, Rect *dstr ); 
extern pascal trap void C_MapPt( GUEST<Point> *pt, Rect *srcr, Rect *dstr ); 
extern pascal trap void C_MapRect( Rect *r, Rect *srcr, Rect *dstr ); 
extern pascal trap void C_MapRgn( RgnHandle rh, Rect *srcr, Rect *dstr ); 
extern pascal trap void C_MapPoly( PolyHandle poly, Rect *srcr, 
 Rect *dstr ); 
extern pascal trap void C_HidePen( void  ); 
extern pascal trap void C_ShowPen( void  ); 
extern pascal trap void C_GetPen( GUEST<Point> *ptp ); 
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
extern pascal trap void C_AddPt( Point src, GUEST<Point> *dst ); 
extern pascal trap void C_SubPt( Point src, GUEST<Point> *dst ); 
extern pascal trap void C_SetPt( GUEST<Point> *pt, INTEGER h, INTEGER v ); 
extern pascal trap BOOLEAN C_EqualPt( Point p1, Point p2 ); 
extern pascal trap void C_LocalToGlobal( GUEST<Point> *pt ); 
extern pascal trap void C_GlobalToLocal( GUEST<Point> *pt ); 
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
extern pascal trap void C_PtToAngle( Rect *rp, Point p, GUEST<INTEGER> *angle ); 
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
}
#endif /* _QUICKDRAW_H_ */
