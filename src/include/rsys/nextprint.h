#if !defined (__RSYS_NEXTPRINT_H__)
#define __RSYS_NEXTPRINT_H__

#include "rsys/mactype.h"
#include "QuickDraw.h"

/*
 * Copyright 1992 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: nextprint.h 63 2004-12-24 18:19:43Z ctm $
 */

/* cottons: added `__' to idle because it conflits with posix
   function of the same name */
namespace Executor {
typedef enum { __idle,
	       seenOpenDoc,
	       seenOpenPage,
	       seenClosePage,
	       seenPageSetUp } dontuse;

typedef LONGINT printstate_t;

extern printstate_t printstate;

// ###autc04 TODO: why are these redefined here??

typedef struct { GUEST_STRUCT;
    GUEST<short> top;
    GUEST<short> left;
    GUEST<short> bottom;
    GUEST<short> right;
} comRect;

typedef struct { GUEST_STRUCT;
    GUEST<short> v;
    GUEST<short> h;
} comPoint;

typedef char *char_ptr;

struct comBitMap { GUEST_STRUCT;
    GUEST< char_ptr> baseAddr;
    GUEST< short> rowBytes;
    GUEST< comRect> bounds;
};

struct comPixMap { GUEST_STRUCT;
    GUEST< char_ptr> baseAddr;
    GUEST< short> rowBytes;
    GUEST< comRect> bounds;
    GUEST< short> pmVersion;
    GUEST< short> packType;
    GUEST< LONGINT> packSize;
    GUEST< LONGINT> hRes;
    GUEST< LONGINT> vRes;
    GUEST< short> pixelType;
    GUEST< short> pixelSize;
    GUEST< short> cmpCount;
    GUEST< short> cmpSize;
    GUEST< LONGINT> planeBytes;
    GUEST< LONGINT> pmTable;    /* really a CTabHandle */
    GUEST< LONGINT> pmReserved;
};

struct comFontInfo { GUEST_STRUCT;
    GUEST< short> ascent;
    GUEST< short> descent;
    GUEST< short> widMax;
    GUEST< short> leading;
};

typedef struct comPoly { GUEST_STRUCT;
    GUEST< short> polySize;
    GUEST< comRect> polyBBox;
    GUEST< comPoint[1]> polyPoints;
} **comPolyHandle;

typedef struct comRgn { GUEST_STRUCT;
    GUEST< short> rgnSize;
    GUEST< comRect> rgnBBox;
} **comRgnHandle;

typedef enum {
    frameVerb,
    paintVerb,
    eraseVerb,
    invertVerb,
    fillVerb
} comverb_t;

typedef char *comPtr;
MAKE_HIDDEN(comPtr);
typedef HIDDEN_comPtr *comHandle;

typedef LONGINT comFixed;

typedef unsigned char comPattern[8];

typedef struct comGrafPort { GUEST_STRUCT;
    GUEST< short> device;
    GUEST< comBitMap> portBits;
    GUEST< comRect> portRect;
    GUEST< comRgnHandle> visRgn;
    GUEST< comRgnHandle> clipRgn;
    GUEST< comPattern> bkPat;
    GUEST< comPattern> fillPat;
    GUEST< comPoint> pnLoc;
    GUEST< comPoint> pnSize;
    GUEST< short> pnMode;
    GUEST< comPattern> pnPat;
    GUEST< short> pnVis;
    GUEST< short> txFont;
    GUEST< char> txFace;
    GUEST< char> padding;
    GUEST< short> txMode;
    GUEST< short> txSize;
    GUEST< comFixed> spExtra;
    GUEST< LONGINT> fgColor;
    GUEST< LONGINT> bkColor;
    GUEST< short> colrBit;
    GUEST< short> patStretch;
    GUEST< comHandle> picSave;
    GUEST< comHandle> rgnSave;
    GUEST< comHandle> polySave;
    GUEST< comPtr> grafProcs;
} *comGrafPtr;

extern void NeXTPrArc(LONGINT verb, comRect *r, LONGINT starta, LONGINT arca,
							        comGrafPtr gp);
extern void NeXTPrBits(comBitMap *srcbmp, comRect *srcrp, comRect *dstrp,
			     LONGINT mode, comRgnHandle mask, comGrafPtr gp);
extern void NeXTPrLine(comPoint to, comGrafPtr gp);
extern void NeXTPrOval(LONGINT v, comRect *rp, comGrafPtr gp);
extern void NeXTsendps(LONGINT size, comPtr textbufp);
extern void NeXTPrGetPic(comPtr dp, LONGINT bc, comGrafPtr gp);
extern void NeXTPrPutPic(comPtr sp, LONGINT bc, comGrafPtr gp);
extern void NeXTPrPoly(LONGINT verb, comPolyHandle ph, comGrafPtr gp);
extern void NeXTPrRRect(LONGINT verb, comRect *r, LONGINT width, LONGINT height,
								comGrafPtr gp);
extern void NeXTPrRect(LONGINT v, comRect *rp, comGrafPtr gp);
extern void NeXTPrRgn(LONGINT verb, comRgnHandle rgn, comGrafPtr gp);
extern void NeXTPrText(LONGINT n, comPtr textbufp, comPoint num, comPoint den,
							        comGrafPtr gp);
extern short NeXTPrTxMeas(LONGINT n, comPtr p, comPoint *nump, comPoint *denp,
				      comFontInfo *finfop, comGrafPtr gp);
extern void NeXTOpenPage( void );
extern void ROMlib_updatenextpagerect(comRect *rp);
extern void ROMlib_updatemacpagerect(comRect *rp1, comRect *rp2, comRect *rp3);

extern char **ROMlib_availableFonts(void);
extern void ROMlib_newFont(char *font, float txSize);
}
#endif
