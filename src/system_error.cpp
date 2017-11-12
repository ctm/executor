/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_system_error[] =
	"$Id: system_error.c 88 2005-05-25 03:59:37Z ctm $";
#endif

#include "rsys/common.h"

#include "WindowMgr.h"
#include "ControlMgr.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "TextEdit.h"
#include "EventMgr.h"
#include "ToolboxEvent.h"
#include "OSEvent.h"

#include "rsys/system_error.h"
#include "rsys/redrawscreen.h"

#include "rsys/cquick.h"
#include "rsys/osevent.h"
#include "rsys/options.h"

using namespace Executor;

#define N_BUTTONS	(3)

/* sanity defines */

#define _NewCWindow(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)	\
  ((WindowPtr) NewCWindow (arg0, arg1, arg2, arg3, arg4,		\
			   (CWindowPtr) (arg5), arg6, arg7))

#define _FindControl(arg0, arg1, arg2)		\
  ({						\
    int16 retval;				\
    GUEST<ControlHandle> bogo_c;		\
						\
    retval = FindControl (arg0, arg1, &bogo_c);	\
    *(arg2) = MR (bogo_c);			\
						\
    retval;					\
  })

#pragma pack(push, 2)

struct button
{
  ControlHandle ctl;
  const char *text;
  system_error_callback_t func;

  bool hilite_p;
  Rect hilite_rect;
  int oval;
  
  int index;
};

static struct button buttons[3] = {};

static WindowPtr msg_window = nullptr;

static const char *message;

static int default_button;

static Rect message_rect;
#pragma pack(pop)

static struct button *
event_loop (void)
{
  EventRecord evt;
  Point where;

  for (;;)
    {
      GetNextEvent ((  mDownMask | updateMask
		     | keyDownMask | autoKeyMask), &evt);
      where = evt.where.get();
      
      switch (CW (evt.what))
	{
	case mouseDown:
	  {
	    Point local_pt;
	    bool control_p;
	    ControlHandle c;
            
            GUEST<Point> tmpPt = evt.where;
	    GlobalToLocal (&tmpPt);
	    local_pt = tmpPt.get();
	    
	    control_p = _FindControl (local_pt, msg_window, &c);
	    if (control_p)
	      {
		int release_part;
		
		release_part = TrackControl (c, local_pt, (ProcPtr) -1);
		
		if (release_part == inButton)
		  {
		    int i;
		    for (i = 0; i < N_BUTTONS; i ++)
		      if (c == buttons[i].ctl)
			return &buttons[i];
		  }
	      }
	  }
	  
	case updateEvt:
	  {
	    int i;
	    
	    BeginUpdate (msg_window);
	    
	    DrawControls (msg_window);
	    
	    PenSize (3, 3);
	    for (i = 0; i < N_BUTTONS; i ++)
	      if (buttons[i].hilite_p)
		{
		  if (!(ROMlib_options & ROMLIB_RECT_SCREEN_BIT))
		    FrameRoundRect (&buttons[i].hilite_rect,
				    buttons[i].oval,
				    buttons[i].oval);
		  else
		    FrameRect (&buttons[i].hilite_rect);
		}
	    
	    /* ### `TextBox ()' should be const */
	    TextBox ((Ptr) message, strlen (message), &message_rect,
		     teFlushLeft);
	    
	    EndUpdate (msg_window);
	    break;
	  }
	  
	case keyDown:
	case autoKey:
	  {
	    char ch;

	    ch = CL (evt.message) & 0xFF;
	    if (ch == '\r' || ch == NUMPAD_ENTER)
	      {
		int i;

		for (i = 0; i < N_BUTTONS; i ++)
		  if (buttons[i].hilite_p)
		    return &buttons[i];
	      }
	    break;
	  }
	}
    }
}

static int
int_sqrt (int x)
{
  int i;
  gui_assert (x >= 0);
  for (i = 0; i * i <= x; i++)
    ;
  return i - 1;
}

