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
 * Module player for SDL_sound. This driver handles anything MikMod does, and
 *  is based on SDL_mixer.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Torbjörn Andersson (d91tan@Update.UU.SE)
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef SOUND_SUPPORTS_MIKMOD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL_sound.h"

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"

#include "mikmod.h"


static int MIKMOD_init(void);
static void MIKMOD_quit(void);
static int MIKMOD_open(Sound_Sample *sample, const char *ext);
static void MIKMOD_close(Sound_Sample *sample);
static Uint32 MIKMOD_read(Sound_Sample *sample);
static int MIKMOD_rewind(Sound_Sample *sample);
static int MIKMOD_seek(Sound_Sample *sample, Uint32 ms);

static const char *extensions_mikmod[] =
{
    "669",   /* Composer 669                                                */
    "AMF",   /* DMP Advanced Module Format                                  */
    "DSM",   /* DSIK internal format                                        */
    "FAR",   /* Farandole module                                            */
    "GDM",   /* General DigiMusic module                                    */
    "IMF",   /* Imago Orpheus module                                        */
    "IT",    /* Impulse tracker                                             */
    "M15",   /* 15 instrument MOD / Ultimate Sound Tracker (old M15 format) */
    "MED",   /* Amiga MED module                                            */
    "MOD",   /* Generic MOD (Protracker, StarTracker, FastTracker, etc)     */
    "MTM",   /* MTM module                                                  */
    "OKT",   /* Oktalyzer module                                            */
    "S3M",   /* Screamtracker module                                        */
    "STM",   /* Screamtracker 2 module                                      */
    "STX",   /* STMIK 0.2 module                                            */
    "ULT",   /* Ultratracker module                                         */
    "UNI",   /* UNIMOD - libmikmod's and APlayer's internal module format   */
    "XM",    /* Fasttracker module                                          */
    NULL
};

const Sound_DecoderFunctions __Sound_DecoderFunctions_MIKMOD =
{
    {
        extensions_mikmod,
        "Play modules through MikMod",
        "Torbjörn Andersson <d91tan@Update.UU.SE>",
        "http://mikmod.raphnet.net/"
    },

    MIKMOD_init,       /*   init() method */
    MIKMOD_quit,       /*   quit() method */
    MIKMOD_open,       /*   open() method */
    MIKMOD_close,      /*  close() method */
    MIKMOD_read,       /*   read() method */
    MIKMOD_rewind,     /* rewind() method */
    MIKMOD_seek        /*   seek() method */
};


/* Make MikMod read from a RWops... */

typedef struct MRWOPSREADER {
    MREADER core;
    Sound_Sample *sample;
    int end;
} MRWOPSREADER;

static BOOL _mm_RWopsReader_eof(MREADER *reader)
{
    MRWOPSREADER *rwops_reader = (MRWOPSREADER *) reader;
    Sound_Sample *sample = rwops_reader->sample;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    int pos = SDL_RWtell(internal->rw);

    if (rwops_reader->end == pos)
        return(1);

    return(0);
} /* _mm_RWopsReader_eof */


static BOOL _mm_RWopsReader_read(MREADER *reader, void *ptr, size_t size)
{
    MRWOPSREADER *rwops_reader = (MRWOPSREADER *) reader;
    Sound_Sample *sample = rwops_reader->sample;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    return(SDL_RWread(internal->rw, ptr, size, 1));
} /* _mm_RWopsReader_Read */


static int _mm_RWopsReader_get(MREADER *reader)
{
    char buf;
    MRWOPSREADER *rwops_reader = (MRWOPSREADER *) reader;
    Sound_Sample *sample = rwops_reader->sample;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;

    if (SDL_RWread(internal->rw, &buf, 1, 1) != 1)
        return(EOF);

    return((int) buf);
} /* _mm_RWopsReader_get */


static BOOL _mm_RWopsReader_seek(MREADER *reader, long offset, int whence)
{
    MRWOPSREADER *rwops_reader = (MRWOPSREADER *) reader;
    Sound_Sample *sample = rwops_reader->sample;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;

    return(SDL_RWseek(internal->rw, offset, whence));
} /* _mm_RWopsReader_seek */


static long _mm_RWopsReader_tell(MREADER *reader)
{
    MRWOPSREADER *rwops_reader = (MRWOPSREADER *) reader;
    Sound_Sample *sample = rwops_reader->sample;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;

    return(SDL_RWtell(internal->rw));
} /* _mm_RWopsReader_tell */


static MREADER *_mm_new_rwops_reader(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;

    MRWOPSREADER *reader = (MRWOPSREADER *) malloc(sizeof (MRWOPSREADER));
    if (reader != NULL)
    {
        int failed_seek = 1;
        int here;
        reader->core.Eof  = _mm_RWopsReader_eof;
        reader->core.Read = _mm_RWopsReader_read;
        reader->core.Get  = _mm_RWopsReader_get;
        reader->core.Seek = _mm_RWopsReader_seek;
        reader->core.Tell = _mm_RWopsReader_tell;
        reader->sample = sample;

        /* RWops does not explicitly support an eof check, so we shall find
           the end manually - this requires seek support for the RWop */
        here = SDL_RWtell(internal->rw);
        if (here != -1)
        {
            reader->end = SDL_RWseek(internal->rw, 0, SEEK_END);
            if (reader->end != -1)
            {
                /* Move back */
                if (SDL_RWseek(internal->rw, here, SEEK_SET) != -1)
                    failed_seek = 0;
            } /* if */
        } /* if */

        if (failed_seek)
        {
            free(reader);
            reader = NULL;
        } /* if */
    } /* if */

    return((MREADER *) reader);
} /* _mm_new_rwops_reader */


