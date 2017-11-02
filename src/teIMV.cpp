/* Copyright 1986 - 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in TextEdit.h (DO NOT DELETE THIS LINE) */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_teIMV[] =
	    "$Id: teIMV.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "QuickDraw.h"
#include "WindowMgr.h"
#include "ControlMgr.h"
#include "ToolboxUtil.h"
#include "FontMgr.h"
#include "TextEdit.h"
#include "MemoryMgr.h"
#include "ScrapMgr.h"

#include "rsys/cquick.h"
#include "rsys/mman.h"
#include "rsys/tesave.h"
#include "rsys/hook.h"

using namespace Executor;

void
Executor::generic_elt_copy (generic_elt_t *dst, generic_elt_t *src)
{
  *dst = *src;
}

void
Executor::generic_elt_calc_height_ascent (generic_elt_t *elt)
{
  int16 savesize, savefont;
  Style saveface;
  FontInfo font_info;
  
  savesize = PORT_TX_SIZE (thePort);
  savefont = PORT_TX_FONT (thePort);
  saveface = PORT_TX_FACE_X (thePort);
  TextSize (GENERIC_ELT_SIZE (elt));
  TextFont (GENERIC_ELT_FONT (elt));
  TextFace (GENERIC_ELT_FACE (elt));
  GetFontInfo (&font_info);
  GENERIC_ELT_HEIGHT_X (elt) = CW (  CW (font_info.ascent)
				   + CW (font_info.descent)
				   + CW (font_info.leading));
  GENERIC_ELT_ASCENT_X (elt) = font_info.ascent;
  TextSize (savesize);
  TextFace (saveface);
  TextFont (savefont);
}

bool
Executor::adjust_attrs (TextStyle *orig_attrs, TextStyle *new_attrs,
	      TextStyle *dst_attrs, TextStyle *continuous_attrs,
	      int16 mode)
{
  GUEST<int16> orig_font, orig_size;
  
  orig_font = TS_FONT_X (orig_attrs);
  orig_size = TS_SIZE_X (orig_attrs);
  
  /* compute the new style for this run */
  if (mode & doFont)
    TS_FONT_X (dst_attrs) = TS_FONT_X (new_attrs);
  else
    TS_FONT_X (dst_attrs) = TS_FONT_X (orig_attrs);
  
  /* ### we don't handle doToggle correctly */
  if (mode & doFace)
    {
      if (mode & doToggle && continuous_attrs)
	TS_FACE (dst_attrs) = ((TS_FACE (orig_attrs) & ~TS_FACE (new_attrs))
			       | (TS_FACE (new_attrs)
				  ^ TS_FACE (continuous_attrs)));
      else
	TS_FACE (dst_attrs) = TS_FACE (new_attrs);
    }
  else
    TS_FACE (dst_attrs) = TS_FACE (orig_attrs);
  
  if (mode & addSize)
    TS_SIZE_X (dst_attrs) = CW (TS_SIZE (new_attrs) + TS_SIZE (orig_attrs));
  else if (mode & doSize)
    TS_SIZE_X (dst_attrs) = TS_SIZE_X (new_attrs);
  else
    TS_SIZE_X (dst_attrs) = TS_SIZE_X (orig_attrs);
  
  if (mode & doColor)
    TS_COLOR (dst_attrs) = TS_COLOR (new_attrs);
  else
    TS_COLOR (dst_attrs) = TS_COLOR (orig_attrs);

  return (orig_font != TS_FONT_X (dst_attrs)
	  || orig_size != TS_SIZE_X (dst_attrs));
}

/* return the index into `runs' that has a starting char `sel'.  if no
   such run exists, create it */

int16
Executor::make_style_run_at (TEStyleHandle te_style, int16 sel)
{
  int run_index;

  run_index = te_char_to_run_index (te_style, sel);
  
  {
    StyleRun *run = TE_STYLE_RUN (te_style, run_index);
    if (STYLE_RUN_START_CHAR (run) == sel)
      return run_index;
  }

  {
    StyleRun *runs;
    int n_runs;
    STHandle style_table;
    STElement *style;
    int style_index;
    
    /* split the current style into two */
    n_runs = TE_STYLE_N_RUNS (te_style) + 1;
    TE_STYLE_N_RUNS_X (te_style) = CW (n_runs);
    SetHandleSize ((Handle) te_style,
		   TE_STYLE_SIZE_FOR_N_RUNS (n_runs));
    runs = TE_STYLE_RUNS (te_style);
    memmove (&runs[run_index + 2],
	     &runs[run_index + 1],
	     (n_runs - run_index - 1) * sizeof *runs);
    
    style_index = STYLE_RUN_STYLE_INDEX (&runs[run_index]);
    
    /* we created a new run with this style index, update the
       reference count appropriately */
    style_table = TE_STYLE_STYLE_TABLE (te_style);
    style = ST_ELT (style_table, style_index);
    
    ST_ELT_COUNT_X (style) = CW (ST_ELT_COUNT (style) + 1);
    
    STYLE_RUN_START_CHAR_X (&runs[run_index + 1])  = CW (sel);
    STYLE_RUN_STYLE_INDEX_X (&runs[run_index + 1]) = CW (style_index);
    
    return run_index + 1;
  }
}