int
Executor::system_error (const char *_message, int _default_button,
	      const char *button0, const char *button1, const char *button2,
	      system_error_callback_t func0,
	      system_error_callback_t func1,
	      system_error_callback_t func2)
{
  static Rect dummy_rect;
  struct button *retval;
  
  message        = _message;
  default_button = _default_button;
  
  buttons[0].text  = button0;
  buttons[0].func  = func0;
  buttons[0].index = 0;
  buttons[1].text  = button1;
  buttons[1].func  = func1;
  buttons[1].index = 1;
  buttons[2].text  = button2;
  buttons[2].func  = func2;
  buttons[2].index = 2;

#if 1
  /* NOTE: when system_error() is called from InitWindows() the colors are
     messed up.  I don't know what the proper solution is, but this works
     for now. */

  redraw_screen ();

  /* NOTE: initializing the cursor here is real rude, and makes it so that
     we really can't call system_error at random times (but then again, we
     never really could).  But since system_error is primarily used to 
     print messages from InitWindows(), it's important that we somehow
     manage to get the cursor visible, since people are given the option of
     exiting but with no way to click on the button. */

  InitCursor ();
#endif

  /* allocate the message window with a bogus size; the text settings
     for this window will determine it's eventual size, at which point
     it will be resized and made visible */
  
  msg_window = _NewCWindow (NULL, &dummy_rect,
			    /* no title */
			    NULL,
			    /* invisible */
			    false,
			    dBoxProc,
			    (WindowPtr) -1,
			    false, /* dummy */ -1);
  
  THEPORT_SAVE_EXCURSION
    (msg_window,
     {
       FontInfo font_info;
       int total_message_width;
       int text_height;
       int line_count;
       int message_width;
       int message_height;
       int button_span;
       int width;
       int height;
       int max_button_width;
       int i;
       
       /* compute the geometry of the various dialog components so we
	  can determine the dialog's size */
       total_message_width = TextWidth ((Ptr) &message[0],
					0, strlen (message));
       
       GetFontInfo (&font_info);
       text_height = (  CW (font_info.ascent)
		      + CW (font_info.descent)
		      + CW (font_info.leading));
       
       line_count
	 /* must be at least one line of text, otherwise we'll get hit
            with a div by zero below */
	 = MAX (int_sqrt (total_message_width / (4 * text_height)), 1);
       
       max_button_width = -1;
       for (i = 0; i < N_BUTTONS; i ++)
	 if (buttons[i].text != NULL)
	   max_button_width = MAX (max_button_width,
				   /* ### `TextWidth ()' should be const */
				   TextWidth ((Ptr) buttons[i].text, 0,
					      strlen (buttons[i].text)));
       
       /* padding for button outline, etc */
       max_button_width += 16;
       
       message_width = total_message_width / line_count;
       message_height = (line_count + 2) * text_height;
       
       button_span = (max_button_width + 13) * N_BUTTONS - 13;
       if (message_width < button_span)
	 message_width = button_span;
       
       width = message_width + 20;
       height = message_height + 20 + 20 + 13;
       
       SizeWindow (msg_window, width, height, false);
       
       /* compute top/left based on screen dimensions */
       {
	 Rect *gd_rect;
	 int gd_width;
	 int gd_height;
	 int top;
	 int left;
	 
	 gd_rect   = &GD_RECT (MR (MainDevice));
	 gd_width  = RECT_WIDTH (gd_rect);
	 gd_height = RECT_HEIGHT (gd_rect);
	 
	 /* centered horizontally, with a third of the space above the
	    window, and two-thirds below */
	 top  = CW (gd_rect->top)  + (gd_height - height) / 3;
	 left = CW (gd_rect->left) + (gd_width - width) / 2;
	 
	 MoveWindow (msg_window, left, top, true);
       }
       
       ShowWindow (msg_window);
       
       message_rect.top    = CWC (10);
       message_rect.left   = CWC (10);
       message_rect.bottom = CW (message_height + 10);
       message_rect.right  = CW (message_width + 10);
       
       for (i = 0; i < N_BUTTONS; i ++)
	 if (buttons[i].text != NULL)
	   {
	     Rect ctl_rect;
	     unsigned char buf[256];
	     
	     ctl_rect.top    = CW (height - 10 - 20);
	     ctl_rect.bottom = CW (height - 10);
	     
	     ctl_rect.right  = CW (  width - 10 - i * (max_button_width + 13));
	     ctl_rect.left   = CW (  width - 10 - i * (max_button_width + 13)
				   - max_button_width);
	     
	     *buf = (unsigned char) strlen (buttons[i].text);
	     strcpy ((char *) &buf[1], buttons[i].text);
	     
	     buttons[i].ctl = NewControl (msg_window, &ctl_rect,
					  /* ### `NewControl ()'
                                             should be const */
					  buf,
					  /* visible */
					  true,
					  0, 0, 1, pushButProc,
					  /* dummy */ -1);
	     if (i == default_button)
	       {
		 buttons[i].hilite_rect = ctl_rect;
		 InsetRect (&buttons[i].hilite_rect, -4, -4);
		 buttons[i].oval
		   = RECT_HEIGHT (&buttons[i].hilite_rect) / 2 - 4;
		 buttons[i].hilite_p = true;
	       }
	     else
	       buttons[i].hilite_p = false;
	   }
       
       retval = event_loop ();
     });
  
  DisposeWindow (msg_window);
  
  if (retval->func)
    (*retval->func) ();
  
  return retval->index;
}
