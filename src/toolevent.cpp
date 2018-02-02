/* Copyright 1987 - 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ToolboxEvent.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "EventMgr.h"
#include "WindowMgr.h"
#include "OSUtil.h"
#include "ToolboxEvent.h"
#include "ToolboxUtil.h"
#include "FileMgr.h"
#include "MemoryMgr.h"
#include "DeskMgr.h"
#include "SysErr.h"
#include "BinaryDecimal.h"
#include "SegmentLdr.h"

#include "rsys/cquick.h"
#include "rsys/hfs.h"
#include "rsys/resource.h"
#include "rsys/notmac.h"
#include "rsys/stdfile.h"
#include "rsys/prefs.h"
#include "rsys/options.h"
#include "rsys/prefpanel.h"
#include "rsys/aboutpanel.h"
#include "rsys/sounddriver.h"
#include "rsys/segment.h"
#include "rsys/version.h"
#include "rsys/syncint.h"
#include "rsys/vbl.h"
#include "rsys/osutil.h"
#include "rsys/osevent.h"
#include "rsys/blockinterrupts.h"
#include "rsys/keyboard.h"
#include "rsys/parse.h"
#include "rsys/refresh.h"
#include "rsys/parseopt.h"
#include "rsys/vdriver.h"
#include "rsys/aboutbox.h"
#include "rsys/redrawscreen.h"
#include "rsys/toolevent.h"
#include "rsys/nextprint.h"
#include "rsys/scrap.h"

#if !defined(WIN32)
#include <sys/socket.h>
#endif

using namespace Executor;

/* #define	EVENTTRACE */

#if defined(EVENTTRACE)
LONGINT Executor::eventstate = 0;
#define TRACE(n) (eventstate = (n))
#else /* !defined(EVENTTRACE) */
#define TRACE(n)
#endif /* !defined(EVENTTRACE) */

static BOOLEAN ROMlib_alarmonmbar = false;

int Executor::ROMlib_delay = 0; /* number of ticks to wait when we
										 * haven't gotten anything interesting */

#define ALARMSICN -16385

BOOLEAN Executor::ROMlib_beepedonce = false;

static void ROMlib_togglealarm()
{
    Handle alarmh;
    static const unsigned char hard_coded_alarm[] = {
        0xFB, 0xBE, /* XXXXX XXX XXXXX  */
        0x84, 0x42, /* X    X   X    X  */
        0x89, 0x22, /* X   X  X  X   X  */
        0x91, 0x12, /* X  X   X   X  X  */
        0xA1, 0x0A, /* X X    X    X X  */
        0x41, 0x04, /*  X     X     X   */
        0x81, 0x02, /* X      X      X  */
        0x81, 0x02, /* X      X      X  */
        0x80, 0x82, /* X       X     X  */
        0x40, 0x44, /*  X       X   X   */
        0xA0, 0x0A, /* X X         X X  */
        0x90, 0x12, /* X  X       X  X  */
        0x88, 0x22, /* X   X     X   X  */
        0x84, 0x42, /* X    X   X    X  */
        0xFB, 0xBE, /* XXXXX XXX XXXXX  */
        0x00, 0x00, /*                  */
    };

    /* rectangle of alarm on the screen */
    static Rect screen_alarm_rect = { CWC(2), CWC(16), CWC(18), CWC(32) };

    BitMap src_alarm_bitmap = {
        /* the baseAddr field of the src_alarm bitmap will either be
	   the hard coded alarm, or the resource 'sicn' */
        nullptr,
        CWC(2),
        { CWC(0), CWC(0), CWC(16), CWC(16) }
    };

    static INTEGER alarm_bits[16];
    BitMap save_alarm_bitmap = {
        RM((Ptr)&alarm_bits[0]),
        CWC(2),
        { CWC(0), CWC(0), CWC(16), CWC(16) }
    };

    if(ROMlib_alarmonmbar)
    {
        CopyBits(&save_alarm_bitmap, (BitMap *)STARH(GD_PMAP(MR(LM(TheGDevice)))),
                 &save_alarm_bitmap.bounds, &screen_alarm_rect,
                 srcCopy, NULL);
        ROMlib_alarmonmbar = false;
    }
    else
    {
        if(ROMlib_shouldalarm())
        {
            if(!ROMlib_beepedonce)
            {
                SysBeep(5);
                ROMlib_beepedonce = true;
            }
            /* save the bits underneath */
            /* draw sicon -16385 up there */
            if((alarmh = ROMlib_getrestid(TICK("SICN"), ALARMSICN)))
                src_alarm_bitmap.baseAddr = *alarmh;
            else
                /* once again, we need to move the (Ptr) cast
		 inside the CL () because it confuses gcc */
                src_alarm_bitmap.baseAddr = RM((Ptr)hard_coded_alarm);

            /* save the screen to save_alarm_bitmap */
            CopyBits((BitMap *)STARH(GD_PMAP(MR(LM(TheGDevice)))), &save_alarm_bitmap,
                     &screen_alarm_rect, &save_alarm_bitmap.bounds,
                     srcCopy, NULL);
            /* and copy the new alarm to the screen */
            CopyBits(&src_alarm_bitmap, (BitMap *)STARH(GD_PMAP(MR(LM(TheGDevice)))),
                     &src_alarm_bitmap.bounds, &screen_alarm_rect,
                     srcCopy, NULL);
            ROMlib_alarmonmbar = true;
        }
    }
}

void Executor::ROMlib_alarmoffmbar()
{
    if(ROMlib_alarmonmbar)
        ROMlib_togglealarm();
}

