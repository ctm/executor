/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_refresh[] =
		"$Id: refresh.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/refresh.h"
#include "rsys/blockinterrupts.h"
#include "rsys/vdriver.h"
#include "rsys/prefs.h"
#include "rsys/dirtyrect.h"
#include "rsys/host.h"
#include "rsys/pstuff.h"
#include "rsys/flags.h"
#include "rsys/autorefresh.h"
#include "TimeMgr.h"
#include "OSEvent.h"

using namespace Executor;

static TMTask refresh_tm_task;

static boolean_t refresh_tm_task_installed_p = FALSE;


A0 (PUBLIC, void, C_handle_refresh)
{
  static boolean_t busy_p = FALSE;

  /* If we're going directly to the screen, hang out and wait to see
   * if they stop writing directly to the screen later.
   */
  if (!ROMlib_shadow_screen_p)
    PrimeTime ((QElemPtr) &refresh_tm_task, 2000   /* Two seconds */);
  
  /* If refresh is off, see if we need to turn it on. */
  if (!ROMlib_refresh && do_autorefresh_p && autodetect_refresh ())
    set_refresh_rate (10);

  if (!ROMlib_refresh)
    {
      /* No need for refresh, so check for autorefresh later. */
      PrimeTime ((QElemPtr) &refresh_tm_task, AUTOREFRESH_CHECK_MSECS);
    }
  else
    {
      boolean_t old_busy_p;

      /* Make sure we aren't called recursively. */
      old_busy_p = busy_p;
      busy_p = TRUE;

      PrimeTime ((QElemPtr) &refresh_tm_task, ROMlib_refresh * 1000 / 60);

      if (!old_busy_p)
	host_flush_shadow_screen ();
      busy_p = old_busy_p;
    }
}


void
Executor::dequeue_refresh_task (void)
{
  /* Make sure refresh is off when the task isn't queued up,
   * so other drawing happens.
   */
  set_refresh_rate (0);
  RmvTime ((QElemPtr) &refresh_tm_task);
  refresh_tm_task_installed_p = FALSE;
}

static boolean_t shadow_screen_invalid_p;

void
Executor::set_refresh_rate (int new1)
{
#if defined (VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
  int old_vis = host_set_cursor_visible (FALSE);
#endif /* VDRIVER_SUPPORTS_REAL_SCREEN_BLITS */
  static int last_refresh_set = 0;

  if (new1 < 0)
    new1 = 0;

#if defined (VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
  if (!last_refresh_set && new)
    vdriver_set_up_internal_screen ();
  else if (last_refresh_set && !new)
    dirty_rect_update_screen ();
#endif /* VDRIVER_SUPPORTS_REAL_SCREEN_BLITS */
  
  if (!last_refresh_set && new1)
    shadow_screen_invalid_p = TRUE;
  
  if (!refresh_tm_task_installed_p)
    {
      refresh_tm_task.tmAddr = RM ((ProcPtr) P_handle_refresh);
      InsTime ((QElemPtr) &refresh_tm_task);

      /* We PrimeTime with 400 to "get the ball rolling".  The handler
       * for this interrupt will use the proper delay from then on.
       * We can't just use ROMlib_refresh here, because it may be zero
       * or may be unused if we're going straight to screen memory.
       */
      PrimeTime ((QElemPtr) &refresh_tm_task, 400);
    }

  ROMlib_refresh = last_refresh_set = new1;

#if defined (VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
  host_set_cursor_visible (old_vis);
#endif /* VDRIVER_SUPPORTS_REAL_SCREEN_BLITS */
}


/* This code identifies a rectangle that encompasses any differences
 * between screen and shadow.  The rectangle is specified in terms of
 * 32 bit longs; the actual bits per pixel isn't important here.
 * If nothing has changed, this returns FALSE.  If something has changed,
 * this routine fills in the rectangle coordinates, copies the changed
 * data from screen to shadow, and returns TRUE.
 */
