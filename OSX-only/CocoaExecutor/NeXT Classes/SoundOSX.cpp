//
//  SoundOSX.cpp
//  CocoaExecutor
//
//  Created by C.W. Betts on 8/2/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#include "SoundOSX.h"

using namespace Executor;


bool SoundOSX::sound_init()
{
  return false;
}

void SoundOSX::sound_shutdown()
{
  
}

bool SoundOSX::sound_works()
{
  return false;
}

bool SoundOSX::sound_silent()
{
  return false;
}

void SoundOSX::sound_go()
{
  
}

void SoundOSX::sound_stop()
{
  
}

void SoundOSX::HungerStart()
{
  
}

struct hunger_info SoundOSX::GetHungerInfo()
{
  struct hunger_info theHung = {0};
  
  return theHung;
}

void SoundOSX::HungerFinish()
{
  
}

void SoundOSX::sound_clear_pending()
{
  
}

syn68k_addr_t SoundOSX::handle_fake_sound_callback (syn68k_addr_t addr, void *ourself)
{
  //SoundOSX* ourSelfUn = (SoundOSX*)ourself;
  //if (ourSelfUn->num_fake_buffers_enqueued > 0)
  //  --ourSelfUn->num_fake_buffers_enqueued;
  
  //if (!ourSelfUn->no_more_sound_p && ourSelfUn->num_fake_buffers_enqueued > 0)
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
    
    //ourSelfUn->set_up_tm_task ();
    //ourSelfUn->note_sound_interrupt ();
    
    memcpy (&cpu_state.regs, saved_regs, sizeof saved_regs);
    cpu_state.ccnz = saved_ccnz;
    cpu_state.ccn  = saved_ccn;
    cpu_state.ccc  = saved_ccc;
    cpu_state.ccv  = saved_ccv;
    cpu_state.ccx  = saved_ccx;
  }
  
  return POPADDR ();
}

bool SoundOSX::HasSoundClearPending()
{
  return false;
}

void Executor::ROMlib_callcompletion( void *chanp )
{
  
}

void Executor::ROMlib_outbuffer( char *buf, LONGINT nsamples, LONGINT rate, void *chanp)
{
  
}