/* return the style index for the style that has the text attributes
   `attrs'.  if no such style exists, create it.

   assign `count_changed_p' to TRUE if no such style existed, and it
   was necessary to allocate a new style

   if `incr_count_p' this function increases the style reference on
   the returned style by one */
   
int16
Executor::get_style_index (TEStyleHandle te_style, TextStyle *attrs, int incr_count_p)
{
  /* these hold the swapped cached height, ascent for the style we are
     searching for */
  GUEST<int16> cached_height = CWC(-1), cached_ascent = CWC(-1);
  int cache_filled_p = FALSE;
  STHandle style_table;
  STElement *st_elt;
  int st_i;
  int n_styles;

  n_styles = TE_STYLE_N_STYLES (te_style);
  style_table = TE_STYLE_STYLE_TABLE (te_style);
  for (st_i = 0; st_i < n_styles; st_i ++)
    {
      st_elt = ST_ELT (style_table, st_i);
      
      if (TS_FONT_X (attrs) == ST_ELT_FONT_X (st_elt)
	  && TS_SIZE_X (attrs) == ST_ELT_SIZE_X (st_elt))
	{
	  if (TS_FACE (attrs) == ST_ELT_FACE (st_elt)
	      && (TS_COLOR (attrs).red == ST_ELT_COLOR (st_elt).red
		  && TS_COLOR (attrs).green == ST_ELT_COLOR (st_elt).green
		  && TS_COLOR (attrs).blue == ST_ELT_COLOR (st_elt).blue))
	    {
	      if (incr_count_p)
		ST_ELT_COUNT_X (st_elt) = CW (ST_ELT_COUNT (st_elt) + 1);
	      return st_i;
	    }
	  else if (!cache_filled_p)
	    {
	      cached_height = ST_ELT_HEIGHT_X (st_elt);
	      cached_ascent = ST_ELT_ASCENT_X (st_elt);
	      cache_filled_p = TRUE;
	    }
	}
    }

  /* a style not already in the style table was asked for.  create it */
  n_styles ++;
  TE_STYLE_N_STYLES_X (te_style) = CW (n_styles);
  SetHandleSize ((Handle) style_table,
		 STYLE_TABLE_SIZE_FOR_N_STYLES (n_styles));
  st_elt = ST_ELT (style_table, n_styles - 1);
  
  ST_ELT_COUNT_X (st_elt) = CWC (incr_count_p ? 1 : 0);
  
  ST_ELT_FONT_X (st_elt) = TS_FONT_X (attrs);
  ST_ELT_FACE (st_elt)   = TS_FACE (attrs);
  ST_ELT_SIZE_X (st_elt) = TS_SIZE_X (attrs);
  ST_ELT_COLOR (st_elt)  = TS_COLOR (attrs);

  if (cache_filled_p)
    {
      ST_ELT_HEIGHT_X (st_elt) = cached_height;
      ST_ELT_ASCENT_X (st_elt) = cached_ascent;
    }
  else
    generic_elt_calc_height_ascent (ST_ELT_TO_GENERIC_ELT (st_elt));

  return n_styles - 1;
}

/* decrease the reference count of style at `style_index' by one */

void
Executor::release_style_index (TEStyleHandle te_style, int16 style_index)
{
  STHandle style_table;
  STElement *st_elt;
  
  style_table = TE_STYLE_STYLE_TABLE (te_style);
  st_elt = ST_ELT (style_table, style_index);
  gui_assert (ST_ELT_COUNT (st_elt) > 0);
  ST_ELT_COUNT_X (st_elt) = CW (ST_ELT_COUNT (st_elt) - 1);
}

/* `release_style_index ()' only decreases the reference count, so
   there may some styles with zero reference counts.  remove those
   styles from the style table, and update the style count
   appropriately */
  
