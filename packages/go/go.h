#define NDEBUG			/* I'm building a production copy -- ctm */

#include <stdio.h>
#include "iconcontrol.h"

enum
  {
    new_folder_menuid = 1,
    open_menuid,
    print_menuid,
    close_menuid,
    save_menuid,
    unused,
    delete_menuid,
    duplicate_menuid,
    get_info_menuid,
    rename_menuid,
    send_to_hotband_menuid,
    unused_2,
    check_for_disk_menuid,
    format_menuid,
    eject_menuid,
    unused_3,
    quit_menuid
  };

#define	MAXDRIVENUM		15
#define DIRBIT			(1 << 4)
#define	ICONTABLESIZE	91
#define SIGARRAYSIZE	41
#define TEXTEDITORPOS	(SIGARRAYSIZE)
#define HOTBANDRESID	200
#define ABOUTDIALOG		201
#define BANDARRAYSIZE	50
#define CREATOR			'GOGO'
#define GOMBARID		128
#define FIRSTMENU		300
#define FILEMENU		301
#define EDITMENU		302
#define VIEWMENU		303
#define SCROLLBARWIDTH	16
#define SCROLLSPEED		9
#define	DEFAULTEDITORSTRINGID	201

#define COPYCURSORID		128
#define	MOVECURSORID		129

#define	GONAME				"\pgo.appl"
#define GOSAVEFILE			"godata.sav"
#define GOBACKUPFILE		"godata.bak"

#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

enum actions
  {
    LAUNCH,
    OPENDIR,
    LAUNCHCREATOR,
    OPENDA,
    NOACTION
  };


typedef struct
  {
    short localid;
    short resid;
  }
identry;

typedef struct
  {
    short sortorder;	/* FIXME - why not use enum here? */
    short view;
    short numitems;
    Handle path;
    long iodirid;
    short vrefnum;
    ControlHandle sbar;
    ControlHandle (**items)[];
  }
opendirinfo;

typedef struct
  {
    OSType filetype;
    short band;
    short action;
    short iconid;
    icontableentry **iconh;
  }
typeinfo;

typedef struct _applist
  {
    short vrefnum;
    long parid;
    Str255 name;
    OSType sig;
    struct _applist **next;
  }
applist;

extern applist **sigowners[TEXTEDITORPOS + 1];
extern typeinfo typearray[];

/* used to determine if a control is an icon control */
extern Handle g_iconcdefproc;

/* someday more than one item may be selected so make g_selection look like a handle */
extern ControlHandle (**g_selection)[];
     
/* time until screensaver kicks in in sixtieths of seconds */
extern long g_screensavetime;
     
extern long g_lastclick;

/* Pointer to the "hotband", the special window at the top of the screen */
extern DialogPtr g_hotband;

extern icontableentry **icontable[ICONTABLESIZE];
extern short g_currentband;
extern short g_done;
extern TEHandle g_currenttitle;

extern CCrsrHandle g_movecursor, g_copycursor;
/*
 * Fun Executor trickery (kids, we are trained professionals; don't try this at home)
 *
 * unixcd is needed because under Executor, we could be traversing a mount point.
 * When that happens, Executor needs to internally "mount" the new filesystem and we need
 * to update our vrefnum.
 *
 * NOTE: this code will cause any version of Executor prior to 1.99m to crash.
 */

     pascal OSErr unixmount (CInfoPBRec * cpbp) =
{
  0x2078, 0x0058, 0x2068, 0x0020, 0x4e90
};
