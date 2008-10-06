
/* bigscroll.c */
void getregions (ControlHandle c);
void draw1part (ControlHandle c, short part, RgnHandle rgn, short where, int val);
void drawcontrol (long part, ControlHandle c);
long test (long param, ControlHandle c);
void calc (RgnHandle rgn, ControlHandle c);
void initbigs (ControlHandle c);
void disposebigs (ControlHandle c);
pascal long main (short varcode, ControlHandle c, short message, long param);
