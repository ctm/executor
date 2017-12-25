#if !defined(__QUICK__)
#define __QUICK__

/*
 * Copyright 1987 - 1993 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "rsys/pstuff.h"

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

#if !defined(BINCOMPAT)

#define CALLTEXT(arg1, arg2, arg3, arg4)                                                \
    (thePort->grafProcs ? ((*(void (*)())thePort->grafProcs->textProc)((arg1), (arg2),  \
                                                                       (arg3), (arg4)), \
                           0)                                                           \
                        : (StdText((arg1), (arg2), (arg3), (arg4)), 0))

#define CALLLINE(arg1)                                                             \
    (thePort->grafProcs ? ((*(void (*)())thePort->grafProcs->lineProc)((arg1)), 0) \
                        : (StdLine((arg1)), 0))

#define CALLRECT(arg1, arg2)                                                               \
    (thePort->grafProcs ? ((*(void (*)())thePort->grafProcs->rectProc)((arg1), (arg2)), 0) \
                        : (StdRect((arg1), (arg2)), 0))

#define CALLOVAL(arg1, arg2)                                                               \
    (thePort->grafProcs ? ((*(void (*)())thePort->grafProcs->ovalProc)((arg1), (arg2)), 0) \
                        : (StdOval((arg1), (arg2)), 0))

#define CALLRRECT(arg1, arg2, arg3, arg4)                                                \
    (thePort->grafProcs ? ((*(void (*)())thePort->grafProcs->rRectProc)((arg1), (arg2),  \
                                                                        (arg3), (arg4)), \
                           0)                                                            \
                        : (StdRRect((arg1), (arg2), (arg3), (arg4)), 0))

#define CALLARC(arg1, arg2, arg3, arg4)                                                \
    (thePort->grafProcs ? ((*(void (*)())thePort->grafProcs->arcProc)((arg1), (arg2),  \
                                                                      (arg3), (arg4)), \
                           0)                                                          \
                        : (StdArc((arg1), (arg2), (arg3), (arg4)), 0))

#define CALLRGN(arg1, arg2)                                                               \
    (thePort->grafProcs ? ((*(void (*)())thePort->grafProcs->rgnProc)((arg1), (arg2)), 0) \
                        : (StdRgn((arg1), (arg2)), 0))

#define CALLPOLY(arg1, arg2)                                                               \
    (thePort->grafProcs ? ((*(void (*)())thePort->grafProcs->polyProc)((arg1), (arg2)), 0) \
                        : (StdPoly((arg1), (arg2)), 0))

#define CALLBITS(arg1, arg2, arg3, arg4, arg5)                                                  \
    (thePort->grafProcs ? ((*(void (*)())thePort->grafProcs->bitsProc)((arg1), (arg2),          \
                                                                       (arg3), (arg4), (arg5)), \
                           0)                                                                   \
                        : (StdBits((arg1), (arg2), (arg3), (arg4), (arg5)), 0))

#define CALLCOMMENT(arg1, arg2, arg3)                                                     \
    (thePort->grafProcs ? ((*(void (*)())thePort->grafProcs->commentProc)((arg1), (arg2), \
                                                                          (arg3)),        \
                           0)                                                             \
                        : (StdComment((arg1), (arg2), (arg3)), 0))

#define CALLTXMEAS(arg1, arg2, arg3, arg4, arg5)                                                  \
    (thePort->grafProcs ? (*(INTEGER(*)())thePort->grafProcs->txMeasProc)((arg1), (arg2),         \
                                                                          (arg3), (arg4), (arg5)) \
                        : StdTxMeas((arg1), (arg2), (arg3), (arg4), (arg5)))

#define PICWRITE(addr, count)                                                               \
    (thePort->grafProcs ? ((*(void (*)())thePort->grafProcs->putPicProc)((Ptr)(addr),       \
                                                                         (INTEGER)(count)), \
                           0)                                                               \
                        : (StdPutPic((Ptr)(addr), (INTEGER)(count)), 0))

#else /* BINCOMPAT */

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

#endif /* BINCOMPAT */

extern RgnHandle ROMlib_circrgn(Rect *rp);
extern void ROMlib_initport(GrafPtr p);
}
#endif /* __QUICK__ */
