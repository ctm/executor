/* Copyright 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_sounddriver[] = "$Id: sounddriver.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/sounddriver.h"
#include "rsys/soundfake.h"

#ifdef Sound_MACOSX_
#include "SoundOSX.h"
#endif

#ifdef Sound_SDL_Sound
#include "sdl-sound.h"
#endif

namespace Executor
{

/* This is the current sound driver. */
class SoundDriver *sound_driver;

int ROMlib_SND_RATE = 22255;

SoundDriver::~SoundDriver()
{
}

void sound_init(void)
{
    bool found_one_p = false;

    /* Try all available sound drivers and keep the first one that works. */
    do
    {
#ifdef Sound_MACOSX_
        {
            SoundOSX *sndOSX = new SoundOSX;
            if(!sndOSX->sound_init())
            {
                delete sndOSX;
            }
            else
            {
                found_one_p = true;
                sound_driver = static_cast<class SoundDriver *>(sndOSX);
                break;
            }
        }
#endif

#ifdef Sound_SDL_Sound
        {
            SDLSound *sndSDL = new SDLSound;
            if(!sndSDL->sound_init())
            {
                delete sndSDL;
            }
            else
            {
                found_one_p = true;
                sound_driver = static_cast<class SoundDriver *>(sndSDL);
                break;
            }
        }
#endif

        {
            SoundFake *sndFake = new SoundFake;
            if(!sndFake->sound_init())
            {
                delete sndFake;
            }
            else
            {
                found_one_p = true;
                sound_driver = static_cast<class SoundDriver *>(sndFake);
                break;
            }
        }

    } while(0);

    /* At least the fake sound driver must always match! */
    gui_assert(found_one_p);
}
}
