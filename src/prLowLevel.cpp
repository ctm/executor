/*
 * Copyright 1989 - 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in PrintMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "PrintMgr.h"
#include "MemoryMgr.h"
#include "OSUtil.h"
#include "BinaryDecimal.h"
#include "ControlMgr.h"
#include "ResourceMgr.h"
#include "rsys/nextprint.h"
#include "rsys/print.h"
#include "rsys/hook.h"
#include "rsys/ctl.h"
#include "rsys/ini.h"
#include "rsys/notmac.h"
#include <ctype.h>
#include "MenuMgr.h"
#include "rsys/vdriver.h"
#include "rsys/uniquefile.h"
#include "rsys/system_error.h"
#include "rsys/options.h"
#include "rsys/string.h"
#include "rsys/osevent.h"
#ifdef MACOSX_
//#include "contextswitch.h"
#endif

#if defined(CYGWIN32)
#include "win_print.h"
#endif

using namespace Executor;

uint32_t ROMlib_PrDrvrVers = 70;

void Executor::C_PrDrvrOpen() /* TODO */
{
    warning_unimplemented(NULL_STRING);
}

void Executor::C_PrDrvrClose() /* TODO */
{
    warning_unimplemented(NULL_STRING);
}

void Executor::C_PrCtlCall(INTEGER iWhichCtl, LONGINT lParam1, LONGINT lParam2,
                           LONGINT lParam3) /* TODO */
{
    warning_unimplemented(NULL_STRING);
}

Handle Executor::C_PrDrvrDCE() /* TODO */
{
    return (Handle)0;
}

INTEGER Executor::C_PrDrvrVers()
{
    INTEGER retval;

    retval = ROMlib_PrDrvrVers;
    return retval;
}

static ControlHandle
GetDControl(DialogPtr dp, INTEGER itemno)
{
    GUEST<Handle> h;
    ControlHandle retval;
    GUEST<INTEGER> unused;

    GetDItem(dp, itemno, &unused, &h, NULL);
    retval = (ControlHandle)MR(h);
    return retval;
}

static Handle
GetDIText(DialogPtr dp, INTEGER itemno)
{
    GUEST<Handle> h;
    Handle retval;
    GUEST<INTEGER> unused;

    GetDItem(dp, itemno, &unused, &h, NULL);
    retval = MR(h);

    return retval;
}

LONGINT
GetDILong(DialogPtr dp, INTEGER item, LONGINT _default)
{
    Str255 str;
    LONGINT retval;
    Handle h;

    h = GetDIText(dp, item);
    GetIText(h, str);
    if(str[0] == 0 || str[1] < '0' || str[1] > '9')
        retval = _default;
    else
        StringToNum(str, &retval);
    return retval;
}

#if defined(CYGWIN32)
win_printp_t ROMlib_wp;
#endif

void Executor::C_ROMlib_myjobproc(DialogPtr dp, INTEGER itemno)
{
    switch(itemno)
    {
        case PRINT_OK_NO:
        {
            ControlHandle ch;
            THPrint hPrint;
            INTEGER num_copies;

            hPrint = MR(((TPPrDlg)dp)->hPrintUsr);
            ch = GetDControl(dp, PRINT_ALL_RADIO_NO);
            if(GetCtlValue(ch))
            {
                HxX(hPrint, prJob.iFstPage) = CWC(1);
                HxX(hPrint, prJob.iLstPage) = CWC(9999);
            }
            else
            {
                INTEGER first, last;

                first = GetDILong(dp, PRINT_FIRST_BOX_NO, 1);
                last = GetDILong(dp, PRINT_LAST_BOX_NO, 9999);
                HxX(hPrint, prJob.iFstPage) = CW(first);
                HxX(hPrint, prJob.iLstPage) = CW(last);
            }
            {

                num_copies = GetDILong(dp, PRINT_COPIES_BOX_NO, 1);
                HxX(hPrint, prJob.iCopies) = CW(num_copies);
            }
#if defined(CYGWIN32)
            // FIXME: #warning TODO use better x and y coords
            warning_trace_info("ROMlib_printer = %s, WIN32_TOKEN = %s",
                               ROMlib_printer, WIN32_TOKEN);
            if(strcmp(ROMlib_printer, WIN32_TOKEN) == 0)
            {
                orientation_t orient;

                if(strcmp(ROMlib_paper_orientation, "Portrait") == 0)
                    orient = WIN_PRINT_PORTRAIT;
                else
                    orient = WIN_PRINT_LANDSCAPE;
                get_info(&ROMlib_wp, ROMlib_paper_x, ROMlib_paper_y, orient,
                         num_copies, NULL);
                HxX(hPrint, prJob.iCopies) = CWC(1); /* Win32 driver handles
						     the multiple copies
						     for us */
            }
#endif
        }
        break;
        case PRINT_ALL_RADIO_NO:
        case PRINT_FROM_RADIO_NO:
        case PRINT_FIRST_BOX_NO:
        case PRINT_LAST_BOX_NO:
        {
            ControlHandle ch;

            ch = GetDControl(dp, PRINT_ALL_RADIO_NO);
            SetCtlValue(ch, itemno == PRINT_ALL_RADIO_NO ? 1 : 0);
            ch = GetDControl(dp, PRINT_FROM_RADIO_NO);
            SetCtlValue(ch, itemno == PRINT_ALL_RADIO_NO ? 0 : 1);
            if(itemno == PRINT_ALL_RADIO_NO)
            {
                SetIText(GetDIText(dp, PRINT_FIRST_BOX_NO), (StringPtr) "");
                SetIText(GetDIText(dp, PRINT_LAST_BOX_NO), (StringPtr) "");
            }
        }
        default:;
    }
}

