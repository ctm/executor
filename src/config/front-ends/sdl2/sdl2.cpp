#include "rsys/common.h"
#include "rsys/host.h"
#include "rsys/vdriver.h"
#include "rsys/cquick.h" /* for ROMlib_log2 */
#include "rsys/adb.h"
#include "rsys/osevent.h"
#include "rsys/scrap.h"
#include "rsys/keyboard.h"
#include "rsys/parse.h"
#include "OSEvent.h"
#include "ToolboxEvent.h"
#include "SegmentLdr.h"
#include "ScrapMgr.h"

#include "keycode_map.h"

#include <SDL.h>
#ifdef WIN32
#include <SDL_syswm.h>
#include "sdl2_hwnd.h"
#endif

namespace Executor
{
/* These variables are required by the vdriver interface. */
uint8_t *vdriver_fbuf;
int vdriver_row_bytes;
int vdriver_width = 1024;
int vdriver_height = 768;
int vdriver_bpp = 8, vdriver_log2_bpp;
int vdriver_max_bpp, vdriver_log2_max_bpp;
vdriver_modes_t *vdriver_mode_list;

int host_cursor_depth = 1;
}

using namespace Executor;

namespace
{
vdriver_modes_t sdl_impotent_modes = { 0, 0 };
SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
SDL_Surface *sdlSurface;
}

void Executor::vdriver_opt_register(void)
{
}
bool Executor::vdriver_init(int _max_width, int _max_height, int _max_bpp,
                            bool fixed_p, int *argc, char *argv[])
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool Executor::vdriver_acceptable_mode_p(int width, int height, int bpp,
                                         bool grayscale_p, bool exact_match_p)
{
    if(bpp == 1 || bpp == 4 || bpp == 8 || bpp == 16 || bpp == 32)
        return true;
    else
        return false;
}

#ifdef WIN32
HWND Executor::getMainSDLWindow()
{
    SDL_SysWMinfo info;
    SDL_GetWindowWMInfo(sdlWindow, &info);
    return info.info.win.window;
}
#endif

bool Executor::vdriver_set_mode(int width, int height, int bpp, bool grayscale_p)
{
    printf("set_mode: %d %d %d", width, height, bpp);
    if(vdriver_fbuf)
        delete[] vdriver_fbuf;

    if(width)
        vdriver_width = width;
    if(height)
        vdriver_height = height;
    if(bpp)
        vdriver_bpp = bpp;
    vdriver_row_bytes = vdriver_width * vdriver_bpp / 8;
    vdriver_log2_bpp = ROMlib_log2[vdriver_bpp];
    vdriver_mode_list = &sdl_impotent_modes;

    vdriver_max_bpp = 8; //32;
    vdriver_log2_max_bpp = 3; //5;

    sdlWindow = SDL_CreateWindow("Window",
                                 SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED,
                                 vdriver_width, vdriver_height,
                                 0);
    //SDL_WINDOW_FULLSCREEN_DESKTOP);

    /*sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);

    SDL_RenderSetLogicalSize(sdlRenderer, vdriver_width, vdriver_height);
    SDL_SetRenderDrawColor(sdlRenderer, 128, 128, 128, 255);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderPresent(sdlRenderer);*/

    uint32_t pixelFormat;

    switch(vdriver_bpp)
    {
        case 1:
            pixelFormat = SDL_PIXELFORMAT_INDEX1LSB;
            break;
        case 4:
            pixelFormat = SDL_PIXELFORMAT_INDEX4LSB;
            break;
        case 8:
            pixelFormat = SDL_PIXELFORMAT_INDEX8;
            break;
        case 16:
            pixelFormat = SDL_PIXELFORMAT_RGB555;
            break;
        case 32:
            pixelFormat = SDL_PIXELFORMAT_BGRX8888;
            break;
        default:
            std::abort();
    }
    
    vdriver_fbuf = new uint8_t[vdriver_width * vdriver_height * 4];

#if 1
    uint32_t rmask, gmask, bmask, amask;
    int sdlBpp;
    SDL_PixelFormatEnumToMasks(pixelFormat, &sdlBpp, &rmask, &gmask, &bmask, &amask);

    sdlSurface = SDL_CreateRGBSurfaceFrom(
        vdriver_fbuf,
        vdriver_width, vdriver_height,
        sdlBpp,
        vdriver_row_bytes,
        rmask, gmask, bmask, amask);
#else
    sdlSurface = SDL_CreateRGBSurfaceWithFormatFrom(
        vdriver_fbuf,
        vdriver_width, vdriver_height,
        vdriver_bpp,
        vdriver_row_bytes,
        pixelFormat);
#endif

    return true;
}
void Executor::vdriver_set_colors(int first_color, int num_colors, const ColorSpec *colors)
{
    SDL_Color *sdlColors = (SDL_Color *)alloca(sizeof(SDL_Color) * num_colors);
    for(int i = 0; i < num_colors; i++)
    {
        sdlColors[i].a = 255;
        sdlColors[i].r = CW(colors[i].rgb.red) >> 8;
        sdlColors[i].g = CW(colors[i].rgb.green) >> 8;
        sdlColors[i].b = CW(colors[i].rgb.blue) >> 8;
    }

    SDL_SetPaletteColors(sdlSurface->format->palette, sdlColors, first_color, num_colors);
}

