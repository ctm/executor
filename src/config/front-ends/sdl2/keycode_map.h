#ifndef _KEYCODE_MAP_H_
#define _KEYCODE_MAP_H_

#include <unordered_map>

#include <SDL_keycode.h>

enum
{
    NOTAKEY = 0x89
};

void init_sdlk_to_mkv(void);

extern std::unordered_map<SDL_Keycode, unsigned char> sdlk_to_mkv;

#endif