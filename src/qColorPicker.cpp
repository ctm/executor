/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "WindowMgr.h"
#include "ControlMgr.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "TextEdit.h"
#include "EventMgr.h"
#include "ToolboxEvent.h"
#include "OSEvent.h"

#include "rsys/cquick.h"
#include "rsys/ctl.h"

/* ### hack for `image_bits ()' */
#include "rsys/menu.h"
#include "rsys/color_wheel_bits.h"
#include "rsys/osevent.h"
#include "rsys/options.h"

#include <ctype.h>
#include <math.h>

using namespace Executor;

/* sanity defines */

#define _NewCWindow(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
    ((WindowPtr)NewCWindow(arg0, arg1, arg2, arg3, arg4,            \
                           (CWindowPtr)(arg5), arg6, arg7))

#define _FindControl(arg0, arg1, arg2)             \
    ({                                             \
        int16_t retval;                            \
        GUEST<ControlHandle> bogo_c;               \
                                                   \
        retval = FindControl(arg0, arg1, &bogo_c); \
        *(arg2) = MR(bogo_c);                      \
                                                   \
        retval;                                    \
    })

#define sqr(v) ({ decltype (v) _v = (v);  _v * _v; })

#define color_picker_window_bounds (_bounds)
#define ok_button_bounds (&_bounds[1])
#define cancel_button_bounds (&_bounds[2])

#define template_text_box_bounds (&_bounds[3])

#define template_compare_box_bounds (&_bounds[9])

#define prompt_bounds (&_bounds[12])
#define template_color_wheel_bounds (&_bounds[13])

static Rect _bounds[] = {
    /* top, left, bottom, right */

    /* color picker window bounds; filled in by `compute_bounds' */
    { CWC((short)-1), CWC((short)-1), CWC((short)-1), CWC((short)-1) },

    /* ok button bounds */
    { CWC(330), CWC(410), CWC(350), CWC(470) },
    /* cancel button bounds */
    { CWC(330), CWC(337), CWC(350), CWC(397) },

    /* text entry box rects */
    { CWC(324), CWC(10), CWC(350), CWC(90) },
    { CWC(299), CWC(10), CWC(325), CWC(90) },
    { CWC(274), CWC(10), CWC(300), CWC(90) },

    { CWC(228), CWC(10), CWC(254), CWC(90) },
    { CWC(203), CWC(10), CWC(229), CWC(90) },
    { CWC(178), CWC(10), CWC(204), CWC(90) },

    /* rectangle for the color comparison boxes */
    /* frame */
    { CWC(78), CWC(0), CWC(138), CWC(100) },
    /* orig, current */
    { CWC(108), CWC(2), CWC(136), CWC(98) },
    { CWC(80), CWC(2), CWC(108), CWC(98) },

    /* prompt bounds */
    { CWC(10), CWC(10), CWC(60), CWC(240) },

    /* color wheel bounds */
    { CWC(20), CWC(268), CWC(212), CWC(460) },
};

#define compare_box_frame_bounds (compare_box_bounds)
#define orig_compare_box_bounds (&compare_box_bounds[1])
#define current_compare_box_bounds (&compare_box_bounds[2])

static Rect compare_box_bounds[3];

static Rect color_wheel_bounds[1];

static int color_wheel_center_x, color_wheel_center_y;

#define N_TEXT_BOXES (6)

struct text_box
{
    Point title_pt;
    StringPtr title;

    Point label_pt;
    StringPtr label;

    Rect frame_rect;
    Rect te_rect;
    Rect miniarrow_rect;

    /* ### maintaining seperate integer/factional portions is way too
     complicated nad messy.  i did this because originally i wanted to
     avoid using floating point (but that became necessary to
     generate/interpret the color wheel target location), and having
     all the color values stored in 16bit quanities resulted in
     roundoff when converting to text and back again */
    int integer, fractional;

    int *value;
    int max;
    int continuous_p;

    int i;
} text_boxes[N_TEXT_BOXES];

TEHandle te;

/* index into `text_boxes' which `te' currently represents */
static struct text_box *te_box;

static WindowPtr color_picker_window;

static ControlHandle cancel_button, ok_button;

static StringPtr prompt_text;

static int font_height, font_ascent;
static int space_width;

static void
text_box_update_value(struct text_box *box, int newval,
                      bool update_if_p);
static void
text_box_update_value_1(struct text_box *box, int newval,
                        bool update_if_p,
                        bool update_color_wheel_target_p);

static void
text_box_update_if_value(struct text_box *box,
                         int new_integer, int new_fractional);
static void
text_box_update_if_value_1(struct text_box *box,
                           int new_integer, int new_fractional);

static void val_if(int val, int max,
                   int *integer_return, int *fractional_return);
static int if_val(int max, int integer, int fractional);

static void color_wheel_update(void);
static void color_wheel_notice_lightness_change(void);
static void color_wheel_target_update(bool short_cut_p);

#define red_index (0)
#define green_index (1)
#define blue_index (2)
#define hue_index (3)
#define saturation_index (4)
#define lightness_index (5)

static int red;
static int green;
static int blue;

