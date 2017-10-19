#if !defined (_ICON_UTIL_H_)
#define _ICON_UTIL_H_

/* Copyright 1986-1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: Iconutil.h 63 2004-12-24 18:19:43Z ctm $
 */

#define large1BitMask (T ('I', 'C', 'N', '#'))
#define large4BitData (T ('i', 'c', 'l', '4'))
#define large8BitData (T ('i', 'c', 'l', '8'))
#define small1BitMask (T ('i', 'c', 's', '#'))
#define small4BitData (T ('i', 'c', 's', '4'))
#define small8BitData (T ('i', 'c', 's', '8'))
#define mini1BitMask  (T ('i', 'c', 'm', '#'))
#define mini4BitData  (T ('i', 'c', 'm', '4'))
#define mini8BitData  (T ('i', 'c', 'm', '8'))

namespace Executor {
/* IconAlignmentType values */
enum
{
  atNone = 0,
  atVerticalCenter = 1,
  atTop = 2,
  atBottom = 3,
  atHorizontalCenter = 4,
  atAbsoluteCenter = (atVerticalCenter | atHorizontalCenter), /* 5 */
  atCenterTop = (atTop | atHorizontalCenter), /* 6 */
  atCenterBottom = (atBottom | atHorizontalCenter), /* 7 */
  atLeft = 8,
  atCenterLeft = (atVerticalCenter | atLeft), /* 9 */
  atTopLeft = (atTop | atLeft), /* 10 */
  atBottomLeft = (atBottom | atLeft), /* 11 */
  atRight = 12,
  atCenterRight = (atVerticalCenter | atRight), /* 13 */
  atTopRight = (atTop | atRight), /* 14 */
  atBottomRight = (atBottom | atRight), /* 15 */
};

/* IconTranformType values */
enum
{
  ttNone = 0,
  ttDisabled = 1,
  ttOffline = 2,
  ttOpen = 3,
  ttLabel1 = 0x0100,
  ttLabel2 = 0x0200,
  ttLabel3 = 0x0300,
  ttLabel4 = 0x0400,
  ttLabel5 = 0x0500,
  ttLabel6 = 0x0600,
  ttLabel7 = 0x0700,
  ttSelected = 0x4000,
  ttSelectedDisabled = (ttSelected | ttDisabled),
  ttSelectedOffline = (ttSelected | ttOffline),
  ttSelectedOpen = (ttSelected | ttOpen),
};

/* IconSelectorValue values */
/* #### what kind of eediot at apple named all the other icon flag
   types *Type, except this one? */
enum
{
  svLarge1Bit = 0x01,
  svLarge4Bit = 0x02,
  svLarge8Bit = 0x04,
  svSmall1Bit = 0x0100,
  svSmall4Bit = 0x0200,
  svSmall8Bit = 0x0400,
  svMini1Bit = 0x010000,
  svMini4Bit = 0x020000,
  svMini8Bit = 0x040000,
  
  svAllLargeData = 0xFF,
  svAllSmallData = 0xFF00,
  svAllMiniData = 0xFF0000,
  
  svAll1BitData = 0x010101,
  svAll4BitData = 0x020202,
  svAll8BitData = 0x040404,
  
