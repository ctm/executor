/* Copyright 1986, 1988, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "QuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/region.h"
#include "rsys/quick.h"
#include "rsys/cquick.h"
#include "rsys/mman.h"
#include "rsys/safe_alloca.h"

using namespace Executor;

#undef ALLOCABEGIN
#define ALLOCABEGIN SAFE_DECL();
#undef ALLOCA
#define ALLOCA(n) SAFE_alloca(n)

#if !defined(NDEBUG)

#define RGN_SLAM(rgn) (ROMlib_sledgehammer_rgn(rgn))

static void
ROMlib_sledgehammer_rgn(RgnHandle rgn)
{
    bool special_rgn_p;
    int16_t size;
    int x, y;
    int16_t *ip, *start_ip;

    gui_assert(rgn);

    if(RGN_SMALL_P(rgn))
        return;

    special_rgn_p = RGN_SPECIAL_P(rgn);
    size = RGN_SIZE(rgn);

#if 0
  /* the line drawing code is as slimy as can be, and creates a bogo
     handle that points to `alloca'ed data */
  gui_assert (size <= GetHandleSize ((Handle) rgn));
#endif

    start_ip = ip = RGN_DATA(rgn);

    /* #### verify that `y's are increasing also */
    for(y = CW_RAW(*ip++); y != RGN_STOP; y = CW_RAW(*ip++))
    {
        /* #### verify that there are an even numbers of `x's */
        if(special_rgn_p)
        {
            int32_t prev_x = INT32_MIN;

            for(x = *ip++; x != RGN_STOP; x = *ip++)
                gui_assert(x > prev_x);
        }
        else
        {
            int32_t prev_x = INT32_MIN;

            for(x = CW_RAW(*ip++); x != RGN_STOP; x = CW_RAW(*ip++))
                gui_assert(x > prev_x);
        }
    }
    gui_assert(ip - start_ip == (size - 10) / (int)sizeof *ip);
}

#else

#define RGN_SLAM(rgn)

#endif

PUBLIC pascal trap RgnHandle Executor::C_NewRgn()
{
    RgnHandle h;

    h = (RgnHandle)NewHandleClear(RGN_SMALL_SIZE);
    RGN_SET_SMALL(h);
    return h;
}

PUBLIC pascal trap void Executor::C_OpenRgn()
{
    RgnHandle rh;

    rh = (RgnHandle)NewHandleClear(RGN_SMALL_SIZE + sizeof(INTEGER));

    RGN_SET_SIZE_AND_SPECIAL(rh, RGN_SMALL_SIZE + sizeof(INTEGER), false);
    /* sentinel */
    (RGN_DATA(rh))[0] = RGN_STOP_X;

    PORT_REGION_SAVE_X(thePort) = RM((Handle)rh);
    HidePen();
}

static bool
rgn_is_rect_p(const RgnHandle rgnh)
{
    const RgnPtr rgnp = STARH(rgnh);
    const INTEGER *ip;

    ip = RGNP_DATA(rgnp);
    return (!RGNP_SPECIAL_P(rgnp)
            && RGNP_SIZE_X(rgnp) == CWC(RGN_SMALL_SIZE + 9 * sizeof *ip)
            && ip[1] == ip[5]
            && ip[2] == ip[6]);
}

/* ROMlib_sizergn: crawl through a region and set bbox and size fields */

void Executor::ROMlib_sizergn(RgnHandle rh, bool special_p) /* INTERNAL */
{
    INTEGER *ip, i, left = RGN_STOP, right = -RGN_STOP, y;
    Size rs;

    ip = RGN_DATA(rh);
    RGN_BBOX(rh).top.raw(*ip);
    y = INT16_MIN;
    if(special_p)
    {
        while(*ip != RGN_STOP_X)
        {
            y = CW_RAW(*ip++);
            while((i = *ip++) != RGN_STOP)
            {
                if(i < left) /* testing every element is a waste here */
                    left = i;
                if(i > right) /* and here. */
                    right = i;
            }
        }
    }
    else
    {
        while(*ip != RGN_STOP_X)
        {
            y = CW_RAW(*ip++);
            while((i = CW_RAW(*ip++)) != RGN_STOP)
            {
                if(i < left) /* testing every element is a waste here */
                    left = i;
                if(i > right) /* and here. */
                    right = i;
            }
        }
    }
    if(y == INT16_MIN)
    {
        SetHandleSize((Handle)rh, RGN_SMALL_SIZE);
        RGN_SET_SMALL(rh);
        RECT_ZERO(&RGN_BBOX(rh));
    }
    else
    {
        rs = (char *)++ip - (char *)STARH(rh);
        RGN_SET_SIZE_AND_SPECIAL(rh, rs, false);
        if(rgn_is_rect_p(rh))
        {
            SetHandleSize((Handle)rh, RGN_SMALL_SIZE);
            RGN_SET_SMALL(rh);
        }
        else if(rs == RGN_SMALL_SIZE + sizeof(INTEGER))
        {
            SetHandleSize((Handle)rh, RGN_SMALL_SIZE);
            RGN_SET_SMALL(rh);
            HASSIGN_1(rh,
                      rgnBBox.top, CWC(0));
            left = right = y = 0;
        }
        else
            SetHandleSize((Handle)rh, rs);

        HASSIGN_3(rh,
                  rgnBBox.left, CW(left),
                  rgnBBox.bottom, CW(y),
                  rgnBBox.right, CW(right));
    }
}

PUBLIC pascal trap void Executor::C_CopyRgn(RgnHandle s, RgnHandle d)
{
    Size size;

    if(s == d)
        return;
    size = RGN_SIZE(s);
    ReallocHandle((Handle)d, size);
    memcpy((Ptr)STARH(d), (Ptr)STARH(s), size);
}

PUBLIC pascal trap void Executor::C_CloseRgn(RgnHandle rh)
{
    RgnHandle rgn_save = (RgnHandle)PORT_REGION_SAVE(thePort);

    if(RGN_SIZE_X(rgn_save) == CWC(RGN_SMALL_SIZE + sizeof(INTEGER))
       || rgn_is_rect_p(rgn_save))
        RGN_SET_SMALL(rgn_save);

    ROMlib_installhandle(PORT_REGION_SAVE(thePort), (Handle)rh);
    SetHandleSize((Handle)rh, RGN_SIZE(rh));
    PORT_REGION_SAVE_X(thePort) = nullptr;
    ShowPen();
}

PUBLIC pascal trap void Executor::C_DisposeRgn(RgnHandle rh)
{
    DisposHandle((Handle)rh);
}

