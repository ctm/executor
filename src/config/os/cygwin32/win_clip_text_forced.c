/* Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <rsys/scrap.h>
#include <rsys/error.h>

#include "SDL/SDL.h"
#include "SDL_bmp.h"
#include "SDL_syswm.h"

#define NO_SDL_GET_WINDOW
#if defined(NO_SDL_GET_WINDOW)
#define SDL_GetWindow() GetActiveWindow()
#endif

#define FORMAT_PREFIX "Executor_0x"

PUBLIC unsigned long
ROMlib_executor_format(LONGINT type)
{
    char *str;
    UINT retval;
    static struct
    {
        LONGINT type;
        UINT value;
        bool valid;
    } cache;

    if(cache.valid && type == cache.type)
        retval = cache.value;
    else
    {
        str = alloca(sizeof FORMAT_PREFIX + 8);
        sprintf(str, "%s%08lx", FORMAT_PREFIX, (unsigned long)type);
        retval = RegisterClipboardFormat(str);
        cache.type = type;
        cache.value = retval;
        cache.valid = true;
    }
    return retval;
}

PRIVATE bool support_cf_dib_p = true;

PUBLIC LONGINT
GetScrapX(LONGINT type, char **h)
{
    UINT format;
    LONGINT retval;

    retval = -1;
    switch(type)
    {
        case T('T', 'E', 'X', 'T'):
            format = CF_TEXT;
            break;
        default:
            format = ROMlib_executor_format(type);
            if(support_cf_dib_p && type == T('P', 'I', 'C', 'T'))
            {
                decltype(format) newval;
                UINT formats[2] = { format, CF_DIB };

                newval = GetPriorityClipboardFormat(formats, NELEM(formats));
                if(newval != 0 && newval != (UINT)-1)
                    format = newval;
            }
            break;
    }
    if(IsClipboardFormatAvailable(format) && OpenClipboard(SDL_GetWindow()))
    {
        HANDLE data;

        data = GetClipboardData(format);
        if(data)
        {
            LPVOID lp;

            lp = GlobalLock(data);
            switch(type)
            {
                case T('T', 'E', 'X', 'T'):
                {
                    int len;

                    len = strlen(lp);
                    retval = get_scrap_helper(h, lp, len, true);
                }
                break;
                default:
                {
#if defined(SDL)
                    if(support_cf_dib_p && format == CF_DIB)
                        retval = get_scrap_helper_dib(h, lp);
                    else
#endif
                    {
                        int32 len;
                        len = *(int32 *)lp;

                        retval = get_scrap_helper(h, lp + sizeof(int32),
                                                  len, false);
                    }
                }
                break;
            }
            GlobalUnlock(data);
            CloseClipboard();
        }
    }
    return retval;
}

PRIVATE int
calc_length_and_format(UINT *formatp, LONGINT type, LONGINT length,
                       const char *p)
{
    int retval;

    switch(type)
    {
        case T('T', 'E', 'X', 'T'):
            retval = length + count_char(p, length, '\r') + 1;
            *formatp = CF_TEXT;
            break;
        default:
            retval = length + 4;
            *formatp = ROMlib_executor_format(type);
            break;
    }
    return retval;
}

PRIVATE void
fill_in_data(char *destp, LONGINT type, LONGINT length, const char *p)
{
    switch(type)
    {
        case T('T', 'E', 'X', 'T'):
            while(--length >= 0)
            {
                char c;

                c = *p++;
                *destp++ = c;
                if(c == '\r')
                    *destp++ = '\n';
            }
            *destp++ = 0;
            break;
        default:
            *(int32 *)destp = length;
            memcpy(destp + sizeof(int32), p, length);
            break;
    }
}

#warning DO NOT CHECK THIS IN
PRIVATE bool old_paste_code_p = false;

PRIVATE HANDLE clip_data = NULL;

PUBLIC void
PutScrapX(LONGINT type, LONGINT length, char *p, int scrap_count)
{
    static int old_count = -1;

    HWND active_wnd, top_wnd, sdl_wnd;

    fprintf(stderr, "type = '%c%c%c%c'\n", type >> 24, type >> 16, type >> 8,
            type);
    if(type != T('T', 'E', 'X', 'T'))
    {
        fprintf(stderr, "not type type we want, so we're leaving\n");
        return;
    }
    else
        fprintf(stderr, "yay, got TEXT\n");

    active_wnd = SDL_GetWindow();
    top_wnd = GetTopWindow(NULL);
    {
        SDL_SysWMinfo info;
        int success;

        memset(&info, 0, sizeof info);
        info.version.major = SDL_MAJOR_VERSION;
        info.version.minor = SDL_MINOR_VERSION;
        success = SDL_GetWMInfo(&info);
        fprintf(stderr, "success = %d\n", success);
        sdl_wnd = info.window;
    }

    fprintf(stderr, "active = 0x%x, top = 0x%x, sdl = 0x%x\n",
            (uint32)active_wnd, (uint32)top_wnd, (uint32)sdl_wnd);

    if(OpenClipboard(sdl_wnd) && (/* scrap_count == old_count || */ EmptyClipboard()))
    {
        UINT format;
        int new_length;
        HANDLE data;

        new_length = calc_length_and_format(&format, type, length, p);
        if(old_paste_code_p)
        {
            data = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, new_length);
        }
        else
        {
            if(clip_data)
                LocalFree(clip_data);
            clip_data = LocalAlloc(LMEM_FIXED, new_length);
            data = clip_data;
        }
        if(data)
        {
            char *destp;

            if(old_paste_code_p)
            {
                destp = GlobalLock(data);
                fill_in_data(destp, type, length, p);
                GlobalUnlock(data);
                SetClipboardData(format, data);
            }
            else
            {
                destp = (char *)clip_data;
                fill_in_data(destp, type, length, p);

                if(format == CF_TEXT)
                    fprintf(stderr, "yes, format == CF_TEXT\n");
                else
                    fprintf(stderr, "BADNESS format = %d, CF_TEXT = %d\n",
                            format, CF_TEXT);

                SetClipboardData(format, NULL); /* we'll give 'em the real
					      thing if they ask for it later */
                if(support_cf_dib_p && type == TICK("PICT"))
                    SetClipboardData(CF_DIB, NULL); /* we can create a DIB if
						    asked to do so */
            }
            CloseClipboard();
            old_count = scrap_count;
        }
    }
}

