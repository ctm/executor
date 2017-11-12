/* define `SDL' the Simple DirectMedia Layer front-end */
#if !defined(SDL)

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

/* Define the front end */
#define SDL

/* We need this for the syn68k_addr_t definition */
#include <syn68k_public.h>

extern syn68k_addr_t
handle_sdl_events(syn68k_addr_t interrupt_addr, void *unused);

extern bool ROMlib_fullscreen_p;
extern bool ROMlib_hwsurface_p;
extern SDL_cond *ROMlib_shouldbeawake_cond;
extern SDL_mutex *ROMlib_shouldbeawake_mutex;

#endif /* !SDL */
