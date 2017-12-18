/* Copyright 1998 - 2004 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"

#include "SegmentLdr.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "win_print.h"
#include "win_print_private.h"
#include "rsys/gestalt.h"
#include "rsys/error.h"
#include "rsys/float_fcw.h"
#include "rsys/segment.h"
#include "rsys/launch.h"
#include "rsys/option.h"

/*
 * Some of the Win32 routines smash the floating point environment registers,
 * which can lead to Executor blowing up when it deliberately creates or
 * accesses NaNs.
 */

#define SAVE_FP_ENVIRONMENT() uint32_t __save_fpe = ROMlib_get_fcw_fsw()
#define RESTORE_FP_ENVIRONMENT() ROMlib_set_fcw_fsw(__save_fpe)

typedef struct
{
    int x_pts;
    int y_pts;
} win_page_size_t;

PRIVATE win_page_size_t
    paper_sizes[]
    = {
        {
            INCHES(8.5), INCHES(11),
        }, /* DMPAPER_LETTER 1 */
        {
            INCHES(8.5), INCHES(11),
        }, /* DMPAPER_LETTERSMALL 2 */
        {
            INCHES(11), INCHES(17),
        }, /* DMPAPER_TABLOID 3 */
        {
            INCHES(17), INCHES(11),
        }, /* DMPAPER_LEDGER 4 */
        {
            INCHES(8.5), INCHES(14),
        }, /* DMPAPER_LEGAL 5 */
        {
            INCHES(5.5), INCHES(8.5),
        }, /* DMPAPER_STATEMENT 6 */
        {
            INCHES(7.25), INCHES(10.5),
        }, /* DMPAPER_EXECUTIVE 7 */
        {
            MMETERS(297), MMETERS(420),
        }, /* DMPAPER_A3 8 */
        {
            MMETERS(210), MMETERS(297),
        }, /* DMPAPER_A4 9 */
        {
            MMETERS(210), MMETERS(297),
        }, /* DMPAPER_A4SMALL 10 */
        {
            MMETERS(148), MMETERS(210),
        }, /* DMPAPER_A5 11 */
        {
            MMETERS(250), MMETERS(354),
        }, /* DMPAPER_B4 12 */
        {
            MMETERS(182), MMETERS(257),
        }, /* DMPAPER_B5 13 */
        {
            INCHES(8.5), INCHES(13),
        }, /* DMPAPER_FOLIO 14 */
        {
            MMETERS(215), MMETERS(275),
        }, /* DMPAPER_QUARTO 15 */
        {
            INCHES(10), INCHES(14),
        }, /* DMPAPER_10X14 16 */
        {
            INCHES(11), INCHES(17),
        }, /* DMPAPER_11X17 17 */
        {
            INCHES(8.5), INCHES(11),
        }, /* DMPAPER_NOTE 18 */
        {
            INCHES(3.875), INCHES(8.875),
        }, /* DMPAPER_ENV_9 19 */
        {
            INCHES(4.125), INCHES(9.5),
        }, /* DMPAPER_ENV_10 20 */
        {
            INCHES(4.5), INCHES(10.375),
        }, /* DMPAPER_ENV_11 21 */
        {
            INCHES(4.75), INCHES(11),
        }, /* DMPAPER_ENV_12 22 */
        {
            INCHES(5), INCHES(11.5),
        }, /* DMPAPER_ENV_14 23 */
        {
            INCHES(17), INCHES(22),
        }, /* DMPAPER_CSHEET 24 */
        {
            INCHES(22), INCHES(34),
        }, /* DMPAPER_DSHEET 25 */
        {
            INCHES(34), INCHES(44),
        }, /* DMPAPER_ESHEET 26 */
        {
            MMETERS(110), MMETERS(220),
        }, /* DMPAPER_ENV_DL 27 */
        {
            MMETERS(162), MMETERS(229),
        }, /* DMPAPER_ENV_C5 28 */
        {
            MMETERS(324), MMETERS(458),
        }, /* DMPAPER_ENV_C3 29 */
        {
            MMETERS(229), MMETERS(324),
        }, /* DMPAPER_ENV_C4 30 */
        {
            MMETERS(114), MMETERS(162),
        }, /* DMPAPER_ENV_C6 31 */
        {
            MMETERS(114), MMETERS(229),
        }, /* DMPAPER_ENV_C65 32 */
        {
            MMETERS(250), MMETERS(353),
        }, /* DMPAPER_ENV_B4 33 */
        {
            MMETERS(176), MMETERS(250),
        }, /* DMPAPER_ENV_B5 34 */
        {
            MMETERS(176), MMETERS(125),
        }, /* DMPAPER_ENV_B6 35 */
        {
            MMETERS(110), MMETERS(230),
        }, /* DMPAPER_ENV_ITALY 36 */
        {
            INCHES(3.875), INCHES(7.5),
        }, /* DMPAPER_ENV_MONARCH 37 */
        {
            INCHES(3.625), INCHES(6.5),
        }, /* DMPAPER_ENV_PERSONAL 38 */
        {
            INCHES(14.875), INCHES(11),
        }, /* DMPAPER_FANFOLD_US 39 */
        {
            INCHES(8.5), INCHES(12),
        }, /* DMPAPER_FANFOLD_STD_GERMAN 40 */
        {
            INCHES(8.5), INCHES(13),
        }, /* DMPAPER_FANFOLD_LGL_GERMAN 41 */
      };

