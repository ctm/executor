/* Copyright 1995-2005 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/*
 * This is a hacked up copy of linux-sound.c that is being used to test
 * a different way to drive SDL's sound.
 */

#include "rsys/common.h"

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sched.h>
#include <pthread.h>

#include "rsys/vdriver.h"
#include "rsys/sounddriver.h"
#include "rsys/m68kint.h"
#include "sdl-sound.h"
#include <SDL_Audio.h>

using namespace Executor;

enum
{
    NON_RUNNING_SOUND_RATE = 22255
};

static char *sdl_audio_driver_name = NULL;

void
ROMlib_set_sdl_audio_driver_name(const char *str)
{
    if(sdl_audio_driver_name)
        free(sdl_audio_driver_name);
    if(str)
        sdl_audio_driver_name = strdup(str);
    else
        sdl_audio_driver_name = NULL;
}

/* Wait on the semaphore (atomic decrement) */

void SDLSound::patl_wait(void)
{
    struct sembuf op_wait[] = {
        { 0, -1, 0 }
    };

    while(semop(semid, op_wait, 1) < 0)
    {
        if(errno == EINTR)
            continue;
        if(errno != EIDRM)
            perror("semop failed");
        pthread_exit(NULL);
    }
}

/* Signal the semaphore (atomic increment) */

void SDLSound::patl_signal(void)
{
    struct sembuf op_signal[] = {
        { 0, 1, 0 }
    };

    while(semop(semid, op_signal, 1) < 0)
    {
        if(errno == EINTR)
            continue;
        gui_fatal("Signal failure");
    }
}

bool SDLSound::sound_works()
{
    return have_sound_p;
}

void SDLSound::sound_go()
{
    sound_on = 1;
    patl_signal();

    SDL_PauseAudio(0);
}

void SDLSound::sound_stop()
{
    sound_on = 0;
}

/* Do any bookkeeping needed to start feeding a hungry device */

void SDLSound::HungerStart()
{
    t1 += num_samples;
}

/* Figure out how to feed the hungry output device. */

struct hunger_info
SDLSound::GetHungerInfo()
{
    struct hunger_info info;

    info.buf = buf;
    info.bufsize = 7 * num_samples;

    info.t2 = t1 + num_samples;
    info.t3 = info.t2 + num_samples;
    info.t4 = info.t3;

    return info;
}

ssize_t
SDLSound::sdl_write(const void *buf, size_t len)
{
    //  fprintf (stderr, "sdl_write: stream = %p, len = %d, buf[0]= 0x%02x, buf[1] = 0x%02x\n", sdl_stream, (int) len, ((uint8_t *) buf)[0], ((uint8_t *) buf)[1]);

    if(sdl_stream)
    {
        memcpy(sdl_stream, buf, len);
        sdl_stream = NULL;
    }

    return len;
}

/* Assuming that the information returned by snd_get_hunger_info was
   honored, send the samples off to the device. */

void SDLSound::HungerFinish()
{
    struct hunger_info info;

    info = GetHungerInfo();

    while(sdl_write(info.buf + (info.t2 % info.bufsize), num_samples)
          < 0)
    {
        if(errno != EINTR)
            gui_fatal("Write failed");
    }

    memset(info.buf + (info.t2 % info.bufsize), 0x80, num_samples);

    patl_signal();
}

#if defined(_SEM_SEMUN_UNDEFINED)
union semun {
    int unused_baggage;
};
#endif

void SDLSound::sound_shutdown()
{
    if(semid >= 0)
        semctl(semid, 0, IPC_RMID, (union semun){.val = 0 });

    if(have_sound_p)
    {
        /* possibly kill the thread here */
        have_sound_p = false;
    }

    sdl_stream = NULL;
}

void SDLSound::sound_clear_pending()
{
}

bool SDLSound::sound_silent()
{
    return false;
}

void SDLSound::sound_sdl_shutdown_at_exit(void)
{
    SOUND_SHUTDOWN();
}

void SDLSound::sdl_wait_until_callback_has_been_called(void)
{
#warning TODO usleep is not the answer
    while(sdl_stream == 0)
        usleep(1);
}

/* Infinite loop for the "sound" thread.  Waits until the audio device
   is ready to accept input, then set a flag for the emulator
   thread. */

void *
SDLSound::loop(void *unused)
{
    while(true)
    {
        SDLSound *ourSelf = (SDLSound *)unused;
        ourSelf->patl_wait();
        //      fprintf (stderr, "waiting until callback has been called\n");
        ourSelf->sdl_wait_until_callback_has_been_called();

        /* Request interrupt */
        if(ourSelf->sound_on)
        {
            cpu_state.interrupt_pending[M68K_SOUND_PRIORITY] = 1;
            cpu_state.interrupt_status_changed = INTERRUPT_STATUS_CHANGED;
        }
    }

    return NULL; /* won't get here */
}

