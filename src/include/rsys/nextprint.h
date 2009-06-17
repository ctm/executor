#if !defined (__RSYS_NEXTPRINT_H__)
#define __RSYS_NEXTPRINT_H__

/*
 * Copyright 1992 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: nextprint.h 63 2004-12-24 18:19:43Z ctm $
 */

/* cottons: added `__' to idle because it conflits with posix
   function of the same name */

typedef enum { __idle,
	       seenOpenDoc,
	       seenOpenPage,
	       seenClosePage,
	       seenPageSetUp } dontuse;

typedef LONGINT printstate_t;

extern printstate_t printstate;

typedef struct {
    short top;
    short left;
    short bottom;
    short right;
} comRect;

typedef struct {
    short v;
    short h;
} comPoint;

typedef struct PACKED {
  char *baseAddr	PACKED_P;
  short rowBytes;
  comRect bounds;
} comBitMap;

typedef struct PACKED {
  char *baseAddr	PACKED_P;
  short rowBytes;
  comRect bounds;
  short pmVersion;
  short packType;
  LONGINT packSize;
  LONGINT hRes;
  LONGINT vRes;
  short pixelType;
  short pixelSize;
  short cmpCount;
  short cmpSize;
  LONGINT planeBytes;
  LONGINT pmTable;	/* really a CTabHandle */
  LONGINT pmReserved;
} comPixMap;

typedef struct PACKED {
  short ascent;
  short descent;
  short widMax;
  short leading;
} comFontInfo;

typedef struct PACKED comPoly {
  short polySize;
  comRect polyBBox;
  comPoint polyPoints[1];
} **comPolyHandle;

typedef struct PACKED comRgn {
  short rgnSize;
  comRect rgnBBox;
} **comRgnHandle;

typedef enum {
    frameVerb,
    paintVerb,
    eraseVerb,
    invertVerb,
    fillVerb
} comverb_t;

typedef char *comPtr;
typedef struct { comPtr p PACKED_P; } HIDDEN_comPtr;
typedef HIDDEN_comPtr *comHandle;

typedef LONGINT comFixed;

typedef unsigned char comPattern[8];

typedef struct PACKED {
  short device;
  comBitMap portBits;
  comRect portRect;
  comRgnHandle visRgn	PACKED_P;
  comRgnHandle clipRgn	PACKED_P;
  comPattern bkPat;
  comPattern fillPat;
  comPoint pnLoc;
  comPoint pnSize;
  short pnMode;
  comPattern pnPat;
  short pnVis;
  short txFont;
  char txFace;
  char padding;
  short txMode;
  short txSize;
  comFixed spExtra;
  LONGINT fgColor;
  LONGINT bkColor;
  short colrBit;
  short patStretch;
  comHandle picSave	PACKED_P;
  comHandle rgnSave	PACKED_P;
  comHandle polySave	PACKED_P;
  comPtr grafProcs	PACKED_P;
} comGrafPort, *comGrafPtr;

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

#endif
