/*
 * box command...
 * box -delete file command ...
 *
 * Runs command... and if "-delete file" is specified, deletes the
 * file after the command has been run.  This is a Win32 program and
 * by using it from within a DOS-based application it's possible to
 * get the original DOS-based application to spawn a new box to run
 * the command in.  Specifically this comes in handy with Executor.
 */

#include <windows.h>
#include <nonansi/process.h>
#include <nonansi/dir.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* Change CRLF to ' ', and drop trailing space.  The code below is overkill,
   because when we read in "text" mode, CRLF gets converted to a single LF
   which means we could just change that LF to a space in place, but I didn't
   know that when I wrote the code. */

static void
change_crlf_to_space (char *p)
{
  char *ip, *op;
  int last_was_space;

  last_was_space = 0;
  for (ip = p, op = p; *ip; ++ip)
    {
      switch (*ip)
	{
	case '\r':
	case '\n':
	  if (!last_was_space)
	    {
	      *op++ = ' ';
	      last_was_space = 1;
	    }
	  break;
	default:
	  *op++ = *ip;
	  last_was_space = 0;
	  break;
	}
    }
  if (last_was_space)
    --op;
  *op++ = 0;
}

int PASCAL
WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
  char *p, *file_to_delete;
  int mess_value;
  FILE *message_fp;
  
  /*
   * Format of box.txt: first line is name we should give to the new window
   * succesive lines are coalesced into warning message that's presented to
   * user.
   */
  
  mess_value = IDCANCEL-1;
  message_fp = fopen ("box.txt", "r");
  if (message_fp)
    {
      char buf[1024];
      size_t nread;
      
      nread = fread (buf, 1, sizeof buf, message_fp);
      if (nread > 0)
	{
	  char *p;
	  
	  if (nread < sizeof buf)
	    buf[nread] = 0;
	  else
	    buf[sizeof buf - 1] = 0;
	  p = strchr (buf, '\n');
	  if (p)
	    {
	      *p = 0;
	      ++p;
	      while (*p && isspace(*p))
		++p;
	      change_crlf_to_space (p);
	      mess_value = MessageBox (NULL, p, buf,
				       MB_ICONINFORMATION|MB_OKCANCEL);
	    }
	}
    }
  for (p = szCmdLine; *p && isspace (*p); ++p)
    ;

#define DELETE_TOKEN "-delete"

  file_to_delete = 0;
  if (strncasecmp (p, DELETE_TOKEN, sizeof (DELETE_TOKEN)-1) == 0)
    {
      p += sizeof (DELETE_TOKEN)-1;
      if (!isspace (*p))
	p = szCmdLine;
      else
	{
	  while (*p && isspace (*p))
	    ++p;
	  if (*p)
	    {
	      file_to_delete = p;
	      while (*p && !isspace (*p))
		++p;
	      if (*p)
		{
		  *p = 0;
		  ++p;
		}
	    }
	}	
    }

  if (mess_value == IDCANCEL)
    {
     if (file_to_delete)
       DeleteFile (file_to_delete);
    }
  else
    {
      STARTUPINFO startinfo;
      PROCESS_INFORMATION procinfo;
      BOOL create_success;

      memset (&startinfo, 0, sizeof startinfo);
      startinfo.cb = sizeof startinfo;
      startinfo.dwFlags = STARTF_USESHOWWINDOW;
      startinfo.wShowWindow = SW_MINIMIZE;
      create_success = CreateProcess (
				      NULL,
				      p,
				      NULL,
				      NULL,
				      FALSE,
				      0, /* DETACHED_PROCESS */
				      NULL, /* environment */
				      NULL, /* start dir "c:\\tts\\gs", */
				      &startinfo,
				      &procinfo
				      );

      if (create_success)
	{
	  CloseHandle (procinfo.hThread);
	  WaitForSingleObject (procinfo.hProcess, INFINITE);
	  CloseHandle (procinfo.hProcess);
	  if (file_to_delete)
	    DeleteFile (file_to_delete);
	}
    }
  return 0;
}