LONGINT Executor::C_KeyTrans(Ptr mapp, unsigned short code, LONGINT *state)
{
    LONGINT ascii;
    int table_num;
    unsigned char virt;
    kchr_ptr_t p;
    int table_num_index;

    p = (kchr_ptr_t)mapp;
    virt = code & VIRT_MASK;

    table_num_index = (code >> MODIFIER_SHIFT) & MODIFIER_MASK;
    table_num = CB((KCHR_MODIFIER_TABLE_X(p))[table_num_index]);
    ascii = (unsigned char)CB(KCHR_TABLE_X(p)[table_num][virt]);

    if(*state == 0)
    {
        if(ascii)
            *state = 0;
        else
        {
            int n_dead;
            dead_key_rec_t *deadp;

            n_dead = KCHR_N_DEAD_KEY_RECS(p);
            deadp = KCHR_DEAD_KEY_RECS_X(p);
            while(--n_dead >= 0
                  && (DEAD_KEY_TABLE_NUMBER(deadp) != table_num
                      || DEAD_KEY_VIRT_KEY(deadp) != virt))
                deadp = (dead_key_rec_t *)(&DEAD_KEY_NO_MATCH_X(deadp) + 1);
            if(n_dead >= 0)
                *state = (char *)deadp - (char *)&KCHR_N_TABLES_X(p);
            else
                *state = 0;
        }
    }
    else
    {
        dead_key_rec_t *deadp;
        completer_t *completep;
        int n_recs, i;

        deadp = (dead_key_rec_t *)((char *)&KCHR_N_TABLES_X(p) + *state);
        *state = 0;
        completep = &DEAD_KEY_COMPLETER_X(deadp);
        n_recs = COMPLETER_N_RECS(completep);
        for(i = 0;
            (i < n_recs
             && (CB((COMPLETER_COMPLETER_RECS_X(completep))[i].to_look_for)
                 != ascii));
            ++i)
            ;
        if(i < n_recs)
            ascii = (unsigned char)
                CB((COMPLETER_COMPLETER_RECS_X(completep)[i]).replacement);
        else
            ascii = (ascii << 16) | (unsigned short)DEAD_KEY_NO_MATCH(deadp);
    }
    return ascii;
}

void Executor::ROMlib_circledefault(DialogPtr dp)
{
    GUEST<INTEGER> type;
    GUEST<Handle> h;
    Rect r;
    GrafPtr saveport;

    saveport = thePort;
    SetPort(dp);
    GetDItem(dp, 1, &type, &h, &r);
    PenSize(3, 3);
    InsetRect(&r, -4, -4);
    if(!(ROMlib_options & ROMLIB_RECT_SCREEN_BIT))
        FrameRoundRect(&r, 16, 16);
    else
        FrameRect(&r);
    PenSize(1, 1);
    SetPort(saveport);
}

void Executor::dofloppymount(void)
{
#if !defined(MSDOS) && !defined(LINUX) && !defined(CYGWIN32)
    SysBeep(5);
#else
    futzwithdosdisks();
#endif
}

static void doscreendumptoprinter(void)
{
    SysBeep(5);
}

typedef enum { SETSTATE,
               CLEARSTATE,
               FLIPSTATE } modstate_t;

void modstate(DialogPtr dp, INTEGER tomod, modstate_t mod)
{
    GUEST<INTEGER> type;
    INTEGER newvalue;
    GUEST<Handle> ch_s;
    ControlHandle ch;
    Rect r;

    GetDItem(dp, tomod, &type, &ch_s, &r);
    ch = (ControlHandle)MR(ch_s);
    if(type & CWC(ctrlItem))
    {
        switch(mod)
        {
            case SETSTATE:
                newvalue = 1;
                break;
            case CLEARSTATE:
                newvalue = 0;
                break;
            case FLIPSTATE:
                newvalue = GetCtlValue(ch) ? 0 : 1;
                break;
#if !defined(LETGCCWAIL)
            default:
                newvalue = 0;
                break;
#endif /* !defined(LETGCCWAIL) */
        }
        if(type & CWC(itemDisable))
        {
            SetCtlValue(ch, 0);
            HiliteControl(ch, 255);
        }
        else
            SetCtlValue(ch, newvalue);
    }
}

INTEGER getvalue(DialogPtr dp, INTEGER toget)
{
    GUEST<INTEGER> type;
    GUEST<ControlHandle> ch;
    Rect r;

    GetDItem(dp, toget, &type, (GUEST<Handle> *)&ch, &r);
    return (type & CWC(ctrlItem)) ? GetCtlValue(MR(ch)) : 0;
}

typedef struct depth
{
    int item;
    int bpp;
} depth_t;

static depth_t depths_list[] = {
    { PREF_COLORS_2, 1 },
    { PREF_COLORS_4, 2 },
    { PREF_COLORS_16, 4 },
    { PREF_COLORS_256, 8 },
    { PREF_COLORS_THOUSANDS, 16 },
    { PREF_COLORS_MILLIONS, 32 },
};

static int current_depth_item;

/* NOTE: Illustrator patches out the Palette Manager, which uses a dispatch
   table.  Our current stubify.h doesn't deal gracefully with the case of
   patched out traps that use dispatch tables, so we use C_SetDepth and
   C_HasDepth below. */

static void
set_depth(DialogPtr dp, int16_t item_to_set)
{
    int i;

    modstate(dp, current_depth_item, CLEARSTATE);

    for(i = 0; i < (int)NELEM(depths_list); i++)
    {
        depth_t *depth = &depths_list[i];

        if(item_to_set == depth->item)
            C_SetDepth(MR(LM(MainDevice)), depth->bpp, 0, 0);
    }

    current_depth_item = item_to_set;
    modstate(dp, current_depth_item, SETSTATE);
}

void setoneofthree(DialogPtr dp, INTEGER toset, INTEGER item1, INTEGER item2,
                   INTEGER item3)
{
    if(toset != item1)
        modstate(dp, item1, CLEARSTATE);
    if(toset != item2)
        modstate(dp, item2, CLEARSTATE);
    if(toset != item3)
        modstate(dp, item3, CLEARSTATE);
    modstate(dp, toset, SETSTATE);
}

