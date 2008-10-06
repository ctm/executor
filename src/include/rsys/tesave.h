#if !defined (__TESAVE__)
#define __TESAVE__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: tesave.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "rsys/pstuff.h"

typedef struct
{
#if 0
  /* TRUE if the save pattern is in _tpat, FALSE if it was placed in
     _tpixpat */
  INTEGER _tinpat;
  Pattern _tpat		PACKED;
  PixPatHandle _tpixpat	PACKED_P;
  Point _tploc		PACKED;
  Point _tpsize		PACKED;
  INTEGER _tpmode		PACKED;
#else
  PenState _tpstate		PACKED_P;
#endif
  
  GrafPtr _tport		PACKED_P;
  
  INTEGER _tpvis		PACKED;
  
  INTEGER _tfont		PACKED;
  INTEGER _tmode		PACKED;
  INTEGER _tsize		PACKED;
  Style _tstyle			PACKED;
  Byte filler			PACKED;
  RgnHandle _tsaveclip	PACKED_P;
  
  /* ### is tesave mac-visible state?  how is color and whatnot really
     stored? */
  int32 fg_color, bk_color;
  /* only filled if `CGrafPort_p ()' */
  RGBColor rgb_fg_color, rgb_bk_color;
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

typedef struct generic_elt
{
  int16 Height		PACKED;
  int16 Ascent		PACKED;
  int16 Font		PACKED;
  Style Face		PACKED;
  Byte filler		PACKED;
  int16 Size		PACKED;
  RGBColor Color	PACKED;
} generic_elt_t;

extern void generic_elt_copy (generic_elt_t *dst, generic_elt_t *src);
extern void generic_elt_calc_height_ascent (generic_elt_t *elt);

#define ST_ELT_TO_ATTR(st_elt) ((TextStyle *) (&(st_elt)->stFont))
#define SCRAP_ELT_TO_ATTR(scrap_elt) ((TextStyle *) (&(scrap_elt)->scrpFont))

#define ST_ELT_TO_GENERIC_ELT(st_elt) ((generic_elt_t *) (&(st_elt)->stHeight))
#define SCRAP_ELT_TO_GENERIC_ELT(scrap_elt)	\
  ((generic_elt_t *) (&(scrap_elt)->scrpHeight))

#if !defined (__STDC__)
extern INTEGER	ROMlib_StyleTextWidth();
extern INTEGER	ROMlib_word();
extern INTEGER	ROMlib_caltext();
extern void	ROMlib_togglelite();
extern void	ROMlib_tesave();
extern void	ROMlib_terestore();
extern void	ROMlib_tedoitall();
extern INTEGER	ROMlib_stylecompare();
extern void	ROMlib_teinsertstyleinfo();
extern void	ROMlib_teremovestyleinfo();
extern INTEGER	C_ROMlib_dotext();
extern void	ROMlib_teautoloop();
#else /* __STDC__ */
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
#endif /* __STDC__ */

extern int16 ROMlib_call_TEDoText (TEPtr tp, int16 first, int16 last,
				   int16 what);

typedef struct {	/* from MPW: ToolEqu.a */
    ProcPtr EOLHook	PACKED_P;
    ProcPtr DRAWHook	PACKED_P;
    ProcPtr WIDTHHook	PACKED_P;
    ProcPtr HITTESTHook	PACKED_P;
    LONGINT flags	PACKED;
} tehidden;

typedef tehidden *tehiddenp;
typedef struct { tehiddenp p PACKED_P; } HIDDEN_tehiddenp;
typedef HIDDEN_tehiddenp *tehiddenh;
typedef struct { tehiddenh p PACKED_P; } HIDDEN_tehiddenh;

#define TEAUTOVIEWBIT	1	/* found by dumping the handle
				   before and after calling TEAutoView */

#define TEHIDDENH(teh)	(STARH((HIDDEN_tehiddenh *)&(MR((*teh).p))->recalBack))
#define TEHIDDENHX(teh)	((*(HIDDEN_tehiddenh *)&(MR((*teh).p))->recalBack).p)

extern void ROMlib_recompute_caret (TEHandle te);

#endif /* __TESAVE__ */
