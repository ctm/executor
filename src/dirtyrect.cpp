/* Copyright 1995 - 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_dirtyrect[] = "$Id: dirtyrect.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/vdriver.h"
#include "rsys/dirtyrect.h"
#include "rsys/quick.h"
#include "rsys/cquick.h"
#include "rsys/prefs.h"
#include "rsys/flags.h"
#include "rsys/autorefresh.h"
#include "OSEvent.h"

#if !defined(MAX_DIRTY_RECTS)
#define MAX_DIRTY_RECTS 5
#endif

using namespace Executor;

int Executor::num_dirty_rects = 0;
static vdriver_rect_t dirty_rect[MAX_DIRTY_RECTS];

/* We will glom a new rectangle into an existing one if it adds no more
 * than this many pixels.
 */
#define ACCEPTABLE_PIXELS_ADDED 8000

static inline bool
rects_overlap_p(int top, int left, int bottom, int right,
                const vdriver_rect_t *r)
{
    return (top < r->bottom
            && r->top < bottom
            && left < r->right
            && r->left < right);
}

/* This routine is only valid for non-overlapping rectangles! */
static inline unsigned long
area_added(int top, int left, int bottom, int right, int r1_area,
           const vdriver_rect_t *r2)
{
    int t, l, b, r, new_area, r2_area;

    t = MIN(top, r2->top);
    l = MIN(left, r2->left);
    b = MAX(bottom, r2->bottom);
    r = MAX(right, r2->right);

    /* This works because we know the rects don't overlap. */
    new_area = (r - l) * (b - t);
    r2_area = (r2->right - r2->left) * (r2->bottom - r2->top);

    return new_area - (r1_area + r2_area);
}

static inline void
union_rect(int top, int left, int bottom, int right, vdriver_rect_t *r)
{
    if(top < r->top)
        r->top = top;
    if(left < r->left)
        r->left = left;
    if(bottom > r->bottom)
        r->bottom = bottom;
    if(right > r->right)
        r->right = right;
}

/* This function adds the specified rectangle to the dirty rect list.
 * If this rectangle is very close to another rectangle already in the
 * rect list, they may be glommed together into one larger rectangle.
 * Since we don't allow overlapping rects in the dirty rect list, if
 * this rect overlaps any rects already in the list, they will be
 * glommed together into a larger rectange.  Since that larger
 * rectangle might suddenly overlap other rectangles already in the
 * list, that glommed rectangle must get reinserted.
 */
void Executor::dirty_rect_accrue(int top, int left, int bottom, int right)
{
    unsigned long best_area_added;
    int ndr, i, best;
    bool done;

    if(bottom <= top || right <= left
       || ROMlib_refresh
       || !ROMlib_shadow_screen_p)
        return;

    /* Note that Executor has touched the screen. */
    note_executor_changed_screen(top, bottom);

    ndr = num_dirty_rects;

    /* Quickly handle the common case of no dirty rects. */
    if(ndr == 0)
    {
        dirty_rect[0].top = top;
        dirty_rect[0].left = left;
        dirty_rect[0].bottom = bottom;
        dirty_rect[0].right = right;
        num_dirty_rects = 1;
        return;
    }

    /* Otherwise, glom away! */
    for(done = false; !done;)
    {
        int new_area;

        new_area = (right - left) * (bottom - top);

        /* Figure out which union adds the least new area. */
        best_area_added = ~0UL;
        best = 0;
        for(i = ndr - 1; i >= 0; i--)
        {
            unsigned long added;

            /* We don't allow overlapping rectangles, so if this overlaps
	   * _any_ of them, then we glom them together.
	   */
            if(rects_overlap_p(top, left, bottom, right, &dirty_rect[i]))
            {
                best_area_added = 0;
                best = i;
                break;
            }
            else
            {
                /* We know that they don't overlap. */
                added = area_added(top, left, bottom, right, new_area,
                                   &dirty_rect[i]);
                if(added < best_area_added)
                {
                    best_area_added = added;
                    best = i;
                }
            }
        }

        /* Add this rect to the dirty list, either by glomming together
       * with an existing rect, or by adding a new rect.
       */
        if(best_area_added <= ACCEPTABLE_PIXELS_ADDED
           || ndr == MAX_DIRTY_RECTS)
        {
            if(ndr == 1)
            {
                union_rect(top, left, bottom, right, &dirty_rect[0]);
                done = true;
            }
            else
            {
                bool old_rect_grows_p;
                vdriver_rect_t *d = &dirty_rect[best];

                /* We now re-insert the glommed rectangle into the list, to
	       * see if it overlaps anyone else.
	       */

                if(d->top <= top)
                {
                    top = d->top;
                    old_rect_grows_p = false;
                }
                else
                    old_rect_grows_p = true;

                if(d->left <= left)
                    left = d->left;
                else
                    old_rect_grows_p = true;

                if(d->bottom >= bottom)
                    bottom = d->bottom;
                else
                    old_rect_grows_p = true;

                if(d->right >= right)
                    right = d->right;
                else
                    old_rect_grows_p = true;

                /* If this new rect is not entirely subsumed by the rect
	       * it interesects, then we delete the subsumed rect,
	       * and insert the union of the two rectangles into
	       * our rect list.  Otherwise, we're done, because the
	       * new rectangle doesn't add any new information.
	       */
                if(old_rect_grows_p)
                    *d = dirty_rect[--ndr];
                else
                    done = true;
            }
        }
        else /* No glomming required. */
        {
            vdriver_rect_t *n = &dirty_rect[ndr];

            /* Just add a new rectangle to the list. */
            n->top = top;
            n->left = left;
            n->bottom = bottom;
            n->right = right;
            ++ndr;
            done = true;
        }
    }

    num_dirty_rects = ndr;
}