static void
add_orientation_icons_to_update_region(DialogPtr dp)
{
    GrafPtr gp;
    Rect r;
    GUEST<INTEGER> unused;

    gp = thePort;
    SetPort(dp);
    GetDItem(dp, LAYOUT_PORTRAIT_NO, &unused, NULL, &r);
    InvalRect(&r);
    GetDItem(dp, LAYOUT_LANDSCAPE_NO, &unused, NULL, &r);
    InvalRect(&r);
    SetPort(gp);
}

static void
update_orientation(DialogPtr dp, INTEGER button)
{
    switch(button)
    {
        case LAYOUT_LANDSCAPE_ICON_NO:
            if(ROMlib_paper_orientation != "Landscape")
            {
                ROMlib_paper_orientation = "Landscape";
                add_orientation_icons_to_update_region(dp);
            }
            break;
        case LAYOUT_PORTRAIT_ICON_NO:
            if(ROMlib_paper_orientation != "Portrait")
            {
                ROMlib_paper_orientation = "Portrait";
                add_orientation_icons_to_update_region(dp);
            }
            break;
        default:
            warning_unexpected("button = %d", button);
            break;
    }
}

char *
Executor::cstring_from_str255(Str255 text)
{
    int len;
    char *retval;

    len = (uint8)text[0];
    retval = (char *)malloc(len + 1);
    if(retval)
    {
        memcpy(retval, text + 1, len);
        retval[len] = 0;
    }
    return retval;
}

namespace Executor
{
ini_key_t ROMlib_printer = "";
static ini_key_t ROMlib_paper = "";
ini_key_t ROMlib_port = "";
ini_key_t ROMlib_paper_orientation = "";
static ini_key_t ROMlib_spool_template = "";
#if defined(MSDOS) || defined(CYGWIN32)
static ini_key_t ROMlib_print_filter = "";
#endif
}

static MenuHandle
GetPopUpMenu(ControlHandle ch)
{
    MenuHandle retval;
    popup_data_handle dh;

    dh = (popup_data_handle)CTL_DATA(ch);
    retval = dh ? POPUP_MENU(dh) : 0;

    return retval;
}

static ini_key_t
find_item_key(DialogPtr dlg, INTEGER itemno)
{
    MenuHandle mh;
    ControlHandle ch;
    Str255 text;
    ini_key_t retval;

    ch = GetDControl(dlg, itemno);
    mh = GetPopUpMenu(ch);
    GetItem(mh, GetCtlValue(ch), text);
    retval = cstring_from_str255(text);
    return retval;
}

char *Executor::ROMlib_spool_file;

void
Executor::update_printing_globals(void)
{
    {
        value_t dimensions;

        /* toss in some defaults, just in case someone screwed up the .ini file */
        ROMlib_paper_x = 612;
        ROMlib_paper_y = 792;
        dimensions = find_key("Paper Size", ROMlib_paper);
        if(dimensions != "")
            sscanf(dimensions.c_str(), "%d %d", &ROMlib_paper_x, &ROMlib_paper_y);
    }

    if((ROMlib_paper_orientation == "Portrait") == (ROMlib_paper_x > ROMlib_paper_y))
        std::swap(ROMlib_paper_x, ROMlib_paper_y);

    ROMlib_document_paper_sizes = "%%DocumentPaperSizes: ";
    ROMlib_paper_size = "%%PaperSize: ";
    ROMlib_paper_size_name = ROMlib_paper.c_str();
    ROMlib_paper_size_name_terminator = "\n";

    if(ROMlib_paper_x < ROMlib_paper_y)
    {
        ROMlib_rotation = 0;
        ROMlib_translate_x = 0;
        ROMlib_translate_y = ROMlib_paper_y;
    }
    else
    {
        ROMlib_rotation = -90;
        ROMlib_translate_x = -ROMlib_paper_x;
        ROMlib_translate_y = ROMlib_paper_y;
    }
}

