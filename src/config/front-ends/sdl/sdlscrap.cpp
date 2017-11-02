/* 
 * Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 *
 * Derived from public domain source code written by Sam Lantinga
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_sdlscrap[] = "$Id: sdlscrap.c 88 2005-05-25 03:59:37Z ctm $";
#endif

/* Handle clipboard text and data in arbitrary formats */

#include <stdio.h>
#include <limits.h>

#include "syswm_vars.h"
#include "sdl_mem.h"

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"

#include "rsys/error.h"

namespace Executor{
        // ### FIXME: clean up headers.
extern void PutScrapX( OSType type, LONGINT length, char *p, int scrap_cnt );
extern LONGINT GetScrapX( OSType type, Handle h );
}

using namespace Executor;


#if defined (CYGWIN32)

#include "SDL_bmp.h"


#warning TODO: copy pixels properly and handle various bit depths

#define advance_n_bytes(ptrp, n_bytes)				\
({								\
  typeof (ptrp) _ptrp;						\
								\
  _ptrp = (ptrp);						\
  *(_ptrp) = (typeof (*_ptrp))((char *)*(_ptrp) + n_bytes);	\
})

#define roundup(val, n)				\
({						\
  typeof (n) _n;				\
  _n = (n);                   			\
  ((val) + (_n-1)) / _n * _n;			\
})

#if 1

PRIVATE int
SDL_bpp (const SDL_Surface *surfp)
{
  int retval;

  retval = surfp->format->BitsPerPixel;
  return retval;
}

PRIVATE int
SDL_pixels_per_line (const SDL_Surface *surfp)
{
  int retval;

  retval = surfp->w;
  return retval;
}

PRIVATE int
SDL_n_lines (const SDL_Surface *surfp)
{
  int retval;

  retval = surfp->h;
  return retval;
}

PUBLIC SDL_Surface *
surface_from_dib (void *lp)
{
  SDL_Surface *retval;

  retval = SDL_LoadCF_DIB (lp);
  if (retval)
    {
      int first_bpp;

      first_bpp = SDL_bpp (retval);

      if (first_bpp < 8)
	{
	  /* TODO: convert < 8bpp to 8bpp */
	}
      else if (first_bpp > 8)
	{
	  /* convert > 8bpp to our particular 32bpp */
	  SDL_Surface *new_surface;
	  int pixels_per_line;
	  int n_lines;
	  enum { A = 0x00000000,
		 R = 0x0000FF00, G = 0x00FF0000, B = 0xFF000000 };

	  pixels_per_line = SDL_pixels_per_line (retval);
	  n_lines = SDL_n_lines (retval);
	  new_surface = SDL_AllocSurface (SDL_SWSURFACE, pixels_per_line,
					  n_lines, 32, R, G, B, A);
	  if (!new_surface ||
#if SDL_MAJOR_VERSION == 0 && SDL_MINOR_VERSION < 9
	      SDL_MapSurface (retval, new_surface->format) != 0 || 
#endif
	      SDL_BlitSurface (retval, NULL, new_surface, NULL) != 0)
	    {
	      if (new_surface)
		{
		  SDL_FreeSurface (new_surface);
		  new_surface = NULL;
		}
	    }
	  SDL_FreeSurface (retval);
	  retval = new_surface;
	}
    }

  return retval;
}
#else
PUBLIC SDL_Surface *
surface_from_dib (void *lp)
{
  BITMAPINFOHEADER *bp;
  SDL_Surface *retval;

  retval = NULL;
  bp = lp;
  switch (bp->biBitCount)
    {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
    case 32:
    default:
      /* TODO */
      break;
    case 24:
      {
	enum { A = 0xFF000000,
	       R = 0x00FF0000,
	       G = 0x0000FF00,
	       B = 0x000000FF };

	typedef struct
	{
	  uint8 zero PACKED;
	  uint8 red PACKED;
	  uint8 green PACKED;
	  uint8 blue PACKED;
	}
	sdl_pixel;

	typedef struct
	{
	  uint8 blue PACKED;
	  uint8 red PACKED;
	  uint8 green PACKED;
	}
	dib_pixel;
	int pixels_per_line;
	bool inverted_p;
	int n_lines;
	sdl_pixel *op, *eop;
	dib_pixel *ip, *eip;
	int in_pitch;
	int out_pitch;
	int ip_advance;
	int op_advance;

	pixels_per_line = bp->biWidth;
	inverted_p = (bp->biHeight > 0);
	n_lines = inverted_p ? bp->biHeight : -bp->biHeight;
	retval = SDL_AllocSurface (SDL_SWSURFACE, pixels_per_line,
				   n_lines, 32,
				   R, G, B, A);
	SDL_LockSurface (retval);
	op = SDL_Surface_pixels (retval);
	ip = (typeof (ip)) (bp + 1);
	out_pitch = SDL_Surface_pitch (retval);
	in_pitch = roundup (pixels_per_line * sizeof *ip, 4);
	ip_advance = in_pitch  - sizeof *ip * pixels_per_line;
	op_advance = out_pitch - sizeof *op * pixels_per_line;

	if (inverted_p)
	  {
	    advance_n_bytes (&ip, in_pitch * (n_lines - 1));
	    ip_advance -= 2 * in_pitch;
	  }
	eip = ip;
	advance_n_bytes (&eip,
			 n_lines *
			 (ip_advance + pixels_per_line * sizeof *ip));
	for (; ip != eip; advance_n_bytes (&ip, ip_advance),
	                  advance_n_bytes (&op, op_advance))
	  for (eop = op + pixels_per_line; op != eop; ++ip, ++op)
	    {
	      op->zero  = 0;
	      op->red   = ip->red;
	      op->green = ip->green;
	      op->blue  = ip->blue;
	    }
	
	SDL_UnlockSurface (retval);
	break;
      }
    }
  return retval;
}
#endif
#endif

