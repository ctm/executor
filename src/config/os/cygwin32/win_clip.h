#if !defined(_win_clip_h_)
#define _win_clip_h_

/*
 * Copyright 1999 by Abacus Research and Development, Inc.
 * All rights reserved.
 */

#if defined(SDL)
extern void write_pict_as_dib_to_clipboard(void);
extern void write_surfp_to_clipboard(SDL_Surface *surfp);
extern unsigned long ROMlib_executor_format(LONGINT type);
extern void write_pict_as_dib_to_clipboard(void);
extern void write_pict_as_pict_to_clipboard(void);
#endif

#endif
