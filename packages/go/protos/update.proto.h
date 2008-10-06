
/* update.c */
void drawpartialgrowicon (WindowPtr wp, int update_flag);
void doactivate (EventRecord * ev);
void doupdate (EventRecord * ev);
void changewindow (Str255 s, long dirid, short vrefnum,
		   void (*f) (WindowPeek, Str255, long, short));
void changehot (ControlHandle c, long todir, short tovol);
