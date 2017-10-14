#if !defined (_TEXTEDIT_H_)
#define _TEXTEDIT_H_

/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: TextEdit.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "QuickDraw.h"

namespace Executor {
/* new justification defines, accepted by `TESetAlignment ()' and
   `TETextBox ()' */
#define teFlushDefault	0
#define teCenter	1
#define teFlushRight	(-1)
#define teFlushLeft	(-2)

/* older justification defines */
#define teJustLeft	0
#define teJustCenter	1
#define teJustRight	(-1)
#define teForceLeft	(-2)

#define doFont		1
#define doFace		2
#define doSize		4
#define doColor		8
#define doAll		15
#define addSize		16
#define doToggle	32

#define teFind		0
#define teHilite	1
#define teDraw		-1
#define teCaret		-2

#define caret_vis	(-1)
#define caret_invis	(255)
#define hilite_vis	(0)

#define teFAutoScroll		(0)
#define teFTextBuffering	(1)
#define teFOutlineHilite	(2)
#define teFInlineInput		(3)
#define teFUseTextServices	(4)

#define teBitClear		(0)
#define teBitSet		(1)
#define teBitTest		(-1)

struct TERec : GuestStruct {
    GUEST< Rect> destRect;
    GUEST< Rect> viewRect;
    GUEST< Rect> selRect;
    GUEST< INTEGER> lineHeight;
    GUEST< INTEGER> fontAscent;
    GUEST< Point> selPoint;
    GUEST< INTEGER> selStart;
    GUEST< INTEGER> selEnd;
    GUEST< INTEGER> active;
    GUEST< ProcPtr> wordBreak;
    GUEST< ProcPtr> clikLoop;
    GUEST< LONGINT> clickTime;
    GUEST< INTEGER> clickLoc;
    GUEST< LONGINT> caretTime;
    GUEST< INTEGER> caretState;
    GUEST< INTEGER> just;
    GUEST< INTEGER> teLength;
    GUEST< Handle> hText;
    GUEST< INTEGER> recalBack;
    GUEST< INTEGER> recalLines;
    GUEST< INTEGER> clikStuff;
    GUEST< INTEGER> crOnly;
    GUEST< INTEGER> txFont;
    GUEST< Style> txFace;
    GUEST< Byte> filler;
    GUEST< INTEGER> txMode;
    GUEST< INTEGER> txSize;
    GUEST< GrafPtr> inPort;
    GUEST< ProcPtr> highHook;
    GUEST< ProcPtr> caretHook;
    GUEST< INTEGER> nLines;
    GUEST< INTEGER[1]> lineStarts;
};

typedef TERec *TEPtr;
MAKE_HIDDEN(TEPtr);
typedef HIDDEN_TEPtr *TEHandle;

struct StyleRun : GuestStruct {
    GUEST< INTEGER> startChar;
    GUEST< INTEGER> styleIndex;
};

struct STElement : GuestStruct {
    GUEST< INTEGER> stCount;
    GUEST< INTEGER> stHeight;
    GUEST< INTEGER> stAscent;
    GUEST< INTEGER> stFont;
    GUEST< Style> stFace;
    GUEST< Byte> filler;
    GUEST< INTEGER> stSize;
    GUEST< RGBColor> stColor;
};

typedef STElement TEStyleTable[1];
typedef STElement *STPtr;
MAKE_HIDDEN(STPtr);
typedef HIDDEN_STPtr *STHandle;

struct LHElement : GuestStruct {
    GUEST< INTEGER> lhHeight;
    GUEST< INTEGER> lhAscent;
};

typedef LHElement LHTable[1];
typedef LHElement *LHPtr;
MAKE_HIDDEN(LHPtr);
typedef HIDDEN_LHPtr *LHHandle;

struct TextStyle : GuestStruct {
    GUEST< INTEGER> tsFont;
    GUEST< Style> tsFace;
    GUEST< Byte> filler;
    GUEST< INTEGER> tsSize;
    GUEST< RGBColor> tsColor;
};

struct ScrpSTElement : GuestStruct {
    GUEST< LONGINT> scrpStartChar;
    GUEST< INTEGER> scrpHeight;
    GUEST< INTEGER> scrpAscent;
    GUEST< INTEGER> scrpFont;
    GUEST< Style> scrpFace;
    GUEST< Byte> filler;
    GUEST< INTEGER> scrpSize;
    GUEST< RGBColor> scrpColor;
};

typedef ScrpSTElement ScrpSTTable[1];

struct StScrpRec : GuestStruct {
    GUEST< INTEGER> scrpNStyles;
    GUEST< ScrpSTTable> scrpStyleTab;
};

typedef StScrpRec *StScrpPtr;
MAKE_HIDDEN(StScrpPtr);
typedef HIDDEN_StScrpPtr *StScrpHandle;

struct NullSTRec : GuestStruct {
    GUEST< LONGINT> TEReserved;
    GUEST< StScrpHandle> nullScrap;
};

typedef NullSTRec *NullSTPtr;
MAKE_HIDDEN(NullSTPtr);
typedef HIDDEN_NullSTPtr *NullSTHandle;

struct TEStyleRec : GuestStruct {
    GUEST< INTEGER> nRuns;
    GUEST< INTEGER> nStyles;
    GUEST< STHandle> styleTab;
    GUEST< LHHandle> lhTab;
    GUEST< LONGINT> teRefCon;
    GUEST< NullSTHandle> nullStyle;
    GUEST< StyleRun[1]> runs;
};

typedef TEStyleRec *TEStylePtr;
MAKE_HIDDEN(TEStylePtr);
typedef HIDDEN_TEStylePtr *TEStyleHandle;

typedef Byte Chars[1], *CharsPtr, **CharsHandle;

/* accessors! */

#define TE_DO_TEXT(te, start, end, what)				\
  ({									\
    int16 retval;							\
    									\
    LOCK_HANDLE_EXCURSION_1						\
      (te,								\
       {								\
	 retval = ROMlib_call_TEDoText (STARH (te), start, end, what);	\
       });								\
    retval;								\
  })
#define TE_CHAR_TO_POINT(te, sel, pt)			\
  ({							\
    LOCK_HANDLE_EXCURSION_1				\
      (te,						\
       {						\
	 te_char_to_point (STARH (te), sel, pt);	\
       });						\
  })

/* no need to lock te, since `te_char_to_lineno' cannot move memory
   blocks */
#define TE_CHAR_TO_LINENO(te, sel)			\
  te_char_to_lineno (STARH (te), sel)

#define TE_DEST_RECT(te)		(HxX ((te), destRect))
#define TE_VIEW_RECT(te)		(HxX ((te), viewRect))
#define TE_SEL_RECT(te)			(HxX ((te), selRect))
#define TE_SEL_POINT(te)		(HxX ((te), selPoint))
#define TE_LINE_STARTS(te)		(HxX ((te), lineStarts))
#define TE_GET_STYLE(te)					\
  ({								\
    TEStyleHandle retval;					\
								\
    if (!TE_STYLIZED_P (te))					\
      retval = NULL;						\
    else							\
      retval = MR (*(TEStyleHandle *) &TE_TX_FONT_X (te));	\
    retval;							\
  })

extern void ROMlib_sledgehammer_te (TEHandle te);
#if ERROR_SUPPORTED_P (ERROR_TEXT_EDIT_SLAM)
#define TE_SLAM(te)					\
  ({							\
    if (ERROR_ENABLED_P (ERROR_TEXT_EDIT_SLAM))		\
      ROMlib_sledgehammer_te (te);			\
  })
#else  /* No ERROR_TEXT_EDIT_SLAM */
#define TE_SLAM(te)
#endif /* No ERROR_TEXT_EDIT_SLAM */

#define TE_TX_FACE(te)			(HxX ((te), txFace))

#define TE_STYLIZED_P(te)		(HxX ((te), txSize) == CWC (-1))
#define TE_LINE_HEIGHT_X(te)		(HxX ((te), lineHeight))
#define TE_FONT_ASCENT_X(te)		(HxX ((te), fontAscent))
#define TE_LENGTH_X(te)			(HxX ((te), teLength))
#define TE_ACTIVE_X(te)			(HxX ((te), active))
#define TE_CARET_STATE_X(te)		(HxX ((te), caretState))
#define TE_SEL_START_X(te)		(HxX ((te), selStart))
#define TE_SEL_END_X(te)		(HxX ((te), selEnd))
#define TE_N_LINES_X(te)		(HxX ((te), nLines))
#define TE_HTEXT_X(te)			(HxX ((te), hText))
#define TE_CLICK_STUFF_X(te)		(HxX ((te), clikStuff))
#define TE_CLICK_LOC_X(te)		(HxX ((te), clickLoc))
#define TE_CLICK_TIME_X(te)		(HxX ((te), clickTime))
#define TE_JUST_X(te)			(HxX ((te), just))
#define TE_TX_FONT_X(te)		(HxX ((te), txFont))
#define TE_TX_SIZE_X(te)		(HxX ((te), txSize))
#define TE_TX_MODE_X(te)		(HxX ((te), txMode))
#define TE_IN_PORT_X(te)		(HxX ((te), inPort))

#define TE_LINE_HEIGHT(te)		(CW (TE_LINE_HEIGHT_X (te)))
#define TE_FONT_ASCENT(te)		(CW (TE_FONT_ASCENT_X (te)))
#define TE_LENGTH(te)			(CW (TE_LENGTH_X (te)))
#define TE_ACTIVE(te)			(CW (TE_ACTIVE_X (te)))
#define TE_CARET_STATE(te)		(CW (TE_CARET_STATE_X (te)))
#define TE_SEL_START(te)		(CW (TE_SEL_START_X (te)))
#define TE_SEL_END(te)			(CW (TE_SEL_END_X (te)))
#define TE_N_LINES(te)			(CW (TE_N_LINES_X (te)))
#define TE_CLICK_STUFF(te)		(CW (TE_CLICK_STUFF_X (te)))
#define TE_CLICK_LOC(te)		(CW (TE_CLICK_LOC_X (te)))
#define TE_CLICK_TIME(te)		(CL (TE_CLICK_TIME_X (te)))
#define TE_HTEXT(te)			(MR (TE_HTEXT_X (te)))
#define TE_JUST(te)			(CW (TE_JUST_X (te)))
#define TE_TX_FONT(te)			(CW (TE_TX_FONT_X (te)))
#define TE_TX_SIZE(te)			(CW (TE_TX_SIZE_X (te)))
#define TE_TX_MODE(te)			(CW (TE_TX_MODE_X (te)))
#define TE_IN_PORT(te)			(MR (TE_IN_PORT_X (te)))

#define TE_FLAGS_X(te)			(HxX (TEHIDDENH (te), flags))
#define TE_FLAGS(te)			(CL (TE_FLAGS_X (te)))

#if !defined (NDEBUG)
#define TEP_DO_TEXT(tep, start, end, what)		\
  ({							\
    Handle te;						\
    							\
    te = RecoverHandle ((Ptr) tep);			\
    gui_assert (te && (HGetState (te) & LOCKBIT));	\
    ROMlib_call_TEDoText (tep, start, end, what);	\
  })
#else
#define TEP_DO_TEXT(tep, start, end, what)		\
  ({ ROMlib_call_TEDoText (tep, start, end, what); })
#endif

#if !defined (NDEBUG)
#define TEP_CHAR_TO_POINT(tep, sel, pt)			\
  ({							\
    Handle te;						\
    							\
    te = RecoverHandle ((Ptr) tep);			\
    gui_assert (te && (HGetState (te) & LOCKBIT));	\
    te_char_to_point (tep, sel, pt);			\
  })
#else
#define TEP_CHAR_TO_POINT(tep, sel, pt)			\
  ({ te_char_to_point (tep, sel, pt); })
#endif
#define TEP_CHAR_TO_LINENO(tep, sel)			\
  te_char_to_lineno (tep, sel)

#define TEP_SLAM(tep)				\
  ({						\
    Handle te;					\
						\
    te = RecoverHandle (tep)			\
    ROMlib_sledgehammer_te (te);		\
  })

#define TEP_SEL_RECT(te)		((tep)->selRect)
#define TEP_DEST_RECT(tep)		((tep)->destRect)
#define TEP_VIEW_RECT(tep)		((tep)->viewRect)
#define TEP_LINE_STARTS(tep)		((tep)->lineStarts)
#define TEP_SEL_POINT(tep)		((tep)->selPoint)
#define TEP_TX_FACE(tep)		((tep)->txFace)
#define TEP_GET_STYLE(tep)					\
  ({								\
    TEStyleHandle retval;					\
								\
    if (!TEP_STYLIZED_P (tep))					\
      retval = NULL;						\
    else							\
      retval = MR (*(TEStyleHandle *) &TEP_TX_FONT_X (tep));	\
    retval;							\
  })


#define TEP_STYLIZED_P(tep)		((tep)->txSize == CWC (-1))
#define TEP_LINE_HEIGHT_X(tep)		((tep)->lineHeight)
#define TEP_FONT_ASCENT_X(tep)		((tep)->fontAscent)
#define TEP_LENGTH_X(tep)		((tep)->teLength)
#define TEP_ACTIVE_X(tep)		((tep)->active)
#define TEP_CARET_STATE_X(tep)		((tep)->caretState)
#define TEP_SEL_START_X(tep)		((tep)->selStart)
#define TEP_SEL_END_X(tep)		((tep)->selEnd)
#define TEP_N_LINES_X(tep)		((tep)->nLines)
#define TEP_HTEXT_X(tep)		((tep)->hText)
#define TEP_CLICK_STUFF_X(tep)		((tep)->clikStuff)
#define TEP_CLICK_LOC_X(tep)		((tep)->clickLoc)
#define TEP_JUST_X(tep)			((tep)->just)
#define TEP_TX_FONT_X(tep)		((tep)->txFont)
#define TEP_TX_SIZE_X(tep)		((tep)->txSize)
#define TEP_IN_PORT_X(tep)		((tep)->inPort)

#define TEP_LINE_HEIGHT(tep)		(CW (TEP_LINE_HEIGHT_X (tep)))
#define TEP_FONT_ASCENT(tep)		(CW (TEP_FONT_ASCENT_X (tep)))
#define TEP_LENGTH(tep)			(CW (TEP_LENGTH_X (tep)))
#define TEP_ACTIVE(tep)			(CW (TEP_ACTIVE_X (tep)))
#define TEP_CARET_STATE(tep)		(CW (TEP_CARET_STATE_X (tep)))
#define TEP_SEL_START(tep)		(CW (TEP_SEL_START_X (tep)))
#define TEP_SEL_END(tep)		(CW (TEP_SEL_END_X (tep)))
#define TEP_N_LINES(tep)		(CW (TEP_N_LINES_X (tep)))
#define TEP_CLICK_STUFF(tep)		(CW (TEP_CLICK_STUFF_X (tep)))
#define TEP_CLICK_LOC(tep)		(CL (TEP_CLICK_LOC_X (tep)))
#define TEP_HTEXT(tep)			(MR (TEP_HTEXT_X (tep)))
#define TEP_JUST(tep)			(CW (TEP_JUST_X (tep)))
#define TEP_TX_FONT(tep)		(CW (TEP_TX_FONT_X (tep)))
#define TEP_TX_SIZE(tep)		(CW (TEP_TX_SIZE_X (tep)))
#define TEP_IN_PORT(tep)		(MR (TEP_IN_PORT_X (tep)))

#define TEP_HEIGHT_FOR_LINE(tep, lineno)			\
  (TEP_STYLIZED_P (tep) && TEP_LINE_HEIGHT_X (tep) == CWC (-1)	\
   ? ({								\
       TEStyleHandle te_style = TEP_GET_STYLE (tep);		\
       LHElement *lh = STARH (TE_STYLE_LH_TABLE (te_style));	\
       LH_HEIGHT (&lh[lineno]);					\
     })								\
   :  TEP_LINE_HEIGHT (tep))
#define TEP_ASCENT_FOR_LINE(tep, lineno)			\
  (TEP_STYLIZED_P (tep) && TEP_FONT_ASCENT_X (tep) == CWC (-1)	\
   ? ({								\
       TEStyleHandle te_style = TEP_GET_STYLE (tep);		\
       LHElement *lh = STARH (TE_STYLE_LH_TABLE (te_style));	\
       LH_ASCENT (&lh[lineno]);					\
     })								\
   :  TEP_FONT_ASCENT (tep))

#define TEP_TEXT_WIDTH(tep, text, start, len)	\
  (TEP_STYLIZED_P (tep)				\
   ? ROMlib_StyleTextWidth (tep, start, len)	\
   : TextWidth (text, start, len))

#define LINE_START_X(line_starts, index)	\
  ((line_starts)[index])
#define LINE_START(line_starts, index)		\
  (CW (LINE_START_X (line_starts, index)))

#define TE_STYLE_SIZE_FOR_N_RUNS(n_runs)			\
  (sizeof (TEStyleRec)						\
   - sizeof TE_STYLE_RUNS ((TEStyleHandle) NULL)		\
   + (n_runs + 1) * sizeof *TE_STYLE_RUNS ((TEStyleHandle) NULL))

#define TE_STYLE_N_RUNS_X(te_style)	(HxX ((te_style), nRuns))
#define TE_STYLE_N_STYLES_X(te_style)	(HxX ((te_style), nStyles))
#define TE_STYLE_RUNS(te_style)		(HxX ((te_style), runs))
#define TE_STYLE_RUN(te_style, run_i)		\
  (&TE_STYLE_RUNS (te_style)[run_i])
#define TE_STYLE_STYLE_TABLE_X(te_style)	\
  (HxX ((te_style), styleTab))
#define TE_STYLE_LH_TABLE_X(te_style)		\
  (HxX ((te_style), lhTab))
#define TE_STYLE_NULL_STYLE_X(te_style)		\
  (HxX ((te_style), nullStyle))

#define TE_STYLE_N_RUNS(te_style)	(CW (TE_STYLE_N_RUNS_X (te_style)))
#define TE_STYLE_N_STYLES(te_style)	(CW (TE_STYLE_N_STYLES_X (te_style)))
#define TE_STYLE_STYLE_TABLE(te_style)		\
 (MR (TE_STYLE_STYLE_TABLE_X (te_style)))
#define TE_STYLE_LH_TABLE(te_style)		\
 (MR (TE_STYLE_LH_TABLE_X (te_style)))
#define TE_STYLE_NULL_STYLE(te_style)		\
 (MR (TE_STYLE_NULL_STYLE_X (te_style)))

#define TE_STYLE_NULL_SCRAP(te_style)		\
  (NULL_STYLE_NULL_SCRAP (TE_STYLE_NULL_STYLE (te_style)))

#define NULL_STYLE_NULL_SCRAP_X(null_style)	\
  (HxX ((null_style), nullScrap))
#define NULL_STYLE_NULL_SCRAP(null_style)	\
  (MR (NULL_STYLE_NULL_SCRAP_X (null_style)))

#define SCRAP_N_STYLES_X(scrap)			\
  (HxX ((scrap), scrpNStyles))

#define SCRAP_ST_ELT(scrap, elt_i)		\
  (&(HxX ((scrap), scrpStyleTab))[elt_i])
#define SCRAP_N_STYLES(scrap)			\
  (CW (SCRAP_N_STYLES_X (scrap)))

#define SCRAP_SIZE_FOR_N_STYLES(n_styles)	\
  (sizeof (StScrpRec) + ((n_styles) - 1) * sizeof (ScrpSTElement))

#define RUN_START_CHAR_X(run)		(STYLE_RUN_START_CHAR_X (run))
#define RUN_STYLE_INDEX_X(run)		(STYLE_RUN_STYLE_INDEX_X (run))

#define RUN_START_CHAR(run)		(STYLE_RUN_START_CHAR (run))
#define RUN_STYLE_INDEX(run)		(STYLE_RUN_STYLE_INDEX (run))

#define STYLE_RUN_START_CHAR_X(run)	((run)->startChar)
#define STYLE_RUN_STYLE_INDEX_X(run)	((run)->styleIndex)

#define STYLE_RUN_START_CHAR(run)	(CW (STYLE_RUN_START_CHAR_X (run)))
#define STYLE_RUN_STYLE_INDEX(run)	(CW (STYLE_RUN_STYLE_INDEX_X (run)))

#define TS_FACE(ts)			((ts)->tsFace)

#define TS_FONT_X(ts)			((ts)->tsFont)
#define TS_SIZE_X(ts)			((ts)->tsSize)
#define TS_COLOR(ts)			((ts)->tsColor)

#define TS_FONT(ts)			(CW (TS_FONT_X (ts)))
#define TS_SIZE(ts)			(CW (TS_SIZE_X (ts)))

#define LH_HEIGHT_X(lh)			((lh)->lhHeight)
#define LH_HEIGHT(lh)			(CW (LH_HEIGHT_X (lh)))
#define LH_ASCENT_X(lh)			((lh)->lhAscent)
#define LH_ASCENT(lh)			(CW (LH_ASCENT_X (lh)))

#define STYLE_TABLE_SIZE_FOR_N_STYLES(n_styles)	\
  ((n_styles) * sizeof (STElement))

#define ST_ELT(st, st_elt_i)			\
  (&STARH (st)[st_elt_i])

#define ST_ELT_FACE(st_elt)		((st_elt)->stFace)

#define ST_ELT_COUNT_X(st_elt)		((st_elt)->stCount)
#define ST_ELT_HEIGHT_X(st_elt)		((st_elt)->stHeight)
#define ST_ELT_ASCENT_X(st_elt)		((st_elt)->stAscent)
#define ST_ELT_FONT_X(st_elt)		((st_elt)->stFont)
#define ST_ELT_SIZE_X(st_elt)		((st_elt)->stSize)
#define ST_ELT_COLOR(st_elt)		((st_elt)->stColor)

#define ST_ELT_COUNT(st_elt)		(CW (ST_ELT_COUNT_X (st_elt)))
#define ST_ELT_HEIGHT(st_elt)		(CW (ST_ELT_HEIGHT_X (st_elt)))
#define ST_ELT_ASCENT(st_elt)		(CW (ST_ELT_ASCENT_X (st_elt)))
#define ST_ELT_FONT(st_elt)		(CW (ST_ELT_FONT_X (st_elt)))
#define ST_ELT_SIZE(st_elt)		(CW (ST_ELT_SIZE_X (st_elt)))

#define SCRAP_ELT_START_CHAR_X(scrap_elt)	\
  ((scrap_elt)->scrpStartChar)
#define SCRAP_ELT_START_CHAR(scrap_elt)		\
  (CL (SCRAP_ELT_START_CHAR_X (scrap_elt)))

extern boolean_t adjust_attrs (TextStyle *orig_attrs, TextStyle *new_attrs,
			       TextStyle *dst_attrs, TextStyle *continuous_attrs,
			       int16 mode);
extern int16 make_style_run_at (TEStyleHandle te_style, int16 sel);
extern int16 get_style_index (TEStyleHandle te_style, TextStyle *attrs,
			      int incr_count_p);
extern void  release_style_index (TEStyleHandle te_style, int16 style_index);
extern void stabilize_style_info (TEStyleHandle te_style);
extern void te_style_combine_runs (TEStyleHandle te_style);

#if !defined (TEDoText_H)
extern HIDDEN_ProcPtr 	TEDoText_H;
extern HIDDEN_Handle 	TEScrpHandle_H;
extern INTEGER 	TEScrpLength;
#endif

#define TEDoText	(TEDoText_H.p)
#define TEScrpHandle	(TEScrpHandle_H.p)


extern pascal trap void C_TESetText( Ptr p, LONGINT ln, TEHandle teh ); extern pascal trap void P_TESetText( Ptr p, LONGINT ln, TEHandle teh);
extern pascal trap CharsHandle C_TEGetText( TEHandle teh ); extern pascal trap CharsHandle P_TEGetText( TEHandle teh); 
extern pascal trap void C_TESetJust( INTEGER j, TEHandle teh ); extern pascal trap void P_TESetJust( INTEGER j, TEHandle teh); 
extern pascal trap void C_TEUpdate( Rect *r, TEHandle teh ); extern pascal trap void P_TEUpdate( Rect *r, TEHandle teh); 
extern pascal trap void C_TextBox( Ptr p, LONGINT ln, Rect *r, 
 INTEGER j ); extern pascal trap void P_TextBox( Ptr p, LONGINT ln, Rect *r, 
 INTEGER j ); 
extern pascal trap void C_TEScroll( INTEGER dh, INTEGER dv, TEHandle teh ); extern pascal trap void P_TEScroll( INTEGER dh, INTEGER dv, TEHandle teh); 
extern pascal trap void C_TEKey( CHAR thec, TEHandle teh ); extern pascal trap void P_TEKey( CHAR thec, TEHandle teh); 
extern pascal trap void C_TECopy( TEHandle teh ); extern pascal trap void P_TECopy( TEHandle teh); 
extern pascal trap void C_TECut( TEHandle teh ); extern pascal trap void P_TECut( TEHandle teh); 
extern pascal trap void C_TEPaste( TEHandle teh ); extern pascal trap void P_TEPaste( TEHandle teh); 
extern pascal trap void C_TEDelete( TEHandle teh ); extern pascal trap void P_TEDelete( TEHandle teh); 
extern pascal trap void C_TEInsert( Ptr p, LONGINT ln, TEHandle teh ); extern pascal trap void P_TEInsert( Ptr p, LONGINT ln, TEHandle teh); 
extern pascal trap void C_TEPinScroll( INTEGER dh, 
 INTEGER dv, TEHandle teh ); extern pascal trap void P_TEPinScroll( INTEGER dh, 
 INTEGER dv, TEHandle teh ); 
extern void ROMlib_teautoloop( TEHandle teh ); 
extern pascal trap void C_TESelView( TEHandle teh ); extern pascal trap void P_TESelView( TEHandle teh); 
extern pascal trap void C_TEAutoView( BOOLEAN autoflag, 
 TEHandle teh ); extern pascal trap void P_TEAutoView( BOOLEAN autoflag, 
 TEHandle teh ); 
extern pascal trap TEHandle C_TEStylNew( Rect *dst, Rect *view ); extern pascal trap TEHandle P_TEStylNew( Rect *dst, Rect *view); 
extern pascal trap void C_SetStylHandle( TEStyleHandle theHandle, 
 TEHandle teh ); extern pascal trap void P_SetStylHandle( TEStyleHandle theHandle, 
 TEHandle teh ); 
extern pascal trap TEStyleHandle C_GetStylHandle( TEHandle teh ); extern pascal trap TEStyleHandle P_GetStylHandle( TEHandle teh); 
extern pascal trap StScrpHandle C_GetStylScrap( TEHandle teh ); extern pascal trap StScrpHandle P_GetStylScrap( TEHandle teh); 
extern pascal trap void C_TEStylInsert( Ptr text, LONGINT length, 
 StScrpHandle hST, TEHandle teh ); extern pascal trap void P_TEStylInsert( Ptr text, LONGINT length, 
 StScrpHandle hST, TEHandle teh ); 
extern pascal trap INTEGER C_TEGetOffset( Point pt, TEHandle teh ); extern pascal trap INTEGER P_TEGetOffset( Point pt, TEHandle teh); 
extern pascal trap LONGINT C_TEGetPoint( INTEGER offset, TEHandle teh ); extern pascal trap LONGINT P_TEGetPoint( INTEGER offset, TEHandle teh); 
extern pascal trap int32 C_TEGetHeight (LONGINT endLine, LONGINT startLine, TEHandle teh);
extern pascal trap LONGINT P_TEGetHeight( LONGINT endLine, 
 LONGINT startLine, TEHandle teh ); 
extern pascal trap void C_TEGetStyle( INTEGER offset, 
 TextStyle *theStyle, INTEGER *lineHeight, INTEGER *fontAscent, 
 TEHandle teh ); extern pascal trap void P_TEGetStyle( INTEGER offset, 
 TextStyle *theStyle, INTEGER *lineHeight, INTEGER *fontAscent, 
 TEHandle teh ); 
extern pascal trap void C_TEStylPaste( TEHandle teh ); extern pascal trap void P_TEStylPaste( TEHandle teh); 
extern pascal trap void C_TESetStyle( INTEGER mode, TextStyle *newStyle, 
 BOOLEAN redraw, TEHandle teh ); extern pascal trap void P_TESetStyle( INTEGER mode, TextStyle *newStyle, 
 BOOLEAN redraw, TEHandle teh ); 
extern pascal trap void C_TEReplaceStyle( INTEGER mode, 
TextStyle *oldStyle, TextStyle *newStyle, BOOLEAN redraw, TEHandle teh ); extern pascal trap void P_TEReplaceStyle( INTEGER mode, 
TextStyle *oldStyle, TextStyle *newStyle, BOOLEAN redraw, TEHandle teh ); 
extern pascal trap BOOLEAN C_TEContinuousStyle( INTEGER *modep, 
 TextStyle *thestyle, TEHandle teh ); extern pascal trap BOOLEAN P_TEContinuousStyle( INTEGER *modep, 
 TextStyle *thestyle, TEHandle teh ); 
extern pascal trap void C_SetStylScrap( LONGINT start, LONGINT stop, 
 StScrpHandle newstyles, BOOLEAN redraw, TEHandle teh ); extern pascal trap void P_SetStylScrap( LONGINT start, LONGINT stop, 
 StScrpHandle newstyles, BOOLEAN redraw, TEHandle teh ); 
extern pascal trap void C_TECustomHook( INTEGER sel, HIDDEN_ProcPtr *addr, 
 TEHandle teh ); extern pascal trap void P_TECustomHook( INTEGER sel, HIDDEN_ProcPtr *addr, 
 TEHandle teh ); 
extern pascal trap LONGINT C_TENumStyles( LONGINT start, LONGINT stop, 
 TEHandle teh ); extern pascal trap LONGINT P_TENumStyles( LONGINT start, LONGINT stop, 
 TEHandle teh ); 
extern pascal trap void C_TEInit( void  ); extern pascal trap void P_TEInit( void ); 
extern pascal trap TEHandle C_TENew( Rect *dst, Rect *view ); extern pascal trap TEHandle P_TENew( Rect *dst, Rect *view); 
extern pascal trap void C_TEDispose( TEHandle teh ); extern pascal trap void P_TEDispose( TEHandle teh); 
extern pascal trap void C_TEIdle( TEHandle teh ); extern pascal trap void P_TEIdle( TEHandle teh); 
extern pascal trap void C_TEClick( Point p, BOOLEAN ext, TEHandle teh ); extern pascal trap void P_TEClick( Point p, BOOLEAN ext, TEHandle teh); 
extern pascal trap void C_TESetSelect( LONGINT start, LONGINT stop, 
 TEHandle teh ); extern pascal trap void P_TESetSelect( LONGINT start, LONGINT stop, 
 TEHandle teh ); 
extern pascal trap void C_TEActivate( TEHandle teh ); extern pascal trap void P_TEActivate( TEHandle teh); 
extern pascal trap void C_TEDeactivate( TEHandle teh ); extern pascal trap void P_TEDeactivate( TEHandle teh); 
extern void SetWordBreak( ProcPtr wb, TEHandle teh ); 
extern void SetClikLoop( ProcPtr cp, TEHandle teh ); 
extern pascal trap void C_TECalText( TEHandle teh ); extern pascal trap void P_TECalText( TEHandle teh); 
extern OSErr TEFromScrap( void  ); 
extern OSErr TEToScrap( void  ); 
extern Handle TEScrapHandle( void  ); 
extern LONGINT TEGetScrapLen( void  ); 
extern void TESetScrapLen( LONGINT ln ); 
extern pascal trap int16 C_TEFeatureFlag (int16 feature, int16 action,
					  TEHandle te);
}
#endif /* _TEXTEDIT_H_ */
