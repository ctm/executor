/* Copyright 1994, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include <windows.h>

/* Globals */
int host_cursor_depth = 1;

/* Handler Functions */
void host_set_cursor(char *cursor_data,
                     unsigned short cursor_mask[16],
                     int hotspot_x, int hotspot_y)
{
}
int host_set_cursor_visible(int show_p)
{
    /* Assume that if ShowCursor() returns true, that the cursor was hidden */
    return (!(ShowCursor(show_p) > 0));
}
