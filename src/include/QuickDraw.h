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

typedef void (*textProc_t)(INTEGER bc, Ptr textb, Point num, Point den);
typedef void (*lineProc_t)(Point drawto);
typedef void (*rectProc_t)(GrafVerb verb, Rect *rp);
typedef void (*rRectProc_t)(GrafVerb verb, Rect *rp, INTEGER ow,
                                        INTEGER oh);
typedef void (*ovalProc_t)(GrafVerb verb, Rect *rp);
typedef void (*arcProc_t)(GrafVerb verb, Rect *rp, INTEGER ang,
                                      INTEGER arc);
typedef void (*polyProc_t)(GrafVerb verb, PolyHandle poly);
typedef void (*rgnProc_t)(GrafVerb verb, RgnHandle rgn);
typedef void (*bitsProc_t)(BitMap *srcb, Rect *srcr, Rect *dstr,
                                       INTEGER mod, RgnHandle mask);
typedef void (*commentProc_t)(INTEGER kind, INTEGER size, Handle data);
typedef INTEGER (*txMeasProc_t)(INTEGER bc, Ptr texta, GUEST<Point> *numer,
                                            GUEST<Point> *denom, FontInfo *info);
typedef void (*getPicProc_t)(Ptr data, INTEGER bc);
typedef void (*putPicProc_t)(Ptr data, INTEGER bc);

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

const LowMemGlobal<INTEGER> ScrVRes { 0x102 }; // QuickDraw IMI-473 (true);
const LowMemGlobal<INTEGER> ScrHRes { 0x104 }; // QuickDraw IMI-473 (true);
const LowMemGlobal<INTEGER> ScreenRow { 0x106 }; // QuickDraw ThinkC (true);
const LowMemGlobal<LONGINT> RndSeed { 0x156 }; // QuickDraw IMI-195 (true);
const LowMemGlobal<Byte[8]> ScreenVars { 0x292 }; // QuickDraw MPW (false);
/*
 * NOTE: Key1Trans in the keyboard translator procedure, and Key2Trans in the
 * numeric keypad translator procedure (MPW).
 */
const LowMemGlobal<Ptr> Key1Trans { 0x29E }; // QuickDraw MPW (false);
const LowMemGlobal<Ptr> Key2Trans { 0x2A2 }; // QuickDraw MPW (false);
const LowMemGlobal<ProcPtr> JUnknown574 { 0x574 }; // QuickDraw IMV (true-b);
const LowMemGlobal<ProcPtr> JADBProc { 0x6B8 }; // QuickDraw IMV (false);
const LowMemGlobal<ProcPtr> JHideCursor { 0x800 }; // QuickDraw Private.a (true-b);
const LowMemGlobal<ProcPtr> JShowCursor { 0x804 }; // QuickDraw Private.a (true-b);
const LowMemGlobal<ProcPtr> JShieldCursor { 0x808 }; // QuickDraw Private.a (true-b);
const LowMemGlobal<ProcPtr> JScrnAddr { 0x80C }; // QuickDraw Private.a (false);
const LowMemGlobal<ProcPtr> JScrnSize { 0x810 }; // QuickDraw Private.a (false);
const LowMemGlobal<ProcPtr> JInitCrsr { 0x814 }; // QuickDraw Private.a (true-b);
const LowMemGlobal<ProcPtr> JSetCrsr { 0x818 }; // QuickDraw Private.a (true-b);
const LowMemGlobal<ProcPtr> JCrsrObscure { 0x81C }; // QuickDraw Private.a (true-b);
const LowMemGlobal<ProcPtr> JUpdateProc { 0x820 }; // QuickDraw Private.a (false);
const LowMemGlobal<Ptr> ScrnBase { 0x824 }; // QuickDraw IMII-19 (true);
/*
 * MouseLocation used to be 0x830, but that doesn't jibe with what I've
 * seen of Crystal Quest --ctm
 */
