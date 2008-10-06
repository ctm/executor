#if !defined(_SDL_BMP_H_)
#define _SDL_BMP_H_

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: SDL_bmp.h 63 2004-12-24 18:19:43Z ctm $
 */

extern int SDL_SaveCF_DIB (SDL_Surface *surfp, char **dib_bytesp,
			   size_t *dib_lenp);

extern SDL_Surface * SDL_LoadCF_DIB (void *mem);

#endif
