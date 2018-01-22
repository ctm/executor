
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

typedef UPP<void(INTEGER depth, INTEGER deviceFlags, GDHandle targetDevice, LONGINT userData)>
    DeviceLoopDrawingProcPtr;

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

const LowMemGlobal<GDHandle> TheGDevice { 0xCC8 }; // QuickDraw IMV (true);
const LowMemGlobal<GDHandle> MainDevice { 0x8A4 }; // QuickDraw IMV (true);
const LowMemGlobal<GDHandle> DeviceList { 0x8A8 }; // QuickDraw IMV (true);
const LowMemGlobal<RGBColor> HiliteRGB { 0xDA0 }; // QuickDraw IMV-62 (true);

extern void C_SetStdCProcs(CQDProcs *cProcs);
PASCAL_TRAP(SetStdCProcs, 0xAA4E);

extern void C_OpenCPort(CGrafPtr);
PASCAL_TRAP(OpenCPort, 0xAA00);
extern void C_InitCPort(CGrafPtr);
PASCAL_TRAP(InitCPort, 0xAA01);
extern void C_CloseCPort(CGrafPtr);
PASCAL_TRAP(CloseCPort, 0xAA02);
extern void C_RGBForeColor(RGBColor *);
PASCAL_TRAP(RGBForeColor, 0xAA14);
extern void C_RGBBackColor(RGBColor *);
PASCAL_TRAP(RGBBackColor, 0xAA15);
extern void C_GetForeColor(RGBColor *);
PASCAL_TRAP(GetForeColor, 0xAA19);
extern void C_GetBackColor(RGBColor *);
PASCAL_TRAP(GetBackColor, 0xAA1A);
extern void C_PenPixPat(PixPatHandle);
PASCAL_TRAP(PenPixPat, 0xAA0A);
extern void C_BackPixPat(PixPatHandle);
PASCAL_TRAP(BackPixPat, 0xAA0B);
extern void C_OpColor(RGBColor *);
PASCAL_TRAP(OpColor, 0xAA21);
extern void C_HiliteColor(RGBColor *);
PASCAL_TRAP(HiliteColor, 0xAA22);
extern PixMapHandle C_NewPixMap();
PASCAL_TRAP(NewPixMap, 0xAA03);
extern void C_DisposPixMap(PixMapHandle);
PASCAL_TRAP(DisposPixMap, 0xAA04);
extern void C_CopyPixMap(PixMapHandle, PixMapHandle);
PASCAL_TRAP(CopyPixMap, 0xAA05);
extern PixPatHandle C_NewPixPat();
PASCAL_TRAP(NewPixPat, 0xAA07);
extern void C_DisposPixPat(PixPatHandle);
PASCAL_TRAP(DisposPixPat, 0xAA08);
extern void C_CopyPixPat(PixPatHandle, PixPatHandle);
PASCAL_TRAP(CopyPixPat, 0xAA09);

extern void C_SetPortPix(PixMapHandle);
PASCAL_TRAP(SetPortPix, 0xAA06);

extern GDHandle C_NewGDevice(INTEGER, LONGINT);
PASCAL_TRAP(NewGDevice, 0xAA2F);
extern void C_InitGDevice(INTEGER, LONGINT, GDHandle);
PASCAL_TRAP(InitGDevice, 0xAA2E);
extern void C_SetDeviceAttribute(GDHandle, INTEGER, BOOLEAN);
PASCAL_TRAP(SetDeviceAttribute, 0xAA2D);
extern void C_SetGDevice(GDHandle);
PASCAL_TRAP(SetGDevice, 0xAA31);