const LowMemGlobal<Rect> CrsrPin { 0x834 }; // QuickDraw ThinkC (false);
const LowMemGlobal<Byte> QDColors { 0x8B0 }; // QuickDraw IMV (false);
const LowMemGlobal<BOOLEAN> CrsrVis { 0x8CC }; // QuickDraw SysEqu.a (true);
const LowMemGlobal<Byte> CrsrBusy { 0x8CD }; // QuickDraw SysEqu.a (true);
const LowMemGlobal<INTEGER> CrsrState { 0x8D0 }; // QuickDraw SysEqu.a (true);
const LowMemGlobal<LONGINT> mousemask { 0x8D6 }; // QuickDraw .a (true-b);
const LowMemGlobal<LONGINT> mouseoffset { 0x8DA }; // QuickDraw SysEqu.a (true-b);
const LowMemGlobal<ProcPtr> JCrsrTask { 0x8EE }; //   (true);
const LowMemGlobal<Byte> HiliteMode { 0x938 }; // QuickDraw IMV (true-b);


extern void C_CopyBits(BitMap *src_bitmap, BitMap *dst_bitmap,
                            const Rect *src_rect, const Rect *dst_rect,
                            INTEGER mode, RgnHandle mask);
PASCAL_TRAP(CopyBits, 0xA8EC);

extern void C_ScrollRect(Rect *rp, INTEGER dh, INTEGER dv,
                              RgnHandle updatergn);
PASCAL_TRAP(ScrollRect, 0xA8EF);

extern void C_ForeColor(LONGINT c);
PASCAL_TRAP(ForeColor, 0xA862);
extern void C_BackColor(LONGINT c);
PASCAL_TRAP(BackColor, 0xA863);
extern void C_ColorBit(INTEGER b);
PASCAL_TRAP(ColorBit, 0xA864);
extern void C_SetCursor(Cursor *cp);
PASCAL_TRAP(SetCursor, 0xA851);
extern void C_InitCursor(void);
PASCAL_TRAP(InitCursor, 0xA850);
extern void C_HideCursor(void);
PASCAL_TRAP(HideCursor, 0xA852);
extern void C_ShowCursor(void);
PASCAL_TRAP(ShowCursor, 0xA853);
extern void C_ObscureCursor(void);
PASCAL_TRAP(ObscureCursor, 0xA856);
extern void C_ShieldCursor(Rect *rp, Point p);
PASCAL_TRAP(ShieldCursor, 0xA855);
extern void C_InitGraf(Ptr gp);
PASCAL_TRAP(InitGraf, 0xA86E);
extern void C_SetPort(GrafPtr p);
PASCAL_TRAP(SetPort, 0xA873);
extern void C_InitPort(GrafPtr p);
PASCAL_TRAP(InitPort, 0xA86D);
extern void C_OpenPort(GrafPtr p);
PASCAL_TRAP(OpenPort, 0xA86F);
extern void C_ClosePort(GrafPtr p);
PASCAL_TRAP(ClosePort, 0xA87D);
extern void C_GetPort(GUEST<GrafPtr> *pp);
PASCAL_TRAP(GetPort, 0xA874);
extern void C_GrafDevice(INTEGER d);
PASCAL_TRAP(GrafDevice, 0xA872);
extern void C_SetPortBits(BitMap *bm);
PASCAL_TRAP(SetPortBits, 0xA875);
extern void C_PortSize(INTEGER w, INTEGER h);
PASCAL_TRAP(PortSize, 0xA876);
extern void C_MovePortTo(INTEGER lg, INTEGER tg);
PASCAL_TRAP(MovePortTo, 0xA877);
extern void C_SetOrigin(INTEGER h, INTEGER v);
PASCAL_TRAP(SetOrigin, 0xA878);
extern void C_SetClip(RgnHandle r);
PASCAL_TRAP(SetClip, 0xA879);
extern void C_GetClip(RgnHandle r);
PASCAL_TRAP(GetClip, 0xA87A);
extern void C_ClipRect(Rect *r);
PASCAL_TRAP(ClipRect, 0xA87B);
extern void C_BackPat(Pattern pp);
PASCAL_TRAP(BackPat, 0xA87C);
extern void C_SeedFill(Ptr srcp, Ptr dstp,
                                   INTEGER srcr, INTEGER dstr, INTEGER height, INTEGER width,
                                   INTEGER seedh, INTEGER seedv);