static int hue;
static int saturation;
static int lightness;

static RGBColor current_color;
static RGBColor orig_color;

static void
compute_bounds(Point maybe_top_left)
{
    int width, height;
    int top, left;

    /* note: must match width/height for `color_picker_window_bounds' */
    width = 480;
    height = 360;

    if((!maybe_top_left.v && !maybe_top_left.h)
       /* #### ignore the application suggestion, and always center the
	 color picker sanely
	 
	 for some reason, Tex-Edit passes in `-1, -1' as the suggested
	 location */
       || 1)
    {
        Rect *gd_rect;
        int gd_width;
        int gd_height;

        gd_rect = &GD_RECT(MR(MainDevice));
        gd_width = RECT_WIDTH(gd_rect);
        gd_height = RECT_HEIGHT(gd_rect);

        /* centered horizontally, with a third of the space above the
	 window, and two-thirds below */
        top = CW(gd_rect->top) + (gd_height - height) / 3;
        left = CW(gd_rect->left) + (gd_width - width) / 2;
    }
    else
    {
        top = maybe_top_left.v;
        left = maybe_top_left.h;
    }

    color_picker_window_bounds->top = CW(top);
    color_picker_window_bounds->left = CW(left);
    color_picker_window_bounds->bottom = CW(top + height);
    color_picker_window_bounds->right = CW(left + width);
}

typedef enum miniarrow_hilite {
    miniarrow_no_hilite,
    miniarrow_up_hilite,
    miniarrow_down_hilite,
} miniarrow_hilite_t;

static bool integer_increment_p;
static int track_kount;

static void
miniarrow_track(struct text_box *box, miniarrow_hilite_t _hilite)
{
    int integer, fractional, max;
    bool continuous_p;

    integer = box->integer;
    fractional = box->fractional;
    max = box->max;
    continuous_p = box->continuous_p;

    switch(_hilite)
    {
        case miniarrow_up_hilite:
            if(integer_increment_p)
            {
                if(integer == max)
                {
                    if(continuous_p)
                        integer = 0;
                }
                else
                    integer++;
            }
            else if(fractional == 0
                    && integer == max)
            {
                if(continuous_p)
                    integer = 0;
            }
            else if(fractional == 9)
            {
                if(track_kount > 5)
                    integer_increment_p = true;

                integer++;
                fractional = 0;
            }
            else
                fractional++;
            break;
        case miniarrow_down_hilite:
            if(integer_increment_p)
            {
                if(integer == 0)
                {
                    if(continuous_p)
                        integer = max;
                }
                else
                    integer--;
            }
            else if(fractional == 0
                    && integer == 0)
            {
                if(continuous_p)
                    integer = max;
            }
            else if(fractional == 0)
            {
                integer--;

                if(track_kount > 5)
                    integer_increment_p = true;
                else
                    fractional = 9;
            }
            else
                fractional--;
            break;
        default:
            break;
    }

    /* note that the integer and/or fractional values have changed */
    text_box_update_if_value(box, integer, fractional);

    track_kount++;
}

static void
text_box_init(void)
{
    static StringPtr titles[N_TEXT_BOXES] = {
        (StringPtr) "\005Red: ",
        (StringPtr) "\007Green: ",
        (StringPtr) "\006Blue: ",
        (StringPtr) "\013Hue Angle: ",
        (StringPtr) "\014Saturation: ",
        (StringPtr) "\013Lightness: ",
    };
    static StringPtr labels[N_TEXT_BOXES] = {
        (StringPtr) "\001%",
        (StringPtr) "\001%",
        (StringPtr) "\001%",
        (StringPtr) "\001\241", /* degree symbol? */
        (StringPtr) "\001%",
        (StringPtr) "\001%",
    };
    int *values[N_TEXT_BOXES] = {
        &red, &green, &blue, &hue, &saturation, &lightness,
    };
    int maxi[N_TEXT_BOXES] = { 100, 100, 100, 360, 100, 100 };
    int offset, label_width;
    int i;

    /* compute the text box offset caused by the titles */
    offset = -1;
    label_width = -1;
    for(i = 0; i < (int)NELEM(titles); i++)
    {
        StringPtr title, label;

        title = titles[i];
        label = labels[i];
        offset = MAX(offset, StringWidth(title));
        label_width = MAX(label_width, StringWidth(label));
    }

    for(i = 0; i < N_TEXT_BOXES; i++)
    {
        struct text_box *box;
        Rect *template_bounds, *frame_bounds, *te_bounds;
        Rect *miniarrow_bounds;
        int baseline;
        Point *title_pt, *label_pt;
        StringPtr title, label;

        template_bounds = &template_text_box_bounds[i];
        title = titles[i];
        label = labels[i];

        box = &text_boxes[i];

        box->i = i;
        box->title = title;
        box->label = label;
        box->max = maxi[i];
        box->value = values[i];
        box->continuous_p = (i == hue_index);

        /* set up the default integer/factional values based on the
	 default value */
        val_if(*(box->value), box->max, &box->integer, &box->fractional);

        frame_bounds = &box->frame_rect;
        te_bounds = &box->te_rect;

        *frame_bounds = *template_bounds;
        OffsetRect(frame_bounds, offset, 0);

        *te_bounds = *frame_bounds;
        InsetRect(te_bounds, 4, 4);

        baseline = ((CW(frame_bounds->top)
                     + CW(frame_bounds->bottom))
                        / 2
                    - font_height / 2 + font_ascent);

        title_pt = &box->title_pt;
        label_pt = &box->label_pt;

        title_pt->v = label_pt->v = baseline;
        title_pt->h = CW(template_bounds->left) + offset - StringWidth(title);
        label_pt->h = CW(frame_bounds->right) + space_width / 2;

        miniarrow_bounds = &box->miniarrow_rect;

        *miniarrow_bounds = *frame_bounds;
        miniarrow_bounds->left = CW(label_pt->h
                                    + label_width
                                    + space_width / 2);
        miniarrow_bounds->right = CW(label_pt->h
                                     + label_width
                                     + space_width / 2
                                     + 22);
    }

    for(i = 0; i < 3; i++)
    {
        Rect *template_bounds, *bounds;

        bounds = &compare_box_bounds[i];
        template_bounds = &template_compare_box_bounds[i];

        *bounds = *template_bounds;
        OffsetRect(bounds, offset, 0);
    }
}

