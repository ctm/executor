/* Copyright 1996 - 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_aboutbox[] = "$Id: aboutbox.c 130 2006-05-10 17:21:35Z ctm $";
#endif

#include "rsys/common.h"

#include "rsys/aboutbox.h"
#include "rsys/mman.h"
#include "rsys/vdriver.h"
#include "rsys/string.h"
#include "rsys/cquick.h"
#include "rsys/quick.h"
#include "rsys/ctl.h"
#include "rsys/version.h"
#include "rsys/licensetext.h"
#include "CQuickDraw.h"
#include "Gestalt.h"
#include "ToolboxEvent.h"
#include "TextEdit.h"
#include "FontMgr.h"
#include "OSUtil.h"
#include <ctype.h>
#include "rsys/file.h"
#include "rsys/osutil.h"
#include "SegmentLdr.h"
#include "rsys/segment.h"
#include "rsys/notmac.h"
#include "rsys/custom.h"
#include "rsys/gestalt.h"
#include "rsys/osevent.h"

#define ABOUT_BOX_WIDTH 500
#define ABOUT_BOX_HEIGHT 300
#define BUTTON_WIDTH 85
#define BUTTON_HEIGHT 20
#define SCROLL_BAR_WIDTH 16
#define TE_MARGIN 4
#define TE_WIDTH (ABOUT_BOX_WIDTH - 20)
#define TE_HEIGHT (ABOUT_BOX_HEIGHT - 106)
#define TE_LEFT ((ABOUT_BOX_WIDTH - TE_WIDTH) / 2)
#define TE_RIGHT (TE_LEFT + TE_WIDTH)
#define TE_TOP 66
#define TE_BOTTOM (TE_TOP + TE_HEIGHT)

#define DONE_BUTTON_NAME "Accept"

#define LICENSE_BUTTON_NAME "License"
#define TIPS_BUTTON_NAME "Tips"

#define COPYRIGHT_STRING_1 "Copyright \251 ARDI 1986-2006"
#define COPYRIGHT_STRING_2 "All rights reserved."

using namespace Executor;

static struct
{
    const char *name;
    const char *text;
    ControlHandle ctl;
} about_box_buttons[] = {
    { LICENSE_BUTTON_NAME, NULL /* generated on the fly from licensetext.c */,
      NULL },
    { "Maker",
      "ARDI\r"
      "World Wide Web: <http://www.ardi.com>\r"
      "FTP: <ftp://ftp.ardi.com/pub>\r",
      NULL },

    { "Credits",
      "Bill Goldman \321 Browser, Testing\r"
      "Mat Hostetter \321 Syn68k, Low Level Graphics, DOS port, more...\r"

#if defined(MSDOS)
      "Joel Hunter \321 Low Level DOS Sound\r"
#endif

#if defined(CYGWIN32)
      "Sam Lantinga \321 Win32 port\r"
#endif

      "Patrick LoPresti \321 High Level Sound, Low Level Linux Sound\r"
      "Cliff Matthews \321 this credit list (and most things not listed)\r"
      "Cotton Seed \321 High Level Graphics, Apple Events, more...\r"

#if defined(MSDOS) || defined(CYGWIN32)
      "Lauri Pesonen \321 Low Level Win32 CD-ROM access (Executor 2.1)\r"
#endif

#if defined(MSDOS)
      "Samuel Vincent \321 Low Level DOS Serial Port Support\r"
#endif

      "and all the engineers and testers who helped us build version 1.x\r"
      "\r"
      "Windows Appearance:\r"
      "\r"
      "The windows appearance option uses \"Jim's CDEFs\" copyright "
      "Jim Stout and the "
      "\"Infinity Windoid\" copyright Troy Gaul.\r"
      "\r"
      "Primary Pre-Beta Testers:\r"
      "\r"
      "Jon Abbott \321 Testing, Icon Design\r"
      "Ziv Arazi \321 Testing\r"
      "Edmund Ronald \321 Advice, Testing\r"
      "K. Harrison Liang \321 Testing\r"
      "Hugh Mclenaghan \321 Testing\r"
      "Emilio Moreno \321 Testing, Spanish Translation + Keyboard, Icon Design\r"
      "Ernst Oud \321 Documentation, Testing\r"
      "\r"
      "Some of the best development tools are Free and written by:\r"
      "\r"
      "The Free Software Foundation \321 the gcc compiler, more ...\r"
      "Cygnus Support \321 the gdb debugger, the gnats bug tracking software\r"
      "Linus Torvalds et al. \321 Linux, our favorite OS"
      "\r"
#if defined(MSDOS)
      "Executor/DOS was ported via DJGPP, DJ Delorie's port of gcc.\r"
      "DJGPP's primary authors are:\r"
      "\r"
      "DJ Delorie \321 Ringleader\r"
      "William Metzenthen \321 80387 emulator\r"
      "Charles Sandmann \321 cwsdpmi, more...\r"
      "Morten Welinder \321 misc.\r"
      "Eli Zaretskii \321 Some DOS-related library functions\r"
      "\rThis product includes software developed by "
      "the University of California, Berkeley and its contributors\r"
#endif
      ,
      NULL },
    { TIPS_BUTTON_NAME,
      "Don't delete tips.txt.  If you do, that will be your only tip.\r", NULL },
    { DONE_BUTTON_NAME, "Internal error!", NULL }
};

