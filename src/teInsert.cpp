/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "WindowMgr.h"
#include "ControlMgr.h"
#include "EventMgr.h"
#include "TextEdit.h"
#include "OSUtil.h"
#include "ToolboxEvent.h"
#include "ToolboxUtil.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/mman.h"
#include "rsys/tesave.h"
#include "rsys/hook.h"
#include "rsys/region.h"
#include "rsys/text.h"

using namespace Executor;

int16 Executor::ROMlib_StyleTextWidth(TEPtr tep,
                                      int16 start, int16 count)
{
    Handle hText;
    SignedByte hText_flags;
    Ptr Text;
    TEStyleHandle te_style;
    StyleRun *runs;
    SignedByte te_style_flags;
    STHandle style_table;
    SignedByte style_table_flags;
    STElement *styles;

    int16 current_run_index;
    GUEST<int16> save_size, save_font;
    Style save_face;
    int16 length;
    int16 retval;

    length = TEP_LENGTH(tep);
    if(start > length)
        return 0;

    hText = TEP_HTEXT(tep);
    hText_flags = HGetState(hText);
    HLock(hText);
    Text = STARH(hText);

    if(!TEP_STYLIZED_P(tep))
    {
        retval = TextWidth(Text, start, count);
        goto cleanup;
    }

    if(start + count > length)
        count = length - start;
    te_style = TEP_GET_STYLE(tep);
    te_style_flags = HGetState((Handle)te_style);
    HLock((Handle)te_style);
    runs = TE_STYLE_RUNS(te_style);

    style_table = TE_STYLE_STYLE_TABLE(te_style);
    style_table_flags = HGetState((Handle)style_table);
    styles = STARH(style_table);

    save_size = PORT_TX_SIZE_X(thePort);
    save_face = PORT_TX_FACE(thePort);
    save_font = PORT_TX_FONT_X(thePort);

    current_run_index = te_char_to_run_index(te_style, start);
    retval = 0;
    for(; count > 0; current_run_index++)
    {
        StyleRun *current_run, *next_run;
        STElement *style;
        int16 next_run_start, run_len;

        current_run = &runs[current_run_index];
        next_run = &runs[current_run_index + 1];

        next_run_start = RUN_START_CHAR(next_run);
        run_len = next_run_start - start;
        if(run_len > count)
            run_len = count;

        style = &styles[RUN_STYLE_INDEX(current_run)];

        PORT_TX_SIZE_X(thePort) = ST_ELT_SIZE_X(style);
        PORT_TX_FACE(thePort) = ST_ELT_FACE(style);
        PORT_TX_FONT_X(thePort) = ST_ELT_FONT_X(style);

        retval += TextWidth(Text, start, run_len);
        count -= run_len;
        start = next_run_start;
    }

    PORT_TX_SIZE_X(thePort) = save_size;
    PORT_TX_FACE(thePort) = save_face;
    PORT_TX_FONT_X(thePort) = save_font;

    HSetState((Handle)style_table, style_table_flags);
    HSetState((Handle)te_style, te_style_flags);
cleanup:
    HSetState(hText, hText_flags);
    return retval;
}

void Executor::te_char_to_point(const TEPtr tep, int16 sel, Point *p)
{
    SignedByte hText_flags;
    Handle hText;
    Ptr Text;
    GUEST<int16> *line_starts;
    int16 line_start, line_end;
    int16 just;
    /* offset from the left edge of the dest rect to the beginning of
     text for this line */
    int16 left_offset;
    int16 lineno, lineno_i;
    int16 top, left;
    Rect *dest_rect;
    int on_break_p;

    hText = TEP_HTEXT(tep);
    hText_flags = HGetState(hText);
    HLock(hText);
    Text = STARH(hText);
    dest_rect = &TEP_DEST_RECT(tep);

    lineno = TEP_CHAR_TO_LINENO(tep, sel);

    line_starts = TEP_LINE_STARTS(tep);
    line_start = LINE_START(line_starts, lineno);
    line_end = LINE_START(line_starts, lineno + 1);

    on_break_p = (sel && sel == TEP_LENGTH(tep) && Text[sel - 1] == '\r');

    just = TEP_JUST(tep);
    if(just == teFlushDefault)
        just = teFlushLeft;
    else if(just != teCenter && just != teFlushRight && just != teFlushLeft)
    {
        warning_unexpected("unknown justification `%d'", just);
        just = teFlushLeft;
    }

    if(just == teCenter || just == teFlushRight)
    {
        int16 line_len;

        line_len = line_end - line_start;

        if(on_break_p)
            left_offset = RECT_WIDTH(dest_rect) - 1;
        else
            left_offset = (RECT_WIDTH(dest_rect)
                           - TEP_TEXT_WIDTH(tep, Text, line_start, line_len)
                           /* ### the old code did this, i'm not quite sure
			  why it is necessary */
                           - 1);
        if(just == teCenter)
            left_offset /= 2;
    }
    else if(just == teFlushLeft)
        left_offset = 0;
    else
        gui_fatal("unknown justification");

    if(on_break_p)
        left = (CW(dest_rect->left) + left_offset);
    else
        left = (CW(dest_rect->left)
                + left_offset
                + TEP_TEXT_WIDTH(tep, Text, line_start, sel - line_start));

    for(top = CW(dest_rect->top), lineno_i = 0; lineno_i < lineno; lineno_i++)
        /* ### hoist alot of the internal constants out of this loop */
        top += TEP_HEIGHT_FOR_LINE(tep, lineno_i);
    if(on_break_p)
        top += TEP_HEIGHT_FOR_LINE(tep, lineno_i);

    HSetState(hText, hText_flags);

    p->v = top;
    p->h = left;
}