PUBLIC pascal trap void Executor::C_SetEmptyRgn(RgnHandle rh)
{
    SetHandleSize((Handle)rh, RGN_SMALL_SIZE);
    RGN_SET_SMALL(rh);
    RECT_ZERO(&RGN_BBOX(rh));
}

PUBLIC pascal trap void Executor::C_SetRectRgn(RgnHandle rh, INTEGER left, INTEGER top, INTEGER right, INTEGER bottom)
{
    SetHandleSize((Handle)rh, RGN_SMALL_SIZE);
    SetRect(&RGN_BBOX(rh), left, top, right, bottom);
    RGN_SET_SMALL(rh);
}

PUBLIC pascal trap void Executor::C_RectRgn(RgnHandle rh, Rect *rect)
{
    Rect r;

    /* suck the rect into a local before we do the `SetHandleSize ()'
     because `rect' may point into an unlocked handle (that shouldn't
     actually ever happen, but it is better to be safe than sorry) */
    r = *rect;
    SetHandleSize((Handle)rh, RGN_SMALL_SIZE);
    RGN_SET_SMALL(rh);
    RGN_BBOX(rh) = r;
}

/* IM is wrong... not based on offsets */
PUBLIC pascal trap void Executor::C_OffsetRgn(RgnHandle rh, INTEGER dh, INTEGER dv)
{
    if(dh || dv)
    {
        INTEGER *ip, *ep;
        RgnPtr rp;

        rp = STARH(rh);
        OffsetRect(&RGNP_BBOX(rp), dh, dv);
        for(ip = RGNP_DATA(rp),
        ep = (INTEGER *)((char *)ip + RGNP_SIZE(rp)) - 6;
            ip < ep; ip++)
        {
            *ip = CW_RAW(CW_RAW(*ip) + (dv));
            ++ip;
            do
            {
                *ip = CW_RAW(CW_RAW(*ip) + (dh));
                ++ip;
                *ip = CW_RAW(CW_RAW(*ip) + (dh));
                ++ip;
            } while(*ip != RGN_STOP_X);
        }
    }
}

#define NHPAIR 1024

PUBLIC pascal trap BOOLEAN Executor::C_PtInRgn(Point p, RgnHandle rh)
{
    if(!PtInRect(p, &RGN_BBOX(rh)))
        return false;
    if(RGN_SMALL_P(rh))
        return true;
    else
    {
        INTEGER *ipe, *op;
        INTEGER *ipr;
        INTEGER v, in;
        /* double buffered (really [2][NHPAIR]) */
        INTEGER endpoints[2 * NHPAIR];

        ipr = RGN_DATA(rh);
        *endpoints = RGN_STOP;
        ipe = endpoints;
        op = endpoints + NHPAIR;
        while((v = CW_RAW(*ipr++)) != RGN_STOP)
        {
            if(v > p.v)
            {
                in = false;
                for(op = ipe; *op++ <= p.h; in = !in)
                    ;
                return in;
            }
            while(*ipr != RGN_STOP_X || *ipe != RGN_STOP)
            {
                if(CW_RAW(*ipr) < *ipe)
                    *op++ = CW_RAW(*ipr++);
                else if(*ipe < CW_RAW(*ipr))
                    *op++ = *ipe++;
                else
                {
                    ipr++;
                    ipe++;
                }
            }
            ipr++;
            ipe++;
            *op = RGN_STOP;
            if(op >= endpoints + NHPAIR)
            {
                ipe = endpoints + NHPAIR;
                op = endpoints;
            }
            else
            {
                ipe = endpoints;
                op = endpoints + NHPAIR;
            }
        }
    }
    return false;
}

/*
 * NOTE: ipr points to native memory and hence has to have dereferences
 *	 swapped.  Neither mergebuf, nor freebuf have this requirement.
 */

#define merge(ipr, mergebuf, freebuf)                                     \
    {                                                                     \
        INTEGER *ipe, *op;                                                \
                                                                          \
        ipe = mergebuf;                                                   \
        op = freebuf;                                                     \
        while(*ipr != RGN_STOP_X || *ipe != RGN_STOP)                     \
        {                                                                 \
            if(CW_RAW(*ipr) < *ipe)                                       \
                *op++ = CW_RAW(*ipr++);                                   \
            else if(*ipe < CW_RAW(*ipr))                                  \
                *op++ = *ipe++;                                           \
            else                                                          \
            {                                                             \
                ipr++;                                                    \
                ipe++;                                                    \
            }                                                             \
        }                                                                 \
        ipr++;                                                            \
        ipe++;                                                            \
        *op++ = RGN_STOP;                                                 \
        *op = RGN_STOP;                                                   \
        op = freebuf;                                                     \
        freebuf = mergebuf;                                               \
        mergebuf = op;                                                    \
                                                                          \
        /* if (mergebuf[0] == mergebuf[1] && mergebuf[0] != RGN_STOP)     \
             printf("m[0] = %d, m[1] = %d, f[0] = %d, f[1] = %d\n",       \
                    mergebuf[0], mergebuf[1], freebuf[0], freebuf[1]); */ \
    }

#define nextline(ipr, mergebuf)   \
    {                             \
        mergebuf = ipr;           \
        while(*ipr++ != RGN_STOP) \
            ;                     \
    }

/*
 * NOTE: I *think* outputrgn has inputs in native space, but should
 *	 output to synthetic space.
 */

#define outputrgn(vx, cur, new, out)                \
    {                                               \
        INTEGER *ipe, *ipr;                         \
                                                    \
        ipe = cur;                                  \
        ipr = new;                                  \
        *out++ = CW_RAW(vx);                        \
        auto hold = out;                            \
        while(*ipr != RGN_STOP || *ipe != RGN_STOP) \
        {                                           \
            if(*ipr < *ipe)                         \
                *out++ = CW_RAW(*ipr++);            \
            else if(*ipe < *ipr)                    \
                *out++ = CW_RAW(*ipe++);            \
            else                                    \
            {                                       \
                ipr++;                              \
                ipe++;                              \
            }                                       \
        }                                           \
        if(hold == out)                             \
            --out;                                  \
        else                                        \
        {                                           \
            *out++ = CW_RAW(RGN_STOP);              \
        }                                           \
        ipe = cur;                                  \
        cur = new;                                  \
        new = ipe;                                  \
    }

#define newsource        \
    {                    \
        sstart = *sp1++; \
        sstop = *sp1++;  \
    }
#define newdest          \
    {                    \
        dstart = *sp2++; \
        dstop = *sp2++;  \
    }
