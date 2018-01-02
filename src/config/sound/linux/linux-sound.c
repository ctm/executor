/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include <sys/soundcard.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <linux/unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sched.h>
#include <pthread.h>

#include "rsys/vdriver.h"
#include "rsys/sounddriver.h"
#include "rsys/m68kint.h"
#include "linux-sound.h"

#define NUM_BUFS 2
#define LOGBUFSIZE 11 /* Must be between 7 and 17 decimal */
#define BUFSIZE (1 << LOGBUFSIZE)

static int semid = -1; /* Semaphore id */
static int devfd; /* Sound device file descriptor */
static int sound_on = 0; /* 1 if we are generating interrupts */
static bool have_sound_p; /* true if sound is supported */

/* Wait on the semaphore (atomic decrement) */

static void
patl_wait(void)
{
    struct sembuf op_wait[] = {
        { 0, -1, 0 }
    };

    while(semop(semid, op_wait, 1) < 0)
    {
        if(errno == EINTR)
            continue;
        pthread_exit(NULL);
    }
}

/* Signal the semaphore (atomic increment) */

static void
patl_signal(void)
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

static snd_time t1 = 0;

static bool
sound_linux_works_p(sound_driver_t *s)
{
    return have_sound_p;
}

static void
sound_linux_go(sound_driver_t *s)
{
    sound_on = 1;
    patl_signal();
}

static void
sound_linux_stop(sound_driver_t *s)
{
    sound_on = 0;
}

/* Do any bookkeeping needed to start feeding a hungry device */

static void
sound_linux_hunger_start(sound_driver_t *s)
{
    t1 += BUFSIZE;
}

static unsigned char buf[7 * BUFSIZE];

/* Figure out how to feed the hungry output device. */

static struct hunger_info
sound_linux_get_hunger_info(sound_driver_t *s)
{
    struct hunger_info info;

    info.buf = buf;
    info.bufsize = sizeof(buf) / sizeof(buf[0]);

    info.t2 = t1 + BUFSIZE;
    info.t3 = info.t2 + BUFSIZE;
    info.t4 = info.t3;

    return info;
}

/* Assuming that the information returned by snd_get_hunger_info was
   honored, send the samples off to the device. */

static void
sound_linux_hunger_finish(sound_driver_t *s)
{
    struct hunger_info info;

    info = sound_linux_get_hunger_info(s);

    while(write(devfd, info.buf + (info.t2 % info.bufsize), BUFSIZE)
          < 0)
    {
        if(errno != EINTR && errno != EAGAIN)
        {
            perror("Write failed");
            gui_fatal("Write failed");
        }
        if(errno == EAGAIN)
            usleep(1);
    }

    memset(info.buf + (info.t2 % info.bufsize), 0x80, BUFSIZE);

    patl_signal();
}

#if defined(_SEM_SEMUN_UNDEFINED)
union semun {
    int unused_baggage;
};
#endif

static void
sound_linux_shutdown(sound_driver_t *s)
{
    if(semid >= 0)
        semctl(semid, 0, IPC_RMID, (union semun)0);

    if(have_sound_p)
    {
        /* possibly kill the thread here */
        have_sound_p = false;
    }
}

static void
sound_linux_clear_pending(sound_driver_t *s)
{
}

static bool
sound_linux_silent_p(sound_driver_t *s)
{
    return false;
}

static void
sound_linux_shutdown_at_exit(void)
{
    sound_linux_shutdown(&sound_driver);
}

/* Infinite loop for the "sound" thread.  Waits until the audio device
   is ready to accept input, then set a flag for the emulator
   thread. */

static void *
loop(void *unused)
{
    while(true)
    {
        fd_set write_fds;

        FD_ZERO(&write_fds);
        FD_SET(devfd, &write_fds);

        patl_wait();

        while(select(getdtablesize(), NULL, &write_fds, NULL, NULL) < 0)
        {
            /* FIXME: there's only one errno in libc (at least in the older
	   * libc's) so technically checking and clobbering errno
	   * in this thread is not safe.
	   */
            if(errno == EINTR)
                continue;
            _exit(127);
        }

        /* Request interrupt */
        if(sound_on)
        {
            cpu_state.interrupt_pending[M68K_SOUND_PRIORITY] = 1;
            cpu_state.interrupt_status_changed = INTERRUPT_STATUS_CHANGED;
        }
    }
}

static bool
valid_rate(int got, int wanted)
{
    bool retval;
    int min, max;

    min = 0.95 * wanted; /* arbitrary fudge factors */
    max = 1.05 * wanted; /* feel free to change them */
    retval = got >= min && got <= max;
    return retval;
}

bool sound_linux_init(sound_driver_t *s)
{
    int arg;
    syn68k_addr_t my_callback;
    int sysret;

    have_sound_p = false;
    devfd = -1;

    if(sound_disabled_p)
        goto fail;

    devfd = open("/dev/dsp", O_WRONLY | O_NONBLOCK, 0);
    if(devfd < 0)
        goto fail;

    arg = (NUM_BUFS << 16) | LOGBUFSIZE;
    if(ioctl(devfd, SNDCTL_DSP_SETFRAGMENT, &arg) < 0)
        goto fail;

    arg = 1;
    if(ioctl(devfd, SOUND_PCM_WRITE_CHANNELS, &arg) < 0)
        goto fail;

    arg = SND_RATE;
    if(ioctl(devfd, SOUND_PCM_WRITE_RATE, &arg) < 0 || !valid_rate(arg, SND_RATE))
        goto fail;

    arg = 8;
    if(ioctl(devfd, SOUND_PCM_WRITE_BITS, &arg) < 0)
        goto fail;

    memset(buf, 0x80, sizeof(buf));
    write(devfd, buf, BUFSIZE);

    semid = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0666);
    if(semid < 0)
        gui_fatal("Couldn't get semaphore.  Kernel needs to have System V IPC "
                  "compiled in (CONFIG_SYSVIPC).  "
                  "Until you recompile your kernel, try using the "
                  "\"-nosound\" command line option.");

    atexit(sound_linux_shutdown_at_exit); /* make sure semid gets freed */

    my_callback = callback_install(sound_callback, NULL);
    *(syn68k_addr_t *)SYN68K_TO_US(M68K_SOUND_VECTOR * 4) = CL(my_callback);

    {
        sigset_t all_signals, current_mask;
        pthread_t thread;

        sigfillset(&all_signals);
        sigdelset(&all_signals, SIGIO);
        sigprocmask(SIG_SETMASK, &all_signals, &current_mask);
        sysret = pthread_create(&thread, NULL, loop, NULL);
        sigprocmask(SIG_SETMASK, &current_mask, 0);
    }
    if(sysret != 0)
        goto fail;

    /*patl_signal ();*/

    have_sound_p = true;

    s->sound_init = sound_linux_init;
    s->sound_shutdown = sound_linux_shutdown;
    s->sound_works_p = sound_linux_works_p;
    s->sound_silent_p = sound_linux_silent_p;
    s->sound_hunger_finish = sound_linux_hunger_finish;
    s->sound_go = sound_linux_go;
    s->sound_stop = sound_linux_stop;
    s->sound_hunger_start = sound_linux_hunger_start;
    s->sound_get_hunger_info = sound_linux_get_hunger_info;
    s->sound_clear_pending = sound_linux_clear_pending;

    /* Success! */
    return true;

fail:
    if(devfd != -1)
        close(devfd);
    devfd = -1;
    return false;
}