PASCAL_TRAP(SeedFill, 0xA839);
extern void C_CalcMask(Ptr srcp, Ptr dstp,
                                   INTEGER srcr, INTEGER dstr, INTEGER height, INTEGER width);
PASCAL_TRAP(CalcMask, 0xA838);
extern void C_CopyMask(BitMap *srcbp,
                                   BitMap *mskbp, BitMap *dstbp, Rect *srcrp, Rect *
                                                                                  mskrp,
                                   Rect *dstrp);
PASCAL_TRAP(CopyMask, 0xA817);
extern INTEGER *GetMaskTable(void);
extern void C_CharExtra(Fixed Extra);
PASCAL_TRAP(CharExtra, 0xAA23);
extern void C_MakeRGBPat(PixPatHandle ph,
                                     RGBColor *colorp);
PASCAL_TRAP(MakeRGBPat, 0xAA0D);
extern INTEGER C_Random(void);
PASCAL_TRAP(Random, 0xA861);
extern BOOLEAN C_GetPixel(INTEGER h, INTEGER v);
PASCAL_TRAP(GetPixel, 0xA865);
extern void C_StuffHex(Ptr p, StringPtr s);
PASCAL_TRAP(StuffHex, 0xA866);
extern void C_ScalePt(GUEST<Point> *pt, Rect *srcr, Rect *dstr);
PASCAL_TRAP(ScalePt, 0xA8F8);
extern void C_MapPt(GUEST<Point> *pt, Rect *srcr, Rect *dstr);
PASCAL_TRAP(MapPt, 0xA8F9);
extern void C_MapRect(Rect *r, Rect *srcr, Rect *dstr);
PASCAL_TRAP(MapRect, 0xA8FA);
extern void C_MapRgn(RgnHandle rh, Rect *srcr, Rect *dstr);
PASCAL_TRAP(MapRgn, 0xA8FB);
extern void C_MapPoly(PolyHandle poly, Rect *srcr,
                                  Rect *dstr);
PASCAL_TRAP(MapPoly, 0xA8FC);
extern void C_HidePen(void);
PASCAL_TRAP(HidePen, 0xA896);
extern void C_ShowPen(void);
PASCAL_TRAP(ShowPen, 0xA897);
extern void C_GetPen(GUEST<Point> *ptp);
PASCAL_TRAP(GetPen, 0xA89A);
extern void C_GetPenState(PenState *ps);
PASCAL_TRAP(GetPenState, 0xA898);
extern void C_SetPenState(PenState *ps);
PASCAL_TRAP(SetPenState, 0xA899);
extern void C_PenSize(INTEGER w, INTEGER h);
PASCAL_TRAP(PenSize, 0xA89B);
extern void C_PenMode(INTEGER m);
PASCAL_TRAP(PenMode, 0xA89C);
extern void C_PenPat(Pattern pp);
PASCAL_TRAP(PenPat, 0xA89D);
extern void C_PenNormal(void);
PASCAL_TRAP(PenNormal, 0xA89E);
extern void C_MoveTo(INTEGER h, INTEGER v);
PASCAL_TRAP(MoveTo, 0xA893);
extern void C_Move(INTEGER dh, INTEGER dv);
PASCAL_TRAP(Move, 0xA894);
extern void C_LineTo(INTEGER h, INTEGER v);
PASCAL_TRAP(LineTo, 0xA891);
extern void C_Line(INTEGER dh, INTEGER dv);
PASCAL_TRAP(Line, 0xA892);
extern void C_DrawPicture(PicHandle pic, Rect *destrp);
PASCAL_TRAP(DrawPicture, 0xA8F6);
extern PicHandle C_OpenPicture(Rect *pf);
PASCAL_TRAP(OpenPicture, 0xA8F3);
extern void C_ClosePicture(void);
PASCAL_TRAP(ClosePicture, 0xA8F4);
extern void C_PicComment(INTEGER kind, INTEGER size,
                                     Handle hand);