#define includ(start, stop, outp) \
    {                             \
        *outp++ = start;          \
        *outp++ = stop;           \
    }

#define sect(src1, src2, outptr)                                                        \
    {                                                                                   \
        INTEGER sstart, dstart, sstop, dstop;                                           \
        INTEGER *sp1, *sp2, *outp;                                                      \
                                                                                        \
        sp1 = src1;                                                                     \
        sp2 = src2;                                                                     \
        outp = outptr;                                                                  \
        newsource                                                                       \
            newdest while(sstart != RGN_STOP && dstart != RGN_STOP) if(sstop <= dstart) \
                newsource else if(dstop <= sstart)                                      \
                    newdest else if(sstart <= dstart)                                   \
        {                                                                               \
            if(sstop < dstop)                                                           \
            {                                                                           \
                includ(dstart, sstop, outp)                                             \
                    dstart                                                              \
                    = sstop;                                                            \
                newsource                                                               \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                includ(dstart, dstop, outp)                                             \
                    sstart                                                              \
                    = dstop;                                                            \
                newdest                                                                 \
            }                                                                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            if(dstop < sstop)                                                           \
            {                                                                           \
                includ(sstart, dstop, outp)                                             \
                    sstart                                                              \
                    = dstop;                                                            \
                newdest                                                                 \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                includ(sstart, sstop, outp)                                             \
                    dstart                                                              \
                    = sstop;                                                            \
                newsource                                                               \
            }                                                                           \
        }                                                                               \
        *outp = RGN_STOP;                                                               \
    }

/*
 * NOTE: here we're creating a "special" region that is really a set
 *	 of start stop pairs that are in native endianness.
 */

#define sectline(src1, src2, outp, y)                   \
    {                                                   \
        INTEGER sstart, dstart, sstop, dstop;           \
        INTEGER *sp1, *sp2, *outptr;                    \
                                                        \
        sp1 = src1;                                     \
        sp2 = src2;                                     \
        newsource                                       \
            newdest                                     \
                *outp++                                 \
            = CW_RAW(y);                                \
        outptr = outp;                                  \
        while(sstart != RGN_STOP && dstart != RGN_STOP) \
        {                                               \
            if(sstop <= dstart)                         \
                newsource else if(dstop <= sstart)      \
                    newdest else if(sstart <= dstart)   \
                {                                       \
                    if(sstop < dstop)                   \
                    {                                   \
                        includ(dstart, sstop, outp)     \
                            dstart                      \
                            = sstop;                    \
                        newsource                       \
                    }                                   \
                    else                                \
                    {                                   \
                        includ(dstart, dstop, outp)     \
                            sstart                      \
                            = dstop;                    \
                        newdest                         \
                    }                                   \
                }                                       \
            else                                        \
            {                                           \
                if(dstop < sstop)                       \
                {                                       \
                    includ(sstart, dstop, outp)         \
                        sstart                          \
                        = dstop;                        \
                    newdest                             \
                }                                       \
                else                                    \
                {                                       \
                    includ(sstart, sstop, outp)         \
                        dstart                          \
                        = sstop;                        \
                    newsource                           \
                }                                       \
            }                                           \
        }                                               \
        if(wehavepairs || outp > outptr)                \
        {                                               \
            *outp++ = RGN_STOP;                         \
            wehavepairs = true;                         \
        }                                               \
        else                                            \
        {                                               \
            outp--;                                     \
            wehavepairs = false;                        \
        }                                               \
    }

#define uunion(src1, src2, outptr)                                                       \
    {                                                                                    \
        INTEGER sstart, dstart, sstop, dstop;                                            \
        INTEGER *sp1, *sp2, *outp;                                                       \
                                                                                         \
        sp1 = src1;                                                                      \
        sp2 = src2;                                                                      \
        outp = outptr;                                                                   \
        newsource                                                                        \
            newdest while(sstart != RGN_STOP || dstart != RGN_STOP) if(sstart <= dstart) \
        {                                                                                \
            if(sstop < dstart)                                                           \
            {                                                                            \
                includ(sstart, sstop, outp)                                              \
                    newsource                                                            \
            }                                                                            \
            else if(sstop <= dstop)                                                      \
            {                                                                            \
                dstart = sstart;                                                         \
                newsource                                                                \
            }                                                                            \
            else                                                                         \
                newdest                                                                  \
        }                                                                                \
        else                                                                             \
        {                                                                                \
            if(dstop < sstart)                                                           \
            {                                                                            \
                includ(dstart, dstop, outp)                                              \
                    newdest                                                              \
            }                                                                            \
            else if(dstop <= sstop)                                                      \
            {                                                                            \
                sstart = dstart;                                                         \
                newdest                                                                  \
            }                                                                            \
            else                                                                         \
                newsource                                                                \
        }                                                                                \
        *outp = RGN_STOP;                                                                \
    }

#define diff(src1, src2, outptr)                                       \
    {                                                                  \
        INTEGER sstart, dstart, sstop, dstop;                          \
        INTEGER *sp1, *sp2, *outp;                                     \
                                                                       \
        sp1 = src1;                                                    \
        sp2 = src2;                                                    \
        outp = outptr;                                                 \
        newsource                                                      \
            newdest while(sstart != RGN_STOP || dstart != RGN_STOP)    \
        {                                                              \
            if(sstop <= dstart)                                        \
            {                                                          \
                includ(sstart, sstop, outp)                            \
                    newsource                                          \
            }                                                          \
            else if(dstop <= sstart)                                   \
                newdest else                                           \
                {                                                      \
                    if(sstart < dstart)                                \
                        includ(sstart, dstart, outp) if(sstop < dstop) \
                            newsource else if(sstop == dstop)          \
                        {                                              \
                            newsource                                  \
                                newdest                                \
                        }                                              \
                    else                                               \
                    {                                                  \
                        sstart = dstop;                                \
                        newdest                                        \
                    }                                                  \
                }                                                      \
        }                                                              \
        *outp = RGN_STOP;                                              \
    }

#if !defined(NDEBUG)

PRIVATE void assertincreasing(INTEGER *ip)
{
    LONGINT lastx = -327680;
    while(*ip != RGN_STOP)
    {
        gui_assert(lastx < *ip);
        lastx = *ip++;
    }
}
#endif /* NDEBUG */

PRIVATE void sectbinop(RgnHandle srcrgn1, RgnHandle srcrgn2,
                       RgnHandle dstrgn);
typedef enum {
    sectop,
    unionop,
    diffop
} optype;