static void
update_ROMlib_printer_vars(TPPrDlg dp)
{
    ROMlib_printer = find_item_key((DialogPtr)dp, LAYOUT_PRINTER_TYPE_NO);

    ROMlib_paper = find_item_key((DialogPtr)dp, LAYOUT_PAPER_NO);

    ROMlib_port = find_item_key((DialogPtr)dp, LAYOUT_PORT_MENU_NO);

    ROMlib_set_default_resolution(MR(dp->hPrintUsr), 72, 72);

    if(ROMlib_printer == "PostScript File")
    {
        Handle h;
        Size hs;

        h = GetDIText((DialogPtr)dp, LAYOUT_FILENAME_NO);
        hs = GetHandleSize(h);
        if(hs > 0)
        {
            if(ROMlib_spool_file)
                free(ROMlib_spool_file);
            ROMlib_spool_file = (char *)malloc(hs + 1);
            memcpy(ROMlib_spool_file, STARH(h), hs);
            ROMlib_spool_file[hs] = 0;
        }
    }

    if(!ROMlib_PrintDef.empty())
    {
        FILE *fp;

        fp = open_ini_file_for_writing(ROMlib_PrintDef.c_str());
        if(fp)
        {
            add_heading_to_file(fp, "Defaults");
            if(ROMlib_printer != "")
                add_key_value_to_file(fp, "Printer", ROMlib_printer);

            if(ROMlib_paper != "")
                add_key_value_to_file(fp, "Paper Size", ROMlib_paper);

            if(ROMlib_port != "")
                add_key_value_to_file(fp, "Port", ROMlib_port);

            if(ROMlib_paper_orientation != "")
                add_key_value_to_file(fp, "Orientation", ROMlib_paper_orientation);

            close_ini_file(fp);
        }
    }
}

#if !defined(LINUX)
static void
get_popup_bounding_box(Rect *rp, DialogPtr dp, INTEGER itemno)
{
    GUEST<INTEGER> unused;

    GetDItem(dp, itemno, &unused, NULL, rp);
    rp->left = CW(CW(rp->left) - 1);
    rp->bottom = CW(CW(rp->bottom) + 3);
}
#endif

/* check to see if we've changed to or from a PostScript file */

typedef enum {
    PRINT_TO_PORT,
    PRINT_TO_FILE,
    PRINT_TO_WIN32,
    PRINT_TO_NEXTSTEP
} print_where_t;

static print_where_t print_where;

static bool filename_chosen_p = false;

static void
update_port(DialogPtr dp)
{
    GrafPtr gp;
    char *keyp;

    gp = thePort;
    SetPort(dp);

    keyp = strdup(find_item_key(dp, LAYOUT_PRINTER_TYPE_NO).c_str());
    if(strcmp(keyp, "PostScript File") == 0)
    {
        if(print_where != PRINT_TO_FILE)
        {
#if !defined(LINUX)
            Rect r;

            get_popup_bounding_box(&r, dp, LAYOUT_PORT_MENU_NO);
            EraseRect(&r);
            HideDItem(dp, LAYOUT_PORT_LABEL_NO);
            HideDItem(dp, LAYOUT_PORT_MENU_NO);
            vdriver_flush_display();
#endif
            ShowDItem(dp, LAYOUT_FILENAME_LABEL_NO);
            if(!filename_chosen_p)
            {
                Str255 str;

                unique_file_name(ROMlib_spool_template.c_str(), "execout*.ps", str);
                SetIText(GetDIText(dp, LAYOUT_FILENAME_NO), str);
                filename_chosen_p = true;
            }
            ShowDItem(dp, LAYOUT_FILENAME_NO);
            SelIText(dp, LAYOUT_FILENAME_NO, 0, 32767);
            print_where = PRINT_TO_FILE;
        }
    }
#if defined(CYGWIN32)
    else if(strcmp(keyp, WIN32_TOKEN) == 0)
    {
        if(print_where != PRINT_TO_WIN32)
        {
            Rect r;

            get_popup_bounding_box(&r, dp, LAYOUT_PORT_MENU_NO);
            EraseRect(&r);
            HideDItem(dp, LAYOUT_PORT_LABEL_NO);
            HideDItem(dp, LAYOUT_PORT_MENU_NO);
            HideDItem(dp, LAYOUT_FILENAME_LABEL_NO);
            HideDItem(dp, LAYOUT_FILENAME_NO);
            vdriver_flush_display();
            print_where = PRINT_TO_WIN32;
        }
    }
#endif
    else
    {
        if(print_where != PRINT_TO_PORT)
        {
            HideDItem(dp, LAYOUT_FILENAME_LABEL_NO);
            HideDItem(dp, LAYOUT_FILENAME_NO);
#if !defined(LINUX)
            {
                Rect r;

                ShowDItem(dp, LAYOUT_PORT_LABEL_NO);
                ShowDItem(dp, LAYOUT_PORT_MENU_NO);
                get_popup_bounding_box(&r, dp, LAYOUT_PORT_MENU_NO);
                InvalRect(&r);
            }
#else
            vdriver_flush_display();
#endif
            print_where = PRINT_TO_PORT;
        }
    }
    free(keyp);
    SetPort(gp);
}