void
Executor::stabilize_style_info (TEStyleHandle te_style)
{
  STHandle style_table;
  STElement *st_elt;
  /* map from original style indexes to new style indexes */
  GUEST<int16> *index_map;
  int16 n_styles, n_runs;
  int i;
  
  n_styles = TE_STYLE_N_STYLES (te_style);
  style_table = TE_STYLE_STYLE_TABLE (te_style);
  index_map = (GUEST<int16>*)alloca (n_styles * sizeof *index_map);
  
  for (i = 0; i < n_styles; i ++)
    index_map[i] = CW (i);
  
  for (i = 0; i < n_styles; i ++)
    {
      st_elt = ST_ELT (style_table, i);
      
      if (! ST_ELT_COUNT_X (st_elt))
	{
	  /* make sure that the last style element in the table is
	     used */
	  for (;;)
	    {
	      STElement *last_st_elt;
	      
	      last_st_elt = ST_ELT (style_table, n_styles - 1);
	      if (ST_ELT_COUNT_X (last_st_elt))
		break;
	      
	      n_styles --;
	      
	      if (last_st_elt == st_elt)
		goto done;
	    }
	  
	  /* there are no runs with the style index `i', so give that
	     index to the last style in the style table, and shrink
	     the style table by 1 */
	  *ST_ELT (style_table, i) = *ST_ELT (style_table, n_styles - 1);
	  index_map[n_styles - 1] = CW (i);

	  /* so that we can verify, when we change the run style
	     indexes, that noone refers to this map index */
	  index_map[i] = CWC (-1);
	  
	  n_styles --;
	}
    }
 done:
  
  TE_STYLE_N_STYLES_X (te_style) = CW (n_styles);
  SetHandleSize ((Handle) style_table,
		 STYLE_TABLE_SIZE_FOR_N_STYLES (n_styles));
  
  n_runs = TE_STYLE_N_RUNS (te_style);
  
  /* now map over every run, and map the orig style index to
     it's new style index */
  for (i = 0; i < n_runs; i ++)
    {
      StyleRun *run;
      
      run = TE_STYLE_RUN (te_style, i);
      STYLE_RUN_STYLE_INDEX_X (run)
	= index_map[STYLE_RUN_STYLE_INDEX (run)];
      gui_assert (STYLE_RUN_STYLE_INDEX_X (run) != CWC (-1));
    }
}

static void
combine_run_with_next (TEStyleHandle te_style, int16 run_index)
{
  StyleRun *runs;
  int16 n_runs;
  STHandle style_table;
  STElement *style;
  int16 style_index;
  
  n_runs = TE_STYLE_N_RUNS (te_style);
  runs = TE_STYLE_RUNS (te_style);
  
  memmove (&runs[run_index + 1], &runs[run_index + 2],
	   (n_runs - run_index - 1) * sizeof *runs);
  
  n_runs --;
  TE_STYLE_N_RUNS_X (te_style) = CW (n_runs);
  SetHandleSize ((Handle) te_style,
		 TE_STYLE_SIZE_FOR_N_RUNS (n_runs));
  
  style_index = STYLE_RUN_STYLE_INDEX (&runs[run_index]);
  style_table = TE_STYLE_STYLE_TABLE (te_style);
  style = ST_ELT (style_table, style_index);
  ST_ELT_COUNT_X (style) = CW (ST_ELT_COUNT (style) - 1);
}

void
Executor::te_style_combine_runs (TEStyleHandle te_style)
{
  int16 current_run_index, n_runs;
  
  /* remove any extra run fragmentation; if two adjacent runs are the
     same style, combine them */
  n_runs = TE_STYLE_N_RUNS (te_style);
  for (current_run_index = 0;
       current_run_index < n_runs - 1;)
    {
      StyleRun *current_run, *next_run;
      
      current_run = TE_STYLE_RUN (te_style, current_run_index);
      next_run = TE_STYLE_RUN (te_style, current_run_index + 1);
      
      if (STYLE_RUN_STYLE_INDEX (current_run) == STYLE_RUN_STYLE_INDEX (next_run))
	{
	  /* styles are the same, combine them */
	  combine_run_with_next (te_style, current_run_index);
	  
	  n_runs --;
	}
      else
	current_run_index ++;
    }
  
}

/* add the attributes specified by `mode' in `attrs' to the range of
   text between `start' and `end' in the text edit record `te'.  note
   mode may also include `addSize' */

static void
te_add_attrs_to_range (TEHandle te,
		       int16 start, int16 end,
		       TextStyle *attrs, int16 mode)
{
  TEStyleHandle te_style;
  STHandle style_table;
  int16 start_run_index, end_run_index;
  int16 current_run_index;
  
  TextStyle continuous_attrs;
  
  if (mode & doToggle)
    {
      GUEST<int16> continuous_mode = CWC (doFace);
      
      TS_FACE (&continuous_attrs) = TS_FACE (attrs);
      TEContinuousStyle (&continuous_mode, &continuous_attrs, te);
    }
  
  te_style = TE_GET_STYLE (te);
  style_table = TE_STYLE_STYLE_TABLE (te_style);
  
  /* make sure that a style run starts and `start' and that a style
     run starts at `end' */
  start_run_index = make_style_run_at (te_style, start);
  end_run_index = make_style_run_at (te_style, end);
  
  LOCK_HANDLE_EXCURSION_1
    (te_style,
     {
       StyleRun *runs = TE_STYLE_RUNS (te_style);
       
       /* now go through each style and make the appropriate change */
       for (current_run_index = start_run_index;
	    current_run_index < end_run_index;
	    current_run_index ++)
	 {
	   StyleRun *current_run;
	   TextStyle new_attrs;
	   STElement *orig_style;
	   int16 orig_style_index;
	   int16 new_style_index;
	   
	   current_run = &runs[current_run_index];
	   orig_style_index = STYLE_RUN_STYLE_INDEX (current_run);
	   orig_style = ST_ELT (style_table, orig_style_index);
	   
	   adjust_attrs (ST_ELT_TO_ATTR (orig_style), attrs, &new_attrs,
			 &continuous_attrs, mode);
	   
	   /* `current_run' valid over call to `get_style_index ()'
	      since `te_style' is locked */
	   new_style_index = get_style_index (te_style, &new_attrs, TRUE);
	   release_style_index (te_style, orig_style_index);
	   
	   STYLE_RUN_STYLE_INDEX_X (current_run) = CW (new_style_index);
	 }
     });
  
  stabilize_style_info (te_style);
  te_style_combine_runs (te_style);
}