void setedittext(DialogPtr dp, INTEGER itemno, StringPtr str)
{
    GUEST<INTEGER> type;
    Rect r;
    GUEST<Handle> h;

    GetDItem(dp, itemno, &type, &h, &r);
    SetIText(MR(h), str);
}

void setedittextnum(DialogPtr dp, INTEGER itemno, INTEGER value)
{
    Str255 str;

    NumToString(value, str);
    setedittext(dp, itemno, str);
}

void setedittextcstr(DialogPtr dp, INTEGER itemno, char *strp)
{
    int len;
    Str255 str;

    if(strp)
        len = strlen(strp);
    else
        len = 0;
    if(len > 255)
        len = 255;
    str[0] = len;
    memcpy(str + 1, strp, len);
    setedittext(dp, itemno, str);
}

INTEGER getedittext(DialogPtr dp, INTEGER itemno)
{
    Str255 str;
    GUEST<Handle> h_s;
    Handle h;
    GUEST<INTEGER> type;
    Rect r;
    LONGINT l;

    GetDItem(dp, itemno, &type, &h_s, &r);
    h = MR(h_s);
    GetIText(h, str);
    StringToNum(str, &l);
    return (INTEGER)l;
}

#define C_STRING_FROM_SYSTEM_VERSION()        \
    ({                                        \
        char *retval;                         \
        retval = (char *)alloca(9);           \
        sprintf(retval, "%d.%d.%d",           \
                (system_version >> 8) & 0xF,  \
                (system_version >> 4) & 0xF,  \
                (system_version >> 0) & 0xF); \
        retval;                               \
    })

void setupprefvalues(DialogPtr dp)
{
    INTEGER toset;

    setedittextnum(dp, PREFREFRESHITEM, ROMlib_refresh);
    setedittextcstr(dp, PREF_COMMENTS, ROMlib_Comments);

    switch(ROMlib_when)
    {
        case WriteAlways:
        case WriteInBltrgn:
            toset = PREFANIMATIONITEM;
            break;
#if !defined(LETGCCWAIL)
        default:
#endif /* !defined(LETGCCWAIL) */
        case WriteNever:
        case WriteInOSEvent:
            toset = PREFNORMALITEM;
            break;
        case WriteAtEndOfTrap:
            toset = PREFINBETWEENITEM;
            break;
    }
    setoneofthree(dp, toset, PREFNORMALITEM, PREFINBETWEENITEM,
                  PREFANIMATIONITEM);
    modstate(dp, PREFNOCLOCKITEM, ROMlib_clock != 2 ? SETSTATE : CLEARSTATE);
    modstate(dp, PREFNO32BITWARNINGSITEM,
             ROMlib_nowarn32 ? SETSTATE : CLEARSTATE);
    modstate(dp, PREFFLUSHCACHEITEM,
             ROMlib_flushoften ? SETSTATE : CLEARSTATE);
    switch(ROMlib_PretendSound)
    {
#if !defined(LETGCCWAIL)
        default:
#endif /* !defined(LETGCCWAIL) */
        case soundoff:
            toset = PREFSOUNDOFFITEM;
            break;
        case soundpretend:
            toset = PREFSOUNDPRETENDITEM;
            break;
        case soundon:
            toset = PREFSOUNDONITEM;
            break;
    }
    setoneofthree(dp, toset, PREFSOUNDOFFITEM, PREFSOUNDPRETENDITEM,
                  PREFSOUNDONITEM);
    modstate(dp, PREFPASSPOSTSCRIPTITEM,
             ROMlib_passpostscript ? SETSTATE : CLEARSTATE);
    modstate(dp, PREFNEWLINEMAPPINGITEM,
             ROMlib_newlinetocr ? SETSTATE : CLEARSTATE);
    modstate(dp, PREFDIRECTDISKITEM,
             ROMlib_directdiskaccess ? SETSTATE : CLEARSTATE);
    modstate(dp, PREFFONTSUBSTITUTIONITEM,
             ROMlib_fontsubstitution ? SETSTATE : CLEARSTATE);
    modstate(dp, PREFCACHEHEURISTICSITEM,
             ROMlib_cacheheuristic ? SETSTATE : CLEARSTATE);

    modstate(dp, PREF_PRETEND_HELP,
             ROMlib_pretend_help ? SETSTATE : CLEARSTATE);
    modstate(dp, PREF_PRETEND_EDITION,
             ROMlib_pretend_edition ? SETSTATE : CLEARSTATE);
    modstate(dp, PREF_PRETEND_SCRIPT,
             ROMlib_pretend_script ? SETSTATE : CLEARSTATE);
    modstate(dp, PREF_PRETEND_ALIAS,
             ROMlib_pretend_alias ? SETSTATE : CLEARSTATE);
    {
        int bpp, i;

        bpp = PIXMAP_PIXEL_SIZE(GD_PMAP(MR(LM(MainDevice))));
        for(i = 0; i < (int)NELEM(depths_list); i++)
        {
            depth_t *depth = &depths_list[i];

            if(depth->bpp == bpp)
            {
                modstate(dp, depth->item, SETSTATE);
                current_depth_item = depth->item;
            }
            else
                modstate(dp, depth->item, CLEARSTATE);
        }
    }
    setedittextcstr(dp, PREF_SYSTEM, C_STRING_FROM_SYSTEM_VERSION());
}

void update_string_from_edit_text(char **strp, DialogPtr dp, INTEGER itemno)
{
    Str255 str;
    GUEST<Handle> h_s;
    Handle h;
    GUEST<INTEGER> type;
    Rect r;

    GetDItem(dp, itemno, &type, &h_s, &r);
    h = MR(h_s);
    GetIText(h, str);
    if(*strp)
        free(*strp);
    *strp = (char *)malloc(str[0] + 1);
    if(*strp)
    {
        memcpy(*strp, str + 1, str[0]);
        (*strp)[str[0]] = 0;
    }
}