void Executor::C_ROMlib_mystlproc(DialogPtr dp, INTEGER itemno)
{
    switch(itemno)
    {
        case LAYOUT_OK_NO:
#if defined(MACOSX_)
//	printstate = seenPageSetUp;
#endif
            update_ROMlib_printer_vars((TPPrDlg)dp);
            break;
        case LAYOUT_PORTRAIT_ICON_NO:
        case LAYOUT_LANDSCAPE_ICON_NO:
            update_orientation(dp, itemno);
            break;

        case LAYOUT_PRINTER_TYPE_NO:
            update_port(dp);
            break;

        default:
            break;
    }
}

using prinitprocp = UPP<TPPrDlg(THPrint hPrint)>;

using pritemprocp = UPP<void(DialogPtr prrecptr, INTEGER item)>;


#define CALLPRINITPROC(hPrint, fp) \
    ROMlib_CALLPRINITPROC((hPrint), (prinitprocp)(fp))

namespace Executor
{
static inline TPPrDlg ROMlib_CALLPRINITPROC(THPrint, prinitprocp);
static inline void ROMlib_CALLPRITEMPROC(TPPrDlg, INTEGER, pritemprocp);
}

static inline TPPrDlg Executor::ROMlib_CALLPRINITPROC(THPrint hPrint, prinitprocp fp)
{
    TPPrDlg retval;
    ROMlib_hook(pr_initnumber);
    if(fp == &PrStlInit)
        retval = C_PrStlInit(hPrint);
    else if(fp == &PrJobInit)
        retval = C_PrJobInit(hPrint);
    else
    {
        retval = fp(hPrint);
    }
    return retval;
}

#define CALLPRITEMPROC(prrecptr, item, fp) \
    ROMlib_CALLPRITEMPROC((prrecptr), (item), (pritemprocp)(fp))

static inline void Executor::ROMlib_CALLPRITEMPROC(TPPrDlg prrecptr, INTEGER item, pritemprocp fp)
{
    ROMlib_hook(pr_itemnumber);
    if(fp == &ROMlib_myjobproc)
        C_ROMlib_myjobproc((DialogPtr)prrecptr, item);
    else if(fp == &ROMlib_mystlproc)
        C_ROMlib_mystlproc((DialogPtr)prrecptr, item);
    else
    {
        fp((GrafPtr)prrecptr, item);
    }
}

BOOLEAN Executor::C_ROMlib_stlfilterproc(
    DialogPtr dlg, EventRecord *evt, GUEST<INTEGER> *ith)
{
    BOOLEAN retval;
    char *keyp;

    retval = false;
    /* Check for user hitting <Enter> or clicking on "OK" button */
    switch(CW(evt->what))
    {
        case keyDown:
        {
            char c;

            c = CL(evt->message) & 0xFF;
            if(c == '\r' || c == NUMPAD_ENTER)
            {
                maybe_wait_for_keyup();
                *ith = CWC(OK);
                retval = true;
            }
        }
        break;
        case mouseDown:
        {
            GUEST<Point> glocalp;
            Point localp;
            GrafPtr gp;
            Rect r;
            GUEST<Handle> h;
            GUEST<INTEGER> unused;

            glocalp = evt->where;
            gp = thePort;
            SetPort(dlg);
            GlobalToLocal(&glocalp);
            localp = glocalp.get();
            SetPort(gp);
            GetDItem(dlg, OK, &unused, &h, &r);
            if(PtInRect(localp, &r))
            {
                ControlHandle ch;

                ch = (ControlHandle)MR(h);
                if(TrackControl(ch, localp, NULL))
                {
                    *ith = CWC(OK);
                    retval = true;
                }
            }
            break;
        }
        default:
            break;
    }

    keyp = strdup(find_item_key(dlg, LAYOUT_PRINTER_TYPE_NO).c_str());
    if(retval && *ith == CWC(OK) && (strcmp(keyp, "PostScript File") == 0))
    {
        struct stat sbuf;
        Handle h;
        char *filename;
        int len;

        h = GetDIText(dlg, LAYOUT_FILENAME_NO);
        len = GetHandleSize(h);
        filename = (char *)alloca(len + 1);
        memcpy(filename, STARH(h), len);
        filename[len] = 0;

        if(stat(filename, &sbuf) == 0)
        {
            char *buf;

#define EXISTS_MESSAGE                                                 \
    "The file \"%s\" already exists.  "                                \
    "Click on the \"Change\" button to choose a different filename.  " \
    "If you click on the \"Overwrite\" button, "                       \
    "\"%s\" will be overwritten as soon as you print."

            buf = (char *)alloca(sizeof(EXISTS_MESSAGE) + strlen(filename) + strlen(filename) + 1);
            sprintf(buf, EXISTS_MESSAGE, filename, filename);
            if(system_error(buf, 0,
                            "Change", "Overwrite", NULL,
                            NULL, NULL, NULL)
               == 0)
                *ith = CWC(LAYOUT_CIRCLE_OK_NO); /* dummy return value */
        }
    }

    free(keyp);
    return retval;
}