static void
togglehilite(TEHandle te)
{
    TE_DO_TEXT(te, TE_SEL_START(te), TE_SEL_END(te), teHilite);
}

static void
togglecaret(TEHandle teh, int16 sel, bool paint_p)
{
    Point p;
    int16 pm;

    pm = PORT_PEN_MODE(thePort);
    TE_CHAR_TO_POINT(teh, sel, &p);
    MoveTo(p.h, p.v);
    PORT_PEN_MODE_X(thePort) = CWC(patXor);
    if(TE_STYLIZED_P(teh))
    {
        TEStyleHandle te_style;
        int lineno;

        te_style = TE_GET_STYLE(teh);
        lineno = TE_CHAR_TO_LINENO(teh, sel);
        SetRect(&TE_SEL_RECT(teh), p.h, p.v,
                p.h + 1, (p.v
                          + LH_ASCENT(&(STARH(TE_STYLE_LH_TABLE(te_style)))[lineno])
                          + 1));
    }
    else
        SetRect(&TE_SEL_RECT(teh), p.h, p.v, p.h + 1,
                p.v + TE_LINE_HEIGHT(teh));
    if(paint_p)
        PaintRect(&TE_SEL_RECT(teh));
    PenMode(pm);
}

PUBLIC void
Executor::ROMlib_recompute_caret(TEHandle te)
{
    int16 state;

    state = TE_CARET_STATE(te);
    if(state == caret_vis || state == caret_invis)
        togglecaret(te, TE_SEL_START(te), false);
}

void Executor::ROMlib_togglelite(TEHandle te)
{
    int16 state;

    state = TE_CARET_STATE(te);
    if(state == caret_vis || state == caret_invis)
        togglecaret(te, TE_SEL_START(te), true);
    else
        togglehilite(te);
}

void Executor::ROMlib_tesave(tesave *t, TEHandle teh)
{
    TEPtr tp;

    tp = STARH(teh);
    t->_tport = thePortX;
    thePortX = tp->inPort;

    t->_tpvis = PORT_PEN_VIS_X(thePort);
    t->_tfont = PORT_TX_FONT_X(thePort);
    t->_tmode = PORT_TX_MODE_X(thePort);
    t->_tsize = PORT_TX_SIZE_X(thePort);
    t->_tstyle = PORT_TX_FACE_X(thePort);

    t->fg_color = PORT_FG_COLOR_X(thePort);
    t->bk_color = PORT_BK_COLOR_X(thePort);

    if(CGrafPort_p(thePort))
    {
        t->rgb_fg_color = CPORT_RGB_FG_COLOR(thePort);
        t->rgb_bk_color = CPORT_RGB_BK_COLOR(thePort);
    }

    GetPenState(&t->_tpstate);

    PORT_PEN_SIZE(thePort).h = PORT_PEN_SIZE(thePort).v = CWC(1);
    PORT_PEN_MODE_X(thePort) = CWC(patCopy);
    PORT_TX_FONT_X(thePort) = tp->txFont;
    PORT_TX_MODE_X(thePort) = tp->txMode;
    PORT_TX_SIZE_X(thePort) = tp->txSize;
    PORT_TX_FACE_X(thePort) = tp->txFace;

    PenPat(black);

    t->_tsaveclip = PORT_CLIP_REGION_X(thePort);
    PORT_CLIP_REGION_X(thePort) = RM(NewRgn());
    HxX(PORT_CLIP_REGION(thePort), rgnBBox) = HxX(teh, viewRect);
    SectRgn(PORT_CLIP_REGION(thePort), MR(t->_tsaveclip),
            PORT_CLIP_REGION(thePort));
}

