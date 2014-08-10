//
//  SoundOSX.h
//  CocoaExecutor
//
//  Created by C.W. Betts on 8/2/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#ifndef __CocoaExecutor__SoundOSX__
#define __CocoaExecutor__SoundOSX__

#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>

#include "rsys/common.h"
#include "rsys/sounddriver.h"

namespace Executor {
  class SoundOSX : public SoundDriver {
  public:
	virtual bool sound_init();
	virtual void sound_shutdown();
	virtual bool sound_works();
	virtual bool sound_silent();
	virtual void sound_go();
	virtual void sound_stop();
	virtual void HungerStart();
	virtual struct hunger_info GetHungerInfo();
	virtual void HungerFinish();
	virtual void sound_clear_pending();
	  virtual bool HasSoundClearPending() {return false;}
    
  private:
	AudioUnit		AudioUnit;
	size_t			BufferOffset;
	char			*Buffer;
    static syn68k_addr_t handle_fake_sound_callback (syn68k_addr_t addr, void *ourself);

  };
}

#endif /* defined(__CocoaExecutor__SoundOSX__) */