static void _mm_delete_rwops_reader(MREADER *reader)
{
    /* SDL_sound will delete the RWops and sample at a higher level... */
    if (reader != NULL)
        free(reader);
} /* _mm_delete_rwops_reader */



static int MIKMOD_init(void)
{
    MikMod_RegisterDriver(&drv_nos);
    
        /*
         * Quick and dirty hack to prevent an infinite loop problem
         *  found when using SDL_mixer and SDL_sound together and
         *  both have MikMod compiled in. So, check to see if
         *  MikMod has already been registered first before calling
         *  RegisterAllLoaders()
         */
    if (MikMod_InfoLoader() == NULL)
    {
        MikMod_RegisterAllLoaders();
    } /* if */

        /*
         * Both DMODE_SOFT_MUSIC and DMODE_16BITS should be set by default,
         * so this is just for clarity. I haven't experimented with any of
         * the other flags. There are a few which are said to give better
         * sound quality.
         */
    md_mode |= (DMODE_SOFT_MUSIC | DMODE_16BITS);
    md_mixfreq = 0;
    md_reverb = 1;

    BAIL_IF_MACRO(MikMod_Init(""), MikMod_strerror(MikMod_errno), 0);

    return(1);  /* success. */
} /* MIKMOD_init */


static void MIKMOD_quit(void)
{
    MikMod_Exit();
    md_mixfreq = 0;
} /* MIKMOD_quit */


static int MIKMOD_open(Sound_Sample *sample, const char *ext)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    MREADER *reader;
    MODULE *module;
    Uint32 i; /* temp counter for time computation */
    double segment_time = 0.0; /* temp holder for time */
    
    reader = _mm_new_rwops_reader(sample);
    BAIL_IF_MACRO(reader == NULL, ERR_OUT_OF_MEMORY, 0);
    module = Player_LoadGeneric(reader, 64, 0);
    _mm_delete_rwops_reader(reader);
    BAIL_IF_MACRO(module == NULL, "MIKMOD: Not a module file.", 0);

    module->extspd  = 1;
    module->panflag = 1;
    module->wrap    = 0;
    module->loop    = 0;

    if (md_mixfreq == 0)
        md_mixfreq = (!sample->desired.rate) ? 44100 : sample->desired.rate;

    sample->actual.channels = 2;
    sample->actual.rate = md_mixfreq;
    sample->actual.format = AUDIO_S16SYS;
    internal->decoder_private = (void *) module;

    Player_Start(module);
    Player_SetPosition(0);

    sample->flags = SOUND_SAMPLEFLAG_NONE;

    /*
     *   module->sngtime = current song time in 2^-10 seconds
     *   internal->total_time = (module->sngtime * 1000) / (1<<10)
     */
    internal->total_time = (module->sngtime * 1000) / (1<<10);

    SNDDBG(("MIKMOD: Name: %s\n", module->songname));
    SNDDBG(("MIKMOD: Type: %s\n", module->modtype));
    SNDDBG(("MIKMOD: Accepting data stream\n"));


    /* 
     * This is a quick and dirty way for getting the play time
     * of a file. This will often be wrong because the tracker format
     * allows for so much. If you want a better one, use ModPlug,
     * demand that the Mikmod people write better functionality,
     * or write a more complicated version of the code below.
     *
     * There are two dumb ways to compute the length. The really
     * dumb way is to look at the header and take the initial
     * speed/tempo  values. However, speed values can change throughout
     * the file. The slightly smarter way is to iterate through
     * all the positions and add up each segment's time based
     * on the idea that each segment will give us its own
     * speed value. The hope is that this is more accurate.
     * However, this demands that the file be seekable
     * and that we can change the position of the sample.
     * Depending on the assumptions of SDL_sound, this block
     * of code should be enabled or disabled. If disabled,
     * you still can make the computations doing the first method.
     * For now, we will assume it's acceptable to seek a Mod file
     * since this is essentially how Modplug also does it.
     *
     * Keep in mind that this will be incorrect for loops, jumps, short
     * patterns and other features.
     */
    sample->flags |= SOUND_SAMPLEFLAG_CANSEEK;

    /* 
     * For each position (which corresponds to a particular pattern),
     * get the speed values and compute the time length of the segment
     */
    internal->total_time = 0;
    for (i = 0; i < module->numpos; i++)
    {
        Player_SetPosition(i);
        /* Must call update, or the speed values won't get reset */
        MikMod_Update();
        /* Now the magic formula:
         * Multiply the number of positions by the
         * Number of rows (usually 64 but can be different) by the
         * time it takes to read one row (1/50)
         * by the speed by the 
         * magic reference beats per minute / the beats per minute
         * 
         * We're using positions instead of patterns because in our
         * test cases, this seems to be the correct value for the 
         * number of sections you hear during normal playback.
         * They typically map to a fewer number of patterns
         * where some patterns get replayed multiple times
         * in a song (think chorus). Since we're in a for-loop,
         * the multiplication is implicit while we're adding
         * all the segments.
         * 
         * From a tracker format spec, it seems that 64 rows
         * is the normal (00-3F), but I've seen songs that 
         * either have less or are jumping positions in the
         * middle of a pattern. It looks like Mikmod might
         * reveal this number for us.
         *
         * According to the spec, it seems that a speed of 1
         * corresponds to reading 1 row in 50 ticks. However,
         * I'm not sure if ticks are real seconds or this
         * notion of second units: 
         * Assuming that it's just normal seconds, we get 1/50 = 0.02.
         *
         * The current speed and current tempo (beats per minute) 
         * we can just grab. However, we need a magic number 
         * to figure out what the tempo is based on. Our primitive
         * stopwatch results and intuition seem to imply 120-130bpm 
         * is the magic number. Looking at the majority of tracker
         * files I have, 125 seems to be the common value. Furthermore
         * most (if not all) of my Future Crew .S3M (Scream Tracker 3)
         * files also use 125. Since they invented that format, 
         * I'll also assume that's the base number.
         */
        if(module->bpm == 0)
        {
            /* 
             * Should never get here, but I don't want any
             * divide by zero errors
             */
            continue;
        } /* if */
        segment_time += (module->numrow * .02 * module->sngspd *
                          125.0 / module->bpm);
    } /* for */
    /* Now convert to milliseconds and store the value */
    internal->total_time = (Sint32)(segment_time * 1000);

    /* Reset the sample to the beginning */
    Player_SetPosition(0);
    MikMod_Update();

    return(1); /* we'll handle this data. */
} /* MIKMOD_open */


