
/* initicons.c */
ControlHandle getnewiconcontrol (WindowPtr wp, char **path, long parid, short vrefnum, Str255 s);
short localidlookup (short localid, iconentry ** iconarray, short n);
void dobundle (short *spacemade, iconentry *** iconarray, char **h);
typeinfo *gettypeinfo (OSType type);
void geticons (icontableentry ** node, short id);
void inithash (void);
icontableentry **gethash (OSType type, OSType creator);
void hashicons (ControlHandle c);
void setoneicon (ControlHandle c);
void seticons (void);
short openappres (ControlHandle c);