  svAllAvailableData = 0xFFFFFF,
};

#define noErr		0
#define paramErr	(-50)
#define noMaskFoundErr	(-1000)

typedef ProcPtr IconActionProcPtr;
typedef ProcPtr IconGetterProcPtr;

typedef uint32 IconSelectorValue;
typedef int16 IconAlignmentType;
typedef int16 IconTransformType;

typedef struct CIcon { GUEST_STRUCT;
    GUEST< PixMap> iconPMap;
    GUEST< BitMap> iconMask;
    GUEST< BitMap> iconBMap;
    GUEST< Handle> iconData;
    GUEST< int16[1]> iconMaskData;
} *CIconPtr;
MAKE_HIDDEN(CIconPtr);
typedef HIDDEN_CIconPtr *CIconHandle;

/* icon utility function prototypes */

extern pascal trap OSErr C_PlotIconID (const Rect *rect, 
				       IconAlignmentType align, 
				       IconTransformType tranform, 
				       short res_id);

extern pascal trap OSErr C_PlotIconMethod (const Rect *rect,
					   IconAlignmentType align,
					   IconTransformType transform,
					   IconGetterProcPtr method,
					   void *data);
extern pascal trap void C_PlotCIcon (const Rect *rect, CIconHandle icon);
extern pascal trap void C_PlotIcon (const Rect *rect, Handle icon);
extern pascal trap OSErr C_PlotIconHandle (const Rect *rect,
					   IconAlignmentType align,
					   IconTransformType transform,
					   Handle icon);
extern pascal trap OSErr C_PlotCIconHandle (const Rect *rect,
					    IconAlignmentType align,
					    IconTransformType transform,
					    CIconHandle icon);
extern pascal trap OSErr C_PlotSICNHandle (const Rect *rect,
					   IconAlignmentType align,
					   IconTransformType transform,
					   Handle icon);

extern pascal trap Handle C_GetIcon (short icon_id);
extern pascal trap CIconHandle C_GetCIcon (short icon_id);

extern  pascal trap void C_DisposeCIcon (CIconHandle icon);

extern pascal trap OSErr C_GetIconSuite (Handle *suite, short res_id,
					 IconSelectorValue selector);
extern pascal trap OSErr C_NewIconSuite (Handle *suite);

extern pascal trap OSErr C_AddIconToSuite (Handle icon_data, Handle suite,
					   ResType type);
extern pascal trap OSErr C_GetIconFromSuite (Handle *icon_data,
					     Handle suite, ResType type);
extern pascal trap OSErr C_PlotIconSuite (const Rect *rect,
					  IconAlignmentType align,
					  IconTransformType transform,
					  Handle suite);
extern pascal trap OSErr C_ForEachIconDo (Handle suite,
					  IconSelectorValue selector,
					  IconActionProcPtr action, 
					  void *data);
extern pascal trap short C_GetSuiteLabel (Handle suite);
extern pascal trap OSErr C_SetSuiteLabel (Handle suite, short label);
extern pascal trap OSErr C_GetLabel (short label, RGBColor *label_color,
				     Str255 label_string);
extern pascal trap OSErr C_DisposeIconSuite (Handle suite,
					     Boolean dispose_data_p);

extern pascal trap OSErr C_IconSuiteToRgn (RgnHandle rgn, const Rect *rect,
					   IconAlignmentType align,
					   Handle suite);
extern pascal trap OSErr C_IconIDToRgn (RgnHandle rgn, const Rect *rect,
					IconAlignmentType align,
					short icon_id);
extern pascal trap OSErr C_IconMethodToRgn (RgnHandle rgn, const Rect *rect,
					    IconAlignmentType align,
					    IconGetterProcPtr method,
					    void *data);

extern pascal trap Boolean C_PtInIconSuite (Point test_pt,
					    const Rect *rect,
					    IconAlignmentType align,
					    Handle suite);
extern pascal trap Boolean C_PtInIconID (Point test_pt, const Rect *rect,
					 IconAlignmentType align,
					 short icon_id);
extern pascal trap Boolean C_PtInIconMethod (Point test_pt, const Rect *rect,
					     IconAlignmentType align,
					     IconGetterProcPtr method,
					     void *data);
extern pascal trap Boolean C_RectInIconSuite (const Rect *test_rect,
					      const Rect *rect,
					      IconAlignmentType align,
					      Handle suite);
extern pascal trap Boolean C_RectInIconID (const Rect *test_rect,
					   const Rect *rect,
					   IconAlignmentType align,
					   short icon_id);
extern pascal trap Boolean C_RectInIconMethod (const Rect *test_rect,
					       const Rect *rect,
					       IconAlignmentType align,
					       IconGetterProcPtr method,
					       void *data);
extern pascal trap OSErr C_MakeIconCache (Handle *cache,
					  IconGetterProcPtr make_icon,
					  void *data);
extern pascal trap OSErr C_LoadIconCache (const Rect *rect,
					  IconAlignmentType align,
					  IconTransformType transform,
					  Handle cache);
extern pascal trap OSErr C_GetIconCacheData (Handle cache, void **data);
extern pascal trap OSErr C_SetIconCacheData (Handle cache, void *data);
extern pascal trap OSErr C_GetIconCacheProc (Handle cache,
					     IconGetterProcPtr *proc);
extern pascal trap OSErr C_SetIconCacheProc (Handle cache,
					     IconGetterProcPtr proc);
}

#endif /* !_ICON_UTIL_H */