void Executor::ROMlib_terestore(tesave *t)
{
    SetPenState(&t->_tpstate);

    DisposeRgn(PORT_CLIP_REGION(thePort));
    PORT_CLIP_REGION_X(thePort) = t->_tsaveclip;

    PORT_TX_FACE_X(thePort) = t->_tstyle;
    PORT_TX_SIZE_X(thePort) = t->_tsize;
    PORT_TX_MODE_X(thePort) = t->_tmode;
    PORT_TX_FONT_X(thePort) = t->_tfont;
    PORT_PEN_VIS_X(thePort) = t->_tpvis;

    PORT_FG_COLOR_X(thePort) = t->fg_color;
    PORT_BK_COLOR_X(thePort) = t->bk_color;

    if(CGrafPort_p(thePort))
    {
        CPORT_RGB_FG_COLOR(thePort) = t->rgb_fg_color;
        CPORT_RGB_BK_COLOR(thePort) = t->rgb_bk_color;
    }

    thePortX = t->_tport;
}

P1(PUBLIC pascal trap, void, TEIdle, TEHandle, teh)
{
    INTEGER sel, state;
    LONGINT ticks;

    TESAVE(teh);
    TE_SLAM(teh);
    if((ticks = TickCount()) > Hx(teh, caretTime) + CL(CaretTime)
       && Hx(teh, active)
       && (sel = Hx(teh, selStart)) == Hx(teh, selEnd)
       && (state = Hx(teh, caretState)))
    {
        togglecaret(teh, sel, true);
        HxX(teh, caretState) = CW(state ^ 0xFF00);
        HxX(teh, caretTime) = CL(ticks);
    }
    TE_SLAM(teh);
    TERESTORE();
}

void style_update_port(STElement *style)
{
    PORT_TX_SIZE_X(thePort) = ST_ELT_SIZE_X(style);
    PORT_TX_FONT_X(thePort) = ST_ELT_FONT_X(style);
    PORT_TX_FACE(thePort) = ST_ELT_FACE(style);
    RGBForeColor(&ST_ELT_COLOR(style));
}

/* ### the old code did something whacky related to italics */