#if defined(SDL) && defined (LINUX) /* DON'T USE THIS CODE FOR CYGWIN32! */

#include "sdlscrap.h"

/* Determine what type of clipboard we are using */
#if defined(__unix__)
#define X11_SCRAP
#elif defined(_WIN32)
#define WIN_SCRAP
#else
#error Unknown window manager for clipboard handling
#endif /* scrap type */

/* System dependent data types */
#if defined(X11_SCRAP)
/* * */
typedef Atom scrap_type;

#elif defined(WIN_SCRAP)
/* * */
typedef UINT scrap_type;

#endif /* scrap type */


#define FORMAT_PREFIX	"SDL_scrap_0x"

PRIVATE scrap_type
convert_format(int type)
{
  switch (type)
    {

    case T('T', 'E', 'X', 'T'):
#if defined(X11_SCRAP)
/* * */
      return XA_STRING;

#elif defined(WIN_SCRAP)
/* * */
      return CF_TEXT;

#endif /* scrap type */

    default:
      {
        char format[sizeof(FORMAT_PREFIX)+8+1];

        sprintf(format, "%s%08lx", FORMAT_PREFIX, (unsigned long)type);

#if defined(X11_SCRAP)
/* * */
        return XInternAtom(SDL_Display, format, False);

#elif defined(WIN_SCRAP)
/* * */
        return RegisterClipboardFormat(format);

#endif /* scrap type */
      }
    }
}

/* Convert internal data to scrap format */
PRIVATE int
convert_data(int type, char *dst, char *src, int srclen)
{
  int dstlen;

  dstlen = 0;
  switch (type)
    {
    case T('T', 'E', 'X', 'T'):
      if ( dst )
        {
          while ( --srclen >= 0 )
            {
#if defined(__unix__)
              if ( *src == '\r' )
                {
                  *dst++ = '\n';
                  ++dstlen;
                }
              else
#elif defined(_WIN32)
              if ( *src == '\r' )
                {
                  *dst++ = '\r';
                  ++dstlen;
                  *dst++ = '\n';
                  ++dstlen;
                }
              else
#endif
                {
                  *dst++ = *src;
                  ++dstlen;
                }
              ++src;
            }
            *dst = '\0';
        }
      else
        {
          while ( --srclen >= 0 )
            {
#if defined(__unix__)
              if ( *src == '\r' )
                {
                  ++dstlen;
                }
              else
#elif defined(_WIN32)
              if ( *src == '\r' )
                {
                  ++dstlen;
                  ++dstlen;
                }
              else
#endif
                {
                  ++dstlen;
                }
              ++src;
            }
            ++dstlen;
        }
      break;

    default:
      if ( dst )
        {
          *(int *)dst = srclen;
          dst += sizeof(int);
          memcpy(dst, src, srclen);
        }
      dstlen = sizeof(int)+srclen;
      break;
    }
    return(dstlen);
}

