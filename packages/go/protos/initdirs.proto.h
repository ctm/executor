
/* initdirs.c */
ControlHandle addtolist (WindowPeek wp, Str255 s, long dirid, short vrefnum);
void removefromlist (WindowPeek wp, Str255 s, long dirid, short vrefnum);
void initopendirs (FILE * f);
void updatemove (WindowPeek wp, Str255 s, long dirid, short vrefnum);