extern void C_DisposeGDevice(GDHandle);
PASCAL_TRAP(DisposeGDevice, 0xAA30);
extern GDHandle C_GetGDevice();
PASCAL_TRAP(GetGDevice, 0xAA32);
extern GDHandle C_GetDeviceList();
PASCAL_TRAP(GetDeviceList, 0xAA29);
extern GDHandle C_GetMainDevice();
PASCAL_TRAP(GetMainDevice, 0xAA2A);
extern GDHandle C_GetMaxDevice(Rect *);
PASCAL_TRAP(GetMaxDevice, 0xAA27);
extern GDHandle C_GetNextDevice(GDHandle);
PASCAL_TRAP(GetNextDevice, 0xAA2B);
extern void C_DeviceLoop(RgnHandle, DeviceLoopDrawingProcPtr, LONGINT, DeviceLoopFlags);
PASCAL_TRAP(DeviceLoop, 0xABCA);
extern BOOLEAN C_TestDeviceAttribute(GDHandle, INTEGER);
PASCAL_TRAP(TestDeviceAttribute, 0xAA2C);
extern void C_ScreenRes(GUEST<INTEGER> *, GUEST<INTEGER> *);
NOTRAP_FUNCTION(ScreenRes);
extern INTEGER C_HasDepth(GDHandle, INTEGER, INTEGER, INTEGER);
PASCAL_SUBTRAP(HasDepth, 0xAAA2, PaletteDispatch);
extern OSErr C_SetDepth(GDHandle, INTEGER, INTEGER, INTEGER);
PASCAL_SUBTRAP(SetDepth, 0xAAA2, PaletteDispatch);

extern void C_MakeITable(CTabHandle, ITabHandle, INTEGER);
PASCAL_TRAP(MakeITable, 0xAA39);

extern LONGINT C_Color2Index(RGBColor *);
PASCAL_TRAP(Color2Index, 0xAA33);
extern void C_Index2Color(LONGINT, RGBColor *);
PASCAL_TRAP(Index2Color, 0xAA34);
extern LONGINT C_GetCTSeed();
PASCAL_TRAP(GetCTSeed, 0xAA28);
extern void C_GetSubTable(CTabHandle, INTEGER, CTabHandle);
PASCAL_TRAP(GetSubTable, 0xAA37);

extern void C_FillCRoundRect(const Rect *, short, short, PixPatHandle);
PASCAL_TRAP(FillCRoundRect, 0xAA10);
extern void C_FillCRect(Rect *, PixPatHandle);
PASCAL_TRAP(FillCRect, 0xAA0E);
extern void C_FillCOval(const Rect *, PixPatHandle);
PASCAL_TRAP(FillCOval, 0xAA0F);
extern void C_FillCArc(const Rect *, short, short, PixPatHandle);
PASCAL_TRAP(FillCArc, 0xAA11);
extern void C_FillCPoly(PolyHandle, PixPatHandle);
PASCAL_TRAP(FillCPoly, 0xAA13);
extern void C_FillCRgn(RgnHandle, PixPatHandle);
PASCAL_TRAP(FillCRgn, 0xAA12);

extern void C_InvertColor(RGBColor *);
PASCAL_TRAP(InvertColor, 0xAA35);
extern BOOLEAN C_RealColor(RGBColor *);
PASCAL_TRAP(RealColor, 0xAA36);
extern void C_ProtectEntry(INTEGER, BOOLEAN);
PASCAL_TRAP(ProtectEntry, 0xAA3D);
extern void C_ReserveEntry(INTEGER, BOOLEAN);
PASCAL_TRAP(ReserveEntry, 0xAA3E);
extern void C_SetEntries(INTEGER, INTEGER, ColorSpec * /* cSpecArray */);
PASCAL_TRAP(SetEntries, 0xAA3F);
extern void C_AddSearch(ProcPtr);
PASCAL_TRAP(AddSearch, 0xAA3A);
extern void C_AddComp(ProcPtr);
PASCAL_TRAP(AddComp, 0xAA3B);
extern void C_DelSearch(ProcPtr);
PASCAL_TRAP(DelSearch, 0xAA4C);
extern void C_DelComp(ProcPtr);
PASCAL_TRAP(DelComp, 0xAA4D);
extern void C_SetClientID(INTEGER);
PASCAL_TRAP(SetClientID, 0xAA3C);

extern BOOLEAN C_GetGray(GDHandle, RGBColor *, RGBColor *);
PASCAL_SUBTRAP(GetGray, 0xAAA2, PaletteDispatch);

extern PixPatHandle C_GetPixPat(INTEGER);
PASCAL_TRAP(GetPixPat, 0xAA0C);

extern INTEGER C_QDError();
PASCAL_TRAP(QDError, 0xAA40);

extern CWindowPtr C_NewCWindow(Ptr, Rect *, StringPtr, BOOLEAN, INTEGER, CWindowPtr, BOOLEAN, LONGINT);
PASCAL_TRAP(NewCWindow, 0xAA45);
extern CWindowPtr C_GetNewCWindow(INTEGER, Ptr, CWindowPtr);
PASCAL_TRAP(GetNewCWindow, 0xAA46);

