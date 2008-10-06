
/* init.c */
void checkgrowitemarray (short numitems, ControlHandle (***items)[]);
void tail (char *path, Str255 s);
void getname (Str255 name, FILE * f);
OSErr cd (short *cwd, Str255 name);
void appenddir (char ***p, Str255 name);
OSErr getonefileinfo (FILE * f, CInfoPBRec * cpb, char ***path, short *volume);
void setupmenus (void);
void setdefaulteditor (void);
void init (void);
