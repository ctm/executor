/* Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qt[] = "$Id: qt.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "QuickTime.h"

using namespace Executor;

#if defined(CYGWIN32)

#define LIB_LOOKUP(lib, proc)                           \
    do                                                  \
    {                                                   \
        proc = (void *)GetProcAddress((lib), #proc);    \
        if(!x)                                          \
        {                                               \
            warning_unexpected("Couldn't find " #proc); \
            goto error;                                 \
        }                                               \
    } while(0)

#if 0
PRIVATE void
qt_init (void)
{
  HINSTANCE qtlib;

  qtlib = LoadLibrary ("???");
  LIB_LOOKUP (qtlib, EnterMovies);
  LIB_LOOKUP (qtlib, ExitMovies);
  LIB_LOOKUP (qtlib, MoviesTask);
  LIB_LOOKUP (qtlib, PrerollMovie);
  LIB_LOOKUP (qtlib, SetMovieActive);
  LIB_LOOKUP (qtlib, StartMovie);
  LIB_LOOKUP (qtlib, StopMovie);
  LIB_LOOKUP (qtlib, GoToBeginningOfMovie);
  LIB_LOOKUP (qtlib, SetMovieGWorld);
  LIB_LOOKUP (qtlib, UpdateMovie);
  LIB_LOOKUP (qtlib, DisposeMovie);
  LIB_LOOKUP (qtlib, GetMovieVolume);
  LIB_LOOKUP (qtlib, CloseMovieFile);
  LIB_LOOKUP (qtlib, IsMovieDone);
  LIB_LOOKUP (qtlib, NewMovieFromFile);
  LIB_LOOKUP (qtlib, GetMoviePreferredRate);
  LIB_LOOKUP (qtlib, GetMovieBox);
  LIB_LOOKUP (qtlib, SetMovieBox);
  LIB_LOOKUP (qtlib, NewMovieController);
  LIB_LOOKUP (qtlib, DisposeMovieController);
  LIB_LOOKUP (qtlib, OpenMovieFile);
}
#endif

#endif

P0(PUBLIC pascal trap, OSErr, EnterMovies)
{
    OSErr retval;

#if !defined(CYGWIN32)
    warning_unimplemented(NULL_STRING);
    retval = noErr;
#else
#if 0
  retval = EnterMovies ();
#else
    retval = 0;
#endif
#endif
    return retval;
}

P0(PUBLIC pascal trap, void, ExitMovies)
{
#if !defined(CYGWIN32)
    warning_unimplemented(NULL_STRING);
#else
#if 0
  ExitMovies ();
#else
#endif
#endif
}

P2(PUBLIC pascal trap, void, MoviesTask, Movie, movie, LONGINT, maxmillisecs)
{
    warning_unimplemented(NULL_STRING);
}

P3(PUBLIC pascal trap, OSErr, PrerollMovie,
   Movie, movie,
   TimeValue, time,
   Fixed, rate)
{
    OSErr retval;

#if !defined(CYGWIN32)
    warning_unimplemented(NULL_STRING);
    retval = noErr;
#else
    retval = 0;
#endif
    return retval;
}

P2(PUBLIC pascal trap, void, SetMovieActive, Movie, movie, BOOLEAN, active)
{
    warning_unimplemented(NULL_STRING);
}

P1(PUBLIC pascal trap, void, StartMovie, Movie, movie)
{
    warning_unimplemented(NULL_STRING);
}

P1(PUBLIC pascal trap, void, StopMovie, Movie, movie)
{
    warning_unimplemented(NULL_STRING);
}

P1(PUBLIC pascal trap, void, GoToBeginningOfMovie, Movie, movie)
{
    warning_unimplemented(NULL_STRING);
}

P3(PUBLIC pascal trap, void, SetMovieGWorld, Movie, movie, CGrafPtr, cgrafp,
   GDHandle, gdh)
{
    warning_unimplemented(NULL_STRING);
}

P1(PUBLIC pascal trap, OSErr, UpdateMovie, Movie, movie)
{
    OSErr retval;

#if !defined(CYGWIN32)
    warning_unimplemented(NULL_STRING);
    retval = noErr;
#else
    retval = 0;
#endif
    return retval;
}

P1(PUBLIC pascal trap, void, DisposeMovie, Movie, movie)
{
    warning_unimplemented(NULL_STRING);
}

P1(PUBLIC pascal trap, INTEGER, GetMovieVolume, Movie, movie)
{
    INTEGER retval;

#if !defined(CYGWIN32)
    warning_unimplemented(NULL_STRING);
    retval = 1;
#else
    retval = 0;
#endif
    return retval;
}

P1(PUBLIC pascal trap, OSErr, CloseMovieFile, INTEGER, refnum)
{
    OSErr retval;

#if !defined(CYGWIN32)
    warning_unimplemented(NULL_STRING);
    retval = noErr;
#else
    retval = 0;
#endif
    return retval;
}

P1(PUBLIC pascal trap, BOOLEAN, IsMovieDone, Movie, movie)
{
    BOOLEAN retval;

#if !defined(CYGWIN32)
    warning_unimplemented(NULL_STRING);
    retval = true;
#else
    retval = 0;
#endif
    return retval;
}

P6(PUBLIC pascal trap, OSErr, NewMovieFromFile, Movie *, moviep,
   INTEGER, refnum, INTEGER *, residp, StringPtr, resnamep, INTEGER, flags,
   BOOLEAN *, datarefwaschangedp)
{
    OSErr retval;

#if !defined(CYGWIN32)
    warning_unimplemented(NULL_STRING);
    retval = noErr;
#else
    retval = 0;
#endif
    return retval;
}

P1(PUBLIC pascal trap, Fixed, GetMoviePreferredRate, Movie, movie)
{
    Fixed retval;

#if !defined(CYGWIN32)
    warning_unimplemented(NULL_STRING);
    retval = 0;
#else
    retval = 0;
#endif
    return retval;
}

P2(PUBLIC pascal trap, void, GetMovieBox, Movie, movie, Rect *, boxp)
{
    warning_unimplemented(NULL_STRING);
}

P2(PUBLIC pascal trap, void, SetMovieBox, Movie, movie, const Rect *, boxp)
{
    warning_unimplemented(NULL_STRING);
}

P3(PUBLIC pascal trap, ComponentInstance, NewMovieController, Movie, movie,
   const Rect *, movierectp, LONGINT, flags)
{
    ComponentInstance retval;

#if !defined(CYGWIN32)
    warning_unimplemented(NULL_STRING);
    retval = 0;
#else
    retval = 0;
#endif
    return retval;
}

P1(PUBLIC pascal trap, void, DisposeMovieController,
   ComponentInstance, controller)
{
    warning_unimplemented(NULL_STRING);
}

P3(PUBLIC pascal trap, OSErr, OpenMovieFile, const FSSpec *, filespecp,
   INTEGER *, refnump, uint8, perm)
{
    OSErr retval;

#if !defined(CYGWIN32)
    warning_unimplemented(NULL_STRING);
    retval = fnfErr;
#else
    retval = 0;
#endif
    return retval;
}
