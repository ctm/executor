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
 * This is a quick and dirty test of SDL_sound.
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

#if HAVE_ASSERT_H
#  include <assert.h>
#elif (!defined assert)
#  define assert(x)
#endif

#if HAVE_SIGNAL_H
#  include <signal.h>
#endif

#include "SDL.h"
#include "SDL_sound.h"

#if SUPPORT_PHYSFS
#include "physfs.h"
#include "physfsrwops.h"
#endif

#define DEFAULT_DECODEBUF 16384
#define DEFAULT_AUDIOBUF  4096

#define PLAYSOUND_VER_MAJOR  1
#define PLAYSOUND_VER_MINOR  0
#define PLAYSOUND_VER_PATCH  0


static const char *option_list[] =
{
    "--rate",      "n       Playback at sample rate of n HZ.",
    "--format",    "fmt   Playback in fmt format (see below).",
    "--channels",  "n   Playback on n channels (1 or 2).",
    "--decodebuf", "n  Buffer n decoded bytes at a time (default 16384).",
    "--audiobuf",  "n   Buffer n samples to audio device (default 4096).",
    "--volume",    "n     Playback volume multiplier (default 1.0).",
    "--stdin",     "[ext]  Read from stdin (treat data as format [ext])",
    "--version",   "     Display version information and exit.",
    "--decoders",  "    List supported data formats and exit.",
    "--predecode", "   Decode entire sample before playback.",
    "--loop",      "n       Loop playback n times.",
    "--seek",      "list    List of seek points and playback durations.",
    "--credits",   "     Shameless promotion.",
    "--help",      "        Display this information and exit.",
    NULL,          NULL
};


static void output_versions(const char *argv0)
{
    Sound_Version compiled;
    Sound_Version linked;
    SDL_version sdl_compiled;
    SDL_version sdl_linked_ver;
    const SDL_version *sdl_linked = &sdl_linked_ver;

    SOUND_VERSION(&compiled);
    Sound_GetLinkedVersion(&linked);
    SDL_VERSION(&sdl_compiled);

    #if SDL_MAJOR_VERSION >= 2
    SDL_GetVersion(&sdl_linked_ver);
    #else
    sdl_linked = SDL_Linked_Version();
    #endif

    fprintf(stdout,
           "%s version %d.%d.%d\n"
           "Copyright 2001 Ryan C. Gordon\n"
           "This program is free software, covered by the GNU Lesser General\n"
           "Public License, and you are welcome to change it and/or\n"
           "distribute copies of it under certain conditions. There is\n"
           "absolutely NO WARRANTY for this program.\n"
           "\n"
           " Compiled against SDL_sound version %d.%d.%d,\n"
           " and linked against %d.%d.%d.\n"
           " Compiled against SDL version %d.%d.%d,\n"
           " and linked against %d.%d.%d.\n\n",
             argv0,
             PLAYSOUND_VER_MAJOR, PLAYSOUND_VER_MINOR, PLAYSOUND_VER_PATCH,
             compiled.major, compiled.minor, compiled.patch,
             linked.major, linked.minor, linked.patch,
             sdl_compiled.major, sdl_compiled.minor, sdl_compiled.patch,
             sdl_linked->major, sdl_linked->minor, sdl_linked->patch);
} /* output_versions */


static void output_decoders(void)
{
    const Sound_DecoderInfo **rc = Sound_AvailableDecoders();
    const Sound_DecoderInfo **i;
    const char **ext;

    fprintf(stdout, "Supported sound formats:\n");
    if (rc == NULL)
        fprintf(stdout, " * Apparently, NONE!\n");
    else
    {
        for (i = rc; *i != NULL; i++)
        {
            fprintf(stdout, " * %s\n", (*i)->description);

            for (ext = (*i)->extensions; *ext != NULL; ext++)
                fprintf(stdout, "   File extension \"%s\"\n", *ext);

            fprintf(stdout, "   Written by %s.\n   %s\n\n",
                    (*i)->author, (*i)->url);
        } /* for */
    } /* else */

    fprintf(stdout, "\n");
} /* output_decoders */