PRIVATE void binop(optype op, RgnHandle srcrgn1, RgnHandle srcrgn2,
                   RgnHandle dstrgn);
PRIVATE LONGINT comparex(char *cp1, char *cp2);
PRIVATE LONGINT comparey(char *cp1, char *cp2);
PRIVATE void ptorh(INTEGER *p, RgnHandle rh);

PRIVATE void sectbinop(RgnHandle srcrgn1, RgnHandle srcrgn2, RgnHandle dstrgn)
{
    /* note that the buffers will not be restricted to the endpoints
       that their name implies.  The pointers will always point to
       the buffer containing the most current set of endpoints */

    INTEGER src1endpoints[NHPAIR], *src1ep = src1endpoints;
    INTEGER src2endpoints[NHPAIR], *src2ep = src2endpoints;
    INTEGER sectsegendpoints[NHPAIR], *sectsegep = sectsegendpoints;
    INTEGER sectcurendpoints[NHPAIR], *sectcurep = sectcurendpoints;
    INTEGER freeendpoints[NHPAIR], *freeep = freeendpoints;
    INTEGER *ipr1, *ipr2;
    INTEGER *temppoints, *tptr;
    INTEGER v1, v2, vx;
    GUEST<INTEGER> r1[9], r2[9];
    Rect *rp;
    INTEGER nspecial;
    RgnHandle exchrgn;
    BOOLEAN wehavepairs;
    ALLOCABEGIN

    /*
     * Some performance enhancements could be put here...
     * if we are secting and the rgnBox's don't sect ... etc.
     */
    if(RGN_SPECIAL_P(srcrgn1))
    {
        if(RGN_SPECIAL_P(srcrgn2))
            nspecial = 2;
        else
            nspecial = 1;
    }
    else
    {
        if(RGN_SPECIAL_P(srcrgn2))
        {
            nspecial = 1;
            exchrgn = srcrgn1;
            srcrgn1 = srcrgn2;
            srcrgn2 = exchrgn;
        }
        else
            nspecial = 0;
    }
    tptr = temppoints = ((INTEGER *)
                             ALLOCA(2 * ((RGN_SIZE(srcrgn1))
                                         + RGN_SIZE(srcrgn2)
                                         + 18 * sizeof(INTEGER))));
    /* todo ... look over these */
    if(RGN_SMALL_P(srcrgn1))
    {
        rp = &RGN_BBOX(srcrgn1);
        r1[0] = rp->top;
        r1[1] = rp->left;
        r1[2] = rp->right != CWC(RGN_STOP) ? rp->right : GUEST<INTEGER>(CWC(RGN_STOP - 1));
        r1[3] = CWC(RGN_STOP);
        r1[4] = rp->bottom != CWC(RGN_STOP) ? rp->bottom : GUEST<INTEGER>(CWC(RGN_STOP - 1));
        r1[5] = rp->left;
        r1[6] = rp->right != CWC(RGN_STOP) ? rp->right : GUEST<INTEGER>(CWC(RGN_STOP - 1));
        r1[7] = CWC(RGN_STOP);
        r1[8] = CWC(RGN_STOP);
        ipr1 = (INTEGER *)r1;
    }
    else
        ipr1 = RGN_DATA(srcrgn1);
    if(RGN_SMALL_P(srcrgn2))
    {
        rp = &RGN_BBOX(srcrgn2);
        r2[0] = rp->top;
        r2[1] = rp->left;
        r2[2] = rp->right != CWC(RGN_STOP) ? rp->right : GUEST<INTEGER>(CWC(RGN_STOP - 1));
        r2[3] = CWC(RGN_STOP);
        r2[4] = rp->bottom != CWC(RGN_STOP) ? rp->bottom : GUEST<INTEGER>(CWC(RGN_STOP - 1));
        r2[5] = rp->left;
        r2[6] = rp->right != CWC(RGN_STOP) ? rp->right : GUEST<INTEGER>(CWC(RGN_STOP - 1));
        r2[7] = CWC(RGN_STOP);
        r2[8] = CWC(RGN_STOP);
        ipr2 = (INTEGER *)r2;
    }
    else
        ipr2 = RGN_DATA(srcrgn2);
    *src1ep = *src2ep = *sectsegep = *sectcurep = *freeep = *(src1ep + 1) = *(src2ep + 1) = RGN_STOP;

    v1 = CW_RAW(*ipr1++);
    v2 = CW_RAW(*ipr2++);
    wehavepairs = false; /* whether or not scan lines have stuff */
    switch(nspecial)
    {
        case 0:
            while(v1 != RGN_STOP && v2 != RGN_STOP)
            {
                if(v1 < v2)
                {
                    merge(ipr1, src1ep, freeep) /* no semi ... macro */
                        vx
                        = v1;
                    v1 = CW_RAW(*ipr1++);
                }
                else if(v2 < v1)
                {
                    merge(ipr2, src2ep, freeep)
                        vx
                        = v2;
                    v2 = CW_RAW(*ipr2++);
                }
                else
                { /* equal */
                    merge(ipr1, src1ep, freeep)
                        merge(ipr2, src2ep, freeep)
                            vx
                        = v1;
                    v1 = CW_RAW(*ipr1++);
                    v2 = CW_RAW(*ipr2++);
                }
                sect(src1ep, src2ep, sectsegep)
                    outputrgn(vx, sectcurep, sectsegep, tptr);
            }
            break;
        case 1:
            vx = -32768;
            while(v1 != RGN_STOP && v2 != RGN_STOP)
            {
                if(v1 < v2)
                {
                    nextline(ipr1, src1ep) /* no semi ... macro */
                        vx
                        = v1;
                    v1 = CW_RAW(*ipr1++);
                }
                else if(v2 < v1)
                {
                    merge(ipr2, src2ep, freeep)
                        vx
                        = v2;
                    v2 = CW_RAW(*ipr2++);
                }
                else
                { /* equal */
                    nextline(ipr1, src1ep)
                        merge(ipr2, src2ep, freeep)
                            vx
                        = v1;
                    v1 = CW_RAW(*ipr1++);
                    v2 = CW_RAW(*ipr2++);
                }
#if !defined(NDEBUG)
                assertincreasing(src1ep);
                assertincreasing(src2ep);
#endif /* NDEBUG */
                sectline(src1ep, src2ep, tptr, vx)
            }
            break;
        case 2:
            vx = -32768;
            while(v1 != RGN_STOP && v2 != RGN_STOP)
            {
                if(v1 < v2)
                {
                    nextline(ipr1, src1ep) /* no semi ... macro */
                        vx
                        = v1;
                    v1 = CW_RAW(*ipr1++);
                }
                else if(v2 < v1)
                {
                    nextline(ipr2, src2ep)
                        vx
                        = v2;
                    v2 = CW_RAW(*ipr2++);
                }
                else
                { /* equal */
                    nextline(ipr1, src1ep)
                        nextline(ipr2, src2ep)
                            vx
                        = v1;
                    v1 = CW_RAW(*ipr1++);
                    v2 = CW_RAW(*ipr2++);
                }
                sectline(src1ep, src2ep, tptr, vx)
            }
            *tptr++ = vx;
            *tptr++ = RGN_STOP;
            break;
    }
    *tptr++ = RGN_STOP_X;
    gui_assert(sizeof(INTEGER) * (tptr - temppoints) <= 2 * ((Hx(srcrgn1, rgnSize) & 0x7FFF) + (Hx(srcrgn2, rgnSize) & 0x7FFF) + 18 * sizeof(INTEGER)));

    {
        int dst_rgn_size = (RGN_SMALL_SIZE
                            + sizeof(INTEGER) * (tptr - temppoints));
        RGN_SET_SIZE_AND_SPECIAL(dstrgn, dst_rgn_size, false);
        /* TODO fix rgnBBox here */
        ReallocHandle((Handle)dstrgn, dst_rgn_size);
    }

    memmove(RGN_DATA(dstrgn), temppoints,
            (tptr - temppoints) * sizeof *temppoints);
    ROMlib_sizergn(dstrgn, nspecial > 0); /* could do this while copying... */
    if(nspecial > 0)
        RGN_SET_SPECIAL(dstrgn, true);
    ASSERT_SAFE(temppoints);
    ALLOCAEND
}

