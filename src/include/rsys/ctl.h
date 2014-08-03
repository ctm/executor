#if !defined(_CTL_H_)
#define _CTL_H_

/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: ctl.h 85 2005-05-24 22:06:13Z ctm $
 */

#include "ControlMgr.h"
#include "MenuMgr.h"
#include "MemoryMgr.h"
#include "ResourceMgr.h"

#include "rsys/mman.h"
#include "rsys/cquick.h"
#include "rsys/pstuff.h"

namespace Executor {
extern const ColorSpec default_ctl_colors[];
extern CTabHandle default_ctl_ctab;
extern AuxCtlHandle default_aux_ctl;

/* accessor macros */
#define CTL_RECT(ctl)		(HxX (ctl, contrlRect))
#define CTL_TITLE(ctl)		(HxX (ctl, contrlTitle))

#define CTL_NEXT_CONTROL_X(ctl)	(HxX (ctl, nextControl))
#define CTL_OWNER_X(ctl)	(HxX (ctl, contrlOwner))
#define CTL_VIS_X(ctl)		(HxX (ctl, contrlVis))
#define CTL_HILITE_X(ctl)	(HxX (ctl, contrlHilite))
#define CTL_VALUE_X(ctl)	(HxX (ctl, contrlValue))
#define CTL_MIN_X(ctl)		(HxX (ctl, contrlMin))
#define CTL_MAX_X(ctl)		(HxX (ctl, contrlMax))

#define CTL_DEFPROC_X(ctl)	(HxX (ctl, contrlDefProc))
#define CTL_DATA_X(ctl)		(HxX (ctl, contrlData))
#define CTL_ACTION_X(ctl)	(HxX (ctl, contrlAction))
#define CTL_REF_CON_X(ctl)	(HxX (ctl, contrlRfCon))

#define CTL_NEXT_CONTROL(ctl)	(MR (CTL_NEXT_CONTROL_X (ctl)))
#define CTL_OWNER(ctl)		(MR (CTL_OWNER_X (ctl)))
#define CTL_VIS(ctl)		(CTL_VIS_X (ctl))
#define CTL_HILITE(ctl)		((uint8) CTL_HILITE_X (ctl))
#define CTL_VALUE(ctl)		(CW (CTL_VALUE_X (ctl)))
#define CTL_MIN(ctl)		(CW (CTL_MIN_X (ctl)))
#define CTL_MAX(ctl)		(CW (CTL_MAX_X (ctl)))
#define CTL_DEFPROC(ctl)	(MR (CTL_DEFPROC_X (ctl)))
#define CTL_DATA(ctl)		((RgnHandle) PPR (CTL_DATA_X (ctl)))
#define CTL_ACTION(ctl)		(MR (CTL_ACTION_X (ctl)))
#define CTL_ACTION_AS_LONG(ctl)	(CL ((LONGINT) CTL_ACTION_X (ctl)))
#define CTL_REF_CON(ctl)	(CL (CTL_REF_CON_X (ctl)))

extern CTabHandle default_ctl_ctab;
extern AuxCtlHandle *lookup_aux_ctl (ControlHandle ctl);

extern int32 C_cdef0    (int16 var, ControlHandle ctl, int16 mess,
			 int32 param);
extern int32 C_cdef16   (int16 var, ControlHandle ctl, int16 mess,
			 int32 param);
extern int32 C_cdef1008 (int16 var, ControlHandle ctl, int16 mess,
			 int32 param);

#define VAR(w)	(GetCVariant((w)))

extern BOOLEAN ROMlib_dirtyvariant;

extern void sb_ctl_init (void);

typedef pascal LONGINT
    (*ctlfuncp)(INTEGER var, ControlHandle ctl, INTEGER mess, LONGINT param);

extern LONGINT ROMlib_ctlcall (ControlHandle c, INTEGER i, LONGINT l);
#define CTLCALL(c, i, l)	ROMlib_ctlcall((c), (i), (l))

#define CTL_CALL_EXCURSION(control, body)			\
  THEPORT_SAVE_EXCURSION (CTL_OWNER (control), { body })

#define ENTIRECONTROL	0
#define MAXPARTCODE	253
#define INACTIVE	255
#define ALLINDICATORS	129

#define POPUP_MENU_X(popup)		(HxX (popup, menu))
#define POPUP_MENU_ID_X(popup)		(HxX (popup, menu_id))

#define POPUP_MENU_ID(popup)		(CW (POPUP_MENU_ID_X (popup)))
#define POPUP_MENU(popup)		(MR (POPUP_MENU_X (popup)))

#define POPUP_TITLE_WIDTH(popup)	(HxX (popup, title_width))
#define POPUP_FLAGS(popup)		(HxX (popup, flags))

struct PACKED popup_data
{
  PACKED_MEMBER(MenuHandle, menu);
  int16 menu_id;
  
  /* private fields */
  int16 title_width;
  int flags;
};

typedef struct popup_data popup_data_t;
typedef popup_data_t *popup_data_ptr;
MAKE_HIDDEN(popup_data_ptr);
typedef HIDDEN_popup_data_ptr *popup_data_handle;

typedef struct PACKED {
  Rect _tlimit;
  Rect _tslop;
  INTEGER _taxis;
} thumbstr;

typedef struct PACKED {
  Rect _crect;
  INTEGER _cvalue;
  INTEGER _cvisible;
  INTEGER _cmax;
  INTEGER _cmin;
  INTEGER _cprocid;
  LONGINT _crefcon;
  Byte _ctitle;
} contrlrestype;

struct PACKED lsastr
{
  Rect limitRect;
  Rect slopRect;
  INTEGER axis;
};

extern BOOLEAN ROMlib_cdef0_is_rectangular;

extern void image_arrow_up_active_init (void);
extern void image_arrow_up_inactive_init (void);
extern void image_arrow_down_active_init (void);
extern void image_arrow_down_inactive_init (void);
extern void image_arrow_left_active_init (void);
extern void image_arrow_left_inactive_init (void);
extern void image_arrow_right_active_init (void);
extern void image_arrow_right_inactive_init (void);
extern void image_thumb_horiz_init (void);
extern void image_thumb_vert_init (void);
extern void image_active_init (void);
extern void image_ractive_init (void);
extern void image_go_away_init (void);
extern void image_grow_init (void);
extern void image_zoom_init (void);
extern void image_apple_init (void);

extern void C_new_draw_scroll (INTEGER depth, INTEGER flags, GDHandle target,
			       LONGINT l);

extern void C_new_pos_ctl (INTEGER depth, INTEGER flags, GDHandle target,
			   LONGINT l);
}
#endif /* !_CTL_H_ */
