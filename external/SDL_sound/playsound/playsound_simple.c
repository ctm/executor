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

/**
 * This is just a simple "decode sound, play it through SDL" example.
 *  The much more complex, fancy, and robust code is playsound.c.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon. (icculus@icculus.org)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "SDL_sound.h"

/* global decoding state. */
typedef struct
{
    Sound_Sample *sample;
    SDL_AudioSpec devformat;
    Uint8 *decoded_ptr;
    Uint32 decoded_bytes;
} PlaysoundAudioCallbackData;

/*
 * This variable is flipped to non-zero when the audio callback has
 *  finished playing the whole file.
 */
static volatile int global_done_flag = 0;


/*
 * The audio callback. SDL calls this frequently to feed the audio device.
 *  We decode the audio file being played in here in small chunks and feed
 *  the device as necessary. Other solutions may want to predecode more
 *  (or all) of the file, since this needs to run fast and frequently,
 *  but since we're only sitting here and waiting for the file to play,
 *  the only real requirement is that we can decode a given audio file
 *  faster than realtime, which isn't really a problem with any modern format
 *  on even pretty old hardware at this point.
 */
static void audio_callback(void *userdata, Uint8 *stream, int len)
{
    PlaysoundAudioCallbackData *data = (PlaysoundAudioCallbackData *) userdata;
    Sound_Sample *sample = data->sample;
    int bw = 0; /* bytes written to stream this time through the callback */

    while (bw < len)
    {
        int cpysize;  /* bytes to copy on this iteration of the loop. */

        if (data->decoded_bytes == 0) /* need more data! */
        {
            /* if there wasn't previously an error or EOF, read more. */
            if ( ((sample->flags & SOUND_SAMPLEFLAG_ERROR) == 0) &&
                 ((sample->flags & SOUND_SAMPLEFLAG_EOF) == 0) )
            {
                data->decoded_bytes = Sound_Decode(sample);
                data->decoded_ptr = sample->buffer;
            } /* if */

            if (data->decoded_bytes == 0)
            {
                /* ...there isn't any more data to read! */
                memset(stream + bw, '\0', len - bw);  /* write silence. */
                global_done_flag = 1;
                return;  /* we're done playback, one way or another. */
            } /* if */
        } /* if */

        /* we have data decoded and ready to write to the device... */
        cpysize = len - bw;  /* len - bw == amount device still wants. */
        if (cpysize > data->decoded_bytes)
            cpysize = data->decoded_bytes;  /* clamp to what we have left. */

        /* if it's 0, next iteration will decode more or decide we're done. */
        if (cpysize > 0)
        {
            /* write this iteration's data to the device. */
            memcpy(stream + bw, (Uint8 *) data->decoded_ptr, cpysize);

            /* update state for next iteration or callback */
            bw += cpysize;
            data->decoded_ptr += cpysize;
            data->decoded_bytes -= cpysize;
        } /* if */
    } /* while */
} /* audio_callback */



static void playOneSoundFile(const char *fname)
{
    PlaysoundAudioCallbackData data;

    memset(&data, '\0', sizeof (PlaysoundAudioCallbackData));
    data.sample = Sound_NewSampleFromFile(fname, NULL, 65536);
    if (data.sample == NULL)
    {
        fprintf(stderr, "Couldn't load '%s': %s.\n", fname, Sound_GetError());
        return;
    } /* if */

    /*
     * Open device in format of the the sound to be played.
     *  We open and close the device for each sound file, so that SDL
     *  handles the data conversion to hardware format; this is the
     *  easy way out, but isn't practical for most apps. Usually you'll
     *  want to pick one format for all the data or one format for the
     *  audio device and convert the data when needed. This is a more
     *  complex issue than I can describe in a source code comment, though.
     */
    data.devformat.freq = data.sample->actual.rate;
    data.devformat.format = data.sample->actual.format;
    data.devformat.channels = data.sample->actual.channels;
    data.devformat.samples = 4096;  /* I just picked a largish number here. */
    data.devformat.callback = audio_callback;
    data.devformat.userdata = &data;
    if (SDL_OpenAudio(&data.devformat, NULL) < 0)
    {
        fprintf(stderr, "Couldn't open audio device: %s.\n", SDL_GetError());
        Sound_FreeSample(data.sample);
        return;
    } /* if */

    printf("Now playing [%s]...\n", fname);
    SDL_PauseAudio(0);  /* SDL audio device is "paused" right after opening. */

    global_done_flag = 0;  /* the audio callback will flip this flag. */
    while (!global_done_flag)
        SDL_Delay(10);  /* just wait for the audio callback to finish. */

    /* at this point, we've played the entire audio file. */
    SDL_PauseAudio(1);  /* so stop the device. */

    /*
     * Sleep two buffers' worth of audio before closing, in order
     *  to allow the playback to finish. This isn't always enough;
     *   perhaps SDL needs a way to explicitly wait for device drain?
     * Most apps don't have this issue, since they aren't explicitly
     *  closing the device as soon as a sound file is done playback.
     * As an alternative for this app, you could also change the callback
     *  to write silence for a call or two before flipping global_done_flag.
     */
    SDL_Delay(2 * 1000 * data.devformat.samples / data.devformat.freq);

    /* if there was an error, tell the user. */
    if (data.sample->flags & SOUND_SAMPLEFLAG_ERROR)
        fprintf(stderr, "Error decoding file: %s\n", Sound_GetError());

    Sound_FreeSample(data.sample);  /* clean up SDL_Sound resources... */
    SDL_CloseAudio();  /* will reopen with next file's format. */
} /* playOneSoundFile */


int main(int argc, char **argv)
{
    int i;

    if (!Sound_Init())  /* this calls SDL_Init(SDL_INIT_AUDIO) ... */
    {
        fprintf(stderr, "Sound_Init() failed: %s.\n", Sound_GetError());
        SDL_Quit();
        return(42);
    } /* if */

    for (i = 1; i < argc; i++)  /* each arg is an audio file to play. */
        playOneSoundFile(argv[i]);

    /* Shutdown the libraries... */
    Sound_Quit();
    SDL_Quit();
    return(0);
} /* main */

/* end of playsound-simple.c ... */