PRIVATE void binop(optype op, RgnHandle srcrgn1, RgnHandle srcrgn2, RgnHandle dstrgn)
{
    /* note that the buffers will not be restricted to the endpoints
       that their name implies.  The pointers will always point to
       the buffer containing the most current set of endpoints */

    INTEGER src1endpoints[NHPAIR], *src1ep = src1endpoints;
    INTEGER src2endpoints[NHPAIR], *src2ep = src2endpoints;
    INTEGER sectsegendpoints[NHPAIR], *sectsegep = sectsegendpoints;
    INTEGER sectcurendpoints[NHPAIR], *sectcurep = sectcurendpoints;
    INTEGER freeendpoints[NHPAIR], *freeep = freeendpoints;
    INTEGER *ipr1, *ipr2;
    INTEGER *temppoints, *tptr;
    INTEGER v1, v2, vx;
    GUEST<INTEGER> r1[9], r2[9];
    Rect *rp;
    ALLOCABEGIN

    /*
     * Some performance enhancements could be put here...
     * if we are secting and the rgnBox's don't sect ... etc.
     */
    tptr = temppoints = (INTEGER *)ALLOCA((Size)2 * (Hx(srcrgn1, rgnSize) + Hx(srcrgn2, rgnSize) + 18 * sizeof(INTEGER)));
    /* todo ... look over these */
    if(RGN_SMALL_P(srcrgn1))
    {
        rp = &(RGN_BBOX(srcrgn1));
        r1[0] = rp->top;
        r1[1] = rp->left;
        r1[2] = rp->right != CWC(RGN_STOP) ? rp->right : GUEST<INTEGER>(CW(RGN_STOP - 1));
        r1[3] = CWC(RGN_STOP);
        r1[4] = rp->bottom != CWC(RGN_STOP) ? rp->bottom : GUEST<INTEGER>(CW(RGN_STOP - 1));
        r1[5] = rp->left;
        r1[6] = rp->right != CWC(RGN_STOP) ? rp->right : GUEST<INTEGER>(CW(RGN_STOP - 1));
        r1[7] = CWC(RGN_STOP);
        r1[8] = CWC(RGN_STOP);
        ipr1 = (INTEGER *)r1;
    }
    else
        ipr1 = RGN_DATA(srcrgn1);
    if(RGN_SMALL_P(srcrgn2))
    {
        rp = &(RGN_BBOX(srcrgn2));
        r2[0] = rp->top;
        r2[1] = rp->left;
        r2[2] = rp->right != CWC(RGN_STOP) ? rp->right : GUEST<INTEGER>(CW(RGN_STOP - 1));
        r2[3] = CWC(RGN_STOP);
        r2[4] = rp->bottom != CWC(RGN_STOP) ? rp->bottom : GUEST<INTEGER>(CW(RGN_STOP - 1));
        r2[5] = rp->left;
        r2[6] = rp->right != CWC(RGN_STOP) ? rp->right : GUEST<INTEGER>(CW(RGN_STOP - 1));
        r2[7] = CWC(RGN_STOP);
        r2[8] = CWC(RGN_STOP);
        ipr2 = (INTEGER *)r2;
    }
    else
        ipr2 = RGN_DATA(srcrgn2);
    *src1ep = *src2ep = *sectsegep = *sectcurep = *freeep = *(src1ep + 1) = *(src2ep + 1) = RGN_STOP;

    v1 = CW_RAW(*ipr1++);
    v2 = CW_RAW(*ipr2++);
    while(v1 != RGN_STOP || v2 != RGN_STOP)
    {
        if(v1 < v2)
        {
            merge(ipr1, src1ep, freeep) /* no semi ... macro */
                vx
                = v1;
            v1 = CW_RAW(*ipr1++);
        }
        else if(v2 < v1)
        {
            merge(ipr2, src2ep, freeep)
                vx
                = v2;
            v2 = CW_RAW(*ipr2++);
        }
        else
        { /* equal */
            merge(ipr1, src1ep, freeep)
                merge(ipr2, src2ep, freeep)
                    vx
                = v1;
            v1 = CW_RAW(*ipr1++);
            v2 = CW_RAW(*ipr2++);
        }
        switch(op)
        {
            case sectop:
                sect(src1ep, src2ep, sectsegep) break;
            case diffop:
                diff(src1ep, src2ep, sectsegep) break;
            case unionop:
                uunion(src1ep, src2ep, sectsegep) break;
        }
        outputrgn(vx, sectcurep, sectsegep, tptr);
    }
    *tptr++ = RGN_STOP_X;
    gui_assert(sizeof(INTEGER) * (tptr - temppoints) <= 2 * (Hx(srcrgn1, rgnSize) + Hx(srcrgn2, rgnSize) + 18 * sizeof(INTEGER)));
    HxX(dstrgn, rgnSize) = CW(RGN_SMALL_SIZE + sizeof(INTEGER) * (tptr - temppoints));
    /* TODO fix rgnBBox here */
    ReallocHandle((Handle)dstrgn,
                  RGN_SMALL_SIZE + sizeof(INTEGER) * (tptr - temppoints));
    {
        INTEGER *ip, *op;
        ip = temppoints;
        op = (INTEGER *)STARH(dstrgn) + 5;
        while(ip != tptr)
            *op++ = *ip++;
        ROMlib_sizergn(dstrgn, false); /* could do this while copying... */
    }
    ASSERT_SAFE(temppoints);
    ALLOCAEND
}