/* Convert scrap data to internal format */
PRIVATE int
convert_scrap(int type, char *dst, char *src, int srclen)
{
  int dstlen;

  dstlen = 0;
  switch (type)
    {
    case T('T', 'E', 'X', 'T'):
      {
        if ( srclen == 0 )
          srclen = strlen(src);
        if ( dst )
          {
            while ( --srclen >= 0 )
              {
#if defined(_WIN32)
                if ( *src == '\r' )
                  /* drop extraneous '\r' */;
                else
#endif
                if ( *src == '\n' )
                  {
                    *dst++ = '\r';
                    ++dstlen;
                  }
                else
                  {
                    *dst++ = *src;
                    ++dstlen;
                  }
                ++src;
              }
          }
        else
          {
            while ( --srclen >= 0 )
              {
#if defined(_WIN32)
                if ( *src == '\r' )
                  /* drop extraneous '\r' */;
                else
#endif
                  ++dstlen;
                ++src;
              }
          }
        }
      break;

    default:
      dstlen = *(int *)src;
      if ( dst )
        {
          if ( srclen == 0 )
            memcpy(dst, src+sizeof(int), dstlen);
          else
            memcpy(dst, src+sizeof(int), srclen-sizeof(int));
        }
      break;
    }
  return dstlen;
}

PUBLIC bool
we_lost_clipboard(void)
{
#if defined(X11_SCRAP)
/* * */
	return ( XGetSelectionOwner(SDL_Display, XA_PRIMARY) != SDL_Window );

#elif defined(WIN_SCRAP)
/* * */
	return ( GetClipboardOwner() != SDL_Window );

#endif /* scrap type */
}

PUBLIC void
put_scrap(int type, int srclen, char *src)
{
  scrap_type format;
  int dstlen;
  char *dst;

  format = convert_format(type);
  dstlen = convert_data(type, NULL, src, srclen);

#if defined(X11_SCRAP)
/* * */
  dst = (char *)alloca(dstlen);
  if ( dst != NULL )
    {
      convert_data(type, dst, src, srclen);
      XChangeProperty(SDL_Display, DefaultRootWindow(SDL_Display),
		      XA_CUT_BUFFER0, format, 8, PropModeReplace,
		      (unsigned char *) dst, dstlen);
      if ( we_lost_clipboard() )
        XSetSelectionOwner(SDL_Display, XA_PRIMARY, SDL_Window, CurrentTime);
    }

#elif defined(WIN_SCRAP)
/* * */
  if ( OpenClipboard(SDL_Window) )
    {
      HANDLE hMem;

      hMem = GlobalAlloc((GMEM_MOVEABLE|GMEM_DDESHARE), dstlen);
      if ( hMem != NULL )
        {
          dst = (char *)GlobalLock(hMem);
          convert_data(type, dst, src, srclen);
          GlobalUnlock(hMem);
          EmptyClipboard();
          SetClipboardData(format, hMem);
        }
      CloseClipboard();
    }

#endif /* scrap type */
}