BOOLEAN Executor::C_ROMlib_numsonlyfilterproc(
    DialogPtr dlg, EventRecord *evt, GUEST<INTEGER> *ith)
{
    char c;

    if(Cx(evt->what) == keyDown)
    {
        c = Cx(evt->message) & 0xFF;
        switch(c)
        {
            case '\r':
            case NUMPAD_ENTER:
                maybe_wait_for_keyup();
                *ith = CW(OK);
                return true;
                break;
            default:
                return false;
                break;
        }
    }
    return false;
}

static void
set_userItem(DialogPtr dp, INTEGER itemno, UserItemProcPtr funcp)
{
    Rect r;
    GUEST<INTEGER> unused;

    GetDItem(dp, itemno, &unused, NULL, &r);
    SetDItem(dp, itemno, userItem, (Handle)(void*)funcp, &r);
}

static void
adjust_num_copies(TPPrDlg dlg, THPrint hPrint)
{
    Str255 text;

    NumToString(Hx(hPrint, prJob.iCopies), text);
    SetIText(GetDIText((DialogPtr)dlg, PRINT_COPIES_BOX_NO), text);
    SelIText((DialogPtr)dlg, PRINT_COPIES_BOX_NO, 0, 100);
#if defined(CYGWIN32)
    if(strcmp(ROMlib_printer, WIN32_TOKEN) == 0)
    {
        HideDItem((DialogPtr)dlg, PRINT_COPIES_LABEL_NO);
        HideDItem((DialogPtr)dlg, PRINT_COPIES_BOX_NO);
    }
#endif
}

static void
adjust_print_range_controls(TPPrDlg dlg, THPrint hPrint)
{
    ControlHandle ch;

    ch = GetDControl((DialogPtr)dlg, PRINT_ALL_RADIO_NO);
    SetCtlValue(ch, 1);
}

static ini_key_t
get_default_key(ini_key_t key, ini_key_t default_if_not_found)
{
    ini_key_t retval = find_key("Defaults", key);

    /* Verify that the default is legitimate */
    if(find_key(key, retval) == "")
    {
        pair_link_t *pairp;

        pairp = get_pair_link_n(key, 0); /* default is first legit entry */
        if(pairp)
            retval = pairp->key;
    }
    if(retval == "")
        retval = default_if_not_found;
    return retval;
}

static void
str255assignc(Str255 str, const char *cstr)
{
    if(!cstr)
        str[0] = 0;
    else
    {
        str[0] = MIN((int)strlen(cstr), 255);
        memcpy(str + 1, cstr, (unsigned char)str[0]);
    }
}

static void
get_all_defaults(void)
{
    static struct temp
    {
        ini_key_t *variablep;
        ini_key_t key;
        ini_key_t default_key;
    } default_table[] = {
        {
            &ROMlib_printer, "Printer", "",
        },
        {
            &ROMlib_paper, "Paper Size", "Letter",
        },
        {
            &ROMlib_port, "Port", "LPT1",
        },
        {
            &ROMlib_paper_orientation, "Orientation", "Portrait",
        },
#if defined(MSDOS) || defined(CYGWIN32)
        {
            &ROMlib_print_filter, "Filter", "GhostScript",
        },
#endif
    };
    int i;

    for(i = 0; i < (int)NELEM(default_table); ++i)
        if(*default_table[i].variablep == "")
            *default_table[i].variablep = get_default_key(default_table[i].key,
                                                          default_table[i].default_key);
    ROMlib_spool_template = find_key("Printer", "PostScript File");
    if(ROMlib_spool_template == "")
        ROMlib_spool_template = "+/execout*.ps";
}

static void
adjust_print_name(DialogPtr dp)
{
    Str255 name;

    if(!ROMlib_spool_file)
    {
        Str255 str;

        unique_file_name(ROMlib_spool_template.c_str(), "execout*.ps", str);
        ROMlib_spool_file = cstring_from_str255(str);
    }

    if(ROMlib_printer == "PostScript File")
        str255assignc(name, ROMlib_spool_file);
    else
        str255assignc(name, ROMlib_printer.c_str());
    SetIText(GetDIText(dp, 3), name);
}

TPPrDlg Executor::C_PrJobInit(THPrint hPrint)
{
    TPPrDlg retval;

    printer_init();
    if(ROMlib_printresfile != -1)
    {
        retval = (TPPrDlg)NewPtr(sizeof(TPrDlg));
        if(GetNewDialog(-8191, (Ptr)retval, (WindowPtr)-1))
        {
            /* TODO: Figure out what to do with the printer name, vs.
	     the spool file name */

            set_userItem((DialogPtr)retval, PRINT_CIRCLE_OK_NO,
                         &ROMlib_circle_ok);

            adjust_num_copies(retval, hPrint);

            adjust_print_range_controls(retval, hPrint);

            adjust_print_name((DialogPtr)retval);

            retval->pFltrProc = RM(&ROMlib_numsonlyfilterproc);
            /* TODO: Get this from the right place */
            retval->pItemProc = RM(&ROMlib_myjobproc);
            /* TODO: Get this from the right place */
            retval->hPrintUsr = RM(hPrint);
        }
        else
        {
            DisposPtr((Ptr)retval);
            retval = 0;
        }
    }
    else
        retval = 0;
    return retval;
}