void te_draw(TEPtr tep,
             int16 start, int16 end)
{
    Handle hText;
    Ptr Text;
    SignedByte hText_flags;

    /* these are only used if the TE record is stylized */
    TEStyleHandle te_style = NULL;
    SignedByte te_style_flags = -1;
    StyleRun *runs, *current_run;
    int16 n_runs;

    STHandle style_table = NULL;
    SignedByte style_table_flags = -1;
    STElement *styles;
    int16 n_styles;

    /* all drawing is done per-line; before any drawing we erase the
     entire target rectangle */
    Point start_pt, end_pt;
    int16 start_lineno, end_lineno, current_lineno;
    GUEST<int16> *line_starts;
    int16 first_visible_lineno, last_visible_lineno;
    int16 start_line_start;
    Rect *dest_rect, r;
    int16 dest_rect_left, dest_rect_right, dest_rect_bottom, dest_rect_top;
    int16 current_line_top, top, bottom;

    Rect *view_rect;
    int16 view_rect_bottom;

    int16 current_run_start, current_run_end;
    int16 current_line_start, current_line_end;
    int16 current_posn;
    int16 start_run_index;
    int16 n_lines;

    start_lineno = TEP_CHAR_TO_LINENO(tep, start);
    end_lineno = TEP_CHAR_TO_LINENO(tep, end);

    n_lines = TEP_N_LINES(tep);

    dest_rect = &TEP_DEST_RECT(tep);
    dest_rect_left = CW(dest_rect->left);
    dest_rect_right = CW(dest_rect->right);
    dest_rect_top = CW(dest_rect->top);
    dest_rect_bottom = CW(dest_rect->bottom);

    view_rect = &TEP_VIEW_RECT(tep);
    view_rect_bottom = CW(view_rect->bottom);

    {
        int16 height;
        Rect *view_rect, *clip_rect, *vis_rect;
        int16 clip_rect_top, clip_rect_bottom;

        view_rect = &TEP_VIEW_RECT(tep);
        vis_rect = &RGN_BBOX(PORT_VIS_REGION(thePort));
        clip_rect = (Rect *)alloca(sizeof *clip_rect);

#if 0
#warning this code appears to cause trouble with TTSs logbook

/* the problem is that they have a picture open and expect the text
 calls to be logged into the picture even though they can't make it to the
							    screen */
    SectRect (view_rect, vis_rect, clip_rect);
#else
#warning this section of code should be cleaned up
        SectRect(view_rect, view_rect, clip_rect);
#endif

        clip_rect_top = CW(clip_rect->top);
        clip_rect_bottom = CW(clip_rect->bottom);

        /* clip the lines to draw by the visible lines */
        for(current_lineno = 0, height = dest_rect_top;; current_lineno++)
        {
            height += TEP_HEIGHT_FOR_LINE(tep, current_lineno);
            if(height > clip_rect_top)
                break;
        }
        first_visible_lineno = current_lineno;

        for(; current_lineno < n_lines - 1; current_lineno++)
        {
            if(height > clip_rect_bottom)
                break;
            height += TEP_HEIGHT_FOR_LINE(tep, current_lineno);
        }
        last_visible_lineno = current_lineno;
    }

    if(end_lineno < first_visible_lineno
       || start_lineno > last_visible_lineno)
        return;

    if(start_lineno < first_visible_lineno)
        start_lineno = first_visible_lineno;
    if(end_lineno > last_visible_lineno)
        end_lineno = last_visible_lineno;

    line_starts = TEP_LINE_STARTS(tep);
    start_line_start = LINE_START(line_starts, start_lineno);

    TEP_CHAR_TO_POINT(tep, start_line_start, &start_pt);
    /* we only care about the vertical component */
    TEP_CHAR_TO_POINT(tep, end, &end_pt);

    hText = TEP_HTEXT(tep);
    hText_flags = HGetState(hText);
    HLock(hText);
    Text = STARH(hText);

    top = start_pt.v;
    bottom = (end == TEP_LENGTH(tep)
                  ? view_rect_bottom
                  : end_pt.v + TEP_HEIGHT_FOR_LINE(tep, end_lineno));
    SetRect(&r, dest_rect_left, top, dest_rect_right, bottom);
    EraseRect(&r);

    if(TEP_STYLIZED_P(tep))
    {
        te_style = TEP_GET_STYLE(tep);
        te_style_flags = HGetState((Handle)te_style);
        HLock((Handle)te_style);

        n_runs = TE_STYLE_N_RUNS(te_style);
        runs = TE_STYLE_RUNS(te_style);

        n_styles = TE_STYLE_N_STYLES(te_style);
        style_table = TE_STYLE_STYLE_TABLE(te_style);
        style_table_flags = HGetState((Handle)style_table);
        HLock((Handle)style_table);
        styles = STARH(style_table);

        start_run_index
            = te_char_to_run_index(te_style,
                                   LINE_START(line_starts, start_lineno));
    }
    else
    {
        /* allocate bogo runs/styles */

        n_runs = 1;
        runs = (StyleRun *)alloca(sizeof *runs * 2);
        RUN_START_CHAR_X(&runs[0]) = CWC(0);
        RUN_STYLE_INDEX_X(&runs[0]) = CWC(0);
        RUN_START_CHAR_X(&runs[1]) = CW(TEP_LENGTH(tep) + 1);
        RUN_STYLE_INDEX_X(&runs[1]) = CWC(-1);

        n_styles = 1;
        styles = (STElement *)alloca(sizeof *styles);

        ST_ELT_FACE(styles) = TEP_TX_FACE(tep);

        ST_ELT_HEIGHT_X(styles) = TEP_LINE_HEIGHT_X(tep);
        ST_ELT_ASCENT_X(styles) = TEP_FONT_ASCENT_X(tep);
        ST_ELT_FONT_X(styles) = TEP_TX_FONT_X(tep);
        ST_ELT_SIZE_X(styles) = TEP_TX_SIZE_X(tep);
        GetForeColor(&ST_ELT_COLOR(styles));

        /* and for completeness */
        ST_ELT_COUNT_X(styles) = CWC(1);

        start_run_index = 0;
    }

    current_line_top = start_pt.v;

    current_lineno = start_lineno;
    current_posn = current_line_start = start_line_start;
    current_line_end = LINE_START(line_starts, current_lineno + 1);

    current_run = &runs[start_run_index];
    current_run_start = RUN_START_CHAR(current_run);
    current_run_end = RUN_START_CHAR(current_run + 1);

    style_update_port(&styles[RUN_STYLE_INDEX(current_run)]);

    MoveTo(start_pt.h, (current_line_top
                        + TEP_ASCENT_FOR_LINE(tep, current_lineno)));

    for(; current_lineno <= end_lineno;)
    {
        if(current_run_end < current_line_end)
        {
#if 0
	  Point orig_pn = PORT_PEN_LOC (thePort);
#endif

            DrawText(Text, current_posn, current_run_end - current_posn);
#if 0
	  gui_assert (CW (orig_pn.h)
		      + TextWidth (Text, current_posn,
				   current_run_end - current_posn)
		      == CW (PORT_PEN_LOC (thePort).h));
#endif
            current_run++;
            style_update_port(&styles[RUN_STYLE_INDEX(current_run)]);
            current_run_start = current_posn = current_run_end;
            current_run_end = RUN_START_CHAR(current_run + 1);
        }
        else if(current_run_end > current_line_end)
        {
            DrawText(Text, current_posn, current_line_end - current_posn);
            if(current_lineno == end_lineno)
                break;
            current_line_top += TEP_HEIGHT_FOR_LINE(tep, current_lineno);
            current_lineno++;
            current_posn = current_line_start = current_line_end;
            current_line_end = LINE_START(line_starts, current_lineno + 1);

            TEP_CHAR_TO_POINT(tep, current_line_start, &start_pt);
            MoveTo(start_pt.h, (current_line_top
                                + TEP_ASCENT_FOR_LINE(tep, current_lineno)));
        }
        else
        {
            gui_assert(current_run_end == current_line_end);

            DrawText(Text, current_posn, current_line_end - current_posn);

            if(current_lineno == end_lineno)
                break;

            /* advance the run */
            current_run++;
            style_update_port(&styles[RUN_STYLE_INDEX(current_run)]);
            current_run_start = current_posn = current_run_end;
            current_run_end = RUN_START_CHAR(current_run + 1);

            /* advance the line */
            current_line_top += TEP_HEIGHT_FOR_LINE(tep, current_lineno);
            current_lineno++;
            current_line_start = current_line_end;
            current_line_end = LINE_START(line_starts, current_lineno + 1);

            TEP_CHAR_TO_POINT(tep, current_line_start, &start_pt);
            MoveTo(start_pt.h, (current_line_top
                                + TEP_ASCENT_FOR_LINE(tep, current_lineno)));
        }
    }

    if(TEP_STYLIZED_P(tep))
    {
        HSetState((Handle)style_table, style_table_flags);
        HSetState((Handle)te_style, te_style_flags);
    }
    HSetState(hText, hText_flags);
}

