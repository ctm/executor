/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "ToolboxEvent.h"
#include "ToolboxUtil.h"

#include "rsys/quick.h"
#include "rsys/cquick.h"
#include "rsys/glue.h"
#include "rsys/region.h"

using namespace Executor;

static BOOLEAN EquivRect(Rect *, Rect *);
static INTEGER fromhex(char c);

PRIVATE BOOLEAN EquivRect(Rect *rp1, Rect *rp2)
{
    return Cx(rp1->bottom) - Cx(rp1->top) == Cx(rp2->bottom) - Cx(rp2->top) && Cx(rp1->right) - Cx(rp1->left) == Cx(rp2->right) - Cx(rp2->left);
}

#define RANDSEED ((ULONGINT)randSeed)
PUBLIC pascal trap INTEGER Executor::C_Random()
{
    INTEGER retval;

    RndSeed = Cx(TickCount()); /* what better? */
    if(RANDSEED >= 0x80000000)
        randSeedX = CL((RANDSEED & 0x7FFFFFFF) + 1);
    randSeedX = CL((RANDSEED * 16807 + ((((RANDSEED >> 14) * 16807) + (((RANDSEED & ((1 << 14) - 1)) * 16807) >> 14))
                                        >> 17))
                   & 0x7FFFFFFF);
    if(RANDSEED == 0x7FFFFFFF)
        randSeedX = 0;
    retval = randSeed;
    return retval == -32768 ? 0 : retval;
}

PUBLIC pascal trap BOOLEAN Executor::C_GetPixel(INTEGER h, INTEGER v)
{
    BitMap temp_bm;
    unsigned char temp_fbuf[4];
    Rect src_rect, dst_rect;

    gui_assert(!CGrafPort_p(thePort));

    temp_bm.baseAddr = RM((Ptr)temp_fbuf);
    temp_bm.bounds.top = CWC(0);
    temp_bm.bounds.bottom = CWC(1);
    temp_bm.bounds.left = CWC(0);
    temp_bm.bounds.right = CWC(1);
    temp_bm.rowBytes = CWC(4);

    src_rect.top = CW(v);
    src_rect.bottom = CW(v + 1);
    src_rect.left = CW(h);
    src_rect.right = CW(h + 1);

    dst_rect = temp_bm.bounds;

    CopyBits(PORT_BITS_FOR_COPY(thePort), &temp_bm,
             &src_rect, &dst_rect, srcCopy, NULL);

    return (*temp_fbuf & 0x80) != 0;
}

/* fromhex: converts from '0'-'9' to 0-9, 'a-z' and 'A-Z' similarly */

PRIVATE INTEGER fromhex(char c)
{
    if(c >= '0' && c <= '9')
        return (c - '0');
    else if(c >= 'A' && c <= 'Z')
        return (c - 'A' + 10);
    else
        return (c - 'a' + 10);
}

PUBLIC pascal trap void Executor::C_StuffHex(Ptr p, StringPtr s)
{
    char *sp, *ep;
    unsigned len;

    len = s[0];
    sp = (char *)s + 1;
    ep = sp + (len & ~1);
    for(; sp != ep; sp += 2)
        *p++ = (fromhex(*sp) << 4) | fromhex(sp[1]);

    if(len & 1)
        *p = (*p & 0xF) | (fromhex(*sp) << 4);
}

PUBLIC pascal trap void Executor::C_ScalePt(GUEST<Point> *pt, Rect *srcr, Rect *dstr)
{
    INTEGER srcdh, srcdv, dstdh, dstdv;

    if(pt->h || pt->v)
    {
        srcdh = Cx(srcr->right) - Cx(srcr->left);
        srcdv = Cx(srcr->bottom) - Cx(srcr->top);
        dstdh = Cx(dstr->right) - Cx(dstr->left);
        dstdv = Cx(dstr->bottom) - Cx(dstr->top);

        pt->h = CW(((((LONGINT)CW(pt->h) * dstdh) << 1) / srcdh + 1) >> 1);
        pt->v = CW(((((LONGINT)CW(pt->v) * dstdv) << 1) / srcdv + 1) >> 1);

        if(CW(pt->v) < 1)
            pt->v = CWC(1);
        if(CW(pt->h) < 1)
            pt->h = CWC(1);
    }
}

PUBLIC pascal trap void Executor::C_MapPt(GUEST<Point> *pt, Rect *srcr, Rect *dstr)
{
    INTEGER srcdh, srcdv, dstdh, dstdv;

    srcdh = Cx(srcr->right) - Cx(srcr->left);
    srcdv = Cx(srcr->bottom) - Cx(srcr->top);
    dstdh = Cx(dstr->right) - Cx(dstr->left);
    dstdv = Cx(dstr->bottom) - Cx(dstr->top);

    pt->h = CW(CW(pt->h) - (Cx(srcr->left)));
    pt->v = CW(CW(pt->v) - (Cx(srcr->top)));
    pt->h = CW((LONGINT)CW(pt->h) * dstdh / srcdh);
    pt->v = CW((LONGINT)CW(pt->v) * dstdv / srcdv);
    pt->h = CW(CW(pt->h) + (Cx(dstr->left)));
    pt->v = CW(CW(pt->v) + (Cx(dstr->top)));
}

