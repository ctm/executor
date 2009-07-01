
#if !defined (_CQUICKDRAW_H_)
#define _CQUICKDRAW_H_

#include "WindowMgr.h"

typedef HIDDEN_CGrafPtr *HIDDEN_CGrafPtr_Ptr;
MAKE_HIDDEN(HIDDEN_CGrafPtr_Ptr);


#define theCPort	(STARH (STARH ((HIDDEN_HIDDEN_CGrafPtr_Ptr *) SYN68K_TO_US(a5))))
#define theCPortX	((*STARH ((HIDDEN_HIDDEN_CGrafPtr_Ptr *) SYN68K_TO_US(a5))).p)

#define minSeed 1024

typedef struct PACKED
{
  LONGINT iTabSeed;
  INTEGER iTabRes;
  /* can't use [0];
     make this an unsigned char even tho the mac has SignedByte;
     it is treated as unsigned */
  unsigned char iTTable[1];
} ITab, *ITabPtr;

MAKE_HIDDEN(ITabPtr);
typedef HIDDEN_ITabPtr *ITabHandle;

typedef struct GDevice *GDPtr;
typedef GDPtr GDevicePtr;
MAKE_HIDDEN(GDevicePtr);
typedef HIDDEN_GDevicePtr *GDHandle;
MAKE_HIDDEN(GDHandle);

typedef struct PACKED SProcRec *SProcPtr;
MAKE_HIDDEN(SProcPtr);
typedef HIDDEN_SProcPtr *SProcHndl;
typedef struct PACKED SProcRec
{
  PACKED_MEMBER(SProcHndl, nxtSrch);
  PACKED_MEMBER(ProcPtr, srchProc);
} SProcRec;

typedef struct PACKED CProcRec *CProcPtr;
MAKE_HIDDEN(CProcPtr);

typedef HIDDEN_CProcPtr *CProcHndl;
typedef struct PACKED CProcRec
{
  PACKED_MEMBER(CProcHndl, nxtComp);
  PACKED_MEMBER(ProcPtr, compProc);
} CProcRec;

typedef void *DeviceLoopDrawingProcPtr;

typedef struct PACKED GDevice
{
  INTEGER gdRefNum;
  INTEGER gdID;
  INTEGER gdType;
  PACKED_MEMBER(ITabHandle, gdITable);
  INTEGER gdResPref;
  PACKED_MEMBER(SProcHndl, gdSearchProc);
  PACKED_MEMBER(CProcHndl, gdCompProc);
  INTEGER gdFlags;
  PACKED_MEMBER(PixMapHandle, gdPMap);
  LONGINT gdRefCon;
  PACKED_MEMBER(GDHandle, gdNextGD);
  Rect gdRect;
  LONGINT gdMode;
  INTEGER gdCCBytes;
  INTEGER gdCCDepth;
  PACKED_MEMBER(Handle, gdCCXData);
  PACKED_MEMBER(Handle, gdCCXMask);
  LONGINT gdReserved;
} GDevice;

typedef uint32 DeviceLoopFlags;

/* DeviceLoop flags. */
#define singleDevices	(1 << 0)
#define dontMatchSeeds	(1 << 1)
#define allDevices	(1 << 2)

typedef struct PACKED ColorInfo
{
  RGBColor ciRGB;
  INTEGER ciUsage;
  INTEGER ciTolerance;
  INTEGER ciFlags;
  LONGINT ciPrivate;
} ColorInfo;

typedef struct PACKED Palette
{
  INTEGER pmEntries;
  PACKED_MEMBER(GrafPtr, pmWindow);
  INTEGER pmPrivate;
  LONGINT /* Handle? */ pmDevices;
  PACKED_MEMBER(Handle,pmSeeds);
  ColorInfo pmInfo[1];
} Palette, *PalettePtr;

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

#define pmNoUpdates ((INTEGER) pmNoUpdates_enum)
#define pmBkUpdates ((INTEGER) pmBkUpdates_enum)
#define pmFgUpdates ((INTEGER) pmFgUpdates_enum)
#define pmAllUpdates ((INTEGER) pmAllUpdates_enum)

MAKE_HIDDEN(PalettePtr);
typedef HIDDEN_PalettePtr *PaletteHandle;

/* return TRUE if `maybe_graphics_world' points to a graphics world,
   and not a graf port or cgraf port */
#define GRAPHICS_WORLD_P(maybe_graphics_world)			\
  (CGrafPort_p(maybe_graphics_world)				\
   && CPORT_VERSION_X (maybe_graphics_world) & GW_FLAG_BIT_X)

#define GW_CPORT(graphics_world)	((CGrafPtr) (graphics_world))

typedef LONGINT GWorldFlags;

typedef CGrafPort GWorld, *GWorldPtr;

typedef struct PACKED ReqListRec
{
  INTEGER reqLSize;
  INTEGER reqLData[1];
} ReqListRec;

/* extended version 2 picture datastructures */

