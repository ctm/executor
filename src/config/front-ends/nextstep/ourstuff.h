#if !defined (_OURSTUFF_H_)

#ifndef OPENSTEP
#include <dpsclient/event.h>
#endif /* not OPENSTEP */

extern void sendresumeevent(LONGINT cvtclip);
extern void sendsuspendevent(void);
extern void ROMlib_writenameorgkey(char *name,  char *org, char *key);
extern void contextswitch(char **from_spp, char **to_spp);
extern void protectus(long serialnumber, long max);
extern LONGINT insertfonttbl(char **op, char doit);

#ifndef OPENSTEP

extern INTEGER ROMlib_next_butmods_to_mac_butmods (LONGINT nextflags);
extern void postnextevent(NXEvent * neventp);
extern void ROMlib_updatemouselocation(NXEvent * neventp);

#else /* OPENSTEP */

extern INTEGER ROMlib_next_butmods_to_mac_butmods (unsigned int nextflags);
extern void postnextevent(NSEvent * neventp);
extern void ROMlib_updatemouselocation(NSEvent * neventp);

#endif /* OPENSTEP */

extern void sendpaste(void);
extern void sendcopy(void);
extern void oldmain(long argc, char **argv);
extern void convertchars(char *data, long length, unsigned char *table);
extern void nextmain(void);

#if defined (__OBJC__)
extern id global_menu;
extern id realcursor, blankcursor;
extern id ROMlib_pasteboard;
#endif

extern int ROMlib_ourchangecount;

#endif /* _OURSTUFF_H_ */