void readprefvalues(DialogPtr dp)
{
    set_refresh_rate(getedittext(dp, PREFREFRESHITEM));
    update_string_from_edit_text(&ROMlib_Comments, dp, PREF_COMMENTS);

    if(getvalue(dp, PREFANIMATIONITEM))
        ROMlib_WriteWhen(WriteInBltrgn);
    else if(getvalue(dp, PREFINBETWEENITEM))
        ROMlib_WriteWhen(WriteAtEndOfTrap);
    else
        ROMlib_WriteWhen(WriteInOSEvent);

    ROMlib_clockonoff(!getvalue(dp, PREFNOCLOCKITEM));

    ROMlib_nowarn32 = getvalue(dp, PREFNO32BITWARNINGSITEM);
    ROMlib_flushoften = getvalue(dp, PREFFLUSHCACHEITEM);

    if(getvalue(dp, PREFSOUNDONITEM))
        ROMlib_PretendSound = soundon;
    else if(getvalue(dp, PREFSOUNDPRETENDITEM))
        ROMlib_PretendSound = soundpretend;
    else
        ROMlib_PretendSound = soundoff;

    ROMlib_passpostscript = getvalue(dp, PREFPASSPOSTSCRIPTITEM);
#if 1
    ROMlib_passpostscript = true; /* wired down for now */
#endif
    ROMlib_newlinetocr = getvalue(dp, PREFNEWLINEMAPPINGITEM);
    ROMlib_directdiskaccess = getvalue(dp, PREFDIRECTDISKITEM);
    ROMlib_fontsubstitution = getvalue(dp, PREFFONTSUBSTITUTIONITEM);
    ROMlib_cacheheuristic = getvalue(dp, PREFCACHEHEURISTICSITEM);

    ROMlib_pretend_help = getvalue(dp, PREF_PRETEND_HELP);
    ROMlib_pretend_edition = getvalue(dp, PREF_PRETEND_EDITION);
    ROMlib_pretend_script = getvalue(dp, PREF_PRETEND_SCRIPT);
    ROMlib_pretend_alias = getvalue(dp, PREF_PRETEND_ALIAS);
    {
        char *system_string;

        system_string = 0;
        update_string_from_edit_text(&system_string, dp, PREF_SYSTEM);
        parse_system_version(system_string);
        free(system_string);
    }
}

/*
 * Get rid of any characters that cause trouble inside a quoted
 * string.  Look at yylex() in parse.y.
 */

static void
clean(char *strp)
{
    char c;

    while((c = *strp++))
        switch(c)
        {
            case '"':
                strp[-1] = '\'';
                break;
        }
}

int saveprefvalues(const char *savefilename, LONGINT locationx, LONGINT locationy)
{
    int retval;
    FILE *fp;

    if((fp = Ufopen(savefilename, "w")))
    {
        {
            const char *lastslash;

            lastslash = strrchr(savefilename, '/');
            lastslash = lastslash ? lastslash + 1 : savefilename;
            fprintf(fp, "// This Configuration file (%s) was built by "
                        "Executor\n",
                    lastslash);
        }
        if(ROMlib_Comments)
        {
            clean(ROMlib_Comments);
            fprintf(fp, "Comments = \"%s\";\n", ROMlib_Comments);
        }
        if(ROMlib_WindowName)
        {
            clean(ROMlib_WindowName);
            fprintf(fp, "WindowName = \"%s\";\n", ROMlib_WindowName);
        }
        else
        {
#if defined(VDRIVER_DISPLAYED_IN_WINDOW)
            char *window_name;

            window_name = ROMlib_GetTitle();
            clean(window_name);
            fprintf(fp, "// WindowName = \"%s\";\n", window_name);
            ROMlib_FreeTitle(window_name);
#endif /* defined(VDRIVER_DISPLAYED_IN_WINDOW) */
        }
        fprintf(fp, "BitsPerPixel = %d;\n",
                PIXMAP_PIXEL_SIZE(GD_PMAP(MR(LM(MainDevice)))));

#if 0 && defined(MACOSX_)
	fprintf(fp, "ScreenSize = { %ld, %ld };\n", (long) curr_width, (long) curr_height);
	fprintf(fp, "MacSize = { %ld, %ld };\n", (long) orig_width, (long) orig_height);
#endif
        fprintf(fp, "ScreenLocation = { %ld, %ld };\n", (long)locationx, (long)locationy);

        fprintf(fp, "SystemVersion = %s;\n", C_STRING_FROM_SYSTEM_VERSION());
        fprintf(fp, "RefreshNumber = %d;\n", ROMlib_refresh);
        fprintf(fp, "Delay = %d;\n", ROMlib_delay);
        fprintf(fp, "Options = {");
        switch(ROMlib_when)
        {
            case WriteAlways:
            case WriteInBltrgn:
                fprintf(fp, "BlitOften");
                break;
            default:
            case WriteNever:
            case WriteInOSEvent:
                fprintf(fp, "BlitInOSEvent");
                break;
            case WriteAtEndOfTrap:
                fprintf(fp, "BlitAtTrapEnd");
                break;
        }
        switch(ROMlib_PretendSound)
        {
            case soundoff:
                fprintf(fp, ", SoundOff");
                break;
            case soundpretend:
                fprintf(fp, ", PretendSound");
                break;
            case soundon:
                fprintf(fp, ", SoundOn");
                break;
        }
        if(ROMlib_passpostscript)
            fprintf(fp, ", PassPostscript");
        if(ROMlib_newlinetocr)
            fprintf(fp, ", NewLineToCR");
        if(ROMlib_directdiskaccess)
            fprintf(fp, ", DirectDiskAccess");
#if 0
	if (ROMlib_accelerated)
	    fprintf(fp, ", Accelerated");
	if (ROMlib_clock != 2)
	    fprintf(fp, ", NoClock");
#endif
        if(ROMlib_nowarn32 != 0)
            fprintf(fp, ", NoWarn32");
        if(ROMlib_flushoften != 0)
            fprintf(fp, ", FlushOften");
        if(ROMlib_options & ROMLIB_DEBUG_BIT)
            fprintf(fp, ", Debug");

        if(ROMlib_pretend_help)
            fprintf(fp, ", PretendHelp");
        if(ROMlib_pretend_edition)
            fprintf(fp, ", PretendEdition");
        if(ROMlib_pretend_script)
            fprintf(fp, ", PretendScript");
        if(ROMlib_pretend_alias)
            fprintf(fp, ", PretendAlias");

        fprintf(fp, "};\n");
        fclose(fp);
        retval = true;
    }
    else
        retval = false;
    return retval;
}