static void output_usage(const char *argv0)
{
    const char **i = option_list;

    fprintf(stderr,
        "USAGE: %s [...options...] [soundFile1] ... [soundFileN]\n"
        "\n"
        "   Options:\n",
            argv0);

    while (*i != NULL)
    {
        const char *option = *(i++);
        const char *optiondesc = *(i++);
        fprintf(stderr, "     %s %s\n", option, optiondesc);
    } /* while */

    fprintf(stderr,
        "\n"
        "   Valid arguments to the --format option are:\n"
        "     U8      Unsigned 8-bit.\n"
        "     S8      Signed 8-bit.\n"
        "     U16LSB  Unsigned 16-bit (least significant byte first).\n"
        "     U16MSB  Unsigned 16-bit (most significant byte first).\n"
        "     S16LSB  Signed 16-bit (least significant byte first).\n"
        "     S16MSB  Signed 16-bit (most significant byte first).\n"
        "\n"
        "   Valid arguments to the --seek options look like:\n"
        "     --seek \"mm:SS:ss;mm:SS:ss;mm:SS:ss\"\n"
        "     Where the first \"mm:SS:ss\" is the position, in minutes,\n"
        "     seconds and milliseconds to seek to at start of playback. The\n"
        "     next mm:SS:ss is how long to play audio from that point.\n"
        "     The third mm:SS:ss is another seek after the duration of\n"
        "     playback has completed. If the final playback duration is\n"
        "     omitted, playback continues until the end of the file.\n"
        "     The \"mm\" and \"SS\" portions may be omitted. --loop\n"
        "     and --seek can coexist.\n"
        "\n");
} /* output_usage */


static void output_credits(void)
{
    fprintf(stdout,
           "playsound version %d.%d.%d\n"
           "Copyright 2001 Ryan C. Gordon\n"
           "playsound is free software, covered by the GNU Lesser General\n"
           "Public License, and you are welcome to change it and/or\n"
           "distribute copies of it under certain conditions. There is\n"
           "absolutely NO WARRANTY for playsound.\n"
           "\n"
           "    Written by Ryan C. Gordon, Torbjörn Andersson, Max Horn,\n"
           "     Tsuyoshi Iguchi, Tyler Montbriand, Darrell Walisser,\n"
           "     and a cast of thousands.\n"
           "\n"
           "    Website and source code: http://icculus.org/SDL_sound/\n"
           "\n",
            PLAYSOUND_VER_MAJOR, PLAYSOUND_VER_MINOR, PLAYSOUND_VER_PATCH);
} /* output_credits */



/* archive stuff... */

static int init_archive(const char *argv0)
{
    int retval = 1;

#if SUPPORT_PHYSFS
    retval = PHYSFS_init(argv0);
    if (!retval)
    {
        fprintf(stderr, "Couldn't init PhysicsFS: %s\n",
                PHYSFS_getLastError());
    } /* if */
#endif

    return(retval);
} /* init_archive */


#if SUPPORT_PHYSFS
static SDL_RWops *rwops_from_physfs(const char *filename)
{
    SDL_RWops *retval = NULL;

    char *path = (char *) malloc(strlen(filename) + 1);
    char *archive;

    if (path == NULL)
    {
        fprintf(stderr, "Out of memory!\n");
        return(NULL);
    } /* if */

    strcpy(path, filename);
    archive = strchr(path, '@');
    if (archive != NULL)
    {
        *(archive++) = '\0';  /* blank '@', point to archive name. */
        if (!PHYSFS_addToSearchPath(archive, 0))
        {
            fprintf(stderr, "Couldn't open archive: %s\n",
                    PHYSFS_getLastError());
            free(path);
            return(NULL);
        } /* if */

        retval = PHYSFSRWOPS_openRead(path);
    } /* if */

    free(path);
    return(retval);
} /* rwops_from_physfs */
#endif


