
#if !defined(__WIND_H_)
#define _WIND_H_

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: wind.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "WindowMgr.h"
#include "MemoryMgr.h"
#include "ResourceMgr.h"
#include "OSUtil.h"

#include "rsys/mman.h"
#include "rsys/smash.h"
#include "rsys/pstuff.h"

extern LONGINT C_wdef0 (INTEGER, WindowPtr, INTEGER, LONGINT),
	       C_wdef16(INTEGER, WindowPtr, INTEGER, LONGINT);

extern WindowPeek ROMlib_firstvisible (WindowPtr w);

extern BOOLEAN ROMlib_window_zoomed(WindowPeek wp);
extern void wind_color_init (void);

typedef pascal void (*draghookp)(void);

#define var(w) (GetWVariant((w)))

extern BOOLEAN ROMlib_dirtyvariant;

typedef pascal LONGINT (*windprocp)(INTEGER var, WindowPtr wind, INTEGER mess,
				    LONGINT param);
typedef struct { windprocp p PACKED_P; } HIDDEN_windprocp;

extern void CALLDRAGHOOK (void);
extern void WINDCALLDESKHOOK (void);
extern LONGINT ROMlib_windcall (WindowPtr wind, INTEGER mess, LONGINT param);

#define WINDCALL(w, m, x)	ROMlib_windcall((w), (m), (x))

#define ZOOMBIT	8

#define USE_DESKCPAT_BIT 0x80
#define USE_DESKCPAT_VAR SPMisc2

extern const ColorSpec default_color_win_ctab[];
extern int ROMlib_emptyvis;

extern RGBColor *validate_colors_for_window (GrafPtr w);

/* the current window manager port being drawn to; since there are two
   windowmanager ports (bw and color), and since we couldn't hope to
   keep them in sync, we use one or the other.

   of course, our definition files use only the WMgrCPort */

#define wmgr_port ((GrafPtr) (WMgrCPort))

/* WindowPeek accessors;
   note these functions cast thier argument to type (WindowPeek),
   so a cast is not necessary on the calling side */
#define WINDOW_PORT(wp)			(&((WindowPeek) (wp))->port)
#define CWINDOW_PORT(wp)		(&((WindowPeek) (wp))->port)

/* big endian byte order */
#define WINDOW_KIND_X(wp)		(((WindowPeek) (wp))->windowKind)
#define WINDOW_VISIBLE_X(wp)		(((WindowPeek) (wp))->visible)
/* does this mean the window has focus? */
#define WINDOW_HILITED_X(wp)		(((WindowPeek) (wp))->hilited)
#define WINDOW_GO_AWAY_FLAG_X(wp)	(((WindowPeek) (wp))->goAwayFlag)
#define WINDOW_SPARE_FLAG_X(wp)		(((WindowPeek) (wp))->spareFlag)

#define WINDOW_STRUCT_REGION_X(wp)	(((WindowPeek) (wp))->strucRgn)
#define WINDOW_CONT_REGION_X(wp)	(((WindowPeek) (wp))->contRgn)
#define WINDOW_UPDATE_REGION_X(wp)	(((WindowPeek) (wp))->updateRgn)
#define WINDOW_DEF_PROC_X(wp)		(((WindowPeek) (wp))->windowDefProc)
#define WINDOW_DATA_X(wp)		(((WindowPeek) (wp))->dataHandle)
#define WINDOW_TITLE_X(wp)		(((WindowPeek) (wp))->titleHandle)
#define WINDOW_TITLE_WIDTH_X(wp)	(((WindowPeek) (wp))->titleWidth)
#define WINDOW_CONTROL_LIST_X(wp)	(((WindowPeek) (wp))->controlList)
#define WINDOW_NEXT_WINDOW_X(wp)	(((WindowPeek) (wp))->nextWindow)
#define WINDOW_PIC_X(wp)		(((WindowPeek) (wp))->windowPic)
#define WINDOW_REF_CON_X(wp)		(((WindowPeek) (wp))->refCon)

/* native byte order */
#define WINDOW_KIND(wp)			(Cx (WINDOW_KIND_X (wp)))
#define WINDOW_VISIBLE(wp)		(Cx (WINDOW_VISIBLE_X (wp)))
/* does this mean the window has focus? */
#define WINDOW_HILITED(wp)		(Cx (WINDOW_HILITED_X (wp)))
#define WINDOW_GO_AWAY_FLAG(wp)		(Cx (WINDOW_GO_AWAY_FLAG_X (wp)))
#define WINDOW_SPARE_FLAG(wp)		(Cx (WINDOW_SPARE_FLAG_X (wp)))

#define WINDOW_STRUCT_REGION(wp)	(MR (WINDOW_STRUCT_REGION_X (wp)))
#define WINDOW_CONT_REGION(wp)		(MR (WINDOW_CONT_REGION_X (wp)))
#define WINDOW_UPDATE_REGION(wp)	(MR (WINDOW_UPDATE_REGION_X (wp)))
#define WINDOW_DEF_PROC(wp)		(MR (WINDOW_DEF_PROC_X (wp)))
#define WINDOW_DATA(wp)			(MR (WINDOW_DATA_X (wp)))
#define WINDOW_TITLE(wp)		(MR (WINDOW_TITLE_X (wp)))
#define WINDOW_TITLE_WIDTH(wp)		(Cx (WINDOW_TITLE_WIDTH_X (wp)))
#define WINDOW_CONTROL_LIST(wp)		(MR (WINDOW_CONTROL_LIST_X (wp)))
#define WINDOW_NEXT_WINDOW(wp)		(MR (WINDOW_NEXT_WINDOW_X (wp)))
#define WINDOW_PIC(wp)			(MR (WINDOW_PIC_X (wp)))
#define WINDOW_REF_CON(wp)		(Cx (WINDOW_REF_CON_X (wp)))

#endif /* !_WIND_H_ */