void te_hilite(TEPtr tep,
               int16 start, int16 end)
{
    Handle hText;
    SignedByte hText_flags;
    Ptr Text;

    Point start_pt, end_pt;
    int16 start_lineno, end_lineno, lineno_i;
    GUEST<int16> *line_starts;
    int16 top, left, bottom, right;
    Rect *dest_rect, r;
    int16 dest_rect_left, dest_rect_right;
    int16 length;

    hText = TEP_HTEXT(tep);
    hText_flags = HGetState(hText);
    HLock(hText);
    Text = STARH(hText);

    TEP_CHAR_TO_POINT(tep, start, &start_pt);
    TEP_CHAR_TO_POINT(tep, end, &end_pt);

    start_lineno = TEP_CHAR_TO_LINENO(tep, start);
    end_lineno = TEP_CHAR_TO_LINENO(tep, end);

    line_starts = TEP_LINE_STARTS(tep);
    dest_rect = &TEP_DEST_RECT(tep);
    dest_rect_left = CW(dest_rect->left);
    dest_rect_right = CW(dest_rect->right);
    length = TEP_LENGTH(tep);

    /* highlight the first line */
    left = (start == LINE_START(line_starts, start_lineno)
                ? dest_rect_left
                : start_pt.h);
    right = ((start_lineno != end_lineno
              || end == length)
                 ? dest_rect_right
                 : end_pt.h);
    top = start_pt.v;
    bottom = top + TEP_HEIGHT_FOR_LINE(tep, start_lineno);
    SetRect(&r, left, top, right, bottom);
    CLEAR_HILITE_BIT();
    InvertRect(&r);

    if(start_lineno == end_lineno)
        goto DONE;

    top = bottom;
    for(lineno_i = start_lineno + 1;
        (end == length && Text[length - 1] == '\r'
             ? lineno_i <= end_lineno
             : lineno_i < end_lineno);
        lineno_i++)
        bottom += TEP_HEIGHT_FOR_LINE(tep, lineno_i);
    if(bottom != top)
    {
        SetRect(&r, dest_rect_left, top, dest_rect_right, bottom);
        CLEAR_HILITE_BIT();
        InvertRect(&r);
    }

    if(end != LINE_START(line_starts, end_lineno))
    {
        top = end_pt.v;
        right = (end == length
                     ? dest_rect_right
                     : end_pt.h);
        bottom = top + TEP_HEIGHT_FOR_LINE(tep, end_lineno);
        SetRect(&r, dest_rect_left, top, right, bottom);
        CLEAR_HILITE_BIT();
        InvertRect(&r);
    }
DONE:
    HSetState(hText, hText_flags);
}

