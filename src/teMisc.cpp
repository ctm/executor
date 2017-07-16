/* Copyright 1986-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_teMisc[] =
	    "$Id: teMisc.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in TextEdit.h (DO NOT DELETE THIS LINE) */

/*
 * TODO:  for better compatability we should ignore trailing spaces when doing
 *        calculations.
 */

#include "rsys/common.h"
#include "WindowMgr.h"
#include "ControlMgr.h"
#include "EventMgr.h"
#include "TextEdit.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/tesave.h"
#include "rsys/hook.h"
#include "rsys/text.h"

using namespace Executor;
using namespace ByteSwap;

int16 nextbreak (TEHandle teh, int16 off, int16 len,
		 int16 max_width);

#if ERROR_SUPPORTED_P (ERROR_TEXT_EDIT_SLAM)
void
Executor::ROMlib_sledgehammer_te (TEHandle te)
{
  int16 n_lines;
  int16 width;
  int16 length;
  Rect *dest_rect;
  Handle hText;
  char *Text;
  int16 current_lineno;
  int16 *line_starts;
  LHHandle lh_table;
  int te_size;
  int i;

  line_starts = TE_LINE_STARTS (te);
  hText = TE_HTEXT (te);
  Text = (char *) STARH (hText);
  length = TE_LENGTH (te);
  n_lines = TE_N_LINES (te);
  dest_rect = &TE_DEST_RECT (te);
  width = RECT_WIDTH (dest_rect);
  
  if (TE_STYLIZED_P (te))
      {
	TEStyleHandle te_style;
	int lh_table_size, te_style_size;
	int16 n_runs, n_styles;
	int16 *style_count_vec;
	STHandle style_table;
	
	te_style = TE_GET_STYLE (te);
	style_table = TE_STYLE_STYLE_TABLE (te_style);
	
	lh_table = TE_STYLE_LH_TABLE (te_style);
	lh_table_size = GetHandleSize ((Handle) lh_table);
	gui_assert (lh_table_size == (n_lines + 1) * (int) sizeof (LHElement));
	
	te_style_size = GetHandleSize ((Handle) te_style);
	n_runs = TE_STYLE_N_RUNS (te_style);
	gui_assert (te_style_size
		    == ((int) sizeof (TEStyleRec)
			- (int) sizeof TE_STYLE_RUNS ((TEStyleHandle) NULL)
			+ ((n_runs + 1)
			   * (int)
			   sizeof *TE_STYLE_RUNS ((TEStyleHandle) NULL))));
	n_styles = TE_STYLE_N_STYLES (te_style);
	style_count_vec = (int16*)alloca (n_styles * sizeof *style_count_vec);
	memset (style_count_vec, 0, n_styles * sizeof *style_count_vec);
	for (i = 0; i < n_runs; i ++)
	  {
	    int16 style_index;
	    StyleRun *run;
	    
	    run = TE_STYLE_RUN (te_style, i);
	    style_index = STYLE_RUN_STYLE_INDEX (run);
	    
	    if (!i)
	      gui_assert (!STYLE_RUN_START_CHAR (run));
	    else if (i > 0)
	      {
		StyleRun *prev_run;
		int16 prev_style_index;
		
		prev_run = TE_STYLE_RUN (te_style, i - 1);
		prev_style_index = STYLE_RUN_STYLE_INDEX (prev_run);
		
		gui_assert (prev_style_index != style_index);
		/* guarantee that runs are strictly increasing */
		gui_assert (STYLE_RUN_START_CHAR (prev_run)
			    < STYLE_RUN_START_CHAR (run));
	      }
	    
	    gui_assert (style_index >= 0 && style_index < n_styles);
	    
	    style_count_vec[style_index] ++;
	  }

	{
	  StyleRun *last_run = TE_STYLE_RUN (te_style, n_runs);
	  
	  gui_assert (STYLE_RUN_START_CHAR (last_run) == length + 1
		      && (STYLE_RUN_STYLE_INDEX (last_run) == -1));
	}
	
	/* ### check that styles are not duplicated */
	for (i = 0; i < n_styles; i ++)
	  {
	    STElement *style;
	    
	    style = ST_ELT (style_table, i);
	    
	    gui_assert (style_count_vec[i] > 0
			&& style_count_vec[i] == ST_ELT_COUNT (style));
	  }
      }
  te_size = GetHandleSize ((Handle) te);
  gui_assert ((((int) sizeof (TERec)
		- (int) sizeof TE_LINE_STARTS (te))
	       + (n_lines + 4) * (int) sizeof TE_LINE_STARTS (te)[1])
	      <= te_size);
  
  for (current_lineno = 0;
       current_lineno < n_lines;
       current_lineno ++)
    {
      int16 current_line_start, next_break;
      
      current_line_start = LINE_START (line_starts,
				       current_lineno);
      next_break = nextbreak (te, current_line_start,
			      length, width);
      gui_assert (next_break == LINE_START (line_starts,
					    current_lineno + 1));
    }
}
#endif