static WindowPtr about_box;
static ControlHandle about_scrollbar;
static TEHandle about_te;
static ProcPtr scroll_bar_callback;

enum
{
    THROTTLE_TICKS = 4
};

PRIVATE bool
enough_time_has_passed(void)
{
    static ULONGINT old_ticks;
    ULONGINT new_ticks;
    bool retval;

    new_ticks = TickCount();
    retval = new_ticks - old_ticks >= THROTTLE_TICKS;
    if(retval)
        old_ticks = new_ticks;
    return retval;
}

/* Menu name for the about box. */
StringPtr Executor::about_box_menu_name_pstr = (StringPtr) "\022\000About Executor...";

static void
help_scroll(ControlHandle c, INTEGER part)
{
    if(enough_time_has_passed())
    {
        int page_size, old_value, new_value, delta;
        int line_height;

        old_value = GetCtlValue(c);
        line_height = TE_LINE_HEIGHT(about_te);
        page_size = RECT_HEIGHT(&CTL_RECT(c)) / line_height;

        switch(part)
        {
            case inUpButton:
                SetCtlValue(c, old_value - 1);
                break;
            case inDownButton:
                SetCtlValue(c, old_value + 1);
                break;
            case inPageUp:
                SetCtlValue(c, old_value - page_size);
                break;
            case inPageDown:
                SetCtlValue(c, old_value + page_size);
                break;
            default:
                break;
        }

        new_value = GetCtlValue(about_scrollbar);
        delta = new_value - old_value;
        if(delta != 0)
            TEScroll(0, -delta * line_height, about_te);
    }
}

static syn68k_addr_t
scroll_stub(syn68k_addr_t junk, void *junk2)
{
    syn68k_addr_t retaddr;
    ControlHandle ctl;
    INTEGER part;

    retaddr = POPADDR();
    part = POPUW();
    ctl = (ControlHandle)SYN68K_TO_US(POPUL());
    help_scroll(ctl, part);
    return retaddr;
}

static int
find_button(const char *button_name)
{
    int i;

    for(i = 0; i < (int)NELEM(about_box_buttons); i++)
        if(!strcmp(about_box_buttons[i].name, button_name))
            break;
    gui_assert(i < (int)NELEM(about_box_buttons));
    return i;
}

/* Returns the index of the button with name LICENSE_BUTTON_NAME. */
static int
find_license_button(void)
{
    int retval;

    retval = find_button(LICENSE_BUTTON_NAME);
    return retval;
}