PRIVATE HWND
main_window(void)
{
    HWND retval;

    retval = GetTopWindow(NULL);
    return retval;
}

/* ugly globals needed because the callback doesn't pass in a user
   supplied argument */

PRIVATE HDC global_hdc;
PRIVATE LONG global_right;
PRIVATE LONG global_bottom;

PRIVATE int GSDLLAPI (*gsdll_revision)(char **product, char **copyright,
                                       long *gs_revision,
                                       long *gs_revisiondate);

PRIVATE int GSDLLAPI (*gsdll_init)(GSDLL_CALLBACK callback, HWND hwnd,
                                   int argc, const char *argv[]);

PRIVATE int GSDLLAPI (*gsdll_execute_begin)(void);

PRIVATE int GSDLLAPI (*gsdll_execute_cont)(const char *str, int len);

PRIVATE int GSDLLAPI (*gsdll_execute_end)(void);

PRIVATE int GSDLLAPI (*gsdll_exit)(void);

PRIVATE int GSDLLAPI (*gsdll_lock_device)(unsigned char *device, int flag);

PRIVATE HGLOBAL GSDLLAPI (*gsdll_copy_dib)(unsigned char *device);

PRIVATE HPALETTE GSDLLAPI (*gsdll_copy_palette)(unsigned char *device);

PRIVATE void GSDLLAPI (*gsdll_draw)(unsigned char *device, HDC hdc,
                                    LPRECT dest, LPRECT src);

PRIVATE int GSDLLAPI (*gsdll_get_bitmap_row)(unsigned char *device,
                                             LPBITMAPINFOHEADER pbmih,
                                             RGBQUAD *prgbquad, LPBYTE *ppbyte,
                                             unsigned int row);

