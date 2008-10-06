#if !defined(__XDEFS__)
#define __XDEFS__

/*
 * Copyright 1989 - 1993 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: Xdefs.h 63 2004-12-24 18:19:43Z ctm $
 */

#if defined (BINCOMPAT)
#include "rsys/pragmal.h"
#endif /* BINCOMPAT */

#define NeedFunctionPrototypes	1

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#define INTERESTINGEVENTS	(KeyPressMask      | \
				 KeyReleaseMask    | \
				 ButtonPressMask   | \
				 ButtonReleaseMask | \
				 EnterWindowMask | \
				 LeaveWindowMask | \
				 ExposureMask)

#if !defined(__alpha)
#if defined (BINCOMPAT)
#undef Bool
#define Bool LONGINT
#endif /* BINCOMPAT */

#if !defined (__STDC__)

extern int XAutoRepeatOff();
extern int XAutoRepeatOn();
extern int XBell();
extern int XChangeProperty();
extern int XChangeWindowAttributes();
extern Bool XCheckTypedEvent();
extern Bool XCheckWindowEvent();
extern int XClearWindow();
extern int XConvertSelection();
extern int XDefineCursor();
extern int XFree();
extern int XFreeCursor();
extern int XFreeGC();
extern int XFreePixmap();
extern int XGeometry();
extern Status XGetGeometry();
extern Window XGetSelectionOwner();
extern int XGetWindowProperty();
extern Atom XInternAtom();
extern int XMapWindow();
extern int XNextEvent();
extern int XPutImage();
extern Bool XQueryPointer();
extern int XSelectInput();
extern Status XSendEvent();
extern int XSetSelectionOwner();
extern int XSetStandardProperties();
extern int XSetWMHints();
extern int XSync();
extern int XWarpPointer();

#else /* __STDC__ */

#if !defined (NEXT)

extern LONGINT XAutoRepeatOff( Display *dp );

extern LONGINT XAutoRepeatOn( Display *dp );

extern LONGINT XBell( Display *dp, LONGINT percent );

extern LONGINT XChangeProperty( Display *dp, Window w, Atom property, Atom type,
		 LONGINT format, LONGINT mode, unsigned char *data, LONGINT nelements );

extern LONGINT XChangeWindowAttributes( Display *dp, Window w,
		      ULONGINT valuemask, XSetWindowAttributes *attribp );

extern Bool XCheckTypedEvent( Display *dp, LONGINT eventtype, XEvent *evp );

extern Bool XCheckWindowEvent( Display *dp, Window w, LONGINT evtmask,
								 XEvent *evp );

extern LONGINT XClearWindow( Display *dp, Window w );

extern LONGINT XConvertSelection( Display *dp, Atom selection, Atom target,
				      Atom prop, Window requestor, Time time );

extern GC XCreateGC( Display *disp, Drawable d, ULONGINT val,
							     XGCValues *vals );

extern XImage *XCreateImage( Display *disp, Visual *vis, ULONGINT depth,
		    LONGINT format, LONGINT offset, char *data, ULONGINT width,
			   ULONGINT height, LONGINT pad, LONGINT bytesperline );

extern Pixmap XCreatePixmap( Display *dp, Drawable d, ULONGINT width,
				    ULONGINT height, ULONGINT depth);

extern Cursor XCreatePixmapCursor( Display *disp, Pixmap source, Pixmap mask,
	  XColor *fgcolor, XColor *bgcolor, ULONGINT x, ULONGINT y );

extern Window XCreateSimpleWindow( Display *disp, Window parent, LONGINT x,
		LONGINT y, ULONGINT width, ULONGINT height,
		ULONGINT borderwidth, ULONGINT border,
		ULONGINT background);

extern LONGINT XDefineCursor( Display *dp, Window w, Cursor cursor );

extern char *XDisplayName( char *string );

extern LONGINT XFlush( Display *disp );

extern LONGINT XFree( char *data );

extern LONGINT XFreeCursor( Display *dp, Cursor cursor );

extern LONGINT XFreeGC( Display *dp, GC gc );

extern LONGINT XFreePixmap( Display *dp, Pixmap pixmap );

extern LONGINT XGeometry( Display *dp, LONGINT screen, char *pos, char *defpos,
		      ULONGINT bwidth, ULONGINT fwidth,
		      ULONGINT fheight, LONGINT xaddr, LONGINT yaddr,
		      LONGINT *xret, LONGINT *yret, LONGINT *widret, LONGINT *heightret );

extern char *XGetDefault( Display *dp, char *prog, char *option );

extern Status XGetGeometry( Display *dp, Drawable d, Window *rootret,
			    LONGINT *xret, LONGINT *yret, ULONGINT *widthret,
			    ULONGINT *heightret, ULONGINT *borderret,
						     ULONGINT *depthret );

extern Window XGetSelectionOwner( Display* dp, Atom a );

extern LONGINT XGetWindowProperty( Display *dp, Window w, Atom prop, LONGINT offset,
		       LONGINT length, Bool delete, Atom req_type, Atom *act_type,
		LONGINT *act_fmt, ULONGINT *nitems, ULONGINT *bytesleft,
							unsigned char **data );

extern Atom XInternAtom( Display *dp, char *name, Bool only_if_exists );

extern KeySym XLookupKeysym( XKeyEvent *keyevt, LONGINT index );

extern LONGINT XMapWindow( Display *dp, Window w );

extern LONGINT XNextEvent( Display *dp, XEvent *evtp );

extern Display *XOpenDisplay( char *dispname );

extern LONGINT XPutImage( Display *dp, Drawable d, GC gc, XImage *imagep,
		      LONGINT srcx, LONGINT srcy, LONGINT destx, LONGINT desty,
				  ULONGINT width, ULONGINT height );

extern Bool XQueryPointer( Display *dp, Window w, Window *rootret,
			   Window *childret, LONGINT *rootxret, LONGINT *rootyret,
		       LONGINT *winxret, LONGINT *winyret, ULONGINT *maskret );

extern LONGINT XSelectInput( Display *dp, Window w, LONGINT eventmask );

extern Status XSendEvent( Display *dp, Window w, Bool propogate, LONGINT evtmask,
							       XEvent *event );

extern LONGINT XSetSelectionOwner( Display *dp, Atom a, Window w,
							   ULONGINT time);

extern LONGINT XSetStandardProperties( Display *dp, Window w, char *wname,
				   char *iconname, Pixmap iconpix, char **argv,
					        LONGINT argc, XSizeHints *hints );

extern LONGINT XSetWMHints( Display *dp, Window w, XWMHints *wmhintp );

extern LONGINT XSync( Display *dp, Bool discard );

extern LONGINT XWarpPointer( Display *dp, Window srcw, Window dstw, LONGINT srcx,
			 LONGINT srcy, ULONGINT srcwidth,
			 ULONGINT srcheight, LONGINT destx, LONGINT desty );

#endif /* NEXT */

#endif /* __STDC__ */

#endif /* !defined(__alpha) */

#if defined (BINCOMPAT)
#include "rsys/pragmas.h"
#endif /* BINCOMPAT */
#endif
