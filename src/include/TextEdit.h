#if !defined(_TEXTEDIT_H_)
#define _TEXTEDIT_H_

/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "QuickDraw.h"

#define MODULE_NAME TextEdit
#include <rsys/api-module.h>

namespace Executor
{
/* new justification defines, accepted by `TESetAlignment ()' and
   `TETextBox ()' */
enum
{
    teFlushDefault = 0,
    teCenter = 1,
    teFlushRight = (-1),
    teFlushLeft = (-2),
};

/* older justification defines */
enum
{
    teJustLeft = 0,
    teJustCenter = 1,
    teJustRight = (-1),
    teForceLeft = (-2),
};

enum
{
    doFont = 1,
    doFace = 2,
    doSize = 4,
    doColor = 8,
    doAll = 15,
    addSize = 16,
    doToggle = 32,
};

enum
{
    teFind = 0,
    teHilite = 1,
    teDraw = -1,
    teCaret = -2,
};

enum
{
    caret_vis = (-1),
    caret_invis = (255),
    hilite_vis = (0),
};

enum
{
    teFAutoScroll = (0),
    teFTextBuffering = (1),
    teFOutlineHilite = (2),
    teFInlineInput = (3),
    teFUseTextServices = (4),
};

enum
{
    teBitClear = (0),
    teBitSet = (1),
    teBitTest = (-1),
};

struct TERec
{
    GUEST_STRUCT;
    GUEST<Rect> destRect;
    GUEST<Rect> viewRect;
    GUEST<Rect> selRect;
    GUEST<INTEGER> lineHeight;
    GUEST<INTEGER> fontAscent;
    GUEST<Point> selPoint;
    GUEST<INTEGER> selStart;
    GUEST<INTEGER> selEnd;
    GUEST<INTEGER> active;
    GUEST<ProcPtr> wordBreak;
    GUEST<ProcPtr> clikLoop;
    GUEST<LONGINT> clickTime;
    GUEST<INTEGER> clickLoc;
    GUEST<LONGINT> caretTime;
    GUEST<INTEGER> caretState;
    GUEST<INTEGER> just;
    GUEST<INTEGER> teLength;
    GUEST<Handle> hText;
    GUEST<INTEGER> recalBack;
    GUEST<INTEGER> recalLines;
    GUEST<INTEGER> clikStuff;
    GUEST<INTEGER> crOnly;
    GUEST<INTEGER> txFont;
    GUEST<Style> txFace;
    GUEST<Byte> filler;
    GUEST<INTEGER> txMode;
    GUEST<INTEGER> txSize;
    GUEST<GrafPtr> inPort;
    GUEST<ProcPtr> highHook;
    GUEST<ProcPtr> caretHook;
    GUEST<INTEGER> nLines;
    GUEST<INTEGER[1]> lineStarts;
};

typedef TERec *TEPtr;

typedef GUEST<TEPtr> *TEHandle;

struct StyleRun
{
    GUEST_STRUCT;
    GUEST<INTEGER> startChar;
    GUEST<INTEGER> styleIndex;
};

struct STElement
{
    GUEST_STRUCT;
    GUEST<INTEGER> stCount;
    GUEST<INTEGER> stHeight;
    GUEST<INTEGER> stAscent;
    GUEST<INTEGER> stFont;
    GUEST<Style> stFace;
    GUEST<Byte> filler;
    GUEST<INTEGER> stSize;
    GUEST<RGBColor> stColor;
};

typedef STElement TEStyleTable[1];
typedef STElement *STPtr;

typedef GUEST<STPtr> *STHandle;

struct LHElement
{
    GUEST_STRUCT;
    GUEST<INTEGER> lhHeight;
    GUEST<INTEGER> lhAscent;
};

typedef LHElement LHTable[1];
typedef LHElement *LHPtr;

typedef GUEST<LHPtr> *LHHandle;

struct TextStyle
{
    GUEST_STRUCT;
    GUEST<INTEGER> tsFont;
    GUEST<Style> tsFace;
    GUEST<Byte> filler;
    GUEST<INTEGER> tsSize;
    GUEST<RGBColor> tsColor;
};

struct ScrpSTElement
{
    GUEST_STRUCT;
    GUEST<LONGINT> scrpStartChar;
    GUEST<INTEGER> scrpHeight;
    GUEST<INTEGER> scrpAscent;
    GUEST<INTEGER> scrpFont;
    GUEST<Style> scrpFace;
    GUEST<Byte> filler;
    GUEST<INTEGER> scrpSize;
    GUEST<RGBColor> scrpColor;
};

typedef ScrpSTElement ScrpSTTable[1];

struct StScrpRec
{
    GUEST_STRUCT;
    GUEST<INTEGER> scrpNStyles;
    GUEST<ScrpSTTable> scrpStyleTab;
};

typedef StScrpRec *StScrpPtr;

typedef GUEST<StScrpPtr> *StScrpHandle;

struct NullSTRec
{
    GUEST_STRUCT;
    GUEST<LONGINT> TEReserved;
    GUEST<StScrpHandle> nullScrap;
};

typedef NullSTRec *NullSTPtr;

typedef GUEST<NullSTPtr> *NullSTHandle;

struct TEStyleRec
{
    GUEST_STRUCT;
    GUEST<INTEGER> nRuns;
    GUEST<INTEGER> nStyles;
    GUEST<STHandle> styleTab;
    GUEST<LHHandle> lhTab;
    GUEST<LONGINT> teRefCon;
    GUEST<NullSTHandle> nullStyle;
    GUEST<StyleRun[1]> runs;
};

typedef TEStyleRec *TEStylePtr;

typedef GUEST<TEStylePtr> *TEStyleHandle;

typedef Byte Chars[1], *CharsPtr, **CharsHandle;

/* accessors! */

#define TE_DO_TEXT(te, start, end, what)                            \
    ({                                                              \
        int16_t retval;                                             \
                                                                    \
        HLockGuard guard(te);                                       \
        retval = ROMlib_call_TEDoText(STARH(te), start, end, what); \
        retval;                                                     \
    })
#define TE_CHAR_TO_POINT(te, sel, pt)         \
    ({                                        \
        HLockGuard guard(te);                 \
        te_char_to_point(STARH(te), sel, pt); \
    })

/* no need to lock te, since `te_char_to_lineno' cannot move memory
   blocks */
#define TE_CHAR_TO_LINENO(te, sel) \
    te_char_to_lineno(STARH(te), sel)

#define TE_DEST_RECT(te) (HxX((te), destRect))
#define TE_VIEW_RECT(te) (HxX((te), viewRect))
#define TE_SEL_RECT(te) (HxX((te), selRect))
#define TE_SEL_POINT(te) (HxX((te), selPoint))
#define TE_LINE_STARTS(te) (HxX((te), lineStarts))
#define TE_GET_STYLE(te)                                             \
    ({                                                               \
        TEStyleHandle retval;                                        \
                                                                     \
        if(!TE_STYLIZED_P(te))                                       \
            retval = NULL;                                           \
        else                                                         \
            retval = MR(*(GUEST<TEStyleHandle> *)&TE_TX_FONT_X(te)); \
        retval;                                                      \
    })

extern void ROMlib_sledgehammer_te(TEHandle te);
#if ERROR_SUPPORTED_P(ERROR_TEXT_EDIT_SLAM)
#define TE_SLAM(te)                               \
    ({                                            \
        if(ERROR_ENABLED_P(ERROR_TEXT_EDIT_SLAM)) \
            ROMlib_sledgehammer_te(te);           \
    })
#else /* No ERROR_TEXT_EDIT_SLAM */
#define TE_SLAM(te)
#endif /* No ERROR_TEXT_EDIT_SLAM */

#define TE_TX_FACE(te) (HxX((te), txFace))

#define TE_STYLIZED_P(te) (HxX((te), txSize) == CWC(-1))
#define TE_LINE_HEIGHT_X(te) (HxX((te), lineHeight))
#define TE_FONT_ASCENT_X(te) (HxX((te), fontAscent))
#define TE_LENGTH_X(te) (HxX((te), teLength))
#define TE_ACTIVE_X(te) (HxX((te), active))
#define TE_CARET_STATE_X(te) (HxX((te), caretState))
#define TE_SEL_START_X(te) (HxX((te), selStart))
#define TE_SEL_END_X(te) (HxX((te), selEnd))
#define TE_N_LINES_X(te) (HxX((te), nLines))
#define TE_HTEXT_X(te) (HxX((te), hText))
#define TE_CLICK_STUFF_X(te) (HxX((te), clikStuff))
#define TE_CLICK_LOC_X(te) (HxX((te), clickLoc))
#define TE_CLICK_TIME_X(te) (HxX((te), clickTime))
#define TE_JUST_X(te) (HxX((te), just))
#define TE_TX_FONT_X(te) (HxX((te), txFont))
#define TE_TX_SIZE_X(te) (HxX((te), txSize))
#define TE_TX_MODE_X(te) (HxX((te), txMode))
#define TE_IN_PORT_X(te) (HxX((te), inPort))

#define TE_LINE_HEIGHT(te) (CW(TE_LINE_HEIGHT_X(te)))
#define TE_FONT_ASCENT(te) (CW(TE_FONT_ASCENT_X(te)))
#define TE_LENGTH(te) (CW(TE_LENGTH_X(te)))
#define TE_ACTIVE(te) (CW(TE_ACTIVE_X(te)))
#define TE_CARET_STATE(te) (CW(TE_CARET_STATE_X(te)))
#define TE_SEL_START(te) (CW(TE_SEL_START_X(te)))
#define TE_SEL_END(te) (CW(TE_SEL_END_X(te)))
#define TE_N_LINES(te) (CW(TE_N_LINES_X(te)))
#define TE_CLICK_STUFF(te) (CW(TE_CLICK_STUFF_X(te)))
#define TE_CLICK_LOC(te) (CW(TE_CLICK_LOC_X(te)))
#define TE_CLICK_TIME(te) (CL(TE_CLICK_TIME_X(te)))
#define TE_HTEXT(te) (MR(TE_HTEXT_X(te)))
#define TE_JUST(te) (CW(TE_JUST_X(te)))
#define TE_TX_FONT(te) (CW(TE_TX_FONT_X(te)))
#define TE_TX_SIZE(te) (CW(TE_TX_SIZE_X(te)))
#define TE_TX_MODE(te) (CW(TE_TX_MODE_X(te)))
#define TE_IN_PORT(te) (MR(TE_IN_PORT_X(te)))

#define TE_FLAGS_X(te) (HxX(TEHIDDENH(te), flags))
#define TE_FLAGS(te) (CL(TE_FLAGS_X(te)))

#if !defined(NDEBUG)
#define TEP_DO_TEXT(tep, start, end, what)           \
    ({                                               \
        Handle te;                                   \
                                                     \
        te = RecoverHandle((Ptr)tep);                \
        gui_assert(te && (HGetState(te) & LOCKBIT)); \
        ROMlib_call_TEDoText(tep, start, end, what); \
    })
#else
#define TEP_DO_TEXT(tep, start, end, what) \
    ({ ROMlib_call_TEDoText(tep, start, end, what); })
#endif

#if !defined(NDEBUG)
#define TEP_CHAR_TO_POINT(tep, sel, pt)              \
    ({                                               \
        Handle te;                                   \
                                                     \
        te = RecoverHandle((Ptr)tep);                \
        gui_assert(te && (HGetState(te) & LOCKBIT)); \
        te_char_to_point(tep, sel, pt);              \
    })
#else
#define TEP_CHAR_TO_POINT(tep, sel, pt) \
    ({ te_char_to_point(tep, sel, pt); })
#endif
#define TEP_CHAR_TO_LINENO(tep, sel) \
    te_char_to_lineno(tep, sel)

#define TEP_SLAM(tep)                   \
    ({                                  \
        Handle te;                      \
                                        \
        te = RecoverHandle(tep)         \
            ROMlib_sledgehammer_te(te); \
    })

#define TEP_SEL_RECT(te) ((tep)->selRect)
#define TEP_DEST_RECT(tep) ((tep)->destRect)
#define TEP_VIEW_RECT(tep) ((tep)->viewRect)
#define TEP_LINE_STARTS(tep) ((tep)->lineStarts)
#define TEP_SEL_POINT(tep) ((tep)->selPoint)
#define TEP_TX_FACE(tep) ((tep)->txFace)
#define TEP_GET_STYLE(tep)                                             \
    ({                                                                 \
        TEStyleHandle retval;                                          \
                                                                       \
        if(!TEP_STYLIZED_P(tep))                                       \
            retval = NULL;                                             \
        else                                                           \
            retval = MR(*(GUEST<TEStyleHandle> *)&TEP_TX_FONT_X(tep)); \
        retval;                                                        \
    })

#define TEP_STYLIZED_P(tep) ((tep)->txSize == CWC(-1))
#define TEP_LINE_HEIGHT_X(tep) ((tep)->lineHeight)
#define TEP_FONT_ASCENT_X(tep) ((tep)->fontAscent)
#define TEP_LENGTH_X(tep) ((tep)->teLength)
#define TEP_ACTIVE_X(tep) ((tep)->active)
#define TEP_CARET_STATE_X(tep) ((tep)->caretState)
#define TEP_SEL_START_X(tep) ((tep)->selStart)
#define TEP_SEL_END_X(tep) ((tep)->selEnd)
#define TEP_N_LINES_X(tep) ((tep)->nLines)
#define TEP_HTEXT_X(tep) ((tep)->hText)
#define TEP_CLICK_STUFF_X(tep) ((tep)->clikStuff)
#define TEP_CLICK_LOC_X(tep) ((tep)->clickLoc)
#define TEP_JUST_X(tep) ((tep)->just)
#define TEP_TX_FONT_X(tep) ((tep)->txFont)
#define TEP_TX_SIZE_X(tep) ((tep)->txSize)
#define TEP_IN_PORT_X(tep) ((tep)->inPort)

#define TEP_LINE_HEIGHT(tep) (CW(TEP_LINE_HEIGHT_X(tep)))
#define TEP_FONT_ASCENT(tep) (CW(TEP_FONT_ASCENT_X(tep)))
#define TEP_LENGTH(tep) (CW(TEP_LENGTH_X(tep)))
#define TEP_ACTIVE(tep) (CW(TEP_ACTIVE_X(tep)))
#define TEP_CARET_STATE(tep) (CW(TEP_CARET_STATE_X(tep)))
#define TEP_SEL_START(tep) (CW(TEP_SEL_START_X(tep)))
#define TEP_SEL_END(tep) (CW(TEP_SEL_END_X(tep)))
#define TEP_N_LINES(tep) (CW(TEP_N_LINES_X(tep)))
#define TEP_CLICK_STUFF(tep) (CW(TEP_CLICK_STUFF_X(tep)))
#define TEP_CLICK_LOC(tep) (CL(TEP_CLICK_LOC_X(tep)))
#define TEP_HTEXT(tep) (MR(TEP_HTEXT_X(tep)))
#define TEP_JUST(tep) (CW(TEP_JUST_X(tep)))
#define TEP_TX_FONT(tep) (CW(TEP_TX_FONT_X(tep)))
#define TEP_TX_SIZE(tep) (CW(TEP_TX_SIZE_X(tep)))
#define TEP_IN_PORT(tep) (MR(TEP_IN_PORT_X(tep)))

#define TEP_HEIGHT_FOR_LINE(tep, lineno)                           \
    (TEP_STYLIZED_P(tep) && TEP_LINE_HEIGHT_X(tep) == CWC(-1)      \
         ? ({                                                      \
               TEStyleHandle te_style = TEP_GET_STYLE(tep);        \
               LHElement *lh = STARH(TE_STYLE_LH_TABLE(te_style)); \
               LH_HEIGHT(&lh[lineno]);                             \
           })                                                      \
         : TEP_LINE_HEIGHT(tep))
#define TEP_ASCENT_FOR_LINE(tep, lineno)                           \
    (TEP_STYLIZED_P(tep) && TEP_FONT_ASCENT_X(tep) == CWC(-1)      \
         ? ({                                                      \
               TEStyleHandle te_style = TEP_GET_STYLE(tep);        \
               LHElement *lh = STARH(TE_STYLE_LH_TABLE(te_style)); \
               LH_ASCENT(&lh[lineno]);                             \
           })                                                      \
         : TEP_FONT_ASCENT(tep))

#define TEP_TEXT_WIDTH(tep, text, start, len)     \
    (TEP_STYLIZED_P(tep)                          \
         ? ROMlib_StyleTextWidth(tep, start, len) \
         : TextWidth(text, start, len))

#define LINE_START_X(line_starts, index) \
    ((line_starts)[index])
#define LINE_START(line_starts, index) \
    (CW(LINE_START_X(line_starts, index)))

#define TE_STYLE_SIZE_FOR_N_RUNS(n_runs)         \
    (sizeof(TEStyleRec)                          \
     - sizeof TE_STYLE_RUNS((TEStyleHandle)NULL) \
     + (n_runs + 1) * sizeof *TE_STYLE_RUNS((TEStyleHandle)NULL))

#define TE_STYLE_N_RUNS_X(te_style) (HxX((te_style), nRuns))
#define TE_STYLE_N_STYLES_X(te_style) (HxX((te_style), nStyles))
#define TE_STYLE_RUNS(te_style) (HxX((te_style), runs))
#define TE_STYLE_RUN(te_style, run_i) \
    (&TE_STYLE_RUNS(te_style)[run_i])
#define TE_STYLE_STYLE_TABLE_X(te_style) \
    (HxX((te_style), styleTab))
#define TE_STYLE_LH_TABLE_X(te_style) \
    (HxX((te_style), lhTab))
#define TE_STYLE_NULL_STYLE_X(te_style) \
    (HxX((te_style), nullStyle))

#define TE_STYLE_N_RUNS(te_style) (CW(TE_STYLE_N_RUNS_X(te_style)))
#define TE_STYLE_N_STYLES(te_style) (CW(TE_STYLE_N_STYLES_X(te_style)))
#define TE_STYLE_STYLE_TABLE(te_style) \
    (MR(TE_STYLE_STYLE_TABLE_X(te_style)))
#define TE_STYLE_LH_TABLE(te_style) \
    (MR(TE_STYLE_LH_TABLE_X(te_style)))
#define TE_STYLE_NULL_STYLE(te_style) \
    (MR(TE_STYLE_NULL_STYLE_X(te_style)))

#define TE_STYLE_NULL_SCRAP(te_style) \
    (NULL_STYLE_NULL_SCRAP(TE_STYLE_NULL_STYLE(te_style)))

#define NULL_STYLE_NULL_SCRAP_X(null_style) \
    (HxX((null_style), nullScrap))
#define NULL_STYLE_NULL_SCRAP(null_style) \
    (MR(NULL_STYLE_NULL_SCRAP_X(null_style)))

#define SCRAP_N_STYLES_X(scrap) \
    (HxX((scrap), scrpNStyles))

#define SCRAP_ST_ELT(scrap, elt_i) \
    (&(HxX((scrap), scrpStyleTab))[elt_i])
#define SCRAP_N_STYLES(scrap) \
    (CW(SCRAP_N_STYLES_X(scrap)))

#define SCRAP_SIZE_FOR_N_STYLES(n_styles) \
    (sizeof(StScrpRec) + ((n_styles)-1) * sizeof(ScrpSTElement))

#define RUN_START_CHAR_X(run) (STYLE_RUN_START_CHAR_X(run))
#define RUN_STYLE_INDEX_X(run) (STYLE_RUN_STYLE_INDEX_X(run))

#define RUN_START_CHAR(run) (STYLE_RUN_START_CHAR(run))
#define RUN_STYLE_INDEX(run) (STYLE_RUN_STYLE_INDEX(run))

#define STYLE_RUN_START_CHAR_X(run) ((run)->startChar)
#define STYLE_RUN_STYLE_INDEX_X(run) ((run)->styleIndex)

#define STYLE_RUN_START_CHAR(run) (CW(STYLE_RUN_START_CHAR_X(run)))
#define STYLE_RUN_STYLE_INDEX(run) (CW(STYLE_RUN_STYLE_INDEX_X(run)))

#define TS_FACE(ts) ((ts)->tsFace)

#define TS_FONT_X(ts) ((ts)->tsFont)
#define TS_SIZE_X(ts) ((ts)->tsSize)
#define TS_COLOR(ts) ((ts)->tsColor)

#define TS_FONT(ts) (CW(TS_FONT_X(ts)))
#define TS_SIZE(ts) (CW(TS_SIZE_X(ts)))

#define LH_HEIGHT_X(lh) ((lh)->lhHeight)
#define LH_HEIGHT(lh) (CW(LH_HEIGHT_X(lh)))
#define LH_ASCENT_X(lh) ((lh)->lhAscent)
#define LH_ASCENT(lh) (CW(LH_ASCENT_X(lh)))

#define STYLE_TABLE_SIZE_FOR_N_STYLES(n_styles) \
    ((n_styles) * sizeof(STElement))

#define ST_ELT(st, st_elt_i) \
    (&STARH(st)[st_elt_i])

#define ST_ELT_FACE(st_elt) ((st_elt)->stFace)

#define ST_ELT_COUNT_X(st_elt) ((st_elt)->stCount)
#define ST_ELT_HEIGHT_X(st_elt) ((st_elt)->stHeight)
#define ST_ELT_ASCENT_X(st_elt) ((st_elt)->stAscent)
#define ST_ELT_FONT_X(st_elt) ((st_elt)->stFont)
#define ST_ELT_SIZE_X(st_elt) ((st_elt)->stSize)
#define ST_ELT_COLOR(st_elt) ((st_elt)->stColor)

#define ST_ELT_COUNT(st_elt) (CW(ST_ELT_COUNT_X(st_elt)))
#define ST_ELT_HEIGHT(st_elt) (CW(ST_ELT_HEIGHT_X(st_elt)))
#define ST_ELT_ASCENT(st_elt) (CW(ST_ELT_ASCENT_X(st_elt)))
#define ST_ELT_FONT(st_elt) (CW(ST_ELT_FONT_X(st_elt)))
#define ST_ELT_SIZE(st_elt) (CW(ST_ELT_SIZE_X(st_elt)))

#define SCRAP_ELT_START_CHAR_X(scrap_elt) \
    ((scrap_elt)->scrpStartChar)
#define SCRAP_ELT_START_CHAR(scrap_elt) \
    (CL(SCRAP_ELT_START_CHAR_X(scrap_elt)))

extern bool adjust_attrs(TextStyle *orig_attrs, TextStyle *new_attrs,
                         TextStyle *dst_attrs, TextStyle *continuous_attrs,
                         int16_t mode);
extern int16_t make_style_run_at(TEStyleHandle te_style, int16_t sel);
extern int16_t get_style_index(TEStyleHandle te_style, TextStyle *attrs,
                               int incr_count_p);
extern void release_style_index(TEStyleHandle te_style, int16_t style_index);
extern void stabilize_style_info(TEStyleHandle te_style);
extern void te_style_combine_runs(TEStyleHandle te_style);

const LowMemGlobal<ProcPtr> TEDoText { 0xA70 }; // TextEdit IMI-391 (true);
const LowMemGlobal<ProcPtr> TERecal { 0xA74 }; // TextEdit IMI-391 (false);
const LowMemGlobal<INTEGER> TEScrpLength { 0xAB0 }; // TextEdit IMI-389 (true);
const LowMemGlobal<Handle> TEScrpHandle { 0xAB4 }; // TextEdit IMI-389 (true);

DISPATCHER_TRAP(TEDispatch, 0xA83D, StackW);

extern void C_TESetText(Ptr p, LONGINT ln, TEHandle teh);
PASCAL_TRAP(TESetText, 0xA9CF);

extern CharsHandle C_TEGetText(TEHandle teh);
PASCAL_TRAP(TEGetText, 0xA9CB);

extern void C_TESetJust(INTEGER j, TEHandle teh);
PASCAL_TRAP(TESetJust, 0xA9DF);

extern void C_TEUpdate(Rect *r, TEHandle teh);
PASCAL_TRAP(TEUpdate, 0xA9D3);

extern void C_TextBox(Ptr p, LONGINT ln, Rect *r,
                                  INTEGER j);
PASCAL_TRAP(TextBox, 0xA9CE);
extern void C_TEScroll(INTEGER dh, INTEGER dv, TEHandle teh);
PASCAL_TRAP(TEScroll, 0xA9DD);

extern void C_TEKey(CharParameter thec, TEHandle teh);
PASCAL_TRAP(TEKey, 0xA9DC);

extern void C_TECopy(TEHandle teh);
PASCAL_TRAP(TECopy, 0xA9D5);

extern void C_TECut(TEHandle teh);
PASCAL_TRAP(TECut, 0xA9D6);

extern void C_TEPaste(TEHandle teh);
PASCAL_TRAP(TEPaste, 0xA9DB);

extern void C_TEDelete(TEHandle teh);
PASCAL_TRAP(TEDelete, 0xA9D7);

extern void C_TEInsert(Ptr p, LONGINT ln, TEHandle teh);
PASCAL_TRAP(TEInsert, 0xA9DE);

extern void C_TEPinScroll(INTEGER dh,
                                      INTEGER dv, TEHandle teh);
PASCAL_TRAP(TEPinScroll, 0xA812);
extern void ROMlib_teautoloop(TEHandle teh);
extern void C_TESelView(TEHandle teh);
PASCAL_TRAP(TESelView, 0xA811);

extern void C_TEAutoView(BOOLEAN autoflag,
                                     TEHandle teh);
PASCAL_TRAP(TEAutoView, 0xA813);
extern TEHandle C_TEStylNew(Rect *dst, Rect *view);
PASCAL_TRAP(TEStylNew, 0xA83E);

extern void C_SetStylHandle(TEStyleHandle theHandle,
                                        TEHandle teh);
PASCAL_SUBTRAP(SetStylHandle, 0xA83D, 0x0005, TEDispatch);
extern TEStyleHandle C_GetStylHandle(TEHandle teh);
PASCAL_SUBTRAP(GetStylHandle, 0xA83D, 0x0004, TEDispatch);

extern StScrpHandle C_GetStylScrap(TEHandle teh);
PASCAL_SUBTRAP(GetStylScrap, 0xA83D, 0x0006, TEDispatch);

extern void C_TEStylInsert(Ptr text, LONGINT length,
                                       StScrpHandle hST, TEHandle teh);
PASCAL_SUBTRAP(TEStylInsert, 0xA83D, 0x0007, TEDispatch);
extern INTEGER C_TEGetOffset(Point pt, TEHandle teh);
PASCAL_TRAP(TEGetOffset, 0xA83C);

extern LONGINT C_TEGetPoint(INTEGER offset, TEHandle teh);
PASCAL_SUBTRAP(TEGetPoint, 0xA83D, 0x0008, TEDispatch);

extern int32_t C_TEGetHeight(LONGINT endLine, LONGINT startLine, TEHandle teh);
PASCAL_SUBTRAP(TEGetHeight, 0xA83D, 0x0009, TEDispatch);
extern void C_TEGetStyle(INTEGER offset,
                                     TextStyle *theStyle, GUEST<INTEGER> *lineHeight, GUEST<INTEGER> *fontAscent,
                                     TEHandle teh);
PASCAL_SUBTRAP(TEGetStyle, 0xA83D, 0x0003, TEDispatch);
extern void C_TEStylPaste(TEHandle teh);
PASCAL_SUBTRAP(TEStylPaste, 0xA83D, 0x0000, TEDispatch);

extern void C_TESetStyle(INTEGER mode, TextStyle *newStyle,
                                     BOOLEAN redraw, TEHandle teh);
PASCAL_SUBTRAP(TESetStyle, 0xA83D, 0x0001, TEDispatch);
extern void C_TEReplaceStyle(INTEGER mode,
                                         TextStyle *oldStyle, TextStyle *newStyle, BOOLEAN redraw, TEHandle teh);
PASCAL_SUBTRAP(TEReplaceStyle, 0xA83D, 0x0002, TEDispatch);
extern BOOLEAN C_TEContinuousStyle(GUEST<INTEGER> *modep,
                                               TextStyle *thestyle, TEHandle teh);
PASCAL_SUBTRAP(TEContinuousStyle, 0xA83D, 0x000A, TEDispatch);
extern void C_SetStylScrap(LONGINT start, LONGINT stop,
                                       StScrpHandle newstyles, BOOLEAN redraw, TEHandle teh);
PASCAL_SUBTRAP(SetStylScrap, 0xA83D, 0x000B, TEDispatch);
extern void C_TECustomHook(INTEGER sel, GUEST<ProcPtr> *addr,
                                       TEHandle teh);
PASCAL_SUBTRAP(TECustomHook, 0xA83D, 0x000C, TEDispatch);
extern LONGINT C_TENumStyles(LONGINT start, LONGINT stop,
                                         TEHandle teh);
PASCAL_SUBTRAP(TENumStyles, 0xA83D, 0x000D, TEDispatch);
extern void C_TEInit(void);
PASCAL_TRAP(TEInit, 0xA9CC);

extern TEHandle C_TENew(Rect *dst, Rect *view);
PASCAL_TRAP(TENew, 0xA9D2);

extern void C_TEDispose(TEHandle teh);
PASCAL_TRAP(TEDispose, 0xA9CD);

extern void C_TEIdle(TEHandle teh);
PASCAL_TRAP(TEIdle, 0xA9DA);

extern void C_TEClick(Point p, BOOLEAN ext, TEHandle teh);
PASCAL_TRAP(TEClick, 0xA9D4);

extern void C_TESetSelect(LONGINT start, LONGINT stop,
                                      TEHandle teh);
PASCAL_TRAP(TESetSelect, 0xA9D1);
extern void C_TEActivate(TEHandle teh);
PASCAL_TRAP(TEActivate, 0xA9D8);

extern void C_TEDeactivate(TEHandle teh);
PASCAL_TRAP(TEDeactivate, 0xA9D9);

extern void SetWordBreak(ProcPtr wb, TEHandle teh);
extern void SetClikLoop(ProcPtr cp, TEHandle teh);
extern void C_TECalText(TEHandle teh);
PASCAL_TRAP(TECalText, 0xA9D0);

extern OSErr TEFromScrap(void);
extern OSErr TEToScrap(void);
extern Handle TEScrapHandle(void);
extern LONGINT TEGetScrapLen(void);
extern void TESetScrapLen(LONGINT ln);
extern int16_t C_TEFeatureFlag(int16_t feature, int16_t action,
                                           TEHandle te);
PASCAL_SUBTRAP(TEFeatureFlag, 0xA83D, 0x000E, TEDispatch);
}
#endif /* _TEXTEDIT_H_ */