void Executor::vdriver_get_colors(int first_color, int num_colors, ColorSpec *colors)
{
    SDL_Color *sdlColors = sdlSurface->format->palette->colors;
    for(int i = 0; i < num_colors; i++)
    {
        SDL_Color &c = sdlColors[first_color + i];

        colors[i].value = CW(first_color + i);
        colors[i].rgb.red = CW(c.r << 8 | c.r);
        colors[i].rgb.green = CW(c.g << 8 | c.g);
        colors[i].rgb.blue = CW(c.b << 8 | c.b);
    }
}
int Executor::vdriver_update_screen_rects(int num_rects, const vdriver_rect_t *r,
                                          bool cursor_p)
{
    /*SDL_UpdateTexture(sdlTexture, NULL, vdriver_fbuf, vdriver_row_bytes);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);*/
    SDL_BlitSurface(sdlSurface, NULL, SDL_GetWindowSurface(sdlWindow), NULL);
    SDL_UpdateWindowSurface(sdlWindow);
}

int Executor::vdriver_update_screen(int top, int left, int bottom, int right,
                                    bool cursor_p)
{
    /*SDL_UpdateTexture(sdlTexture, NULL, vdriver_fbuf, vdriver_row_bytes);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);*/
    SDL_BlitSurface(sdlSurface, NULL, SDL_GetWindowSurface(sdlWindow), NULL);
    SDL_UpdateWindowSurface(sdlWindow);
}

void Executor::vdriver_flush_display(void)
{
}

void Executor::vdriver_shutdown(void)
{
}

void Executor::host_flush_shadow_screen(void)
{
}

static bool ConfirmQuit()
{
    const SDL_MessageBoxButtonData buttons[] = {
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "Cancel" },
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Force Quit" },
    };
    const SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_INFORMATION, /* .flags */
        sdlWindow, /* .window */
        "Quit", /* .title */
        "Do you want to quit Executor?", /* .message */
        SDL_arraysize(buttons), /* .numbuttons */
        buttons, /* .buttons */
        NULL /* .colorScheme */
    };
    int buttonid;
    SDL_ShowMessageBox(&messageboxdata, &buttonid);
    return buttonid == 1;
}

