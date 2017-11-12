/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/ctl.h"
#include "rsys/image.h"

using namespace Executor;

void Executor::image_inits(void)
{
    image_arrow_up_active_init();
    image_arrow_up_inactive_init();
    image_arrow_down_active_init();
    image_arrow_down_inactive_init();
    image_arrow_left_active_init();
    image_arrow_left_inactive_init();
    image_arrow_right_active_init();
    image_arrow_right_inactive_init();

    image_thumb_horiz_init();
    image_thumb_vert_init();

    image_active_init();
    image_ractive_init();
    image_go_away_init();
    image_grow_init();
    image_zoom_init();

    image_apple_init();
}