PASCAL_TRAP(PicComment, 0xA8F2);
extern void C_ReadComment(INTEGER kind, INTEGER size,
                                      Handle hand);
PASCAL_FUNCTION(ReadComment);
extern void C_KillPicture(PicHandle pic);
PASCAL_TRAP(KillPicture, 0xA8F5);
extern void C_AddPt(Point src, GUEST<Point> *dst);
PASCAL_TRAP(AddPt, 0xA87E);
extern void C_SubPt(Point src, GUEST<Point> *dst);
PASCAL_TRAP(SubPt, 0xA87F);
extern void C_SetPt(GUEST<Point> *pt, INTEGER h, INTEGER v);
PASCAL_TRAP(SetPt, 0xA880);
extern BOOLEAN C_EqualPt(Point p1, Point p2);
PASCAL_TRAP(EqualPt, 0xA881);
extern void C_LocalToGlobal(GUEST<Point> *pt);
PASCAL_TRAP(LocalToGlobal, 0xA870);
extern void C_GlobalToLocal(GUEST<Point> *pt);
PASCAL_TRAP(GlobalToLocal, 0xA871);
extern PolyHandle C_OpenPoly(void);
PASCAL_TRAP(OpenPoly, 0xA8CB);
extern void C_ClosePoly(void);
PASCAL_TRAP(ClosePoly, 0xA8CC);
extern void C_KillPoly(PolyHandle poly);
PASCAL_TRAP(KillPoly, 0xA8CD);
extern void C_OffsetPoly(PolyHandle poly,
                                     INTEGER dh, INTEGER dv);
PASCAL_TRAP(OffsetPoly, 0xA8CE);
extern void C_SetRect(Rect *r, INTEGER left, INTEGER top,
                                  INTEGER right, INTEGER bottom);
PASCAL_TRAP(SetRect, 0xA8A7);
extern void C_OffsetRect(Rect *r, INTEGER dh, INTEGER dv);
PASCAL_TRAP(OffsetRect, 0xA8A8);
extern void C_InsetRect(Rect *r, INTEGER dh, INTEGER dv);
PASCAL_TRAP(InsetRect, 0xA8A9);

extern BOOLEAN C_EmptyRect(Rect *r);
PASCAL_TRAP(EmptyRect, 0xA8AE);
extern BOOLEAN C_SectRect(const Rect *s1, const Rect *s2, Rect *dest);
PASCAL_TRAP(SectRect, 0xA8AA);
extern void C_UnionRect(Rect *s1, Rect *s2, Rect *dest);
PASCAL_TRAP(UnionRect, 0xA8AB);
extern BOOLEAN C_PtInRect(Point p, Rect *r);
PASCAL_TRAP(PtInRect, 0xA8AD);
extern void C_Pt2Rect(Point p1, Point p2, Rect *dest);
PASCAL_TRAP(Pt2Rect, 0xA8AC);
extern void C_PtToAngle(Rect *rp, Point p, GUEST<INTEGER> *angle);
PASCAL_TRAP(PtToAngle, 0xA8C3);
extern BOOLEAN C_EqualRect(const Rect *r1, const Rect *r2);
PASCAL_TRAP(EqualRect, 0xA8A6);
extern RgnHandle C_NewRgn(void);
PASCAL_TRAP(NewRgn, 0xA8D8);
extern void C_OpenRgn(void);
PASCAL_TRAP(OpenRgn, 0xA8DA);
extern void C_CopyRgn(RgnHandle s, RgnHandle d);
PASCAL_TRAP(CopyRgn, 0xA8DC);
extern void C_CloseRgn(RgnHandle rh);
PASCAL_TRAP(CloseRgn, 0xA8DB);
extern void C_DisposeRgn(RgnHandle rh);
PASCAL_TRAP(DisposeRgn, 0xA8D9);
extern void C_SetEmptyRgn(RgnHandle rh);
PASCAL_TRAP(SetEmptyRgn, 0xA8DD);
extern void C_SetRectRgn(RgnHandle rh, INTEGER left,
                                     INTEGER top, INTEGER right, INTEGER bottom);
