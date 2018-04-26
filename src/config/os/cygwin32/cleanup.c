#include <windows.h>
#include "paramline.h"

/*
 * Rename file specified by argv[1] to name specified by argv[2].
 * keep hammering until we can do it.
 *
 * This program is a helper for the Executor registration process.
 * Executor makes a copy of itself with some bytes changed, but can't
 * actually rename itself because it's still open, so starts up this
 * program and then exits.
 */

int STDCALL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
    int retval;
    int argc;
    char **argv;

    paramline_to_argcv(GetCommandLine(), &argc, &argv);

    if(!SetFileAttributes(argv[2], FILE_ATTRIBUTE_NORMAL))
        retval = 1;
    else
    {
        while(!DeleteFile(argv[2]))
            ;
        retval = MoveFile(argv[1], argv[2]) ? 0 : 1;
    }

    return retval;
}