static void
create_license_text(void)
{
    int b;

    b = find_license_button();
    if(about_box_buttons[b].text == NULL)
    {
        long new_size;
        char *license_text, *p, *q;
        const char *licensep;

        new_size = 0;

        /* Compute the length of the license string. */
        for(licensep = ROMlib_licensep ? (const char *)ROMlib_licensep->chars : "";
            *licensep;)
        {
            int title_len;
            int body_len;

            title_len = strlen(licensep);
            licensep += title_len + 1;
            body_len = strlen(licensep);
            licensep += body_len + 1;
            new_size += title_len + body_len + 5;
        }

        /* Allocate and construct the license string. */
        license_text = (char *)NewPtrSys(new_size + 1);
        license_text[0] = '\0';

        p = license_text;

        for(licensep = ROMlib_licensep ? (char *)ROMlib_licensep->chars : "";
            *licensep;)
        {
            const char *titlep;
            const char *bodyp;

            titlep = licensep;
            licensep += strlen(licensep) + 1;
            bodyp = licensep;
            licensep += strlen(licensep) + 1;
            p += sprintf(p, "%s\r\r%s\r\r\r", titlep, bodyp);
        }

        /* Nuke any trailing whitespace, and end with exactly one linefeed */
        for(q = p - 1; q >= license_text && isspace(*q); q--)
            *q = '\0';
        if(q >= license_text)
            strcpy(q + 1, "\r");

        about_box_buttons[b].text = license_text;
    }
}

/*
 * get an upper bound on the number of tips that may be in the buffer
 */

static int
approximate_tips(char *p)
{
    int retval;

    retval = 0;
    while(*p)
    {
        ++retval;
        while(*p && *p != '\n')
            ++p;
        while(*p && *p == '\n')
            ++p;
    }
    return retval;
}

typedef struct
{
    int tip_offset;
    int tip_length;
} tip_t;

static int
find_tips(tip_t *tips, const char *p)
{
    const char *orig_p;
    int retval;

    retval = 0;

    orig_p = p;
    while(*p)
    {
        /* suck up any excess leading \n */
        while(*p && *p == '\n')
            ++p;

        if(*p)
        {
            tips->tip_offset = p - orig_p;
            while(*p && (*p != '\n' || (p[1] && p[1] != '\n')))
                ++p;
            tips->tip_length = p - orig_p - tips->tip_offset;
            ++retval;
            ++tips;
        }
    }

    return retval;
}

static void
add_to_str(char **pp, const char *ip, int n)
{
    int i;

    for(i = 0; i < n; ++i)
        (*pp)[i] = ip[i] == '\n' ? ' ' : ip[i];
    *pp += n;
}

static char *
parse_and_randomize_tips(char *buf)
{
    int n_tips, chars_needed;
    tip_t *tips;
    bool seen_tip;
    char *retval;
    char *p;
    int i;

    n_tips = approximate_tips(buf);
    tips = (tip_t *)alloca(n_tips * sizeof *tips);
    n_tips = find_tips(tips, buf);

    chars_needed = 0;
    for(i = 0; i < n_tips; ++i)
        chars_needed += tips[i].tip_length;
    chars_needed += 2 * (n_tips - 1) + 1;

    retval = (char *)malloc(chars_needed);
    seen_tip = false;
    p = retval;
    while(n_tips)
    {
        if(seen_tip)
            add_to_str(&p, "\r\r", 2);
        else
            seen_tip = true;
        i = rand() % n_tips;
        add_to_str(&p, buf + tips[i].tip_offset, tips[i].tip_length);
        tips[i] = tips[n_tips - 1];
        --n_tips;
    }
    *p = 0;
    return retval;
}

static char *orig_text;