typedef struct PACKED OpenCPicParams
{
  Rect srcRect;
  Fixed hRes;
  Fixed vRes;
  int16 version;
  int16 reserved1;
  int32 reserved2;
} OpenCPicParams;

typedef struct PACKED CommonSpec
{
  int16 count;
  int16 ID;
} CommentSpec;

typedef CommentSpec *CommentSpecPtr;
MAKE_HIDDEN(CommentSpecPtr);
typedef HIDDEN_CommentSpecPtr *CommentSpecHandle;

typedef struct PACKED FontSpec
{
  int16 pictFontID;
  int16 sysFontID;
  int32 size[4];
  int16 style;
  int32 nameOffset;
} FontSpec;

typedef FontSpec *FontSpecPtr;
MAKE_HIDDEN(FontSpecPtr);
typedef HIDDEN_FontSpecPtr *FontSpecHandle;

typedef struct PACKED PictInfo
{
  int16 version; /* 0 */
  int32 uniqueColors; /* 2 */
  PACKED_MEMBER(PaletteHandle, thePalette); /* 6 */
  PACKED_MEMBER(CTabHandle, theColorTable); /* 10 */
  Fixed hRes; /* 14 */
  Fixed vRes; /* 18 */
  INTEGER depth; /* 22 */
  Rect sourceRect; /* top24, left26, bottom28, right30 */
  int32 textCount; /* 32 */
  int32 lineCount;
  int32 rectCount;
  int32 rRectCount;
  int32 ovalCount;
  int32 arcCount;
  int32 polyCount;
  int32 regionCount;
  int32 bitMapCount;
  int32 pixMapCount;
  int32 commentCount;
  
  int32 uniqueComments;
  PACKED_MEMBER(CommentSpecHandle, commentHandle);
  
  int32 uniqueFonts;
  PACKED_MEMBER(FontSpecHandle, fontHandle);

  PACKED_MEMBER(Handle, fontNamesHandle);

  int32 reserved1;
  int32 reserved2;
} PictInfo;

typedef PictInfo *PictInfoPtr;
MAKE_HIDDEN(PictInfoPtr);
typedef HIDDEN_PictInfoPtr *PictInfoHandle;

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
extern pascal trap void C_ScreenRes (INTEGER *, INTEGER *);
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
extern pascal trap void C_GetEntryUsage (PaletteHandle, INTEGER, INTEGER *, INTEGER *);
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
extern pascal trap BOOLEAN C_GetAuxWin (WindowPtr, HIDDEN_AuxWinHandle *);

extern pascal trap void C_GetCWMgrPort (HIDDEN_CGrafPtr *);

/* QDExtensions trap */
extern pascal trap QDErr C_NewGWorld (GWorldPtr *, INTEGER, Rect *, CTabHandle, GDHandle, GWorldFlags);
extern pascal trap Boolean C_LockPixels (PixMapHandle);
extern pascal trap void C_UnlockPixels (PixMapHandle);
extern pascal trap GWorldFlags C_UpdateGWorld (GWorldPtr *, INTEGER, Rect *, CTabHandle, GDHandle, GWorldFlags);
extern pascal trap void C_DisposeGWorld (GWorldPtr);
extern pascal trap void C_GetGWorld (CGrafPtr *, GDHandle *);
extern pascal trap void C_SetGWorld (CGrafPtr, GDHandle);
extern pascal trap void C_AllowPurgePixels (PixMapHandle);
extern pascal trap void C_NoPurgePixels (PixMapHandle);
extern pascal trap GWorldFlags C_GetPixelsState (PixMapHandle);
extern pascal trap void C_SetPixelsState (PixMapHandle, GWorldFlags);
extern pascal trap Ptr C_GetPixBaseAddr (PixMapHandle);
extern pascal trap QDErr C_NewScreenBuffer (Rect *, Boolean, GDHandle *, PixMapHandle *);
extern pascal trap void C_DisposeScreenBuffer (PixMapHandle);
extern pascal trap GDHandle C_GetGWorldDevice (GWorldPtr);
extern pascal trap Boolean C_PixMap32Bit (PixMapHandle);
extern pascal trap PixMapHandle C_GetGWorldPixMap (GWorldPtr);
extern pascal trap QDErr C_NewTempScreenBuffer (Rect *, Boolean, GDHandle *, PixMapHandle *);
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
extern pascal trap OSErr C_NewPictInfo (PictInfoID *, int16, int16, int16, int16);
extern pascal trap OSErr C_GetPictInfo (PicHandle, PictInfo *, int16, int16, int16, int16);
extern pascal trap OSErr C_GetPixMapInfo (PixMapHandle, PictInfo *, int16, int16, int16, int16);

extern pascal trap PicHandle C_OpenCPicture (OpenCPicParams *newheaderp);

#if !defined (TheGDevice_H)
extern HIDDEN_GDHandle	TheGDevice_H;
extern HIDDEN_GDHandle	MainDevice_H;
extern HIDDEN_GDHandle	DeviceList_H;
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

#endif /* _CQUICKDRAW_H_ */
