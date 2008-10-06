/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_soundfake[] =
	    "$Id: soundfake.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/sounddriver.h"
#include "rsys/m68kint.h"
#include <TimeMgr.h>

/* This driver "goes through the motions" of playing a sound without
 * actually interacting with any sound hardware.  The intent is to
 * allow systems with and without sound hardware to be indistinguishable
 * from the perspective of the program being emulated.  For example,
 * this driver should cause callbacks to get called at the right times,
 * etc.
 */

#define LOGBUFSIZE 11
#define FAKE_BUF_SIZE (1U << LOGBUFSIZE)
#define NUM_FAKE_BUFS 4U

#define MSECS_FOR_BUFFER_TO_PLAY ((FAKE_BUF_SIZE * 1000L) / SND_RATE)

static snd_time t1;

/* # of fake buffers currently enqueued. */
static int num_fake_buffers_enqueued;

static syn68k_addr_t fake_sound_callback;

/* Set to TRUE when we're shutting down, and don't want any new sound
 * to creep in.
 */
static boolean_t no_more_sound_p;

static TMTask fake_sound_tm_task;

static boolean_t
sound_fake_works_p (sound_driver_t *s)
{
  return !no_more_sound_p;
}

static void
sound_fake_hunger_start (sound_driver_t *s)
{
  t1 += FAKE_BUF_SIZE;
}

static struct hunger_info
sound_fake_get_hunger_info (sound_driver_t *s)
{
  struct hunger_info info;

  info.buf = NULL;	/* no buffer at all */
  info.bufsize = NUM_FAKE_BUFS * FAKE_BUF_SIZE;
  info.t2 = t1 + FAKE_BUF_SIZE;
  info.t3 = info.t2 + FAKE_BUF_SIZE;
  info.t4 = info.t3;

  return info;
}

static void
sound_fake_clear_pending (sound_driver_t *s)
{
  fake_sound_tm_task.tmCount = CLC (-1);
  num_fake_buffers_enqueued = 0;
}

static void
_note_sound_interrupt (void)
{
  if (!no_more_sound_p && num_fake_buffers_enqueued < 2)
    sound_callback (0, NULL);
}

/* Installs a time manager task to call back at the requested time. */
static void
set_up_tm_task (void)
{
  fake_sound_tm_task.tmAddr = (ProcPtr) CL ((uint32) fake_sound_callback);
  InsTime ((QElemPtr) &fake_sound_tm_task);
  PrimeTime ((QElemPtr) &fake_sound_tm_task, MSECS_FOR_BUFFER_TO_PLAY);
}

static void
sound_fake_hunger_finish (sound_driver_t *s)
{
  ++num_fake_buffers_enqueued;
  if (num_fake_buffers_enqueued > (int) NUM_FAKE_BUFS)
    warning_unexpected ("Too many fake sound buffers got enqueued; this "
			"should not be possible.");

  /* If we're the only sound enqueued, we need to add a time manager task
   * to give us a callback when appropriate.
   */
  if (num_fake_buffers_enqueued == 1)
    set_up_tm_task ();

  _note_sound_interrupt ();	/* request even more sound if appropriate. */
}

static syn68k_addr_t
handle_fake_sound_callback (syn68k_addr_t addr, void *junk)
{
  if (num_fake_buffers_enqueued > 0)
    --num_fake_buffers_enqueued;

  if (!no_more_sound_p && num_fake_buffers_enqueued > 0)
    {
      M68kReg saved_regs[16];
      CCRElement saved_ccnz, saved_ccn, saved_ccc, saved_ccv, saved_ccx;

      /* Save the 68k registers and cc bits away. */
      memcpy (saved_regs, &cpu_state.regs, sizeof saved_regs);
      saved_ccnz = cpu_state.ccnz;
      saved_ccn  = cpu_state.ccn;
      saved_ccc  = cpu_state.ccc;
      saved_ccv  = cpu_state.ccv;
      saved_ccx  = cpu_state.ccx;

      set_up_tm_task ();
      _note_sound_interrupt ();

      memcpy (&cpu_state.regs, saved_regs, sizeof saved_regs);
      cpu_state.ccnz = saved_ccnz;
      cpu_state.ccn  = saved_ccn;
      cpu_state.ccc  = saved_ccc;
      cpu_state.ccv  = saved_ccv;
      cpu_state.ccx  = saved_ccx;
    }

  return POPADDR ();
}

void
sound_fake_go (sound_driver_t *s)
{
  _note_sound_interrupt ();
}

void
sound_fake_stop (sound_driver_t *s)
{
}

static void
sound_fake_shutdown (sound_driver_t *s)
{
  no_more_sound_p = TRUE;	/* No more callbacks, etc. */
  num_fake_buffers_enqueued = 0;
}

static boolean_t
sound_fake_silent_p (sound_driver_t *s)
{
  return TRUE;
}

boolean_t
sound_fake_init (sound_driver_t *s)
{
  fake_sound_callback = callback_install (handle_fake_sound_callback, NULL);
  no_more_sound_p = FALSE;
  num_fake_buffers_enqueued = 0;

  s->sound_init            = sound_fake_init;
  s->sound_shutdown        = sound_fake_shutdown;
  s->sound_works_p         = sound_fake_works_p;
  s->sound_silent_p        = sound_fake_silent_p;
  s->sound_hunger_finish   = sound_fake_hunger_finish;
  s->sound_go              = sound_fake_go;
  s->sound_stop            = sound_fake_stop;
  s->sound_hunger_start    = sound_fake_hunger_start;
  s->sound_get_hunger_info = sound_fake_get_hunger_info;
  s->sound_clear_pending   = sound_fake_clear_pending;

  return TRUE;
}