static void
create_tips_text(void)
{
    char *filename;
    FILE *fp;
    int b;

    b = find_button(TIPS_BUTTON_NAME);
    if(!orig_text)
    {
        srand(Ticks.get()); /* NOTE: we don't care that rand/srand is not a
			   particular good way to generate random numbers */
        orig_text = (char *)about_box_buttons[b].text;
    }
    if(about_box_buttons[b].text == orig_text)
    {
#if !defined(LINUX)
        filename = copystr("+/tips.txt");
#else
        filename = copystr("/opt/executor/tips.txt");
#endif
        fp = fopen(filename, "r");
        free(filename);
        if(fp)
        {
            char tempbuf[32768];
            size_t nread;

            nread = fread(tempbuf, 1, sizeof tempbuf - 1, fp);
            tempbuf[nread] = 0;
            if(nread > 0)
                about_box_buttons[b].text = parse_and_randomize_tips(tempbuf);
            fclose(fp);
        }
    }
}

static void
dispose_license_text(void)
{
    int b;

    b = find_license_button();
    if(about_box_buttons[b].text != NULL)
    {
        DisposPtr((Ptr)about_box_buttons[b].text);
        about_box_buttons[b].text = NULL;
    }
}

static void
dispose_tips_text(void)
{
    int b;

    b = find_button(TIPS_BUTTON_NAME);
    if(about_box_buttons[b].text != orig_text)
    {
        free((void *)about_box_buttons[b].text);
        about_box_buttons[b].text = orig_text;
    }
}

static void
create_about_box()
{
    static Rect scroll_bar_bounds = {
        CWC(TE_TOP),
        CWC(TE_RIGHT - SCROLL_BAR_WIDTH),
        CWC(TE_BOTTOM),
        CWC(TE_RIGHT)
    };
    static Rect te_bounds = {
        CWC(TE_TOP + 1),
        CWC(TE_LEFT + TE_MARGIN),
        CWC(TE_BOTTOM - 1),
        CWC(TE_RIGHT - TE_MARGIN - SCROLL_BAR_WIDTH)
    };
    Rect about_box_bounds;
    int b;

    create_license_text();
    create_tips_text();

    SetRect(&about_box_bounds,
            (vdriver_width - ABOUT_BOX_WIDTH) / 2U,
            (vdriver_height - ABOUT_BOX_HEIGHT) / 3U + 15,
            (vdriver_width + ABOUT_BOX_WIDTH) / 2U,
            (vdriver_height + 2 * ABOUT_BOX_HEIGHT) / 3U + 15);

    /* Create the window. */
    about_box = (WindowPtr)NewCWindow(NULL, &about_box_bounds,
                                      (StringPtr) "\016About Executor",
                                      false, dBoxProc, (CWindowPtr)-1,
                                      true, /* go away flag */
                                      -5 /* unused */);
    ThePortGuard guard(about_box);
    /* Create the push buttons. */
    for(b = 0; b < (int)NELEM(about_box_buttons); b++)
    {
        Str255 str;
        Rect r;

        /* Set up the rectangle enclosing each button. */
        r.top = CWC(ABOUT_BOX_HEIGHT - 30);
        r.bottom = CWC(ABOUT_BOX_HEIGHT - 30 + BUTTON_HEIGHT);
        r.left = CW((b * ABOUT_BOX_WIDTH / NELEM(about_box_buttons))
                    + (ABOUT_BOX_WIDTH / NELEM(about_box_buttons)
                       - BUTTON_WIDTH)
                        / 2);
        r.right = CW(CW(r.left) + BUTTON_WIDTH);

        str255_from_c_string(str, about_box_buttons[b].name);
        about_box_buttons[b].ctl = NewControl(about_box, &r, str, true, 0,
                                              0, 1, pushButProc, b);
    }

    about_scrollbar = NewControl(about_box, &scroll_bar_bounds, NULL, true,
                                 0, 0, 100, scrollBarProc, -1);
    about_te = TENew(&te_bounds, &te_bounds);
    TESetJust(teFlushLeft, about_te);
}

/* Closes about box and frees up memory taken by it. */
static void
dispose_about_box(void)
{
    C_DisposeWindow(about_box);
    about_box = NULL;
    about_scrollbar = NULL;
    dispose_license_text();
    dispose_tips_text();
}

