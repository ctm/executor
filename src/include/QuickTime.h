#if !defined (__QUICKTIME__)
#define __QUICKTIME__

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: QuickTime.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "Components.h"
#include "CQuickDraw.h"
#include "FileMgr.h"

typedef struct MovieRecord
{
  LONGINT data[1];
}
MovieRecord;

typedef MovieRecord *Movie;

typedef LONGINT TimeValue;

extern pascal trap OSErr C_EnterMovies (void);
extern pascal trap void C_ExitMovies (void);
extern pascal trap void C_MoviesTask (Movie movie, LONGINT maxmillisecs);
extern pascal trap OSErr C_PrerollMovie (Movie movie, TimeValue time,
					 Fixed rate);
extern pascal trap void C_SetMovieActive (Movie movie, BOOLEAN active);
extern pascal trap void C_StartMovie (Movie movie);
extern pascal trap void C_StopMovie (Movie movie);
extern pascal trap void C_GoToBeginningOfMovie (Movie movie);
extern pascal trap void C_SetMovieGWorld (Movie movie, CGrafPtr cgrafp,
					  GDHandle gdh);
extern pascal trap OSErr C_UpdateMovie (Movie movie);
extern pascal trap void C_DisposeMovie (Movie movie);
extern pascal trap INTEGER C_GetMovieVolume (Movie movie);
extern pascal trap OSErr C_CloseMovieFile (INTEGER refnum);
extern pascal trap BOOLEAN C_IsMovieDone (Movie movie);
extern pascal trap OSErr C_NewMovieFromFile (Movie *moviep, INTEGER refnum,
					     INTEGER *residp,
					     StringPtr resnamep,
					     INTEGER flags,
					     BOOLEAN *datarefwaschangedp);
extern pascal trap Fixed C_GetMoviePreferredRate (Movie movie);
extern pascal trap void C_GetMovieBox (Movie movie, Rect *boxp);
extern pascal trap void C_SetMovieBox (Movie movie, const Rect *boxp);
extern pascal trap ComponentInstance C_NewMovieController (Movie movie,
							   const Rect *mrectp,
							   LONGINT flags);
extern pascal trap void C_DisposeMovieController (ComponentInstance cntrller);
extern pascal trap OSErr C_OpenMovieFile (const FSSpec *filespecp,
					  INTEGER *refnump, uint8 perm);
#endif