int16 te_find(TEPtr tep, int16 start, int16 end)
{
    Handle hText;
    SignedByte hText_flags;
    Ptr Text;
    int16 length;

    int16 just;
    int16 v, h;
    int16 dest_rect_top, dest_rect_left;
    Rect *dest_rect;

    int16 line_start, line_end, end_limit, lineno;
    GUEST<int16> *line_starts;
    int16 current_posn;
    int height, width, char_width = 0;
    int16 n_lines;
    int16 left_offset;
    int16 retval;

    hText = TEP_HTEXT(tep);
    hText_flags = HGetState(hText);
    HLock(hText);
    Text = STARH(hText);

    dest_rect = &TEP_DEST_RECT(tep);
    dest_rect_top = CW(dest_rect->top);
    dest_rect_left = CW(dest_rect->left);

    length = TEP_LENGTH(tep);
    n_lines = TEP_N_LINES(tep);
    line_starts = TEP_LINE_STARTS(tep);

    v = CW(TEP_SEL_POINT(tep).v);
    h = CW(TEP_SEL_POINT(tep).h) - dest_rect_left;

    if(!Text)
    {
        retval = -1;
        goto DONE;
    }

    if(v < dest_rect_top)
    {
        retval = 0;
        goto DONE;
    }

    /* compute the line the click is on */
    for(lineno = 0, height = dest_rect_top; lineno < n_lines; lineno++)
    {
        height += TEP_HEIGHT_FOR_LINE(tep, lineno);
        if(v < height)
            break;
    }

    line_start = LINE_START(line_starts, lineno);
    line_end = LINE_START(line_starts, lineno + 1);

    just = TEP_JUST(tep);
    if(just == teFlushDefault)
        just = teFlushLeft;
    else if(just != teCenter && just != teFlushRight && just != teFlushLeft)
    {
        warning_unexpected("unknown justification `%d'", just);
        just = teFlushLeft;
    }

    /* compute the left offset for this line */
    if(just == teCenter || just == teFlushRight)
    {
        int16 line_len;

        line_len = line_end - line_start;

        left_offset = (RECT_WIDTH(dest_rect)
                       - TEP_TEXT_WIDTH(tep, Text, line_start, line_len)
                       /* ### the old code did this, i'm not quite sure
			why it is necessary */
                       - 1);
        if(just == teCenter)
            left_offset /= 2;
    }
    else /* if (just == teFlushLeft) */
        left_offset = 0;

    if(h < left_offset)
    {
        retval = line_start;
        goto DONE;
    }

    end_limit = ((line_end && line_end == length
                  && Text[line_end - 1] != '\r')
                     ? line_end
                     : line_end - 1);
    for(width = left_offset, current_posn = line_start;
        width < h && current_posn < end_limit; current_posn++)
    {
        if(TEP_STYLIZED_P(tep))
            char_width = ROMlib_StyleTextWidth(tep, current_posn, 1);
        else
            char_width = CharWidth(Text[current_posn]);

        width += char_width;
    }
    if(current_posn > line_start
       && h < width - char_width / 2)
        current_posn--;
    retval = current_posn;

DONE:
    HSetState(hText, hText_flags);
    return retval;
}

/* custom text hook; this is the defualt text hook stored in
   `TEDoText'.
   
   for calling conventions, see IM Text, 2-63 */

A4(PUBLIC, int16, C_ROMlib_dotext, TEPtr, tep, /* INTERNAL */
   int16, start, int16, end, int16, what)
{
    if(what == teHilite)
        te_hilite(tep, start, end);
    else if(what == teDraw)
        te_draw(tep, start, end);
    else if(what == teFind)
        return te_find(tep, start, end);
    return 0;
}

PUBLIC INTEGERRET ROMlib_dotext(void)
{
    INTEGERRET retval;
    TEPtr tep;
    INTEGER first, last, what;

    tep = (TEPtr)(long)SYN68K_TO_US(EM_A3);
    first = EM_D3;
    last = EM_D4;
    what = EM_D7;

    retval = C_ROMlib_dotext(tep, first, last, what);

    EM_A0 = US_TO_SYN68K(thePort);
    EM_D0 = retval;
    return retval;
}

/* return true if click at location `cl' was a double click; otherwise
   save away this click information (location, time) */
static bool
double_click_p(TEHandle te, int16 cl)
{
    int32 ticks;

    ticks = TickCount();
    if(cl == TE_CLICK_LOC(te)
       && ticks <= TE_CLICK_TIME(te) + CL(DoubleTime))
        return true;

    TE_CLICK_LOC_X(te) = CW(cl);
    TE_CLICK_TIME_X(te) = CL(ticks);
    return false;
}

typedef BOOLEAN (*cliklooptype)(void);

namespace Executor
{
static inline BOOLEAN CALLCLIKOK(TEHandle);
}

A1(static inline, BOOLEAN, CALLCLIKOK, TEHandle, teh)
{
    BOOLEAN retval;
    cliklooptype cp;

    if((cp = (cliklooptype)HxP(teh, clikLoop)))
    {
        ROMlib_hook(te_clikloopnumber);
        {
            LONGINT saved0, saved1, saved2, saved3,
                savea0, savea1, savea2, savea3;

            saved0 = EM_D0;
            saved1 = EM_D1;
            saved2 = EM_D2;
            saved3 = EM_D3;
            savea0 = EM_A0;
            savea1 = EM_A1;
            savea2 = EM_A2;
            savea3 = EM_A3;
            CALL_EMULATOR(US_TO_SYN68K((long)cp));

#define USE_Z_BIT_NOT_D0_FOR_CLIK_DETERMINATION /* as per Tom Pittman */
#if !defined(USE_Z_BIT_NOT_D0_FOR_CLIK_DETERMINATION)
            retval = EM_D0;
#else
            retval = !!cpu_state.ccnz;
#endif

            EM_D0 = saved0;
            EM_D1 = saved1;
            EM_D2 = saved2;
            EM_D3 = saved3;
            EM_A0 = savea0;
            EM_A1 = savea1;
            EM_A2 = savea2;
            EM_A3 = savea3;
        }
    }
    else
        retval = true;
    return retval;
}

