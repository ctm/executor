/*
    WINMAIN.C, placed in the public domain by Sam Lantinga  4/13/98

    The WinMain function -- calls your program's main() function
*/

#ifdef _WIN32
#if !defined (WIN32)
#define WIN32
#endif
/* The WinMain function -- calls your program's main() function 

*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <windows.h>

#include "paramline.h"

#include "SDL/SDL.h"

/* This is where execution begins */
int STDCALL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
	char *appname;
	char *ptr;
	int  i;
	FILE *fp;
	char **argv;
	int argc;

	/*
	 * Direct Draw has a nasty bug where a file that is opened before
	 * Direct Draw is invoked, *stays* open until the system is rebooted.
	 * So we need to get Direct Draw loaded before we open any files.
	 */
	
	{
	  HANDLE h;

	  h = LoadLibrary ("DDRAW.DLL");
	  if (h)
	    FreeLibrary (LoadLibrary ("DDRAW.DLL"));
	}

	/* FIXME:
	 * fprintf needs to be remapped to a windows function, otherwise when 
	 * executor dies the user has no idea why it just vanished.
	 */
	fp = freopen ("stdout.txt", "w", stdout);
#if !defined (stdout)
	if (!fp)
		stdout = fopen ("stdout.txt", "w");
#else
	if (!fp)
		*stdout = *fopen ("stdout.txt", "w");
#endif
	setbuf (stdout, 0);
	fp = freopen ("stderr.txt", "w", stderr);
#if !defined (stderr)
	if (!fp)
		stderr = fopen ("stderr.txt", "w");
#else
	if (!fp)
		*stderr = *fopen ("stderr.txt", "w");
#endif
	setbuf (stderr, 0);

	paramline_to_argcv (GetCommandLine (), &argc, &argv);

	/* Get the class name from argv[0] */
	/* Basename... */
	if ( (ptr=strrchr(argv[0], '\\')) == NULL )
		appname = argv[0];
	else
		appname = ptr+1;
	/* minus extension... */
	if ( (ptr=strrchr(appname, '.')) == NULL )
		i = strlen(appname);
	else
		i = (ptr-appname);
	/* equals appname! */
	ptr=(char *)alloca(i+1);
	strncpy(ptr, appname, i);
	ptr[i] = '\0';
	appname = ptr;
	
	/* Load SDL dynamic link library */
	if ( SDL_Init(0) < 0 ) {
		fprintf(stderr, "WinMain() error: %s\n", SDL_GetError());
		return(false);
	}
	atexit(SDL_Quit);

	/* Create and register our class, then run main code */
	if ( SDL_RegisterApp(appname, CS_BYTEALIGNCLIENT, hInst) < 0 ) {
		fprintf(stderr, "WinMain() error: %s\n", SDL_GetError());
		return(false);
	}
	SDL_main(argc, argv);

	exit(0);
}

#endif /* _WIN32 */