#define GETPROCADDRESS(lib, func)                      \
    do                                                 \
    {                                                  \
        if(lib)                                        \
        {                                              \
            func = (void *)GetProcAddress(lib, #func); \
            if(!func)                                  \
            {                                          \
                FreeLibrary(lib);                      \
                lib = NULL;                            \
            }                                          \
        }                                              \
    } while(0)

typedef struct
{
    int major;
    int minor;
    char *dll_key;
    char *dll_valuename;
    char *lib_valuename;
    char *lib_string;
} dll_info_t;

/* the defaults are from the bad-old days before other installers were
   released that provided the new key system we use */

enum
{
    DEFAULT_MAJOR = 4,
    DEFAULT_MINOR = 3
};
#define DEFAULT_DLL_KEY \
    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\gsdll32.dll"
#define DEFAULT_DLL_VALUENAME ""

PRIVATE char *
combine_key(const char *left, const char *right)
{
    char *retval;

    retval = malloc(strlen(left) + 1 + strlen(right) + 1);
    if(retval)
        sprintf(retval, "%s\\%s", left, right);
    return retval;
}

PRIVATE char *
valid_valuename(HKEY key, const char *valuename, char **valuep)
{
    char *retval;
    DWORD type, dlen;
    char buf[1024];

    retval = NULL;
    if(valuep)
    {
        free(*valuep);
        *valuep = NULL;
    }
    dlen = sizeof buf;
    if((RegQueryValueEx(key, valuename, NULL, &type, buf, &dlen)
        == ERROR_SUCCESS)
       && type == REG_SZ)
    {
        retval = strdup(valuename);
        if(valuep)
        {
            buf[sizeof buf - 1] = 0;
            *valuep = strdup(buf);
        }
    }

    return retval;
}

PRIVATE void
free_dll_info(dll_info_t *dllp)
{
    free(dllp->dll_key);
    free(dllp->dll_valuename);
    free(dllp->lib_valuename);
    free(dllp->lib_string);
    memset(dllp, 0, sizeof *dllp);
}

/*
 * sanity check the values stored in dllp, including stating the DLL
 * file.  If we pass the sanity tests, fill in *libstringpp and return
 * a string containing the DLL path.  Otherwise, clear out the info
 * pointed to by dllp and retrun NULL
 */

PRIVATE char *
string_from_dllp(dll_info_t *dllp, char **libstringpp)
{
    HKEY key;
    char *retval;

    retval = NULL;
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, dllp->dll_key, 0, KEY_READ, &key)
       != ERROR_SUCCESS)
        /* failure -- forget the cached info; it's useless */
        free_dll_info(dllp);
    else
    {
        char val[4096];
        DWORD val_len;

        val_len = sizeof val;
        if(RegQueryValueEx(key, dllp->dll_valuename, NULL, NULL, val,
                           &val_len)
           == ERROR_SUCCESS)
        {
            int len;
            struct stat sbuf;

            len = strlen(val);
            retval = malloc(len + 1);
            memcpy(retval, val, len);
            retval[len] = 0;
            if(stat(retval, &sbuf) == 0)
            {
                if(libstringpp && dllp->lib_string)
                    *libstringpp = strdup(dllp->lib_string);
            }
            else
            {
                free_dll_info(dllp);
                free(retval);
                retval = NULL;
            }
        }
        RegCloseKey(key);
    }

    return retval;
}

PRIVATE char *
new_find_preferred_dll(const char *name, dll_info_t *dllp, char **libstringpp)
{
    char *retval;
    char *key_name;

    retval = NULL;
    key_name = combine_key("SOFTWARE", name);
    if(key_name)
    {
        HKEY key;

        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, key_name, 0, KEY_READ, &key)
           == ERROR_SUCCESS)
        {
            DWORD i;
            char buf[32]; /* we're looking for keys of the form m.n */

            for(i = 0;
                RegEnumKey(key, i, buf, sizeof buf) == ERROR_SUCCESS;
                ++i)
            {
                int major, minor, n_found;

                buf[sizeof buf - 1] = 0;
                n_found = sscanf(buf, "%d.%d", &major, &minor);
                if(n_found == 2 && (major > dllp->major || (major == dllp->major && minor > dllp->minor)))
                {
                    char *new_key_name;

                    new_key_name = combine_key(key_name, buf);
                    if(new_key_name)
                    {
                        HKEY new_key;
                        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, new_key_name, 0,
                                        KEY_READ, &new_key)
                           == ERROR_SUCCESS)
                        {
                            char *dll_valuename;
                            char *new_retval;

                            dll_valuename = valid_valuename(new_key, "GS_DLL",
                                                            NULL);
                            if(dll_valuename)
                            {
                                free_dll_info(dllp);
                                dllp->dll_valuename = dll_valuename;
                                dllp->lib_valuename
                                    = valid_valuename(new_key, "GS_LIB",
                                                      &dllp->lib_string);
                                dllp->dll_key = strdup(new_key_name);
                                dllp->major = major;
                                dllp->minor = minor;
                                new_retval = string_from_dllp(dllp, libstringpp);
                                if(new_retval)
                                    retval = new_retval;
                            }
                            RegCloseKey(new_key);
                        }
                        free(new_key_name);
                    }
                }
            }
            /* "SOFTWARE" */
            RegCloseKey(key);
        }
        free(key_name);
    }

    return retval;
}