typedef enum { disable,
               enable } enableness_t;

static void mod_item_enableness(DialogPtr dp, INTEGER item,
                                enableness_t enableness_wanted)
{
    INTEGER type;
    GUEST<INTEGER> type_s;
    Handle h;
    GUEST<Handle> tmpH;
    ControlHandle ch;
    Rect r;

    GetDItem(dp, item, &type_s, &tmpH, &r);
    type = CW(type_s);
    h = MR(tmpH);
    if(((type & itemDisable) && enableness_wanted == enable)
       || (!(type & itemDisable) && enableness_wanted == disable))
    {
        type ^= itemDisable;
        SetDItem(dp, item, type, h, &r);
    }
    ch = (ControlHandle)h;
    if(Hx(ch, contrlHilite) == 255 && enableness_wanted == enable)
        HiliteControl(ch, 0);
    else if(Hx(ch, contrlHilite) != 255 && enableness_wanted == disable)
        HiliteControl(ch, 255);
}

static void
set_sound_on_string(DialogPtr dp)
{
    Str255 sound_string;
    GUEST<INTEGER> junk1;
    GUEST<Handle> h;
    Rect junk2;

    str255_from_c_string(sound_string,
                         (SOUND_SILENT_P()
                              ? "On (silent)"
                              : "On"));
    GetDItem(dp, PREFSOUNDONITEM, &junk1, &h, &junk2);
    SetCTitle((ControlHandle)MR(h), sound_string);
}

/*
 * Don't count on the proper items to be enabled/disabled, do it
 * explicitly.  This is to avoid version skew problems associated with
 * our System file.
 */

static void enable_disable_pref_items(DialogPtr dp)
{
    static INTEGER to_enable[] = /* put only controls in this list */
        {
          PREFOKITEM,
          PREFCANCELITEM,
          PREFSAVEITEM,
          PREFNORMALITEM,
          PREFANIMATIONITEM,
          PREFSOUNDOFFITEM,
          PREFSOUNDPRETENDITEM,
          PREFNEWLINEMAPPINGITEM, /* Perhaps this should be disabled under DOS */
          PREFFLUSHCACHEITEM,
          PREFDIRECTDISKITEM,
          PREFNO32BITWARNINGSITEM,
          PREFFONTSUBSTITUTIONITEM,
          PREFCACHEHEURISTICSITEM,
          PREF_PRETEND_HELP,
          PREF_PRETEND_EDITION,
          PREF_PRETEND_SCRIPT,
          PREF_PRETEND_ALIAS,
          PREFREFRESHITEM,
        };
    static INTEGER to_disable[] = /* put only controls in this list */
        {
          PREFNOCLOCKITEM,
          PREFPASSPOSTSCRIPTITEM,
          PREF_COLORS_THOUSANDS,
          PREF_COLORS_MILLIONS,
          PREF_GRAY_SCALE,
          PREFSOUNDONITEM,
        };
    int i;

    for(i = 0; i < (int)NELEM(to_enable); ++i)
        mod_item_enableness(dp, to_enable[i], enable);
    for(i = 0; i < (int)NELEM(to_disable); ++i)
        mod_item_enableness(dp, to_disable[i], disable);

    if(SOUND_WORKS_P())
        mod_item_enableness(dp, PREFSOUNDONITEM, enable);
    set_sound_on_string(dp);

    for(i = 0; i < (int)NELEM(depths_list); i++)
    {
        depth_t *depth = &depths_list[i];

        if(C_HasDepth(MR(LM(MainDevice)), depth->bpp, 0, 0))
            mod_item_enableness(dp, depth->item, enable);
        else
            mod_item_enableness(dp, depth->item, disable);
    }
}

