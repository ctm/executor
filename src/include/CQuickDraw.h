
#if !defined (_CQUICKDRAW_H_)
#define _CQUICKDRAW_H_

#include "WindowMgr.h"

namespace Executor {
#define theCPort	(STARH (STARH ((GUEST<GUEST<CGrafPtr>*> *) SYN68K_TO_US(a5))))
#define theCPortX	((*STARH ((GUEST<GUEST<CGrafPtr>*> *) SYN68K_TO_US(a5))))

#define minSeed 1024

    /* can't use [0];
     make this an unsigned char even tho the mac has SignedByte;
     it is treated as unsigned */
typedef struct ITab { GUEST_STRUCT;
    GUEST< LONGINT> iTabSeed;
    GUEST< INTEGER> iTabRes;
    GUEST< unsigned char[1]> iTTable;
} *ITabPtr;


typedef GUEST<ITabPtr> *ITabHandle;

typedef struct GDevice *GDPtr;
typedef GDPtr GDevicePtr;

typedef GUEST<GDevicePtr> *GDHandle;


typedef struct SProcRec *SProcPtr;

typedef GUEST<SProcPtr> *SProcHndl;
struct SProcRec { GUEST_STRUCT;
    GUEST< SProcHndl> nxtSrch;
    GUEST< ProcPtr> srchProc;
};

typedef struct CProcRec *CProcPtr;


typedef GUEST<CProcPtr> *CProcHndl;
struct CProcRec { GUEST_STRUCT;
    GUEST< CProcHndl> nxtComp;
    GUEST< ProcPtr> compProc;
};

typedef void *DeviceLoopDrawingProcPtr;

struct GDevice { GUEST_STRUCT;
    GUEST< INTEGER> gdRefNum;
    GUEST< INTEGER> gdID;
    GUEST< INTEGER> gdType;
    GUEST< ITabHandle> gdITable;
    GUEST< INTEGER> gdResPref;
    GUEST< SProcHndl> gdSearchProc;
    GUEST< CProcHndl> gdCompProc;
    GUEST< INTEGER> gdFlags;
    GUEST< PixMapHandle> gdPMap;
    GUEST< LONGINT> gdRefCon;
    GUEST< GDHandle> gdNextGD;
    GUEST< Rect> gdRect;
    GUEST< LONGINT> gdMode;
    GUEST< INTEGER> gdCCBytes;
    GUEST< INTEGER> gdCCDepth;
    GUEST< Handle> gdCCXData;
    GUEST< Handle> gdCCXMask;
    GUEST< LONGINT> gdReserved;
};

typedef uint32 DeviceLoopFlags;

/* DeviceLoop flags. */
#define singleDevices	(1 << 0)
#define dontMatchSeeds	(1 << 1)
#define allDevices	(1 << 2)

struct ColorInfo {
    GUEST_STRUCT;
    GUEST< RGBColor> ciRGB;
    GUEST< INTEGER> ciUsage;
    GUEST< INTEGER> ciTolerance;
    GUEST< INTEGER> ciFlags;
    GUEST< LONGINT> ciPrivate;
};

typedef struct Palette { GUEST_STRUCT;
    GUEST< INTEGER> pmEntries;
    GUEST< GrafPtr> pmWindow;
    GUEST< INTEGER> pmPrivate;
    GUEST< LONGINT> pmDevices;    /* Handle? */
    GUEST< Handle> pmSeeds;
    GUEST< ColorInfo[1]> pmInfo;
} *PalettePtr;

typedef enum 
{
#define CI_USAGE_TYPE_BITS_X  (CWC (0xF))
#define CI_USAGE_TYPE_BITS    (0xF)
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

typedef enum
{
  pmNoUpdates_enum = 0x8000,
  pmBkUpdates_enum = 0xA000,
  pmFgUpdates_enum = 0xC000,
  pmAllUpdates_enum = 0xE000,
} pmUpdates;

#define pmNoUpdates ((pmUpdates) pmNoUpdates_enum)
#define pmBkUpdates ((pmUpdates) pmBkUpdates_enum)
#define pmFgUpdates ((pmUpdates) pmFgUpdates_enum)
#define pmAllUpdates ((pmUpdates) pmAllUpdates_enum)


typedef GUEST<PalettePtr> *PaletteHandle;

/* return TRUE if `maybe_graphics_world' points to a graphics world,
   and not a graf port or cgraf port */
#define GRAPHICS_WORLD_P(maybe_graphics_world)			\
  (CGrafPort_p(maybe_graphics_world)				\
   && CPORT_VERSION_X (maybe_graphics_world) & GW_FLAG_BIT_X)

#define GW_CPORT(graphics_world)	((CGrafPtr) (graphics_world))

typedef LONGINT GWorldFlags;

typedef CGrafPort GWorld, *GWorldPtr;

struct ReqListRec { GUEST_STRUCT;
    GUEST< INTEGER> reqLSize;
    GUEST< INTEGER[1]> reqLData;
};

/* extended version 2 picture datastructures */

struct OpenCPicParams { GUEST_STRUCT;
    GUEST< Rect> srcRect;
    GUEST< Fixed> hRes;
    GUEST< Fixed> vRes;
    GUEST< int16> version;
    GUEST< int16> reserved1;
    GUEST< int32> reserved2;
};

typedef struct CommonSpec { GUEST_STRUCT;
    GUEST< int16> count;
    GUEST< int16> ID;
} CommentSpec;

typedef CommentSpec *CommentSpecPtr;

typedef GUEST<CommentSpecPtr> *CommentSpecHandle;

struct FontSpec { GUEST_STRUCT;
    GUEST< int16> pictFontID;
    GUEST< int16> sysFontID;
    GUEST< int32[4]> size;
    GUEST< int16> style;
    GUEST< int32> nameOffset;
};

typedef FontSpec *FontSpecPtr;

typedef GUEST<FontSpecPtr> *FontSpecHandle;





struct PictInfo { GUEST_STRUCT;
    GUEST< int16> version;    /* 0 */
    GUEST< int32> uniqueColors;    /* 2 */
    GUEST< PaletteHandle> thePalette;    /* 6 */
    GUEST< CTabHandle> theColorTable;    /* 10 */
    GUEST< Fixed> hRes;    /* 14 */
    GUEST< Fixed> vRes;    /* 18 */
    GUEST< INTEGER> depth;    /* 22 */
    GUEST< Rect> sourceRect;    /* top24, left26, bottom28, right30 */
    GUEST< int32> textCount;    /* 32 */
    GUEST< int32> lineCount;
    GUEST< int32> rectCount;
    GUEST< int32> rRectCount;
    GUEST< int32> ovalCount;
    GUEST< int32> arcCount;
    GUEST< int32> polyCount;
    GUEST< int32> regionCount;
    GUEST< int32> bitMapCount;
    GUEST< int32> pixMapCount;
    GUEST< int32> commentCount;
    GUEST< int32> uniqueComments;
    GUEST< CommentSpecHandle> commentHandle;
    GUEST< int32> uniqueFonts;
    GUEST< FontSpecHandle> fontHandle;
    GUEST< Handle> fontNamesHandle;
    GUEST< int32> reserved1;
    GUEST< int32> reserved2;
};

typedef PictInfo *PictInfoPtr;

typedef GUEST<PictInfoPtr> *PictInfoHandle;

typedef int32 PictInfoID;

#define RGBDirect (0x10)
#define Indirect  (0)
/* a pixmap pixelType of `native_rgb_pixel_type' means that the format
   of the pixmap is the same as that of the screen */
#define vdriver_rgb_pixel_type (0xB9)

#define pixPurge	(1 << 0)
#define noNewDevice	(1 << 1)
#define useTempMem	(1 << 2)
#define keepLocal	(1 << 3)
#define pixelsPurgeable	(1 << 6)
#define pixelsLocked	(1 << 7)
#define mapPix		(1 << 16)
#define newDepth	(1 << 17)
#define alignPix	(1 << 18)
#define newRowBytes	(1 << 19)
#define reallocPix	(1 << 20)
#define clipPix		(1 << 28)
#define stretchPix	(1 << 29)
#define ditherPix	(1 << 30)
#define gwFlagErr	(1 << 31)

typedef int16 QDErr;

/* error codes returned by QDError */
#define noErr			0
#define paramErr		(-50)
#define noMemForPictPlaybackErr	(-145)
#define regionTooBigErr		(-147)
#define pixmapTooDeepErr	(-148)
#define nsStackErr		(-149)
#define cMatchErr		(-150)
#define cTempMemErr		(-151)
#define cNoMemErr		(-152)
#define cRangeErr		(-153)
#define cProtectErr		(-154)
#define cDevErr			(-155)
#define cResErr			(-156)
#define cDepthErr		(-157)
#define rgnTooBigErr		(-500)

/* TODO:  FIXME -- #warning find out correct value for colReqErr */
/*	  -158 is just a guess */

#define colReqErr		(-158)

extern pascal trap void C_SetStdCProcs(CQDProcs *cProcs);

extern pascal trap void C_OpenCPort (CGrafPtr);
extern pascal trap void C_InitCPort (CGrafPtr);
extern pascal trap void C_CloseCPort (CGrafPtr);
extern pascal trap void C_RGBForeColor (RGBColor *);
extern pascal trap void C_RGBBackColor (RGBColor *);
extern pascal trap void C_GetForeColor (RGBColor *);
extern pascal trap void C_GetBackColor (RGBColor *);
extern pascal trap void C_PenPixPat (PixPatHandle);
extern pascal trap void C_BackPixPat (PixPatHandle);
extern pascal trap void C_OpColor (RGBColor *);
extern pascal trap void C_HiliteColor (RGBColor *);
extern pascal trap PixMapHandle C_NewPixMap ();
extern pascal trap void C_DisposPixMap (PixMapHandle);
extern pascal trap void C_CopyPixMap (PixMapHandle, PixMapHandle);
extern pascal trap PixPatHandle C_NewPixPat ();
extern pascal trap void C_DisposPixPat (PixPatHandle);
extern pascal trap void C_CopyPixPat (PixPatHandle, PixPatHandle);

extern pascal trap void C_SetPortPix (PixMapHandle);

extern pascal trap GDHandle C_NewGDevice (INTEGER, LONGINT);
extern pascal trap void C_InitGDevice (INTEGER, LONGINT, GDHandle);
extern pascal trap void C_SetDeviceAttribute (GDHandle, INTEGER, BOOLEAN);
extern pascal trap void C_SetGDevice (GDHandle);

extern pascal trap void C_DisposeGDevice (GDHandle);
extern pascal trap GDHandle C_GetGDevice ();
extern pascal trap GDHandle C_GetDeviceList ();
extern pascal trap GDHandle C_GetMainDevice ();
extern pascal trap GDHandle C_GetMaxDevice (Rect *);
extern pascal trap GDHandle C_GetNextDevice (GDHandle);
extern pascal trap void C_DeviceLoop (RgnHandle, DeviceLoopDrawingProcPtr, LONGINT, DeviceLoopFlags);
extern pascal trap BOOLEAN C_TestDeviceAttribute (GDHandle, INTEGER);
extern pascal trap void C_ScreenRes (GUEST<INTEGER> *, GUEST<INTEGER> *);
extern pascal trap INTEGER C_HasDepth (GDHandle, INTEGER, INTEGER, INTEGER);
extern pascal trap OSErr C_SetDepth (GDHandle, INTEGER, INTEGER, INTEGER);

extern pascal trap void C_MakeITable (CTabHandle, ITabHandle, INTEGER);

extern pascal trap LONGINT C_Color2Index (RGBColor *);
extern pascal trap void C_Index2Color (LONGINT, RGBColor *);
extern pascal trap LONGINT C_GetCTSeed ();
extern pascal trap void C_GetSubTable (CTabHandle, INTEGER, CTabHandle);

extern pascal trap void C_FillCRoundRect (const Rect *, short, short, PixPatHandle);
extern pascal trap void C_FillCRect (Rect *, PixPatHandle);
extern pascal trap void C_FillCOval (const Rect *, PixPatHandle);
extern pascal trap void C_FillCArc (const Rect *, short, short, PixPatHandle);
extern pascal trap void C_FillCPoly (PolyHandle, PixPatHandle);
extern pascal trap void C_FillCRgn (RgnHandle, PixPatHandle);

extern pascal trap void C_InvertColor (RGBColor *);
extern pascal trap BOOLEAN C_RealColor (RGBColor *);
extern pascal trap void C_ProtectEntry (INTEGER, BOOLEAN);
extern pascal trap void C_ReserveEntry (INTEGER, BOOLEAN);
extern pascal trap void C_SetEntries (INTEGER, INTEGER, ColorSpec * /* cSpecArray */);
extern pascal trap void C_AddSearch (ProcPtr);
extern pascal trap void C_AddComp (ProcPtr);
extern pascal trap void C_DelSearch (ProcPtr);
extern pascal trap void C_DelComp (ProcPtr);
extern pascal trap void C_SetClientID (INTEGER);

extern pascal trap BOOLEAN C_GetGray (GDHandle, RGBColor *, RGBColor *);

extern pascal trap PixPatHandle C_GetPixPat (INTEGER);

extern pascal trap INTEGER C_QDError ();

extern pascal trap CWindowPtr C_NewCWindow (Ptr, Rect *, StringPtr, BOOLEAN, INTEGER, CWindowPtr, BOOLEAN, LONGINT);
extern pascal trap CWindowPtr C_GetNewCWindow (INTEGER, Ptr, CWindowPtr);

extern pascal trap void C_CMY2RGB (CMYColor *, RGBColor *);
extern pascal trap void C_RGB2CMY (RGBColor *, CMYColor *);
extern pascal trap void C_HSL2RGB (HSLColor *, RGBColor *);
extern pascal trap void C_RGB2HSL (RGBColor *, HSLColor *);
extern pascal trap void C_HSV2RGB (HSVColor *, RGBColor *);
extern pascal trap void C_RGB2HSV (RGBColor *, HSVColor *);
extern pascal trap SmallFract C_Fix2SmallFract (Fixed);
extern pascal trap Fixed C_SmallFract2Fix (SmallFract);
extern pascal trap BOOLEAN C_GetColor (Point, Str255, RGBColor *, RGBColor *);

extern pascal trap CTabHandle C_GetCTable (INTEGER);
extern pascal trap void C_DisposCTable (CTabHandle);

extern pascal trap void C_InitPalettes ();
extern pascal trap PaletteHandle C_NewPalette (INTEGER, CTabHandle, INTEGER, INTEGER);
extern pascal trap PaletteHandle C_GetNewPalette (INTEGER);
extern pascal trap void C_DisposePalette (PaletteHandle);
extern pascal trap void C_ActivatePalette (WindowPtr);
extern pascal trap void C_SetPalette (WindowPtr, PaletteHandle, BOOLEAN);
extern pascal trap void C_NSetPalette (WindowPtr, PaletteHandle, INTEGER updates);
extern pascal trap PaletteHandle C_GetPalette (WindowPtr);

extern pascal trap void C_PmForeColor (INTEGER);
extern pascal trap void C_PmBackColor (INTEGER);
extern pascal trap void C_AnimateEntry (WindowPtr, INTEGER, RGBColor *);
extern pascal trap void C_AnimatePalette (WindowPtr, CTabHandle, INTEGER, INTEGER, INTEGER);
extern pascal trap void C_GetEntryColor (PaletteHandle, INTEGER,  RGBColor *);
extern pascal trap void C_SetEntryColor (PaletteHandle, INTEGER, RGBColor *);
extern pascal trap void C_GetEntryUsage (PaletteHandle, INTEGER, GUEST<INTEGER> *, GUEST<INTEGER> *);
extern pascal trap void C_SetEntryUsage (PaletteHandle, INTEGER, INTEGER, INTEGER);
extern pascal trap void C_CTab2Palette (CTabHandle, PaletteHandle, INTEGER, INTEGER);
extern pascal trap void C_Palette2CTab (PaletteHandle, CTabHandle);

extern pascal trap CCrsrHandle C_GetCCursor (INTEGER);
extern pascal trap void C_SetCCursor (CCrsrHandle);
extern pascal trap void C_DisposCCursor (CCrsrHandle);
extern pascal trap void C_AllocCursor (void);

extern pascal trap void C_RestoreClutDevice (GDHandle);
extern pascal trap void C_ResizePalette (PaletteHandle, INTEGER);
extern pascal trap INTEGER C_PMgrVersion ();
extern pascal trap void C_SaveFore (ColorSpec *);
extern pascal trap void C_RestoreFore (ColorSpec *);
extern pascal trap void C_SaveBack (ColorSpec *);
extern pascal trap void C_RestoreBack (ColorSpec *);
extern pascal trap void C_SetPaletteUpdates (PaletteHandle, INTEGER);
extern pascal trap INTEGER C_GetPaletteUpdates (PaletteHandle);
extern pascal trap void C_CopyPalette (PaletteHandle src_palette,
				       PaletteHandle dst_palette,
				       int16 src_start, int16 dst_start,
				       int16 n_entries);

extern pascal trap void C_SetWinColor (WindowPtr, CTabHandle);
extern pascal trap BOOLEAN C_GetAuxWin (WindowPtr, GUEST<AuxWinHandle> *);

extern pascal trap void C_GetCWMgrPort (GUEST<CGrafPtr> *);

/* QDExtensions trap */
extern pascal trap QDErr C_NewGWorld (GUEST<GWorldPtr> *, INTEGER, Rect *, CTabHandle, GDHandle, GWorldFlags);
extern pascal trap Boolean C_LockPixels (PixMapHandle);
extern pascal trap void C_UnlockPixels (PixMapHandle);
extern pascal trap GWorldFlags C_UpdateGWorld (GUEST<GWorldPtr> *, INTEGER, Rect *, CTabHandle, GDHandle, GWorldFlags);
extern pascal trap void C_DisposeGWorld (GWorldPtr);
extern pascal trap void C_GetGWorld (GUEST<CGrafPtr> *, GUEST<GDHandle> *);
extern pascal trap void C_SetGWorld (CGrafPtr, GDHandle);
extern pascal trap void C_AllowPurgePixels (PixMapHandle);
extern pascal trap void C_NoPurgePixels (PixMapHandle);
extern pascal trap GWorldFlags C_GetPixelsState (PixMapHandle);
extern pascal trap void C_SetPixelsState (PixMapHandle, GWorldFlags);
extern pascal trap Ptr C_GetPixBaseAddr (PixMapHandle);
extern pascal trap QDErr C_NewScreenBuffer (Rect *, Boolean, GUEST<GDHandle> *, GUEST<PixMapHandle> *);
extern pascal trap void C_DisposeScreenBuffer (PixMapHandle);
extern pascal trap GDHandle C_GetGWorldDevice (GWorldPtr);
extern pascal trap Boolean C_PixMap32Bit (PixMapHandle);
extern pascal trap PixMapHandle C_GetGWorldPixMap (GWorldPtr);
extern pascal trap QDErr C_NewTempScreenBuffer (Rect *, Boolean, GUEST<GDHandle> *, GUEST<PixMapHandle> *);
extern pascal trap void C_GDeviceChanged (GDHandle);
extern pascal trap void C_PortChanged (GrafPtr);
extern pascal trap void C_PixPatChanged (PixPatHandle);
extern pascal trap void C_CTabChanged (CTabHandle);
extern pascal trap Boolean C_QDDone (GrafPtr);

extern pascal trap LONGINT C_OffscreenVersion ();

extern pascal trap OSErr C_BitMapToRegion(RgnHandle, const BitMap *);

extern pascal trap LONGINT C_Entry2Index(INTEGER);
extern pascal trap void C_SaveEntries (CTabHandle, CTabHandle, ReqListRec *);
extern pascal trap void C_RestoreEntries (CTabHandle, CTabHandle, ReqListRec *);

extern pascal trap void C_DisposGDevice(GDHandle gdh);

extern pascal trap OSErr C_DisposePictInfo (PictInfoID);
extern pascal trap OSErr C_RecordPictInfo (PictInfoID, PicHandle);
extern pascal trap OSErr C_RecordPixMapInfo (PictInfoID, PixMapHandle);
extern pascal trap OSErr C_RetrievePictInfo (PictInfoID, PictInfo *, int16);
extern pascal trap OSErr C_NewPictInfo (GUEST<PictInfoID> *, int16, int16, int16, int16);
extern pascal trap OSErr C_GetPictInfo (PicHandle, PictInfo *, int16, int16, int16, int16);
extern pascal trap OSErr C_GetPixMapInfo (PixMapHandle, PictInfo *, int16, int16, int16, int16);

extern pascal trap PicHandle C_OpenCPicture (OpenCPicParams *newheaderp);

#if 0
#if !defined (TheGDevice_H)
extern GUEST<GDHandle>	TheGDevice_H;
extern GUEST<GDHandle>	MainDevice_H;
extern GUEST<GDHandle>	DeviceList_H;
#endif

#if SIZEOF_CHAR_P == 4 && !FORCE_EXPERIMENTAL_PACKED_MACROS

#  define TheGDevice	(TheGDevice_H.p)
#  define MainDevice	(MainDevice_H.p)
#  define DeviceList	(DeviceList_H.p)

#else

#  define TheGDevice	((typeof (TheGDevice_H.type[0]))(TheGDevice_H.pp))
#  define MainDevice	((typeof MainDevice_H.type[0])(MainDevice_H.p))
#  define DeviceList	((typeof DeviceList_H.type[0])(DeviceList_H.p))

#endif
#endif

}
#endif /* _CQUICKDRAW_H_ */