void Executor::C_ROMlib_circle_ok(DialogPtr dp, INTEGER which)
{
    Rect r;
    GUEST<INTEGER> unused;

    GetDItem(dp, which, &unused, NULL, &r);
    PenNormal();
    PenSize(3, 3);
    InsetRect(&r, -4, -4);
    if(!(ROMlib_options & ROMLIB_RECT_SCREEN_BIT))
        FrameRoundRect(&r, 16, 16);
    else
        FrameRect(&r);
}

void Executor::C_ROMlib_orientation(DialogPtr dp, INTEGER which)
{
    Rect r;
    GUEST<INTEGER> unused;

    GetDItem(dp, which, &unused, NULL, &r);
    PenNormal();
    PenSize(1, 1);
    InsetRect(&r, 1, 1);
    if((which == LAYOUT_PORTRAIT_NO) != (ROMlib_paper_orientation == "Portrait"))
        PenPat(white);
    FrameRect(&r);
    PenPat(black);
}

std::string Executor::ROMlib_PrintersIni;
std::string Executor::ROMlib_PrintDef;

static void
adjust_menu_common(TPPrDlg dlg, INTEGER item, heading_t heading, ini_key_t defkey)
{
    ControlHandle ch;
    MenuHandle mh;
#if defined(MSDOS) || defined(CYGWIN32)
    bool skip_all_but_a_few;

    if(strcmp(heading, "Printer") != 0)
        skip_all_but_a_few = false;
    else
    {
        char *gs_dll;
        struct stat sbuf;

        gs_dll = get_gs_dll(NULL);
        skip_all_but_a_few = stat(gs_dll, &sbuf) != 0;
        free(gs_dll);
    }
#endif

    ch = GetDControl((DialogPtr)dlg, item);
    if(ch)
    {
        mh = GetPopUpMenu(ch);
        if(mh)
        {
            {
                int i;
                pair_link_t *pairp;
                int default_index;
                GrafPtr gp;
                INTEGER max_wid;

                gp = thePort;
                SetPort((GrafPtr)dlg);
                default_index = -1;
                max_wid = 0;
                for(i = 0; (pairp = get_pair_link_n(heading, i)); ++i)
                {
                    Str255 str;
                    INTEGER wid;

#if defined(MSDOS) || defined(CYGWIN32)
                    if(skip_all_but_a_few
                       && strcmp(pairp->key, "Direct To Port") != 0
                       && strcmp(pairp->key, "PostScript File") != 0)
                        continue;
#endif

                    if(pairp->key == defkey)
                        default_index = i;
                    str255assignc(str, pairp->key.c_str());
                    AppendMenu(mh, str);
                    wid = StringWidth(str);
                    if(wid > max_wid)
                        max_wid = wid;
                }
                if(max_wid)
                {
                    GUEST<Handle> h;
                    Rect r;
                    GUEST<INTEGER> unused;

                    GetDItem((DialogPtr)dlg, item, &unused, &h, &r);
                    r.right = CW(CW(r.left) + max_wid + 38);
                    SetDItem((DialogPtr)dlg, item, ctrlItem, MR(h), &r);
                    SizeControl(ch, CW(r.right) - CW(r.left),
                                CW(r.bottom) - CW(r.top));
                }
                SetCtlMax(ch, i);
                if(default_index > -1)
                    SetCtlValue(ch, default_index + 1);
                SetPort(gp);
            }
        }
    }
}

static void
adjust_paper_menu(TPPrDlg dlg)
{
    adjust_menu_common(dlg, LAYOUT_PAPER_NO, "Paper Size", ROMlib_paper);
}

static void
adjust_printer_type_menu(TPPrDlg dlg)
{
    adjust_menu_common(dlg, LAYOUT_PRINTER_TYPE_NO, "Printer", ROMlib_printer);
}

static void
adjust_port(TPPrDlg dlg)
{
#if !defined(LINUX)
    adjust_menu_common(dlg, LAYOUT_PORT_MENU_NO, "Port", ROMlib_port);
#else
    HideDItem((DialogPtr)dlg, LAYOUT_PORT_LABEL_NO);
    HideDItem((DialogPtr)dlg, LAYOUT_PORT_MENU_NO);
    ROMlib_port = "";
#endif
}

static void
set_default_orientation(TPPrDlg dlg)
{
    INTEGER orientation_button;

    orientation_button = (ROMlib_paper_orientation != "" && ROMlib_paper_orientation == "Landscape")
        ? LAYOUT_LANDSCAPE_ICON_NO
        : LAYOUT_PORTRAIT_ICON_NO;
    update_orientation((DialogPtr)dlg, orientation_button);
}