void Executor::nonspecial_rgn_to_special_rgn(const INTEGER *src, INTEGER *dst)
{
    static const INTEGER empty_row[2] = { 0, RGN_STOP };
    const INTEGER *prev;

    /* Each scanline gets XOR'd with the previous one.  We proceed until we
   * hit a RGN_STOP_X y value.
   */
    for(prev = empty_row; (dst[0] = src[0]) != RGN_STOP_X;)
    {
        int srcx, prevx;
        INTEGER *next_prev;

        /* Fetch the first X on this new scanline. */
        srcx = CW_RAW(src[1]);
        if(srcx == RGN_STOP)
        {
            /* The row with which to XOR is empty, so just extend the
	   * current row.
	   */
            src += 2;
            continue;
        }

        next_prev = dst;

        /* Fetch the first X on the previous scanline.  If the previous
       * scanline was empty, then XORing just gives us the scanline
       * from src.
       */
        prevx = prev[1];
        if(prevx == RGN_STOP)
        {
            for(; (dst[1] = CW_RAW(src[1])) != RGN_STOP; src += 2, dst += 2)
                dst[2] = CW_RAW(src[2]);
            src += 2;
            dst += 2;
            prev = next_prev;
            continue;
        }

        /* Neither scanline is empty.  This is the tricky case.  Since
       * we are XORing two scanlines of start/stop X pairs, we just
       * merge the two lists of X coordinates into one sorted array.
       * If you draw two scanlines, one above the other, you'll see
       * that all of the X coordinates just "drop down" to the
       * resulting scanline.  The only exception is when both
       * scanlines have the same X value; in that case, it
       * "disappears".
       */
        src += 2;
        prev += 2;
        dst++;

        while(1)
        {
            if(srcx < prevx)
            {
                *dst++ = srcx;
                srcx = CW_RAW(*src++);
                if(srcx == RGN_STOP)
                    goto read_prev_only;
            }
            else if(srcx > prevx)
            {
                *dst++ = prevx;
                prevx = *prev++;
                if(prevx == RGN_STOP)
                    goto read_src_only;
            }
            else /* srcx == prevx */
            {
                srcx = CW_RAW(*src++);
                prevx = *prev++;
                if(srcx == RGN_STOP)
                    goto read_prev_only;
                if(prevx == RGN_STOP)
                    goto read_src_only;
            }
        }

    /* We've run out of data from "src", so just copy the rest of the
       * prev scanline.
       */
    read_prev_only:
        while(prevx != RGN_STOP)
        {
            *dst++ = prevx;
            prevx = *prev++;
        }
        goto do_next;

    /* We've run out of data from "prev", so just copy the rest of the
       * src scanline.
       */
    read_src_only:
        while(srcx != RGN_STOP)
        {
            *dst++ = srcx;
            srcx = CW_RAW(*src++);
        }

    do_next:
        *dst++ = RGN_STOP;
        prev = next_prev;
    }
}

/*
 * Here's how the little bugger works:
 *  Three macros are needed to be defined so that some boilerplate
 *  can expand into rhtopandinseth and inseth
 *  the pairs will always be kept as (y, x) so there need to be
 *  two different NEXTPAIR routines and comparison routines...
 */

static INTEGER npairs;

#define DECL void rhtopandinseth(RgnHandle rh, INTEGER *p, INTEGER dh)

#define STATEDECL SignedByte state;
#define ITYPE INTEGER
#define SETIO                      \
    ip = RGN_DATA(rh);             \
    op = p;                        \
    y = CW_RAW(*ip++);             \
    npairs = 0;                    \
    state = HGetState((Handle)rh); \
    HLock((Handle)rh)
#define NEXTPAIR (x = CW_RAW(*ip++)) == RGN_STOP ? (y = CW_RAW(*ip++), 0) : 1
#define INCLXY(x, y) *op++ = y, *op++ = x, npairs++
#define UNSETIO                   \
    HSetState((Handle)rh, state); \
    INCLXY(RGN_STOP, RGN_STOP)

#include "hintemplate.h"

#define DECL void hinset(INTEGER *p, INTEGER dh)

#define STATEDECL
#define ITYPE INTEGER
#define SETIO      \
    ip = p;        \
    op = p;        \
    y = *(ip + 1); \
    npairs = 0
#define NEXTPAIR y == *(ip + 1) ? (x = *ip++, ip++, 1) : (y = *(ip + 1), 0)
#define INCLXY(x, y) *op++ = x, *op++ = y, npairs++
#define UNSETIO

#include "hintemplate.h"

PRIVATE LONGINT comparex(const void *cp1, const void *cp2)
{
    const INTEGER *p1, *p2;
    LONGINT retval;

    p1 = (const INTEGER *)cp1 + 1;
    p2 = (const INTEGER *)cp2 + 1;
    if(*p1 < *p2)
        retval = -1;
    else if(*p1 > *p2)
        retval = 1;
    else
    {
        p1--;
        p2--;
        if(*p1 < *p2)
            retval = -1;
        else if(*p1 > *p2)
            retval = 1;
        else
            retval = 0;
    }
    return retval;
}

PRIVATE LONGINT comparey(const void *cp1, const void *cp2)
{
    const INTEGER *p1, *p2;
    LONGINT retval;

    p1 = (const INTEGER *)cp1;
    p2 = (const INTEGER *)cp2;
    if(*p1 < *p2)
        retval = -1;
    else if(*p1 > *p2)
        retval = 1;
    else
    {
        p1++;
        p2++;
        if(*p1 < *p2)
            retval = -1;
        else if(*p1 > *p2)
            retval = 1;
        else
            retval = 0;
    }
    return retval;
}

/*
 * BEWARE:  ptorh can trash memory... regions can grow by being inset (honest)
 */