extern void C_CMY2RGB(CMYColor *, RGBColor *);
PASCAL_SUBTRAP(CMY2RGB, 0xA82E, Pack12);
extern void C_RGB2CMY(RGBColor *, CMYColor *);
PASCAL_SUBTRAP(RGB2CMY, 0xA82E, Pack12);
extern void C_HSL2RGB(HSLColor *, RGBColor *);
PASCAL_SUBTRAP(HSL2RGB, 0xA82E, Pack12);
extern void C_RGB2HSL(RGBColor *, HSLColor *);
PASCAL_SUBTRAP(RGB2HSL, 0xA82E, Pack12);
extern void C_HSV2RGB(HSVColor *, RGBColor *);
PASCAL_SUBTRAP(HSV2RGB, 0xA82E, Pack12);
extern void C_RGB2HSV(RGBColor *, HSVColor *);
PASCAL_SUBTRAP(RGB2HSV, 0xA82E, Pack12);
extern SmallFract C_Fix2SmallFract(Fixed);
PASCAL_SUBTRAP(Fix2SmallFract, 0xA82E, Pack12);
extern Fixed C_SmallFract2Fix(SmallFract);
PASCAL_SUBTRAP(SmallFract2Fix, 0xA82E, Pack12);
extern BOOLEAN C_GetColor(Point, Str255, RGBColor *, RGBColor *);
PASCAL_SUBTRAP(GetColor, 0xA82E, Pack12);

extern CTabHandle C_GetCTable(INTEGER);
PASCAL_TRAP(GetCTable, 0xAA18);
extern void C_DisposCTable(CTabHandle);
PASCAL_TRAP(DisposCTable, 0xAA24);

extern void C_InitPalettes();
PASCAL_TRAP(InitPalettes, 0xAA90);
extern PaletteHandle C_NewPalette(INTEGER, CTabHandle, INTEGER, INTEGER);
PASCAL_TRAP(NewPalette, 0xAA91);
extern PaletteHandle C_GetNewPalette(INTEGER);
PASCAL_TRAP(GetNewPalette, 0xAA92);
extern void C_DisposePalette(PaletteHandle);
PASCAL_TRAP(DisposePalette, 0xAA93);
extern void C_ActivatePalette(WindowPtr);
PASCAL_TRAP(ActivatePalette, 0xAA94);
extern void C_SetPalette(WindowPtr, PaletteHandle, BOOLEAN);
NOTRAP_FUNCTION(SetPalette);
extern void C_NSetPalette(WindowPtr, PaletteHandle, INTEGER updates);
PASCAL_TRAP(NSetPalette, 0xAA95);
extern PaletteHandle C_GetPalette(WindowPtr);
PASCAL_TRAP(GetPalette, 0xAA96);

extern void C_PmForeColor(INTEGER);
PASCAL_TRAP(PmForeColor, 0xAA97);
extern void C_PmBackColor(INTEGER);
PASCAL_TRAP(PmBackColor, 0xAA98);
extern void C_AnimateEntry(WindowPtr, INTEGER, RGBColor *);
PASCAL_TRAP(AnimateEntry, 0xAA99);
extern void C_AnimatePalette(WindowPtr, CTabHandle, INTEGER, INTEGER, INTEGER);
PASCAL_TRAP(AnimatePalette, 0xAA9A);
extern void C_GetEntryColor(PaletteHandle, INTEGER, RGBColor *);
PASCAL_TRAP(GetEntryColor, 0xAA9B);
extern void C_SetEntryColor(PaletteHandle, INTEGER, RGBColor *);
PASCAL_TRAP(SetEntryColor, 0xAA9C);
extern void C_GetEntryUsage(PaletteHandle, INTEGER, GUEST<INTEGER> *, GUEST<INTEGER> *);
PASCAL_TRAP(GetEntryUsage, 0xAA9D);
extern void C_SetEntryUsage(PaletteHandle, INTEGER, INTEGER, INTEGER);
PASCAL_TRAP(SetEntryUsage, 0xAA9E);
extern void C_CTab2Palette(CTabHandle, PaletteHandle, INTEGER, INTEGER);
PASCAL_TRAP(CTab2Palette, 0xAA9F);
extern void C_Palette2CTab(PaletteHandle, CTabHandle);
PASCAL_TRAP(Palette2CTab, 0xAAA0);

