
#if !defined(_CQUICKDRAW_H_)
#define _CQUICKDRAW_H_

#include "WindowMgr.h"

namespace Executor
{
#define theCPort (STARH(STARH((GUEST<GUEST<CGrafPtr> *> *)SYN68K_TO_US(EM_A5))))
#define theCPortX ((*STARH((GUEST<GUEST<CGrafPtr> *> *)SYN68K_TO_US(EM_A5))))

enum
{
    minSeed = 1024,
};

/* can't use [0];
     make this an unsigned char even tho the mac has SignedByte;
     it is treated as unsigned */
typedef struct ITab
{
    GUEST_STRUCT;
    GUEST<LONGINT> iTabSeed;
    GUEST<INTEGER> iTabRes;
    GUEST<unsigned char[1]> iTTable;
} * ITabPtr;

typedef GUEST<ITabPtr> *ITabHandle;

typedef struct GDevice *GDPtr;
typedef GDPtr GDevicePtr;

typedef GUEST<GDevicePtr> *GDHandle;

typedef struct SProcRec *SProcPtr;

typedef GUEST<SProcPtr> *SProcHndl;
struct SProcRec
{
    GUEST_STRUCT;
    GUEST<SProcHndl> nxtSrch;
    GUEST<ProcPtr> srchProc;
};

typedef struct CProcRec *CProcPtr;

typedef GUEST<CProcPtr> *CProcHndl;
struct CProcRec
{
    GUEST_STRUCT;
    GUEST<CProcHndl> nxtComp;
    GUEST<ProcPtr> compProc;
};

typedef void *DeviceLoopDrawingProcPtr;

struct GDevice
{
    GUEST_STRUCT;
    GUEST<INTEGER> gdRefNum;
    GUEST<INTEGER> gdID;
    GUEST<INTEGER> gdType;
    GUEST<ITabHandle> gdITable;
    GUEST<INTEGER> gdResPref;
    GUEST<SProcHndl> gdSearchProc;
    GUEST<CProcHndl> gdCompProc;
    GUEST<INTEGER> gdFlags;
    GUEST<PixMapHandle> gdPMap;
    GUEST<LONGINT> gdRefCon;
    GUEST<GDHandle> gdNextGD;
    GUEST<Rect> gdRect;
    GUEST<LONGINT> gdMode;
    GUEST<INTEGER> gdCCBytes;
    GUEST<INTEGER> gdCCDepth;
    GUEST<Handle> gdCCXData;
    GUEST<Handle> gdCCXMask;
    GUEST<LONGINT> gdReserved;
};

typedef uint32_t DeviceLoopFlags;

/* DeviceLoop flags. */
enum
{
    singleDevices = (1 << 0),
    dontMatchSeeds = (1 << 1),
    allDevices = (1 << 2),
};

struct ColorInfo
{
    GUEST_STRUCT;
    GUEST<RGBColor> ciRGB;
    GUEST<INTEGER> ciUsage;
    GUEST<INTEGER> ciTolerance;
    GUEST<INTEGER> ciFlags;
    GUEST<LONGINT> ciPrivate;
};

typedef struct Palette
{
    GUEST_STRUCT;
    GUEST<INTEGER> pmEntries;
    GUEST<GrafPtr> pmWindow;
    GUEST<INTEGER> pmPrivate;
    GUEST<LONGINT> pmDevices; /* Handle? */
    GUEST<Handle> pmSeeds;
    GUEST<ColorInfo[1]> pmInfo;
} * PalettePtr;

#define CI_USAGE_TYPE_BITS_X (CWC(0xF))
enum
{
    CI_USAGE_TYPE_BITS = (0xF),
};

typedef enum {
    pmCourteous = 0,
    pmDithered = 1,
    pmTolerant = 2,
    pmAnimated = 4,
    pmExplicit = 8,

    pmInhibitG2 = 0x0100,
    pmInhibitC2 = 0x0200,
    pmInhibitG4 = 0x0400,
    pmInhibitC4 = 0x0800,
    pmInhibitG8 = 0x1000,
    pmInhibitC8 = 0x2000,
} pmColorUsage;

typedef enum {
    pmNoUpdates = 0x8000,
    pmBkUpdates = 0xA000,
    pmFgUpdates = 0xC000,
    pmAllUpdates = 0xE000,
} pmUpdates;

typedef GUEST<PalettePtr> *PaletteHandle;

/* return true if `maybe_graphics_world' points to a graphics world,
   and not a graf port or cgraf port */
#define GRAPHICS_WORLD_P(maybe_graphics_world) \
    (CGrafPort_p(maybe_graphics_world)         \
     && CPORT_VERSION_X(maybe_graphics_world) & GW_FLAG_BIT_X)

#define GW_CPORT(graphics_world) ((CGrafPtr)(graphics_world))

typedef LONGINT GWorldFlags;

typedef CGrafPort GWorld, *GWorldPtr;

struct ReqListRec
{
    GUEST_STRUCT;
    GUEST<INTEGER> reqLSize;
    GUEST<INTEGER[1]> reqLData;
};

/* extended version 2 picture datastructures */

struct OpenCPicParams
{
    GUEST_STRUCT;
    GUEST<Rect> srcRect;
    GUEST<Fixed> hRes;
    GUEST<Fixed> vRes;
    GUEST<int16_t> version;
    GUEST<int16_t> reserved1;
    GUEST<int32_t> reserved2;
};

typedef struct CommonSpec
{
    GUEST_STRUCT;
    GUEST<int16_t> count;
    GUEST<int16_t> ID;
} CommentSpec;

typedef CommentSpec *CommentSpecPtr;

typedef GUEST<CommentSpecPtr> *CommentSpecHandle;

struct FontSpec
{
    GUEST_STRUCT;
    GUEST<int16_t> pictFontID;
    GUEST<int16_t> sysFontID;
    GUEST<int32_t[4]> size;
    GUEST<int16_t> style;
    GUEST<int32_t> nameOffset;
};

typedef FontSpec *FontSpecPtr;

typedef GUEST<FontSpecPtr> *FontSpecHandle;

struct PictInfo
{
    GUEST_STRUCT;
    GUEST<int16_t> version; /* 0 */
    GUEST<int32_t> uniqueColors; /* 2 */
    GUEST<PaletteHandle> thePalette; /* 6 */
    GUEST<CTabHandle> theColorTable; /* 10 */
    GUEST<Fixed> hRes; /* 14 */
    GUEST<Fixed> vRes; /* 18 */
    GUEST<INTEGER> depth; /* 22 */
    GUEST<Rect> sourceRect; /* top24, left26, bottom28, right30 */
    GUEST<int32_t> textCount; /* 32 */
    GUEST<int32_t> lineCount;
    GUEST<int32_t> rectCount;
    GUEST<int32_t> rRectCount;
    GUEST<int32_t> ovalCount;
    GUEST<int32_t> arcCount;
    GUEST<int32_t> polyCount;
    GUEST<int32_t> regionCount;
    GUEST<int32_t> bitMapCount;
    GUEST<int32_t> pixMapCount;
    GUEST<int32_t> commentCount;
    GUEST<int32_t> uniqueComments;
    GUEST<CommentSpecHandle> commentHandle;
    GUEST<int32_t> uniqueFonts;
    GUEST<FontSpecHandle> fontHandle;
    GUEST<Handle> fontNamesHandle;
    GUEST<int32_t> reserved1;
    GUEST<int32_t> reserved2;
};

typedef PictInfo *PictInfoPtr;

typedef GUEST<PictInfoPtr> *PictInfoHandle;

typedef int32_t PictInfoID;

enum
{
    RGBDirect = (0x10),
    Indirect = (0),
};
/* a pixmap pixelType of `native_rgb_pixel_type' means that the format
   of the pixmap is the same as that of the screen */
enum
{
    vdriver_rgb_pixel_type = (0xB9),
};

enum
{
    pixPurge = (1 << 0),
    noNewDevice = (1 << 1),
    useTempMem = (1 << 2),
    keepLocal = (1 << 3),
    pixelsPurgeable = (1 << 6),
    pixelsLocked = (1 << 7),
    mapPix = (1 << 16),
    newDepth = (1 << 17),
    alignPix = (1 << 18),
    newRowBytes = (1 << 19),
    reallocPix = (1 << 20),
    clipPix = (1 << 28),
    stretchPix = (1 << 29),
    ditherPix = (1 << 30),
    gwFlagErr = (1 << 31),
};

typedef int16_t QDErr;

/* error codes returned by QDError */
enum
{
    noMemForPictPlaybackErr = (-145),
    regionTooBigErr = (-147),
    pixmapTooDeepErr = (-148),
    nsStackErr = (-149),
    cMatchErr = (-150),
    cTempMemErr = (-151),
    cNoMemErr = (-152),
    cRangeErr = (-153),
    cProtectErr = (-154),
    cDevErr = (-155),
    cResErr = (-156),
    cDepthErr = (-157),
    rgnTooBigErr = (-500),
};

/* TODO:  FIXME -- #warning find out correct value for colReqErr */
/*	  -158 is just a guess */

enum
{
    colReqErr = (-158),
};

extern pascal trap void C_SetStdCProcs(CQDProcs *cProcs);
PASCAL_TRAP(SetStdCProcs, 0xAA4E);

extern pascal trap void C_OpenCPort(CGrafPtr);
PASCAL_TRAP(OpenCPort, 0xAA00);
extern pascal trap void C_InitCPort(CGrafPtr);
PASCAL_TRAP(InitCPort, 0xAA01);
extern pascal trap void C_CloseCPort(CGrafPtr);
PASCAL_TRAP(CloseCPort, 0xAA02);
extern pascal trap void C_RGBForeColor(RGBColor *);
PASCAL_TRAP(RGBForeColor, 0xAA14);
extern pascal trap void C_RGBBackColor(RGBColor *);
PASCAL_TRAP(RGBBackColor, 0xAA15);
extern pascal trap void C_GetForeColor(RGBColor *);
PASCAL_TRAP(GetForeColor, 0xAA19);
extern pascal trap void C_GetBackColor(RGBColor *);
PASCAL_TRAP(GetBackColor, 0xAA1A);
extern pascal trap void C_PenPixPat(PixPatHandle);
PASCAL_TRAP(PenPixPat, 0xAA0A);
extern pascal trap void C_BackPixPat(PixPatHandle);
PASCAL_TRAP(BackPixPat, 0xAA0B);
extern pascal trap void C_OpColor(RGBColor *);
PASCAL_TRAP(OpColor, 0xAA21);
extern pascal trap void C_HiliteColor(RGBColor *);
PASCAL_TRAP(HiliteColor, 0xAA22);
extern pascal trap PixMapHandle C_NewPixMap();
PASCAL_TRAP(NewPixMap, 0xAA03);
extern pascal trap void C_DisposPixMap(PixMapHandle);
PASCAL_TRAP(DisposPixMap, 0xAA04);
extern pascal trap void C_CopyPixMap(PixMapHandle, PixMapHandle);
PASCAL_TRAP(CopyPixMap, 0xAA05);
extern pascal trap PixPatHandle C_NewPixPat();
PASCAL_TRAP(NewPixPat, 0xAA07);
extern pascal trap void C_DisposPixPat(PixPatHandle);
PASCAL_TRAP(DisposPixPat, 0xAA08);
extern pascal trap void C_CopyPixPat(PixPatHandle, PixPatHandle);
PASCAL_TRAP(CopyPixPat, 0xAA09);

extern pascal trap void C_SetPortPix(PixMapHandle);
PASCAL_TRAP(SetPortPix, 0xAA06);

extern pascal trap GDHandle C_NewGDevice(INTEGER, LONGINT);
PASCAL_TRAP(NewGDevice, 0xAA2F);
extern pascal trap void C_InitGDevice(INTEGER, LONGINT, GDHandle);
PASCAL_TRAP(InitGDevice, 0xAA2E);
extern pascal trap void C_SetDeviceAttribute(GDHandle, INTEGER, BOOLEAN);
PASCAL_TRAP(SetDeviceAttribute, 0xAA2D);
extern pascal trap void C_SetGDevice(GDHandle);
PASCAL_TRAP(SetGDevice, 0xAA31);

extern pascal trap void C_DisposeGDevice(GDHandle);
PASCAL_FUNCTION(DisposeGDevice);
extern pascal trap GDHandle C_GetGDevice();
PASCAL_TRAP(GetGDevice, 0xAA32);
extern pascal trap GDHandle C_GetDeviceList();
PASCAL_TRAP(GetDeviceList, 0xAA29);
extern pascal trap GDHandle C_GetMainDevice();
PASCAL_TRAP(GetMainDevice, 0xAA2A);
extern pascal trap GDHandle C_GetMaxDevice(Rect *);
PASCAL_TRAP(GetMaxDevice, 0xAA27);
extern pascal trap GDHandle C_GetNextDevice(GDHandle);
PASCAL_TRAP(GetNextDevice, 0xAA2B);
extern pascal trap void C_DeviceLoop(RgnHandle, DeviceLoopDrawingProcPtr, LONGINT, DeviceLoopFlags);
PASCAL_TRAP(DeviceLoop, 0xABCA);
extern pascal trap BOOLEAN C_TestDeviceAttribute(GDHandle, INTEGER);
PASCAL_TRAP(TestDeviceAttribute, 0xAA2C);
extern pascal trap void C_ScreenRes(GUEST<INTEGER> *, GUEST<INTEGER> *);
PASCAL_FUNCTION(ScreenRes);
extern pascal trap INTEGER C_HasDepth(GDHandle, INTEGER, INTEGER, INTEGER);
PASCAL_FUNCTION(HasDepth);
extern pascal trap OSErr C_SetDepth(GDHandle, INTEGER, INTEGER, INTEGER);
PASCAL_FUNCTION(SetDepth);

extern pascal trap void C_MakeITable(CTabHandle, ITabHandle, INTEGER);
PASCAL_TRAP(MakeITable, 0xAA39);

extern pascal trap LONGINT C_Color2Index(RGBColor *);
PASCAL_TRAP(Color2Index, 0xAA33);
extern pascal trap void C_Index2Color(LONGINT, RGBColor *);
PASCAL_TRAP(Index2Color, 0xAA34);
extern pascal trap LONGINT C_GetCTSeed();
PASCAL_TRAP(GetCTSeed, 0xAA28);
extern pascal trap void C_GetSubTable(CTabHandle, INTEGER, CTabHandle);
PASCAL_TRAP(GetSubTable, 0xAA37);

extern pascal trap void C_FillCRoundRect(const Rect *, short, short, PixPatHandle);
PASCAL_TRAP(FillCRoundRect, 0xAA10);
extern pascal trap void C_FillCRect(Rect *, PixPatHandle);
PASCAL_TRAP(FillCRect, 0xAA0E);
extern pascal trap void C_FillCOval(const Rect *, PixPatHandle);
PASCAL_TRAP(FillCOval, 0xAA0F);
extern pascal trap void C_FillCArc(const Rect *, short, short, PixPatHandle);
PASCAL_TRAP(FillCArc, 0xAA11);
extern pascal trap void C_FillCPoly(PolyHandle, PixPatHandle);
PASCAL_TRAP(FillCPoly, 0xAA13);
extern pascal trap void C_FillCRgn(RgnHandle, PixPatHandle);
PASCAL_TRAP(FillCRgn, 0xAA12);

extern pascal trap void C_InvertColor(RGBColor *);
PASCAL_TRAP(InvertColor, 0xAA35);
extern pascal trap BOOLEAN C_RealColor(RGBColor *);
PASCAL_TRAP(RealColor, 0xAA36);
extern pascal trap void C_ProtectEntry(INTEGER, BOOLEAN);
PASCAL_TRAP(ProtectEntry, 0xAA3D);
extern pascal trap void C_ReserveEntry(INTEGER, BOOLEAN);
PASCAL_TRAP(ReserveEntry, 0xAA3E);
extern pascal trap void C_SetEntries(INTEGER, INTEGER, ColorSpec * /* cSpecArray */);
PASCAL_TRAP(SetEntries, 0xAA3F);
extern pascal trap void C_AddSearch(ProcPtr);
PASCAL_TRAP(AddSearch, 0xAA3A);
extern pascal trap void C_AddComp(ProcPtr);
PASCAL_TRAP(AddComp, 0xAA3B);
extern pascal trap void C_DelSearch(ProcPtr);
PASCAL_TRAP(DelSearch, 0xAA4C);
extern pascal trap void C_DelComp(ProcPtr);
PASCAL_TRAP(DelComp, 0xAA4D);
extern pascal trap void C_SetClientID(INTEGER);
PASCAL_TRAP(SetClientID, 0xAA3C);

extern pascal trap BOOLEAN C_GetGray(GDHandle, RGBColor *, RGBColor *);
PASCAL_FUNCTION(GetGray);

extern pascal trap PixPatHandle C_GetPixPat(INTEGER);
PASCAL_TRAP(GetPixPat, 0xAA0C);

extern pascal trap INTEGER C_QDError();
PASCAL_TRAP(QDError, 0xAA40);

extern pascal trap CWindowPtr C_NewCWindow(Ptr, Rect *, StringPtr, BOOLEAN, INTEGER, CWindowPtr, BOOLEAN, LONGINT);
PASCAL_TRAP(NewCWindow, 0xAA45);
extern pascal trap CWindowPtr C_GetNewCWindow(INTEGER, Ptr, CWindowPtr);
PASCAL_TRAP(GetNewCWindow, 0xAA46);

extern pascal trap void C_CMY2RGB(CMYColor *, RGBColor *);
PASCAL_FUNCTION(CMY2RGB);
extern pascal trap void C_RGB2CMY(RGBColor *, CMYColor *);
PASCAL_FUNCTION(RGB2CMY);
extern pascal trap void C_HSL2RGB(HSLColor *, RGBColor *);
PASCAL_FUNCTION(HSL2RGB);
extern pascal trap void C_RGB2HSL(RGBColor *, HSLColor *);
PASCAL_FUNCTION(RGB2HSL);
extern pascal trap void C_HSV2RGB(HSVColor *, RGBColor *);
PASCAL_FUNCTION(HSV2RGB);
extern pascal trap void C_RGB2HSV(RGBColor *, HSVColor *);
PASCAL_FUNCTION(RGB2HSV);
extern pascal trap SmallFract C_Fix2SmallFract(Fixed);
PASCAL_FUNCTION(Fix2SmallFract);
extern pascal trap Fixed C_SmallFract2Fix(SmallFract);
PASCAL_FUNCTION(SmallFract2Fix);
extern pascal trap BOOLEAN C_GetColor(Point, Str255, RGBColor *, RGBColor *);
PASCAL_FUNCTION(GetColor);

extern pascal trap CTabHandle C_GetCTable(INTEGER);
PASCAL_TRAP(GetCTable, 0xAA18);
extern pascal trap void C_DisposCTable(CTabHandle);
PASCAL_TRAP(DisposCTable, 0xAA24);

extern pascal trap void C_InitPalettes();
PASCAL_TRAP(InitPalettes, 0xAA90);
extern pascal trap PaletteHandle C_NewPalette(INTEGER, CTabHandle, INTEGER, INTEGER);
PASCAL_TRAP(NewPalette, 0xAA91);
extern pascal trap PaletteHandle C_GetNewPalette(INTEGER);
PASCAL_TRAP(GetNewPalette, 0xAA92);
extern pascal trap void C_DisposePalette(PaletteHandle);
PASCAL_TRAP(DisposePalette, 0xAA93);
extern pascal trap void C_ActivatePalette(WindowPtr);
PASCAL_TRAP(ActivatePalette, 0xAA94);
extern pascal trap void C_SetPalette(WindowPtr, PaletteHandle, BOOLEAN);
PASCAL_FUNCTION(SetPalette);
extern pascal trap void C_NSetPalette(WindowPtr, PaletteHandle, INTEGER updates);
PASCAL_TRAP(NSetPalette, 0xAA95);
extern pascal trap PaletteHandle C_GetPalette(WindowPtr);
PASCAL_TRAP(GetPalette, 0xAA96);

extern pascal trap void C_PmForeColor(INTEGER);
PASCAL_TRAP(PmForeColor, 0xAA97);
extern pascal trap void C_PmBackColor(INTEGER);
PASCAL_TRAP(PmBackColor, 0xAA98);
extern pascal trap void C_AnimateEntry(WindowPtr, INTEGER, RGBColor *);
PASCAL_TRAP(AnimateEntry, 0xAA99);
extern pascal trap void C_AnimatePalette(WindowPtr, CTabHandle, INTEGER, INTEGER, INTEGER);
PASCAL_TRAP(AnimatePalette, 0xAA9A);
extern pascal trap void C_GetEntryColor(PaletteHandle, INTEGER, RGBColor *);
PASCAL_TRAP(GetEntryColor, 0xAA9B);
extern pascal trap void C_SetEntryColor(PaletteHandle, INTEGER, RGBColor *);
PASCAL_TRAP(SetEntryColor, 0xAA9C);
extern pascal trap void C_GetEntryUsage(PaletteHandle, INTEGER, GUEST<INTEGER> *, GUEST<INTEGER> *);
PASCAL_TRAP(GetEntryUsage, 0xAA9D);
extern pascal trap void C_SetEntryUsage(PaletteHandle, INTEGER, INTEGER, INTEGER);
PASCAL_TRAP(SetEntryUsage, 0xAA9E);
extern pascal trap void C_CTab2Palette(CTabHandle, PaletteHandle, INTEGER, INTEGER);
PASCAL_TRAP(CTab2Palette, 0xAA9F);
extern pascal trap void C_Palette2CTab(PaletteHandle, CTabHandle);
PASCAL_TRAP(Palette2CTab, 0xAAA0);

extern pascal trap CCrsrHandle C_GetCCursor(INTEGER);
PASCAL_TRAP(GetCCursor, 0xAA1B);
extern pascal trap void C_SetCCursor(CCrsrHandle);
PASCAL_TRAP(SetCCursor, 0xAA1C);
extern pascal trap void C_DisposCCursor(CCrsrHandle);
PASCAL_TRAP(DisposCCursor, 0xAA26);
extern pascal trap void C_AllocCursor(void);
PASCAL_TRAP(AllocCursor, 0xAA1D);

extern pascal trap void C_RestoreClutDevice(GDHandle);
PASCAL_FUNCTION(RestoreClutDevice);
extern pascal trap void C_ResizePalette(PaletteHandle, INTEGER);
PASCAL_FUNCTION(ResizePalette);
extern pascal trap INTEGER C_PMgrVersion();
PASCAL_FUNCTION(PMgrVersion);
extern pascal trap void C_SaveFore(ColorSpec *);
PASCAL_FUNCTION(SaveFore);
extern pascal trap void C_RestoreFore(ColorSpec *);
PASCAL_FUNCTION(RestoreFore);
extern pascal trap void C_SaveBack(ColorSpec *);
PASCAL_FUNCTION(SaveBack);
extern pascal trap void C_RestoreBack(ColorSpec *);
PASCAL_FUNCTION(RestoreBack);
extern pascal trap void C_SetPaletteUpdates(PaletteHandle, INTEGER);
PASCAL_FUNCTION(SetPaletteUpdates);
extern pascal trap INTEGER C_GetPaletteUpdates(PaletteHandle);
PASCAL_FUNCTION(GetPaletteUpdates);
extern pascal trap void C_CopyPalette(PaletteHandle src_palette,
                                      PaletteHandle dst_palette,
                                      int16_t src_start, int16_t dst_start,
                                      int16_t n_entries);
PASCAL_TRAP(CopyPalette, 0xAAA1);

extern pascal trap void C_GetCWMgrPort(GUEST<CGrafPtr> *);
PASCAL_TRAP(GetCWMgrPort, 0xAA48);

/* QDExtensions trap */
extern pascal trap QDErr C_NewGWorld(GUEST<GWorldPtr> *, INTEGER, Rect *, CTabHandle, GDHandle, GWorldFlags);
PASCAL_FUNCTION(NewGWorld);
extern pascal trap Boolean C_LockPixels(PixMapHandle);
PASCAL_FUNCTION(LockPixels);
extern pascal trap void C_UnlockPixels(PixMapHandle);
PASCAL_FUNCTION(UnlockPixels);
extern pascal trap GWorldFlags C_UpdateGWorld(GUEST<GWorldPtr> *, INTEGER, Rect *, CTabHandle, GDHandle, GWorldFlags);
PASCAL_FUNCTION(UpdateGWorld);
extern pascal trap void C_DisposeGWorld(GWorldPtr);
PASCAL_FUNCTION(DisposeGWorld);
extern pascal trap void C_GetGWorld(GUEST<CGrafPtr> *, GUEST<GDHandle> *);
PASCAL_FUNCTION(GetGWorld);
extern pascal trap void C_SetGWorld(CGrafPtr, GDHandle);
PASCAL_FUNCTION(SetGWorld);
extern pascal trap void C_AllowPurgePixels(PixMapHandle);
PASCAL_FUNCTION(AllowPurgePixels);
extern pascal trap void C_NoPurgePixels(PixMapHandle);
PASCAL_FUNCTION(NoPurgePixels);
extern pascal trap GWorldFlags C_GetPixelsState(PixMapHandle);
PASCAL_FUNCTION(GetPixelsState);
extern pascal trap void C_SetPixelsState(PixMapHandle, GWorldFlags);
PASCAL_FUNCTION(SetPixelsState);
extern pascal trap Ptr C_GetPixBaseAddr(PixMapHandle);
PASCAL_FUNCTION(GetPixBaseAddr);
extern pascal trap QDErr C_NewScreenBuffer(Rect *, Boolean, GUEST<GDHandle> *, GUEST<PixMapHandle> *);
PASCAL_FUNCTION(NewScreenBuffer);
extern pascal trap void C_DisposeScreenBuffer(PixMapHandle);
PASCAL_FUNCTION(DisposeScreenBuffer);
extern pascal trap GDHandle C_GetGWorldDevice(GWorldPtr);
PASCAL_FUNCTION(GetGWorldDevice);
extern pascal trap Boolean C_PixMap32Bit(PixMapHandle);
PASCAL_FUNCTION(PixMap32Bit);
extern pascal trap PixMapHandle C_GetGWorldPixMap(GWorldPtr);
PASCAL_FUNCTION(GetGWorldPixMap);
extern pascal trap QDErr C_NewTempScreenBuffer(Rect *, Boolean, GUEST<GDHandle> *, GUEST<PixMapHandle> *);
PASCAL_FUNCTION(NewTempScreenBuffer);
extern pascal trap void C_GDeviceChanged(GDHandle);
PASCAL_FUNCTION(GDeviceChanged);
extern pascal trap void C_PortChanged(GrafPtr);
PASCAL_FUNCTION(PortChanged);
extern pascal trap void C_PixPatChanged(PixPatHandle);
PASCAL_FUNCTION(PixPatChanged);
extern pascal trap void C_CTabChanged(CTabHandle);
PASCAL_FUNCTION(CTabChanged);
extern pascal trap Boolean C_QDDone(GrafPtr);
PASCAL_FUNCTION(QDDone);

extern pascal trap LONGINT C_OffscreenVersion();
PASCAL_FUNCTION(OffscreenVersion);

extern pascal trap OSErr C_BitMapToRegion(RgnHandle, const BitMap *);
PASCAL_TRAP(BitMapToRegion, 0xA8D7);

extern pascal trap LONGINT C_Entry2Index(INTEGER);
PASCAL_FUNCTION(Entry2Index);
extern pascal trap void C_SaveEntries(CTabHandle, CTabHandle, ReqListRec *);
PASCAL_TRAP(SaveEntries, 0xAA49);
extern pascal trap void C_RestoreEntries(CTabHandle, CTabHandle, ReqListRec *);
PASCAL_TRAP(RestoreEntries, 0xAA4A);

extern pascal trap void C_DisposGDevice(GDHandle gdh);
PASCAL_TRAP(DisposGDevice, 0xAA30);

extern pascal trap OSErr C_DisposePictInfo(PictInfoID);
PASCAL_FUNCTION(DisposePictInfo);
extern pascal trap OSErr C_RecordPictInfo(PictInfoID, PicHandle);
PASCAL_FUNCTION(RecordPictInfo);
extern pascal trap OSErr C_RecordPixMapInfo(PictInfoID, PixMapHandle);
PASCAL_FUNCTION(RecordPixMapInfo);
extern pascal trap OSErr C_RetrievePictInfo(PictInfoID, PictInfo *, int16_t);
PASCAL_FUNCTION(RetrievePictInfo);
extern pascal trap OSErr C_NewPictInfo(GUEST<PictInfoID> *, int16_t, int16_t, int16_t, int16_t);
PASCAL_FUNCTION(NewPictInfo);
extern pascal trap OSErr C_GetPictInfo(PicHandle, PictInfo *, int16_t, int16_t, int16_t, int16_t);
PASCAL_FUNCTION(GetPictInfo);
extern pascal trap OSErr C_GetPixMapInfo(PixMapHandle, PictInfo *, int16_t, int16_t, int16_t, int16_t);
PASCAL_FUNCTION(GetPixMapInfo);

extern pascal trap PicHandle C_OpenCPicture(OpenCPicParams *newheaderp);
PASCAL_TRAP(OpenCPicture, 0xAA20);

#if 0
#if !defined(TheGDevice_H)
extern GUEST<GDHandle>	TheGDevice_H;
extern GUEST<GDHandle>	MainDevice_H;
extern GUEST<GDHandle>	DeviceList_H;
#endif

#if SIZEOF_CHAR_P == 4 && !FORCE_EXPERIMENTAL_PACKED_MACROS

enum
{
    TheGDevice = (TheGDevice_H.p),
    MainDevice = (MainDevice_H.p),
    DeviceList = (DeviceList_H.p),
};

#else

enum
{
    TheGDevice = ((decltype(TheGDevice_H.type[0]))(TheGDevice_H.pp)),
    MainDevice = ((decltype MainDevice_H.type[0])(MainDevice_H.p)),
    DeviceList = ((decltype DeviceList_H.type[0])(DeviceList_H.p)),
};

#endif
#endif
}
#endif /* _CQUICKDRAW_H_ */