/*
 * This find_preferred_dll stuff is clumsily written because it was added
 * at the last minute before releasing 2.1 pre-release 0.  The goal was to
 * change the existing code as little as possible and still pick up the
 * registry values that the stock Aladdin Ghostscript installer gives us.
 */

PRIVATE char *
find_preferred_dll(char **libstringpp)
{
    dll_info_t dll_info;
    char *software_keys[] = {
        "AFPL Ghostscript",
        "Aladdin Ghostscript",
        "GNU Ghostscript",
        "Aladdin\\Ghostscript",
        "GNU\\Ghostscript",
    };
    int i;
    char *retval;

    retval = NULL;
    memset(&dll_info, 0, sizeof dll_info);
    for(i = 0; !retval && i < (int)NELEM(software_keys); ++i)
        retval = new_find_preferred_dll(software_keys[i], &dll_info, libstringpp);

    if(!retval) /* no new one -- revert to old defaults */
    {
        free_dll_info(&dll_info);
        dll_info.major = DEFAULT_MAJOR;
        dll_info.minor = DEFAULT_MINOR;
        dll_info.dll_key = strdup(DEFAULT_DLL_KEY);
        dll_info.dll_valuename = strdup(DEFAULT_DLL_VALUENAME);
        dll_info.lib_valuename = NULL;
        dll_info.lib_string = NULL;
        retval = string_from_dllp(&dll_info, libstringpp);
    }

    return retval;
}

#define GS_DEFAULT "C:\\gs\\gsdll32.dll"

PUBLIC char *
get_gs_dll(char **libstringpp)
{
    char *retval;

    SAVE_FP_ENVIRONMENT();
    retval = find_preferred_dll(libstringpp);
    if(!retval)
    {
        if(libstringpp)
            *libstringpp = NULL;
        retval = strdup(GS_DEFAULT);
    }
    RESTORE_FP_ENVIRONMENT();

    return retval;
}

PUBLIC void
complain_if_no_ghostscript(void)
{
    FILE *fp;

    SAVE_FP_ENVIRONMENT();
    warning_trace_info("about to check for nogs.txt");
    fp = executor_dir_fopen("nogs.txt", "r");
    if(fp)
    {
        char *gs;
        char buf[4096];
        size_t nread;

        nread = fread(buf, 1, sizeof buf - 1, fp);
        warning_trace_info("nread = %d", nread);
        fclose(fp);
        if(nread > 0)
        {
            gs = find_preferred_dll(NULL);
            warning_trace_info("gs = %s", gs ? gs : "NULL");
            if(gs)
                free(gs);
            else
            {
                int reply;

                buf[nread] = 0;
                reply = MessageBox(NULL, buf, "Printing Disabled", MB_OKCANCEL);
                if(reply != IDOK)
                {
                    ROMlib_exit = true;
                    C_ExitToShell();
                }
            }
        }
    }
    RESTORE_FP_ENVIRONMENT();
}

PRIVATE HINSTANCE lib;

PRIVATE void
loadgs(void)
{
    char *gs_dll;

    gs_dll = get_gs_dll(NULL);

    lib = LoadLibrary(gs_dll);
    GETPROCADDRESS(lib, gsdll_revision);
    GETPROCADDRESS(lib, gsdll_init);
    GETPROCADDRESS(lib, gsdll_execute_begin);
    GETPROCADDRESS(lib, gsdll_execute_cont);
    GETPROCADDRESS(lib, gsdll_execute_end);
    GETPROCADDRESS(lib, gsdll_exit);
    GETPROCADDRESS(lib, gsdll_lock_device);
    GETPROCADDRESS(lib, gsdll_copy_dib);
    GETPROCADDRESS(lib, gsdll_copy_palette);
    GETPROCADDRESS(lib, gsdll_draw);
    GETPROCADDRESS(lib, gsdll_get_bitmap_row);
    free(gs_dll);
}

PRIVATE void
unloadgs(void)
{
    if(lib)
    {
        FreeLibrary(lib);
        lib = NULL;
    }

    gsdll_revision = NULL;
    gsdll_init = NULL;
    gsdll_execute_begin = NULL;
    gsdll_execute_cont = NULL;
    gsdll_execute_end = NULL;
    gsdll_exit = NULL;
    gsdll_lock_device = NULL;
    gsdll_copy_dib = NULL;
    gsdll_copy_palette = NULL;
    gsdll_draw = NULL;
    gsdll_get_bitmap_row = NULL;
}

