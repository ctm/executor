#include "rsys/common.h"
#include "QuickDraw.h"
#include "WindowMgr.h"
#include "EventMgr.h"
#include "ControlMgr.h"
#include "DialogMgr.h"
#include "ToolboxUtil.h"
#include "TextEdit.h"
#include "FontMgr.h"
#include "ListMgr.h"
#include "MenuMgr.h"
#include "ResourceMgr.h"
#include "FileMgr.h"
#include "MemoryMgr.h"
#include "DeskMgr.h"
#include "StdFilePkg.h"
#include "OSUtil.h"
#include "SegmentLdr.h"
#if !defined(UNIX)
#include "HFS.h"
#else /* UNIX */
#include "ToolboxEvent.h"
#include "BinaryDecimal.h"
#endif /* UNIX */
#include "myhfs.h"

#define PLAN8

#define NELEM(x) (sizeof(x) / sizeof(x[0]))
#define PIECHARTID 508
#define ONEPARAMALERT 600
#define DOERRORALERT 601
#define ASKALERT 602
#define NOROOMALERTID 603
#define ABOUTALERT 604
#define XFERDLGID 701
#define DESTDIRWINID 704
#define DIRSONLYDLG 999
#define ACTIONBUTTON 11
#define NEWDESTBUTTON 12
#define DESTNAME 13
#define TEXTITEM 14
#define FILENAMEITEM 2
#define PIECHARTITEM 3
#define SELECTBUTTON 11

#define VOLLOCKEDMASK ((1 << 7) | (1 << 15))
#define ISDIRMASK 16

#define ON 0
#define OFF 255

extern void about_HFS_XFer(), movefiles(), copyfiles(),
    renamefile(), deletefiles(), copydisk(), newdir(),
    getnewdest();
extern INTEGER quitfunc(), docopydisk(), donewdir();
extern void dotransfer();
#if !defined(__STDC__)
extern pascal INTEGER movefileshook(),
    copyfileshook(), renamefileshook(), deletefileshook(),
    copydiskhook(), newdirhook();
extern void delete1file(), doerror();
extern INTEGER copy1file(), move1file();
extern INTEGER ask();
extern BOOLEAN caneject();
extern void getnameandfromdirid();
#else /* __STDC__ */
extern pascal INTEGER movefileshook(INTEGER item, DialogPtr dp);
extern pascal INTEGER copyfileshook(INTEGER item, DialogPtr dp);
extern pascal INTEGER renamefileshook(INTEGER item, DialogPtr dp);
extern pascal INTEGER deletefileshook(INTEGER item, DialogPtr dp);
extern pascal INTEGER copydiskhook(INTEGER item, DialogPtr dp);
extern pascal INTEGER newdirhook(INTEGER item, DialogPtr dp);
extern void getnameandfromdirid(Str255 *sp, LONGINT *fromdirid);
extern INTEGER copy1file(INTEGER srcvrn, INTEGER dstvrn, LONGINT srcdirid,
                         LONGINT dstdirid, Str255 s, BOOLEAN doit);
extern INTEGER move1file(INTEGER srcvrn, INTEGER dstvrn, LONGINT srcdirid,
                         LONGINT dstdirid, Str255 s, BOOLEAN doit);
extern void doerror(OSErr errno, char *s);
extern void delete1file(INTEGER vrn, LONGINT dirid, Str255 s);
extern INTEGER ask(char *s1, Str255 s2);
extern BOOLEAN caneject(DialogPtr dp);
#endif /* __STDC__ */

extern LONGINT destdir;
extern INTEGER destdisk, verifyfileoverwrite,
    verifydiroverwrite, verifyfiledelete, verifydirdelete;
extern SFReply globalreply;

typedef enum { up,
               down,
               nowhere } direction;

typedef struct
{
    char *name;
    void (*ptr)();
} func;

typedef struct
{
    char *name;
    INTEGER (*dlgHook)
    (INTEGER item, DialogPtr dp);
    INTEGER h, v;
    char *prompt;
} funcinfo;

typedef struct
{
    char *name;
    INTEGER *var;
} option;

typedef struct
{
    INTEGER number;
    char *message;
} errortable;