static Sound_Sample *sample_from_archive(const char *fname,
                                         Sound_AudioInfo *desired,
                                         Uint32 decode_buffersize)
{
    Sound_Sample *retval = NULL;

#if SUPPORT_PHYSFS
    SDL_RWops *rw = rwops_from_physfs(fname);
    if (rw != NULL)
    {
        char *path = (char *) malloc(strlen(fname) + 1);
        char *ptr;
        strcpy(path, fname);
        ptr = strchr(path, '@');
        *ptr = '\0';
        ptr = strrchr(path, '.');
        if (ptr != NULL)
            ptr++;

        retval = Sound_NewSample(rw, ptr, desired, decode_buffersize);
        free(path);
    } /* if */
#endif

    return(retval);
} /* sample_from_archive */


static void close_archive(const char *filename)
{
#if SUPPORT_PHYSFS
    char *archive_name = strchr(filename, '@');
    if (archive_name != NULL)
        PHYSFS_removeFromSearchPath(archive_name + 1);
#endif
} /* close_archive */


static void deinit_archive(void)
{
#if SUPPORT_PHYSFS
    PHYSFS_deinit();
#endif
} /* deinit_archive */



static volatile int done_flag = 0;

#if HAVE_SIGNAL_H
void sigint_catcher(int signum)
{
    static Uint32 last_sigint = 0;
    Uint32 ticks = SDL_GetTicks();

    assert(signum == SIGINT);
    if (done_flag < 0)
        return;  /* mashing CTRL-C, we get it already. */

    done_flag = ((last_sigint != 0) && (ticks - last_sigint < 500)) ? -1 : 1;
    last_sigint = ticks;
} /* sigint_catcher */
#endif


/* global decoding state. */
typedef struct
{
    Uint8 *decoded_ptr;
    Uint32 decoded_bytes;
    int predecode;
    int looping;
    int wants_volume_change;
    float volume;
    Uint32 total_seeks;
    Uint32 *seek_list;
    Uint32 seek_index;
    Sint32 bytes_before_next_seek;
} playsound_global_state;

static volatile playsound_global_state global_state;


static Uint32 cvtMsToBytePos(Sound_AudioInfo *info, Uint32 ms)
{
    /* "frames" == "sample frames" */
    float frames_per_ms = ((float) info->rate) / 1000.0;
    Uint32 frame_offset = (Uint32) (frames_per_ms * ((float) ms));
    Uint32 frame_size = (Uint32) ((info->format & 0xFF) / 8) * info->channels;
    return(frame_offset * frame_size);
} /* cvtMsToBytePos */


static void do_seek(Sound_Sample *sample)
{
    Uint32 *seek_list = global_state.seek_list;
    Uint32 seek_index = global_state.seek_index;
    Uint32 total_seeks = global_state.total_seeks;

    fprintf(stdout, "Seeking to %.2d:%.2d:%.4d...\n",
            (int) ((seek_list[seek_index] / 1000) / 60),
            (int) ((seek_list[seek_index] / 1000) % 60),
            (int) ((seek_list[seek_index] % 1000)));

    if (global_state.predecode)
    {
        Uint32 pos = cvtMsToBytePos(&sample->desired, seek_list[seek_index]);
        if (pos > sample->buffer_size)
        {
            fprintf(stderr, "Seek past end of predecoded buffer.\n");
            done_flag = 1;
        } /* if */
        else
        {
            global_state.decoded_ptr = (((Uint8 *) sample->buffer) + pos);
            global_state.decoded_bytes = sample->buffer_size - pos;
        } /* else */
    } /* if */
    else
    {
        if (!Sound_Seek(sample, seek_list[seek_index]))
        {
            fprintf(stderr, "Sound_Seek() failed: %s\n", Sound_GetError());
            done_flag = 1;
        } /* if */
    } /* else */

    seek_index++;
    if (seek_index >= total_seeks)
        global_state.bytes_before_next_seek = -1;  /* no more seeks. */
    else
    {
        global_state.bytes_before_next_seek = cvtMsToBytePos(&sample->desired,
                                                        seek_list[seek_index]);
        seek_index++;
    } /* else */

    global_state.seek_index = seek_index;
} /* do_seek */


/*
 * This updates (decoded_bytes) and (decoded_ptr) with more audio data,
 *  taking into account potential looping, seeking and predecoding.
 */