P2 (PUBLIC pascal trap, TEHandle, TEStylNew, Rect *, dst, Rect *, view)
{
  FontInfo font_info;
  int16 font_height;
  TEHandle teh;
  TEStyleHandle te_style;
  StScrpHandle stsh;
  ScrpSTElement *stp;
  LHHandle lh_table;
  LHPtr lh;
  NullSTHandle tempnullsth;
  STHandle style_table;
  
  teh = TENew (dst, view);
  
  HASSIGN_3
    (teh,
     lineHeight, CWC (-1),
     fontAscent, CWC (-1),
     txSize, CWC (-1));
  
  te_style = (TEStyleHandle) NewHandle (TE_STYLE_SIZE_FOR_N_RUNS (1));
  
  HxX (te_style, nRuns) = CWC (1);
  HxX (te_style, runs[0].startChar)  = CWC (0);
  HxX (te_style, runs[0].styleIndex) = CWC (0);
  HxX (te_style, runs[1].startChar)  = CWC (1);
  HxX (te_style, runs[1].styleIndex) = CWC (-1);
  
  HxX (te_style, nStyles) = CWC (1);
  
  /* font info used to fill in the fisrt style element */
  GetFontInfo (&font_info);
  font_height = (CW (font_info.ascent)
		 + CW (font_info.descent)
		 + CW (font_info.leading));
  style_table = (STHandle) NewHandle (sizeof (STElement));
  HASSIGN_7
    (style_table,
     stCount, CWC (1),
     stFont, PORT_TX_FONT_X (thePort),
     stFace, PORT_TX_FACE (thePort),
     stSize, PORT_TX_SIZE_X (thePort),
     stColor, ROMlib_black_rgb_color,
     stHeight, CW (font_height),
     stAscent, font_info.ascent);
  
  TE_STYLE_STYLE_TABLE_X (te_style) = RM (style_table);
  
  lh_table = (LHHandle) NewHandle (sizeof (LHElement));
  lh = STARH (lh_table);
  LH_HEIGHT_X (lh) = CW(font_height);
  LH_ASCENT_X (lh) = font_info.ascent;
  
  TE_STYLE_LH_TABLE_X (te_style) = RM (lh_table);
  
  HxX(te_style, teRefCon) = 0;
  
  tempnullsth = (NullSTHandle) NewHandle(sizeof(NullSTRec));
  HxX (te_style, nullStyle) = RM(tempnullsth);
  stsh = (StScrpHandle) NewHandle(sizeof(StScrpRec));
  HxX (tempnullsth, nullScrap) = RM((StScrpHandle)stsh);
  HxX (tempnullsth, TEReserved) = CLC(0);
  HxX (stsh, scrpNStyles) = CWC(0);

  stp = HxX(stsh, scrpStyleTab);
  stp->scrpFont = PORT_TX_FONT_X (thePort);
  stp->scrpFace = PORT_TX_FACE_X (thePort);
  stp->scrpSize = PORT_TX_SIZE_X (thePort);
  stp->scrpColor.red = 0;		/* black ? */
  stp->scrpColor.green = 0;		/* black ? */
  stp->scrpColor.blue = 0;		/* black ? */
  stp->scrpStartChar = CLC(0);
  
  stp->scrpHeight = CW (font_height);
  stp->scrpAscent = font_info.ascent;
  
  SetStylHandle (te_style, teh);
  
  TE_SLAM (teh);
  
  return teh;
}

P2(PUBLIC pascal trap, void, SetStylHandle, TEStyleHandle, theHandle,
								 TEHandle, teh)
{
  if (!TE_STYLIZED_P (teh))
      return;
  *(GUEST<TEStyleHandle> *) &HxX (teh, txFont) = RM (theHandle);
}

P1 (PUBLIC pascal trap, TEStyleHandle, GetStylHandle, TEHandle, teh)
{
  return TE_GET_STYLE (teh);
}