BOOLEAN
Executor::find_changed_rect_and_update_shadow (const uint32 *screen, uint32 *shadow,
				     long row_longs, long num_rows,
				     int *top_long, int *left_long,
				     int *bottom_long, int *right_long)
{
  long longs_left, last_ix, x, y, screen_longs;
  uint32 *s1;
  const uint32 *s2;
  int top, left, bottom, right;
#if !defined(i386)
  const uint32 *shadow_first_changed, *screen_first_changed;
#endif

  if (shadow_screen_invalid_p)
    {
      top = left = 0;
      bottom = num_rows;
      right = row_longs;
      goto found_rect;
    }
  
  screen_longs = row_longs * num_rows;

  /* Find the first long that changed (if any). */

#if defined (i386)
  {
    typeof (shadow) shadow_dregs_unused;
    typeof (screen) screen_dregs_unused;
  asm ("cld\n\t"
       "repe\n\t"
       "cmpsl"
       : "=c" (longs_left), "=S" (shadow_dregs_unused), "=D" (screen_dregs_unused)
       : "1" (shadow), "2" (screen), "c" (screen_longs)
       : "cc");
  }
#else /* !i386 */
  for (longs_left = screen_longs, shadow_first_changed = shadow,
		  screen_first_changed = screen; longs_left != 0; longs_left--)
    if (*shadow_first_changed++ != *screen_first_changed++)
      break;
#endif /* !i386 */

  /* If nothing has changed, return immediately. */
  if (longs_left == 0)
    return FALSE;

  check_virtual_interrupt ();

  /* Record the top row. */
  top = (screen_longs - longs_left) / row_longs;

  /* Find the last long that changed (if any). */
#if defined (i386)
  {
    typeof (shadow) shadow_dregs_unused;
    typeof (screen) screen_dregs_unused;

    asm volatile ("std\n\t"
		  "repe\n\t"
		  "cmpsl"
		  : "=c" (longs_left), "=S" (shadow_dregs_unused), "=D" (screen_dregs_unused)
		  : "1" (shadow + screen_longs - 1),
		  "2" (screen + screen_longs - 1),
		  "c" (screen_longs)
		  : "cc");
  }
#else /* !i386 */
  for (longs_left = screen_longs - 1; longs_left >= 0; longs_left--)
    if (shadow[longs_left] != screen[longs_left])
      {
	longs_left++;
	break;
      }
#endif /* !i386 */

  /* This shouldn't be possible unless the screen changed after the first
   * search (interrupt?)  But handle it gracefully anyway.
   */
  if (longs_left == 0)
    return FALSE;

  check_virtual_interrupt ();

  /* Record one row past the last row that changed. */
  bottom = (longs_left + row_longs - 1) / row_longs;
  if (bottom <= top)
    return FALSE;

  /* Now creep in from the left and find the leftmost changed long. */
  last_ix = (bottom - top - 1) * row_longs;
  s1 = &shadow[top * row_longs];
  s2 = &screen[top * row_longs];
  for (x = 0; x < row_longs; x++)  /* Check x just to be safe. */
    {
      long ix;
      for (ix = last_ix; ix >= 0; ix -= row_longs)
	{
	  if (s1[ix] != s2[ix])
	    goto found_left;
	}

      s1++;
      s2++;
    }

 found_left:
  if (x >= row_longs)  /* Again, handle this gracefully. */
    return FALSE;
  left = x;

  check_virtual_interrupt ();

  /* Now creep in from the right and find the rightmost changed long. */
  s1 = &shadow[top * row_longs + row_longs - 1];
  s2 = &screen[top * row_longs + row_longs - 1];
  for (x = row_longs; x > 0; x--)  /* Check x just to be safe. */
    {
      long ix;
      for (ix = last_ix; ix >= 0; ix -= row_longs)
	{
	  if (s1[ix] != s2[ix])
	    goto found_right;
	}

      s1--;
      s2--;
    }

 found_right:
  if (x <= left)  /* Again, handle this gracefully. */
    return FALSE;
  right = x;

 found_rect:
  /* Now update the dirty rect of the shadow screen. */
  s1 = &shadow[top * row_longs + left];
  s2 = &screen[top * row_longs + left];
#if defined (i386)
      asm ("cld");
#endif
  for (y = bottom - top; y != 0; y--)
    {
#if defined (i386)
      {
	typeof (s2) s2_dregs_unused;
	typeof (s1) s1_dregs_unused;
	int count_dregs_unused;
	asm ("rep\n\t"
	     "movsl"
	     : "=S" (s2_dregs_unused), "=D" (s1_dregs_unused),  "=c" (count_dregs_unused)
	     : "0" (s2), "1" (s1), "2" (right - left)
	     : "memory");
      }
#else
      for (x = right - left - 1; x >= 0; x--)
	s1[x] = s2[x];
#endif
      s1 += row_longs;
      s2 += row_longs;
    }

  *right_long  = right;
  *left_long   = left;
  *bottom_long = bottom;
  *top_long    = top;
  
  shadow_screen_invalid_p = FALSE;
  
  return TRUE;
}
