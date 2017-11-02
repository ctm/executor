/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_djgpp_sound[] =
	    "$Id: djgpp-sound.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/sounddriver.h"
#include "rsys/m68kint.h"
#include "rsys/checkpoint.h"
#include "djgpp-sound.h"
#include "dpmilock.h"
#include "sb_lib.h"


#define LOGBUFSIZE 11
#define BUFSIZE (1U << LOGBUFSIZE)
#define NUM_BUFS 4U

#define TIME_TO_BUFFER(t) (((t) / BUFSIZE) % NUM_BUFS)

/* true iff we really support sound. */
static bool have_sound_p;

static unsigned char buf[NUM_BUFS][BUFSIZE] __attribute__ ((aligned (4)));


static bool
sound_djgpp_works_p (sound_driver_t *s)
{
  return have_sound_p;
}

static snd_time t1;

static void
sound_djgpp_hunger_start (sound_driver_t *s)
{
  memset (&buf[TIME_TO_BUFFER (t1)][0], 0x80, BUFSIZE);
  t1 += BUFSIZE;
  warning_sound_log ("t1 == %d\n", (int) t1);
}

static struct hunger_info
sound_djgpp_get_hunger_info (sound_driver_t *s)
{
  struct hunger_info info;

  info.buf = &buf[0][0];
  info.bufsize = sizeof (buf);

  info.t2 = t1 + BUFSIZE;
  info.t3 = info.t2 + BUFSIZE;
  info.t4 = info.t3;

  warning_sound_log ("info.t2 == %d, t3 == %d, t4 == %d\n",
		     (int) info.t2, (int) info.t3, (int) info.t4);

  return info;
}


/* Don't make these functions static!  Otherwise they might get
 * rearranged and not get locked down properly.
 */
void sound_djgpp_note_sound_interrupt_begin (void) {}
void
sound_djgpp_note_sound_interrupt (void)
{
  /* Don't get too backlogged. */
  if (sb_numInQueue < 2)
    {
      cpu_state.interrupt_pending[M68K_SOUND_PRIORITY] = 1;
      cpu_state.interrupt_status_changed = INTERRUPT_STATUS_CHANGED;
    }
}
void sound_djgpp_note_sound_interrupt_end (void) {}


static void
sound_djgpp_hunger_finish (sound_driver_t *s)
{
  struct hunger_info info;

  info = sound_djgpp_get_hunger_info (s);
  warning_sound_log ("queueing sample in buffer %d, t2 == %d\n",
		     (int) TIME_TO_BUFFER (info.t2), (int) info.t2);

  sb_enqueue_sample (&buf[TIME_TO_BUFFER (info.t2)][0], BUFSIZE);

  /* Ask for more sound immediately, if appropriate. */
  sound_djgpp_note_sound_interrupt ();
}


static void
sound_djgpp_go (sound_driver_t *s)
{
  warning_sound_log (NULL_STRING);

  /* Get the ball rolling. */
  sound_djgpp_note_sound_interrupt ();
}

static void
sound_djgpp_stop (sound_driver_t *s)
{
  warning_sound_log (NULL_STRING);
}

static void
sound_djgpp_shutdown (sound_driver_t *s)
{
  if (have_sound_p)
    {
      sb_uninstall_driver ();
      have_sound_p = false;
    }
}

static void
sound_djgpp_shutdown_at_exit (void)
{
  sound_djgpp_shutdown (&sound_driver);
}

static void
sound_djgpp_clear_pending (sound_driver_t *s)
{
  /* We can't actually do this...? */
}

static bool
sound_djgpp_silent_p (sound_driver_t *s)
{
  return false;
}

bool
sound_djgpp_init (sound_driver_t *s)
{
  warning_sound_log ("About to install sound driver...");

  if (sound_disabled_p)
    have_sound_p = false;
  else
    {
      sb_status success;

      checkpoint_sound (checkpointp, begin);
      success = sb_install_driver (sound_djgpp_note_sound_interrupt);
      have_sound_p = (success == SB_SUCCESS);
    }

  if (!have_sound_p)
    {
      if (!sound_disabled_p)
	warning_sound_log ("No sound driver detected: %s", sb_driver_error);
    }
  else
    {
      syn68k_addr_t my_callback;

      dpmi_lock_memory (buf, sizeof buf);
      dpmi_lock_memory ((char *) sound_djgpp_note_sound_interrupt_begin,
			((char *) sound_djgpp_note_sound_interrupt_end
			 - (char *) sound_djgpp_note_sound_interrupt_begin));

      /* Silence is golden. */
      memset (buf, 0x80, sizeof buf);
      
      warning_sound_log ("Successfully detected and initialized SoundBlaster "
			 "compatible sound card\n");

      my_callback = callback_install (sound_callback, NULL);
      *(syn68k_addr_t *) SYN68K_TO_US(M68K_SOUND_VECTOR * 4) = CL (my_callback);

      t1 = 0;

      s->sound_init            = sound_djgpp_init;
      s->sound_shutdown        = sound_djgpp_shutdown;
      s->sound_works_p         = sound_djgpp_works_p;
      s->sound_silent_p        = sound_djgpp_silent_p;
      s->sound_hunger_finish   = sound_djgpp_hunger_finish;
      s->sound_go              = sound_djgpp_go;
      s->sound_stop            = sound_djgpp_stop;
      s->sound_hunger_start    = sound_djgpp_hunger_start;
      s->sound_get_hunger_info = sound_djgpp_get_hunger_info;
      s->sound_clear_pending   = sound_djgpp_clear_pending;
    }

  atexit (sound_djgpp_shutdown_at_exit);

  if (!sound_disabled_p)
    checkpoint_sound (checkpointp, end);
  return have_sound_p;
}
