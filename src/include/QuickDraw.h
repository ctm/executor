#if !defined(_QUICKDRAW_H_)
#define _QUICKDRAW_H_

/*
 * Copyright 1986 - 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{

enum
{
    grafSize = 206
}; /* number of bytes InitGraf needs */

enum
{
    srcCopy = 0,
    srcOr = 1,
    srcXor = 2,
    srcBic = 3,
    notSrcCopy = 4,
    notSrcOr = 5,
    notSrcXor = 6,
    notSrcBic = 7,
};

enum
{
    patCopy = 8,
    patOr = 9,
    patXor = 10,
    patBic = 11,
    notPatCopy = 12,
    notPatOr = 13,
    notPatXor = 14,
    notPatBic = 15,
};

enum
{
    grayishTextOr = 49,
};

enum
{
    hilite = 50,
};

enum
{
    blackColor = 33,
    whiteColor = 30,
    redColor = 205,
    greenColor = 341,
    blueColor = 409,
    cyanColor = 273,
    magentaColor = 137,
    yellowColor = 69
};

enum
{
    picLParen = 0,
    picRParen = 1,
};

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

struct Region
{
    GUEST_STRUCT;
    GUEST<INTEGER> rgnSize;
    GUEST<Rect> rgnBBox;
};

typedef Region *RgnPtr;

typedef GUEST<RgnPtr> *RgnHandle;

struct BitMap
{
    GUEST_STRUCT;
    GUEST<Ptr> baseAddr;
    GUEST<INTEGER> rowBytes;
    GUEST<Rect> bounds;
};

typedef Byte Pattern[8];
typedef INTEGER Bits16[16];

struct Cursor
{
    GUEST_STRUCT;
    GUEST<Bits16> data;
    GUEST<Bits16> mask;
    GUEST<Point> hotSpot;
};

typedef Cursor *CursPtr;

typedef GUEST<CursPtr> *CursHandle;

typedef SignedByte GrafVerb;
enum
{
    frame = 0,
    paint = 1,
    erase = 2,
    invert = 3,
    fill = 4
};

struct Polygon
{
    GUEST_STRUCT;
    GUEST<INTEGER> polySize;
    GUEST<Rect> polyBBox;
    GUEST<Point[1]> polyPoints;
};

typedef Polygon *PolyPtr;

typedef GUEST<PolyPtr> *PolyHandle;

