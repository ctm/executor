#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "win_print.h"

enum
{
  DEFAULT_PHYSX = (int) IN(8.5),
  DEFAULT_PHYSY = (int) IN(11),
  DEFAULT_ORIENTATION = WIN_PRINT_PORTRAIT,
  DEFAULT_COPIES = 1,
};

int PASCAL
WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
  int retval;
  win_printp_t wp;
  DWORD last_error;
  BOOL success;

  success = get_info (&wp, DEFAULT_PHYSX, DEFAULT_PHYSY, DEFAULT_ORIENTATION,
		      DEFAULT_COPIES, &last_error);
  if (success)
    success = print_file (wp, &last_error);

  retval = success ? 0 : 1;
  return retval;
}