A2 (PUBLIC, void, SetWordBreak, ProcPtr, wb, TEHandle, teh)
{
  TE_SLAM (teh);
  HxX (teh, wordBreak) = RM(wb);
  TE_SLAM (teh);
}

/*
 * TODO: make callback routines conform to IMI-395
 */

/*
 * NOTE: on the 68k, SetClikLoop is provided by glue, and the glue doesn't
 *       correspond to the implementation of SetClikLoop below, because it
 *       has to address the goofy calling conventions that are used there.
 *       So this routine is only useful on the PPC.
 */

A2(PUBLIC, void, SetClikLoop, ProcPtr, cp, TEHandle, teh)
{
  TE_SLAM (teh);
  HxX(teh, clikLoop) = RM(cp);
  TE_SLAM (teh);
}

A1(PUBLIC, INTEGER, ROMlib_wordb, char *, p)			/* INTERNAL */
{
  return (U(*p) <= 0x20);
}

#define MYWORDB(p)  (U(p) <= 0x20)

int16
nextbreak (TEHandle teh, int16 off, int16 len,
	   int16 max_width)
{
  char *sp, *ep;
  char *minsp, *maxsp;
  int16 width = 0;
  Handle hText;
  char *Text;
  INTEGER curpos;
  SignedByte hText_flags;
  int16 retval;

  /* ### warn if the width is less than that of any char? */
  hText = TE_HTEXT (teh);
  hText_flags = HGetState (hText);
  HLock (hText);
  Text = (char *) STARH (hText);
  sp = Text + off;
  ep = Text + len;
  if (off > len)
    warning_unexpected ("off > len");
  minsp = sp; 
  if (Hx(teh, crOnly) < 0)
    {
      while (sp != ep && *sp++ != '\r')
	;
      retval = sp - Text;	/* includes newline */
    }
  else
    {
      if (TE_STYLIZED_P (teh))
	{
	  curpos = off;
	  while (width <= max_width && curpos != len
		 && Text[curpos] != '\r')
	    width += ROMlib_StyleTextWidth (STARH (teh), curpos++, 1);
	  sp = Text + curpos;
	}
      else
	{
	  /* Only the size face and font need to be saved so this is
             overkill. */
	    TESAVE (teh);
	    PORT_TX_SIZE_X (thePort) = TE_TX_SIZE_X (teh);
	    PORT_TX_FACE (thePort) = TE_TX_FACE (teh);
	    PORT_TX_FONT_X (thePort) = TE_TX_FONT_X (teh);
	    while (width <= max_width && sp != ep && *sp != '\r')
		width += CharWidth(*sp++);
	    TERESTORE ();
	}
      if (width > max_width)
	{
	  maxsp = --sp;
	  if (*sp == ' ')
	    {
	      while (sp != ep && *sp == ' ')
		sp++;
	      retval = sp - Text;
	    }
	  else
	    {
	      while (sp > minsp && !MYWORDB (*--sp))
		;
	      if (sp == minsp)
		retval = maxsp - Text;   /* i.e. no wordbreaks */
	      else
		retval = ++sp - Text;    /* include the break on the line */
	    }
	}
      else
	{
	  if (sp == ep)
	    retval = len;        	/* can't go no further */
	  else
	    retval = ++sp - Text;	/* skip over the newline */
	}
    }
  HSetState (hText, hText_flags);
  if (retval == off && off < len) /* always advance at least one character */
    ++retval;
  return retval;
}

