/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#warning the icon suite representation is our own brew -- tests should
#warning be written and we should do what the Mac does

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_icon[] =
	"$Id: icon.c 88 2005-05-25 03:59:37Z ctm $";
#endif

#include "rsys/common.h"

#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "Iconutil.h"

#include "rsys/cquick.h"
#include "rsys/resource.h"
#include "rsys/mman.h"
#include "rsys/icon.h"

using namespace Executor;
using namespace ByteSwap;

#define ICON_RETURN_ERROR(error)					      \
  ({									      \
    OSErr _error_ = (error);						      \
									      \
    if (_error_ != noErr)						      \
      warning_unexpected ("error `%s', `%d'", # error, _error_);	      \
    return _error_;							      \
  })

#define _GetIconSuite(icon_suite, res_id, selector)			      \
  ({									      \
    Handle _icon_suite_;						      \
    OSErr _err_;							      \
									      \
    _err_ = GetIconSuite (&_icon_suite_, (res_id), (selector));		      \
									      \
    *(icon_suite) = MR (_icon_suite_);					      \
									      \
    _err_;								      \
  })

P4 (PUBLIC pascal trap, OSErr, PlotIconID,
    const Rect *, rect, IconAlignmentType, align,
    IconTransformType, transform, short, res_id)
{
  Handle icon_suite;
  OSErr err;
  
  err = _GetIconSuite (&icon_suite, res_id, svAllAvailableData);
  if (err != noErr)
    ICON_RETURN_ERROR (err);
  
  err = PlotIconSuite (rect, align, transform, icon_suite);
  if (err != noErr)
    ICON_RETURN_ERROR (err);
  
  DisposeIconSuite (icon_suite, FALSE);
  
  ICON_RETURN_ERROR (noErr);
}

P5 (PUBLIC pascal trap, OSErr, PlotIconMethod,
    const Rect *, rect, IconAlignmentType, align,
    IconTransformType, transform, IconGetterProcPtr, method,
    void *, data)
{
  warning_unimplemented (NULL_STRING);
  ICON_RETURN_ERROR (paramErr);
}

P2 (PUBLIC pascal trap, void, PlotIcon,
    const Rect *, rect, Handle, icon)
{
  if (icon == NULL)
    return;
  
  if (!(*icon).p)
    LoadResource (icon);
  
  LOCK_HANDLE_EXCURSION_1
    (icon,
     {
       BitMap bm;
       
       bm.baseAddr = (Ptr) icon->p;
       bm.rowBytes = CWC(4);
       bm.bounds.left = bm.bounds.top = 0;
       if (GetHandleSize (icon) == 2 * 16)
	 {
	   bm.rowBytes = CWC(2);
	   bm.bounds.bottom = CWC(16);
	 }
       else
	 {
	   bm.rowBytes = CWC(4);
	   bm.bounds.bottom = CWC(32);
	 }
       bm.bounds.right = bm.bounds.bottom;
       CopyBits(&bm, PORT_BITS_FOR_COPY (thePort), &bm.bounds, rect,
		srcCopy, NULL);
     });
}

P4 (PUBLIC pascal trap, OSErr, PlotIconHandle,
    const Rect *, rect, IconAlignmentType, align,
    IconTransformType, transform, Handle, icon)
{
  /* #### change plotting routines to respect alignment and transform */
  if (align != atNone)
    warning_unimplemented ("unhandled icon alignment `%d'", align);
  if (transform != ttNone)
    warning_unimplemented ("unhandled icon transform `%d'", transform);
  
  PlotIcon (rect, icon);
  
  ICON_RETURN_ERROR (noErr);
}

P2 (PUBLIC pascal trap, void, PlotCIcon,
    const Rect *, rect, CIconHandle, icon)
{
  /* when plotting, `ignore' the current fg/bk colors */
  GrafPtr current_port;
  RGBColor bk_rgb, fg_rgb;
  int32 bk_color, fg_color;
  int cgrafport_p;
  
  if (! icon)
    return;
  
  current_port = thePort;
  
  cgrafport_p = CGrafPort_p (current_port);
  if (cgrafport_p)
    {
      fg_rgb = CPORT_RGB_FG_COLOR (current_port);
      bk_rgb = CPORT_RGB_BK_COLOR (current_port);
    }
  fg_color = PORT_FG_COLOR_X (current_port);
  bk_color = PORT_BK_COLOR_X (current_port);
  
  RGBForeColor (&ROMlib_black_rgb_color);
  RGBBackColor (&ROMlib_white_rgb_color);
  
  LOCK_HANDLE_EXCURSION_1
    (icon,
     {
       PixMapHandle gd_pixmap;
       
       BitMap *mask_bm;
       BitMap *icon_bm;
       
       icon_bm = &CICON_BMAP (icon);
       
       mask_bm = &CICON_MASK (icon);
       BITMAP_BASEADDR_X (mask_bm) = RM ((Ptr) CICON_MASK_DATA (icon));
       
       gd_pixmap = GD_PMAP (MR (MainDevice));
       
       if (   (PORT_BASEADDR_X (current_port) == PIXMAP_BASEADDR_X (gd_pixmap)
	       && PIXMAP_PIXEL_SIZE (gd_pixmap) > 2)
	   || (CGrafPort_p (current_port)
	       && PIXMAP_PIXEL_SIZE (CPORT_PIXMAP (current_port)) > 2)
	   || ! BITMAP_ROWBYTES_X (icon_bm))
	 {
	   Handle icon_data;
	   
	   icon_data = CICON_DATA (icon);
	   LOCK_HANDLE_EXCURSION_1
	     (icon_data,
	      {
		PixMap *icon_pm;
		
		icon_pm = &CICON_PMAP (icon);
		BITMAP_BASEADDR_X (icon_pm) = (Ptr) icon_data->p;
		
		CopyMask ((BitMap *) icon_pm,
			  mask_bm,
			  PORT_BITS_FOR_COPY (current_port),
			  &BITMAP_BOUNDS (icon_pm),
			  &BITMAP_BOUNDS (mask_bm),
			  /* #### fix up the need for this cast */
			  (Rect *) rect);
	      });
	 }
       else
	 {
	   Rect *icon_bm_bounds;
	   Ptr bm_baseaddr;
	   int height;
	   int mask_data_size;

	   icon_bm_bounds = &BITMAP_BOUNDS (icon_bm);
	   
	   height = RECT_HEIGHT (icon_bm_bounds);
	   mask_data_size = BITMAP_ROWBYTES (icon_bm) * height;
	   bm_baseaddr = (Ptr) ((char *) CICON_MASK_DATA (icon)
			  + mask_data_size);

	   BITMAP_BASEADDR_X (icon_bm) = RM (bm_baseaddr);
	   CopyMask (icon_bm,
		     mask_bm,
		     PORT_BITS_FOR_COPY (current_port),
		     icon_bm_bounds,
		     &BITMAP_BOUNDS (mask_bm),
		     /* #### fix up the need for this cast */
		     (Rect *) rect);
	 }
     });
  
  if (cgrafport_p)
    {
      CPORT_RGB_FG_COLOR (current_port) = fg_rgb;
      CPORT_RGB_BK_COLOR (current_port) = bk_rgb;
    }
  PORT_FG_COLOR_X (current_port) = fg_color;
  PORT_BK_COLOR_X (current_port) = bk_color;
}

P4 (PUBLIC pascal trap, OSErr, PlotCIconHandle, 
    const Rect *, rect, IconAlignmentType, align,
    IconTransformType, transform, CIconHandle, icon)
{
  /* #### change plotting routines to respect alignment and transform */
  if (align != atNone)
    warning_unimplemented ("unhandled icon alignment `%d'", align);
  if (transform != ttNone)
    warning_unimplemented ("unhandled icon transform `%d'", transform);
  
  PlotCIcon (rect, icon);
  
  ICON_RETURN_ERROR (noErr);
}

P4 (PUBLIC pascal trap, OSErr, PlotSICNHandle, 
    const Rect *, rect, IconAlignmentType, align,
    IconTransformType, transform, Handle, icon)
{
  /* #### change plotting routines to respect alignment and transform */
  if (align != atNone)
    warning_unimplemented ("unhandled icon alignment `%d'", align);
  if (transform != ttNone)
    warning_unimplemented ("unhandled icon transform `%d'", transform);
  
  PlotIcon (rect, icon);
  
  ICON_RETURN_ERROR (noErr);
}

P1 (PUBLIC pascal trap, Handle, GetIcon,
    short, icon_id)
{
  return ROMlib_getrestid (TICK ("ICON"), icon_id);
}

P1 (PUBLIC pascal trap, CIconHandle, GetCIcon,
    short, icon_id)
{
  CIconHandle cicon_handle;
  CIconHandle cicon_res_handle;
  CIconPtr cicon_res;
  int height;
  int mask_data_size;
  int bmap_data_size;
  int new_size;

  cicon_res_handle = (CIconHandle) ROMlib_getrestid (TICK ("cicn"), icon_id);
  if (cicon_res_handle == NULL)
    return NULL;
  
  cicon_res = STARH (cicon_res_handle);
  height = RECT_HEIGHT (&cicon_res->iconPMap.bounds);
  mask_data_size  = BigEndianValue (cicon_res->iconMask.rowBytes) * height;
  bmap_data_size  = BigEndianValue (cicon_res->iconBMap.rowBytes) * height;
  new_size = sizeof(CIcon) - sizeof(INTEGER) + mask_data_size + bmap_data_size;

  cicon_handle = (CIconHandle) NewHandle (new_size);
  LOCK_HANDLE_EXCURSION_2
    (cicon_handle, cicon_res_handle,
     {
       CTabPtr tmp_ctab;
       int mask_data_offset;
       int bmap_data_offset;
       int pmap_ctab_offset;
       int pmap_ctab_size;
       int pmap_data_offset;
       int pmap_data_size;
       CIconPtr cicon;
       
       cicon     = STARH (cicon_handle);
       cicon_res = STARH (cicon_res_handle);
       
       BlockMove ((Ptr) cicon_res, (Ptr) cicon, new_size);
       
       mask_data_offset = 0;
       
       bmap_data_offset = mask_data_size;
       
       pmap_ctab_offset = bmap_data_offset + bmap_data_size;
       tmp_ctab = (CTabPtr) ((char *) &cicon_res->iconMaskData +
							     pmap_ctab_offset);
       pmap_ctab_size   = sizeof (ColorTable) + (BigEndianValue (tmp_ctab->ctSize)
						 * sizeof (ColorSpec));
       
       pmap_data_offset = pmap_ctab_offset + pmap_ctab_size;
       pmap_data_size   = (BigEndianValue (cicon->iconPMap.rowBytes)
			   & ROWBYTES_VALUE_BITS) * height;
       
       cicon->iconMask.baseAddr = CLC_NULL;
       
       cicon->iconBMap.baseAddr = CLC_NULL;
       
       {
	 CTabHandle color_table;
	 
	 color_table
	   = (CTabHandle) NewHandle (pmap_ctab_size);
	 BlockMove ((Ptr) &cicon_res->iconMaskData + pmap_ctab_offset,
		    (Ptr) STARH (color_table),
		    pmap_ctab_size);
	 CTAB_SEED_X (color_table) = BigEndianValue (GetCTSeed ());
	 cicon->iconPMap.pmTable = RM (color_table);
	 
	 cicon->iconPMap.baseAddr = CLC_NULL;
	 cicon->iconData = RM (NewHandle (pmap_data_size));
	 BlockMove ((Ptr) &cicon_res->iconMaskData + pmap_data_offset,
		    (Ptr) STARH(MR(cicon->iconData)),
		    pmap_data_size);
       }
     });

  return cicon_handle;
}

P1 (PUBLIC pascal trap, void, DisposeCIcon,
    CIconHandle, icon)
{
  DisposHandle (CICON_DATA (icon));
  DisposHandle ((Handle) MR (CICON_PMAP (icon).pmTable));
  DisposHandle ((Handle) icon);
}

#define large_bw_icon 0
#define small_bw_icon 3

static int icon_for_log2_bpp[] =
{
  0, 0, 1, 2, 2, 2,
};

static int bpp_for_icon[] =
{
  1, 4, 8,
};

static int restype_for_icon[] =
{
  large1BitMask,
  large4BitData,
  large8BitData,
  small1BitMask,
  small4BitData,
  small8BitData,
};

static int mask_for_icon[] =
{
  svLarge1Bit,
  svLarge4Bit,
  svLarge8Bit,
  svSmall1Bit,
  svSmall4Bit,
  svSmall8Bit,
};

static int
restype_to_index (ResType type)
{
  int i;
  
  for (i = 0; i < N_SUITE_ICONS; i ++)
    {
      if (type == restype_for_icon[i])
	return i;
    }
  
  gui_fatal ("unknown icon restype `%d'");
}

P3 (PUBLIC pascal trap, OSErr, GetIconSuite, 
    Handle *, icon_suite_return, short, res_id, IconSelectorValue, selector)
{
  Handle icon_suite, *icons;
  int i;
  
  icon_suite = NewHandleClear (sizeof (cotton_suite_layout_t));
  if (MemErr != CWC (noErr))
    ICON_RETURN_ERROR (memFullErr);

  LOCK_HANDLE_EXCURSION_1
    (icon_suite,
     {
       icons = (Handle *) STARH (icon_suite);
  
       for (i = 0; i < N_SUITE_ICONS; i ++)
	 {
	   if (selector & mask_for_icon [i])
	     {
	       Handle icon;
	       
	       icon = GetResource (restype_for_icon[i], res_id);
	       if (icon != NULL)
		 icons[i] = icon;
	     }
	 }
     });
  
  *icon_suite_return = RM (icon_suite);
  
  ICON_RETURN_ERROR (noErr);
}

P1 (PUBLIC pascal trap, OSErr, NewIconSuite,
    Handle *, icon_suite_return)
{
  Handle icon_suite;
  
  icon_suite = NewHandleClear (sizeof (cotton_suite_layout_t));
  if (MemErr != CWC (noErr))
    ICON_RETURN_ERROR (memFullErr);
  
  *icon_suite_return = RM (icon_suite);
  
  ICON_RETURN_ERROR (noErr);
}

P3 (PUBLIC pascal trap, OSErr, AddIconToSuite,
    Handle, icon_data, Handle, icon_suite,
    ResType, type)
{
  Handle *icons;
  
  icons = (Handle *) STARH (icon_suite);
  icons[restype_to_index (type)] = icon_data;
  
  ICON_RETURN_ERROR (noErr);
}

P3 (PUBLIC pascal trap, OSErr, GetIconFromSuite,
    Handle *, icon_data_return, Handle, icon_suite, ResType, type)
{
  Handle *icons, icon_data;
  
  icons = (Handle *) STARH (icon_suite);
  icon_data = icons[restype_to_index (type)];
  
  if (icon_data == NULL)
    ICON_RETURN_ERROR (paramErr);
  
  *icon_data_return = RM (icon_data);
  ICON_RETURN_ERROR (noErr);
}

static OSErr
find_best_icon (boolean_t small_p, int bpp,
		Handle icon_suite_h,
		Handle *icon_data_return, Handle *icon_mask_return,
		boolean_t *small_return_p, int *icon_bpp_return)
{
  Handle *icons, *sized_icons;
  Handle icon_data, icon_mask;
  int best_icon;
  
  icons = (Handle *) STARH (icon_suite_h);
  
  sized_icons = (small_p
		 ? &icons[small_bw_icon]
		 : &icons[large_bw_icon]);
  icon_mask = *sized_icons;
  if (icon_mask == NULL)
    {
      small_p = ! small_p;
      
      sized_icons = (small_p
		     ? &icons[small_bw_icon]
		     : &icons[large_bw_icon]);
      icon_mask = *sized_icons;
      if (icon_mask == NULL)
	ICON_RETURN_ERROR (noMaskFoundErr);
    }
  
  best_icon = icon_for_log2_bpp[ROMlib_log2[bpp]];
  
#if !defined (LETGCCWAIL)
  icon_data = NULL;
#endif

  for (; best_icon > -1; best_icon --)
    {
      icon_data = sized_icons[best_icon];
      if (icon_data != NULL)
	break;
    }
  
  gui_assert (best_icon > -1);
  
  *small_return_p = small_p;
  *icon_bpp_return = bpp_for_icon[best_icon];
  
  *icon_mask_return = icon_mask;
  *icon_data_return = icon_data;
  
  ICON_RETURN_ERROR (noErr);
}

P4 (PUBLIC pascal trap, OSErr, PlotIconSuite, 
    const Rect *, rect, IconAlignmentType, align,
    IconTransformType, transform, Handle, icon_suite)
{
  GrafPtr current_port;
  int port_bpp, icon_bpp;
  boolean_t little_rect_p, little_icon_p;
  Handle icon_data, icon_mask;
  OSErr err;

  /* #### change plotting routines to respect alignment and transform */
  if (align != atNone)
    warning_unimplemented ("unhandled icon alignment `%d'", align);
  if (transform != ttNone)
    warning_unimplemented ("unhandled icon transform `%d'", transform);
  
  current_port = thePort;
  little_rect_p = (RECT_WIDTH (rect) < 32
		   && RECT_HEIGHT (rect) < 32);
  port_bpp = (CGrafPort_p (current_port)
	      ? PIXMAP_PIXEL_SIZE (CPORT_PIXMAP (current_port))
	      : 1);
  
  err = find_best_icon (little_rect_p, port_bpp, icon_suite,
			&icon_data, &icon_mask,
			&little_icon_p, &icon_bpp);
  if (err != noErr)
    ICON_RETURN_ERROR (err);
  
  /* plot our icon */
  
  LOCK_HANDLE_EXCURSION_2
    (icon_data, icon_mask,
     {
       PixMap icon_pm;
       BitMap mask_bm;
       CTabHandle color_table;
       Rect icon_rect;
       int icon_size;
       
       color_table = GetCTable (icon_bpp);
       
       memset (&icon_pm, '\000', sizeof icon_pm);
       memset (&icon_rect, '\000', sizeof icon_rect);
       
       icon_size = (little_icon_p ? 16 : 32);
       icon_rect.bottom = icon_rect.right = BigEndianValue (icon_size);
       
       icon_pm.baseAddr = icon_data->p;
       icon_pm.rowBytes = BigEndianValue (  (icon_size * icon_bpp / 8)
			      | PIXMAP_DEFAULT_ROW_BYTES);
       icon_pm.bounds = icon_rect;
       icon_pm.pixelSize = icon_pm.cmpSize = BigEndianValue (icon_bpp);
       icon_pm.cmpCount = CWC (1);
       icon_pm.pmTable = RM (color_table);
       
       mask_bm.baseAddr = (Ptr) RM ((char *) STARH (icon_mask)
			      + icon_size * icon_size / 8);
       mask_bm.rowBytes = BigEndianValue (icon_size / 8);
       mask_bm.bounds = icon_rect;
       
       CopyMask ((BitMap *) &icon_pm, &mask_bm,
		 PORT_BITS_FOR_COPY (current_port),
		 &icon_pm.bounds, &mask_bm.bounds,
		 /* #### fix up the need for this cast */
		 (Rect *) rect);
       
       DisposCTable (color_table);
     });
  
  ICON_RETURN_ERROR (noErr);
}

P4 (PUBLIC pascal trap, OSErr, ForEachIconDo,
    Handle, suite, IconSelectorValue, selector,
    IconActionProcPtr, action, void *, data)
{
  warning_unimplemented (NULL_STRING);
  ICON_RETURN_ERROR (paramErr);
}

P1 (PUBLIC pascal trap, short, GetSuiteLabel, 
    Handle, suite)
{
  short retval;
  cotton_suite_layout_t *suitep;

  suitep = (cotton_suite_layout_t *) STARH (suite);
  retval = BigEndianValue (suitep->label);
  return retval;
}

P2 (PUBLIC pascal trap, OSErr, SetSuiteLabel,
    Handle, suite, short, label)
{
  OSErr retval;
  cotton_suite_layout_t *suitep;

  suitep = (cotton_suite_layout_t *) STARH (suite);
  suitep->label = BigEndianValue (label);
  retval = noErr;

  return retval;
}

typedef struct
{
  RGBColor	rgb_color;
  Str255	string;
}
label_info_t;

PRIVATE label_info_t labels[7] =
{
  { { 0, 0, 0,}, "\pEssential", },
  { { 0, 0, 0,}, "\pHot", },
  { { 0, 0, 0,}, "\pIn Progress", },
  { { 0, 0, 0,}, "\pCool", },
  { { 0, 0, 0,}, "\pPersonal", },
  { { 0, 0, 0,}, "\pProject 1", },
  { { 0, 0, 0,}, "\pProject 2", },
};

P3 (PUBLIC pascal trap, OSErr, GetLabel,
    short, label, RGBColor *, label_color, Str255, label_string)
{
  unsigned int index;
  OSErr retval;
  static boolean_t been_here = FALSE;

  if (!been_here)
    {
      /* icky */
      labels[0].rgb_color = ROMlib_QDColors[1].rgb; /* orange->yellow */
      labels[1].rgb_color = ROMlib_QDColors[3].rgb; /* red */
      labels[2].rgb_color = ROMlib_QDColors[2].rgb; /* magenta */
      labels[3].rgb_color = ROMlib_QDColors[4].rgb; /* cyan */
      labels[4].rgb_color = ROMlib_QDColors[6].rgb; /* blue */
      labels[5].rgb_color = ROMlib_QDColors[5].rgb; /* green */
      labels[6].rgb_color = ROMlib_QDColors[0].rgb; /* brown->black */
      been_here = TRUE;
    }

  index = label - 1;
  if (index > 6)
    retval = paramErr;
  else
    {
      *label_color = labels[index].rgb_color;
      str255assign ((StringPtr) label_string,
		    (StringPtr) labels[index].string);
      retval = noErr;
    }

  ICON_RETURN_ERROR (retval);
}

P2 (PUBLIC pascal trap, OSErr, DisposeIconSuite,
    Handle, suite, Boolean, dispose_data_p)
{
  if (dispose_data_p)
    {
      LOCK_HANDLE_EXCURSION_1
	(suite,
	 {
	   Handle *icons;
	   int i;

	   icons = (Handle *) STARH (suite);
	   for (i = 0; i < N_SUITE_ICONS; i ++)
	     {
	       Handle icon;
	       SignedByte icon_state;

	       icon = icons[i];
	       if (icon)
		 {
		   icon_state = HGetState (icon);
		   if (icon_state & RSRCBIT)
		     ;
		   else
		     DisposHandle (icons[i]);
		 }
	     }
	 });
    }
  
  DisposHandle (suite);
  
  ICON_RETURN_ERROR (noErr);
}

P4 (PUBLIC pascal trap, OSErr, IconSuiteToRgn,
    RgnHandle, rgn, const Rect *, rect, IconAlignmentType, align,
    Handle, suite)
{
  warning_unimplemented (NULL_STRING);
  ICON_RETURN_ERROR (paramErr);
}

P4 (PUBLIC pascal trap, OSErr, IconIDToRgn,
    RgnHandle, rgn, const Rect *, rect,
    IconAlignmentType, align, short, icon_id)
{
  warning_unimplemented (NULL_STRING);
  ICON_RETURN_ERROR (paramErr);
}

P5 (PUBLIC pascal trap, OSErr, IconMethodToRgn, 
    RgnHandle, rgn, const Rect *, rect,
    IconAlignmentType, align, IconGetterProcPtr, method,
    void *, data)
{
  warning_unimplemented (NULL_STRING);
  ICON_RETURN_ERROR (paramErr);
}

P4 (PUBLIC pascal trap, Boolean, PtInIconSuite,
    Point, test_pt, const Rect *, rect, IconAlignmentType, align,
    Handle, suite)
{
  warning_unimplemented (NULL_STRING);
  return FALSE;
}

P4 (PUBLIC pascal trap, Boolean, PtInIconID,
    Point, test_pt, const Rect *, rect, IconAlignmentType, align,
    short, icon_id)
{
  Boolean retval;

  warning_unimplemented ("poorly implemented");
  retval = PtInRect (test_pt, (Rect *) rect);
  return retval;
}

P5 (PUBLIC pascal trap, Boolean, PtInIconMethod,
    Point, test_pt, const Rect *, rect, IconAlignmentType, align,
    IconGetterProcPtr, method, void *, data)
{
  warning_unimplemented (NULL_STRING);
  return FALSE;
}

P4 (PUBLIC pascal trap, Boolean, RectInIconSuite,
    const Rect *, test_rect, const Rect *, rect, IconAlignmentType, align,
    Handle, suite)
{
  warning_unimplemented (NULL_STRING);
  return FALSE;
}

P4 (PUBLIC pascal trap, Boolean, RectInIconID,
    const Rect *, test_rect, const Rect *, rect, IconAlignmentType, align,
    short, icon_id)
{
  warning_unimplemented (NULL_STRING);
  return FALSE;
}

P5 (PUBLIC pascal trap, Boolean, RectInIconMethod,
    const Rect *, test_rect, const Rect *, rect, IconAlignmentType, align,
    IconGetterProcPtr, method, void *, data)
{
  warning_unimplemented (NULL_STRING);
  return FALSE;
}

P3 (PUBLIC pascal trap, OSErr, MakeIconCache,
    Handle *, cache, IconGetterProcPtr, make_icon,
    void *, data)
{
  warning_unimplemented (NULL_STRING);
  ICON_RETURN_ERROR (paramErr);
}

P4 (PUBLIC pascal trap, OSErr, LoadIconCache, 
    const Rect *, rect, IconAlignmentType, align,
    IconTransformType, transform, Handle, cache)
{
  warning_unimplemented (NULL_STRING);
  ICON_RETURN_ERROR (paramErr);
}

P2 (PUBLIC pascal trap, OSErr, GetIconCacheData,
    Handle, cache, void **, data)
{
  warning_unimplemented (NULL_STRING);
  ICON_RETURN_ERROR (paramErr);
}

P2 (PUBLIC pascal trap, OSErr, SetIconCacheData,
    Handle, cache, void *, data)
{
  warning_unimplemented (NULL_STRING);
  ICON_RETURN_ERROR (paramErr);
}

P2 (PUBLIC pascal trap, OSErr, GetIconCacheProc,
    Handle, cache, IconGetterProcPtr *, proc)
{
  warning_unimplemented (NULL_STRING);
  ICON_RETURN_ERROR (paramErr);
}

P2 (PUBLIC pascal trap, OSErr, SetIconCacheProc,
    Handle, cache, IconGetterProcPtr, proc)
{
  warning_unimplemented (NULL_STRING);
  ICON_RETURN_ERROR (paramErr);
}
