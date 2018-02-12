#if !defined(__RSYS_NEXTPRINT_H__)
#define __RSYS_NEXTPRINT_H__

#include "rsys/mactype.h"
#include "QuickDraw.h"

/*
 * Copyright 1992 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

/* cottons: added `__' to idle because it conflits with posix
   function of the same name */
namespace Executor
{
typedef enum { __idle,
               seenOpenDoc,
               seenOpenPage,
               seenClosePage,
               seenPageSetUp } dontuse;

typedef LONGINT printstate_t;

extern printstate_t printstate;

typedef enum {
    frameVerb,
    paintVerb,
    eraseVerb,
    invertVerb,
    fillVerb
} comverb_t;

extern void NeXTPrArc(LONGINT verb, Rect *r, LONGINT starta, LONGINT arca,
                      GrafPtr gp);
extern void NeXTPrBits(const BitMap *srcbmp, const Rect *srcrp, const Rect *dstrp,
                       LONGINT mode, RgnHandle mask, GrafPtr gp);
extern void NeXTPrLine(Point to, GrafPtr gp);
extern void NeXTPrOval(LONGINT v, Rect *rp, GrafPtr gp);
extern void NeXTsendps(LONGINT size, Ptr textbufp);
extern void NeXTPrGetPic(Ptr dp, LONGINT bc, GrafPtr gp);
extern void NeXTPrPutPic(Ptr sp, LONGINT bc, GrafPtr gp);
extern void NeXTPrPoly(LONGINT verb, PolyHandle ph, GrafPtr gp);
extern void NeXTPrRRect(LONGINT verb, Rect *r, LONGINT width, LONGINT height,
                        GrafPtr gp);
extern void NeXTPrRect(LONGINT v, Rect *rp, GrafPtr gp);
extern void NeXTPrRgn(LONGINT verb, RgnHandle rgn, GrafPtr gp);
extern void NeXTPrText(LONGINT n, Ptr textbufp, Point num, Point den,
                       GrafPtr gp);
extern short NeXTPrTxMeas(LONGINT n, Ptr p, GUEST<Point> *nump, GUEST<Point> *denp,
                          FontInfo *finfop, GrafPtr gp);
extern void NeXTOpenPage(void);
extern void ROMlib_updatenextpagerect(Rect *rp);
extern void ROMlib_updatemacpagerect(Rect *rp1, Rect *rp2, Rect *rp3);

extern char **ROMlib_availableFonts(void);
extern void ROMlib_newFont(char *font, float txSize);
}
#endif
