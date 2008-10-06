#include <MacHeaders>

#define INTEXT				12
#define INSELECTEDICON		13
#define ICONSIZE			32
#define ICSSIZE				16
#define FONTSIZE			9
#define SIZETEXTLEFT		200
#define DATETEXTLEFT		240
#define LISTRIGHT			500

typedef struct
  {
    Rect iconrect;
    Rect textrect;
    short x;
    short y;
  }
icondraginfo;

typedef struct _iconfam
  {
    struct _iconfam **next;
    Handle icnsh;
    Handle icssh;
    Handle icl8;
    Handle icl4;
    Handle ics8;
    Handle ics4;
    OSType sig;
    OSType type;
  }
icontableentry;

enum views
  {
    ICONVIEW,
    ICSVIEW,
    LISTVIEW
  };

typedef struct
  {
    char **path;
    icontableentry **iconfam;
    long ioparid;
    short vrefnum;
    short selected;
    short action;
    short view;
    long moddate;
    long size;
  }
item;

extern unsigned char hiliteMode:0x938;
extern GDHandle theGDevice:0xCC8;