PASCAL_TRAP(SetRectRgn, 0xA8DE);
extern void C_RectRgn(RgnHandle rh, Rect *rect);
PASCAL_TRAP(RectRgn, 0xA8DF);
extern void C_OffsetRgn(RgnHandle rh, INTEGER dh,
                                    INTEGER dv);
PASCAL_TRAP(OffsetRgn, 0xA8E0);
extern BOOLEAN C_PtInRgn(Point p, RgnHandle rh);
PASCAL_TRAP(PtInRgn, 0xA8E8);
extern void C_InsetRgn(RgnHandle rh, INTEGER dh, INTEGER dv);
PASCAL_TRAP(InsetRgn, 0xA8E1);
extern void C_SectRgn(RgnHandle s1, RgnHandle s2,
                                  RgnHandle dest);
PASCAL_TRAP(SectRgn, 0xA8E4);
extern void C_UnionRgn(RgnHandle s1, RgnHandle s2,
                                   RgnHandle dest);
PASCAL_TRAP(UnionRgn, 0xA8E5);
extern void C_DiffRgn(RgnHandle s1, RgnHandle s2,
                                  RgnHandle dest);
PASCAL_TRAP(DiffRgn, 0xA8E6);
extern void C_XorRgn(RgnHandle s1, RgnHandle s2,
                                 RgnHandle dest);
PASCAL_TRAP(XorRgn, 0xA8E7);
extern BOOLEAN C_RectInRgn(Rect *rp,
                                       RgnHandle rh);
PASCAL_TRAP(RectInRgn, 0xA8E9);
extern BOOLEAN C_EqualRgn(RgnHandle r1, RgnHandle r2);
PASCAL_TRAP(EqualRgn, 0xA8E3);
extern BOOLEAN C_EmptyRgn(RgnHandle rh);
PASCAL_TRAP(EmptyRgn, 0xA8E2);
extern void ROMlib_printrgn(RgnHandle h);
extern void ROMlib_printpairs(INTEGER *p, LONGINT n);
extern void C_FrameRect(Rect *r);
PASCAL_TRAP(FrameRect, 0xA8A1);
extern void C_PaintRect(Rect *r);
PASCAL_TRAP(PaintRect, 0xA8A2);
extern void C_EraseRect(Rect *r);
PASCAL_TRAP(EraseRect, 0xA8A3);
extern void C_InvertRect(Rect *r);
PASCAL_TRAP(InvertRect, 0xA8A4);
extern void C_FillRect(Rect *r, Pattern pat);
PASCAL_TRAP(FillRect, 0xA8A5);
extern void C_FrameOval(Rect *r);
PASCAL_TRAP(FrameOval, 0xA8B7);
extern void C_PaintOval(Rect *r);
PASCAL_TRAP(PaintOval, 0xA8B8);
extern void C_EraseOval(Rect *r);
PASCAL_TRAP(EraseOval, 0xA8B9);
extern void C_InvertOval(Rect *r);
PASCAL_TRAP(InvertOval, 0xA8BA);
extern void C_FillOval(Rect *r, Pattern pat);
PASCAL_TRAP(FillOval, 0xA8BB);
extern void C_FrameRoundRect(Rect *r, INTEGER ow,
                                         INTEGER oh);
PASCAL_TRAP(FrameRoundRect, 0xA8B0);
extern void C_PaintRoundRect(Rect *r, INTEGER ow,
                                         INTEGER oh);