static void MIKMOD_close(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    MODULE *module = (MODULE *) internal->decoder_private;

    Player_Free(module);
} /* MIKMOD_close */


static Uint32 MIKMOD_read(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    MODULE *module = (MODULE *) internal->decoder_private;

        /* Switch to the current module, stopping any previous one. */
    Player_Start(module);
    if (!Player_Active())
    {
        sample->flags |= SOUND_SAMPLEFLAG_EOF;
        return(0);
    } /* if */
    return((Uint32) VC_WriteBytes(internal->buffer, internal->buffer_size));
} /* MIKMOD_read */


static int MIKMOD_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    MODULE *module = (MODULE *) internal->decoder_private;

    Player_Start(module);
    Player_SetPosition(0);
    return(1);
} /* MIKMOD_rewind */


static int MIKMOD_seek(Sound_Sample *sample, Uint32 ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    MODULE *module = (MODULE *) internal->decoder_private;
    double last_time = 0.0;
    double current_time = 0.0;
    double target_time;
    Uint32 i; 

        /*
         * Heaven may know what the argument to Player_SetPosition() is.
         * I, however, haven't the faintest idea.
         */
    Player_Start(module);

        /*
         * Mikmod only lets you seek to the beginning of a pattern.
         * This means we'll get very coarse grain seeks. The 
         * value we pass to SetPosition is a value between 0 
         * and the number of positions in the file. The
         * dumb approach would be to take our total_time that
         * we've already calculated and divide it up by the 
         * number of positions and seek to the position that results.
         * However, because songs can alter their speed/tempo during
         * playback, different patterns in the song can take 
         * up different amounts of time. So the slightly
         * smarter approach is to repeat what was done in the
         * total_time computation and traverse through the file
         * until we find the closest position.
         * The follwing is basically cut and paste from the 
         * open function.
         */
    if (ms == 0)  /* Check end conditions to simplify things */
    {
        Player_SetPosition(0);
        return(1);
    } /* if */

    if (ms >= internal->total_time)
        Player_SetPosition(module->numpos);

    /* Convert time to seconds (double) to make comparisons easier */
    target_time = ms / 1000.0;
    
    for (i = 0; i < module->numpos; i++)
    {
        Player_SetPosition(i);
        /* Must call update, or the speed values won't get reset */
        MikMod_Update();
        /* Divide by zero check */
        if(module->bpm == 0)
            continue;
        last_time = current_time;
        /* See the notes in the open function about the formula */
        current_time += (module->numrow * .02 
            * module->sngspd * 125.0 / module->bpm);
        if(target_time <= current_time)
            break; /* We now have our interval, so break out */
    } /* for */
    
    if( (target_time-last_time) > (current_time-target_time) )
    {
        /* The target time is closer to the higher position, so go there */
        Player_SetPosition(i+1);
    } /* if */
    else
    {
        /* The target time is closer to the lower position, so go there */
        Player_SetPosition(i);
    } /* else */

    return(1);
} /* MIKMOD_seek */

#endif /* SOUND_SUPPORTS_MIKMOD */


/* end of mikmod.c ... */
