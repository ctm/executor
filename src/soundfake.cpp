/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_soundfake[] =
	    "$Id: soundfake.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/sounddriver.h"
#include "rsys/soundfake.h"
#include "rsys/m68kint.h"
#include "TimeMgr.h"

using namespace Executor;

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

bool SoundFake::sound_works()
{
  return !no_more_sound_p;
}

void SoundFake::HungerStart()
{
  t1 += FAKE_BUF_SIZE;
}

struct hunger_info SoundFake::GetHungerInfo()
{
  struct hunger_info info;

  info.buf = NULL;	/* no buffer at all */
  info.bufsize = NUM_FAKE_BUFS * FAKE_BUF_SIZE;
  info.t2 = t1 + FAKE_BUF_SIZE;
  info.t3 = info.t2 + FAKE_BUF_SIZE;
  info.t4 = info.t3;

  return info;
}

void SoundFake::sound_clear_pending()
{
  fake_sound_tm_task.tmCount = CLC (-1);
  num_fake_buffers_enqueued = 0;
}

void SoundFake::NoteSoundInterrupt(void)
{
  if (!no_more_sound_p && num_fake_buffers_enqueued < 2)
    sound_callback (0, NULL);
}

/* Installs a time manager task to call back at the requested time. */
void SoundFake::set_up_tm_task(void)
{
  fake_sound_tm_task.tmAddr = guest_cast<ProcPtr>( CL ((uint32) fake_sound_callback) );
  InsTime ((QElemPtr) &fake_sound_tm_task);
  PrimeTime ((QElemPtr) &fake_sound_tm_task, MSECS_FOR_BUFFER_TO_PLAY);
}

void SoundFake::HungerFinish()
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

  NoteSoundInterrupt();	/* request even more sound if appropriate. */
}

syn68k_addr_t SoundFake::handle_fake_sound_callback (syn68k_addr_t addr, void *ourself)
{
   SoundFake* ourSelfUn = (SoundFake*)ourself;
   if (ourSelfUn->num_fake_buffers_enqueued > 0)
	  --ourSelfUn->num_fake_buffers_enqueued;
   
   if (!ourSelfUn->no_more_sound_p && ourSelfUn->num_fake_buffers_enqueued > 0)
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
	  
	  ourSelfUn->set_up_tm_task ();
	  ourSelfUn->NoteSoundInterrupt();
	  
	  memcpy (&cpu_state.regs, saved_regs, sizeof saved_regs);
	  cpu_state.ccnz = saved_ccnz;
	  cpu_state.ccn  = saved_ccn;
	  cpu_state.ccc  = saved_ccc;
	  cpu_state.ccv  = saved_ccv;
	  cpu_state.ccx  = saved_ccx;
   }
   
   return POPADDR ();
}

void SoundFake::sound_go ()
{
  NoteSoundInterrupt ();
}

void SoundFake::sound_stop ()
{
}

void SoundFake::sound_shutdown ()
{
  no_more_sound_p = true;	/* No more callbacks, etc. */
  num_fake_buffers_enqueued = 0;
}

bool SoundFake::sound_silent()
{
  return true;
}

bool SoundFake::sound_init()
{
	fake_sound_callback = callback_install (handle_fake_sound_callback, this);
	no_more_sound_p = FALSE;
	num_fake_buffers_enqueued = 0;
	ROMlib_SND_RATE = 22255;
   
	return true;
}
