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
PASCAL_FUNCTION(EnterMovies);
extern void C_ExitMovies(void);
PASCAL_FUNCTION(ExitMovies);
extern void C_MoviesTask(Movie movie, LONGINT maxmillisecs);
PASCAL_FUNCTION(MoviesTask);
extern OSErr C_PrerollMovie(Movie movie, TimeValue time,
                                        Fixed rate);
PASCAL_FUNCTION(PrerollMovie);
extern void C_SetMovieActive(Movie movie, BOOLEAN active);
PASCAL_FUNCTION(SetMovieActive);
extern void C_StartMovie(Movie movie);
PASCAL_FUNCTION(StartMovie);
extern void C_StopMovie(Movie movie);
PASCAL_FUNCTION(StopMovie);
extern void C_GoToBeginningOfMovie(Movie movie);
PASCAL_FUNCTION(GoToBeginningOfMovie);
extern void C_SetMovieGWorld(Movie movie, CGrafPtr cgrafp,
                                         GDHandle gdh);
PASCAL_FUNCTION(SetMovieGWorld);
extern OSErr C_UpdateMovie(Movie movie);
PASCAL_FUNCTION(UpdateMovie);
extern void C_DisposeMovie(Movie movie);
PASCAL_FUNCTION(DisposeMovie);
extern INTEGER C_GetMovieVolume(Movie movie);
PASCAL_FUNCTION(GetMovieVolume);
extern OSErr C_CloseMovieFile(INTEGER refnum);
PASCAL_FUNCTION(CloseMovieFile);
extern BOOLEAN C_IsMovieDone(Movie movie);
PASCAL_FUNCTION(IsMovieDone);
extern OSErr C_NewMovieFromFile(Movie *moviep, INTEGER refnum,
                                            INTEGER *residp,
                                            StringPtr resnamep,
                                            INTEGER flags,
                                            BOOLEAN *datarefwaschangedp);
PASCAL_FUNCTION(NewMovieFromFile);
extern Fixed C_GetMoviePreferredRate(Movie movie);
PASCAL_FUNCTION(GetMoviePreferredRate);
extern void C_GetMovieBox(Movie movie, Rect *boxp);
PASCAL_FUNCTION(GetMovieBox);
extern void C_SetMovieBox(Movie movie, const Rect *boxp);
PASCAL_FUNCTION(SetMovieBox);
extern ComponentInstance C_NewMovieController(Movie movie,
                                                          const Rect *mrectp,
                                                          LONGINT flags);
PASCAL_FUNCTION(NewMovieController);
extern void C_DisposeMovieController(ComponentInstance cntrller);
PASCAL_FUNCTION(DisposeMovieController);
extern OSErr C_OpenMovieFile(const FSSpec *filespecp,
                                         INTEGER *refnump, uint8 perm);
PASCAL_FUNCTION(OpenMovieFile);
}
#endif