struct FontInfo
{
    GUEST_STRUCT;
    GUEST<INTEGER> ascent;
    GUEST<INTEGER> descent;
    GUEST<INTEGER> widMax;
    GUEST<INTEGER> leading;
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
typedef pascal trap INTEGER (*txMeasProc_t)(INTEGER bc, Ptr texta, GUEST<Point> *numer,
                                            GUEST<Point> *denom, FontInfo *info);
typedef pascal trap void (*getPicProc_t)(Ptr data, INTEGER bc);
typedef pascal trap void (*putPicProc_t)(Ptr data, INTEGER bc);

struct QDProcs
{
    GUEST_STRUCT;
    GUEST<textProc_t> textProc;
    GUEST<lineProc_t> lineProc;
    GUEST<rectProc_t> rectProc;
    GUEST<rRectProc_t> rRectProc;
    GUEST<ovalProc_t> ovalProc;
    GUEST<arcProc_t> arcProc;
    GUEST<polyProc_t> polyProc;
    GUEST<rgnProc_t> rgnProc;
    GUEST<bitsProc_t> bitsProc;
    GUEST<commentProc_t> commentProc;
    GUEST<txMeasProc_t> txMeasProc;
    GUEST<getPicProc_t> getPicProc;
    GUEST<putPicProc_t> putPicProc;
};

typedef QDProcs *QDProcsPtr;

struct GrafPort
{
    GUEST_STRUCT;
    GUEST<INTEGER> device;
    GUEST<BitMap> portBits;
    GUEST<Rect> portRect;
    GUEST<RgnHandle> visRgn;
    GUEST<RgnHandle> clipRgn;
    GUEST<Pattern> bkPat;
    GUEST<Pattern> fillPat;
    GUEST<Point> pnLoc;
    GUEST<Point> pnSize;
    GUEST<INTEGER> pnMode;
    GUEST<Pattern> pnPat;
    GUEST<INTEGER> pnVis;
    GUEST<INTEGER> txFont;
    GUEST<Style> txFace;
    GUEST<Byte> filler;
    GUEST<INTEGER> txMode;
    GUEST<INTEGER> txSize;
    GUEST<Fixed> spExtra;
    GUEST<LONGINT> fgColor;
    GUEST<LONGINT> bkColor;
    GUEST<INTEGER> colrBit;
    GUEST<INTEGER> patStretch;
    GUEST<Handle> picSave;
    GUEST<Handle> rgnSave;
    GUEST<Handle> polySave;
    GUEST<QDProcsPtr> grafProcs;
};

typedef GrafPort *GrafPtr;

struct Picture
{
    GUEST_STRUCT;
    GUEST<INTEGER> picSize;
    GUEST<Rect> picFrame;
};

typedef Picture *PicPtr;

typedef GUEST<PicPtr> *PicHandle;

struct PenState
{
    GUEST_STRUCT;
    GUEST<Point> pnLoc;
    GUEST<Point> pnSize;
    GUEST<INTEGER> pnMode;
    GUEST<Pattern> pnPat;
};

/* IMV stuff is used when we parse Version 2 pictures, but the IMV calls
   are not supported in V1.0 */

typedef enum { blend = 32,
               addPin,
               addOver,
               subPin,
               transparent,
               adMax,
               subOver,
               adMin,
               mask = 64 } colormodes;

enum
{
    pHiliteBit = 0,
};

enum
{
    defQDColors = 127,
};

struct RGBColor
{
    GUEST_STRUCT;
    GUEST<unsigned short> red;
    GUEST<unsigned short> green;
    GUEST<unsigned short> blue;
};

struct HSVColor
{
    GUEST_STRUCT;
    GUEST<SmallFract> hue;
    GUEST<SmallFract> saturation;
    GUEST<SmallFract> value;
};

struct HSLColor
{
    GUEST_STRUCT;
    GUEST<SmallFract> hue;
    GUEST<SmallFract> saturation;
    GUEST<SmallFract> lightness;
};

struct CMYColor
{
    GUEST_STRUCT;
    GUEST<SmallFract> cyan;
    GUEST<SmallFract> magenta;
    GUEST<SmallFract> yellow;
};

struct ColorSpec
{
    GUEST_STRUCT;
    GUEST<INTEGER> value;
    GUEST<RGBColor> rgb;
};

struct NativeRGBColor
{
    unsigned short red;
    unsigned short green;
    unsigned short blue;
};
struct NativeColorSpec
{
    INTEGER value;
    NativeRGBColor rgb;
};

typedef ColorSpec cSpecArray[1]; /* can't use 0 */

typedef struct ColorTable
{
    GUEST_STRUCT;
    GUEST<LONGINT> ctSeed;
    GUEST<unsigned short> ctFlags;
    GUEST<INTEGER> ctSize;
    GUEST<cSpecArray> ctTable;
} * CTabPtr;

typedef GUEST<CTabPtr> *CTabHandle;

typedef struct CQDProcs
{
    GUEST_STRUCT;
    GUEST<Ptr> textProc;
    GUEST<Ptr> lineProc;
    GUEST<Ptr> rectProc;
    GUEST<Ptr> rRectProc;
    GUEST<Ptr> ovalProc;
    GUEST<Ptr> arcProc;
    GUEST<Ptr> polyProc;
    GUEST<Ptr> rgnProc;
    GUEST<Ptr> bitsProc;
    GUEST<Ptr> commentProc;
    GUEST<Ptr> txMeasProc;
    GUEST<Ptr> getPicProc;
    GUEST<Ptr> putPicProc;
    GUEST<Ptr> opcodeProc;
    GUEST<Ptr> newProc1Proc;
    GUEST<Ptr> newProc2Proc;
    GUEST<Ptr> newProc3Proc;
    GUEST<Ptr> newProc4Proc;
    GUEST<Ptr> newProc5Proc;
    GUEST<Ptr> newProc6Proc;
} * CQDProcsPtr;

typedef struct PixMap
{
    GUEST_STRUCT;
    GUEST<Ptr> baseAddr;
    GUEST<INTEGER> rowBytes;
    GUEST<Rect> bounds;
    GUEST<INTEGER> pmVersion;
    GUEST<INTEGER> packType;
    GUEST<LONGINT> packSize;
    GUEST<Fixed> hRes;
    GUEST<Fixed> vRes;
    GUEST<INTEGER> pixelType;
    GUEST<INTEGER> pixelSize;
    GUEST<INTEGER> cmpCount;
    GUEST<INTEGER> cmpSize;
    GUEST<LONGINT> planeBytes;
    GUEST<CTabHandle> pmTable;
    GUEST<LONGINT> pmReserved;
} * PixMapPtr;

typedef GUEST<PixMapPtr> *PixMapHandle;

enum pixmap_pixel_types
{
    chunky_pixel_type,
    chunky_planar_pixel_type,
    planar_pixel_type,
};

enum
{
    ROWMASK = 0x1FFF,
};

typedef struct PixPat
{
    GUEST_STRUCT;
    GUEST<INTEGER> patType;
    GUEST<PixMapHandle> patMap;
    GUEST<Handle> patData;
    GUEST<Handle> patXData;
    GUEST<INTEGER> patXValid;
    GUEST<Handle> patXMap;
    GUEST<Pattern> pat1Data;
} * PixPatPtr;

typedef GUEST<PixPatPtr> *PixPatHandle;

typedef struct CGrafPort
{
    GUEST_STRUCT;
    GUEST<INTEGER> device;
    GUEST<PixMapHandle> portPixMap;
    GUEST<INTEGER> portVersion;
    GUEST<Handle> grafVars;
    GUEST<INTEGER> chExtra;
    GUEST<INTEGER> pnLocHFrac;
    GUEST<Rect> portRect;
    GUEST<RgnHandle> visRgn;
    GUEST<RgnHandle> clipRgn;
    GUEST<PixPatHandle> bkPixPat;
    GUEST<RGBColor> rgbFgColor;
    GUEST<RGBColor> rgbBkColor;
    GUEST<Point> pnLoc;
    GUEST<Point> pnSize;
    GUEST<INTEGER> pnMode;
    GUEST<PixPatHandle> pnPixPat;
    GUEST<PixPatHandle> fillPixPat;
    GUEST<INTEGER> pnVis;
    GUEST<INTEGER> txFont;
    GUEST<Style> txFace;
    GUEST<Byte> filler;
    GUEST<INTEGER> txMode;
    GUEST<INTEGER> txSize;
    GUEST<Fixed> spExtra;
    GUEST<LONGINT> fgColor;
    GUEST<LONGINT> bkColor;
    GUEST<INTEGER> colrBit;
    GUEST<INTEGER> patStretch;
    GUEST<Handle> picSave;
    GUEST<Handle> rgnSave;
    GUEST<Handle> polySave;
    GUEST<CQDProcsPtr> grafProcs;
} * CGrafPtr;

typedef struct CCrsr
{
    GUEST_STRUCT;
    GUEST<INTEGER> crsrType;
    GUEST<PixMapHandle> crsrMap;
    GUEST<Handle> crsrData;
    GUEST<Handle> crsrXData;
    GUEST<INTEGER> crsrXValid;
    GUEST<Handle> crsrXHandle;
    GUEST<Bits16> crsr1Data;
    GUEST<Bits16> crsrMask;
    GUEST<Point> crsrHotSpot;
    GUEST<LONGINT> crsrXTable;
    GUEST<LONGINT> crsrID;
} * CCrsrPtr;

typedef GUEST<CCrsrPtr> *CCrsrHandle;

struct MatchRec
{
    GUEST_STRUCT;
    GUEST<uint16_t> red;
    GUEST<uint16_t> green;
    GUEST<uint16_t> blue;
    GUEST<int32_t> matchData;
};

typedef Byte *BytePtr;

#define thePort (STARH(STARH((GUEST<GUEST<GrafPtr> *> *)SYN68K_TO_US(EM_A5))))
#define thePortX ((*STARH((GUEST<GUEST<GrafPtr> *> *)SYN68K_TO_US(EM_A5))))
#define white (STARH((GUEST<BytePtr> *)SYN68K_TO_US(EM_A5)) - 8)
#define black (STARH((GUEST<BytePtr> *)SYN68K_TO_US(EM_A5)) - 16)
#define gray (STARH((GUEST<BytePtr> *)SYN68K_TO_US(EM_A5)) - 24)
#define ltGray (STARH((GUEST<BytePtr> *)SYN68K_TO_US(EM_A5)) - 32)
#define dkGray (STARH((GUEST<BytePtr> *)SYN68K_TO_US(EM_A5)) - 40)
#define arrowX (*(Cursor *)(STARH((GUEST<BytePtr> *)SYN68K_TO_US(EM_A5)) - 108))
#define screenBitsX (*(BitMap *)(STARH((GUEST<BytePtr> *)SYN68K_TO_US(EM_A5)) - 122))
#define randSeed CL(*(GUEST<LONGINT> *)(STARH((GUEST<BytePtr> *)SYN68K_TO_US(EM_A5)) - 126))
#define randSeedX ((*(GUEST<LONGINT> *)(STARH((GUEST<BytePtr> *)SYN68K_TO_US(EM_A5)) - 126)))

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
enum
{
    RndSeed = (RndSeed_L.l),
    JInitCrsr = (JInitCrsr_H.p),
    JHideCursor = (JHideCursor_H.p),
    JShowCursor = (JShowCursor_H.p),
    JShieldCursor = (JShieldCursor_H.p),
    JSetCrsr = (JSetCrsr_H.p),
    JCrsrObscure = (JCrsrObscure_H.p),
    JUnknown574 = (JUnknown574_H.p),
    JCrsrTask = (JCrsrTask_H.p),
    ScrnBase = (ScrnBase_H.p),
    Key1Trans = (Key1Trans_H.p),
    Key2Trans = (Key2Trans_H.p),
};
#endif

extern trap void C_CopyBits(BitMap *src_bitmap, BitMap *dst_bitmap,
                            const Rect *src_rect, const Rect *dst_rect,
                            INTEGER mode, RgnHandle mask);
PASCAL_TRAP(CopyBits, 0xA8EC);

extern trap void C_ScrollRect(Rect *rp, INTEGER dh, INTEGER dv,
                              RgnHandle updatergn);
PASCAL_TRAP(ScrollRect, 0xA8EF);

extern pascal trap void C_ForeColor(LONGINT c);
PASCAL_TRAP(ForeColor, 0xA862);
extern pascal trap void C_BackColor(LONGINT c);
PASCAL_TRAP(BackColor, 0xA863);
extern pascal trap void C_ColorBit(INTEGER b);
PASCAL_TRAP(ColorBit, 0xA864);
extern pascal trap void C_SetCursor(Cursor *cp);
PASCAL_TRAP(SetCursor, 0xA851);
extern pascal trap void C_InitCursor(void);
PASCAL_TRAP(InitCursor, 0xA850);
extern pascal trap void C_HideCursor(void);
PASCAL_TRAP(HideCursor, 0xA852);
extern pascal trap void C_ShowCursor(void);
PASCAL_TRAP(ShowCursor, 0xA853);
extern pascal trap void C_ObscureCursor(void);
PASCAL_TRAP(ObscureCursor, 0xA856);
extern pascal trap void C_ShieldCursor(Rect *rp, Point p);
PASCAL_TRAP(ShieldCursor, 0xA855);
extern pascal trap void C_InitGraf(Ptr gp);
PASCAL_TRAP(InitGraf, 0xA86E);
extern pascal trap void C_SetPort(GrafPtr p);
PASCAL_TRAP(SetPort, 0xA873);
extern pascal trap void C_InitPort(GrafPtr p);
PASCAL_TRAP(InitPort, 0xA86D);
extern pascal trap void C_OpenPort(GrafPtr p);
PASCAL_TRAP(OpenPort, 0xA86F);
extern pascal trap void C_ClosePort(GrafPtr p);
PASCAL_TRAP(ClosePort, 0xA87D);
extern pascal trap void C_GetPort(GUEST<GrafPtr> *pp);
PASCAL_TRAP(GetPort, 0xA874);
extern pascal trap void C_GrafDevice(INTEGER d);
PASCAL_TRAP(GrafDevice, 0xA872);
extern pascal trap void C_SetPortBits(BitMap *bm);
PASCAL_TRAP(SetPortBits, 0xA875);
extern pascal trap void C_PortSize(INTEGER w, INTEGER h);
PASCAL_TRAP(PortSize, 0xA876);
extern pascal trap void C_MovePortTo(INTEGER lg, INTEGER tg);
PASCAL_TRAP(MovePortTo, 0xA877);
extern pascal trap void C_SetOrigin(INTEGER h, INTEGER v);
PASCAL_TRAP(SetOrigin, 0xA878);
extern pascal trap void C_SetClip(RgnHandle r);
PASCAL_TRAP(SetClip, 0xA879);
extern pascal trap void C_GetClip(RgnHandle r);
PASCAL_TRAP(GetClip, 0xA87A);
extern pascal trap void C_ClipRect(Rect *r);
PASCAL_TRAP(ClipRect, 0xA87B);
extern pascal trap void C_BackPat(Pattern pp);
PASCAL_TRAP(BackPat, 0xA87C);
extern pascal trap void C_SeedFill(Ptr srcp, Ptr dstp,
                                   INTEGER srcr, INTEGER dstr, INTEGER height, INTEGER width,
                                   INTEGER seedh, INTEGER seedv);
PASCAL_TRAP(SeedFill, 0xA839);
extern pascal trap void C_CalcMask(Ptr srcp, Ptr dstp,
                                   INTEGER srcr, INTEGER dstr, INTEGER height, INTEGER width);
PASCAL_TRAP(CalcMask, 0xA838);
extern pascal trap void C_CopyMask(BitMap *srcbp,
                                   BitMap *mskbp, BitMap *dstbp, Rect *srcrp, Rect *
                                                                                  mskrp,
                                   Rect *dstrp);
PASCAL_TRAP(CopyMask, 0xA817);
extern a0trap INTEGER *GetMaskTable(void);
extern pascal trap void C_CharExtra(Fixed Extra);
PASCAL_TRAP(CharExtra, 0xAA23);
extern pascal trap void C_MakeRGBPat(PixPatHandle ph,
                                     RGBColor *colorp);
PASCAL_TRAP(MakeRGBPat, 0xAA0D);
extern pascal trap INTEGER C_Random(void);
PASCAL_TRAP(Random, 0xA861);
extern pascal trap BOOLEAN C_GetPixel(INTEGER h, INTEGER v);
PASCAL_TRAP(GetPixel, 0xA865);
extern pascal trap void C_StuffHex(Ptr p, StringPtr s);
PASCAL_TRAP(StuffHex, 0xA866);
extern pascal trap void C_ScalePt(GUEST<Point> *pt, Rect *srcr, Rect *dstr);
PASCAL_TRAP(ScalePt, 0xA8F8);
extern pascal trap void C_MapPt(GUEST<Point> *pt, Rect *srcr, Rect *dstr);
PASCAL_TRAP(MapPt, 0xA8F9);
extern pascal trap void C_MapRect(Rect *r, Rect *srcr, Rect *dstr);
PASCAL_TRAP(MapRect, 0xA8FA);
extern pascal trap void C_MapRgn(RgnHandle rh, Rect *srcr, Rect *dstr);
PASCAL_TRAP(MapRgn, 0xA8FB);
extern pascal trap void C_MapPoly(PolyHandle poly, Rect *srcr,
                                  Rect *dstr);
PASCAL_TRAP(MapPoly, 0xA8FC);
extern pascal trap void C_HidePen(void);
PASCAL_TRAP(HidePen, 0xA896);
extern pascal trap void C_ShowPen(void);
PASCAL_TRAP(ShowPen, 0xA897);
extern pascal trap void C_GetPen(GUEST<Point> *ptp);
PASCAL_TRAP(GetPen, 0xA89A);
extern pascal trap void C_GetPenState(PenState *ps);
PASCAL_TRAP(GetPenState, 0xA898);
extern pascal trap void C_SetPenState(PenState *ps);
PASCAL_TRAP(SetPenState, 0xA899);
extern pascal trap void C_PenSize(INTEGER w, INTEGER h);
PASCAL_TRAP(PenSize, 0xA89B);
extern pascal trap void C_PenMode(INTEGER m);
PASCAL_TRAP(PenMode, 0xA89C);
extern pascal trap void C_PenPat(Pattern pp);
PASCAL_TRAP(PenPat, 0xA89D);
extern pascal trap void C_PenNormal(void);
PASCAL_TRAP(PenNormal, 0xA89E);
extern pascal trap void C_MoveTo(INTEGER h, INTEGER v);
PASCAL_TRAP(MoveTo, 0xA893);
extern pascal trap void C_Move(INTEGER dh, INTEGER dv);
PASCAL_TRAP(Move, 0xA894);
extern pascal trap void C_LineTo(INTEGER h, INTEGER v);
PASCAL_TRAP(LineTo, 0xA891);
extern pascal trap void C_Line(INTEGER dh, INTEGER dv);
PASCAL_TRAP(Line, 0xA892);
extern pascal trap void C_DrawPicture(PicHandle pic, Rect *destrp);
PASCAL_TRAP(DrawPicture, 0xA8F6);
extern pascal trap PicHandle C_OpenPicture(Rect *pf);
PASCAL_TRAP(OpenPicture, 0xA8F3);
extern pascal trap void C_ClosePicture(void);
PASCAL_TRAP(ClosePicture, 0xA8F4);
extern pascal trap void C_PicComment(INTEGER kind, INTEGER size,
                                     Handle hand);
PASCAL_TRAP(PicComment, 0xA8F2);
extern pascal trap void C_ReadComment(INTEGER kind, INTEGER size,
                                      Handle hand);
PASCAL_FUNCTION(ReadComment);
extern pascal trap void C_KillPicture(PicHandle pic);
PASCAL_TRAP(KillPicture, 0xA8F5);
extern pascal trap void C_AddPt(Point src, GUEST<Point> *dst);
PASCAL_TRAP(AddPt, 0xA87E);
extern pascal trap void C_SubPt(Point src, GUEST<Point> *dst);
PASCAL_TRAP(SubPt, 0xA87F);
extern pascal trap void C_SetPt(GUEST<Point> *pt, INTEGER h, INTEGER v);
PASCAL_TRAP(SetPt, 0xA880);
extern pascal trap BOOLEAN C_EqualPt(Point p1, Point p2);
PASCAL_TRAP(EqualPt, 0xA881);
extern pascal trap void C_LocalToGlobal(GUEST<Point> *pt);
PASCAL_TRAP(LocalToGlobal, 0xA870);
extern pascal trap void C_GlobalToLocal(GUEST<Point> *pt);
PASCAL_TRAP(GlobalToLocal, 0xA871);
extern pascal trap PolyHandle C_OpenPoly(void);
PASCAL_TRAP(OpenPoly, 0xA8CB);
extern pascal trap void C_ClosePoly(void);
PASCAL_TRAP(ClosePoly, 0xA8CC);
extern pascal trap void C_KillPoly(PolyHandle poly);
PASCAL_TRAP(KillPoly, 0xA8CD);
extern pascal trap void C_OffsetPoly(PolyHandle poly,
                                     INTEGER dh, INTEGER dv);
PASCAL_TRAP(OffsetPoly, 0xA8CE);
extern pascal trap void C_SetRect(Rect *r, INTEGER left, INTEGER top,
                                  INTEGER right, INTEGER bottom);
PASCAL_TRAP(SetRect, 0xA8A7);
extern pascal trap void C_OffsetRect(Rect *r, INTEGER dh, INTEGER dv);
PASCAL_TRAP(OffsetRect, 0xA8A8);
extern pascal trap void C_InsetRect(Rect *r, INTEGER dh, INTEGER dv);
PASCAL_TRAP(InsetRect, 0xA8A9);

extern pascal trap BOOLEAN C_EmptyRect(Rect *r);
PASCAL_TRAP(EmptyRect, 0xA8AE);
extern pascal trap BOOLEAN C_SectRect(const Rect *s1, const Rect *s2, Rect *dest);
PASCAL_TRAP(SectRect, 0xA8AA);
extern pascal trap void C_UnionRect(Rect *s1, Rect *s2, Rect *dest);
PASCAL_TRAP(UnionRect, 0xA8AB);
extern pascal trap BOOLEAN C_PtInRect(Point p, Rect *r);
PASCAL_TRAP(PtInRect, 0xA8AD);
extern pascal trap void C_Pt2Rect(Point p1, Point p2, Rect *dest);
PASCAL_TRAP(Pt2Rect, 0xA8AC);
extern pascal trap void C_PtToAngle(Rect *rp, Point p, GUEST<INTEGER> *angle);
PASCAL_TRAP(PtToAngle, 0xA8C3);
extern pascal trap BOOLEAN C_EqualRect(const Rect *r1, const Rect *r2);
PASCAL_TRAP(EqualRect, 0xA8A6);
extern pascal trap RgnHandle C_NewRgn(void);
PASCAL_TRAP(NewRgn, 0xA8D8);
extern pascal trap void C_OpenRgn(void);
PASCAL_TRAP(OpenRgn, 0xA8DA);
extern pascal trap void C_CopyRgn(RgnHandle s, RgnHandle d);
PASCAL_TRAP(CopyRgn, 0xA8DC);
extern pascal trap void C_CloseRgn(RgnHandle rh);
PASCAL_TRAP(CloseRgn, 0xA8DB);
extern pascal trap void C_DisposeRgn(RgnHandle rh);
PASCAL_TRAP(DisposeRgn, 0xA8D9);
extern pascal trap void C_SetEmptyRgn(RgnHandle rh);
PASCAL_TRAP(SetEmptyRgn, 0xA8DD);
extern pascal trap void C_SetRectRgn(RgnHandle rh, INTEGER left,
                                     INTEGER top, INTEGER right, INTEGER bottom);
PASCAL_TRAP(SetRectRgn, 0xA8DE);
extern pascal trap void C_RectRgn(RgnHandle rh, Rect *rect);
PASCAL_TRAP(RectRgn, 0xA8DF);
extern pascal trap void C_OffsetRgn(RgnHandle rh, INTEGER dh,
                                    INTEGER dv);
PASCAL_TRAP(OffsetRgn, 0xA8E0);
extern pascal trap BOOLEAN C_PtInRgn(Point p, RgnHandle rh);
PASCAL_TRAP(PtInRgn, 0xA8E8);
extern pascal trap void C_InsetRgn(RgnHandle rh, INTEGER dh, INTEGER dv);
PASCAL_TRAP(InsetRgn, 0xA8E1);
extern pascal trap void C_SectRgn(RgnHandle s1, RgnHandle s2,
                                  RgnHandle dest);
PASCAL_TRAP(SectRgn, 0xA8E4);
extern pascal trap void C_UnionRgn(RgnHandle s1, RgnHandle s2,
                                   RgnHandle dest);
PASCAL_TRAP(UnionRgn, 0xA8E5);
extern pascal trap void C_DiffRgn(RgnHandle s1, RgnHandle s2,
                                  RgnHandle dest);
PASCAL_TRAP(DiffRgn, 0xA8E6);
extern pascal trap void C_XorRgn(RgnHandle s1, RgnHandle s2,
                                 RgnHandle dest);
PASCAL_TRAP(XorRgn, 0xA8E7);
extern pascal trap BOOLEAN C_RectInRgn(Rect *rp,
                                       RgnHandle rh);
PASCAL_TRAP(RectInRgn, 0xA8E9);
extern pascal trap BOOLEAN C_EqualRgn(RgnHandle r1, RgnHandle r2);
PASCAL_TRAP(EqualRgn, 0xA8E3);
extern pascal trap BOOLEAN C_EmptyRgn(RgnHandle rh);
PASCAL_TRAP(EmptyRgn, 0xA8E2);
extern void ROMlib_printrgn(RgnHandle h);
extern void ROMlib_printpairs(INTEGER *p, LONGINT n);
extern pascal trap void C_FrameRect(Rect *r);
PASCAL_TRAP(FrameRect, 0xA8A1);
extern pascal trap void C_PaintRect(Rect *r);
PASCAL_TRAP(PaintRect, 0xA8A2);
extern pascal trap void C_EraseRect(Rect *r);
PASCAL_TRAP(EraseRect, 0xA8A3);
extern pascal trap void C_InvertRect(Rect *r);
PASCAL_TRAP(InvertRect, 0xA8A4);
extern pascal trap void C_FillRect(Rect *r, Pattern pat);
PASCAL_TRAP(FillRect, 0xA8A5);
extern pascal trap void C_FrameOval(Rect *r);
PASCAL_TRAP(FrameOval, 0xA8B7);
extern pascal trap void C_PaintOval(Rect *r);
PASCAL_TRAP(PaintOval, 0xA8B8);
extern pascal trap void C_EraseOval(Rect *r);
PASCAL_TRAP(EraseOval, 0xA8B9);
extern pascal trap void C_InvertOval(Rect *r);
PASCAL_TRAP(InvertOval, 0xA8BA);
extern pascal trap void C_FillOval(Rect *r, Pattern pat);
PASCAL_TRAP(FillOval, 0xA8BB);
extern pascal trap void C_FrameRoundRect(Rect *r, INTEGER ow,
                                         INTEGER oh);
PASCAL_TRAP(FrameRoundRect, 0xA8B0);
extern pascal trap void C_PaintRoundRect(Rect *r, INTEGER ow,
                                         INTEGER oh);
PASCAL_TRAP(PaintRoundRect, 0xA8B1);
extern pascal trap void C_EraseRoundRect(Rect *r, INTEGER ow,
                                         INTEGER oh);
PASCAL_TRAP(EraseRoundRect, 0xA8B2);
extern pascal trap void C_InvertRoundRect(Rect *r, INTEGER ow,
                                          INTEGER oh);
PASCAL_TRAP(InvertRoundRect, 0xA8B3);
extern pascal trap void C_FillRoundRect(Rect *r, INTEGER ow,
                                        INTEGER oh, Pattern pat);
PASCAL_TRAP(FillRoundRect, 0xA8B4);
extern pascal trap void C_FrameArc(Rect *r, INTEGER start,
                                   INTEGER angle);
PASCAL_TRAP(FrameArc, 0xA8BE);
extern pascal trap void C_PaintArc(Rect *r, INTEGER start,
                                   INTEGER angle);
PASCAL_TRAP(PaintArc, 0xA8BF);
extern pascal trap void C_EraseArc(Rect *r, INTEGER start,
                                   INTEGER angle);
PASCAL_TRAP(EraseArc, 0xA8C0);
extern pascal trap void C_InvertArc(Rect *r, INTEGER start,
                                    INTEGER angle);
PASCAL_TRAP(InvertArc, 0xA8C1);
extern pascal trap void C_FillArc(Rect *r, INTEGER start,
                                  INTEGER angle, Pattern pat);
PASCAL_TRAP(FillArc, 0xA8C2);
extern pascal trap void C_FrameRgn(RgnHandle rh);
PASCAL_TRAP(FrameRgn, 0xA8D2);
extern pascal trap void C_PaintRgn(RgnHandle rh);
PASCAL_TRAP(PaintRgn, 0xA8D3);
extern pascal trap void C_EraseRgn(RgnHandle rh);
PASCAL_TRAP(EraseRgn, 0xA8D4);
extern pascal trap void C_InvertRgn(RgnHandle rh);
PASCAL_TRAP(InvertRgn, 0xA8D5);
extern pascal trap void C_FillRgn(RgnHandle rh, Pattern pat);
PASCAL_TRAP(FillRgn, 0xA8D6);
extern pascal trap void C_FramePoly(PolyHandle poly);
PASCAL_TRAP(FramePoly, 0xA8C6);
extern pascal trap void C_PaintPoly(PolyHandle poly);
PASCAL_TRAP(PaintPoly, 0xA8C7);
extern pascal trap void C_ErasePoly(PolyHandle poly);
PASCAL_TRAP(ErasePoly, 0xA8C8);
extern pascal trap void C_InvertPoly(PolyHandle poly);
PASCAL_TRAP(InvertPoly, 0xA8C9);
extern pascal trap void C_FillPoly(PolyHandle poly, Pattern pat);
PASCAL_TRAP(FillPoly, 0xA8CA);
extern pascal trap void C_SetStdProcs(QDProcs *procs);
PASCAL_TRAP(SetStdProcs, 0xA8EA);
extern pascal trap void C_StdArc(GrafVerb verb, Rect *r,
                                 INTEGER starta, INTEGER arca);
PASCAL_TRAP(StdArc, 0xA8BD);

extern pascal trap void C_StdBits(BitMap *srcbmp,
                                  const Rect *srcrp, const Rect *dstrp,
                                  INTEGER mode, RgnHandle mask);
PASCAL_TRAP(StdBits, 0xA8EB);
extern void StdBitsPicSaveFlag(BitMap *srcbmp,
                               const Rect *srcrp, const Rect *dstrp,
                               INTEGER mode, RgnHandle mask, BOOLEAN savepic);

extern void ROMlib_printsegs(INTEGER *ip);
extern pascal trap void C_StdLine(Point p);
PASCAL_TRAP(StdLine, 0xA890);
extern pascal trap void C_StdOval(GrafVerb v, Rect *rp);
PASCAL_TRAP(StdOval, 0xA8B6);
extern pascal trap void C_StdComment(INTEGER kind, INTEGER size,
                                     Handle hand);
PASCAL_TRAP(StdComment, 0xA8F1);
extern pascal trap void C_StdGetPic(Ptr dp, INTEGER bc);
PASCAL_TRAP(StdGetPic, 0xA8EE);
extern pascal trap void C_StdPutPic(Ptr sp, INTEGER bc);
PASCAL_TRAP(StdPutPic, 0xA8F0);
extern pascal trap void C_StdPoly(GrafVerb verb, PolyHandle ph);
PASCAL_TRAP(StdPoly, 0xA8C5);
extern pascal trap void C_StdRRect(GrafVerb verb, Rect *r,
                                   INTEGER width, INTEGER height);
PASCAL_TRAP(StdRRect, 0xA8AF);
extern pascal trap void C_StdRect(GrafVerb v, Rect *rp);
PASCAL_TRAP(StdRect, 0xA8A0);
extern pascal trap void C_StdRgn(GrafVerb verb, RgnHandle rgn);
PASCAL_TRAP(StdRgn, 0xA8D1);
extern pascal trap void C_StdText(INTEGER n, Ptr textbufp,
                                  Point num, Point den);
PASCAL_TRAP(StdText, 0xA882);
extern pascal trap INTEGER C_StdTxMeas(INTEGER n, Ptr p,
                                       GUEST<Point> *nump, GUEST<Point> *denp, FontInfo *finfop);
PASCAL_TRAP(StdTxMeas, 0xA8ED);
extern INTEGER ROMlib_StdTxMeas(LONGINT n, Ptr p,
                                GUEST<Point> *nump, GUEST<Point> *denp, FontInfo *finfop);
extern pascal trap void C_MeasureText(INTEGER n, Ptr text,
                                      Ptr chars);
PASCAL_TRAP(MeasureText, 0xA837);
extern pascal trap void C_TextFont(INTEGER f);
PASCAL_TRAP(TextFont, 0xA887);
extern pascal trap void C_TextFace(INTEGER thef);
PASCAL_TRAP(TextFace, 0xA888);
extern pascal trap void C_TextMode(INTEGER m);
PASCAL_TRAP(TextMode, 0xA889);
extern pascal trap void C_TextSize(INTEGER s);
PASCAL_TRAP(TextSize, 0xA88A);
extern pascal trap void C_SpaceExtra(Fixed e);
PASCAL_TRAP(SpaceExtra, 0xA88E);
extern pascal trap void C_DrawChar(CHAR thec);
PASCAL_TRAP(DrawChar, 0xA883);
extern pascal trap void C_DrawString(StringPtr s);
PASCAL_TRAP(DrawString, 0xA884);
extern pascal trap void C_DrawText(Ptr tb, INTEGER fb, INTEGER bc);
PASCAL_TRAP(DrawText, 0xA885);
extern pascal trap INTEGER C_CharWidth(CHAR thec);
PASCAL_TRAP(CharWidth, 0xA88D);
extern pascal trap INTEGER C_StringWidth(StringPtr s);
PASCAL_TRAP(StringWidth, 0xA88C);
extern pascal trap INTEGER C_TextWidth(Ptr tb, INTEGER fb, INTEGER bc);
PASCAL_TRAP(TextWidth, 0xA886);
extern pascal trap void C_GetFontInfo(FontInfo *ip);
PASCAL_TRAP(GetFontInfo, 0xA88B);

extern pascal trap void C_GetCPixel(INTEGER h, INTEGER v, RGBColor *colorp);
PASCAL_TRAP(GetCPixel, 0xAA17);
extern pascal trap void C_SetCPixel(INTEGER h, INTEGER v,
                                    RGBColor *colorp);
PASCAL_TRAP(SetCPixel, 0xAA16);

extern pascal trap void C_SeedCFill(BitMap *srcbp, BitMap *dstbp,
                                    Rect *srcrp, Rect *dstrp, INTEGER seedh, INTEGER seedv,
                                    ProcPtr matchprocp, LONGINT matchdata);
PASCAL_TRAP(SeedCFill, 0xAA50);

extern pascal trap void C_CalcCMask(BitMap *srcbp, BitMap *dstbp,
                                    Rect *srcrp, Rect *dstrp, RGBColor *seedrgbp, ProcPtr matchprocp,
                                    LONGINT matchdata);
PASCAL_TRAP(CalcCMask, 0xAA4F);
extern pascal trap void C_IMVI_CopyDeepMask(
    BitMap *srcBits,
    BitMap *maskBits,
    BitMap *dstBits,
    Rect *srcRect,
    Rect *maskRect,
    Rect *dstRect,
    INTEGER mode,
    RgnHandle maskRgn);
PASCAL_TRAP(IMVI_CopyDeepMask, 0xAA51);
}
#endif /* _QUICKDRAW_H_ */
