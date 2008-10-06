/* Copyright 1994, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_winevents[] = "$Id: winevents.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include <windows.h>
#include "windriver.h"

#include "OSEvent.h"
#include "ToolboxEvent.h"
#include "SegmentLdr.h"
#include "rsys/adb.h"
#include "rsys/segment.h"
#include "rsys/osevent.h"
#include "rsys/keyboard.h"
#include "rsys/param.h"

#include "vk_to_mkv.h"

#define APPNAME "executor"

/* Globals for the windows display subsystem */
const char *Win_AppName = APPNAME;
HINSTANCE   Win_Instance;


/* * *
	The main chimichunga. :)
 * * */
void process_win32_events(void)
{
  MSG msg;

  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      if (msg.message == WM_QUIT)
	{
	  ROMlib_exit = 1;
	  ExitToShell ();
	}
      else
	{
	  DispatchMessage(&msg);
	}
    }
}

static unsigned char w32_clover_p = 0;    /* Set by the WM_SYS* messages */
static unsigned char w32_mousedown_p = 0; /* Set by WM_LBUTTON* messages */
static int16 Keyboard_State(void)
{
  int16 keystate;

  keystate = 0;

  /* Check for <SHIFT> down */
  keystate |= ( (GetKeyState(VK_SHIFT)>>1) ? shiftKey : 0 );
  /* Check for <CAPSLOCK> set */
  keystate |= ( (GetKeyState(VK_CAPITAL)&0x01) ? alphaLock : 0 );
  /* Check for <CTRL> down */
  keystate |= ( (GetKeyState(VK_CONTROL)>>1) ? ControlKey : 0 );
  /* ALT is used by the system.  Check for VK_MENU and use it for
     the clover (command) key.  Also account for WM_SYS* messages. */
  keystate |= ( (w32_clover_p || (GetKeyState(VK_MENU)>>1)) ? cmdKey : 0 );
  /* Check the current mouse state */
  if ( !w32_mousedown_p )
    {
      keystate |= btnState;
    }

  return keystate;
}

static int w32_modifier_p(unsigned char virt, int16 *modstore)
{
  *modstore = 0;
  switch (virt)
    {
    case VK_LSHIFT:
    case VK_RSHIFT:
    case VK_SHIFT:
      *modstore = shiftKey;
      break;
    case VK_CAPITAL:
      *modstore = alphaLock;
      break;
    case VK_LCONTROL:
    case VK_RCONTROL:
    case VK_CONTROL:
      *modstore = ControlKey;
      break;
    case VK_MENU:
      *modstore = cmdKey;
      break;
    default:
      return 0;
    }
  return 1;
}
 