typedef enum {
    forward = 1,
    backward = -1,
} word_break_dir_t;

static int
tep_find_word_break(TEPtr tep, int start, word_break_dir_t dir)
{
    Handle hText;
    SignedByte hText_flags;
    Ptr Text;
    int length;
    int current_index;
    int retval;

    hText = TEP_HTEXT(tep);
    hText_flags = HGetState(hText);
    HLock(hText);
    Text = STARH(hText);

    length = TEP_LENGTH(tep);

    for(current_index = start;
        current_index > 0 && current_index < length;
        current_index += dir)
    {
        if(ROMlib_wordb((char *)&Text[current_index]))
        {
            retval = current_index;
            if(dir == backward)
                retval++;
            goto done;
        }
    }
    retval = current_index;
done:
    HSetState(hText, hText_flags);
    return retval;
}

P3(PUBLIC pascal trap, void, TEClick, Point, pt, BOOLEAN, extend,
   TEHandle, te)
{
    EventRecord evt;
    SignedByte te_flags;
    TEPtr tep;
    int16 start, end;
    int16 click_posn;
    int16 origin = -1, true_origin = -1;
    int16 left_origin = -1, right_origin = -1;
    int16 length;
    int16 state;
    bool word_hilite_p = false;

    TESAVE(te);
    te_flags = HGetState((Handle)te);
    HLock((Handle)te);
    tep = STARH(te);

    length = TEP_LENGTH(tep);
    if(TEP_STYLIZED_P(tep))
    {
        TEStyleHandle te_style = TEP_GET_STYLE(tep);
        SCRAP_N_STYLES_X(TE_STYLE_NULL_SCRAP(te_style)) = CWC(0);
    }

    start = TEP_SEL_START(tep);
    end = TEP_SEL_END(tep);

    TEP_CLICK_STUFF_X(tep) = CWC(0);

    TEP_SEL_POINT(tep).h = CW(pt.h);
    TEP_SEL_POINT(tep).v = CW(pt.v);
    click_posn = TEP_DO_TEXT(tep, 0, length, teFind);

    state = TEP_CARET_STATE(tep);

#define te_toggle_hilite_range(start, end) \
    TEP_DO_TEXT(tep, start, end, teHilite)
#define te_find() \
    TEP_DO_TEXT(tep, 0, length, teFind)

    /* extend the current selection range; else nuke it */
    if(extend
       && (click_posn <= start
           || click_posn >= end)
       && (start != end
           || start != click_posn))
    {
        if(state == caret_vis)
            togglecaret(te, start, true);
        state = hilite_vis;

        if(click_posn < start)
        {
            te_toggle_hilite_range(click_posn, start);
            start = click_posn;
            origin = end;
        }
        else if(click_posn > end)
        {
            if(state == caret_vis)
                togglecaret(te, start, true);

            te_toggle_hilite_range(end, click_posn);
            end = click_posn;
            origin = start;
        }
        /* ### need else */
    }
    else
    {
        if(state == hilite_vis || state == caret_vis)
            ROMlib_togglelite(te);
        state = caret_invis;

        if(double_click_p(te, click_posn))
        {
            word_hilite_p = true;

            true_origin = click_posn;
            left_origin = start
                = tep_find_word_break(tep, click_posn, backward);
            right_origin = end
                = tep_find_word_break(tep, click_posn, forward);
            if(start != end)
            {
                te_toggle_hilite_range(start, end);
                state = hilite_vis;
            }
            else
            {
                /* draw the caret */
                togglecaret(te, start, true);
                state = caret_vis;
            }
        }
        else
        {
            origin = start = end = click_posn;

            /* draw the caret */
            togglecaret(te, start, true);
            state = caret_vis;
        }
    }

#define te_sel_new_end(_new_end)                      \
    ({                                                \
        decltype(_new_end) new_end = (_new_end);      \
                                                      \
        if(end != new_end)                            \
        {                                             \
            if(start == end)                          \
            {                                         \
                gui_assert(state == caret_vis);       \
                togglecaret(te, start, true);         \
                state = caret_invis;                  \
            }                                         \
                                                      \
            if(new_end < end)                         \
                te_toggle_hilite_range(new_end, end); \
            else if(new_end > end)                    \
                te_toggle_hilite_range(end, new_end); \
            end = new_end;                            \
                                                      \
            if(start == end)                          \
            {                                         \
                togglecaret(te, start, true);         \
                state = caret_vis;                    \
            }                                         \
            else                                      \
                state = hilite_vis;                   \
        }                                             \
    })

#define te_sel_new_start(_new_start)                      \
    ({                                                    \
        decltype(_new_start) new_start = (_new_start);    \
                                                          \
        if(start != new_start)                            \
        {                                                 \
            if(start == end)                              \
            {                                             \
                gui_assert(state == caret_vis);           \
                togglecaret(te, start, true);             \
                state = caret_invis;                      \
            }                                             \
                                                          \
            if(new_start < start)                         \
                te_toggle_hilite_range(new_start, start); \
            else if(new_start > start)                    \
                te_toggle_hilite_range(start, new_start); \
            start = new_start;                            \
                                                          \
            if(start == end)                              \
            {                                             \
                togglecaret(te, start, true);             \
                state = caret_vis;                        \
            }                                             \
            else                                          \
                state = hilite_vis;                       \
        }                                                 \
    })

    TEP_SEL_START_X(tep) = CW(start);
    TEP_SEL_END_X(tep) = CW(end);
    TEP_CARET_STATE_X(tep) = CW(state);

    while(Button() && CALLCLIKOK(te))
    {
        if(STARH(TEHIDDENH(te))->flags & CLC(TEAUTOVIEWBIT))
            ROMlib_teautoloop(te);

        GetOSEvent(0, &evt);
        GlobalToLocal(&evt.where);

        TEP_SEL_POINT(tep) = evt.where;

        click_posn = TEP_DO_TEXT(tep, 0, length, teFind);
        if(word_hilite_p)
        {
            if(click_posn < true_origin)
                click_posn = tep_find_word_break(tep, click_posn, backward);
            else
                click_posn = tep_find_word_break(tep, click_posn, forward);

            if(click_posn <= left_origin)
            {
                origin = right_origin;
                if(end != right_origin)
                {
                    te_toggle_hilite_range(right_origin, end);
                    end = right_origin;
                }
            }
            else
            {
                origin = left_origin;
                if(start != left_origin)
                {
                    te_toggle_hilite_range(start, left_origin);
                    start = left_origin;
                }
            }
        }

        if(click_posn != -1)
        {
            if(origin == start)
            {
                if((click_posn > end)
                   || (click_posn >= start
                       && click_posn < end))
                    te_sel_new_end(click_posn);
                else if(click_posn < start)
                {
                    te_sel_new_end(start);
                    te_sel_new_start(click_posn);

                    origin = end;
                }
            }
            else if(origin == end)
            {
                if(click_posn < start
                   || (click_posn <= end
                       && click_posn > start))
                    te_sel_new_start(click_posn);
                else if(click_posn > end)
                {
                    te_sel_new_start(end);
                    te_sel_new_end(click_posn);

                    origin = start;
                }
            }
            else
                gui_fatal("origin is neither start nor end");
        }

        TEP_SEL_START_X(tep) = CW(start);
        TEP_SEL_END_X(tep) = CW(end);
        TEP_CARET_STATE_X(tep) = CW(state);
    }

    /* suck out the up event if it is there */
    GetOSEvent(mUpMask, &evt);

    HSetState((Handle)te, te_flags);
    TERESTORE();
}