void
Executor::printer_init(void)
{
    static bool ini_read_p = false;

    if(!ini_read_p)
    {
        read_ini_file(ROMlib_PrintersIni.c_str());
        read_ini_file(ROMlib_PrintDef.c_str());
        get_all_defaults();
        ini_read_p = true;
    }
}

TPPrDlg Executor::C_PrStlInit(THPrint hPrint)
{
    TPPrDlg retval;

    printer_init();
    if(ROMlib_printresfile != -1)
    {
        retval = (TPPrDlg)NewPtr(sizeof(TPrDlg));
        if(GetNewDialog(-8192, (Ptr)retval, (WindowPtr)-1))
        {

            filename_chosen_p = false;
            print_where = PRINT_TO_PORT;

            HideDItem((DialogPtr)retval, LAYOUT_FILENAME_LABEL_NO);
            HideDItem((DialogPtr)retval, LAYOUT_FILENAME_NO);

            set_userItem((DialogPtr)retval, LAYOUT_CIRCLE_OK_NO,
                         &ROMlib_circle_ok);

            set_userItem((DialogPtr)retval, LAYOUT_PORTRAIT_NO,
                         &ROMlib_orientation);

            set_userItem((DialogPtr)retval, LAYOUT_LANDSCAPE_NO,
                         &ROMlib_orientation);

            {
                Str255 appname;

                str255assignc(appname, ROMlib_appname.c_str());

                if(!(ROMlib_options & ROMLIB_NOLOWER_BIT))
                {
                    char *p;
                    bool all_upper;
                    int n;

                    /* convert all upper (e.g. EXECUTOR.EXE) to 
		 all lower (e.g. executor.exe) */

                    all_upper = true;
                    for(p = (char *)appname + 1, n = appname[0];
                        n > 0 && all_upper; ++p, --n)
                        if(islower(*p))
                            all_upper = false;
                    if(all_upper)
                        for(p = (char *)appname + 1, n = appname[0];
                            n > 0; ++p, --n)
                            *p = tolower(*p);
                }

/* remove trailing .exe */
#define EXE_SUFFIX ".exe"
                if(strcasecmp((char *)appname + appname[0] + 1
                                  - sizeof EXE_SUFFIX + 1,
                              EXE_SUFFIX)
                   == 0)
                    appname[0] -= sizeof EXE_SUFFIX - 1;

                /* Capitalize first character */
                if(islower(appname[1]))
                    appname[1] = toupper((unsigned char)appname[1]);

                ParamText(appname, 0, 0, 0);
            }
            if(ROMlib_new_printer_name || ROMlib_new_label)
            {
                Rect r;
                GUEST<INTEGER> item_type;
                GUEST<Handle> hh;
                Handle h;
                Str255 str;
                int orig, new1;
                Str255 new_printer_name;
                Str255 new_type_label;

#define NEW_PRINTER_NAME "Print Selection"
#define NEW_TYPE_LABEL "Print to:"

                str255_from_c_string(new_printer_name, ROMlib_new_printer_name ? ROMlib_new_printer_name : NEW_PRINTER_NAME);
                str255_from_c_string(new_type_label, ROMlib_new_label ? ROMlib_new_label : NEW_TYPE_LABEL);

                GetDItem((DialogPtr)retval, LAYOUT_PRINTER_TYPE_LABEL_NO,
                         &item_type, &hh, &r);

                h = MR(hh);
                GetIText(h, str);
                orig = StringWidth(str);
                new1 = StringWidth(new_type_label);
                HUnlock(h);

                r.left = CW(CW(r.left) + orig - new1);
                SetDItem((DialogPtr)retval, LAYOUT_PRINTER_TYPE_LABEL_NO,
                         CW(item_type), h, &r);
                SetIText(GetDIText((DialogPtr)retval, LAYOUT_PRINTER_NAME_NO),
                         new_printer_name);
                SetIText(GetDIText((DialogPtr)retval,
                                   LAYOUT_PRINTER_TYPE_LABEL_NO),
                         new_type_label);
            }
            adjust_paper_menu(retval);
            adjust_printer_type_menu(retval);
            update_port((DialogPtr)retval);
            adjust_port(retval);
            set_default_orientation(retval); /* must be called after paper
					       menu is adjusted */

            retval->pFltrProc = RM(&ROMlib_stlfilterproc);
            retval->pItemProc = RM(&ROMlib_mystlproc);
            retval->hPrintUsr = RM(hPrint);
        }
        else
        {
            DisposPtr((Ptr)retval);
            retval = 0;
        }
    }
    else
        retval = 0;
    return retval;
}

#define SUNPATH_HACK (ROMlib_options & ROMLIB_PRINTING_HACK_BIT)

