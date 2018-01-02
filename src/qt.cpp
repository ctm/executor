/* Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

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

PUBLIC pascal trap OSErr Executor::C_EnterMovies()
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

PUBLIC pascal trap void Executor::C_ExitMovies()
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

PUBLIC pascal trap void Executor::C_MoviesTask(Movie movie, LONGINT maxmillisecs)
{
    warning_unimplemented(NULL_STRING);
}

PUBLIC pascal trap OSErr Executor::C_PrerollMovie(Movie movie, TimeValue time, Fixed rate)
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

PUBLIC pascal trap void Executor::C_SetMovieActive(Movie movie, BOOLEAN active)
{
    warning_unimplemented(NULL_STRING);
}

PUBLIC pascal trap void Executor::C_StartMovie(Movie movie)
{
    warning_unimplemented(NULL_STRING);
}

PUBLIC pascal trap void Executor::C_StopMovie(Movie movie)
{
    warning_unimplemented(NULL_STRING);
}

PUBLIC pascal trap void Executor::C_GoToBeginningOfMovie(Movie movie)
{
    warning_unimplemented(NULL_STRING);
}

PUBLIC pascal trap void Executor::C_SetMovieGWorld(Movie movie, CGrafPtr cgrafp, GDHandle gdh)
{
    warning_unimplemented(NULL_STRING);
}

PUBLIC pascal trap OSErr Executor::C_UpdateMovie(Movie movie)
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

PUBLIC pascal trap void Executor::C_DisposeMovie(Movie movie)
{
    warning_unimplemented(NULL_STRING);
}

PUBLIC pascal trap INTEGER Executor::C_GetMovieVolume(Movie movie)
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

PUBLIC pascal trap OSErr Executor::C_CloseMovieFile(INTEGER refnum)
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

PUBLIC pascal trap BOOLEAN Executor::C_IsMovieDone(Movie movie)
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

PUBLIC pascal trap OSErr Executor::C_NewMovieFromFile(Movie * moviep, INTEGER refnum, INTEGER * residp, StringPtr resnamep, INTEGER flags, BOOLEAN * datarefwaschangedp)
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

PUBLIC pascal trap Fixed Executor::C_GetMoviePreferredRate(Movie movie)
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

PUBLIC pascal trap void Executor::C_GetMovieBox(Movie movie, Rect * boxp)
{
    warning_unimplemented(NULL_STRING);
}

PUBLIC pascal trap void Executor::C_SetMovieBox(Movie movie, const Rect * boxp)
{
    warning_unimplemented(NULL_STRING);
}

PUBLIC pascal trap ComponentInstance Executor::C_NewMovieController(Movie movie, const Rect * movierectp, LONGINT flags)
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

PUBLIC pascal trap void Executor::C_DisposeMovieController(ComponentInstance controller)
{
    warning_unimplemented(NULL_STRING);
}

PUBLIC pascal trap OSErr Executor::C_OpenMovieFile(const FSSpec * filespecp, INTEGER * refnump, uint8 perm)
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