static int read_more_data(Sound_Sample *sample)
{
    if (done_flag)              /* probably a sigint; stop trying to read. */
    {
        global_state.decoded_bytes = 0;
        return(0);
    } /* if */

    if ((global_state.bytes_before_next_seek >= 0) &&
        (global_state.decoded_bytes > global_state.bytes_before_next_seek))
    {
        global_state.decoded_bytes = global_state.bytes_before_next_seek;
    } /* if */

    if (global_state.decoded_bytes > 0) /* don't need more data; just return. */
        return(global_state.decoded_bytes);

        /* Need more audio data. See if we're supposed to seek... */
    if ((global_state.bytes_before_next_seek == 0) &&
        (global_state.seek_index < global_state.total_seeks))
    {
        do_seek(sample);  /* do it, baby! */
        return(read_more_data(sample));  /* handle loops conditions. */
    } /* if */

        /* See if there's more to be read... */
    if ( (global_state.bytes_before_next_seek != 0) &&
         (!(sample->flags & (SOUND_SAMPLEFLAG_ERROR | SOUND_SAMPLEFLAG_EOF))) )
    {
        global_state.decoded_bytes = Sound_Decode(sample);
        if (sample->flags & SOUND_SAMPLEFLAG_ERROR)
        {
            fprintf(stderr, "Error in decoding sound file!\n"
                            "  reason: [%s].\n", Sound_GetError());
        } /* if */

        global_state.decoded_ptr = sample->buffer;
        return(read_more_data(sample));  /* handle loops conditions. */
    } /* if */

    /* No more to be read from stream, but we may want to loop the sample. */

    if (!global_state.looping)
        return(0);

    global_state.looping--;

    global_state.seek_index = 0;
    global_state.bytes_before_next_seek =
        (global_state.total_seeks > 0) ? 0 : -1;

    /* we just need to point predecoded samples to the start of the buffer. */
    if (global_state.predecode)
    {
        global_state.decoded_bytes = sample->buffer_size;
        global_state.decoded_ptr = sample->buffer;
    } /* if */
    else
    {
        Sound_Rewind(sample);  /* error is checked in recursion. */
    } /* else */

    return(read_more_data(sample));
} /* read_more_data */


static void memcpy_with_volume(Sound_Sample *sample,
                               Uint8 *dst, Uint8 *src, int len)
{
    int i;
    Uint16 *u16src = NULL;
    Uint16 *u16dst = NULL;
    Sint16 *s16src = NULL;
    Sint16 *s16dst = NULL;
    float volume = global_state.volume;

    if (!global_state.wants_volume_change)
    {
        memcpy(dst, src, len);
        return;
    } /* if */

    /* !!! FIXME: This would be more efficient with a lookup table. */
    switch (sample->desired.format)
    {
        case AUDIO_U8:
            for (i = 0; i < len; i++, src++, dst++)
                *dst = (Uint8) (((float) (*src)) * volume);
            break;

        case AUDIO_S8:
            for (i = 0; i < len; i++, src++, dst++)
                *dst = (Sint8) (((float) (*src)) * volume);
            break;

        case AUDIO_U16LSB:
            u16src = (Uint16 *) src;
            u16dst = (Uint16 *) dst;
            for (i = 0; i < len; i += sizeof (Uint16), u16src++, u16dst++)
            {
                *u16dst = (Uint16) (((float) (SDL_SwapLE16(*u16src))) * volume);
                *u16dst = SDL_SwapLE16(*u16dst);
            } /* for */
            break;

        case AUDIO_S16LSB:
            s16src = (Sint16 *) src;
            s16dst = (Sint16 *) dst;
            for (i = 0; i < len; i += sizeof (Sint16), s16src++, s16dst++)
            {
                *s16dst = (Sint16) (((float) (SDL_SwapLE16(*s16src))) * volume);
                *s16dst = SDL_SwapLE16(*s16dst);
            } /* for */
            break;

        case AUDIO_U16MSB:
            u16src = (Uint16 *) src;
            u16dst = (Uint16 *) dst;
            for (i = 0; i < len; i += sizeof (Uint16), u16src++, u16dst++)
            {
                *u16dst = (Uint16) (((float) (SDL_SwapBE16(*u16src))) * volume);
                *u16dst = SDL_SwapBE16(*u16dst);
            } /* for */
            break;

        case AUDIO_S16MSB:
            s16src = (Sint16 *) src;
            s16dst = (Sint16 *) dst;
            for (i = 0; i < len; i += sizeof (Sint16), s16src++, s16dst++)
            {
                *s16dst = (Sint16) (((float) (SDL_SwapBE16(*s16src))) * volume);
                *s16dst = SDL_SwapBE16(*s16dst);
            } /* for */
            break;
    } /* switch */
} /* memcpy_with_volume */