static void
str_if(char *text, int len,
       int *return_integer, int *return_fractional)
{
    char intbuf[16], fractbuf[16];
    int i;

    for(i = 0; i < 15 && i < len && isdigit(text[i]); i++)
        intbuf[i] = text[i];
    intbuf[i] = '\000';

    if(i < 16 && i < len && text[i] == '.' && isdigit(text[i + 1]))
    {
        fractbuf[0] = text[i + 1];
        fractbuf[1] = '\000';
    }
    else
        *fractbuf = '\000';

    *return_integer = atoi(intbuf);
    *return_fractional = atoi(fractbuf);
}

static unsigned char normal_bits[] = {
    image_bits(00000000), image_bits(00000000),
    image_bits(00011111), image_bits(11000000),
    image_bits(00100000), image_bits(00100000),
    image_bits(01000000), image_bits(00010000),
    image_bits(01000010), image_bits(00010000),
    image_bits(01000111), image_bits(00010000),
    image_bits(01001111), image_bits(10010000),
    image_bits(01011111), image_bits(11010000),
    image_bits(01000111), image_bits(00010000),
    image_bits(01000111), image_bits(00010000),
    image_bits(01000000), image_bits(00010000),
    image_bits(01000000), image_bits(00010000),
    image_bits(01000111), image_bits(00010000),
    image_bits(01000111), image_bits(00010000),
    image_bits(01011111), image_bits(11010000),
    image_bits(01001111), image_bits(10010000),
    image_bits(01000111), image_bits(00010000),
    image_bits(01000010), image_bits(00010000),
    image_bits(01000000), image_bits(00010000),
    image_bits(00100000), image_bits(00100000),
    image_bits(00011111), image_bits(11000000),
    image_bits(00000000), image_bits(00000000),
};

static unsigned char lower_highlighted_bits[] = {
    image_bits(00000000), image_bits(00000000),
    image_bits(00011111), image_bits(11000000),
    image_bits(00100000), image_bits(00100000),
    image_bits(01000000), image_bits(00010000),
    image_bits(01000010), image_bits(00010000),
    image_bits(01000111), image_bits(00010000),
    image_bits(01001111), image_bits(10010000),
    image_bits(01011111), image_bits(11010000),
    image_bits(01000111), image_bits(00010000),
    image_bits(01000111), image_bits(00010000),
    image_bits(01000000), image_bits(00010000),
    image_bits(01111111), image_bits(11110000),
    image_bits(01111000), image_bits(11110000),
    image_bits(01111000), image_bits(11110000),
    image_bits(01100000), image_bits(00110000),
    image_bits(01110000), image_bits(01110000),
    image_bits(01111000), image_bits(11110000),
    image_bits(01111101), image_bits(11110000),
    image_bits(01111111), image_bits(11110000),
    image_bits(00111111), image_bits(11100000),
    image_bits(00011111), image_bits(11000000),
    image_bits(00000000), image_bits(00000000),
};

static unsigned char upper_highlighted_bits[] = {
    image_bits(00000000), image_bits(00000000),
    image_bits(00011111), image_bits(11000000),
    image_bits(00111111), image_bits(11100000),
    image_bits(01111111), image_bits(11110000),
    image_bits(01111101), image_bits(11110000),
    image_bits(01111000), image_bits(11110000),
    image_bits(01110000), image_bits(01110000),
    image_bits(01100000), image_bits(00110000),
    image_bits(01111000), image_bits(11110000),
    image_bits(01111000), image_bits(11110000),
    image_bits(01111111), image_bits(11110000),
    image_bits(01000000), image_bits(00010000),
    image_bits(01000111), image_bits(00010000),
    image_bits(01000111), image_bits(00010000),
    image_bits(01011111), image_bits(11010000),
    image_bits(01001111), image_bits(10010000),
    image_bits(01000111), image_bits(00010000),
    image_bits(01000010), image_bits(00010000),
    image_bits(01000000), image_bits(00010000),
    image_bits(00100000), image_bits(00100000),
    image_bits(00011111), image_bits(11000000),
    image_bits(00000000), image_bits(00000000),
};

