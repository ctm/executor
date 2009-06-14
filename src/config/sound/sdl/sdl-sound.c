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

#define LOGBUFSIZE 11  /* Must be between 7 and 17 decimal */

/*
 * There's what appears to be a bug in some of the SDLs out there that
 * results in SDL choosing to use one half the number of samples that we ask
 * for.  As such, we're going to make room for twice the amount we want and
 * then ask for twice the amount.  If we get it, oh well, it just means
 * more latency.
 */

static int num_samples;

#define BUFSIZE (1 << (LOGBUFSIZE+1)) /* +1 as bug workaround */

/* Stack size for child thread */
#define STACKSIZE 16384

static int semid = -1;  /* Semaphore id */
static int sound_on = 0; /* 1 if we are generating interrupts */
static boolean_t have_sound_p; /* TRUE if sound is supported */

enum { NON_RUNNING_SOUND_RATE = 22255 };

PRIVATE char *sdl_audio_driver_name = NULL;

PUBLIC void
ROMlib_set_sdl_audio_driver_name (const char *str)
{
  if (sdl_audio_driver_name)
    free (sdl_audio_driver_name);
  if (str)
    sdl_audio_driver_name = strdup (str);
  else
    sdl_audio_driver_name = NULL;
}

PUBLIC int ROMlib_SND_RATE =
            NON_RUNNING_SOUND_RATE; /* we need to set this to something,
				       in case we don't succeed when we
				       try to initialize.  May as well set
				       it to the common Mac value. */


/* Wait on the semaphore (atomic decrement) */

static void
patl_wait (void)
{
  struct sembuf op_wait[] = {
    { 0, -1, 0 } };

  while (semop (semid, op_wait, 1) < 0)
    {
      if (errno == EINTR)
	continue;
      if (errno != EIDRM)
	perror ("semop failed");
      pthread_exit (NULL);
    }
}

/* Signal the semaphore (atomic increment) */

static void
patl_signal (void)
{
  struct sembuf op_signal[] = {
    { 0, 1, 0 } };

  while (semop (semid, op_signal, 1) < 0)
    {
      if (errno == EINTR)
	continue;
      gui_fatal ("Signal failure");
    }
}

static snd_time t1 = 0;

static boolean_t
sound_sdl_works_p (sound_driver_t *s)
{
  return have_sound_p;
}

static void
sound_sdl_go (sound_driver_t *s)
{
  sound_on = 1;
  patl_signal ();

 SDL_PauseAudio(0);
}

static void
sound_sdl_stop (sound_driver_t *s)
{
  sound_on = 0;
}

/* Do any bookkeeping needed to start feeding a hungry device */

static void
sound_sdl_hunger_start (sound_driver_t *s)
{
  t1 += num_samples;
}

static unsigned char buf[7*BUFSIZE];

/* Figure out how to feed the hungry output device. */

static struct hunger_info
sound_sdl_get_hunger_info (sound_driver_t *s)
{
  struct hunger_info info;

  info.buf = buf;
  info.bufsize = 7 * num_samples;

  info.t2 = t1 + num_samples;
  info.t3 = info.t2 + num_samples;
  info.t4 = info.t3;

  return info;
}

PRIVATE Uint8 *sdl_stream = NULL;

static ssize_t
sdl_write (const void *buf, size_t len)
{
  //  fprintf (stderr, "sdl_write: stream = %p, len = %d, buf[0]= 0x%02x, buf[1] = 0x%02x\n", sdl_stream, (int) len, ((uint8_t *) buf)[0], ((uint8_t *) buf)[1]);

  if (sdl_stream)
    {
      memcpy (sdl_stream, buf, len);
      sdl_stream = NULL;
    }

  return len;
}

/* Assuming that the information returned by snd_get_hunger_info was
   honored, send the samples off to the device. */

static void
sound_sdl_hunger_finish (sound_driver_t *s)
{
  struct hunger_info info;

  info = sound_sdl_get_hunger_info (s);

  while (sdl_write (info.buf + (info.t2 % info.bufsize), num_samples)
	 < 0)
    {
      if (errno != EINTR)
	gui_fatal ("Write failed");
    }

  memset (info.buf + (info.t2 % info.bufsize), 0x80, num_samples);

  patl_signal ();
}

#if defined (_SEM_SEMUN_UNDEFINED)
union semun
{
  int unused_baggage;
};
#endif

static void
sound_sdl_shutdown (sound_driver_t *s)
{
  if (semid >= 0)
    semctl (semid, 0, IPC_RMID, (union semun)0);

  if (have_sound_p)
    {
      /* possibly kill the thread here */
      have_sound_p = FALSE;
    }

  sdl_stream = NULL;
}

static void
sound_sdl_clear_pending (sound_driver_t *s)
{
}

static boolean_t
sound_sdl_silent_p (sound_driver_t *s)
{
  return FALSE;
}

static void
sound_sdl_shutdown_at_exit (void)
{
  sound_sdl_shutdown (&sound_driver);
}

PRIVATE void
sdl_wait_until_callback_has_been_called (void)
{
#warning TODO usleep is not the answer
  while (sdl_stream == 0)
    usleep(1);
}

/* Infinite loop for the "sound" thread.  Waits until the audio device
   is ready to accept input, then set a flag for the emulator
   thread. */