/* Sets the text currently being displayed. */
static void
set_text(const char *text)
{
    TESetText((Ptr)text, strlen(text), about_te);
    SetCtlMax(about_scrollbar,
              MAX(0, (TE_N_LINES(about_te)
                      - ((TE_HEIGHT - 2) / TE_LINE_HEIGHT(about_te)))));
    SetCtlValue(about_scrollbar, 0);
    TE_DEST_RECT(about_te) = TE_VIEW_RECT(about_te);
    InvalRect(&TE_VIEW_RECT(about_te));
}

/* Specifies which button is currently pressed. */
static void
set_current_button(int button)
{
    int i;
    set_text(about_box_buttons[button].text);
    for(i = 0; i < (int)NELEM(about_box_buttons); i++)
        HiliteControl(about_box_buttons[i].ctl, (i == button) ? 255 : 0);
}

static void
draw_mem_string(char *s, int y)
{
    MoveTo((ABOUT_BOX_WIDTH - 9 - TextWidth((Ptr)s, 0, strlen(s))), y);
    DrawText_c_string(s);
}

static void
draw_status_info(bool executor_p)
{
    char total_ram_string[128];
    char applzone_ram_string[128];
    char syszone_ram_string[128];
    bool gestalt_success_p;
    LONGINT total_ram;
    GUEST<LONGINT> total_ram_x;
    const char *ram_tag, *system_ram_tag, *application_ram_tag;

    if(executor_p)
    {
        ram_tag = "Emulated RAM: ";
        system_ram_tag = "System RAM free: ";
        application_ram_tag = "Application RAM free: ";
    }
    else
    {
        ram_tag = "";
        system_ram_tag = "";
        application_ram_tag = "";
    }

/* Compute a string for total RAM. */
#define MB (1024 * 1024U)
    gestalt_success_p = (C_GestaltTablesOnly(gestaltLogicalRAMSize, &total_ram_x)
                         == noErr);
    total_ram = CL(total_ram_x);
    if(gestalt_success_p)
        sprintf(total_ram_string, "%s%u.%02u MB", ram_tag,
                total_ram / MB, (total_ram % MB) * 100 / MB);
    else
        sprintf(total_ram_string, "%s??? MB", ram_tag);

    /* Compute a string for ApplZone RAM. */
    {
        TheZoneGuard guard(ApplZone);
        sprintf(applzone_ram_string, "%s%lu KB / %u KB", application_ram_tag,
                FreeMem() / 1024UL, ROMlib_applzone_size / 1024U);
    }

    /* Compute a string for SysZone RAM. */
    sprintf(syszone_ram_string, "%s%lu KB / %u KB", system_ram_tag,
            FreeMemSys() / 1024UL, ROMlib_syszone_size / 1024U);

    draw_mem_string(total_ram_string, 20);
    draw_mem_string(syszone_ram_string, 36);
    draw_mem_string(applzone_ram_string, 52);
}