/* Returns true iff the specified rect is already encompassed by
 * the dirty rect list.
 */
bool Executor::dirty_rect_subsumed_p(int top, int left, int bottom, int right)
{
    int i;

    for(i = num_dirty_rects - 1; i >= 0; i--)
    {
        const vdriver_rect_t *r = &dirty_rect[i];
        if(r->top <= top && r->left <= left
           && r->bottom >= bottom && r->right >= right)
            return true;
    }

    return false;
}

/* Note: the process of aligning these rectangles can make some of them
 * overlap slightly in strange cases.
 */
static inline void
clip_and_align_dirty_rects(void)
{
    vdriver_rect_t *r;
    int i;
#if VDRIVER_DIRTY_RECT_BYTE_ALIGNMENT
    int log2_bpp;
#endif

#if VDRIVER_DIRTY_RECT_BYTE_ALIGNMENT
    log2_bpp = vdriver_log2_bpp;
#endif

    /* Loop over all rects and canonicalize them. */
    for(i = num_dirty_rects, r = &dirty_rect[0]; --i >= 0; r++)
    {
        int left, right;

        if(r->top < 0)
            r->top = 0;
        if(r->bottom > vdriver_height)
            r->bottom = vdriver_height;

        /* Pin left and round it down. */
        left = r->left;
        if(left < 0)
            left = 0;
#if VDRIVER_DIRTY_RECT_BYTE_ALIGNMENT
#define ALIGN_BITS (VDRIVER_DIRTY_RECT_BYTE_ALIGNMENT * 8UL - 1)
        else
            left &= ~(ALIGN_BITS >> log2_bpp);
#endif
        r->left = left;

        /* Pin right and round it up. */
        right = r->right;
        if(right > vdriver_width)
            right = vdriver_width;
#if VDRIVER_DIRTY_RECT_BYTE_ALIGNMENT
        right = ((((right << log2_bpp) + ALIGN_BITS) & ~ALIGN_BITS)
                 >> log2_bpp);
#endif
        r->right = right;
    }
}

static inline void
sort_dirty_rects_by_top(void)
{
#if defined(VDRIVER_SORT_DIRTY_RECTS_BY_TOP)
    int i, j;

    /* Bubble sort is fine for only a small number of rectangles. */
    for(i = num_dirty_rects - 1; i > 0; i--)
    {
        int cur_top = dirty_rect[0].top;

        for(j = 0; j < i; j++)
        {
            int next_top = dirty_rect[j + 1].top;
            if(cur_top > next_top)
            {
                vdriver_rect_t tmp;
                tmp = dirty_rect[j];
                dirty_rect[j] = dirty_rect[j + 1];
                dirty_rect[j + 1] = tmp;
            }
            else
                cur_top = next_top;
        }
    }
#endif
}

void Executor::dirty_rect_update_screen(void)
{
    if(num_dirty_rects)
    {
        vdriver_rect_t dirty_rect_copy[MAX_DIRTY_RECTS];
        int ndr;

        clip_and_align_dirty_rects();
        sort_dirty_rects_by_top();
        ndr = num_dirty_rects;
        num_dirty_rects = 0;

        /* Copy rects to a local copy so we are reentrant. */
        memcpy(&dirty_rect_copy[0], &dirty_rect[0], ndr * sizeof dirty_rect[0]);

        vdriver_update_screen_rects(ndr, &dirty_rect_copy[0], false);
    }
}