static void audio_callback(void *userdata, Uint8 *stream, int len)
{
    Sound_Sample *sample = (Sound_Sample *) userdata;
    int bw = 0; /* bytes written to stream this time through the callback */

    while (bw < len)
    {
        int cpysize;  /* bytes to copy on this iteration of the loop. */

        if (!read_more_data(sample)) /* read more data, if needed. */
        {
            /* ...there isn't any more data to read! */
            memset(stream + bw, '\0', len - bw);
            done_flag = 1;
            return;
        } /* if */

        /* decoded_bytes and decoder_ptr are updated as necessary... */

        cpysize = len - bw;
        if (cpysize > global_state.decoded_bytes)
            cpysize = global_state.decoded_bytes;

        if (cpysize > 0)
        {
            memcpy_with_volume(sample, stream + bw,
                               (Uint8 *) global_state.decoded_ptr,
                               cpysize);

            bw += cpysize;
            global_state.decoded_ptr += cpysize;
            global_state.decoded_bytes -= cpysize;
            if (global_state.bytes_before_next_seek >= 0)
                global_state.bytes_before_next_seek -= cpysize;
        } /* if */
    } /* while */
} /* audio_callback */


static int count_seek_list(const char *list)
{
    const char *ptr;
    int retval = 0;

    for (ptr = list; ptr != NULL; ptr = strchr(ptr + 1, ';'))
        retval++;

    return(retval);
} /* count_seek_list */


static Uint32 parse_time_str(char *str)
{
    Uint32 minutes = 0;
    Uint32 seconds = 0;
    Uint32 ms = 0;

    char *ptr = strchr(str, ':');
    if (ptr != NULL)
    {
        char *ptr2;

        *ptr = '\0';
        ptr2 = strchr(ptr + 1, ':');
        if (ptr2 != NULL)
        {
            *ptr2 = '\0';
            minutes = atoi(str);
            str = ptr + 1;
            ptr = ptr2;
        } /* if */

        seconds = atoi(str);
        str = ptr + 1;
    } /* if */

    ms = atoi(str);
    return( (((minutes * 60) + seconds) * 1000) + ms );
} /* parse_time_str */


static void parse_seek_list(const char *_list)
{
    Uint32 i;

    char *list = (char*) malloc(strlen(_list) + 1);
    char *save_list = list;
    if (list == NULL)
    {
        fprintf(stderr, "malloc() failed. Skipping seek list.\n");
        return;
    } /* if */

    strcpy(list, _list);

    if (global_state.seek_list != NULL)
        free((void *) global_state.seek_list);

    global_state.total_seeks = count_seek_list(list);

    global_state.seek_list =
              (Uint32 *) malloc(global_state.total_seeks * sizeof (Uint32));

    if (global_state.seek_list == NULL)
    {
        fprintf(stderr, "malloc() failed. Skipping seek list.\n");
        global_state.total_seeks = 0;
        return;
    } /* if */

    for (i = 0; i < global_state.total_seeks; i++)
    {
        char *ptr = strchr(list, ';');
        if (ptr != NULL)
            *ptr = '\0';
        global_state.seek_list[i] = parse_time_str(list);
        list = ptr + 1;
    } /* for */

    global_state.bytes_before_next_seek = 0;

    free(save_list);
} /* parse_seek_list */


static int str_to_fmt(char *str)
{
    if (strcmp(str, "U8") == 0)
        return AUDIO_U8;
    if (strcmp(str, "S8") == 0)
        return AUDIO_S8;
    if (strcmp(str, "U16LSB") == 0)
        return AUDIO_U16LSB;
    if (strcmp(str, "S16LSB") == 0)
        return AUDIO_S16LSB;
    if (strcmp(str, "U16MSB") == 0)
        return AUDIO_U16MSB;
    if (strcmp(str, "S16MSB") == 0)
        return AUDIO_S16MSB;
    return 0;
} /* str_to_fmt */


