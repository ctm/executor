
/* misc.c */
void help (void);
void getinfo (void);
void screensaver (void);
void setband (short whichband);
void mystr255copy (Str255 dest, Str255 src);
void mystrncpy (char *dest, char *src, short len);
void straightenwindow (WindowPtr wp);
void savestate (void);
pascal void helpscroll (ControlHandle c, short part);
pascal void drawhelp (DialogPtr dp, short item);
pascal char helpfilter (DialogPtr dp, EventRecord * ev, short *item);