static void
text_box_miniarrow_update(struct text_box *box,
                          miniarrow_hilite_t _hilite)
{
    Rect dst_rect, *miniarrow_rect;
    BitMap miniarrow_bitmap;
    int center_x, center_y;
    char *bits = NULL;

    switch(_hilite)
    {
        case miniarrow_no_hilite:
            bits = (char *)normal_bits;
            break;
        case miniarrow_up_hilite:
            bits = (char *)upper_highlighted_bits;
            break;
        case miniarrow_down_hilite:
            bits = (char *)lower_highlighted_bits;
            break;
    }

    miniarrow_bitmap.baseAddr = RM((Ptr)bits);
    miniarrow_bitmap.rowBytes = CWC(2);
    SetRect(&miniarrow_bitmap.bounds, 0, 0, 13, 22);

    miniarrow_rect = &box->miniarrow_rect;

    center_x = (CW(miniarrow_rect->left) + CW(miniarrow_rect->right)) / 2;
    center_y = (CW(miniarrow_rect->top) + CW(miniarrow_rect->bottom)) / 2;

    SetRect(&dst_rect,
            center_x - 13 / 2, center_y - 22 / 2,
            center_x - 13 / 2 + 13, center_y - 22 / 2 + 22);

    CopyBits(&miniarrow_bitmap, PORT_BITS_FOR_COPY(thePort),
             &miniarrow_bitmap.bounds, &dst_rect, srcCopy, NULL);
}

static void
text_box_update(struct text_box *box, bool update_text_p)
{
    char buf[16];

    MoveTo(box->title_pt.h, box->title_pt.v);
    DrawString(box->title);

    sprintf(buf, "%d.%d", box->integer, box->fractional);
    if(te_box == box)
    {
        if(update_text_p)
        {
            Rect dummy_rect;

            te_box = NULL;

            TEDeactivate(te);

            memset(&dummy_rect, '\000', sizeof dummy_rect);
            TE_DEST_RECT(te) = dummy_rect;
            TE_VIEW_RECT(te) = dummy_rect;

            TextBox((Ptr)buf, strlen(buf), &box->te_rect, teFlushRight);
        }
        else
            TEUpdate(&box->te_rect, te);
    }
    else
        TextBox((Ptr)buf, strlen(buf), &box->te_rect, teFlushRight);

    PenSize(1, 1);
    FrameRect(&box->frame_rect);

    MoveTo(box->label_pt.h, box->label_pt.v);
    DrawString(box->label);
}

static int
if_val(int max, int integer, int fractional)
{
    return ((integer * 0xFFFF) + (fractional * 0x1999)) / max;
}

static void
val_if(int val, int max,
       int *integer_return, int *fractional_return)
{
    *integer_return = (val * max) / 0xFFFF;
    *fractional_return = (((val * max) - (*integer_return * 0xFFFF)) * 10) / 0xFFFF;
}

static void
text_box_update_value_1(struct text_box *box, int newval,
                        bool update_if_p,
                        bool update_color_wheel_target_p)
{
    int *value;

    value = box->value;
    if(*value == newval)
    {
        /* redisplay even if the values are same, so the display always
	 appears `cannonical' (even if, say, the user sets the text
	 box to `' (whic defaults to zero), and `*value' is zero) */
        text_box_update(box, true);
        return;
    }
    *value = newval;

    if(update_if_p)
    {
        int integer, fractional;

        val_if(newval, box->max, &integer, &fractional);
        text_box_update_if_value_1(box, integer, fractional);
    }

    switch(box->i)
    {
        case red_index:
            current_color.red = CW(red);
            break;
        case green_index:
            current_color.green = CW(green);
            break;
        case blue_index:
            current_color.blue = CW(blue);
            break;
        case hue_index:
        case saturation_index:
            if(update_color_wheel_target_p)
                color_wheel_target_update(true);
        case lightness_index:
            color_wheel_notice_lightness_change();
    }

    /* redraw the box with the `cannonicalized' value */
    text_box_update(box, true);
}

static void
compare_box_update(void)
{
    /* ### titles? */
    /* draw the color comparison box */

    RGBForeColor(&orig_color);
    FillRect(orig_compare_box_bounds, black);

    RGBForeColor(&current_color);
    FillRect(current_compare_box_bounds, black);

    PenSize(2, 2);
    ForeColor(blackColor);
    FrameRect(compare_box_frame_bounds);
}