/* return the `run' index that contains the current character */
int16 Executor::te_char_to_run_index (TEStyleHandle te_style, int16 sel)
{
  StyleRun *current_run;
  int16 n_runs;
  int16 high, low, current;
  int16 retval;
  int16 current_elt;

  n_runs = TE_STYLE_N_RUNS (te_style);
  if (n_runs <= 1)
    return 0;
  
  low = 0;
  high = n_runs;
  current = (low + high) / 2;
  
  while (low < high
	 && (current_run = TE_STYLE_RUN (te_style, current),
	     current_elt = STYLE_RUN_START_CHAR (current_run)) != sel)
    {
      if (current_elt < sel)
	low = current + 1;
      else
	high = current - 1;
      current = (high + low) / 2;
    }
  if (STYLE_RUN_START_CHAR (TE_STYLE_RUN (te_style, current)) > sel)
    retval = current - 1;
  else
    retval = current;
  
  return retval;
}

/* return the line number that contains character index `sel' */
int16 Executor::te_char_to_lineno (TEPtr te, int16 sel)
{
  int16 n_lines;
  int16 *line_starts;
  int16 high, low, current;
  int16 retval;
  int16 current_elt;
  
  n_lines = TEP_N_LINES (te);
  if (n_lines <= 1)
    return 0;
  line_starts = TEP_LINE_STARTS (te);
  
  high = n_lines;
  low = 0;
  current = (high + low) / 2;

  while (low < high
	 && (current_elt = BigEndianValue (line_starts[current])) != sel)
    {
      if (current_elt < sel)
	low = current + 1;
      else
	high = current - 1;
      current = (high + low) / 2;
    }
  if (BigEndianValue (line_starts[current]) > sel
      || current == n_lines)
    retval = current - 1;
  else
    retval = current;
  
  return retval;
}

int16
calclhtab (TEHandle teh)
{
  int16 n_lines;
  StyleRun *current_run;
  int16 *linestarts;
  int16 first_changed;
  int16 orig_height = -1, orig_ascent = -1;
  LHHandle lh_table;
  LHPtr lh;
  TEStyleHandle sth;
  int clear_lh_p;
  
  n_lines = TE_N_LINES (teh);
  
  sth = TE_GET_STYLE (teh);
  lh_table = TE_STYLE_LH_TABLE (sth);
  SetHandleSize ((Handle) lh_table,
		 sizeof (LHElement) * (n_lines + 1));
  lh = STARH (lh_table);
  first_changed = -1;
  clear_lh_p = TRUE;
  for (current_run = TE_STYLE_RUNS (sth), linestarts = TE_LINE_STARTS (teh);
       linestarts <= (TE_LINE_STARTS (teh) + n_lines);)
    {
      STPtr current_style;
      
      current_style = ST_ELT (TE_STYLE_STYLE_TABLE (sth),
			      STYLE_RUN_STYLE_INDEX (current_run));
      if (clear_lh_p)
	{
	  clear_lh_p = FALSE;
	  if (first_changed == -1)
	    {
	      orig_height = LH_HEIGHT (lh);
	      orig_ascent = LH_ASCENT (lh);
	    }
	  LH_HEIGHT_X (lh) = CWC (0);
	  LH_ASCENT_X (lh) = CWC (0);
	}
      
      if (ST_ELT_HEIGHT (current_style) > LH_HEIGHT (lh))
	LH_HEIGHT_X (lh) = ST_ELT_HEIGHT_X (current_style);
      if (ST_ELT_ASCENT (current_style) > LH_ASCENT (lh))
	LH_ASCENT_X (lh) = ST_ELT_ASCENT_X (current_style);

      if (BigEndianValue (*linestarts) == TE_LENGTH (teh))
	break;
      
      if (STYLE_RUN_START_CHAR (current_run + 1) > BigEndianValue (linestarts[1]))
	{
	  if (first_changed == -1
	      && (   LH_HEIGHT (lh) != orig_height
		  || LH_ASCENT (lh) != orig_ascent))
	    first_changed = lh - STARH (lh_table);
	  linestarts ++;
	  lh ++;
	  clear_lh_p = TRUE;
	}
      else if (STYLE_RUN_START_CHAR (current_run + 1) < BigEndianValue (linestarts[1]))
	current_run ++;
      else
	{
	  if (first_changed == -1
	      && (   LH_HEIGHT (lh) != orig_height
		  || LH_ASCENT (lh) != orig_ascent))
	    first_changed = lh - STARH (lh_table);
	  linestarts ++;
	  lh ++;
	  clear_lh_p = TRUE;
	  current_run ++;
	}
    }
  
  return first_changed;
}