P1 (PUBLIC pascal trap, StScrpHandle, GetStylScrap, TEHandle, te)
{
  StScrpHandle scrap;
  StyleRun *runs;
  TEStyleHandle te_style;
  STHandle style_table;
  int16 start, end, length;
  int16 start_run_index, end_run_index;
  int16 scrap_n_styles;
  int i;
  
  TE_SLAM (te);
  
  start = TE_SEL_START (te); 
  end = TE_SEL_END (te);
  length = TE_LENGTH (te);
  if (!TE_STYLIZED_P (te))
    return NULL;

  if (end < start)
    {
      warning_unexpected ("end < start");
      end = start;
    }

  te_style = TE_GET_STYLE (te);
  style_table = TE_STYLE_STYLE_TABLE (te_style);
  if (end > length)
    end = length;
  
  start_run_index = make_style_run_at (te_style, start);
  end_run_index = make_style_run_at (te_style, end);
  
  scrap_n_styles = MAX (end_run_index - start_run_index, 1);
  scrap = (StScrpHandle) NewHandle (SCRAP_SIZE_FOR_N_STYLES (scrap_n_styles));
  SCRAP_N_STYLES_X (scrap) = CW (scrap_n_styles);
  
  if (start == end)
    warning_unimplemented ("should check null scrap, first");

  runs = TE_STYLE_RUNS (te_style);
  
  for (i = 0; i < scrap_n_styles; i ++)
    {
      ScrpSTElement *scrap_elt;
      StyleRun *current_run;
      STElement *style;
      
      current_run = &runs[start_run_index + i];
      style = ST_ELT (style_table, RUN_STYLE_INDEX (current_run));
      scrap_elt = SCRAP_ST_ELT (scrap, i);
      
      generic_elt_copy (SCRAP_ELT_TO_GENERIC_ELT (scrap_elt),
			ST_ELT_TO_GENERIC_ELT (style));
      SCRAP_ELT_START_CHAR_X (scrap_elt)
	= CL (RUN_START_CHAR (current_run) - start);
    }
  te_style_combine_runs (te_style);
  
  return scrap;
}

P4 (PUBLIC pascal trap, void, TEStylInsert, Ptr, text, LONGINT, length,
    StScrpHandle, hST, TEHandle, te)
{
  TE_SLAM (te);
  ROMlib_tedoitall (te, text, length, TRUE, hST);
  TE_SLAM (te);
}

P2 (PUBLIC pascal trap, INTEGER, TEGetOffset, Point, pt, TEHandle, te)
{
  int16 retval;
  GUEST<Point> sp;
  
  sp = TE_SEL_POINT (te);
  TE_SEL_POINT (te).h = CW (pt.h);
  TE_SEL_POINT (te).v = CW (pt.v);
  retval = TE_DO_TEXT (te, 0, TE_LENGTH (te), teFind);
  TE_SEL_POINT (te) = sp;
  
  return retval;
}

P2 (PUBLIC pascal trap, LONGINT, TEGetPoint, INTEGER, offset, TEHandle, teh)
{
  Point p;
  int ascent;
  
  TE_CHAR_TO_POINT (teh, offset, &p);
  LOCK_HANDLE_EXCURSION_1
    (teh,
     {
       TEPtr tep = STARH (teh);
       int lineno;
       
       lineno = TEP_CHAR_TO_LINENO (tep, offset);
       ascent = TEP_ASCENT_FOR_LINE (tep, lineno);
     });

  return ((int32) (p.v + ascent) << 16) + (int32) p.h;
}


P3 (PUBLIC pascal trap, int32, TEGetHeight,
    LONGINT, endLine, LONGINT, startLine, TEHandle, teh)
{
  int32 retval;
  
  if (startLine > 0)
    startLine --;
  else
    startLine = 0;

  endLine = MIN (TE_N_LINES (teh), endLine);
  if (endLine < 0)
    endLine = 0;
  else if (endLine > 0)
    endLine --;
  
  /* ### 1. write a `swap' macro, that should go into `rsys/macros.h',
     and 2. pin start and end by `TE_N_LINES ()' */
  if (startLine > endLine)
    {
      uint32 temp;
      
      temp = startLine;
      startLine = endLine;
      endLine = temp;
    }
  
  if (TE_STYLIZED_P (teh))
    {
      TEStyleHandle te_style;
      LHElement *l, *le;
      
      retval = 0;
      
      te_style = TE_GET_STYLE (teh);
      
      l = STARH (MR (STARH (te_style)->lhTab)) + startLine;
      le = l + endLine - startLine;
      for ( ; l <= le ; l++)
	retval += CW (l->lhHeight);
    }
  else
    retval = TE_LINE_HEIGHT (teh) * (endLine - startLine + 1);
  
  return retval;
}

