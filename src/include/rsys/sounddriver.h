#if !defined (_RSYS_SOUNDDRIVER_H_)
#define _RSYS_SOUNDDRIVER_H_

#include "rsys/soundopts.h"

namespace Executor {
class SoundDriver {
public:
  virtual bool sound_init() = 0;
  virtual void sound_shutdown() = 0;
  virtual bool sound_works() = 0;
  virtual bool sound_silent() = 0;
  virtual void sound_go() = 0;
  virtual void sound_stop() = 0;
  virtual void HungerStart() = 0;
  virtual struct hunger_info GetHungerInfo() = 0;
  virtual void HungerFinish() = 0;
  virtual void sound_clear_pending() = 0;
  virtual bool HasSoundClearPending() = 0;
    
  virtual ~SoundDriver();
};

/* Current sound driver in use. */
extern SoundDriver *sound_driver;

extern void sound_init (void);

#define _SOUND_CALL(func) (sound_driver->func ())

#define SOUND_SHUTDOWN()        _SOUND_CALL (sound_shutdown)
#define SOUND_WORKS_P()         _SOUND_CALL (sound_works)
#define SOUND_SILENT_P()        _SOUND_CALL (sound_silent)
#define SOUND_HUNGER_FINISH()   _SOUND_CALL (HungerFinish)
#define SOUND_GO()              _SOUND_CALL (sound_go)
#define SOUND_STOP()            _SOUND_CALL (sound_stop)
#define SOUND_HUNGER_START()    _SOUND_CALL (HungerStart)
#define SOUND_GET_HUNGER_INFO() _SOUND_CALL (GetHungerInfo)
#define SOUND_CLEAR_PENDING()   _SOUND_CALL (sound_clear_pending)
}
#include "sound-config.h"

#endif /* !_RSYS_SOUNDDRIVER_H_ */