/* guarantee that handle `te' has allocated enough space for at least
   `n_lines' worth of `line_starts' */
void
te_guarantee_line_starts_allocation (TEHandle te, int n_lines)
{
  int te_size;
  int min_te_size;
  
  te_size = GetHandleSize ((Handle) te);
  
  min_te_size = ((sizeof (TERec)
		  - sizeof TE_LINE_STARTS (te))
		 + (n_lines + 4) * sizeof *TE_LINE_STARTS (te));
  if (te_size < min_te_size)
    SetHandleSize ((Handle) te, min_te_size);
}

void
te_set_line_starts_allocation (TEHandle te, int n_lines)
{
  SetHandleSize ((Handle) te,
		 ((sizeof (TERec)
		   - sizeof TE_LINE_STARTS (te))
		  + (n_lines + 4) * sizeof *TE_LINE_STARTS (te)));
}

/* recalculate the various text edit tables associated with `te' after
   `nadded' characters are to be added (or deleted) at `sel' */
void
Executor::ROMlib_caltext (TEHandle te,
		int16 sel, int16 n_added,
		int16 *first_changed_out, int16 *last_changed_out)
{
  int16 n_lines;
  int16 *line_starts;
  int16 first_lineno;
  int16 t;
  int16 width;
  int16 length;
  int16 first_changed = -1, last_changed = -1;
  
  width = RECT_WIDTH (&TE_DEST_RECT (te));
  
  if (width <= 0)
    {
      warning_unexpected ("width <= 0");
      return;
    }

  n_lines = TE_N_LINES (te);
  line_starts  = TE_LINE_STARTS (te);
  length = TE_LENGTH (te);
  
  first_lineno = TE_CHAR_TO_LINENO (te, sel);
  
  /* advance each line after the current line forward by `n_added' in
     the optimistic case, where the insertion/deletion changes no
     lines breaks, this is all we will do */
  for (t = first_lineno + 1; t <= n_lines; t ++)
    {
      int line_start = LINE_START (line_starts, t);
      int new_line_start = line_start + n_added;
      
      LINE_START_X (line_starts, t) = BigEndianValue (new_line_start);
    }
  
  /* starting from the first line, recompute all the end lines.  we
     can stop when we have pasted `sel + n_added', and the end of
     lines are unchanging

     note, as we loop, we may create more lines than there were
     previuosly.  make sure we keep `te' allocated to an appropriate
     length */
  {
    /* break point for the current line, also the start point for
       the next line */
    int16 current_line_break;
    int16 current_lineno;

    /* start with the previous line, since deltion on the current line
       can cause the previous line's break to change */
    for (current_lineno = MAX (first_lineno - 1, 0);; current_lineno ++)
      {
	int16 current_line_start;
	int16 orig_current_line_break;
	
	/* compute the new break for the current line */
	current_line_start = LINE_START (line_starts, current_lineno);
	current_line_break = nextbreak (te, current_line_start,
					length, width);
	
	te_guarantee_line_starts_allocation (te, current_lineno + 1);
	/* pull `line_starts' back out, since the `te' handle may have
	   been relocated */
	line_starts  = TE_LINE_STARTS (te);
	orig_current_line_break = LINE_START (line_starts, current_lineno + 1);
	LINE_START_X (line_starts, current_lineno + 1) = BigEndianValue (current_line_break);
	
	if (first_changed == -1
	    && orig_current_line_break != current_line_break)
	  first_changed = LINE_START (line_starts, current_lineno);
	
	if (current_line_break == length)
	  {
	    if (length == 0)
	      n_lines = 0;
	    else
	      n_lines = current_lineno + 1;
	    
	    TE_N_LINES_X (te) = BigEndianValue (n_lines);
	    te_set_line_starts_allocation (te, n_lines);
	    line_starts  = TE_LINE_STARTS (te);
	    LINE_START_X (line_starts, n_lines + 1) = CWC (0);
	    
	    last_changed = length;
	    break;
	  }
	else if (current_line_break == orig_current_line_break
		 && current_line_break > MAX (sel, sel + n_added))
	  {
	    last_changed = LINE_START (line_starts + 1, current_lineno) - 1;
	    break;
	  }
      }
  }
  
  {
    int lh_first_changed = -1;
    
    if (TE_STYLIZED_P (te)
	&& TE_TX_SIZE_X (te) == CWC (-1))
      lh_first_changed = calclhtab (te);
    
    if (lh_first_changed != -1)
      {
	if (first_changed == -1)
	  first_changed = lh_first_changed;
	else
	  first_changed = MIN (lh_first_changed, first_changed);
	last_changed = length;
      }
  }

  if (first_changed == -1)
    first_changed = sel;
  
  if (last_changed == -1
      || last_changed < first_changed)
    last_changed = first_changed + n_added;
  
  if (first_changed_out)
    *first_changed_out = first_changed;
  if (last_changed_out)
    *last_changed_out = last_changed;
}