PASCAL_TRAP(PaintRoundRect, 0xA8B1);
extern void C_EraseRoundRect(Rect *r, INTEGER ow,
                                         INTEGER oh);
PASCAL_TRAP(EraseRoundRect, 0xA8B2);
extern void C_InvertRoundRect(Rect *r, INTEGER ow,
                                          INTEGER oh);
PASCAL_TRAP(InvertRoundRect, 0xA8B3);
extern void C_FillRoundRect(Rect *r, INTEGER ow,
                                        INTEGER oh, Pattern pat);
PASCAL_TRAP(FillRoundRect, 0xA8B4);
extern void C_FrameArc(Rect *r, INTEGER start,
                                   INTEGER angle);
PASCAL_TRAP(FrameArc, 0xA8BE);
extern void C_PaintArc(Rect *r, INTEGER start,
                                   INTEGER angle);
PASCAL_TRAP(PaintArc, 0xA8BF);
extern void C_EraseArc(Rect *r, INTEGER start,
                                   INTEGER angle);
PASCAL_TRAP(EraseArc, 0xA8C0);
extern void C_InvertArc(Rect *r, INTEGER start,
                                    INTEGER angle);
PASCAL_TRAP(InvertArc, 0xA8C1);
extern void C_FillArc(Rect *r, INTEGER start,
                                  INTEGER angle, Pattern pat);
PASCAL_TRAP(FillArc, 0xA8C2);
extern void C_FrameRgn(RgnHandle rh);
PASCAL_TRAP(FrameRgn, 0xA8D2);
extern void C_PaintRgn(RgnHandle rh);
PASCAL_TRAP(PaintRgn, 0xA8D3);
extern void C_EraseRgn(RgnHandle rh);
PASCAL_TRAP(EraseRgn, 0xA8D4);
extern void C_InvertRgn(RgnHandle rh);
PASCAL_TRAP(InvertRgn, 0xA8D5);
extern void C_FillRgn(RgnHandle rh, Pattern pat);
PASCAL_TRAP(FillRgn, 0xA8D6);
extern void C_FramePoly(PolyHandle poly);
PASCAL_TRAP(FramePoly, 0xA8C6);
extern void C_PaintPoly(PolyHandle poly);
PASCAL_TRAP(PaintPoly, 0xA8C7);
extern void C_ErasePoly(PolyHandle poly);
PASCAL_TRAP(ErasePoly, 0xA8C8);
extern void C_InvertPoly(PolyHandle poly);
PASCAL_TRAP(InvertPoly, 0xA8C9);
extern void C_FillPoly(PolyHandle poly, Pattern pat);
PASCAL_TRAP(FillPoly, 0xA8CA);
extern void C_SetStdProcs(QDProcs *procs);
PASCAL_TRAP(SetStdProcs, 0xA8EA);
extern void C_StdArc(GrafVerb verb, Rect *r,
                                 INTEGER starta, INTEGER arca);
PASCAL_TRAP(StdArc, 0xA8BD);

extern void C_StdBits(BitMap *srcbmp,
                                  const Rect *srcrp, const Rect *dstrp,
                                  INTEGER mode, RgnHandle mask);
PASCAL_TRAP(StdBits, 0xA8EB);
extern void StdBitsPicSaveFlag(BitMap *srcbmp,
                               const Rect *srcrp, const Rect *dstrp,
                               INTEGER mode, RgnHandle mask, BOOLEAN savepic);

extern void ROMlib_printsegs(INTEGER *ip);
extern void C_StdLine(Point p);
PASCAL_TRAP(StdLine, 0xA890);
extern void C_StdOval(GrafVerb v, Rect *rp);
PASCAL_TRAP(StdOval, 0xA8B6);
extern void C_StdComment(INTEGER kind, INTEGER size,
                                     Handle hand);