static void
hue_saturation_update(int new_hue, int new_saturation,
                      bool full_update_p)
{
    RGBColor rgb_color;
    HSLColor hsl_color;

    if(hue == new_hue && saturation == new_saturation)
        return;

    text_box_update_value_1(&text_boxes[hue_index], new_hue,
                            true, false);
    text_box_update_value_1(&text_boxes[saturation_index], new_saturation,
                            true, false);

    if(full_update_p)
    {
        hsl_color.hue = CW(hue);
        hsl_color.saturation = CW(saturation);
        hsl_color.lightness = CW(lightness);

        HSL2RGB(&hsl_color, &rgb_color);

        text_box_update_value_1(&text_boxes[red_index],
                                CW(rgb_color.red), true, true);
        text_box_update_value_1(&text_boxes[green_index],
                                CW(rgb_color.green), true, true);
        text_box_update_value_1(&text_boxes[blue_index],
                                CW(rgb_color.blue), true, true);

        compare_box_update();
    }
    color_wheel_target_update(true);
}

static void
text_box_update_if_value_1(struct text_box *box,
                           int new_integer, int new_fractional)
{
    box->integer = new_integer;
    box->fractional = new_fractional;

    text_box_update(box, true);
}

static void
text_box_update_if_value(struct text_box *box,
                         int new_integer, int new_fractional)
{
    if(box->integer == new_integer
       && box->fractional == new_fractional)
    {
        /* to keep the box `cannonical' */
        text_box_update(box, true);
        return;
    }

    text_box_update_if_value_1(box, new_integer, new_fractional);
    text_box_update_value(box, if_val(box->max, new_integer, new_fractional),
                          false);
}

/* the value of `box' has changed, and we need to recompute the
   values for the `dual' color because of it */

static void
text_box_update_value(struct text_box *box, int newval,
                      bool update_if_p)
{
    RGBColor rgb_color;
    HSLColor hsl_color;

    switch(box->i)
    {
        case red_index:
        case green_index:
        case blue_index:
        {
            text_box_update_value_1(box, newval, update_if_p, true);

            rgb_color.red = CW(red);
            rgb_color.green = CW(green);
            rgb_color.blue = CW(blue);

            RGB2HSL(&rgb_color, &hsl_color);

            hue_saturation_update(CW(hsl_color.hue), CW(hsl_color.saturation),
                                  false);

            text_box_update_value_1(&text_boxes[lightness_index],
                                    CW(hsl_color.lightness), true, true);
            break;
        }

        case hue_index:
        case saturation_index:
        case lightness_index:
        {
            text_box_update_value_1(box, newval, update_if_p, true);

            hsl_color.hue = CW(hue);
            hsl_color.saturation = CW(saturation);
            hsl_color.lightness = CW(lightness);

            HSL2RGB(&hsl_color, &rgb_color);

            text_box_update_value_1(&text_boxes[red_index],
                                    CW(rgb_color.red), true, true);
            text_box_update_value_1(&text_boxes[green_index],
                                    CW(rgb_color.green), true, true);
            text_box_update_value_1(&text_boxes[blue_index],
                                    CW(rgb_color.blue), true, true);
            break;
        }
    }

    compare_box_update();
}

static void
text_box_set_te(struct text_box *box)
{
    struct text_box *orig_te_box;
    char buf[16], *text;
    Handle texth;

    if(box == te_box)
        return;

    orig_te_box = te_box;
    te_box = box;

    TEDeactivate(te);

    if(orig_te_box)
    {
        int integer, fractional;
        int max;

        texth = TE_HTEXT(te);
        text = (char *)STARH(texth);

        str_if(text, TE_LENGTH(te), &integer, &fractional);
        max = orig_te_box->max;
        if(integer > max
           || (integer == max && fractional))
        {
            integer = max;
            fractional = 0;
        }
        text_box_update_if_value(orig_te_box, integer, fractional);
    }

    /* slimy */
    TE_DEST_RECT(te) = box->te_rect;
    TE_VIEW_RECT(te) = box->te_rect;

    sprintf(buf, "%d.%d", box->integer, box->fractional);
    TESetText((Ptr)buf, strlen(buf), te);
    TESetSelect(0, TE_LENGTH(te), te);

    TEActivate(te);
}