static void dopreferences(void)
{
    DialogPtr dp;
    INTEGER ihit;
    GUEST<INTEGER> ihit_s;

    if(!(ROMlib_options & ROMLIB_NOPREFS_BIT))
    {
        if(LM(WWExist) != EXIST_YES)
            SysBeep(5);
        else
        {
            static BOOLEAN am_already_here = false;

            if(!am_already_here)
            {
                am_already_here = true;

                ParamText(LM(CurApName), 0, 0, 0);

                dp = GetNewDialog(PREFDIALID, (Ptr)0, (WindowPtr)-1);
                enable_disable_pref_items(dp);
                setupprefvalues(dp);
                ROMlib_circledefault(dp);
                do
                {
                    ModalDialog(nullptr, &ihit_s);
                    ihit = CW(ihit_s);
                    switch(ihit)
                    {
                        case PREFNORMALITEM:
                        case PREFINBETWEENITEM:
                        case PREFANIMATIONITEM:
                            setoneofthree(dp, ihit, PREFNORMALITEM, PREFINBETWEENITEM,
                                          PREFANIMATIONITEM);
                            break;
                        case PREFSOUNDOFFITEM:
                        case PREFSOUNDPRETENDITEM:
                        case PREFSOUNDONITEM:
                            setoneofthree(dp, ihit, PREFSOUNDOFFITEM,
                                          PREFSOUNDPRETENDITEM, PREFSOUNDONITEM);
                            break;
                        case PREF_COLORS_2:
                        case PREF_COLORS_4:
                        case PREF_COLORS_16:
                        case PREF_COLORS_256:
                        case PREF_COLORS_THOUSANDS:
                        case PREF_COLORS_MILLIONS:
                            set_depth(dp, ihit);
                            break;
                        case PREFNOCLOCKITEM:
                        case PREFPASSPOSTSCRIPTITEM:
                        case PREFNEWLINEMAPPINGITEM:
                        case PREFFLUSHCACHEITEM:
                        case PREFDIRECTDISKITEM:
                        case PREFNO32BITWARNINGSITEM:
                        case PREFFONTSUBSTITUTIONITEM:
                        case PREFCACHEHEURISTICSITEM:
                        case PREF_PRETEND_HELP:
                        case PREF_PRETEND_EDITION:
                        case PREF_PRETEND_SCRIPT:
                        case PREF_PRETEND_ALIAS:
                            modstate(dp, ihit, FLIPSTATE);
                            break;
                    }
                } while(ihit != PREFOKITEM && ihit != PREFCANCELITEM && ihit != PREFSAVEITEM);
                if(ihit == PREFOKITEM || ihit == PREFSAVEITEM)
                {
                    readprefvalues(dp);
                    if(ihit == PREFSAVEITEM)
                        saveprefvalues(ROMlib_configfilename.c_str(), 0, 0);
                }
                DisposDialog(dp);
                am_already_here = false;
            }
        }
    }
}

static void doquitreallyquits(void)
{
    ROMlib_exit = !ROMlib_exit;
}

extern void ROMlib_updateworkspace(void);

static bool isSuspended = false;
static bool shouldBeSuspended = false;

static BOOLEAN doevent(INTEGER em, EventRecord *evt,
                       BOOLEAN remflag) /* no DA support */
{
    BOOLEAN retval;
    GUEST<ULONGINT> now_s;
    ULONGINT now;
    static int beenhere = 0;
    ALLOCABEGIN

    /* We tend to call this routine from various ROMlib modal loops, so this
     * is a good place to check for timer interrupts. */
    check_virtual_interrupt();

    hle_reset();

    evt->message = CLC(0);
    TRACE(2);
    if(LM(SPVolCtl) & 0x80)
    {
        TRACE(3);
        GetDateTime(&now_s);
        now = CL(now_s);
        TRACE(4);
        if(now >= (ULONGINT)Cx(LM(SPAlarm)))
        {
            TRACE(5);
            if(now & 1)
            {
                TRACE(6);
                if(ROMlib_alarmonmbar)
                    ROMlib_togglealarm();
            }
            else
            {
                TRACE(7);
                if(!ROMlib_alarmonmbar)
                    ROMlib_togglealarm();
            }
            TRACE(8);
        }
        else if(ROMlib_alarmonmbar)
        {
            TRACE(9);
            ROMlib_togglealarm();
        }
    }
    else if(ROMlib_alarmonmbar)
    {
        TRACE(10);
        ROMlib_togglealarm();
    }

    if(em & activMask)
    {
        TRACE(11);
        if(LM(CurDeactive))
        {
            TRACE(12);
            GetOSEvent(0, evt);
            TRACE(13);
            evt->what = CW(activateEvt);
            evt->message = guest_cast<LONGINT>(LM(CurDeactive));
            if(remflag)
                LM(CurDeactive) = nullptr;
            retval = true;
            /*-->*/ goto done;
        }
        if(LM(CurActivate))
        {
            TRACE(14);
            GetOSEvent(0, evt);
            TRACE(15);
            evt->what = CW(activateEvt);
            evt->message = guest_cast<LONGINT>(LM(CurActivate));
            evt->modifiers.raw_or(CW(activeFlag));
            if(remflag)
                LM(CurActivate) = nullptr;
            retval = true;
            /*-->*/ goto done;
        }
    }

    /*
 * NOTE: Currently (Sept. 15, 1993), we get different charCodes for
 *	 command-shift-1 on the NeXT and under DOS.  I suspect that
 *	 the DOS behaviour (the lower case stuff) is correct, my
 *	 clue is 1.15 vs 1.16 of osevent.c, but I don't want to mess
 *	 around with things just before releasing Executor/DOS 1.0.
 */

    if(remflag)
    {
        TRACE(16);
        retval = GetOSEvent(em, evt);
        TRACE(17);
        if(retval && Cx(evt->what) == keyDown && LM(ScrDmpEnb) && (Cx(evt->modifiers) & (shiftKey | cmdKey)) == (shiftKey | cmdKey))
        {
            TRACE(18);
            switch((Cx(evt->message) & charCodeMask))
            {
                case '1':
                case '!': /* command shift 1: About Box / Help */
                    retval = false;
                    do_about_box();
                    break;
                case '2':
                case '@': /* command shift 2: Floppy Stuff */
                    retval = false;
                    /* dofloppymount(); already done at a lower level */
                    break;
                case '3':
                case '#': /* command shift 3: Screen Dump to File */
                    retval = false;
                    do_dump_screen();
                    break;
                case '4':
                case '$': /* command shift 4: Screen Dump to Printer */
                    retval = false;
                    doscreendumptoprinter();
                    break;
                case '5':
                case '%': /* command shift 5: Preferences */
                    retval = false;
                    dopreferences();
                    break;
                case '6':
                case '^': /* command shift 6: Don't restart Executor */
                    retval = false;
                    doquitreallyquits();
                    break;
                case '7':
                case '&':
                    retval = false;
                    /* Reset the video mode.  Seems to be needed under DOS
		 * sometimes when hotkeying around.
		 */
                    vdriver_set_mode(0, 0, 0, vdriver_grayscale_p);
                    redraw_screen();
                    break;

#if defined(SUPPORT_LOG_ERR_TO_RAM)
                case '9':
                case '(': /* command shift 9: Dump RAM error log */
                    retval = false;
                    error_dump_ram_err_buf("\n *** cmd-shift-9 pressed ***\n");
                    break;
#endif
            }
            if(!retval)
                evt->what = CW(nullEvent);
            /*-->*/ goto done;
        }
    }
    else
    {
        TRACE(24);
        retval = OSEventAvail(em, evt);
        TRACE(25);
    }
    /*
 * NOTE: I think the following block of code should probably be in SystemEvent,
 *	 rather than here.  It will probably make a difference when an event
 *	 call is made without diskMask set.  (I think SystemTask does the
 *	 mount anyway and it potentially gets lost if no one is looking for
 *	 it).
 */
    if(!retval && remflag && (em & diskMask))
    {
        TRACE(26);
        if(!beenhere)
        {
            TRACE(26);
            beenhere = 1;
            ROMlib_openharddisk("/tmp/testvol\0\0", &evt->message);
            if(evt->message)
            {
                TRACE(27);
                evt->what = CW(diskEvt);
                retval = true;
            }
        }
    }
    if(!retval && (em & updateMask))
    {
        TRACE(28);
        GetOSEvent(0, evt);
        TRACE(29);
        retval = CheckUpdate(evt);
    }

    /* check for high level events */
    if(!retval && (em & highLevelEventMask))
    {
        retval = hle_get_event(evt, remflag);
    }

    if(!retval && ROMlib_delay)
    {
        TRACE(30);
        Delay((LONGINT)ROMlib_delay, (LONGINT *)0);
    }
done:
    ALLOCAEND
    TRACE(31);
    if(SystemEvent(evt))
    {
        TRACE(32);
        evt->what = CWC(nullEvent);
        retval = false;
    }
    TRACE(33);

    if(!retval)
    {
        if(isSuspended != shouldBeSuspended)
        {
            WindowPtr w = C_FrontWindow();
            if(w)
            {
                if(shouldBeSuspended)
                    LM(CurDeactive) = RM(w);
                else
                    LM(CurActivate) = RM(w);
                C_HiliteWindow(w, !shouldBeSuspended);
                isSuspended = shouldBeSuspended;
            }
        }
    }

    return retval;
}