PRIVATE void ptorh(INTEGER *p, RgnHandle rh)
{
    INTEGER y, oy, *op;

    op = RGN_DATA(rh);
    if(npairs)
    { /* decrement one 'cause of the 32767 sentinel */
        *op++ = CW_RAW(oy = *p);
        for(; npairs; npairs -= 2)
        {
            if((y = *p++) != oy)
            {
                *op++ = RGN_STOP_X;
                *op++ = CW_RAW(y);
                oy = y;
            }
            *op++ = CW_RAW(*p++);
            ++p; /* if Cx((*ip)++ != oy) error! */
            *op++ = CW_RAW(*p++);
        }
    }
    *op++ = RGN_STOP_X; /* need one or two? */
    *op++ = RGN_STOP_X; /* need one or two? */
}

PUBLIC pascal trap void Executor::C_InsetRgn(RgnHandle rh, INTEGER dh, INTEGER dv)
{
    Handle h;
    INTEGER *p;
    Rect *rp;
    Size newsize;
    ;

    if(RGN_SMALL_P(rh))
    {
        InsetRect(&RGN_BBOX(rh), dh, dv);
#define INSANEBUTNECESSARY
#if defined(INSANEBUTNECESSARY)
        rp = &RGN_BBOX(rh);
        if(CW(rp->top) >= CW(rp->bottom) || CW(rp->left) >= CW(rp->right))
            RECT_ZERO(rp);
#endif /* INSANEBUTNECESSARY */
    }
    else
    {
        newsize = 4 * RGN_SIZE(rh);
        h = NewHandle(newsize);
        HLock(h);
        p = (INTEGER *)STARH(h);
        rhtopandinseth(rh, p, dh); /* must be combined for efficiency */
        gui_assert(npairs * (int)sizeof(INTEGER) * 2 <= newsize);
        qsort(p, npairs, sizeof(INTEGER) * 2, comparex);
        hinset(p, dv);
        qsort(p, npairs, sizeof(INTEGER) * 2, comparey);
        ReallocHandle((Handle)rh, newsize);
        ptorh(p, rh);
        ROMlib_sizergn(rh, false);
        gui_assert(Hx(rh, rgnSize) <= newsize);
        HUnlock(h);
        DisposHandle(h);
    }
}

static bool
justone(const Rect *rp, RgnHandle rgn, RgnHandle dest)
{
    const Rect *rp2 = &RGN_BBOX(rgn);

    if(CW(rp->left) <= CW(rp2->left)
       && CW(rp->top) <= CW(rp2->top)
       && CW(rp->right) >= CW(rp2->right)
       && CW(rp->bottom) >= CW(rp2->bottom))
    {
        CopyRgn(rgn, dest);
        return true;
    }
    else
        return false;
}

PUBLIC pascal trap void Executor::C_SectRgn(RgnHandle s1, RgnHandle s2, RgnHandle dest)
{
    Rect dummy;
    const Region *rp1, *rp2;

    RGN_SLAM(s1);
    RGN_SLAM(s2);

    rp1 = STARH(s1);
    rp2 = STARH(s2);

    if(RGNP_SMALL_P(rp1))
    {
        if(RGNP_SMALL_P(rp2))
        {
            SectRect(&RGNP_BBOX(rp1), &RGNP_BBOX(rp2), &RGN_BBOX(dest));
            /* #### should this set the handle size of `dest' */
            RGN_SET_SMALL(dest);
            return;
        }
        else if(justone(&RGNP_BBOX(rp1), s2, dest))
            return;
    }
    else if(RGNP_SMALL_P(rp2))
        if(justone(&RGNP_BBOX(rp2), s1, dest))
            return;
    if(SectRect(&RGNP_BBOX(rp1), &RGNP_BBOX(rp2), &dummy))
        sectbinop(s1, s2, dest);
    else
        SetEmptyRgn(dest);
}

PUBLIC pascal trap void Executor::C_UnionRgn(RgnHandle s1, RgnHandle s2, RgnHandle dest)
{
    if(EmptyRgn(s1))
        CopyRgn(s2, dest);
    else if(EmptyRgn(s2))
        CopyRgn(s1, dest);
    else
        binop(unionop, s1, s2, dest);
}

PUBLIC pascal trap void Executor::C_DiffRgn(RgnHandle s1, RgnHandle s2, RgnHandle dest)
{
    if(EmptyRgn(s1) || EmptyRgn(s2))
        CopyRgn(s1, dest);
    else
        binop(diffop, s1, s2, dest);
}