static bool
event_loop(void)
{
    EventRecord evt;

    for(;;)
    {
        GetNextEvent((mDownMask | mUpMask
                      | keyDownMask | keyUpMask | autoKeyMask
                      | updateMask | activMask),
                     &evt);

        TEIdle(te);

        switch(CW(evt.what))
        {
            case mouseDown:
            {
                Point local_pt;
                bool control_p;
                ControlHandle c;
                int16_t release_part;
                int i;

                /* ### beep if the mousedown is not in
               `color_picker_window' */

                GUEST<Point> tmpPt;
                tmpPt = evt.where;
                GlobalToLocal(&tmpPt);
                local_pt = tmpPt.get();

                control_p = _FindControl(local_pt, color_picker_window, &c);
                if(control_p)
                {
                    release_part = TrackControl(c, local_pt, (ProcPtr)-1);
                    if(release_part == inButton && c == ok_button)
                        return true;
                    if(release_part == inButton && c == cancel_button)
                        return false;

                    break;
                }

                for(i = 0; i < N_TEXT_BOXES; i++)
                {
                    struct text_box *box;

                    box = &text_boxes[i];
                    if(PtInRect(local_pt, &box->te_rect))
                    {
                        text_box_set_te(box);
                        TEClick(local_pt, ((evt.modifiers & CWC(shiftKey))
                                               ? true
                                               : false),
                                te);
                        break;
                    }
                    else if(PtInRect(local_pt, &box->miniarrow_rect))
                    {
                        int center_y;

                        center_y = (CW(box->miniarrow_rect.top)
                                    + CW(box->miniarrow_rect.bottom))
                            / 2;

                        integer_increment_p = false;
                        track_kount = 0;

                        goto handle_pt_1;

                        while(!GetOSEvent(mUpMask, &evt))
                        {
                            miniarrow_hilite_t _hilite;

                            {
                                GUEST<Point> tmpPt = evt.where;
                                GlobalToLocal(&tmpPt);
                                local_pt = tmpPt.get();
                            }
                        handle_pt_1:
                            if(PtInRect(local_pt, &box->miniarrow_rect))
                            {
                                if(local_pt.v < center_y)
                                    _hilite = miniarrow_up_hilite;
                                else
                                    _hilite = miniarrow_down_hilite;
                            }
                            else
                                _hilite = miniarrow_no_hilite;

                            text_box_miniarrow_update(box, _hilite);
                            miniarrow_track(box, _hilite);
                        }
                        text_box_miniarrow_update(box, miniarrow_no_hilite);
                    }
                }

                if(PtInRect(local_pt, color_wheel_bounds))
                {
                    goto handle_pt_2;

                    while(!GetOSEvent(mUpMask, &evt))
                    {
                        int x, y;
                        double saturation_fp;

                        {
                            GUEST<Point> tmpPt = evt.where;
                            GlobalToLocal(&tmpPt);
                            local_pt = tmpPt.get();
                        }

                    handle_pt_2:
                        x = local_pt.h - color_wheel_center_x;
                        y = local_pt.v - color_wheel_center_y;

                        saturation_fp = sqrt(sqr(x) + sqr(y));
                        if(saturation_fp < (192 / 2))
                        {
                            int hue, saturation;

                            hue = (atan2(y, -x) + M_PI) / M_PI / 2 * 0xFFFF;
                            saturation = saturation_fp / (192 / 2) * 0xFFFF;

                            hue_saturation_update(hue, saturation, true);
                        }
                    }
                }
                break;
            }

            case mouseUp:
                break;

            case updateEvt:
            {
                BeginUpdate(color_picker_window);

                DrawControls(color_picker_window);

                {
                    /* draw bold border around the default button */
                    Rect r = *ok_button_bounds;
                    int oval;

                    InsetRect(&r, -4, -4);
                    PenSize(3, 3);
                    oval = RECT_HEIGHT(&r) / 2 - 4;
                    /* ### frame in the frame color for the `ok_button'
                 control */
                    if(!(ROMlib_options & ROMLIB_RECT_SCREEN_BIT))
                        FrameRoundRect(&r, oval, oval);
                    else
                        FrameRect(&r);
                }

                /* update the text boxes */
                {
                    int i;

                    for(i = 0; i < N_TEXT_BOXES; i++)
                    {
                        struct text_box *box;

                        box = &text_boxes[i];
                        text_box_update(box, false);

                        text_box_miniarrow_update(box, miniarrow_no_hilite);
                    }
                }

                /* draw the compare box */
                compare_box_update();

                /* draw the prompt */
                TextBox((Ptr)&prompt_text[1], *prompt_text,
                        prompt_bounds, teFlushLeft);

                color_wheel_update();

/* compare box labels */
#define orig_compare_box_label ((StringPtr) "\012Original: ")
#define current_compare_box_label ((StringPtr) "\005New: ")

                MoveTo((CW(orig_compare_box_bounds->left)
                        - StringWidth(orig_compare_box_label)),
                       ((CW(orig_compare_box_bounds->top)
                         + CW(orig_compare_box_bounds->bottom))
                            / 2
                        - font_height / 2 + font_ascent));
                DrawString(orig_compare_box_label);

                MoveTo((CW(current_compare_box_bounds->left)
                        - StringWidth(current_compare_box_label)),
                       ((CW(current_compare_box_bounds->top)
                         + CW(current_compare_box_bounds->bottom))
                            / 2
                        - font_height / 2 + font_ascent));
                DrawString(current_compare_box_label);

                EndUpdate(color_picker_window);
                break;
            }

            case keyDown:
            case autoKey:
            {
                char ch;

                ch = CL(evt.message) & 0xFF;
                switch(ch)
                {
                    case '\r':
                    case NUMPAD_ENTER:
                        return true;
                    /* ESC */
                    case '\033':
                        return false;
                    default:
                        if(te_box)
                        {
                            if(ch == '\t')
                                text_box_set_te(&text_boxes[((te_box->i + 5)
                                                             % N_TEXT_BOXES)]);
                            else
                                TEKey(ch, te);
                        }
                }
                break;
            }

            case activateEvt:
            case keyUp:
                break;

            default:
                warning_unexpected("unknown event.what `%d'",
                                   CW(evt.what));
                break;
        }
    }
}

static PixMap color_wheel_pixmap;

static CTabHandle color_wheel_color_table;
static ColorSpec *color_wheel_colors;

static int current_target_x, current_target_y;

