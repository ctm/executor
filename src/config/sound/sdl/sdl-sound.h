#if !defined (_SDL_SOUND_H_)
#define _SDL_SOUND_H_

#define SOUND_SDL

#include <SDL_Sound/SDL_sound.h>
#include "rsys/sounddriver.h"

namespace Executor {
  
#define LOGBUFSIZE 11  /* Must be between 7 and 17 decimal */
  
  /*
   * There's what appears to be a bug in some of the SDLs out there that
   * results in SDL choosing to use one half the number of samples that we ask
   * for.  As such, we're going to make room for twice the amount we want and
   * then ask for twice the amount.  If we get it, oh well, it just means
   * more latency.
   */
  
#define BUFSIZE (1 << (LOGBUFSIZE+1)) /* +1 as bug workaround */
  
  class SDLSound : public SoundDriver {
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
    virtual bool HasSoundClearPending() { return true; }
    
  private:
    int num_samples;
    
    int semid;  /* Semaphore id */
    int sound_on; /* 1 if we are generating interrupts */
    bool have_sound_p; /* TRUE if sound is supported */
    
    unsigned char buf[7*BUFSIZE];
    void patl_wait();
    void patl_signal (void);
    snd_time t1;
    Uint8 *sdl_stream;
    ssize_t sdl_write (const void *buf, size_t len);
    void sdl_wait_until_callback_has_been_called (void);
    static void sound_sdl_shutdown_at_exit (void);
    static void *loop (void *unused);
    static void hunger_callback(void *unused, Uint8 *stream, int len);
  };
}

//extern boolean_t sound_sdl_init (sound_driver_t *s);
extern void ROMlib_set_sdl_audio_driver_name (const char *str);

#endif /* !_SDL_SOUND_H_ */