P5 (PUBLIC pascal trap, void, TEGetStyle, int16, sel,
    TextStyle *, attrs, GUEST<int16> *, line_height, GUEST<int16> *, font_ascent,
    TEHandle, te)
{
  if (TE_STYLIZED_P (te))
    {
      TEStyleHandle te_style;
      STElement *style;
      StyleRun *runs;
      int run_index;
      
      te_style = TE_GET_STYLE (te);
      runs = TE_STYLE_RUNS (te_style);
      run_index = te_char_to_run_index (te_style, sel);
      style = ST_ELT (TE_STYLE_STYLE_TABLE (te_style),
		      RUN_STYLE_INDEX (&runs[run_index]));
      
      TS_FONT_X (attrs) = ST_ELT_FONT_X (style);
      TS_FACE (attrs)   = ST_ELT_FACE (style);
      TS_SIZE_X (attrs) = ST_ELT_SIZE_X (style);
      TS_COLOR (attrs)   = ST_ELT_COLOR (style);
      
      *line_height = ST_ELT_HEIGHT_X (style);
      *font_ascent = ST_ELT_ASCENT_X (style);
    }
  else
    {
      TS_FONT_X (attrs) = TE_TX_FONT_X (te);
      TS_FACE (attrs)   = TE_TX_FACE (te);
      TS_SIZE_X (attrs) = TE_TX_SIZE_X (te);
      TS_COLOR (attrs) = ROMlib_black_rgb_color;
      *line_height = TE_LINE_HEIGHT_X (te);
      *font_ascent = TE_FONT_ASCENT_X (te);
    }
}

P1 (PUBLIC pascal trap, void, TEStylPaste, TEHandle, te)
{
  Handle hText;
  GUEST<int32> dummy;
  /* length of the scrap text */
  int16 length, retval;
  StScrpHandle scrap;
  

  hText = NewHandle (1);
  length = GetScrap (hText, TICK ("TEXT"), &dummy);
  if (length < 0)
    {
      /* error, there is no scrap element */
      DisposHandle (hText);
      
      /* remove the selected text */
      ROMlib_tedoitall (te, NULL, 0, FALSE, NULL);
      return;
    }
  
  scrap = (StScrpHandle) NewHandle (sizeof (StScrpRec));
  retval = GetScrap ((Handle) scrap, TICK ("styl"), &dummy);
  if (retval < 0)
    {
      DisposHandle ((Handle) scrap);
      scrap = NULL;
    }
  
  LOCK_HANDLE_EXCURSION_1
    (hText,
     {
       ROMlib_tedoitall (te, STARH (hText), length, FALSE, scrap);
     });
  if (scrap)
    DisposHandle ((Handle) scrap);
  DisposHandle (hText);
}

static void
te_do_redraw (TEHandle te)
{
  int16 cal_start, cal_end, sel_start, sel_end;
  
  TESAVE (te);
  
  sel_start = TE_SEL_START (te);
  sel_end = TE_SEL_END (te);
  
  ROMlib_caltext (te, sel_start, 0, &cal_start, &cal_end);
  if (cal_start > sel_start)
    cal_start = sel_start;
  if (cal_end < sel_end)
    cal_end = sel_end;
  
  TE_DO_TEXT (te, cal_start, cal_end, teDraw);
  TE_DO_TEXT (te, sel_start, sel_end, teHilite);
  
  TERESTORE ();
}

P4 (PUBLIC pascal trap, void, TESetStyle, int16, mode, TextStyle *, new_attrs,
    BOOLEAN, redraw, TEHandle, te)
{
  int16 start, end;
  
  if (!TE_STYLIZED_P (te))
    return;
  
  TE_SLAM (te);
  
  start = TE_SEL_START (te);
  end = TE_SEL_END (te);
  
  if (start == end)
    {
      TEStyleHandle te_style;
      StScrpHandle null_scrap;
      ScrpSTElement *scrap_st_elt;

      /* store new attributes into the null scrap style */
      
      te_style = TE_GET_STYLE (te);
      null_scrap = TE_STYLE_NULL_SCRAP (te_style);
      scrap_st_elt = SCRAP_ST_ELT (null_scrap, 0);
      if (!SCRAP_N_STYLES (null_scrap))
	{
	  STHandle style_table;	  
	  StyleRun *runs;
	  int start_run_index, start_style_index;
	  
	  style_table = TE_STYLE_STYLE_TABLE (te_style);
	  runs = TE_STYLE_RUNS (te_style);
	  start_run_index = te_char_to_run_index (te_style, start);
	  start_style_index = RUN_STYLE_INDEX (&runs[start_run_index]);
	  
	  /* ### if the scrap has no styles, the old TESetStyle code
	     copied the style from the insertion point.  although this
	     seems reasonable, i haven't been able to find this
	     documented anywhere */
	  generic_elt_copy (ST_ELT_TO_GENERIC_ELT (ST_ELT (style_table,
							   start_style_index)),
			    SCRAP_ELT_TO_GENERIC_ELT (scrap_st_elt));
	  
	  SCRAP_N_STYLES_X (null_scrap) = CWC (1);
	}
      if (adjust_attrs (SCRAP_ELT_TO_ATTR (scrap_st_elt), new_attrs,
			SCRAP_ELT_TO_ATTR (scrap_st_elt),
			SCRAP_ELT_TO_ATTR (scrap_st_elt), mode))
	generic_elt_calc_height_ascent
	  (SCRAP_ELT_TO_GENERIC_ELT (scrap_st_elt));
      
      return;
    }
  
  te_add_attrs_to_range (te, start, end, new_attrs, mode);

  if (redraw)
    te_do_redraw (te);
  
  TE_SLAM (te);
}