PASCAL_TRAP(StdComment, 0xA8F1);
extern void C_StdGetPic(Ptr dp, INTEGER bc);
PASCAL_TRAP(StdGetPic, 0xA8EE);
extern void C_StdPutPic(Ptr sp, INTEGER bc);
PASCAL_TRAP(StdPutPic, 0xA8F0);
extern void C_StdPoly(GrafVerb verb, PolyHandle ph);
PASCAL_TRAP(StdPoly, 0xA8C5);
extern void C_StdRRect(GrafVerb verb, Rect *r,
                                   INTEGER width, INTEGER height);
PASCAL_TRAP(StdRRect, 0xA8AF);
extern void C_StdRect(GrafVerb v, Rect *rp);
PASCAL_TRAP(StdRect, 0xA8A0);
extern void C_StdRgn(GrafVerb verb, RgnHandle rgn);
PASCAL_TRAP(StdRgn, 0xA8D1);
extern void C_StdText(INTEGER n, Ptr textbufp,
                                  Point num, Point den);
PASCAL_TRAP(StdText, 0xA882);
extern INTEGER C_StdTxMeas(INTEGER n, Ptr p,
                                       GUEST<Point> *nump, GUEST<Point> *denp, FontInfo *finfop);
PASCAL_TRAP(StdTxMeas, 0xA8ED);
extern INTEGER ROMlib_StdTxMeas(LONGINT n, Ptr p,
                                GUEST<Point> *nump, GUEST<Point> *denp, FontInfo *finfop);
extern void C_MeasureText(INTEGER n, Ptr text,
                                      Ptr chars);
PASCAL_TRAP(MeasureText, 0xA837);
extern void C_TextFont(INTEGER f);
PASCAL_TRAP(TextFont, 0xA887);
extern void C_TextFace(INTEGER thef);
PASCAL_TRAP(TextFace, 0xA888);
extern void C_TextMode(INTEGER m);
PASCAL_TRAP(TextMode, 0xA889);
extern void C_TextSize(INTEGER s);
PASCAL_TRAP(TextSize, 0xA88A);
extern void C_SpaceExtra(Fixed e);
PASCAL_TRAP(SpaceExtra, 0xA88E);
extern void C_DrawChar(CharParameter thec);
PASCAL_TRAP(DrawChar, 0xA883);
extern void C_DrawString(StringPtr s);
PASCAL_TRAP(DrawString, 0xA884);
extern void C_DrawText(Ptr tb, INTEGER fb, INTEGER bc);
PASCAL_TRAP(DrawText, 0xA885);
extern INTEGER C_CharWidth(CharParameter thec);
PASCAL_TRAP(CharWidth, 0xA88D);
extern INTEGER C_StringWidth(StringPtr s);
PASCAL_TRAP(StringWidth, 0xA88C);
extern INTEGER C_TextWidth(Ptr tb, INTEGER fb, INTEGER bc);
PASCAL_TRAP(TextWidth, 0xA886);
extern void C_GetFontInfo(FontInfo *ip);
PASCAL_TRAP(GetFontInfo, 0xA88B);

extern void C_GetCPixel(INTEGER h, INTEGER v, RGBColor *colorp);
PASCAL_TRAP(GetCPixel, 0xAA17);
extern void C_SetCPixel(INTEGER h, INTEGER v,
                                    RGBColor *colorp);
PASCAL_TRAP(SetCPixel, 0xAA16);

extern void C_SeedCFill(BitMap *srcbp, BitMap *dstbp,
                                    Rect *srcrp, Rect *dstrp, INTEGER seedh, INTEGER seedv,
                                    ProcPtr matchprocp, LONGINT matchdata);
PASCAL_TRAP(SeedCFill, 0xAA50);

extern void C_CalcCMask(BitMap *srcbp, BitMap *dstbp,
                                    Rect *srcrp, Rect *dstrp, RGBColor *seedrgbp, ProcPtr matchprocp,
                                    LONGINT matchdata);
PASCAL_TRAP(CalcCMask, 0xAA4F);
extern void C_IMVI_CopyDeepMask(
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