P3(PUBLIC pascal trap, void, TESetSelect, int32, start, int32, stop,
   TEHandle, teh)
{
    int16 length;

    TE_SLAM(teh);
    length = TE_LENGTH(teh);

    if(start < 0)
        start = 0;
    if(stop > length)
        stop = length;
    if(start > stop)
        start = stop;
    if(start != TE_SEL_START(teh)
       || stop != TE_SEL_END(teh))
    {
        TESAVE(teh);
        if(TE_CARET_STATE(teh) != caret_invis)
            ROMlib_togglelite(teh);
        if(start != stop && TE_ACTIVE(teh))
            TE_DO_TEXT(teh, start, stop, teHilite);
        TERESTORE();

        TE_SEL_START_X(teh) = CW(start);
        TE_SEL_END_X(teh) = CW(stop);
        if(TE_ACTIVE(teh))
            TE_CARET_STATE_X(teh) = (start != stop
                                         ? CWC(0)
                                         : CWC(255));
    }
    TE_SLAM(teh);
}

P1(PUBLIC pascal trap, void, TEActivate, TEHandle, teh)
{
    int16 start, end;

    TE_SLAM(teh);

    if(TE_ACTIVE_X(teh))
        return;

    start = TE_SEL_START(teh);
    end = TE_SEL_END(teh);
    if(start != end)
    {
        TESAVE(teh);
        TE_DO_TEXT(teh, start, end, teHilite);
        TE_CARET_STATE_X(teh) = CWC(hilite_vis);
        TERESTORE();
    }
    TE_ACTIVE_X(teh) = CWC(-256);

    TE_SLAM(teh);
}

P1(PUBLIC pascal trap, void, TEDeactivate, TEHandle, teh)
{
    TE_SLAM(teh);

    if(!TE_ACTIVE_X(teh))
        return;

    {
        TESAVE(teh);
        if(TE_CARET_STATE(teh) != caret_invis)
            ROMlib_togglelite(teh);
        TE_CARET_STATE_X(teh) = CWC(caret_invis);
        TERESTORE();
    }

    TE_ACTIVE_X(teh) = CWC(false);
}