P5 (PUBLIC pascal trap, void, TEReplaceStyle, int16, mode,
    TextStyle *, attrs_to_replace, TextStyle *, replacement_attrs, BOOLEAN, redraw,
    TEHandle, te)
{
  TEStyleHandle te_style;
  SignedByte te_style_flags;
  int16 sel_start, sel_end;
  int16 start_run_index, end_run_index;
  STHandle style_table;
  StyleRun *runs;
  int16 run_i;
  
  TE_SLAM (te);
  
  sel_start = TE_SEL_START (te);
  sel_end = TE_SEL_END (te);
  if (! TE_STYLIZED_P (te)
      || sel_start == sel_end)
    return;
  
  te_style = TE_GET_STYLE(te);

  /* create the new runs before locking `te_style' */
  start_run_index = make_style_run_at (te_style, sel_start);
  end_run_index = make_style_run_at (te_style, sel_end);
  
  te_style_flags = HGetState ((Handle) te_style);
  HLock ((Handle) te_style);
  
  runs = TE_STYLE_RUNS (te_style);
  
  style_table = TE_STYLE_STYLE_TABLE (te_style);
  
  for (run_i = start_run_index; run_i < end_run_index; run_i ++)
    {
      StyleRun *run;
      int16 orig_style_index;
      STElement *style;
      
      run = &runs[run_i];
      orig_style_index = RUN_STYLE_INDEX (run);
      style = ST_ELT (style_table, orig_style_index);
      
      /* ### from the IM documentation, it isn't clear if
	 only those attributes specified by `mode' need to match,
	 or if all attributes need to match */
      
      if ((! (mode & doFont)
	   || ST_ELT_FONT_X (style) == TS_FONT_X (attrs_to_replace))
	  && (! (mode & doFace)
	      || ST_ELT_FACE (style) == TS_FACE (attrs_to_replace))
	  && (! (mode & (doSize | addSize))
	      || ST_ELT_SIZE_X (style) == TS_SIZE_X (attrs_to_replace))
	  && (! (mode & doColor)
	      || !memcmp (&ST_ELT_COLOR (style),
			  &TS_COLOR (attrs_to_replace),
			  sizeof (RGBColor))))
	{
	  int16 new_style_index;
	  
	  TextStyle *new_attrs = (TextStyle*)alloca (sizeof *new_attrs);
	  
	  adjust_attrs (ST_ELT_TO_ATTR (style),	replacement_attrs, new_attrs,
			NULL, mode);

	  /* `get_style_index' may resize `style_table', so `style'
	     is no longer valid */
	  new_style_index = get_style_index (te_style, new_attrs, TRUE);
	  release_style_index (te_style, orig_style_index);
	  
	  RUN_STYLE_INDEX_X (run) = CW (new_style_index);
	}
    }
  HSetState ((Handle) te_style, te_style_flags);
  
  stabilize_style_info (te_style);
  te_style_combine_runs (te_style);
  
  if (redraw)
    te_do_redraw (te);

  TE_SLAM (te);
}

