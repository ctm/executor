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

#define MODULE_NAME QuickTime
#include <rsys/api-module.h>

namespace Executor
{
typedef struct MovieRecord
{
    LONGINT data[1];
} MovieRecord;

typedef MovieRecord *Movie;

typedef LONGINT TimeValue;

DISPATCHER_TRAP(QuickTime, 0xAAAA, D0W);

extern OSErr C_EnterMovies(void);
PASCAL_SUBTRAP(EnterMovies, 0xAAAA, 0x0001, QuickTime);
extern void C_ExitMovies(void);
PASCAL_SUBTRAP(ExitMovies, 0xAAAA, 0x0002, QuickTime);
extern void C_MoviesTask(Movie movie, LONGINT maxmillisecs);
PASCAL_SUBTRAP(MoviesTask, 0xAAAA, 0x0005, QuickTime);
extern OSErr C_PrerollMovie(Movie movie, TimeValue time,
                                        Fixed rate);
PASCAL_SUBTRAP(PrerollMovie, 0xAAAA, 0x0006, QuickTime);
extern void C_SetMovieActive(Movie movie, BOOLEAN active);
PASCAL_SUBTRAP(SetMovieActive, 0xAAAA, 0x0009, QuickTime);
extern void C_StartMovie(Movie movie);
PASCAL_SUBTRAP(StartMovie, 0xAAAA, 0x000B, QuickTime);
extern void C_StopMovie(Movie movie);
PASCAL_SUBTRAP(StopMovie, 0xAAAA, 0x000C, QuickTime);
extern void C_GoToBeginningOfMovie(Movie movie);
PASCAL_SUBTRAP(GoToBeginningOfMovie, 0xAAAA, 0x000D, QuickTime);
extern void C_SetMovieGWorld(Movie movie, CGrafPtr cgrafp,
                                         GDHandle gdh);
PASCAL_SUBTRAP(SetMovieGWorld, 0xAAAA, 0x0016, QuickTime);
extern OSErr C_UpdateMovie(Movie movie);
PASCAL_SUBTRAP(UpdateMovie, 0xAAAA, 0x001F, QuickTime);
extern void C_DisposeMovie(Movie movie);
PASCAL_SUBTRAP(DisposeMovie, 0xAAAA, 0x0023, QuickTime);
extern INTEGER C_GetMovieVolume(Movie movie);
PASCAL_SUBTRAP(GetMovieVolume, 0xAAAA, 0x002E, QuickTime);
extern OSErr C_CloseMovieFile(INTEGER refnum);
PASCAL_SUBTRAP(CloseMovieFile, 0xAAAA, 0x00D5, QuickTime);
extern BOOLEAN C_IsMovieDone(Movie movie);
PASCAL_SUBTRAP(IsMovieDone, 0xAAAA, 0x00DD, QuickTime);
extern OSErr C_NewMovieFromFile(Movie *moviep, INTEGER refnum,
                                            INTEGER *residp,
                                            StringPtr resnamep,
                                            INTEGER flags,
                                            BOOLEAN *datarefwaschangedp);
PASCAL_SUBTRAP(NewMovieFromFile, 0xAAAA, 0x00F0, QuickTime);
extern Fixed C_GetMoviePreferredRate(Movie movie);
PASCAL_SUBTRAP(GetMoviePreferredRate, 0xAAAA, 0x00F3, QuickTime);
extern void C_GetMovieBox(Movie movie, Rect *boxp);
PASCAL_SUBTRAP(GetMovieBox, 0xAAAA, 0x00F9, QuickTime);
extern void C_SetMovieBox(Movie movie, const Rect *boxp);
PASCAL_SUBTRAP(SetMovieBox, 0xAAAA, 0x00FA, QuickTime);
extern ComponentInstance C_NewMovieController(Movie movie,
                                                          const Rect *mrectp,
                                                          LONGINT flags);
PASCAL_SUBTRAP(NewMovieController, 0xAAAA, 0x018A, QuickTime);
extern void C_DisposeMovieController(ComponentInstance cntrller);
PASCAL_SUBTRAP(DisposeMovieController, 0xAAAA, 0x018B, QuickTime);
extern OSErr C_OpenMovieFile(const FSSpec *filespecp,
                                         INTEGER *refnump, uint8_t perm);
PASCAL_SUBTRAP(OpenMovieFile, 0xAAAA, 0x0192, QuickTime);
}
#endif
