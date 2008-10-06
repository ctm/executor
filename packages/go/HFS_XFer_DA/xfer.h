#ifdef THINK_C
#include <pascal.h>
#endif /* THINK_C */

#include "mytype.h"

#define PLAN8

#ifndef NELEM
#define NELEM(x)	(sizeof(x) / sizeof(x[0]))
#endif /*NELEM */

#define	DRVR(id, offset)	((short) (0xC000|(id<<5)|offset))

extern short ourid;

#define FOUR_PARAM_ALERT			DRVR(ourid,  0)
#define DOERRORALERT			DRVR(ourid,  1)
#define ASKALERT				DRVR(ourid,  2)
#define NOROOMALERTID			DRVR(ourid,  3)
#define ABOUTALERT				DRVR(ourid,  4)
#define PIECHARTID				DRVR(ourid,  5)
#define XFERDLGID				DRVR(ourid,  6)
#define DIRSONLYDLG				DRVR(ourid,  7)
#define DESTDIRWINID			DRVR(ourid,  8)
#define ONEPARAMABORTALERT		DRVR(ourid,  9)
#define DOERRORABORTALERT		DRVR(ourid, 10)

#define	STARTALERT				DRVR(ourid, 11)

#define	COPY_DISK_ITEM					1
#define	MOVE_FILE_ITEM					2
#define	COPY_FILE_ITEM					3
#define	RENAME_FILE_ITEM				4
#define	DELETE_FILE_ITEM				5
#define	NEW_FOLDER_ITEM					6
#define	NEW_VOLUME_ITEM					11

#define	FIRST_VERIFY_ITEM				7	/* THESE MUST BE CONTIGUOUS */

#define	VERIFY_OVERWRITE_FILE_ITEM		7
#define	VERIFY_OVERWRITE_FOLDER_ITEM	8
#define	VERIFY_DELETE_FILE_ITEM			9
#define	VERIFY_DELETE_FOLDER_ITEM		10

#define	LAST_VERIFY_ITEM				10

#define	ITEM_TO_BIT(item)	(1 << (item - VERIFY_OVERWRITE_FILE_ITEM))

#define	VERIFY_OVERWRITE_FILE		ITEM_TO_BIT(VERIFY_OVERWRITE_FILE_ITEM)
#define	VERIFY_OVERWRITE_FOLDER		ITEM_TO_BIT(VERIFY_OVERWRITE_FOLDER_ITEM)
#define	VERIFY_DELETE_FILE			ITEM_TO_BIT(VERIFY_DELETE_FILE_ITEM)
#define	VERIFY_DELETE_FOLDER		ITEM_TO_BIT(VERIFY_DELETE_FOLDER_ITEM)

#define ABORTITEM	3
extern short abortflag;

#define ACTIONBUTTON			11
#define NEWDESTBUTTON			12
#define DESTNAME				13
#define TEXTITEM				14
#define FILENAMEITEM			2
#define PIECHARTITEM			3
#define SELECTBUTTON			11

#define VOLLOCKEDMASK			((1<<7) | (1<<15))
#define ISDIRMASK				16

#define ON						0
#define OFF						255

#define CHECKR(call, var)										\
	err = call(var, false);										\
	if (err != noErr) {											\
		doerror(err, (StringPtr)"\p" #call);					\
		return err;												\
	}

#define CHECKRV(call, var)										\
	err = call(var, false);										\
	if (err != noErr) {											\
		doerror(err, (StringPtr)"\p" #call);					\
		return;													\
	}

extern long destdir;
extern short destdisk, verify_flags;
extern SFReply globalreply;

typedef enum
  {
    up, down, nowhere
  }
direction;

typedef struct
  {
    StringPtr name;
    void (*ptr) (void);
  }
func;

typedef struct
  {
    StringPtr name;
    short pascal (*dlgHook) (short item, DialogPtr dp);
    short h, v;
    StringPtr prompt;
  }
funcinfo;

typedef struct
  {
    StringPtr name;
    short *var;
  }
option;

typedef struct
  {
    short number;
    char message[50];
  }
errortable;

typedef enum
  {
    NOTE, CAUTION, STOP
  }
alerttype;

typedef enum
  {
    datafork, resourcefork = 0xFF
  }
forktype;
