#if !defined (_OURSTUFF_H_)

#import <Cocoa/Cocoa.h>
#include <Availability.h>

#if __MAC_OS_X_VERSION_MAX_ALLOWED < 101000
typedef NSUInteger NSEventModifierFlags;
#endif

namespace Executor {
extern void sendresumeevent(LONGINT cvtclip);
extern void sendsuspendevent(void);
extern void ROMlib_writenameorgkey(char *name,  char *org, char *key);
extern void contextswitch(char **from_spp, char **to_spp);
extern LONGINT insertfonttbl(char **op, char doit);

extern INTEGER ROMlib_next_butmods_to_mac_butmods (NSEventModifierFlags nextflags);
extern void postnextevent(NSEvent * neventp);
extern void ROMlib_updatemouselocation(NSEvent * neventp);

extern void sendpaste(void);
extern void sendcopy(void);
extern void oldmain(int argc, char **argv);
extern void convertchars(char *data, long length, const unsigned char *table);
extern void nextmain(void);

#if defined (__OBJC__)
extern NSMenu *global_menu;
extern NSCursor *realcursor, *blankcursor;
extern NSPasteboard *ROMlib_pasteboard;
#endif
void NeXTMain();
}
extern NSInteger ROMlib_ourchangecount;

#endif /* _OURSTUFF_H_ */