PUBLIC pascal trap void Executor::C_MapRect(Rect *r, Rect *srcr, Rect *dstr)
{
    MapPt((GUEST<Point> *)&r->top, srcr, dstr);
    MapPt((GUEST<Point> *)&r->bottom, srcr, dstr);
}

#define IMPOSSIBLE -1
#define MAPH(x) (x = UNFIX((x - xoff1) * xcoff) + xoff2)
#define MAPV(y) (y = UNFIX((y - yoff1) * ycoff) + yoff2)
#define UNFIX(x) ((x) >> 16)

PUBLIC pascal trap void Executor::C_MapRgn(RgnHandle rh, Rect *srcr, Rect *dstr)
{
    GUEST<INTEGER> *ip, *op, *saveop;
    INTEGER oldv, newv, *tempp, srcdh, dstdh, srcdv, dstdv;
    INTEGER xoff1, xoff2, yoff1, yoff2, x1, x2;
    Fixed xcoff, ycoff;
    INTEGER buf1[1000], buf2[1000], *mergebuf, *freebuf, *ipe;
    LONGINT hold;
    BOOLEAN done;

    if(EquivRect(srcr, dstr))
        OffsetRgn(rh, Cx(dstr->left) - Cx(srcr->left),
                  Cx(dstr->top) - Cx(srcr->top));
    else
    {
        if(RGN_SMALL_P(rh))
            MapRect(&HxX(rh, rgnBBox), srcr, dstr);
        else
        {
            srcdh = CW(srcr->right) - (xoff1 = CW(srcr->left));
            dstdh = CW(dstr->right) - (xoff2 = CW(dstr->left));
            srcdv = CW(srcr->bottom) - (yoff1 = CW(srcr->top));
            dstdv = CW(dstr->bottom) - (yoff2 = CW(dstr->top));
            xcoff = FixRatio(dstdh, srcdh);
            ycoff = FixRatio(dstdv, srcdv);
            ip = op = (GUEST<INTEGER> *)STARH(rh) + 5;
            oldv = -32768;
            buf1[0] = 32767;
            mergebuf = buf1;
            freebuf = buf2;
            do
            {
                done = (newv = CW(*ip++)) == 32767;
                MAPV(newv);
                if(newv != oldv || done)
                {
                    if(mergebuf[0] != 32767)
                    {
                        *op++ = CW(oldv);
                        saveop = op;
                        hold = IMPOSSIBLE;
                        for(tempp = mergebuf; (x1 = *tempp++) != 32767;)
                        {
                            MAPH(x1);
                            if(hold == IMPOSSIBLE)
                                hold = (unsigned short)x1;
                            else if(hold == x1)
                                hold = IMPOSSIBLE;
                            else
                            {
                                *op++ = CW(hold);
                                hold = (unsigned short)x1;
                            }
                        }
                        if(hold != IMPOSSIBLE)
                            *op++ = CW(hold);
                        gui_assert(!((op - saveop) & 1));
                        if(op == saveop)
                            --op;
                        else
                            *op++ = CWC(32767);
                    }
                    gui_assert(op < ip);
                    mergebuf[0] = 32767;
                    oldv = newv;
                }
                if(!done)
                {
                    for(tempp = freebuf, ipe = mergebuf,
                    x1 = CW(*ip++), x2 = *ipe++;
                        ;)
                    {
                        if(x1 < x2)
                        {
                            *tempp++ = x1;
                            x1 = CW(*ip++);
                        }
                        else if(x1 > x2)
                        {
                            *tempp++ = x2;
                            x2 = *ipe++;
                        }
                        else
                        {
                            if(x1 == 32767)
                                /*-->*/ break;
                            x1 = CW(*ip++);
                            x2 = *ipe++;
                        }
                    }
                    gui_assert(!((tempp - freebuf) & 1));
                    *tempp++ = 32767;
                    tempp = freebuf;
                    freebuf = mergebuf;
                    mergebuf = tempp;
                }
            } while(!done);
            *op++ = CWC(32767);
            ROMlib_sizergn(rh, false);
        }
    }
}

PUBLIC pascal trap void Executor::C_MapPoly(PolyHandle poly, Rect *srcr, Rect *dstr)
{
    GUEST<Point> *ip, *ep;

    if(EquivRect(srcr, dstr))
        OffsetPoly(poly, Cx(dstr->left) - Cx(srcr->left),
                   Cx(dstr->top) - Cx(srcr->top));
    else
    {
        MapPt((GUEST<Point> *)&HxX(poly, polyBBox.top), srcr, dstr);
        MapPt((GUEST<Point> *)&HxX(poly, polyBBox.bottom), srcr, dstr);
        for(ip = HxX(poly, polyPoints),
        ep = ip + (Hx(poly, polySize) - SMALLPOLY) / sizeof(Point);
            ip != ep;)
            MapPt(ip++, srcr, dstr);
    }
}