PUBLIC void
get_scrap(int type, int *dstlen, Handle dst)
{
  scrap_type format;

  *dstlen = -1;
  format = convert_format(type);

#if defined(X11_SCRAP)
/* * */
  {
    Window owner;
    Atom selection;
    Atom seln_type;
    int seln_format;
    unsigned long nbytes;
    unsigned long overflow;
    char *src;

    owner = XGetSelectionOwner(SDL_Display, XA_PRIMARY);
    if ( (owner == None) || (owner == SDL_Window) )
      {
        owner = DefaultRootWindow(SDL_Display);
        selection = XA_CUT_BUFFER0;
      }
    else
      {
        int selection_response = 0;
        SDL_Event event;

        owner = SDL_Window;
        selection = XInternAtom(SDL_Display, "SDL_SELECTION", False);
        XConvertSelection(SDL_Display, XA_PRIMARY, format,
                                        selection, owner, CurrentTime);
        while ( ! selection_response )
          {
            SDL_WaitEvent(&event);
            if ( event.type == SDL_SYSWMEVENT )
              {
#if SDL_MAJOR_VERSION == 0 && SDL_MINOR_VERSION < 9
                XEvent xevent = event.syswm.msg->xevent;
#else
                XEvent xevent = event.syswm.msg->event.xevent;
#endif
                if ( (xevent.type == SelectionNotify) &&
                     (xevent.xselection.requestor == owner) )
                    selection_response = 1;
              }
            else
              {
                /* FIXME: dropped event? */;
              }
          }
      }
    if ( XGetWindowProperty(SDL_Display, owner, selection, 0, INT_MAX/4,
                            False, format, &seln_type, &seln_format,
                       &nbytes, &overflow, (unsigned char **)&src) == Success )
      {
        if ( seln_type == format )
          {
            char *mem;
            *dstlen = convert_scrap(type, NULL, src, nbytes);
            mem = sdl_ReallocHandle(dst, *dstlen);
            if ( mem == NULL )
              *dstlen = -1;
            else
              convert_scrap(type, mem, src, nbytes);
          }
        XFree(src);
      }
    }

#elif defined(WIN_SCRAP)
/* * */
  if ( IsClipboardFormatAvailable(format) && OpenClipboard(SDL_Window) )
    {
      HANDLE hMem;
      char *src;

      hMem = GetClipboardData(format);
      if ( hMem != NULL )
        {
          char *mem;
          src = (char *)GlobalLock(hMem);
          *dstlen = convert_scrap(type, NULL, src, 0);
          mem = sdl_ReallocHandle(dst, *dstlen);
          if ( mem == NULL )
            *dstlen = -1;
          else
            convert_scrap(type, mem, src, 0);
          GlobalUnlock(hMem);
        }
      CloseClipboard();
    }

#endif /* scrap type */
}

PUBLIC void export_scrap(const SDL_Event *event)
{
#if defined(X11_SCRAP)
/* * */
  XSelectionRequestEvent *req;
  XEvent sevent;
  int seln_format;
  unsigned long nbytes;
  unsigned long overflow;
  unsigned char *seln_data;

#if SDL_MAJOR_VERSION == 0 && SDL_MINOR_VERSION < 9
  req = &event->syswm.msg->xevent.xselectionrequest;
#else
  req = &event->syswm.msg->event.xevent.xselectionrequest;
#endif
  sevent.xselection.type = SelectionNotify;
  sevent.xselection.display = req->display;
  sevent.xselection.selection = req->selection;
  sevent.xselection.target = None;
  sevent.xselection.property = None;
  sevent.xselection.requestor = req->requestor;
  sevent.xselection.time = req->time;
  if ( XGetWindowProperty(SDL_Display, DefaultRootWindow(SDL_Display),
                          XA_CUT_BUFFER0, 0, INT_MAX/4, False, req->target,
                          &sevent.xselection.target, &seln_format,
                          &nbytes, &overflow, &seln_data) == Success )
    {
      if ( sevent.xselection.target == req->target )
        {
          if ( sevent.xselection.target == XA_STRING )
            {
              if ( seln_data[nbytes-1] == '\0' )
                --nbytes;
            }
          XChangeProperty(SDL_Display, req->requestor, req->property,
            sevent.xselection.target, seln_format, PropModeReplace,
                                                  seln_data, nbytes);
          sevent.xselection.property = req->property;
        }
      XFree(seln_data);
    }
  XSendEvent(SDL_Display,req->requestor,False,0,&sevent);
  XSync(SDL_Display, False);

#endif /* X11_SCRAP */
}

/* For Executor compatibility */
LONGINT Executor::GetScrapX(LONGINT type, Executor::Handle h)
{
  int scraplen;

  get_scrap(type, &scraplen, h);
  return(scraplen);
}
void Executor::PutScrapX(LONGINT type, LONGINT length, char *p, int scrap_count)
{
  put_scrap(type, length, p);
}

#endif

#if defined (MACOSX)
#warning "Need to support clipboard"

PUBLIC bool
we_lost_clipboard(void)
{
  return false; /* TODO */
}

LONGINT Executor::GetScrapX(LONGINT type, char **h)
{
  return -1;  /* TODO */
}
void Executor::PutScrapX(LONGINT type, LONGINT length, char *p, int scrap_count)
{
  /* TODO */
}

#endif