static int valid_cmdline(int argc, char **argv)
{
    int i;

    if (argc < 2)  /* no command line? Show help text and quit. */
    {
        output_usage(argv[0]);
        return(0);
    } /* if */

    /* Make sure all command line options are valid. */
    for (i = 1; i < argc; i++)
    {
        const char **opts = option_list;

        if (strncmp(argv[i], "--", 2) != 0)  /* not an option; skip it. */
            continue;

        while (*opts != NULL)
        {
            if (strcmp(argv[i], *(opts++)) == 0)
                break;

            opts++;  /* skip option description. */
        } /* else */

        if (*opts == NULL)  /* didn't find it in option_list... */
        {
            fprintf(stderr, "unknown option: \"%s\"\n", argv[i]);
            return(0);
        } /* if */
    } /* for */

    return(1);  /* everything appears to be in order. */
} /* valid_cmdline */


static void report_filename(const char *filename)
{
    const char *icon = "playsound";
    size_t len = 0;
    char *buf = NULL;
    char *ptr = NULL;

    fprintf(stdout, "%s: Now playing [%s]...\n", icon, filename);

#if SDL_MAJOR_VERSION < 2
    /*
     * Bleeding edge versions of SDL 1.2 can use this to set the
     *  PulseAudio application name. It's a harmless no-op elsewhere,
     *  and 2.0 will probably have a formal API for this.
     */
    ptr = strrchr(filename, '/');
    if (ptr != NULL)
        filename = ptr + 1;
    ptr = strrchr(filename, '\\');
    if (ptr != NULL)
        filename = ptr + 1;

    len = strlen(filename) + strlen(icon) + 3;
    buf = (char *) malloc(len);
    if (buf == NULL)
        SDL_WM_SetCaption(icon, icon);
    else
    {
        snprintf(buf, len, "%s: %s", icon, filename);
        SDL_WM_SetCaption(buf, icon);
        free(buf);
    } /* else */
#endif
} /* report_filename */


