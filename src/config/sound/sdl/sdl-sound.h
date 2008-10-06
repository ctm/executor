#if !defined (_SDL_SOUND_H_)
#define _SDL_SOUND_H_

#define SOUND_SDL

#include "rsys/sounddriver.h"

extern boolean_t sound_sdl_init (sound_driver_t *s);
extern void ROMlib_set_sdl_audio_driver_name (const char *str);

#endif /* !_SDL_SOUND_H_ */