PUBLIC pascal trap void Executor::C_XorRgn(RgnHandle s1, RgnHandle s2, RgnHandle dest)
{
    INTEGER y1, y2, x1, x2;
    INTEGER *ip1, *ip2, *op;
    INTEGER left, right, bottom;
    INTEGER cnt;
    RgnHandle finalrestingplace;
    GUEST<RgnPtr> temp2, temp3;
    RgnPtr temp;
    ALLOCABEGIN

    /* the +36 below is necessary because small regions have size of 10
       yet they actually have an additional implied 18 bytes associated
       with them */

    if(s1 == dest || s2 == dest)
    {
        finalrestingplace = dest;
        dest = (RgnHandle)NewHandle((Size)Hx(s1, rgnSize) + Hx(s2, rgnSize) + 36);
    }
    else
    {
        finalrestingplace = 0;
        ReallocHandle((Handle)dest, Hx(s1, rgnSize) + Hx(s2, rgnSize) + 36);
    }

    if(RGN_SMALL_P(s1))
    {
        temp = (RgnPtr)ALLOCA(RGN_SMALL_SIZE + 9 * sizeof(INTEGER));
#if 0
	BlockMoveData(CL(*(Ptr *) s1), (Ptr) temp, RGN_SMALL_SIZE);
#else
        memcpy((Ptr)temp, MR(*s1), RGN_SMALL_SIZE);
#endif
        op = (INTEGER *)((char *)temp + RGN_SMALL_SIZE);
        *op++ = HxX(s1, rgnBBox.top).raw();
        *op++ = HxX(s1, rgnBBox.left).raw();
        *op++ = HxX(s1, rgnBBox.right).raw();
        *op++ = RGN_STOP_X;
        *op++ = HxX(s1, rgnBBox.bottom).raw();
        *op++ = HxX(s1, rgnBBox.left).raw();
        *op++ = HxX(s1, rgnBBox.right).raw();
        *op++ = RGN_STOP_X;
        *op++ = RGN_STOP_X;
        ASSERT_SAFE(temp);
        temp2 = RM(temp);
        s1 = &temp2;
    }

    if(RGN_SMALL_P(s2))
    {
        temp = (RgnPtr)ALLOCA(RGN_SMALL_SIZE + 9 * sizeof(INTEGER));
#if 0
	BlockMoveData(CL(*(Ptr *) s2), (Ptr) temp, RGN_SMALL_SIZE);
#else
        memcpy((Ptr)temp, STARH(s2), RGN_SMALL_SIZE);
#endif
        op = (INTEGER *)((char *)temp + RGN_SMALL_SIZE);
        *op++ = HxX(s2, rgnBBox.top).raw();
        *op++ = HxX(s2, rgnBBox.left).raw();
        *op++ = HxX(s2, rgnBBox.right).raw();
        *op++ = RGN_STOP_X;
        *op++ = HxX(s2, rgnBBox.bottom).raw();
        *op++ = HxX(s2, rgnBBox.left).raw();
        *op++ = HxX(s2, rgnBBox.right).raw();
        *op++ = RGN_STOP_X;
        *op++ = RGN_STOP_X;
        ASSERT_SAFE(temp);
        temp3 = RM(temp);
        s2 = &temp3;
    }

    ip1 = RGN_DATA(s1);
    ip2 = RGN_DATA(s2);
    op = RGN_DATA(dest);
    left = RGN_STOP;
    right = -32768;
    bottom = -32768;
    for(y1 = CW_RAW(*ip1++), y2 = CW_RAW(*ip2++); y1 != RGN_STOP || y2 != RGN_STOP;)
    {
        if(y1 < y2)
        {
            bottom = y1;
            *op++ = CW_RAW(y1);
            while((*op++ = *ip1++) != RGN_STOP_X)
            {
                x1 = CW_RAW(op[-1]);
                if(x1 < left)
                    left = x1;
                if(x1 > right)
                    right = x1;
            }
            y1 = CW_RAW(*ip1++);
        }
        else if(y2 < y1)
        {
            bottom = y2;
            *op++ = CW_RAW(y2);
            while((*op++ = *ip2++) != RGN_STOP_X)
            {
                x2 = CW_RAW(op[-1]);
                if(x2 < left)
                    left = x2;
                if(x2 > right)
                    right = x2;
            }
            y2 = CW_RAW(*ip2++);
        }
        else
        {
            cnt = 0;
            for(x1 = CW_RAW(*ip1++), x2 = CW_RAW(*ip2++);
                x1 != RGN_STOP || x2 != RGN_STOP;)
            {
                if(x1 < x2)
                {
                    if(!cnt)
                    {
                        bottom = y1;
                        *op++ = CW_RAW(y1);
                        if(x1 < left)
                            left = x1;
                    }
                    else if(x1 > right)
                        right = x1;
                    *op++ = CW_RAW(x1);
                    cnt++;
                    x1 = CW_RAW(*ip1++);
                }
                else if(x2 < x1)
                {
                    if(!cnt)
                    {
                        bottom = y1;
                        *op++ = CW_RAW(y1);
                        if(x2 < left)
                            left = x2;
                    }
                    else if(x2 > right)
                        right = x2;
                    *op++ = CW_RAW(x2);
                    cnt++;
                    x2 = CW_RAW(*ip2++);
                }
                else
                {
                    x1 = CW_RAW(*ip1++);
                    x2 = CW_RAW(*ip2++);
                }
            }
            if(cnt)
                *op++ = RGN_STOP_X;
            y1 = CW_RAW(*ip1++);
            y2 = CW_RAW(*ip2++);
        }
    }
    *op++ = RGN_STOP_X;
    HASSIGN_5(dest,
              rgnBBox.top, *(GUEST<INTEGER> *)RGN_DATA(dest),
              rgnBBox.left, CW(left),
              rgnBBox.bottom, CW(bottom),
              rgnBBox.right, CW(right),
              rgnSize, CW((char *)op - (char *)STARH(dest)));
    if(rgn_is_rect_p(dest))
        RGN_SET_SMALL(dest);
    if(finalrestingplace)
    {
        ROMlib_installhandle((Handle)dest, (Handle)finalrestingplace);
        SetHandleSize((Handle)finalrestingplace,
                      RGN_SIZE(finalrestingplace));
    }
    else
        SetHandleSize((Handle)dest, RGN_SIZE(dest));
    ALLOCAEND
}

PUBLIC pascal trap BOOLEAN Executor::C_RectInRgn(Rect *rp, /* IMIV-23 */
                                                 RgnHandle rh)
{
    RgnHandle newrh;
    BOOLEAN retval;

    newrh = NewRgn();
    RectRgn(newrh, rp);
    SectRgn(newrh, rh, newrh);
    retval = !EmptyRgn(newrh);
    DisposeRgn(newrh);
    return retval;
}

PUBLIC pascal trap BOOLEAN Executor::C_EqualRgn(RgnHandle r1, RgnHandle r2)
{
    /* Since the first field of the region is the size, this
   * will return false if the sizes differ, too.
   */
    return !memcmp(STARH(r1), STARH(r2), RGN_SIZE(r1));
}

PUBLIC pascal trap BOOLEAN Executor::C_EmptyRgn(RgnHandle rh)
{
    // FIXME: #warning What does a mac do with a NULL HANDLE here?
    BOOLEAN retval;

    retval = rh ? EmptyRect(&RGN_BBOX(rh)) : true;
    return retval;
}

#if !defined(NDEBUG)
PUBLIC void Executor::ROMlib_printrgn(RgnHandle h)
{
    INTEGER *ip, x, y;
    INTEGER special, size, newsize;

    size = Hx(h, rgnSize);
    special = size & 0x8000;
    size &= ~0x8000;
    if(special)
        printf("SPECIAL, ");
    printf("size = %ld, l = %ld, t = %ld, r = %ld, b = %ld\n",
           (long)size, (long)Hx(h, rgnBBox.left),
           (long)Hx(h, rgnBBox.top),
           (long)Hx(h, rgnBBox.right), (long)Hx(h, rgnBBox.bottom));

    HLockGuard guard(h);
    if(!RGN_SMALL_P(h))
    {
        ip = RGN_DATA(h);
        while((y = CW_RAW(*ip++)) != RGN_STOP)
        {
            printf("%ld:", (long)y);
            if(special)
            {
                while((x = *ip++) != RGN_STOP)
                    printf(" %ld", (long)x);
            }
            else
            {
                while((x = CW_RAW(*ip++)) != RGN_STOP)
                    printf(" %ld", (long)x);
            }
            printf(" 32767\n");
        }
        printf("32767\n");
        newsize = ((char *)ip - (char *)STARH(h));
        if(newsize != size)
            printf("WARNING: computed size = %d\n", newsize);
    }
}

#endif /* NDEBUG */