static void
event_loop(bool executor_p)
{
    static Rect frame_rect = {
        CWC(TE_TOP),
        CWC(TE_LEFT),
        CWC(TE_BOTTOM),
        CWC(TE_RIGHT - SCROLL_BAR_WIDTH + 1)
    };
    EventRecord evt;
    bool done_p;
    int which_text;
    Rect te_dest_rect;
    int old_scroll_bar_value;

    /* Make sure our drawing mode is sane. */
    ForeColor(blackColor);
    BackColor(whiteColor);
    PenNormal();

    te_dest_rect = TE_DEST_RECT(about_te);

    set_current_button(0);
    which_text = 0;
    InvalRect(&about_box->portRect);

    old_scroll_bar_value = GetCtlValue(about_scrollbar);

    for(done_p = false; !done_p;)
    {
        GetNextEvent((mDownMask | mUpMask
                      | keyDownMask | keyUpMask | autoKeyMask
                      | updateMask | activMask),
                     &evt);

        TEIdle(about_te);

        switch(CW(evt.what))
        {
            case updateEvt:
                BeginUpdate(about_box);
                PenNormal();
                ForeColor(blackColor);
                BackColor(whiteColor);
                EraseRect(&about_box->portRect);
                TextFont(helvetica);
                TextSize(24);
                MoveTo(TE_LEFT, 30);
                if(executor_p)
                    DrawText_c_string(ROMlib_executor_full_name);
                else
                    DrawText_c_string("Carbonless Copies Runtime System");
                TextSize(12);
                MoveTo(TE_LEFT, 49);
                DrawText_c_string(COPYRIGHT_STRING_1);
                MoveTo(TE_LEFT, 62);
                DrawText_c_string(COPYRIGHT_STRING_2);
                draw_status_info(executor_p);
                FrameRect(&frame_rect);
                TEUpdate(&te_dest_rect, about_te);
                DrawControls(about_box);
                EndUpdate(about_box);
                break;

            case keyDown:
            case autoKey:
            {
                char ch;

                ch = CL(evt.message) & 0xFF;
                switch(ch)
                {
                    case '\r':
                    case NUMPAD_ENTER:
                        done_p = true;
                        break;
                    default:
                        TEKey(ch, about_te);
                        break;
                }
            }
            break;

#define _FindControl(arg0, arg1, arg2)             \
    ({                                             \
        int16 retval;                              \
        GUEST<ControlHandle> bogo_c;               \
                                                   \
        retval = FindControl(arg0, arg1, &bogo_c); \
        *(arg2) = MR(bogo_c);                      \
                                                   \
        retval;                                    \
    })

            case mouseDown:
            {
                Point local_pt;
                bool control_p;
                ControlHandle c;

                GUEST<Point> tmpPt = evt.where;
                GlobalToLocal(&tmpPt);
                local_pt = tmpPt.get();

                control_p = _FindControl(local_pt, about_box, &c);
                if(!control_p)
                    SysBeep(1);
                else
                {
                    if(c == about_scrollbar)
                    {
                        int new_val, delta;
                        INTEGER part;

                        part = TestControl(c, local_pt);

                        if(TrackControl(c, local_pt,
                                        (part == inThumb
                                             ? (ProcPtr)-1
                                             : scroll_bar_callback))
                           == inThumb)
                        {
                            new_val = GetCtlValue(about_scrollbar);
                            delta = new_val - old_scroll_bar_value;
                            if(delta != 0)
                            {
                                TEScroll(0, -delta * TE_LINE_HEIGHT(about_te),
                                         about_te);
                            }
                        }

                        old_scroll_bar_value = GetCtlValue(about_scrollbar);
                    }
                    else if(TrackControl(c, local_pt, (ProcPtr)-1)
                            == inButton)
                    {
                        int new_text;

                        new_text = CTL_REF_CON(c);
                        if(new_text != which_text)
                        {
                            if(!strcmp(about_box_buttons[new_text].name,
                                       DONE_BUTTON_NAME))
                                done_p = true;
                            else
                            {
                                set_current_button(new_text);
                                which_text = new_text;
                            }
                        }
                    }
                }
            }
            break;

            case activateEvt:
            case mouseUp:
            case keyUp:
                break;
        }
    }
}

void Executor::do_about_box(void)
{
    static bool busy_p = false;

    if(!busy_p)
    {
        busy_p = true; /* Only allow one about box at a time. */

        if(scroll_bar_callback == 0)
            scroll_bar_callback = (ProcPtr)SYN68K_TO_US(callback_install(scroll_stub, NULL));

        {
            TheZoneGuard guard(SysZone);
            create_about_box();

            {
                ThePortGuard portGuard(about_box);
                C_ShowWindow(about_box);
                event_loop(strncasecmp(ROMlib_appname, EXECUTOR_NAME,
                                       sizeof EXECUTOR_NAME - 1)
                           == 0);
            }

            TEDispose(about_te);
            about_te = NULL;
            dispose_about_box();
        }

        busy_p = false;
    }
}