BOOLEAN Executor::C_PrDlgMain(THPrint hPrint, ProcPtr initfptr)
{
    GUEST<INTEGER> item_swapped;
    INTEGER item;
    TPPrDlg prrecptr;
    BOOLEAN retval;

    printer_init();
    retval = false;
#if 0 && defined(MACOSX_)
/*
 * NOTE: we don't actually call this because Excel sets up some goofy pages
 *	 and then we get way confused.
 */
    ROMlib_updatenextpagerect(&Hx(hPrint, rPaper));
#endif /* defined(MACOSX_) */
    if((prrecptr = CALLPRINITPROC(hPrint, (prinitprocp) initfptr)))
    {
        if(!SUNPATH_HACK || (((pritemprocp)MR(prrecptr->pItemProc)
                              != (pritemprocp)&ROMlib_myjobproc)
                             || ROMlib_printer != std::string(WIN32_TOKEN)))
        {
            ShowWindow((WindowPtr)prrecptr);
            SelectWindow((WindowPtr)prrecptr);
        }
        do
        {
            if(SUNPATH_HACK && (((pritemprocp)MR(prrecptr->pItemProc)
                                 == (pritemprocp)&ROMlib_myjobproc)
                                && ROMlib_printer == std::string(WIN32_TOKEN)))
                item = 1;
            else
            {
                ModalDialog(MR(prrecptr->pFltrProc), &item_swapped);
                item = CW(item_swapped);
            }
            CALLPRITEMPROC(prrecptr, item, MR(prrecptr->pItemProc));

#if defined(CYGWIN32)
            /* Don't allow them to continue Win32 stuff if we can't
	       initialize the Win32 subsystem */
            if(((pritemprocp)MR(prrecptr->pItemProc)
                == (pritemprocp)&ROMlib_myjobproc)
               && item == 1
               && strcmp(ROMlib_printer, WIN32_TOKEN) == 0
               && !ROMlib_wp)
            {
                if(SUNPATH_HACK)
                    item = 2;
                else
                    item = 999;
            }
#endif
        } while(item != 1 && item != 2);
        if(item == 1)
        {
            /* TODO: transfer data from prrecptr into hPrint */
            C_PrValidate(hPrint);
            /* TODO: if PrValidate returns true maybe ModalDialog should be called again.*/
            retval = true;
        }
        else
            retval = false;
        CloseDialog((DialogPtr)prrecptr);
        DisposPtr((Ptr)prrecptr);
    }
#if defined(MACOSX_)
    ROMlib_updatemacpagerect(&HxX(hPrint, rPaper),
                             &HxX(hPrint, prInfo.rPage),
                             &HxX(hPrint, prInfoPT.rPage));
#endif /* defined(MACOSX_) */
    return retval;
}

void Executor::C_PrGeneral(Ptr pData) /* IMV-410 */
{
    TGnlData *tgp;

    tgp = (TGnlData *)pData;

    ((TGnlData *)pData)->iError = CWC(OpNotImpl);
    switch(CW(tgp->iOpCode))
    {
        case GetRslData:
        {
            TGetRslBlk *resolp;

            resolp = (TGetRslBlk *)pData;
            resolp->iError = CWC(noErr);
            resolp->iRgType = CWC(1);
            resolp->xRslRg.iMin = CWC(0);
            resolp->xRslRg.iMax = CWC(0);
            resolp->yRslRg.iMin = CWC(0);
            resolp->yRslRg.iMax = CWC(0);
            resolp->rgRslRec[0].iXRsl = CWC(72);
            resolp->rgRslRec[0].iYRsl = CWC(72);
            if(ROMlib_optional_res_x <= 0 || ROMlib_optional_res_y <= 0)
                resolp->iRslRecCnt = CWC(1);
            else
            {
                resolp->iRslRecCnt = CWC(2);
                resolp->rgRslRec[1].iXRsl = CW(ROMlib_optional_res_x);
                resolp->rgRslRec[1].iYRsl = CW(ROMlib_optional_res_y);
            }
        }
        break;
        case SetRsl:
        {
            TSetRslBlk *resolp;

            resolp = (TSetRslBlk *)pData;
            if(!((resolp->iXRsl == CWC(72) && resolp->iYRsl == CWC(72)) || (resolp->iXRsl == CW(ROMlib_optional_res_x) && resolp->iYRsl == CW(ROMlib_optional_res_y))))
                resolp->iError = CWC(NoSuchRsl);
            else
            {
                resolp->iError = CWC(noErr);
                ROMlib_set_default_resolution(MR(resolp->hPrint),
                                              CW(resolp->iYRsl),
                                              CW(resolp->iXRsl));
            }
        }
        break;
        case DraftBits:
            warning_unimplemented("DraftBits");
            break;
        case NoDraftBits:
            warning_unimplemented("NoDraftBits");
            break;
        case GetRotn:
            warning_unimplemented("GetRotn");
            break;
        default:
            warning_unimplemented(NULL_STRING);
            break;
    }
}