int main(int argc, char **argv)
{
    Sound_AudioInfo sound_desired;
    SDL_AudioSpec sdl_desired;
    Uint32 audio_buffersize;
    Uint32 decode_buffersize;
    Sound_Sample *sample;
    int use_specific_audiofmt = 0;
    int i;
    int delay;
    int new_sample = 1;
    Uint32 sdl_init_flags = SDL_INIT_AUDIO;

    #if ENABLE_EVENTS
    SDL_Surface *screen = NULL;
    SDL_Event event;

    sdl_init_flags |= SDL_INIT_VIDEO;
    #endif

    #ifdef HAVE_SETBUF
        setbuf(stdout, NULL);
        setbuf(stderr, NULL);
    #endif

    if (!valid_cmdline(argc, argv))
        return(42);

    /* Handle some command lines upfront. */
    for (i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], "--", 2) != 0)
            continue;

        if (strcmp(argv[i], "--version") == 0)
        {
            output_versions(argv[0]);
            return(42);
        } /* if */

        if (strcmp(argv[i], "--credits") == 0)
        {
            output_credits();
            return(42);
        } /* if */

        else if (strcmp(argv[i], "--help") == 0)
        {
            output_usage(argv[0]);
            return(42);
        } /* if */

        else if (strcmp(argv[i], "--decoders") == 0)
        {
            if (!Sound_Init())
            {
                fprintf(stderr, "Sound_Init() failed!\n"
                                "  reason: [%s].\n", Sound_GetError());
                SDL_Quit();
                return(42);
            } /* if */

            output_decoders();
            Sound_Quit();
            return(0);
        } /* else if */
    } /* for */

    if (!init_archive(argv[0]))
        return(42);

    if (SDL_Init(sdl_init_flags) == -1)
    {
        fprintf(stderr, "SDL_Init() failed!\n"
                        "  reason: [%s].\n", SDL_GetError());
        return(42);
    } /* if */

    if (!Sound_Init())
    {
        fprintf(stderr, "Sound_Init() failed!\n"
                        "  reason: [%s].\n", Sound_GetError());
        SDL_Quit();
        return(42);
    } /* if */

    #if HAVE_SIGNAL_H
        signal(SIGINT, sigint_catcher);
    #endif

    #if ENABLE_EVENTS
        screen = SDL_SetVideoMode(320, 240, 8, 0);
        assert(screen != NULL);
    #endif

    for (i = 1; i < argc; i++)
    {
        char *filename = NULL;

        /* set variables back to defaults for next file... */
        if (new_sample)
        {
            if (global_state.seek_list != NULL)
                free((void *) global_state.seek_list);

            memset((void *) &global_state, '\0', sizeof (global_state));
            memset(&sdl_desired, '\0', sizeof (SDL_AudioSpec));
            global_state.volume = 1.0;
            global_state.bytes_before_next_seek = -1;
            audio_buffersize = DEFAULT_AUDIOBUF;
            decode_buffersize = DEFAULT_DECODEBUF;
            new_sample = 0;
        } /* if */

        if (strcmp(argv[i], "--rate") == 0 && argc > i + 1)
        {
            use_specific_audiofmt = 1;
            sound_desired.rate = atoi(argv[++i]);
            if (sound_desired.rate <= 0)
            {
                fprintf(stderr, "Bad argument to --rate!\n");
                return(42);
            } /* if */
        } /* else if */

        else if (strcmp(argv[i], "--format") == 0 && argc > i + 1)
        {
            use_specific_audiofmt = 1;
            sound_desired.format = str_to_fmt(argv[++i]);
            if (sound_desired.format == 0)
            {
                fprintf(stderr, "Bad argument to --format! Try one of:\n"
                                "U8, S8, U16LSB, S16LSB, U16MSB, S16MSB\n");
                return(42);
            } /* if */
        } /* else if */

        else if (strcmp(argv[i], "--channels") == 0 && argc > i + 1)
        {
            use_specific_audiofmt = 1;
            sound_desired.channels = atoi(argv[++i]);
            if (sound_desired.channels < 1 || sound_desired.channels > 2)
            {
                fprintf(stderr,
                        "Bad argument to --channels! Try 1 (mono) or 2 "
                        "(stereo).\n");
                return(42);
            } /* if */
        } /* else if */

        else if (strcmp(argv[i], "--audiobuf") == 0 && argc > i + 1)
        {
            audio_buffersize = atoi(argv[++i]);
        } /* else if */

        else if (strcmp(argv[i], "--decodebuf") == 0 && argc > i + 1)
        {
            decode_buffersize = atoi(argv[++i]);
        } /* else if */

        else if (strcmp(argv[i], "--volume") == 0 && argc > i + 1)
        {
            global_state.volume = atof(argv[++i]);
            if (global_state.volume != 1.0)
                global_state.wants_volume_change = 1;
        } /* else if */

        else if (strcmp(argv[i], "--predecode") == 0)
        {
            global_state.predecode = 1;
        } /* else if */

        else if (strcmp(argv[i], "--loop") == 0)
        {
            global_state.looping = atoi(argv[++i]);
        } /* else if */

        else if (strcmp(argv[i], "--seek") == 0)
        {
            parse_seek_list(argv[++i]);
        } /* else if */

        else if (strcmp(argv[i], "--stdin") == 0)
        {
            SDL_RWops *rw = SDL_RWFromFP(stdin, 1);
            filename = "...from stdin...";

            /*
             * The second argument will be NULL if --stdin is the last
             *  thing on the command line. This is correct behaviour.
             */
            sample = Sound_NewSample(rw, argv[++i],
                        use_specific_audiofmt ? &sound_desired : NULL,
                        decode_buffersize);
        } /* if */

        else if (strncmp(argv[i], "--", 2) == 0)
        {
            /* ignore it, since it was handled at startup. */
        } /* else if */

        else
        {
            filename = argv[i];
            sample = sample_from_archive(filename,
                            use_specific_audiofmt ? &sound_desired : NULL,
                            decode_buffersize);

            if (sample == NULL)
            {
                sample = Sound_NewSampleFromFile(filename,
                            use_specific_audiofmt ? &sound_desired : NULL,
                            decode_buffersize);
            } /* if */
        } /* else */

        if (filename == NULL) /* still parsing command line stuff? */
            continue;

        new_sample = 1;

        if (sample == NULL)
        {
            fprintf(stderr, "Couldn't load \"%s\"!\n"
                            "  reason: [%s].\n",
                            filename, Sound_GetError());
            continue;
        } /* if */

        if (global_state.total_seeks > 0)
        {
            if ((!global_state.predecode) &&
                (!(sample->flags & SOUND_SAMPLEFLAG_CANSEEK)))
            {
                fprintf(stderr, "Want seeks, but sample cannot handle it!\n");
                Sound_FreeSample(sample);
                close_archive(filename);
                continue;
            } /* if */
        } /* if */

            /*
             * Unless explicitly specified, pick the format from the sound
             * to be played.
             */
        if (use_specific_audiofmt)
        {
            sdl_desired.freq = sample->desired.rate;
            sdl_desired.format = sample->desired.format;
            sdl_desired.channels = sample->desired.channels;
        } /* if */
        else
        {
            sdl_desired.freq = sample->actual.rate;
            sdl_desired.format = sample->actual.format;
            sdl_desired.channels = sample->actual.channels;
        } /* else */

        sdl_desired.samples = audio_buffersize;
        sdl_desired.callback = audio_callback;
        sdl_desired.userdata = sample;

        /* grr, SDL_CloseAudio() calls SDL_QuitSubSystem internally. */
        if (!SDL_WasInit(SDL_INIT_AUDIO))
        {
            if (SDL_Init(SDL_INIT_AUDIO) == -1)
            {
                fprintf(stderr, "SDL_Init() failed!\n"
                                "  reason: [%s].\n", SDL_GetError());
                Sound_Quit();
                SDL_Quit();
                return(42);
            } /* if */
        } /* if */

        report_filename(filename);

        if (SDL_OpenAudio(&sdl_desired, NULL) < 0)
        {
            fprintf(stderr, "Couldn't open audio device!\n"
                            "  reason: [%s].\n", SDL_GetError());
            Sound_Quit();
            SDL_Quit();
            return(42);
        } /* if */

        if (global_state.predecode)
        {
            fprintf(stdout, "  predecoding...");
            global_state.decoded_bytes = Sound_DecodeAll(sample);
            global_state.decoded_ptr = sample->buffer;
            if (sample->flags & SOUND_SAMPLEFLAG_ERROR)
            {
                fprintf(stderr,
                        "Couldn't fully decode \"%s\"!\n"
                        "  reason: [%s].\n"
                        "  (playing first %lu bytes of decoded data...)\n",
                        filename, Sound_GetError(),
                        (unsigned long) global_state.decoded_bytes);
            } /* if */
            else
            {
                fprintf(stdout, "done.\n");
            } /* else */
        } /* if */

        SDL_PauseAudio(0);

        done_flag = 0;  /* the audio callback will flip this flag. */
        while (!done_flag)
        {
            #if ENABLE_EVENTS
                SDL_PollEvent(&event);
                if ((event.type == SDL_KEYDOWN) || (event.type == SDL_QUIT))
                    done_flag = 1;
            #endif

            SDL_Delay(10);
        } /* while */

        SDL_PauseAudio(1);

            /*
             * Sleep two buffers' worth of audio before closing, in order
             *  to allow the playback to finish. This isn't always enough;
             *   perhaps SDL needs a way to explicitly wait for device drain?
             */
        delay = 2 * 1000 * sdl_desired.samples / sdl_desired.freq;
        SDL_Delay(delay);

        SDL_CloseAudio();  /* reopen with next sample's format if possible */
        Sound_FreeSample(sample);

        close_archive(filename);

        if (done_flag < 0)
            break;
    } /* for */

    Sound_Quit();
    SDL_Quit();
    deinit_archive();
    return((done_flag < 0) ? 1 : 0);
} /* main */

/* end of playsound.c ... */