P1 (PUBLIC pascal trap, void, TECalText, TEHandle, te)
{
  /* don't do this check because people call this caltext when the TE
     has been frobbed and is in a wacky state */
  /* TE_SLAM (te); */
  TE_LENGTH_X (te) = BigEndianValue (GetHandleSize (TE_HTEXT (te)));
  ROMlib_caltext (te, 0, 32767, NULL, NULL);
  TE_SLAM (te);
}

P3 (PUBLIC pascal trap, int16, TEFeatureFlag,
    int16, feature, int16, action, TEHandle, te)
{
  switch (feature)
    {
    case teFAutoScroll:
      if (action == teBitTest)
	return ((TE_FLAGS (te) & TEAUTOVIEWBIT)
		? teBitSet
		: teBitClear);
      TEAutoView (action == teBitSet, te);
      break;
      
    case teFOutlineHilite:
      /* #### implement outline hilite */
      
    case teFTextBuffering:
    case teFInlineInput:
    case teFUseTextServices:
      warning_unimplemented ("unable to handle te feature flag `%d'",
			     feature);
      return teBitClear;
    }
  
  return action;
}

int16
Executor::ROMlib_call_TEDoText (TEPtr tp, int16 first, int16 last, int16 what)
{
  register int16 myd0;

  if ((a0trap INTEGERRET (*) (void)) MR (TEDoText) == (void *) P_ROMlib_dotext)
    myd0 = C_ROMlib_dotext (tp, first, last, what);
  else
    {
      ROMlib_hook (te_dotextnumber);
      {
	int32 saved2, saved3, saved4, saved7, savea2, savea3;

	saved2 = EM_D2;
	saved3 = EM_D3;
	saved4 = EM_D4;
	saved7 = EM_D7;
	savea2 = EM_A2;
	savea3 = EM_A3;
	EM_A3 = (LONGINT) (long) US_TO_SYN68K(tp);
	EM_D3 = (LONGINT) first;
	EM_D4 = (LONGINT) last;
	EM_D7 = (LONGINT) what;
	EM_A0 = (LONGINT) (long) BigEndianValue ((long) TEDoText);
	CALL_EMULATOR (EM_A0);
	myd0 = EM_D0;
	EM_D2 = saved2;
	EM_D3 = saved3;
	EM_D4 = saved4;
	EM_D7 = saved7;
	EM_A2 = savea2;
	EM_A3 = savea3;
      }
    }
  return myd0;
}
