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

extern pascal trap OSErr C_EnterMovies(void);
PASCAL_FUNCTION(EnterMovies);
extern pascal trap void C_ExitMovies(void);
PASCAL_FUNCTION(ExitMovies);
extern pascal trap void C_MoviesTask(Movie movie, LONGINT maxmillisecs);
PASCAL_FUNCTION(MoviesTask);
extern pascal trap OSErr C_PrerollMovie(Movie movie, TimeValue time,
                                        Fixed rate);
PASCAL_FUNCTION(PrerollMovie);
extern pascal trap void C_SetMovieActive(Movie movie, BOOLEAN active);
PASCAL_FUNCTION(SetMovieActive);
extern pascal trap void C_StartMovie(Movie movie);
PASCAL_FUNCTION(StartMovie);
extern pascal trap void C_StopMovie(Movie movie);
PASCAL_FUNCTION(StopMovie);
extern pascal trap void C_GoToBeginningOfMovie(Movie movie);
PASCAL_FUNCTION(GoToBeginningOfMovie);
extern pascal trap void C_SetMovieGWorld(Movie movie, CGrafPtr cgrafp,
                                         GDHandle gdh);
PASCAL_FUNCTION(SetMovieGWorld);
extern pascal trap OSErr C_UpdateMovie(Movie movie);
PASCAL_FUNCTION(UpdateMovie);
extern pascal trap void C_DisposeMovie(Movie movie);
PASCAL_FUNCTION(DisposeMovie);
extern pascal trap INTEGER C_GetMovieVolume(Movie movie);
PASCAL_FUNCTION(GetMovieVolume);
extern pascal trap OSErr C_CloseMovieFile(INTEGER refnum);
PASCAL_FUNCTION(CloseMovieFile);
extern pascal trap BOOLEAN C_IsMovieDone(Movie movie);
PASCAL_FUNCTION(IsMovieDone);
extern pascal trap OSErr C_NewMovieFromFile(Movie *moviep, INTEGER refnum,
                                            INTEGER *residp,
                                            StringPtr resnamep,
                                            INTEGER flags,
                                            BOOLEAN *datarefwaschangedp);
PASCAL_FUNCTION(NewMovieFromFile);
extern pascal trap Fixed C_GetMoviePreferredRate(Movie movie);
PASCAL_FUNCTION(GetMoviePreferredRate);
extern pascal trap void C_GetMovieBox(Movie movie, Rect *boxp);
PASCAL_FUNCTION(GetMovieBox);
extern pascal trap void C_SetMovieBox(Movie movie, const Rect *boxp);
PASCAL_FUNCTION(SetMovieBox);
extern pascal trap ComponentInstance C_NewMovieController(Movie movie,
                                                          const Rect *mrectp,
                                                          LONGINT flags);
PASCAL_FUNCTION(NewMovieController);
extern pascal trap void C_DisposeMovieController(ComponentInstance cntrller);
PASCAL_FUNCTION(DisposeMovieController);
extern pascal trap OSErr C_OpenMovieFile(const FSSpec *filespecp,
                                         INTEGER *refnump, uint8 perm);
PASCAL_FUNCTION(OpenMovieFile);
}
#endif