P3 (PUBLIC pascal trap, BOOLEAN, TEContinuousStyle, GUEST<INTEGER> *, modep,
    TextStyle *, ts_out, TEHandle, teh)
{
  int16 sel_start, sel_end;
  TEStyleHandle te_style;
  STHandle style_table;
  STElement *style;
  int style_index = -1;
  int run_i;
  GUEST<int16> orig_mode;

  if (!TE_STYLIZED_P (teh))
    {
      warning_unimplemented (NULL_STRING);
      if (*modep & CWC (doFont))
	TS_FONT_X (ts_out) = PORT_TX_FONT_X (thePort);
      if (*modep & CWC (doFace))
	TS_FACE (ts_out) = PORT_TX_FACE_X (thePort);
      if (*modep & CWC (doSize))
	TS_SIZE_X (ts_out) = PORT_TX_SIZE_X (thePort);
      if (*modep & CWC (doColor))
	TS_COLOR (ts_out) = ROMlib_black_rgb_color;
      return TRUE;
    }

  /* just slam on entry, this function doesn't modify `teh' */
  TE_SLAM (teh);
  
  te_style = TE_GET_STYLE (teh);
  style_table = TE_STYLE_STYLE_TABLE (te_style);
  
  sel_start = TE_SEL_START (teh);
  sel_end = TE_SEL_END (teh);
  
  orig_mode = *modep;

  /* selection is `insertion point' */
  if (sel_start > sel_end)
    {
      warning_unexpected ("start = %d end = %d", sel_start, sel_end);
      sel_start = sel_end;
    }
  if (sel_start == sel_end)
    {
      TextStyle *attr;
      StScrpHandle null_scrap;
      int insertion_point;
      
      null_scrap = TE_STYLE_NULL_SCRAP (te_style);
      if (SCRAP_N_STYLES_X (null_scrap))
	{
	  ScrpSTElement *scrap_elt;
	  
	  scrap_elt = SCRAP_ST_ELT (null_scrap, 0);
	  attr = SCRAP_ELT_TO_ATTR (scrap_elt);
	}
      else
	{
	  /* get the style of the character preceeding the insertion
	     point; in this case we always return `TRUE', since a single
	     character is contiguous */
	  insertion_point = sel_start;
	  for (run_i = TE_STYLE_N_RUNS (te_style) - 1; run_i >= 0; run_i --)
	    {
	      StyleRun *run;
	      
	      run = TE_STYLE_RUN (te_style, run_i);
	      if (STYLE_RUN_START_CHAR (run) <= MAX (insertion_point - 1, 0))
		{
		  style_index = STYLE_RUN_STYLE_INDEX (run);
		  break;
		}
	    }
	  /* warning, this pulls out a pointer into an unlocked handle! */
	  attr = ST_ELT_TO_ATTR (ST_ELT (style_table, style_index));
	}
      
      if (*modep & CWC (doFont))
	TS_FONT_X (ts_out) = TS_FONT_X (attr);
      if (*modep & CWC (doFace))
	TS_FACE (ts_out) = TS_FACE (attr);
      if (*modep & CWC (doSize))
	TS_SIZE_X (ts_out) = TS_SIZE_X (attr);
      if (*modep & CWC (doColor))
	TS_COLOR (ts_out) = TS_COLOR (attr);
      return TRUE;
    }
  else
    {
      StyleRun *run = NULL;
      GUEST<int16> font = CWC(0);
      Style face = 0;
      GUEST<int16> size = CWC(0);
      RGBColor color;
      
      /* locate the starting run */
      for (run_i = TE_STYLE_N_RUNS (te_style) - 1; run_i >= 0; run_i --)
	{
	  run = TE_STYLE_RUN (te_style, run_i);
	  if (STYLE_RUN_START_CHAR (run) < sel_end)
	    {
	      style_index = STYLE_RUN_STYLE_INDEX (run);
	      break;
	    }
	}
      
      /* must have a style */
      gui_assert (style_index > -1
		  && style_index < TE_STYLE_N_STYLES (te_style));
      style = ST_ELT (style_table, style_index);
      if (*modep & CWC (doFont))
	font = ST_ELT_FONT_X (style);
      if (*modep & CWC (doFace))
	face = ST_ELT_FACE (style);
      if (*modep & CWC (doSize))
	size = ST_ELT_SIZE_X (style);
      if (*modep & CWC (doColor))
	color = ST_ELT_COLOR (style);
      
      for (; run_i >= 0
	     && sel_start < STYLE_RUN_START_CHAR (run); run_i --)
	{
	  run = TE_STYLE_RUN (te_style, run_i);
	  style_index = STYLE_RUN_STYLE_INDEX (run);
	  gui_assert (style_index < TE_STYLE_N_STYLES (te_style));
	  style = ST_ELT (style_table, style_index);
	  if (*modep & CWC (doFont)
	      && font != ST_ELT_FONT_X (style))
	    *modep &= ~CWC (doFont);
	  
	  face &= ST_ELT_FACE (style);
	  if (*modep & CWC (doFace)
	      && !face)
	    {
	      *modep &= ~CWC (doFace);
	      TS_FACE (ts_out) = face;
	    }
	  
	  if (*modep & CWC (doSize)
	      && size != ST_ELT_SIZE_X (style))
	    *modep &= ~CWC (doSize);
	  if (*modep & CWC (doColor)
	      && !!memcmp (&color, &ST_ELT_COLOR (style), sizeof color))
	    *modep &= ~CWC (doColor);
	}
      
      if (*modep & CWC (doFont))
	TS_FONT_X (ts_out) = font;
      if (*modep & CWC (doFace))
	TS_FACE (ts_out) = face;
      if (*modep & CWC (doSize))
	TS_SIZE_X (ts_out) = size;
      if (*modep & CWC (doColor))
	TS_COLOR (ts_out) = color;
      
      return orig_mode == *modep;
    }
}

P5 (PUBLIC pascal trap, void, SetStylScrap, int32, start, int32, stop,
    StScrpHandle, newstyles, BOOLEAN, redraw, TEHandle, teh)
{
  ROMlib_hook (te_notsupported);
  warning_unimplemented (NULL_STRING);
}

P3 (PUBLIC pascal trap, void, TECustomHook, int16, sel, GUEST<ProcPtr> *,
    addr, TEHandle, te)
{
  ROMlib_hook (te_notsupported);
  warning_unimplemented (NULL_STRING);
}

P3 (PUBLIC pascal trap, LONGINT, TENumStyles, int32, start, int32, stop,
    TEHandle, te)
{
  ROMlib_hook (te_notsupported);
  warning_unimplemented (NULL_STRING);
  return 0;
}