BOOLEAN Executor::C_GetNextEvent(INTEGER em, EventRecord *evt)
{
    BOOLEAN retval;

    TRACE(1);
    retval = doevent(em, evt, true);
    TRACE(0);
    return retval;
}

/*
 * NOTE: the code for WaitNextEvent below is not really according to spec,
 *	 but it should do the job.  We could rig a fancy scheme that
 *	 communicates with the display postscript loop on the NeXT but that
 *	 wouldn't be general purpose and this *should* be good enough.  We
 *	 have to use Delay rather than timer calls ourself in case the timer
 *	 is already in use.  We also can't delay the full interval because
 *	 that would prevent window resizing from working on the NeXT (and
 *	 we'd never get the mouse moved messages we need)
 */

BOOLEAN Executor::C_WaitNextEvent(INTEGER mask, EventRecord *evp,
                                  LONGINT sleep, RgnHandle mousergn)
{
    BOOLEAN retval;
    Point p;

    do
    {
        retval = GetNextEvent(mask, evp);
        if(!retval)
        {
            static INTEGER saved_h, saved_v;

            /* TODO: see what PtInRgn does with 0 as a RgnHandle */
            p.h = CW(evp->where.h);
            p.v = CW(evp->where.v);
            if(mousergn && !EmptyRgn(mousergn) && !PtInRgn(p, mousergn)
               && (p.h != saved_h || p.v != saved_v))
            {
                evp->what = CWC(osEvt);
                evp->message = CLC(mouseMovedMessage << 24);
                retval = true;
            }
            else if(sleep > 0)
            {
                Delay(MIN(sleep, 4), (LONGINT *)0);
                sleep -= 4;
            }
            saved_h = p.h;
            saved_v = p.v;
        }
    } while(!retval && sleep > 0);
    return retval;
}

BOOLEAN Executor::C_EventAvail(INTEGER em, EventRecord *evt)
{
    return (doevent(em, evt, false));
}

void Executor::C_GetMouse(GUEST<Point> *p)
{
    EventRecord evt;

    GetOSEvent(0, &evt);
    *p = evt.where;
    GlobalToLocal(p);
}

// FIXME: #warning Button not coded per IM Macintosh Toolbox Essentials 2-108
BOOLEAN Executor::C_Button()
{
    EventRecord evt;
    BOOLEAN retval;

    GetOSEvent(0, &evt);
    retval = (evt.modifiers & CWC(btnState)) ? false : true;
    return retval;
}

// FIXME: #warning StillDown not coded per IM Macintosh Toolbox Essentials 2-109
BOOLEAN Executor::C_StillDown() /* IMI-259 */
{
    EventRecord evt;

    return Button() ? !OSEventAvail((mDownMask | mUpMask), &evt) : false;
}

/*
 * The weirdness below is because Word 5.1a gets very unhappy if
 * TickCount makes large adjustments to LM(Ticks).  Even when we
 * just increase by one there are problems... The "no clock" option
 * might be retiring soon.
 */

BOOLEAN Executor::C_WaitMouseUp()
{
    EventRecord evt;
    int retval;

    retval = StillDown();
    if(!retval)
        GetOSEvent(mUpMask, &evt); /* just remove one ? */
    return (retval);
}