extern CCrsrHandle C_GetCCursor(INTEGER);
PASCAL_TRAP(GetCCursor, 0xAA1B);
extern void C_SetCCursor(CCrsrHandle);
PASCAL_TRAP(SetCCursor, 0xAA1C);
extern void C_DisposCCursor(CCrsrHandle);
PASCAL_TRAP(DisposCCursor, 0xAA26);
extern void C_AllocCursor(void);
PASCAL_TRAP(AllocCursor, 0xAA1D);

extern void C_RestoreDeviceClut(GDHandle);
PASCAL_SUBTRAP(RestoreDeviceClut, 0xAAA2, PaletteDispatch);
extern void C_ResizePalette(PaletteHandle, INTEGER);
PASCAL_SUBTRAP(ResizePalette, 0xAAA2, PaletteDispatch);
extern INTEGER C_PMgrVersion();
PASCAL_SUBTRAP(PMgrVersion, 0xAAA2, PaletteDispatch);
extern void C_SaveFore(ColorSpec *);
PASCAL_SUBTRAP(SaveFore, 0xAAA2, PaletteDispatch);
extern void C_RestoreFore(ColorSpec *);
PASCAL_SUBTRAP(RestoreFore, 0xAAA2, PaletteDispatch);
extern void C_SaveBack(ColorSpec *);
PASCAL_SUBTRAP(SaveBack, 0xAAA2, PaletteDispatch);
extern void C_RestoreBack(ColorSpec *);
PASCAL_SUBTRAP(RestoreBack, 0xAAA2, PaletteDispatch);
extern void C_SetPaletteUpdates(PaletteHandle, INTEGER);
PASCAL_SUBTRAP(SetPaletteUpdates, 0xAAA2, PaletteDispatch);
extern INTEGER C_GetPaletteUpdates(PaletteHandle);
PASCAL_SUBTRAP(GetPaletteUpdates, 0xAAA2, PaletteDispatch);
extern void C_CopyPalette(PaletteHandle src_palette,
                                      PaletteHandle dst_palette,
                                      int16_t src_start, int16_t dst_start,
                                      int16_t n_entries);
PASCAL_TRAP(CopyPalette, 0xAAA1);

extern void C_GetCWMgrPort(GUEST<CGrafPtr> *);
PASCAL_TRAP(GetCWMgrPort, 0xAA48);

