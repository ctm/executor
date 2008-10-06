
/* iconcontrol.c */
void drawicxx (Ptr pat, BitMap * mbm, Rect * dest, CWindowPtr w, short size, short depth);
void dodrawcontrol (short varcode, ControlHandle c);
long dotest (long param, ControlHandle c);
void docalc (RgnHandle rgn, ControlHandle c);
void dodispose (ControlHandle c);
pascal long main (short varcode, ControlHandle c, short message, long param);
