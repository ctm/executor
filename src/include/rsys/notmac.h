#if !defined (__NOTMAC__)
#define __NOTMAC__
/*
 * Copyright 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: notmac.h 63 2004-12-24 18:19:43Z ctm $
 */

#if defined (MSDOS) || defined (CYGWIN32)
extern char ROMlib_start_drive;
#endif

#if !defined (USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)

#include "rsys/commonevt.h"
namespace Executor {
extern char ROMlib_startdir[];

extern char *ROMlib_ConfigurationFolder;
extern char *ROMlib_SystemFolder;
extern char *ROMlib_PublicDirectoryMap;
extern char *ROMlib_PrivateDirectoryMap;
extern char *ROMlib_DefaultFolder;
extern char *ROMlib_ExcelApp;
extern char *ROMlib_WordApp;
extern char *ROMlib_appname;
extern char *ROMlib_ScreenDumpFile;

#if !defined (__STDC__)
extern BOOLEAN ROMlib_shouldalarm();
#else /* __STDC__ */
extern BOOLEAN ROMlib_shouldalarm( void );
#endif /* __STDC__ */

/* DO NOT DELETE THIS LINE */
#if !defined (__STDC__)
extern void initX();
extern void putimageX();
extern void warppointerX();
extern void showcursorX();
extern void setcursorX();
extern void querypointerX();
extern BOOLEAN checkwindoweventX();
extern void autorepeatonX();
extern LONGINT lookupkeysymX();
extern void bellX();
extern void PutScrapX();
extern LONGINT GetScrapX();
#else /* __STDC__ */
extern void initX( LONGINT argc, char **argv, INTEGER *rowbytep, INTEGER *bottomp, INTEGER *rightp, char **addrp );
extern void putimageX( LONGINT top, LONGINT left, LONGINT bottom, LONGINT right );
extern void warppointerX( INTEGER dh, INTEGER dv );
extern void showcursorX( LONGINT show );
extern void setcursorX( INTEGER *data, INTEGER *mask, LONGINT hotx, LONGINT hoty );
extern void querypointerX( LONGINT *xp, LONGINT *yp, LONGINT *modp );
extern BOOLEAN checkwindoweventX( char **eventp, commonevent *commonp );
extern void autorepeatonX( void );
extern LONGINT lookupkeysymX( char *p );
extern void bellX( void );
extern void PutScrapX( OSType type, LONGINT length, char *p, int scrap_cnt );
extern LONGINT GetScrapX( OSType type, char **h );
#endif /* __STDC__ */
}
#endif

#endif /* __NOTMAC__ */