void Executor::C_GetKeys(unsigned char *keys)
{
    BlockMoveData((Ptr)LM(KeyMap), (Ptr)keys, (Size)sizeof_KeyMap);
}

LONGINT Executor::C_TickCount()
{
    unsigned long ticks;
    unsigned long new_time;

    ticks = msecs_elapsed() * 3.0 / 50; /* == 60 / 1000 */

    /* Update LM(Ticks) and LM(Time).  We only update LM(Ticks) if the clock is on;
   * this seems to be necessary to make Word happy.
   */

    if(ROMlib_clock)
        LM(Ticks) = CL(ticks);

    new_time = (UNIXTIMETOMACTIME(ROMlib_start_time.tv_sec)
                + (long)((ROMlib_start_time.tv_usec / (1000000.0 / 60) + ticks) / 60));

    LM(Time) = CL(new_time);
    return ticks;
}

LONGINT Executor::GetDblTime()
{
    return (Cx(LM(DoubleTime)));
}

LONGINT Executor::GetCaretTime()
{
    return (Cx(LM(CaretTime)));
}

/*
 * These routines used to live in the various OS-specific config directories,
 * which was a real crock, since only the NEXTSTEP implementation worked
 * well.  I'm moving them here as I attempt to implement cut and paste
 * properly under Win32.
 */

#define SANE_DEBUGGING
#if defined(SANE_DEBUGGING)
static int sane_debugging_on = 0; /* Leave this off and let the person doing the
			      debugging turn it on if he/she wants.  If this
			      is set to non-zero, it breaks code.  Not a
			      very nice thing to do. */
#endif /* SANE_DEBUGGING */

void
Executor::sendsuspendevent(void)
{
    Point p;

    shouldBeSuspended = true;
    if(
#if defined(MACOSX_) || defined(MACOSX_)
        printstate == __idle &&
#endif
        (size_info.size_flags & SZacceptSuspendResumeEvents)
#if defined(SANE_DEBUGGING)
        && !sane_debugging_on
#endif /* SANE_DEBUGGING */
        /* NOTE: Since Executor can currently only run one app at a time,
	 suspending an app that can background causes trouble with apps
	 like StuffIt Expander, since it makes it impossible to do work
	 in another window while waiting for StuffIt Expander to expand
	 a file.  It's not clear that the next two lines are the best solution,
	 but it's *a* solution. */
        && (!(ROMlib_options & ROMLIB_NOSUSPEND_BIT) /* ||
	  !(size_info.size_flags & SZcanBackground) */))
    {
        p.h = CW(LM(MouseLocation).h);
        p.v = CW(LM(MouseLocation).v);
        ROMlib_PPostEvent(osEvt, SUSPENDRESUMEBITS | SUSPEND | CONVERTCLIPBOARD,
                          (GUEST<EvQElPtr> *)0, TickCount(), p, ROMlib_mods);
    }
}

void
Executor::sendresumeevent(bool cvtclip)
{
    LONGINT what;
    Point p;

    shouldBeSuspended = false;
    if(
#if defined(MACOSX_) || defined(MACOSX_)
        printstate == __idle &&
#endif
        (size_info.size_flags & SZacceptSuspendResumeEvents)
#if defined(SANE_DEBUGGING)
        && !sane_debugging_on
#endif /* SANE_DEBUGGING */
        )
    {
        what = SUSPENDRESUMEBITS | RESUME;
        if(cvtclip)
            what |= CONVERTCLIPBOARD;
        p.h = CW(LM(MouseLocation).h);
        p.v = CW(LM(MouseLocation).v);
        ROMlib_PPostEvent(osEvt, what, (GUEST<EvQElPtr> *)0, TickCount(),
                          p, ROMlib_mods);
    }
}

void
sendcopy(void)
{
    Point p;

    p.h = CW(LM(MouseLocation).h);
    p.v = CW(LM(MouseLocation).v);
    ROMlib_PPostEvent(keyDown, 0x0863, /* 0x63 == 'c' */
                      (GUEST<EvQElPtr> *)0, TickCount(), p, cmdKey | btnState);
    ROMlib_PPostEvent(keyUp, 0x0863,
                      (GUEST<EvQElPtr> *)0, TickCount(), p, cmdKey | btnState);
}

void
sendpaste(void)
{
    Point p;

    p.h = CW(LM(MouseLocation).h);
    p.v = CW(LM(MouseLocation).v);
    ROMlib_PPostEvent(keyDown, 0x0976, /* 0x76 == 'v' */
                      (GUEST<EvQElPtr> *)0, TickCount(), p, cmdKey | btnState);
    ROMlib_PPostEvent(keyUp, 0x0976,
                      (GUEST<EvQElPtr> *)0, TickCount(), p, cmdKey | btnState);
}

/*
 * NOTE: the code for ROMlib_send_quit below is cleaner than the code for
 *       sendcopy and sendpaste above, but it was added after 2.1pr0
 *       was released but before 2.1 was released, so we can't tamper
 *       with the above code because there's a chance it would break
 *       something.  Ick.
 */

static void
post_helper(INTEGER code, uint8_t raw, uint8_t mapped, INTEGER mods)
{
    Point p;

    p.h = CW(LM(MouseLocation).h);
    p.v = CW(LM(MouseLocation).v);

    ROMlib_PPostEvent(code, (raw << 8) | mapped, (GUEST<EvQElPtr> *)0,
                      TickCount(), p, btnState | mods);
}

void
Executor::ROMlib_send_quit(void)
{
    post_helper(keyDown, MKV_CLOVER, 0, 0);
    post_helper(keyDown, MKV_q, 'q', cmdKey);
    post_helper(keyUp, MKV_q, 'q', cmdKey);
    post_helper(keyUp, MKV_CLOVER, 0, cmdKey);
}