LONG CALLBACK AppWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  Point where;
  int32 when;

  switch (msg)
    {
      /* A WM_ACTIVATE message occurs when our window just before losing
       * focus, or just after gaining focus.  Which is determined by the
       * low word of wParam.
       */
    case WM_ACTIVATE:
      {
	int active;
	active = LOWORD(wParam);
	Win_Focus(active);
	return 0;
      }

    /* A WM_PALETTECHANGED message occurs when the system palette has
     * changed, and allows us to redraw our window in the new colors.
     */
    case WM_PALETTECHANGED:
      {
	HWND thewin;
	thewin = (HWND)wParam;
	if ( thewin != hwnd )
	  Win_NewPal();
	break;
      }

      /* A WM_SIZE message occurs when the window is mapped, during the 
       * ShowWindow() in Win_Init().  This means that this code will be
       * called before a palette has been realized, and so the first
       * draw call will consist of drawing with a black palette.
       */
    case WM_SIZE:
      switch (wParam)
	{
	case SIZE_MINIMIZED:
	  /* Ignore minimizations */;
	break;
	case SIZE_MAXIMIZED:
#if 0
	  /* Go to fullscreen */
	  Win_FullScreen(1);
	  /* Redraw screen contents here */
#endif
	  break;
	default:
	  /* We ignore size changes here, because we get this message when
	   * we are mapped.
	   * Instead, we lock the size by catching WM_WINDOWPOSCHANGING
	   */
	  break;
	}
      break;
  
    /* This message is sent as a way for us to "check" the values of a 
     * position change.  If we don't like it, we can adjust the values 
     * before they are changed.
     */
    case WM_GETMINMAXINFO:
      {
	MINMAXINFO *info;
	RECT        size;
	int x, y;
	int width;
	int height;

	/* Get the current position of our window */
	GetWindowRect(hwnd, &size);
	x = size.left;
	y = size.top;

	/* Calculate the current width and height of our window */
	size.top = 0;
	size.bottom = vdriver_height;
	size.left = 0;
	size.right = vdriver_width;
	AdjustWindowRect(&size, GetWindowLong(hwnd, GWL_STYLE), FALSE);
	width = size.right - size.left;
	height = size.bottom - size.top;

	/* Fix our size to the current size */
	info = (MINMAXINFO *)lParam;
	info->ptMaxSize.x = width;
	info->ptMaxSize.y = height;
	info->ptMaxPosition.x = x;
	info->ptMaxPosition.y = y;
	info->ptMinTrackSize.x = width;
	info->ptMinTrackSize.y = height;
	info->ptMaxTrackSize.x = width;
	info->ptMaxTrackSize.y = height;

	return(0);
      }

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      {
	int16 button_state;
	when = TickCount ();
	where.h = LOWORD (lParam);
	where.v = HIWORD (lParam);
	w32_mousedown_p = (msg == WM_LBUTTONDOWN);
	button_state = Keyboard_State ();
	ROMlib_PPostEvent(w32_mousedown_p ? mouseDown : mouseUp,
			  0, (HIDDEN_EvQElPtr *) 0,
			  when, where, button_state);
	adb_apeiron_hack (FALSE);
	break;
      }

    case WM_MOUSEMOVE:
      MouseLocation.h = CW (LOWORD (lParam));
      MouseLocation.v = CW (HIWORD (lParam));
      adb_apeiron_hack (FALSE);
      break;

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
      /* Handle any ALT-<KEY> combinations */
      /* ... */
      /* Fall through, KEYDOWN detects the ALT key */
      
    case WM_KEYDOWN:
    case WM_KEYUP:
      {
	unsigned char down_p;
	LONGINT keywhat;
	unsigned char virt;
	uint16 whichmod;
	uint16 button_state;

	/* Check for autorepeat (bit 30 specifies a repeated key) */
	down_p = ((msg == WM_KEYDOWN) || (msg == WM_SYSKEYDOWN));
	if ( down_p && ((lParam>>30)&0x01) )
	    return 0;

	/* Check the keysym, do we recognise it? */
	virt = vk_to_mkv[wParam];
	if ( virt != NOTAKEY )
	  {
	    /* Check for ALT key - should SYS_clover be persistent? */
	    if ( ((msg == WM_SYSKEYDOWN) || (msg == WM_SYSKEYUP)) &&
		 ((lParam>>29)&0x01) )
	      {
		w32_clover_p = 1;
	      }
	    else
	      {
		w32_clover_p = 0;
	      }
	    when = TickCount ();
	    where.h = CW(MouseLocation.h);
	    where.v = CW(MouseLocation.v);
	    button_state = Keyboard_State ();
	    if ( w32_modifier_p(virt, &whichmod) )
	      button_state &= ~whichmod;
	    keywhat = ROMlib_xlate(virt, button_state, down_p);
	    post_keytrans_key_events(down_p ? keyDown : keyUp,
				     keywhat, when, where,
				     button_state, virt);
	  }
	/* We handled the message, so return. */
	return 0;
      }

    case WM_CLOSE:
      {
	int reply;

	reply = MessageBox(hwnd, "Terminate running Macintosh application?",
			   "-= Executor =-", MB_OKCANCEL);
	if ( reply == IDOK )
	  PostQuitMessage(0);
	return 0;
      }

    case WM_PAINT:
      /* Redraw a portion of the screen */
      Win_PAINT();
      return 0;
    }
  return DefWindowProc(hwnd,msg,wParam,lParam);
}

#ifndef SOUND_GGA
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
  int    argc, i;
  char **argv;
  FILE *fp;

  /* FIXME:
   * fprintf needs to be remapped to a windows function, otherwise when 
   * executor dies the user has no idea why it just vanished.  Also, I'm
   * running this from a read-only SMB mount. :-)
   */
  fp = freopen ("stdout.txt", "w", stdout);
  if (!fp)
    stdout = fopen ("stdout.txt", "w");
  setbuf (stdout, 0);
  fp = freopen ("stderr.txt", "w", stderr);
  if (!fp)
    stderr = fopen ("stderr.txt", "w");
  setbuf (stderr, 0);

  if (!hPrev) {
    /* Register a class for the main application window */
    WNDCLASS cls;
    cls.hCursor        = LoadCursor(NULL, IDC_ARROW);
    cls.hIcon          = LoadIcon(hInst, "(none)");
    cls.lpszMenuName   = "(none)";
    cls.lpszClassName  = APPNAME;
    cls.hbrBackground  = NULL;
    cls.hInstance      = hInst;
    cls.style          = CS_BYTEALIGNCLIENT;
    cls.lpfnWndProc    = (LPVOID)AppWndProc;
    cls.cbWndExtra     = 0;
    cls.cbClsExtra     = 0;
    if (!RegisterClass(&cls))
      return FALSE;
  }

  /* Initialize the global windowing variables */
  Win_Instance = hInst;

  {
    char *cmdp;
    int len;

    cmdp = GetCommandLine ();
    len = strlen (cmdp);
    argc = count_params (cmdp, len);
    argv = malloc((argc+1) * sizeof *argv);
    for (i = 0; i < argc; ++i)
      argv[i] = get_param (&cmdp, &len);
    argv[i] = 0;
  }

  return(ROMlib_main(argc, argv));
}
#endif
