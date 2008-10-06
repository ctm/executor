/* Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_linux_except[] = "$Id: linux_except.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/system_error.h"

#include "SegmentLdr.h"

#include "linux_except.h"

#include <stdio.h>
#include <signal.h>

static void
my_fault_proc( int sig )
{
  // FIXME:  Change this to an internal Executor dialog
  fprintf(stderr, "Unexpected Application Failure\n");

  // If we are already in the browser, does this exit the program?
  C_ExitToShell ();
}

static int except_list[] = { SIGSEGV, SIGBUS, 0 };

void
install_exception_handler (void)
{
  int i;

  for ( i=0; except_list[i]; ++i )
    {
      signal(except_list[i], my_fault_proc);
    }
}

void
uninstall_exception_handler (void)
{
  int i;

  for ( i=0; except_list[i]; ++i )
    {
      signal(except_list[i], SIG_DFL);
    }
}