PRIVATE bool
release_info(win_printp_t wp, uint32_t *last_errorp)
{
    bool retval;

    free(wp);

    retval = true;
    return retval;
}

PRIVATE int
gsdll_callback(int message, char *str, unsigned long count)
{
    switch(message)
    {
        case GSDLL_PAGE:
            StartPage(global_hdc);
            gsdll_lock_device(str, 1);
            {
                struct
                {
                    BITMAPINFOHEADER h PACKED;
                    RGBQUAD colors[2] PACKED;
                } bmi;
                LPBYTE bytep;

                bmi.colors[0].rgbRed = 0;
                bmi.colors[0].rgbGreen = 0;
                bmi.colors[0].rgbBlue = 0;
                bmi.colors[1].rgbRed = ~0;
                bmi.colors[1].rgbGreen = ~0;
                bmi.colors[1].rgbBlue = ~0;
                gsdll_get_bitmap_row(str, &bmi.h, NULL, &bytep, 0);

#if !defined(DIB_PAL_INDICES)
#define DIB_PAL_INDICES 2
#endif

                SetDIBitsToDevice(global_hdc, 0, 0, global_right, global_bottom,
                                  0, 0, 0, global_bottom, bytep,
                                  (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
            }
            gsdll_lock_device(str, 0);
            EndPage(global_hdc);
            break;
        case GSDLL_STDOUT:
            if(str != NULL)
                fwrite(str, 1, count, stdout);
            break;
        case GSDLL_DEVICE:
            if(count == 0)
            {
                EndDoc(global_hdc);
            }
            break;
        default:
            /* ignore */
            break;
    }
    fflush(stdout);
    return 0;
}

PUBLIC void
set_gs_gestalt_info(void)
{
    SAVE_FP_ENVIRONMENT();

    if(!gsdll_revision)
        loadgs();
    if(gsdll_revision)
    {
        char *product; /* should these be freed? */
        char *copyright; /* should these be freed? */
        long revision;
        long revisiondate;

        gsdll_revision(&product, &copyright, &revision, &revisiondate);
        replace_physgestalt_selector(gestaltGhostScriptVersion, revision);
    }
    RESTORE_FP_ENVIRONMENT();
}

PUBLIC bool
get_info(win_printp_t *wpp, int physx, int physy,
         orientation_t orientation, int copies, uint32_t *last_errorp)
{
    bool retval;
    win_printp_t wp;

    SAVE_FP_ENVIRONMENT();
    wp = NULL;
    if(!gsdll_revision)
        loadgs();
    if(!gsdll_revision)
    {
        retval = false;
        if(last_errorp)
            *last_errorp = GetLastError();
    }
    else
    {
        wp = malloc(sizeof *wp);
        if(!wp)
        {
            retval = false;
            if(last_errorp)
                *last_errorp = MALLOC_FAILED;
        }
        else
        {
            PRINTDLG pd;

            memset(wp, 0, sizeof *wp);

#warning TODO preload dialog to use physx, physy and orientation

            memset(&pd, 0, sizeof pd);
            pd.nCopies = copies;
            /* NOTE: pd.nCopies appears to be ignored */
            pd.hwndOwner = main_window();
            pd.lStructSize = sizeof pd;
            pd.Flags = (PD_HIDEPRINTTOFILE | PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC | PD_USEDEVMODECOPIES);
            retval = PrintDlg(&pd);
            if(!retval)
            {
                if(last_errorp)
                    *last_errorp = CommDlgExtendedError();
            }
            else
            {
                win_print_info_t *infop;
                int raster_caps;

                infop = &wp->info;
                infop->hDC = pd.hDC;
                raster_caps = GetDeviceCaps(infop->hDC, RASTERCAPS);

                if(!(raster_caps & RC_DIBTODEV))
                {
                    DeleteDC(infop->hDC);
                    retval = false;
                    if(last_errorp)
                        *last_errorp = NO_DIBTODEV;
                }
                else
                {
                    DEVMODE *dmp;

                    infop->hres = GetDeviceCaps(pd.hDC, LOGPIXELSX);
                    infop->vres = GetDeviceCaps(pd.hDC, LOGPIXELSY);
                    infop->printx = GetDeviceCaps(pd.hDC, HORZRES);
                    infop->printy = GetDeviceCaps(pd.hDC, VERTRES);
                    dmp = GlobalLock(pd.hDevMode);

                    if(dmp->dmFields & DM_ORIENTATION)
                        infop->orientation = dmp->dmOrientation;
                    else
                        infop->orientation = DMORIENT_PORTRAIT;

                    if((dmp->dmFields & DM_PAPERWIDTH) && dmp->dmPaperWidth)
                        infop->physx = MMETERS(dmp->dmPaperWidth) / 10;
                    else if((unsigned)dmp->dmPaperSize < NELEM(paper_sizes))
                        infop->physx = paper_sizes[dmp->dmPaperSize].x_pts;
                    else
                        infop->physx = INCHES(8.5);

                    if((dmp->dmFields & DM_PAPERLENGTH) && dmp->dmPaperLength)
                        infop->physy = MMETERS(dmp->dmPaperLength) / 10;
                    else if((unsigned)dmp->dmPaperSize < NELEM(paper_sizes))
                        infop->physy = paper_sizes[dmp->dmPaperSize].y_pts;
                    else
                        infop->physy = INCHES(11);

                    {
                        int lesser, greater;

                        lesser = MIN(infop->physx, infop->physy);
                        greater = MAX(infop->physx, infop->physy);

                        if(infop->orientation == DMORIENT_PORTRAIT)
                        {
                            infop->physx = lesser;
                            infop->physy = greater;
                        }
                        else
                        {
                            infop->physx = greater;
                            infop->physy = lesser;
                        }
                    }

                    if((dmp->dmFields & DM_SCALE) && dmp->dmScale)
                        infop->scale = dmp->dmScale / 100.0;
                    else
                        infop->scale = 1.0;

                    if((dmp->dmFields & DM_COPIES) && dmp->dmCopies)
                        infop->copies = dmp->dmCopies;
                    else
                        infop->copies = 1;

                    GlobalUnlock(pd.hDevMode);

                    /* Do I need to delete pd.hDevMode and pd.hDevNames? */

                    {
                        global_hdc = pd.hDC;
                        global_right = wp->info.printx;
                        global_bottom = wp->info.printy;
                    }
                    retval = true;
                }
            }
        }
    }
    if(retval)
        *wpp = wp;
    else
    {
        if(wp)
            free(wp);
        *wpp = NULL;
    }
    RESTORE_FP_ENVIRONMENT();
    return retval;
}

PUBLIC bool
print_file(win_printp_t wp, const char *spool_namep, uint32_t *last_errorp)
{
    bool retval;

    SAVE_FP_ENVIRONMENT();
    if(!wp)
    {
        retval = false;
        *last_errorp = NIL_HANDLE;
    }
    else
    {
        DOCINFO di;
        int job_identifier;

        memset(&di, 0, sizeof di);
        di.cbSize = sizeof di;
        di.lpszDocName = "Document";
        job_identifier = StartDoc(wp->info.hDC, &di);
        if(job_identifier <= 0)
        {
            retval = false;
            if(last_errorp)
                *last_errorp = GetLastError();
        }
        else
        {
            static const char *argv[] = {
                "gs",
                NULL, /* "-IC:\\GS;C:\\GS\\FONTS", */
                "-q",
                "-dNOPAUSE",
                "-sDEVICE=mswindll",
                "-dBitsPerPixel=1",
                NULL, /* -g%dx%d (papersize) */
                NULL, /* -r%dx%d (resolution)*/
                NULL,
            };
            char *papersize;
            char *resolution;

            {
                char *gs_dll;
                char *last_slash;
                int len;
                char *argv1;
                char *libstring;

                libstring = NULL;
                gs_dll = get_gs_dll(&libstring);
                if(libstring)
                {
#define MINUS_I_STRING "-I%s"
                    len = sizeof MINUS_I_STRING + strlen(libstring);
                    argv1 = alloca(len);
                    sprintf(argv1, MINUS_I_STRING, libstring);
                    free(libstring);
                }
                else
                {
#undef MINUS_I_STRING
#define MINUS_I_STRING "-I%s;%s\\FONTS"
                    last_slash = strrchr(gs_dll, '\\');
                    *last_slash = 0;
                    len = sizeof MINUS_I_STRING + strlen(gs_dll) + strlen(gs_dll);
                    argv1 = alloca(len);
                    sprintf(argv1, MINUS_I_STRING, gs_dll, gs_dll);
                }
                argv[1] = argv1;
                free(gs_dll);
            }

            papersize = alloca(128);
            sprintf(papersize, "-g%dx%d", wp->info.hres * wp->info.physx / 72,
                    wp->info.vres * wp->info.physy / 72);

            resolution = alloca(128);
            sprintf(resolution, "-r%dx%d", wp->info.hres, wp->info.vres);

            argv[6] = papersize;
            argv[7] = resolution;

            if(gsdll_init(gsdll_callback, NULL, NELEM(argv) - 1, argv) != 0)
            {
                retval = false;
                if(last_errorp)
                    *last_errorp = GSDLL_INIT_FAILED;
            }
            else
            {
                int code;
                char *initial_command;
                int n;
                float margin_x, margin_y;

                margin_x = ((global_right - ((float)wp->info.hres * wp->info.physx / 72))
                            / (2 * wp->info.hres));
                margin_y = ((global_bottom - ((float)wp->info.vres * wp->info.physy / 72))
                            / (2 * wp->info.vres));

#define INITIAL_COMMAND                                         \
    "<< /Margins [ "                                            \
    "%f currentpagedevice /.MarginsHWResolution get 0 get mul " \
    "%f currentpagedevice /.MarginsHWResolution get 1 get mul " \
    "] >> setpagedevice (%s) run\n"

                initial_command = alloca(sizeof INITIAL_COMMAND + 20 + 20
                                         + strlen(spool_namep) + 100);
                n = sprintf(initial_command, INITIAL_COMMAND,
                            margin_x, margin_y, spool_namep);
                {
                    char *p;

                    for(p = initial_command; *p; ++p)
                        if(*p == '\\')
                            *p = '/';
                }

                gsdll_execute_begin();
                code = gsdll_execute_cont(initial_command, n);
                if(code > -100)
                    gsdll_execute_end();
                gsdll_exit();
                retval = true;
            }
        }
    }
    release_info(wp, NULL);
    DeleteFile(spool_namep);
    unloadgs();
    RESTORE_FP_ENVIRONMENT();
    return retval;
}

#if 0

typedef enum
{
  bold = 1,
  italic = 2,
}
face_t;

typedef enum
{
  helvetica,
  times
}
family_t;

void
xxx (int size, face_t face, family_t family)
{
  HFONT font;
  LOGFONT lf;
 
  /* NOTE: this is hacky code, just to get sunPATH started.  I'm not
     calling AddFontResource, like Win32V1-693 says we should, 'cause
     you'd think that Arial and Times will already be available. */

  lf.lfHeight = size;
  lf.lfWidth = 0;
  lf.lfEscapement = 0;
  lf.lfOrientation = 0;
  lf.lfWeight = face & bold ? 700 : 400;
  lf.lfItalic = face & italic ? true : false;
  lf.lfUnderline = false;
  lf.lfStrikeOut = false;
  lf.lfCharSet = ANSI_CHARSET;
  lf.lfOutPrecision = OUT_DEFAULT_PRECIS; /* ? */
  lf.lfClipPrecision = CLIP_DEFAULT_PRECIS; /* ? */
  lf.lfQuality = PROOF_QUALITY;
  lf.lfPitchAndFamily = VARIABLE_PITCH;
  switch (family)
    {
    case helvetica:
      lf.lfPitchAndFamily |= FF_ROMAN;
      strcpy (lf.lfFaceName, "Arial");
      break;
    case times:
      lf.lfPitchAndFamily |= FF_SWISS;
      strcpy (lf.lfFaceName, "Times New Roman");
      break;
    }
  font = CreateFontIndirect (&lf);
}

void
yyy (void)
{
  SetBkMode (global_hdc, TRANSPARENT);
  SelectObject (global_hdc, hfont);
  ExtTextOut (hdc, ...);
}
#endif