PUBLIC bool
we_lost_clipboard(void)
{
    bool retval;

    retval = GetClipboardOwner() != SDL_GetWindow();
    return retval;
}

PUBLIC void
write_pict_as_dib_to_clipboard(void)
{
    if(clip_data)
        put_scrap_helper_dib((LPVOID)clip_data);
}

PUBLIC void
write_pict_as_pict_to_clipboard(void)
{
    if(clip_data)
    {
        size_t len;
        HGLOBAL hg;

        len = LocalSize(clip_data);
        hg = GlobalAlloc(GMEM_DDESHARE, len);
        if(hg)
        {
            LPVOID lp;

            lp = GlobalLock(hg);
            memcpy(lp, clip_data, len);
            GlobalUnlock(hg);
            SetClipboardData(ROMlib_executor_format(T('P', 'I', 'C', 'T')), hg);
        }
    }
}

void write_surfp_to_clipboard(SDL_Surface *surfp)
{
    char *bytesp;
    size_t len;

    if(SDL_SaveCF_DIB(surfp, &bytesp, &len) == 0)
    {
        HGLOBAL hg;

        hg = GlobalAlloc(GMEM_DDESHARE, len);
        if(hg)
        {
            void *lp;

            lp = GlobalLock(hg);
            memcpy(lp, bytesp, len);
            GlobalUnlock(hg);
            SetClipboardData(CF_DIB, hg);
        }
        free(bytesp);
    }
}
