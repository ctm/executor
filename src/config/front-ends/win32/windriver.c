/* Copyright 1994, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Implementation of the framebuffer portion of win32 vdriver */

#include <windows.h>
#include "windriver.h"

/* Win32 functions we use that conflict with the Executor headers */
#undef ShowWindow

/* Variables used in all versions of the win32 vdriver subsystems */
HWND Win_Window;

/* Variables required by the vdriver interface */
uint8 *vdriver_fbuf = NULL;
int vdriver_row_bytes;
int vdriver_width = VDRIVER_DEFAULT_SCREEN_WIDTH;
int vdriver_height = VDRIVER_DEFAULT_SCREEN_HEIGHT;
int vdriver_bpp, vdriver_log2_bpp;
int vdriver_max_bpp, vdriver_log2_max_bpp;
rgb_spec_t *vdriver_rgb_spec;

/* Include the C file for the currently compiled graphics subsystem */
#if WIN_SUBSYSTEM == WIN_DIB
#include "windib.c"
#else
#if WIN_SUBSYSTEM == WIN_MGL
#include "winmgl.c"
#endif /* WIN_MGL */
#endif /* WIN_DIB */

/* Used as a replacement for fprintf() for printing messages to display */
/* This should probably go in some sort of compatibility library...
   The redefinition of fprintf is currently in win32.h
*/
void Win_Message(FILE *stream, const char *fmt, ...)
{
    char *title_str = NULL;
    char buffer[1024];
    va_list ap;

    if(stream == stdout)
        title_str = "Executor Message";
    else if(stream == stderr)
        title_str = "Executor Error";
    else
        title_str = NULL;

    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);
    if(title_str)
        MessageBox(GetActiveWindow(), buffer, title_str, MB_OK);
    else
        fprintf(stream, "%s", buffer);
    va_end(ap);
}

/* host functions that should go away */

/* shadow buffer; created on demand */
unsigned char *vdriver_shadow_fbuf = NULL;

void host_flush_shadow_screen(void)
{
    int top_long, left_long, bottom_long, right_long;

    /* Lazily allocate a shadow screen.  We won't be doing refresh that often,
   * so don't waste the memory unless we need it.  Note: memory never reclaimed
   */
    if(vdriver_shadow_fbuf == NULL)
    {
        vdriver_shadow_fbuf = malloc(vdriver_row_bytes * vdriver_height);
        memcpy(vdriver_shadow_fbuf, vdriver_fbuf,
               vdriver_row_bytes * vdriver_height);
        vdriver_update_screen(0, 0, vdriver_height, vdriver_width, false);
    }
    else if(find_changed_rect_and_update_shadow((uint32_t *)vdriver_fbuf,
                                                (uint32_t *)vdriver_shadow_fbuf,
                                                (vdriver_row_bytes
                                                 / sizeof(uint32_t)),
                                                vdriver_height,
                                                &top_long, &left_long,
                                                &bottom_long, &right_long))
    {
        vdriver_update_screen(top_long, (left_long * 32) >> vdriver_log2_bpp,
                              bottom_long,
                              (right_long * 32) >> vdriver_log2_bpp, false);
    }
}