static StringPtr label_0_degs = (StringPtr) "\0020\241";
static StringPtr label_90_degs = (StringPtr) "\00390\241";
static StringPtr label_180_degs = (StringPtr) "\004180\241";
static StringPtr label_270_degs = (StringPtr) "\004270\241";

static void
color_wheel_init(void)
{
    int bpp;

    *color_wheel_bounds = *template_color_wheel_bounds;
    OffsetRect(color_wheel_bounds, -StringWidth(label_0_degs), font_height);

    color_wheel_center_y = (CW(color_wheel_bounds->top)
                            + CW(color_wheel_bounds->bottom))
        / 2;
    color_wheel_center_x = (CW(color_wheel_bounds->left)
                            + CW(color_wheel_bounds->right))
        / 2;

    /* the colorwheel appears to be 192 pixels on a side, but it is
     really 208 pixels to handle target overlap */
    InsetRect(color_wheel_bounds, -8, -8);

    bpp = PIXMAP_PIXEL_SIZE(GD_PMAP(MR(MainDevice)));
    color_wheel_pixmap.baseAddr = RM((Ptr)((bpp == 8)
                                               ? color_wheel_bits_8
                                               : (bpp == 4
                                                      ? color_wheel_bits_4
                                                      : (gui_abort(), nullptr))));
    color_wheel_pixmap.rowBytes = CW((bpp == 8)
                                         ? 0x80D0
                                         : (bpp == 4
                                                ? 0x8068
                                                : (gui_abort(), -1)));
    SetRect(&color_wheel_pixmap.bounds, 0, 0, 208, 208);
    pixmap_set_pixel_fields(&color_wheel_pixmap, bpp);
    color_wheel_pixmap.pmTable = RM(no_stdbits_color_conversion_color_table);

    current_target_x = color_wheel_center_x;
    current_target_y = color_wheel_center_y;
}

static void
color_wheel_target_update(bool short_cut_p)
{
    double dist, angle;
    Rect src_rect, dst_rect;

    int x, y;

    /* draw the target at point corresponding to the current
     hue/saturation */

    angle = (double)hue / 0xFFFF * 2 * M_PI;
    dist = (double)saturation / 0xFFFF * 96.0;

    x = color_wheel_center_x + dist * cos(angle);
    y = color_wheel_center_y - dist * sin(angle);

    if(short_cut_p
       && x == current_target_x
       && y == current_target_y)
        return;

    /* erase the current target */
    dst_rect.top = CW(current_target_y - 7);
    dst_rect.left = CW(current_target_x - 7);
    dst_rect.bottom = CW(current_target_y + 9);
    dst_rect.right = CW(current_target_x + 9);

    src_rect = dst_rect;
    OffsetRect(&src_rect, -CW(color_wheel_bounds->left),
               -CW(color_wheel_bounds->top));

    CopyBits((BitMap *)&color_wheel_pixmap, PORT_BITS_FOR_COPY(thePort),
             &src_rect, &dst_rect, srcCopy, NULL);

    PenSize(1, 1);

    ForeColor(whiteColor);
    MoveTo(x - 7, y);
    Line(6, 0);
    MoveTo(x + 1, y);
    Line(6, 0);
    MoveTo(x, y - 7);
    Line(0, 6);
    MoveTo(x, y + 1);
    Line(0, 6);

    ForeColor(blackColor);
    MoveTo(x - 6, y + 1);
    Line(6, 0);
    MoveTo(x + 2, y + 1);
    Line(6, 0);
    MoveTo(x + 1, y - 6);
    Line(0, 6);
    MoveTo(x + 1, y + 2);
    Line(0, 6);

    current_target_x = x;
    current_target_y = y;
}

static void
color_wheel_update(void)
{
    CopyBits((BitMap *)&color_wheel_pixmap, PORT_BITS_FOR_COPY(thePort),
             &color_wheel_pixmap.bounds, color_wheel_bounds, srcCopy, NULL);

    MoveTo(470 - StringWidth(label_0_degs),
           color_wheel_center_y - font_height / 2 + font_ascent);
    DrawString(label_0_degs);

    MoveTo(color_wheel_center_x - StringWidth(label_90_degs) / 2,
           10 + font_ascent);
    DrawString(label_90_degs);

    MoveTo(470 - StringWidth(label_0_degs) - 10
               - 192
               - 10 - StringWidth(label_180_degs),
           color_wheel_center_y - font_height / 2 + font_ascent);
    DrawString(label_180_degs);

    MoveTo(color_wheel_center_x - StringWidth(label_270_degs) / 2,
           10 + font_height + 10 + 192 + 10 + font_ascent);
    DrawString(label_270_degs);

    color_wheel_target_update(false);
}

static int color_wheel_noticed_lightness;

