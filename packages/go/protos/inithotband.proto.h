
/* inithotband.c */
void additemtohotband (short whichband, char **path, long parid, short vrefnum, short action);
void checkhotbandcontrol (void);
void shiftband (short start, short offset);
void removeitemfromhotband (ControlHandle c);
void copycontroltohotband (ControlHandle c);
void selectiontohotband (void);
void addvolumes (void);
void resetvolumes (void);
void fillbands (FILE * f);
void inithotband (FILE * f);