/* QDExtensions trap */
extern QDErr C_NewGWorld(GUEST<GWorldPtr> *, INTEGER, Rect *, CTabHandle, GDHandle, GWorldFlags);
PASCAL_SUBTRAP(NewGWorld, 0xAB1D, QDExtensions);
extern Boolean C_LockPixels(PixMapHandle);
PASCAL_SUBTRAP(LockPixels, 0xAB1D, QDExtensions);
extern void C_UnlockPixels(PixMapHandle);
PASCAL_SUBTRAP(UnlockPixels, 0xAB1D, QDExtensions);
extern GWorldFlags C_UpdateGWorld(GUEST<GWorldPtr> *, INTEGER, Rect *, CTabHandle, GDHandle, GWorldFlags);
PASCAL_SUBTRAP(UpdateGWorld, 0xAB1D, QDExtensions);
extern void C_DisposeGWorld(GWorldPtr);
PASCAL_SUBTRAP(DisposeGWorld, 0xAB1D, QDExtensions);
extern void C_GetGWorld(GUEST<CGrafPtr> *, GUEST<GDHandle> *);
PASCAL_SUBTRAP(GetGWorld, 0xAB1D, QDExtensions);
extern void C_SetGWorld(CGrafPtr, GDHandle);
PASCAL_SUBTRAP(SetGWorld, 0xAB1D, QDExtensions);
extern void C_AllowPurgePixels(PixMapHandle);
PASCAL_SUBTRAP(AllowPurgePixels, 0xAB1D, QDExtensions);
extern void C_NoPurgePixels(PixMapHandle);
PASCAL_SUBTRAP(NoPurgePixels, 0xAB1D, QDExtensions);
extern GWorldFlags C_GetPixelsState(PixMapHandle);
PASCAL_SUBTRAP(GetPixelsState, 0xAB1D, QDExtensions);
extern void C_SetPixelsState(PixMapHandle, GWorldFlags);
PASCAL_SUBTRAP(SetPixelsState, 0xAB1D, QDExtensions);
extern Ptr C_GetPixBaseAddr(PixMapHandle);
PASCAL_SUBTRAP(GetPixBaseAddr, 0xAB1D, QDExtensions);
extern QDErr C_NewScreenBuffer(Rect *, Boolean, GUEST<GDHandle> *, GUEST<PixMapHandle> *);
PASCAL_SUBTRAP(NewScreenBuffer, 0xAB1D, QDExtensions);
extern void C_DisposeScreenBuffer(PixMapHandle);
PASCAL_SUBTRAP(DisposeScreenBuffer, 0xAB1D, QDExtensions);
extern GDHandle C_GetGWorldDevice(GWorldPtr);
PASCAL_SUBTRAP(GetGWorldDevice, 0xAB1D, QDExtensions);
extern Boolean C_PixMap32Bit(PixMapHandle);
PASCAL_SUBTRAP(PixMap32Bit, 0xAB1D, QDExtensions);
extern PixMapHandle C_GetGWorldPixMap(GWorldPtr);
PASCAL_SUBTRAP(GetGWorldPixMap, 0xAB1D, QDExtensions);
extern QDErr C_NewTempScreenBuffer(Rect *, Boolean, GUEST<GDHandle> *, GUEST<PixMapHandle> *);
PASCAL_SUBTRAP(NewTempScreenBuffer, 0xAB1D, QDExtensions);
extern void C_GDeviceChanged(GDHandle);
PASCAL_SUBTRAP(GDeviceChanged, 0xAB1D, QDExtensions);
extern void C_PortChanged(GrafPtr);
PASCAL_SUBTRAP(PortChanged, 0xAB1D, QDExtensions);
extern void C_PixPatChanged(PixPatHandle);
PASCAL_SUBTRAP(PixPatChanged, 0xAB1D, QDExtensions);
extern void C_CTabChanged(CTabHandle);
PASCAL_SUBTRAP(CTabChanged, 0xAB1D, QDExtensions);
extern Boolean C_QDDone(GrafPtr);
PASCAL_SUBTRAP(QDDone, 0xAB1D, QDExtensions);

extern LONGINT C_OffscreenVersion();
PASCAL_SUBTRAP(OffscreenVersion, 0xAB1D, QDExtensions);

extern OSErr C_BitMapToRegion(RgnHandle, const BitMap *);
PASCAL_TRAP(BitMapToRegion, 0xA8D7);

extern LONGINT C_Entry2Index(INTEGER);
PASCAL_SUBTRAP(Entry2Index, 0xAAA2, PaletteDispatch);
extern void C_SaveEntries(CTabHandle, CTabHandle, ReqListRec *);
PASCAL_TRAP(SaveEntries, 0xAA49);
extern void C_RestoreEntries(CTabHandle, CTabHandle, ReqListRec *);
PASCAL_TRAP(RestoreEntries, 0xAA4A);

extern OSErr C_DisposePictInfo(PictInfoID);
PASCAL_SUBTRAP(DisposePictInfo, 0xA831, Pack15);
extern OSErr C_RecordPictInfo(PictInfoID, PicHandle);
PASCAL_SUBTRAP(RecordPictInfo, 0xA831, Pack15);
extern OSErr C_RecordPixMapInfo(PictInfoID, PixMapHandle);
PASCAL_SUBTRAP(RecordPixMapInfo, 0xA831, Pack15);
extern OSErr C_RetrievePictInfo(PictInfoID, PictInfo *, int16_t);
PASCAL_SUBTRAP(RetrievePictInfo, 0xA831, Pack15);
extern OSErr C_NewPictInfo(GUEST<PictInfoID> *, int16_t, int16_t, int16_t, int16_t);
PASCAL_SUBTRAP(NewPictInfo, 0xA831, Pack15);
extern OSErr C_GetPictInfo(PicHandle, PictInfo *, int16_t, int16_t, int16_t, int16_t);
PASCAL_SUBTRAP(GetPictInfo, 0xA831, Pack15);
extern OSErr C_GetPixMapInfo(PixMapHandle, PictInfo *, int16_t, int16_t, int16_t, int16_t);
PASCAL_SUBTRAP(GetPixMapInfo, 0xA831, Pack15);

extern PicHandle C_OpenCPicture(OpenCPicParams *newheaderp);
PASCAL_TRAP(OpenCPicture, 0xAA20);
}
#endif /* _CQUICKDRAW_H_ */
