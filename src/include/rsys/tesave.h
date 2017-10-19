#if !defined (__TESAVE__)
#define __TESAVE__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: tesave.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "rsys/pstuff.h"

namespace Executor {

typedef struct PACKED
{
#if 0
  /* TRUE if the save pattern is in _tpat, FALSE if it was placed in
     _tpixpat */
  INTEGER _tinpat;
  Pattern _tpat;
  PACKED_MEMBER(PixPatHandle, _tpixpat);
  Point _tploc;
  Point _tpsize;
  INTEGER _tpmode;
#else
  GUEST<PenState> _tpstate;
#endif
  
  GUEST<GrafPtr> _tport;
  
  GUEST<INTEGER> _tpvis;
  
  GUEST<INTEGER> _tfont;
  GUEST<INTEGER> _tmode;
  GUEST<INTEGER> _tsize;
  GUEST<Style> _tstyle;
  GUEST<Byte> filler;
  GUEST<RgnHandle> _tsaveclip;
  
  /* ### is tesave mac-visible state?  how is color and whatnot really
     stored? */
  GUEST<int32> fg_color, bk_color;
  /* only filled if `CGrafPort_p ()' */
  GUEST<RGBColor> rgb_fg_color, rgb_bk_color;
} tesave;

#define TESAVE(x)	tesave _txx; ROMlib_tesave(&_txx, x);
#define TERESTORE()	ROMlib_terestore(&_txx);

#define GENERIC_ELT_FACE(generic_elt) ((generic_elt)->Face)
#define GENERIC_ELT_COLOR(generic_elt) ((generic_elt)->Color)

#define GENERIC_ELT_HEIGHT_X(generic_elt) ((generic_elt)->Height)
#define GENERIC_ELT_ASCENT_X(generic_elt) ((generic_elt)->Ascent)
#define GENERIC_ELT_FONT_X(generic_elt) ((generic_elt)->Font)
#define GENERIC_ELT_SIZE_X(generic_elt) ((generic_elt)->Size)

#define GENERIC_ELT_HEIGHT(generic_elt) (CW (GENERIC_ELT_HEIGHT_X (generic_elt)))
#define GENERIC_ELT_ASCENT(generic_elt) (CW (GENERIC_ELT_ASCENT_X (generic_elt)))
#define GENERIC_ELT_FONT(generic_elt) (CW (GENERIC_ELT_FONT_X (generic_elt)))
#define GENERIC_ELT_SIZE(generic_elt) (CW (GENERIC_ELT_SIZE_X (generic_elt)))

typedef struct generic_elt { GUEST_STRUCT;
    GUEST< int16> Height;
    GUEST< int16> Ascent;
    GUEST< int16> Font;
    GUEST< Style> Face;
    GUEST< Byte> filler;
    GUEST< int16> Size;
    GUEST< RGBColor> Color;
} generic_elt_t;

extern void generic_elt_copy (generic_elt_t *dst, generic_elt_t *src);
extern void generic_elt_calc_height_ascent (generic_elt_t *elt);

#define ST_ELT_TO_ATTR(st_elt) ((TextStyle *) (&(st_elt)->stFont))
#define SCRAP_ELT_TO_ATTR(scrap_elt) ((TextStyle *) (&(scrap_elt)->scrpFont))

#define ST_ELT_TO_GENERIC_ELT(st_elt) ((generic_elt_t *) (&(st_elt)->stHeight))
#define SCRAP_ELT_TO_GENERIC_ELT(scrap_elt)	\
  ((generic_elt_t *) (&(scrap_elt)->scrpHeight))

extern INTEGER	ROMlib_StyleTextWidth( TEPtr tep, INTEGER start,
							       INTEGER count );
extern INTEGER	ROMlib_word( char *p );

extern void ROMlib_caltext (TEHandle te,
			    int16 sel, int16 n_added,
			    int16 *first_changed_out, int16 *last_changed_out);

extern void	te_char_to_point (const TEPtr tep, int16 sel, Point *p);
extern void	ROMlib_togglelite( TEHandle teh );
extern void	ROMlib_tesave( tesave *t, TEHandle teh );
extern void	ROMlib_terestore( tesave *t );

extern void ROMlib_tedoitall (TEHandle teh, Ptr ptr,
			      int16 len, boolean_t insert, StScrpHandle styleh);

extern void	ROMlib_teinsertstyleinfo( TEHandle teh, INTEGER start,
					     INTEGER len, StScrpHandle styleh);
extern void	ROMlib_teremovestyleinfo( TEStyleHandle sth, INTEGER start,
								 INTEGER stop);
extern INTEGER	C_ROMlib_dotext( TEPtr tep, INTEGER first, INTEGER last,
								INTEGER what );
extern int16 te_char_to_run_index (TEStyleHandle te_style, int16 sel);
extern int16 te_char_to_lineno (TEPtr te, int16 sel);

extern void	ROMlib_teautoloop( TEHandle teh );

extern int16 ROMlib_call_TEDoText (TEPtr tp, int16 first, int16 last,
				   int16 what);

struct tehidden { GUEST_STRUCT;
    GUEST< ProcPtr> EOLHook;
    GUEST< ProcPtr> DRAWHook;
    GUEST< ProcPtr> WIDTHHook;
    GUEST< ProcPtr> HITTESTHook;
    GUEST< LONGINT> flags;
};

typedef tehidden *tehiddenp;
MAKE_HIDDEN(tehiddenp);
typedef HIDDEN_tehiddenp *tehiddenh;
MAKE_HIDDEN(tehiddenh);

#define TEAUTOVIEWBIT	1	/* found by dumping the handle
				   before and after calling TEAutoView */

#define TEHIDDENH(teh)	(STARH((HIDDEN_tehiddenh *)&(MR(*teh))->recalBack))
#define TEHIDDENHX(teh)	((*(HIDDEN_tehiddenh *)&(MR(*teh))->recalBack))

extern void ROMlib_recompute_caret (TEHandle te);
}
#endif /* __TESAVE__ */
