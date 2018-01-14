#if !defined(__QUICK__)
#define __QUICK__

/*
 * Copyright 1987 - 1993 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

/* include trapglue.h for prototype for CToPascalCall () used
   in inline functions below */
#include "rsys/trapglue.h"

namespace Executor
{
typedef struct ccrsr_res
{
    GUEST_STRUCT;
    GUEST<CCrsr> crsr;
    GUEST<PixMap> crsr_map;
    GUEST<char> data;
} * ccrsr_res_ptr;

extern bool ROMlib_text_output_disabled_p;

#define RGN_SMALL_SIZE_X (CWC(sizeof(Region)))
#define RGN_SMALL_SIZE (sizeof(Region))

#define RGN_STOP_X (CWC_RAW(32767))
#define RGN_STOP (32767)

#define SMALLPOLY (sizeof(Rect) + sizeof(INTEGER))

#define REALCOLORSHIM 16
#define REALMODE(x) ((x) + 16)
#define WHITEMODE (REALMODE(notPatBic) + 1)
#define BLACKMODE (REALMODE(notPatBic) + 2)

typedef enum { text_helper_draw,
               text_helper_measure } text_helper_action_t;

extern LONGINT text_helper(LONGINT n, Ptr textbufp, GUEST<Point> *nump, GUEST<Point> *denp,
                           FontInfo *finfop, GUEST<INTEGER> *charlocp,
                           text_helper_action_t action);
extern void DrawText_c_string(const char *string);
extern void nonspecial_rgn_to_special_rgn(const INTEGER *src, INTEGER *dst);
extern void ROMlib_bltrgn(RgnHandle rh, unsigned char *patp,
                          INTEGER mode, Rect *srcr, Rect *dstr);
extern void ROMlib_sizergn(RgnHandle rh, bool special_p);
extern void ROMlib_installhandle(Handle sh, Handle dh);
extern void ROMlib_showcursor(void);
extern void ROMlib_restorecursor(void);


/* Declare functions in qHooks.c. */
extern void ROMlib_CALLTEXT(INTEGER bc, Ptr bufp, Point num, Point den);
extern void ROMlib_CALLLINE(Point p);
extern void ROMlib_CALLRECT(GrafVerb v, Rect *rp);
extern void ROMlib_CALLOVAL(GrafVerb v, Rect *rp);
extern void ROMlib_CALLRRECT(GrafVerb v, Rect *rp, INTEGER ow, INTEGER oh);
extern void ROMlib_CALLARC(GrafVerb v, Rect *rp, INTEGER starta,
                           INTEGER arca);
extern void ROMlib_CALLRGN(GrafVerb v, RgnHandle rh);
extern void ROMlib_CALLPOLY(GrafVerb v, PolyHandle rh);
extern void ROMlib_CALLBITS(BitMap *bmp, const Rect *srcrp, const Rect *dstrp,
                            INTEGER mode, RgnHandle maskrh);
extern void ROMlib_CALLCOMMENT(INTEGER kind, INTEGER size, Handle datah);
extern INTEGER ROMlib_CALLTXMEAS(INTEGER bc, Ptr bufp, GUEST<Point> *nump,
                                 GUEST<Point> *denp, FontInfo *fip);
extern void ROMlib_PICWRITE(Ptr addr, INTEGER count);

/* Set up some macros to call the actual hook functions. */
#define CALLTEXT ROMlib_CALLTEXT
#define CALLLINE ROMlib_CALLLINE
#define CALLRECT ROMlib_CALLRECT
#define CALLOVAL ROMlib_CALLOVAL
#define CALLRRECT ROMlib_CALLRRECT
#define CALLARC ROMlib_CALLARC
#define CALLRGN ROMlib_CALLRGN
#define CALLPOLY ROMlib_CALLPOLY
#define CALLBITS ROMlib_CALLBITS
#define CALLCOMMENT ROMlib_CALLCOMMENT
#define CALLTXMEAS ROMlib_CALLTXMEAS
#define PICWRITE(a, c) ROMlib_PICWRITE((Ptr)(a), (c)) /* Cast is handy. */


extern RgnHandle ROMlib_circrgn(Rect *rp);
extern void ROMlib_initport(GrafPtr p);
}
#endif /* __QUICK__ */
