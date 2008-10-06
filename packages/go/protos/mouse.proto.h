
/* mouse.c */
void replaceselected (ControlHandle c);
void windfromicon (ControlHandle c);
void activateicon (ControlHandle c);
pascal void scrollicons (ControlHandle c, short part);
void offsetwindow (ControlHandle c, short offset);
pascal void windowscroller (ControlHandle c, short part);
void actoncontrol (ControlHandle c, short part, EventRecord * ev);
void checkcontrol (EventRecord * ev, WindowPtr wp);
void mouseinhotband (EventRecord * ev);
void domousedown (EventRecord * ev);
short dirwindowexists (Rect * r);
