#if !defined(__QUICKTIME__)
#define __QUICKTIME__

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "Components.h"
#include "CQuickDraw.h"
#include "FileMgr.h"

namespace Executor
{
typedef struct MovieRecord
{
    LONGINT data[1];
} MovieRecord;

typedef MovieRecord *Movie;

typedef LONGINT TimeValue;

extern OSErr C_EnterMovies(void);
PASCAL_SUBTRAP(EnterMovies, 0xAAAA, QuickTime);
extern void C_ExitMovies(void);
PASCAL_SUBTRAP(ExitMovies, 0xAAAA, QuickTime);
extern void C_MoviesTask(Movie movie, LONGINT maxmillisecs);
PASCAL_SUBTRAP(MoviesTask, 0xAAAA, QuickTime);
extern OSErr C_PrerollMovie(Movie movie, TimeValue time,
                                        Fixed rate);
PASCAL_SUBTRAP(PrerollMovie, 0xAAAA, QuickTime);
extern void C_SetMovieActive(Movie movie, BOOLEAN active);
PASCAL_SUBTRAP(SetMovieActive, 0xAAAA, QuickTime);
extern void C_StartMovie(Movie movie);
PASCAL_SUBTRAP(StartMovie, 0xAAAA, QuickTime);
extern void C_StopMovie(Movie movie);
PASCAL_SUBTRAP(StopMovie, 0xAAAA, QuickTime);
extern void C_GoToBeginningOfMovie(Movie movie);
PASCAL_SUBTRAP(GoToBeginningOfMovie, 0xAAAA, QuickTime);
extern void C_SetMovieGWorld(Movie movie, CGrafPtr cgrafp,
                                         GDHandle gdh);
PASCAL_SUBTRAP(SetMovieGWorld, 0xAAAA, QuickTime);
extern OSErr C_UpdateMovie(Movie movie);
PASCAL_SUBTRAP(UpdateMovie, 0xAAAA, QuickTime);
extern void C_DisposeMovie(Movie movie);
PASCAL_SUBTRAP(DisposeMovie, 0xAAAA, QuickTime);
extern INTEGER C_GetMovieVolume(Movie movie);
PASCAL_SUBTRAP(GetMovieVolume, 0xAAAA, QuickTime);
extern OSErr C_CloseMovieFile(INTEGER refnum);
PASCAL_SUBTRAP(CloseMovieFile, 0xAAAA, QuickTime);
extern BOOLEAN C_IsMovieDone(Movie movie);
PASCAL_SUBTRAP(IsMovieDone, 0xAAAA, QuickTime);
extern OSErr C_NewMovieFromFile(Movie *moviep, INTEGER refnum,
                                            INTEGER *residp,
                                            StringPtr resnamep,
                                            INTEGER flags,
                                            BOOLEAN *datarefwaschangedp);
PASCAL_SUBTRAP(NewMovieFromFile, 0xAAAA, QuickTime);
extern Fixed C_GetMoviePreferredRate(Movie movie);
PASCAL_SUBTRAP(GetMoviePreferredRate, 0xAAAA, QuickTime);
extern void C_GetMovieBox(Movie movie, Rect *boxp);
PASCAL_SUBTRAP(GetMovieBox, 0xAAAA, QuickTime);
extern void C_SetMovieBox(Movie movie, const Rect *boxp);
PASCAL_SUBTRAP(SetMovieBox, 0xAAAA, QuickTime);
extern ComponentInstance C_NewMovieController(Movie movie,
                                                          const Rect *mrectp,
                                                          LONGINT flags);
PASCAL_SUBTRAP(NewMovieController, 0xAAAA, QuickTime);
extern void C_DisposeMovieController(ComponentInstance cntrller);
PASCAL_SUBTRAP(DisposeMovieController, 0xAAAA, QuickTime);
extern OSErr C_OpenMovieFile(const FSSpec *filespecp,
                                         INTEGER *refnump, uint8 perm);
PASCAL_SUBTRAP(OpenMovieFile, 0xAAAA, QuickTime);
}
#endif