void SDLSound::hunger_callback(void *unused, Uint8 *stream, int len)
{
    SDLSound *ourself = (SDLSound *)unused;
    //  FILE *fp;
    //  int fd;

    //  fd = -1;
    //  fd = open("/tmp/sdl-sound.log", O_CREAT|O_WRONLY, 0644);
    //  fp = fopen ("/tmp/sdl-sound.log", "w");
    //  fprintf (stderr,  "callback: stream = %p, len = %d, BUFSIZE = %d\n", stream,
    //	   len, BUFSIZE);
    //  fclose (fp);
    //  close (fd);
    //  fprintf (stderr, "callback: stream = %p\n", stream);

    if(len != ourself->num_samples)
        gui_fatal("len = %d, expected %d", len, ourself->num_samples);
    ourself->sdl_stream = stream;
    while(ourself->sdl_stream != 0)
        usleep(1);
#warning TODO signal something to say that we can write
}

bool SDLSound::sound_init()
{
    SDL_AudioSpec spec;
    syn68k_addr_t my_callback;
    int sysret;

    semid = -1; /* Semaphore id */
    sound_on = 0; /* 1 if we are generating interrupts */
    t1 = 0;
    sdl_stream = NULL;
    ROMlib_SND_RATE = NON_RUNNING_SOUND_RATE; /* we need to set this to something,
                           in case we don't succeed when we
                           try to initialize.  May as well set
                           it to the common Mac value. */

    have_sound_p = false;

    if(sound_disabled_p)
        goto fail;

    if(SDL_AudioInit(sdl_audio_driver_name) != 0)
    {
        const char *err;

        err = SDL_GetError();
        fprintf(stderr, "SDL_Init(SDL_INIT_AUDIO) failed: '%s'\n",
                err ? err : "(NULL)");
        return false;
    }

    if(sdl_audio_driver_name)
    {
        const char *sanity_check_name;
        bool success = false;

        sanity_check_name = SDL_GetCurrentAudioDriver();
        if(sanity_check_name)
            success = strcmp(sdl_audio_driver_name, sanity_check_name) == 0;
        if(!success)
        {
            fprintf(stderr, "Wanted '%s', got '%s'", sdl_audio_driver_name,
                    sanity_check_name ? sanity_check_name : "NULL");
            return false;
        }
    }

    memset(&spec, 0, sizeof spec);

    spec.freq = ROMlib_SND_RATE;
    spec.format = AUDIO_U8;
    spec.channels = 1;
    spec.samples = BUFSIZE;
    spec.callback = hunger_callback;
    spec.userdata = this;

#if !defined(HAS_OPENAUDIO_EX)
#define SDL_OpenAudioEx(a, b, c, d) SDL_OpenAudio(a, b)
#endif

    if(SDL_OpenAudioEx(&spec, NULL, sdl_audio_driver_name, 0) < 0)
    {
        if(!sdl_audio_driver_name || SDL_OpenAudioEx(&spec, NULL, NULL, 0) < 0)
        {
            fprintf(stderr, "SDL_OpenAudio failed '%s'\n", SDL_GetError());
            goto fail;
        }
    }

    num_samples = spec.samples;
    if(spec.samples != BUFSIZE / 2)
    {
        if(spec.samples == BUFSIZE)
            fprintf(stderr, "Got number of samples we asked for\n");
        else if(spec.samples == 0 || spec.samples > BUFSIZE)
        {
            fprintf(stderr, "Failing: Got %d samples, asked for %d\n", spec.samples,
                    BUFSIZE);
            goto fail;
        }
        else
            fprintf(stderr, "Got %d samples\n", spec.samples);
    }

    memset(buf, 0x80, sizeof(buf));
    sdl_write(buf, num_samples);

    semid = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0666);
    if(semid < 0)
        gui_fatal("Couldn't get semaphore.  Kernel needs to have System V IPC "
                  "compiled in (CONFIG_SYSVIPC).  "
                  "Until you recompile your kernel, try using the "
                  "\"-nosound\" command line option.");

    atexit(sound_sdl_shutdown_at_exit); /* make sure semid gets freed */

    my_callback = callback_install(sound_callback, NULL);
    *(syn68k_addr_t *)SYN68K_TO_US(M68K_SOUND_VECTOR * 4) = BigEndianValue(my_callback);

    {
        sigset_t all_signals, current_mask;
        pthread_t thread;

        sigfillset(&all_signals);
        sigdelset(&all_signals, SIGIO);
        sigprocmask(SIG_SETMASK, &all_signals, &current_mask);
        //    fprintf (stderr, "about to start thread\n");
        sysret = pthread_create(&thread, NULL, loop, this);
        sigprocmask(SIG_SETMASK, &current_mask, 0);
    }
    if(sysret != 0)
        goto fail;

    /*patl_signal ();*/

    have_sound_p = true;

    /* Success! */
    return true;

fail:
#warning TODO some sort of sdl shutdown
    fprintf(stderr, "failure of some sort\n");
    return false;
}
