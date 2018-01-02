#include "keycode_map.h"
#include "rsys/common.h"
#include "rsys/keyboard.h"

#include <SDL.h>

std::unordered_map<SDL_Keycode, unsigned char> sdlk_to_mkv;

typedef struct
{
    SDL_Keycode sdlk;
    unsigned char mkv;
} sdl_to_mkv_map_t;

static sdl_to_mkv_map_t map[] = {
    { SDLK_BACKSPACE, MKV_BACKSPACE },
    { SDLK_TAB, MKV_TAB },
    { SDLK_CLEAR, NOTAKEY },
    { SDLK_RETURN, MKV_RETURN },
    { SDLK_ESCAPE, MKV_ESCAPE },
    { SDLK_SPACE, MKV_SPACE },
    { SDLK_QUOTE, MKV_TICK },
    { SDLK_COMMA, MKV_COMMA },
    { SDLK_MINUS, MKV_MINUS },
    { SDLK_PERIOD, MKV_PERIOD },
    { SDLK_SLASH, MKV_SLASH },
    { SDLK_0, MKV_0 },
    { SDLK_1, MKV_1 },
    { SDLK_2, MKV_2 },
    { SDLK_3, MKV_3 },
    { SDLK_4, MKV_4 },
    { SDLK_5, MKV_5 },
    { SDLK_6, MKV_6 },
    { SDLK_7, MKV_7 },
    { SDLK_8, MKV_8 },
    { SDLK_9, MKV_9 },
    { SDLK_SEMICOLON, MKV_SEMI },
    { SDLK_EQUALS, MKV_EQUAL },
    { SDLK_KP_0, MKV_NUM0 },
    { SDLK_KP_1, MKV_NUM1 },
    { SDLK_KP_2, MKV_NUM2 },
    { SDLK_KP_3, MKV_NUM3 },
    { SDLK_KP_4, MKV_NUM4 },
    { SDLK_KP_5, MKV_NUM5 },
    { SDLK_KP_6, MKV_NUM6 },
    { SDLK_KP_7, MKV_NUM7 },
    { SDLK_KP_8, MKV_NUM8 },
    { SDLK_KP_9, MKV_NUM9 },
    { SDLK_KP_PERIOD, MKV_NUMPOINT },
    { SDLK_KP_DIVIDE, MKV_NUMDIVIDE },
    { SDLK_KP_MULTIPLY, MKV_NUMMULTIPLY },
    { SDLK_KP_MINUS, MKV_NUMMINUS },
    { SDLK_KP_PLUS, MKV_NUMPLUS },
    { SDLK_KP_ENTER, MKV_NUMENTER },
    { SDLK_LEFTBRACKET, MKV_LEFTBRACKET },
    { SDLK_BACKSLASH, MKV_BACKSLASH },
    { SDLK_RIGHTBRACKET, MKV_RIGHTBRACKET },
    { SDLK_BACKQUOTE, MKV_BACKTICK },
    { SDLK_a, MKV_a },
    { SDLK_b, MKV_b },
    { SDLK_c, MKV_c },
    { SDLK_d, MKV_d },
    { SDLK_e, MKV_e },
    { SDLK_f, MKV_f },
    { SDLK_g, MKV_g },
    { SDLK_h, MKV_h },
    { SDLK_i, MKV_i },
    { SDLK_j, MKV_j },
    { SDLK_k, MKV_k },
    { SDLK_l, MKV_l },
    { SDLK_m, MKV_m },
    { SDLK_n, MKV_n },
    { SDLK_o, MKV_o },
    { SDLK_p, MKV_p },
    { SDLK_q, MKV_q },
    { SDLK_r, MKV_r },
    { SDLK_s, MKV_s },
    { SDLK_t, MKV_t },
    { SDLK_u, MKV_u },
    { SDLK_v, MKV_v },
    { SDLK_w, MKV_w },
    { SDLK_x, MKV_x },
    { SDLK_y, MKV_y },
    { SDLK_z, MKV_z },
    { SDLK_DELETE, MKV_DELFORWARD },
    { SDLK_F1, MKV_F1 },
    { SDLK_F2, MKV_F2 },
    { SDLK_F3, MKV_F3 },
    { SDLK_F4, MKV_F4 },
    { SDLK_F5, MKV_F5 },
    { SDLK_F6, MKV_F6 },
    { SDLK_F7, MKV_F7 },
    { SDLK_F8, MKV_F8 },
    { SDLK_F9, MKV_F9 },
    { SDLK_F10, MKV_F10 },
    { SDLK_F11, MKV_F11 },
    { SDLK_F12, MKV_F12 },
    { SDLK_F13, MKV_F13 },
    { SDLK_F14, MKV_F14 },
    { SDLK_F15, MKV_F15 },
    { SDLK_PAUSE, MKV_PAUSE },
    { SDLK_NUMLOCKCLEAR, MKV_NUMCLEAR },
    { SDLK_UP, MKV_UPARROW },
    { SDLK_DOWN, MKV_DOWNARROW },
    { SDLK_RIGHT, MKV_RIGHTARROW },
    { SDLK_LEFT, MKV_LEFTARROW },
    { SDLK_INSERT, MKV_HELP },
    { SDLK_HOME, MKV_HOME },
    { SDLK_END, MKV_END },
    { SDLK_PAGEUP, MKV_PAGEUP },
    { SDLK_PAGEDOWN, MKV_PAGEDOWN },
    { SDLK_CAPSLOCK, MKV_CAPS },
    { SDLK_SCROLLLOCK, MKV_SCROLL_LOCK },
    { SDLK_RSHIFT, MKV_RIGHTSHIFT },
    { SDLK_LSHIFT, MKV_LEFTSHIFT },
    { SDLK_RCTRL, MKV_RIGHTCNTL },
    { SDLK_LCTRL, MKV_LEFTCNTL },
#ifdef MACOSX
    { SDLK_RGUI, MKV_CLOVER },
    { SDLK_LGUI, MKV_CLOVER },
    { SDLK_RALT, MKV_RIGHTOPTION },
    { SDLK_LALT, MKV_LEFTOPTION },
#else
    { SDLK_RGUI, MKV_RIGHTOPTION },
    { SDLK_LGUI, MKV_LEFTOPTION },
    { SDLK_RALT, MKV_RIGHTOPTION },
    { SDLK_LALT, MKV_CLOVER },
#endif
    { SDLK_HELP, MKV_HELP },
    { SDLK_PRINTSCREEN, MKV_PRINT_SCREEN },
    { SDLK_SYSREQ, NOTAKEY },
    { SDLK_MENU, NOTAKEY },
};

void init_sdlk_to_mkv(void)
{
    static bool been_here = false;

    if(!been_here)
    {
        unsigned int i;

        for(i = 0; i < NELEM(map); ++i)
        {
            SDL_Keycode sdlk;
            unsigned char mkv;

            sdlk = map[i].sdlk;
            mkv = map[i].mkv;
            sdlk_to_mkv[sdlk] = mkv;
        }
        been_here = true;
    }
}
