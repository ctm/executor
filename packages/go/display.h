#define MENUBARHEIGHT	(* (short*) 0x0BAA)
#define HOTBANDHEIGHT	(ICONHEIGHTUSED + 0)
#define HOTBANDBOTTOM	(HOTBANDHEIGHT + MENUBARHEIGHT)
#define ICONWIDTHUSED	84
#define ICONHEIGHTUSED	44
#define ICSWIDTHUSED	104
#define ICSHEIGHTUSED	20
#define LISTWIDTHUSED	300
#define LISTHEIGHTUSED	20
#define FIRSTICONX		140
#define	SCROLLITEM		9
#define WIPEBIT			1
#define ICONCONTROL		(102 * 16)

#define DISKICONNUM			100
#define FOLDERICONNUM		101
#define FONTICONNUM			102
#define FONTSCICONNUM		103
#define TTFONTICONNUM		104
#define DAICONNUM			105
#define DASCICONNUM			106
#define DEFAULTAPPLICONNUM	107
#define DEFAULTDOCICONNUM	108

typedef struct
  {
    short localid;
    short resid;
    OSType type;
  }
iconentry;

enum bandnames
  {
    VOLBAND,
    FONTBAND,
    APPBAND,
    DABAND,
    FOLDERBAND,
    DOCBAND,
    NUMBANDS,			/* todo: put in a better way to figure out NUMBANDS */
    SORTBUTTON,
    HELPBUTTON
  };

enum sortorders
  {
    ALPHABETIC,
    SIZE,
    MODDATE
  };

typedef struct
  {
    short sortorder;
    short numitems;
    short bandpos;
    ControlHandle (**items)[];
  }
bandinfo;

extern icontableentry g_foldericons, g_diskicons;
extern icontableentry *g_foldericonsptr, *g_diskiconsptr;
extern short g_numdispinhotband;
extern bandinfo bands[NUMBANDS];