static bool isModifier(unsigned char virt, uint16_t *modstore)
{
    /* Note: shift and control can be cleared if right* and left* are pressed */
    switch(virt)
    {
        case MKV_LEFTSHIFT:
        case MKV_RIGHTSHIFT:
            *modstore = shiftKey;
            break;
        case MKV_CAPS:
            *modstore = alphaLock;
            break;
        case MKV_LEFTCNTL:
        case MKV_RIGHTCNTL:
            *modstore = ControlKey;
            break;
        case MKV_CLOVER:
            *modstore = cmdKey;
            break;
        case MKV_LEFTOPTION:
        case MKV_RIGHTOPTION:
            *modstore = optionKey;
            break;
        default:
            *modstore = 0;
            return false;
    }
    return true;
}



void Executor::vdriver_pump_events()
{
    SDL_Event event;
    static uint16_t keymod = 0;

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_MOUSEMOTION:
                MouseLocation.h = CW(event.motion.x);
                MouseLocation.v = CW(event.motion.y);

                adb_apeiron_hack(false);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                bool down_p;
                int32_t when;
                Point where;

                down_p = (event.button.state == SDL_PRESSED);
                if(down_p)
                    keymod &= ~btnState;
                else
                    keymod |= btnState;
                when = TickCount();
                where.h = event.button.x;
                where.v = event.button.y;
                ROMlib_PPostEvent(down_p ? mouseDown : mouseUp,
                                  0, (GUEST<EvQElPtr> *)0, when, where,
                                  keymod);
                adb_apeiron_hack(false);
            }
            break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                bool down_p;
                unsigned char mkvkey;
                uint16_t mod;
                LONGINT keywhat;
                int32_t when;
                Point where;

                init_sdlk_to_mkv();
                down_p = (event.key.state == SDL_PRESSED);

                /*if(use_scan_codes)
                    mkvkey = ibm_virt_to_mac_virt[event.key.keysym.scancode];
                else*/
                {
                    auto p = sdlk_to_mkv.find(event.key.keysym.sym);
                    if(p == sdlk_to_mkv.end())
                        mkvkey = NOTAKEY;
                    else
                        mkvkey = p->second;
                }
                mkvkey = ROMlib_right_to_left_key_map(mkvkey);
                if(isModifier(mkvkey, &mod))
                {
                    if(down_p)
                        keymod |= mod;
                    else
                        keymod &= ~mod;
                }
                when = TickCount();
                where.h = CW(MouseLocation.h);
                where.v = CW(MouseLocation.v);
                keywhat = ROMlib_xlate(mkvkey, keymod, down_p);
                post_keytrans_key_events(down_p ? keyDown : keyUp,
                                         keywhat, when, where,
                                         keymod, mkvkey);
            }
            break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                //if(!we_lost_clipboard())
                    sendresumeevent(false);
                //else
                //{
                //    ZeroScrap();
                //    sendresumeevent(true);
                //}
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                sendsuspendevent();
                break;
            case SDL_QUIT:
                if(ConfirmQuit())
                    ExitToShell();
                break;
        }
    }
}

void
Executor::ROMlib_SetTitle(char *title)
{
}

char *
Executor::ROMlib_GetTitle(void)
{
    static char str[] = "Foo";
    return str;
}

void
Executor::ROMlib_FreeTitle(char *title)
{
}

/* This is really inefficient.  We should hash the cursors */
void Executor::host_set_cursor(char *cursor_data,
                               unsigned short cursor_mask[16],
                               int hotspot_x, int hotspot_y)
{
    SDL_Cursor *old_cursor, *new_cursor;

    old_cursor = SDL_GetCursor();
    new_cursor = SDL_CreateCursor((unsigned char *)cursor_data,
                                  (unsigned char *)cursor_mask,
                                  16, 16, hotspot_x, hotspot_y);
    if(new_cursor != NULL)
    {
        SDL_SetCursor(new_cursor);
        SDL_FreeCursor(old_cursor);
    }
}

int Executor::host_set_cursor_visible(int show_p)
{
    return (SDL_ShowCursor(show_p));
}
