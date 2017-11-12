/* 
 * Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(__syswm_map_h__)
#define __syswm_map_h__

extern int sdl_syswm_init(void);
extern int sdl_syswm_event(const SDL_Event *event);

#if defined(__unix__) || defined(MACOSX)

extern int os_current_screen_width(void);
extern int os_current_screen_height(void);

#endif

#endif