static void
color_wheel_notice_lightness_change(void)
{
    struct
    {
        double angle;
        double dist;
    } color_desc[13] = {
        { 0.0, 0.9375 },
        { 60.0, 0.9375 },
        { 120.0, 0.9375 },
        { 180.0, 0.9375 },
        { 240.0, 0.9375 },
        { 300.0, 0.9375 },

        {
            30.0, 0.625,
        },
        {
            90.0, 0.625,
        },
        {
            150.0, 0.625,
        },
        {
            210.0, 0.625,
        },
        {
            270.0, 0.625,
        },
        {
            330.0, 0.625,
        },

        { 0.0, 0.0 }
    };
    HSLColor hsl_color;
    RGBColor rgb_color;
    int i;

    if(color_wheel_noticed_lightness == lightness)
        return;

    color_wheel_noticed_lightness = lightness;

    for(i = 0; i < (int)NELEM(color_desc); i++)
    {
        /* one half */
        hsl_color.lightness = CW(lightness);
        hsl_color.hue = CW(color_desc[i].angle
                           * 0xFFFF
                           / 360.0);
        hsl_color.saturation = CW(color_desc[i].dist
                                  * 0xFFFF);

        HSL2RGB(&hsl_color, &rgb_color);

        color_wheel_colors[i].rgb = rgb_color;
    }
    AnimatePalette(color_picker_window, color_wheel_color_table,
                   0, 1, 13);
}

/* color picker entry point */

PUBLIC pascal trap BOOLEAN Executor::C_GetColor(Point where, Str255 prompt, RGBColor *in_color, RGBColor *out_color)
{
    PaletteHandle palette;
    bool retval;

    /* compute relevent font information */

    {
        FontInfo font_info;

        GetFontInfo(&font_info);

        font_height = (CW(font_info.ascent)
                       + CW(font_info.descent)
                       + CW(font_info.leading));
        font_ascent = CW(font_info.ascent);

        space_width = CharWidth(' ');
    }

    {
        HSLColor *hsl_color = (HSLColor *)alloca(sizeof *hsl_color);

        red = CW(in_color->red);
        green = CW(in_color->green);
        blue = CW(in_color->blue);

        RGB2HSL(in_color, hsl_color);

        hue = CW(hsl_color->hue);
        saturation = CW(hsl_color->saturation);
        lightness = CW(hsl_color->lightness);

        color_wheel_noticed_lightness = -1;

        orig_color = *in_color;
        current_color = *in_color;
    }

    prompt_text = prompt;

    compute_bounds(where);
    color_wheel_init();

    color_picker_window = _NewCWindow(NULL, color_picker_window_bounds,
                                      /* no title */
                                      NULL,
                                      /* visible */
                                      true,
                                      dBoxProc,
                                      (WindowPtr)-1,
                                      false, /* dummy */ -1);

    /* #### the correct thing to do is have a 24bpp color wheel, and
     draw it to the screen dithered, with a reasonable colormap
     guaranteed by a reasonable palette.
     
     instead, we are going to use an animated palette and a constant
     dither pattern, which won't work on non-clut devices (palette
     manager will probably just eat flaming death)
     
     it also won't work very well on 16bpp devices (which the mac does
     reasonably well)
     
     of course, even the mac doesn't try it on 4bpp or b/w devices */

    palette = NewPalette(16, NULL, pmAnimated | pmExplicit, -1);
    NSetPalette(color_picker_window, palette, pmAllUpdates);

    SetEntryUsage(palette, 0, pmCourteous, -1);
    SetEntryColor(palette, 0, &ROMlib_white_rgb_color);
    SetEntryUsage(palette, 15, pmCourteous, -1);
    SetEntryColor(palette, 15, &ROMlib_black_rgb_color);

    /* sometimes the animated palette entries suck up the hilite color */
    SetEntryUsage(palette, 14, pmTolerant, 1);
    SetEntryColor(palette, 14, &HiliteRGB);

    /* #### add palette entries for the {current, orig}_colors? */

    color_wheel_color_table = (CTabHandle)NewHandle(CTAB_STORAGE_FOR_SIZE(23));
    {
        HLockGuard guard(color_wheel_color_table);

        CTAB_SIZE_X(color_wheel_color_table) = CW(23);
        color_wheel_colors = CTAB_TABLE(color_wheel_color_table);

        color_wheel_notice_lightness_change();

        ActivatePalette(color_picker_window);

        {
            ThePortGuard guard(color_picker_window);
            Rect *dummy_rect = (Rect *)alloca(sizeof *dummy_rect);
            memset(dummy_rect, '\000', sizeof *dummy_rect);

            ok_button = NewControl(color_picker_window,
                                   ok_button_bounds, (StringPtr) "\002OK",
                                   /* visible */
                                   true,
                                   0, 0, 1, pushButProc, -1);
            cancel_button = NewControl(color_picker_window,
                                       cancel_button_bounds,
                                       (StringPtr) "\006Cancel",
                                       /* visible */
                                       true,
                                       0, 0, 1, pushButProc, -1);

            te = TENew(dummy_rect, dummy_rect);
            TESetJust(teFlushRight, te);
            text_box_init();

            /* event loop exits when one of `Cancel' (returning zero) or
	       `OK' (returning one) are clicked */
            retval = event_loop();

            TEDispose(te);
            te_box = NULL;
        }
    }

    DisposHandle((Handle)color_wheel_color_table);

    DisposePalette(palette);

    DisposeWindow(color_picker_window);

    *out_color = retval ? current_color : orig_color;
    return retval;
}
