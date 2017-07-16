/*
 * SDL_sound -- An sound processing toolkit.
 * Copyright (C) 2001  Ryan C. Gordon.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * This file implements the mixer itself. Largely, this is handled in the
 *  SDL audio callback.
 *
 * Documentation is in SDL_sound.h ... It's verbose, honest.  :)
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon. (icculus@icculus.org)
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_sound.h"

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"



typedef struct S_PlayingList
{
    Sound_Sample *sample;
    struct S_PlayingList *next;
} PlayingList;

static PlayingList *playlist = NULL;

static inline void mix_predecoded(Sound_Sample *samp,
                                  UInt32 *samp_frames_left,
                                  float *gains)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) samp->opaque;
    Uint32 sfl = *samp_frames_left;  /* move to a local. */
    Uint32 max = internal->buffer_size - internal->mix_position;
    float *wptr;  /* write pointer */

    /* !!! FIXME: max must be converted to sample frame count... */

    if (max > sfl)  /* we have more data than mix buffer? */
        max = sfl;

    assert(max > 0);
    *samp_frames_left -= max;

    wptr = mixbuf + ((mixbufsize / sizeof (float)) - (max * MAX_CHANNELS));
    internal->mix(wptr, internal->buffer, max, gains);
} /* mix_predecoded */


static void mix_playing_samples(Uint8 *stream, int len)
{
    PlayingList *samples = playlist;
    const int frames = len / framesize;
    const Uint32 ticks = SDL_GetTicks();  /* used for calculating fade. */

    while (samples)  /* iterate linked list of playing samples... */
    {
        Sound_Sample *samp = samples->sample;
        Uint32 sample_frames_left = mixbuf_frames;
        float gains[MAX_CHANNELS];

        calculate_gains(samp, ticks, gains);
        while (sample_frames_left)
        {
            mix_predecoded(samp, &sample_frames_left);
            if (!decode_more(samp))
                break;
        } /* while */

        samples = samples->next;  /* set up for next iteration. */
    } /* while */
} /* mix_playing_samples */


static inline void run_pre_mix(void)
{
    if (premixer)
        premixer(mixbuf, mixbufsize);
    else  /* !!! FIXME: Do memset in another thread after mix is done. */
        memset(mixbuf, '\0', mixbufsize * sizeof (float) * 2);
} /* run_pre_mix */


static inline void run_post_mix(void)
{
    if (postmixer)
        postmixer(mixbuf, mixbufsize);
} /* run_post_mix */


/* this is where it happens: the SDL audio callback. */
static void audio_callback(void *userdata, Uint8 *stream, int len)
{
    mixer_callback_running = 1;
    run_pre_mix();
    mix_playing_samples();
    run_post_mix();
    mixer_callback_running = 0;
} /* audio_callback */

/* end of mixercore.c ... */