static void *
loop (void *unused)
{
  while (TRUE)
    {
      patl_wait ();
      //      fprintf (stderr, "waiting until callback has been called\n");
      sdl_wait_until_callback_has_been_called ();

      /* Request interrupt */
      if (sound_on)
	{
	  cpu_state.interrupt_pending[M68K_SOUND_PRIORITY] = 1;
	  cpu_state.interrupt_status_changed = INTERRUPT_STATUS_CHANGED;
	}
    }

  return NULL; /* won't get here */
}

static void
sound_sdl_hunger_callback(void *unused, Uint8 *stream, int len)
{
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

  if (len != num_samples)
    gui_fatal ("len = %d, expected %d", len, num_samples);
  sdl_stream = stream;
  while (sdl_stream != 0)
    usleep (1);
#warning TODO signal something to say that we can write
}

boolean_t
sound_sdl_init (sound_driver_t *s)
{
  SDL_AudioSpec spec;
  syn68k_addr_t my_callback;
  int sysret;

  have_sound_p = FALSE;

  if (sound_disabled_p)
    goto fail;

  if (SDL_AudioInit (sdl_audio_driver_name) != 0)
    {
      char *err;

      err = SDL_GetError ();
      fprintf (stderr, "SDL_Init(SDL_INIT_AUDIO) failed: '%s'\n",
	       err ? err : "(NULL)");
      return FALSE;
    }

  if (sdl_audio_driver_name)
    {
      char *sanity_check_name;
      int sanity_len;
      boolean_t success;

      /* we add 3 below due to ambiguity in the documentation for
	 SDL_AudioDriverName.  It doesn't hurt to add 3 here. */

      sanity_len = strlen (sdl_audio_driver_name) + 3;
      sanity_check_name = alloca (sanity_len);
      SDL_AudioDriverName (sanity_check_name, sanity_len);
      success = strcmp (sdl_audio_driver_name, sanity_check_name) == 0;
      if (!success)
	{
	  fprintf (stderr, "Wanted '%s', got '%s'", sdl_audio_driver_name,
		   sanity_check_name);
	  return FALSE;
	}
    }

  memset (&spec, 0, sizeof spec);

  spec.freq = ROMlib_SND_RATE;
  spec.format = AUDIO_U8;
  spec.channels = 1;
  spec.samples = BUFSIZE;
  spec.callback = sound_sdl_hunger_callback;
  spec.userdata = NULL;

#if !defined (HAS_OPENAUDIO_EX)
#define SDL_OpenAudioEx(a,b,c,d) SDL_OpenAudio (a, b)
#endif

  if ( SDL_OpenAudioEx(&spec, NULL, sdl_audio_driver_name, 0) < 0 )
    {
      if (!sdl_audio_driver_name ||
	  SDL_OpenAudioEx(&spec, NULL, NULL, 0) < 0 )
	{
	  fprintf (stderr, "SDL_OpenAudio failed '%s'\n", SDL_GetError ());
	  goto fail;
	}
    }

  num_samples = spec.samples;
  if (spec.samples != BUFSIZE / 2) {
    if (spec.samples == BUFSIZE)
      fprintf(stderr, "Got number of samples we asked for\n");
    else if (spec.samples == 0 || spec.samples > BUFSIZE) {
      fprintf(stderr, "Failing: Got %d samples, asked for %d\n", spec.samples,
              BUFSIZE);
      goto fail;
    } else
      fprintf(stderr, "Got %d samples\n", spec.samples);
  }

  memset (buf, 0x80, sizeof (buf));
  sdl_write (buf, num_samples);

  semid = semget (IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0666);
  if (semid < 0)
    gui_fatal ("Couldn't get semaphore.  Kernel needs to have System V IPC "
	       "compiled in (CONFIG_SYSVIPC).  "
	       "Until you recompile your kernel, try using the "
	       "\"-nosound\" command line option.");

  atexit (sound_sdl_shutdown_at_exit); /* make sure semid gets freed */

  my_callback = callback_install (sound_callback, NULL);
  *(syn68k_addr_t *) SYN68K_TO_US(M68K_SOUND_VECTOR * 4) = CL (my_callback);

  {
    sigset_t all_signals, current_mask;
    pthread_t thread;

    sigfillset (&all_signals);
    sigdelset (&all_signals, SIGIO);
    sigprocmask (SIG_SETMASK, &all_signals, &current_mask);
    //    fprintf (stderr, "about to start thread\n");
    sysret = pthread_create (&thread, NULL, loop, NULL);
    sigprocmask (SIG_SETMASK, &current_mask, 0);
  }
  if (sysret != 0)
    goto fail;

  /*patl_signal ();*/

  have_sound_p = TRUE;

  s->sound_init            = sound_sdl_init;
  s->sound_shutdown        = sound_sdl_shutdown;
  s->sound_works_p         = sound_sdl_works_p;
  s->sound_silent_p        = sound_sdl_silent_p;
  s->sound_hunger_finish   = sound_sdl_hunger_finish;
  s->sound_go              = sound_sdl_go;
  s->sound_stop            = sound_sdl_stop;
  s->sound_hunger_start    = sound_sdl_hunger_start;
  s->sound_get_hunger_info = sound_sdl_get_hunger_info;
  s->sound_clear_pending   = sound_sdl_clear_pending;

  /* Success! */
  return TRUE;

 fail:
#warning TODO some sort of sdl shutdown
  fprintf (stderr, "failure of some sort\n");
  return FALSE;
}
