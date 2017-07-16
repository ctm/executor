/*
 * SDL_sound -- An abstract sound format decoding API.
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

/*
 * FMT decoder for SDL_sound.
 *
 * This driver handles FMT audio data. Blahblahblah... The author should
 *  have done a search and replace on "fmt" and "FMT" and changed this
 *  comment. This is the default comment in the skeleton decoder file...
 *
 * None of this code, even the parts that LOOK right, have been compiled,
 *  so you cut-and-paste at your own risk.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon. (icculus@icculus.org)
 */

#error DO NOT COMPILE THIS.
#error  This is an example decoder skeleton.
#error  You should base your code on this file, and remove these error lines
#error  from your version.

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef SOUND_SUPPORTS_FMT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL_sound.h"

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"

static int FMT_init(void);
static void FMT_quit(void);
static int FMT_open(Sound_Sample *sample, const char *ext);
static void FMT_close(Sound_Sample *sample);
static Uint32 FMT_read(Sound_Sample *sample);
static int FMT_rewind(Sound_Sample *sample);
static int FMT_seek(Sound_Sample *sample, Uint32 ms);

static const char *extensions_fmt[] = { "FMT", NULL };
const Sound_DecoderFunctions __Sound_DecoderFunctions_FMT =
{
    {
        extensions_fmt,
        "FMT audio format description",
        "Ryan C. Gordon <icculus@icculus.org>",
        "http://www.icculus.org/SDL_sound/"
    },

    FMT_init,       /*   init() method */
    FMT_quit,       /*   quit() method */
    FMT_open,       /*   open() method */
    FMT_close,      /*  close() method */
    FMT_read,       /*   read() method */
    FMT_rewind,     /* rewind() method */
    FMT_seek        /*   seek() method */
};


static int FMT_init(void)
{
    /* do any global decoder/library initialization you need here. */

    return(1);  /* initialization successful. */
} /* FMT_init */


static void FMT_quit(void)
{
    /* do any global decoder/library cleanup you need here. */
} /* FMT_quit */


static int FMT_open(Sound_Sample *sample, const char *ext)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    SDL_RWops *rw = internal->rw;

    if (can NOT accept the data)
        BAIL_MACRO("FMT: expected X, got Y.", 0);

    SNDDBG(("FMT: Accepting data stream.\n"));
    set up sample->actual;
    sample->flags = SOUND_SAMPLEFLAG_NONE;
    return(1); /* we'll handle this data. */
} /* FMT_open */


static void FMT_close(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    clean up anything you put into internal->decoder_private;
} /* FMT_close */


static Uint32 FMT_read(Sound_Sample *sample)
{
    Uint32 retval;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;

        /*
         * We don't actually do any decoding, so we read the fmt data
         *  directly into the internal buffer...
         */
    retval = SDL_RWread(internal->rw, internal->buffer,
                        1, internal->buffer_size);

    (or whatever. Do some decoding here...)

        /* Make sure the read went smoothly... */
    if (retval == 0)
        sample->flags |= SOUND_SAMPLEFLAG_EOF;

    else if (retval == -1)
        sample->flags |= SOUND_SAMPLEFLAG_ERROR;

        /* (next call this EAGAIN may turn into an EOF or error.) */
    else if (retval < internal->buffer_size)
        sample->flags |= SOUND_SAMPLEFLAG_EAGAIN;

    (or whatever. retval == number of bytes you put in internal->buffer).

    return(retval);
} /* FMT_read */


static int FMT_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;

        /* seek to the appropriate place... */
    BAIL_IF_MACRO(SDL_RWseek(internal->rw, 0, SEEK_SET) != 0, ERR_IO_ERROR, 0);

    (reset state as necessary.)

    return(1);  /* success. */
} /* FMT_rewind */


static int FMT_seek(Sound_Sample *sample, Uint32 ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;

        /* seek to the appropriate place... */
    BAIL_IF_MACRO(SDL_RWseek(internal->rw, 0, SEEK_SET) != 0, ERR_IO_ERROR, 0);

    (set state as necessary.)

    return(1);  /* success. */
} /* FMT_seek */


#endif /* SOUND_SUPPORTS_FMT */


/* end of fmt.c ... */

