/*
 * Copyright 1992 - 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: PSprint.c 87 2005-05-25 01:57:33Z ctm $
 */

#include "rsys/common.h"
#include <stdarg.h>

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_PSprint[] = "$Id: PSprint.c 87 2005-05-25 01:57:33Z ctm $";
#endif

#undef MACOSX_ /* no idea what's going on here... */

#ifdef MACOSX_
#import <Foundation/NSString.h>
#import <AppKit/NSFontManager.h>
#endif

#include "MemoryMgr.h"
#include "QuickDraw.h"
#include "OSEvent.h"
#include "FontMgr.h"
#include "rsys/nextprint.h"
#include "rsys/blockinterrupts.h"
#include "rsys/next.h"
#include "rsys/PSstrings.h"
#include "rsys/tempalloc.h"

#include "rsys/print.h"
#include "rsys/text.h"
#include "rsys/cquick.h"
#include "rsys/quick.h"

#include <ctype.h>

using namespace Executor;

typedef struct
{
    bool rotated_p;
    float center_x;
    float center_y;
    float angle;
} rotation_t;

PRIVATE rotation_t rotation;

PUBLIC FILE *ROMlib_printfile;
#define DPSContext long
#if !defined(MACOSX_)
typedef enum { NO,
               YES } boolean;
#endif

PRIVATE DPSContext DPSGetCurrentContext(void)
{
    return 0;
}

PRIVATE void DPSPrintf(DPSContext unused, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    if(ROMlib_printfile)
        vfprintf(ROMlib_printfile, format, ap);
    va_end(ap);
}

PRIVATE void DPSWritePostScript(DPSContext unused, Ptr bufp, int n)
{
    if(ROMlib_printfile)
        fwrite(bufp, n, 1, ROMlib_printfile);
}

PRIVATE void PSFontDirectory(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "FontDirectory\n");
}

PRIVATE void PSarc(float x, float y, float radius, float angle1, float angle2)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%f %f %f %f %f arc\n",
                x, y, radius, angle1, angle2);
}

PRIVATE void PSarcn(float x, float y, float radius, float angle1, float angle2)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%f %f %f %f %f arcn\n",
                x, y, radius, angle1, angle2);
}

PRIVATE void PSarct(float x1, float y1, float x2, float y2, float radius)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%f %f %f %f %f arcto pop pop pop pop\n",
                x1, y1, x2, y2, radius);
}

PRIVATE void PSbegin(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "begin\n");
}

PRIVATE void PSclip(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "clip\n");
}

PRIVATE void PSclippath(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "clippath\n");
}

PRIVATE void PSclosepath(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "closepath\n");
}

PRIVATE void PScurrentdict(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "currentdict\n");
}

PRIVATE void PSdefinefont(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "definefont\n");
}

PRIVATE void PSdiv(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "div\n");
}

PRIVATE void PSdup(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "dup\n");
}

PRIVATE void PSend(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "end\n");
}

PRIVATE void PSexch(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "exch\n");
}

PRIVATE void PSfill(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "fill\n");
}

PRIVATE void PSget(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "get\n");
}

PRIVATE void PSgrestore(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "grestore\n");
}

PRIVATE void PSgsave(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "gsave\n");
}

PRIVATE void PSimage(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "image\n");
}

PRIVATE void PScolorimage(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "colorimage\n");
}

PRIVATE void PSimagemask(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "imagemask\n");
}

PRIVATE void PSindex(int n)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%d index\n", n);
}

PRIVATE void PSlineto(float x, float y)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%f %f lineto\n", x, y);
}

PRIVATE void PSmoveto(float x, float y)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%f %f moveto\n", x, y);
}

PRIVATE void PSmul(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "mul\n");
}

PRIVATE void PSnewpath(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "newpath\n");
}

PRIVATE void PSpop(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "pop\n");
}

PRIVATE void PSrectclip(float x, float y, float width, float height)
{
    if(ROMlib_printfile)
    {
#if 0
      /* rectclip is level 2, believe it or not */
      fprintf(ROMlib_printfile, "%f %f %f %f rectclip\n", x, y, width, height);
#else
        fprintf(ROMlib_printfile, ("newpath\n"
                                   "%f %f moveto\n"
                                   "%f 0 rlineto\n"
                                   "0 %f rlineto\n"
                                   "%f neg 0 rlineto\n"
                                   "closepath\n"
                                   "clip\n"
                                   "newpath\n"),
                x, y, width, height, width);
#endif
    }
}

PRIVATE void PSrlineto(float x, float y)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%f %f rlineto\n", x, y);
}

PRIVATE void PSroll(int n, int j)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%d %d roll\n", n, j);
}

PRIVATE void PSrotate(float angle)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%f rotate\n", angle);
}

PRIVATE void PSscale(float sx, float sy)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%f %f scale\n", sx, sy);
}

PRIVATE void PSsendboolean(Executor::Boolean flag)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%s\n", flag ? "true" : "false");
}

PRIVATE void output_quoted_character(unsigned char c)
{
    if(isprint(c))
    {
        switch(c)
        {
            case '(':
            case ')':
            case '\\':
                if(ROMlib_printfile)
                    fprintf(ROMlib_printfile, "\\");
            /* FALL THROUGH */
            default:
                if(ROMlib_printfile)
                    fprintf(ROMlib_printfile, "%c", c);
        }
    }
    else if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "\\%03o", c);
}

PRIVATE void PSsendchararray(const char *stringp, int size)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "(");
    while(--size >= 0)
        output_quoted_character(*stringp++);
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, ")\n");
}

PRIVATE void PSsendfloat(float value)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%f\n", value);
}

PRIVATE void PSsendfloatarray(const float *arrayp, int size)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "[\n");
    while(--size >= 0)
        if(ROMlib_printfile)
            fprintf(ROMlib_printfile, "%f\n", *arrayp++);

    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "]\n");
}

PRIVATE void PSsendint(int value)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%d\n", value);
}

PRIVATE void PSsetgray(float num)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%f setgray\n", num);
}

PRIVATE void PSstroke(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "stroke\n");
}

PRIVATE void PSsub(void)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "sub\n");
}

PRIVATE void PStranslate(float x, float y)
{
    if(ROMlib_printfile)
        fprintf(ROMlib_printfile, "%f %f translate\n", x, y);
}

typedef struct
{
    const char *macfontname;
    const char *nextfontname;
    const char *suffix[4];
} fontentry_t;

static fontentry_t fonttable[] = {
    { "Avant Garde", "AvantGarde", { "-Book", "-Demi", "-BookOblique", "-DemiOblique" } },

    { "Bookman", "Bookman", { "-Light", "-Demi", "-LightItalic", "-DemiItalic" } },

    { "Chicago", "Chicago", { "-Roman", "-Bold", "-Italic", "-BoldItalic" } },

    { "Courier", "Courier", { "", "-Bold", "-Oblique", "-BoldOblique" } },

    { "Garamond", "Garamond", { "-Light", "-Bold", "-LightItalic", "-BoldItalic" } },

    { "Geneva", "Geneva", { "", "-Bold", "-Oblique", "-BoldOblique" } },

    { "Helvetica", "Helvetica", { "", "-Bold", "-Oblique", "-BoldOblique" } },

    { "Lubalin Graph", "LubalinGraph", { "-Book", "-Demi", "-BookOblique", "-DemiOblique" } },

    { "Monaco", "Monaco", { "", "-Bold", "-Oblique", "-BoldOblique" } },

    { "N Helvetica Narrow", "Helvetica-Narrow", { "", "-Bold", "-Oblique", "-BoldOblique" } },

    { "New Century Schlbk", "NewCenturySchlbk", { "-Roman", "-Bold", "-Italic", "-BoldItalic" } },

    { "New York", "NewYork", { "-Roman", "-Bold", "-Italic", "-BoldItalic" } },

    { "Optima", "Optima", { "", "-Bold", "-Oblique", "-BoldOblique" } },

    { "Palatino", "Palatino", { "-Roman", "-Bold", "-Italic", "-BoldItalic" } },

    { "Souvenir", "Souvenir", { "-Light", "-Demi", "-LightItalic", "-DemiItalic" } },

    { "Symbol", "Symbol", { "", "", "", "" } },

    { "Times", "Times", { "-Roman", "-Bold", "-Italic", "-BoldItalic" } },

    { "Zapf Chancery", "ZapfChancery-MediumItalic", { "", "", "", "" } },

    { "Zapf Dingbats", "ZapfDingbats", { "", "", "", "" } },
};

GrafPort Executor::printport;

#define patCopy 8

/*
 * NOTE: for now we use alloca for a temporary buffer, even though
 *	 sending big char arrays no longer works... We should just
 *	 blast out the hex without an intermediate buffer.
 */

#define DONTSENDBIGARRAYS

static void ourimageproc(int numbytes)
{
    DPSPrintf(DPSGetCurrentContext(),
              "{ currentfile %d string readhexstring pop }\n", numbytes);
}

static void dumpimage(unsigned char *p, int numrows, int numbytes,
                      int rowbytes)
{
    DPSContext context;
    int i, j;
    int toskip;

    toskip = rowbytes - numbytes;
    context = DPSGetCurrentContext();
    for(i = 0; i < numrows; ++i)
    {
        for(j = 0; j < numbytes; ++j)
            DPSPrintf(context, "%02X", *p++);
        DPSPrintf(context, "\n");
        p += toskip;
    }
}

static void
dumpcolorimage(unsigned char *p, int numrows, int numbytes,
               int rowbytes, int pixelsize)
{
    DPSContext context;
    int i, j;
    int toskip;

    /* TODO: split up values into RGB(and ignored) components and pump them out
     in the right order */

    toskip = rowbytes - numbytes;
    context = DPSGetCurrentContext();
    for(i = 0; i < numrows; ++i)
    {
        for(j = 0; j < numbytes;)
        {
            int r, g, b;
            switch(pixelsize)
            {
                case 16:
                    r = (p[0] << 1) & 0xF8;
                    g = ((p[0] << 6) | (p[1] >> 2)) & 0xF8;
                    b = (p[1] << 3); /* 5 bits of b */
                    p += 2;
                    j += 2;
                    break;
                default:
                case 24:
                    warning_unexpected("pixelsize %d", pixelsize);
                /* fall through */
                case 32:
                    if(pixelsize == 32)
                    {
                        ++p;
                        ++j;
                    }
                    r = *p++;
                    g = *p++;
                    b = *p++;
                    j += 3;
                    break;
            }
            DPSPrintf(context, "%02X%02X%02X", r, g, b);
        }
        DPSPrintf(context, "\n");
        p += toskip;
    }
}

PRIVATE float
mac_old_color_to_ps_gray(long color)
{
    float retval;

    switch(color)
    {
        case blackColor:
            retval = 0;
            break;
        case whiteColor:
            retval = 1.0;
            break;
        default:
            /* really not sure what to do here */
            warning_unexpected("%ld", color);
            retval = 1.0;
            break;
    }

    return retval;
}

PRIVATE bool
graymatch(unsigned char patp[8], INTEGER pnMode,
          GrafPtr thePortp, float *grayp)
{
    uint32 *pl;
    bool pat_is_black;
    bool pat_is_white;
    bool retval;
    float gray_fore, gray_back;

    int i, j;
    static struct
    {
        float graylevel;
        unsigned pat[8];
    } table[] = {
        { 0.0000, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } },
        { 0.2500, { 0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD } },
        { 0.5000, { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 } },
        { 0.7500, { 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22 } },
        { 0.9375, { 0x80, 0x00, 0x08, 0x00, 0x80, 0x00, 0x08, 0x00 } },
        { 1.0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    };

    gray_fore = mac_old_color_to_ps_gray(CL(thePortp->fgColor));
    gray_back = mac_old_color_to_ps_gray(CL(thePortp->bkColor));

    pl = (uint32 *)patp;
    pat_is_black = ((gray_fore == 0 && gray_back == 0) || (gray_fore == 0 && pl[0] == -1 && pl[1] == -1) || (gray_back == 0 && pl[0] == 0 && pl[1] == 0));

    pat_is_white = ((gray_fore == 1 && gray_back == 1) || (gray_fore == 1 && pl[0] == -1 && pl[1] == -1) || (gray_back == 1 && pl[0] == 0 && pl[1] == 0));

#if 0
#warning DO NOT CHECK THIS IN
  {
    long fg, bk;
    const char *fgs, *bks;

    fg = CL (thePortp->fgColor);
    bk = CL (thePortp->bkColor);
    if (fg == whiteColor)
      fgs = "white";
    else if (fg == blackColor)
      fgs = "black";
    else
      fgs = "unknown";
    if (bk == whiteColor)
      bks = "white";
    else if (bk == blackColor)
      bks = "black";
    else
      bks = "unknown";
    fprintf (stderr, "gray_fore = %f, gray_back = %f, fg = %s(%ld), bk = %s(%ld), is_black = %s, is_white = %s\n", gray_fore, gray_back, fgs, fg, bks, bk, pat_is_black ? "true" : "false", pat_is_white ? "true" : "false");
  }
#endif

    retval = false;

    if(((pnMode == patOr || pnMode == patCopy) && pat_is_black) || ((pnMode == notPatOr || pnMode == notPatCopy) && pat_is_white))
    {
        retval = true;
        *grayp = gray_fore;
    }
    else if(((pnMode == patBic || pnMode == notPatCopy) && pat_is_black) || ((pnMode == notPatBic || pnMode == patCopy) && pat_is_white))
    {
        retval = true;
        *grayp = gray_back;
    }
    else if(pnMode == patCopy || pnMode == notPatCopy)
    {
        if(gray_fore == gray_back)
        {
            retval = true;
            *grayp = gray_fore;
        }
        else
        {
            for(i = 0; !retval && i < (int)NELEM(table); ++i)
            {
                float darkness;

                for(j = 0; j < 8; ++j)
                {
                    if(table[i].pat[j] != patp[j])
                        /*-->*/ goto CONT;
                }
                darkness = table[i].graylevel;
                if(pnMode == notPatCopy)
                    darkness = 1.0 - darkness;

                *grayp = (darkness * gray_fore) + ((1 - darkness) * gray_back);
                retval = true;
            CONT:;
            }
        }
    }
    return retval;
}

static void doimage(LONGINT verb, Rect *rp, GrafPtr thePortp)
{
    unsigned char *patp, pat[8], c, *bytes, *p;
    int rowbytes, i, j, toshift, numbytesneeded;
    float gray_val;
    int numrows;
    float matrix[6];
    short pnMode;
    TEMP_ALLOC_DECL(temp_alloc_space);

    pnMode = CW(thePortp->pnMode);
    switch(verb)
    {
        default:
        case paintVerb:
            patp = thePortp->pnPat;
            break;
        case eraseVerb:
            patp = thePortp->bkPat;
            pnMode = patCopy;
            break;
        case fillVerb:
            patp = thePortp->fillPat;
            pnMode = patCopy;
            break;
    }

    /* convert erroneous srcXXX to patXXX */

    if(pnMode >= srcCopy && pnMode <= notSrcBic)
        pnMode |= 8;

    if(graymatch(patp, pnMode, thePortp, &gray_val))
    {
        PSclippath();
        PSsetgray(gray_val);
        PSfill();
    }
    else
    {
        PSmoveto(CW(rp->left), CW(rp->bottom));
        PSlineto(CW(rp->left), CW(rp->top));
        PSlineto(CW(rp->right), CW(rp->top));
        PSlineto(CW(rp->right), CW(rp->bottom));
        PSclosepath();
        PSmoveto(CW(rp->left) - 1, CW(rp->bottom) + 1);
        PSlineto(CW(rp->right) + 1, CW(rp->bottom) + 1);
        PSlineto(CW(rp->right) + 1, CW(rp->top) - 1);
        PSlineto(CW(rp->left) - 1, CW(rp->top) - 1);
        PSclip();
        rowbytes = ((CW(rp->right) - CW(rp->left)) * 72 + 72 * 8 - 1) / 72 / 8;
        numrows = ((CW(rp->bottom) - CW(rp->top)) * 72 + 71) / 72;
        if(rowbytes > 0 && numrows > 0)
        {
            numbytesneeded = rowbytes * numrows;
            TEMP_ALLOC_ALLOCATE(bytes, temp_alloc_space, numbytesneeded);
            toshift = (CW(rp->left) - CW(thePortp->portBits.bounds.left)) & 7;
            for(i = 0; i < 8; ++i)
                pat[i] = (patp[i] << toshift) | (patp[i] >> (8 - toshift));
            p = bytes;
            for(i = CW(rp->top) - CW(thePortp->portBits.bounds.top);
                i < CW(rp->top) + numrows - CW(thePortp->portBits.bounds.top);
                ++i)
            {
                c = pat[i & 7];
                if(pnMode == patCopy)
                    c = ~c;
                for(j = rowbytes; --j >= 0;)
                    *p++ = c;
            }
            PStranslate(CW(rp->left), CW(rp->top));
#if !defined(DONTSENDBIGARRAYS)
            PSsendchararray((char *)bytes, numbytesneeded);
#endif /* DONTSENDBIGARRAYS */
            PSsendint(rowbytes * 8);
            PSsendint(numrows);
            if(pnMode == patCopy)
                PSsendint(1);
            else
                PSsendboolean(YES);
            matrix[0] = 72 / (float)72;
            matrix[1] = 0;
            matrix[2] = 0;
            matrix[3] = -72 / (float)72;
            matrix[4] = 0;
            matrix[5] = numrows;
            PSsendfloatarray(matrix, 6);
#if !defined(DONTSENDBIGARRAYS)
            PSsendintarray((int *)0, 0);
            PScvx();
#else /* DONTSENDBIGARRAYS */
            ourimageproc(rowbytes);
#endif /* DONTSENDBIGARRAYS */
            if(pnMode == patCopy)
                PSimage();
            else
                PSimagemask();
#if defined(DONTSENDBIGARRAYS)
            dumpimage(bytes, numrows, rowbytes, rowbytes);
#endif
        }
    }

    TEMP_ALLOC_FREE(temp_alloc_space);
}

void Executor::ROMlib_gsave(void)
{
    virtual_int_state_t block;

    block = block_virtual_ints();
    PSgsave();
    restore_virtual_ints(block);
}

void Executor::ROMlib_grestore(void)
{
    virtual_int_state_t block;

    block = block_virtual_ints();
    PSgrestore();
    restore_virtual_ints(block);
}

char Executor::ROMlib_suppressclip = NO;
static char reloadclip = NO;

PRIVATE double save_xoffset, save_yoffset;

void Executor::ROMlib_rotatebegin(LONGINT flippage, LONGINT angle)
{
    virtual_int_state_t block;

    block = block_virtual_ints();
    PSrotate(angle);
    PStranslate(-save_xoffset, -save_yoffset);
    ROMlib_suppressclip = YES;
    printport.pnLoc.h = CWC(-32768); /* force reload */
    printport.txFont = CWC(-32768); /* force reload */
    restore_virtual_ints(block);
}

void Executor::ROMlib_rotatecenter(double yoffset, double xoffset)
{
    DPSContext context;
    virtual_int_state_t block;

    /*
 * TODO:  support flippage
 */

    block = block_virtual_ints();
    context = DPSGetCurrentContext();
    PSgsave();
    PStranslate(xoffset, yoffset);
    save_xoffset = xoffset;
    save_yoffset = yoffset;
    restore_virtual_ints(block);
}

void Executor::ROMlib_rotateend(void)
{
    virtual_int_state_t block;

    block = block_virtual_ints();
    PSgrestore();
    ROMlib_suppressclip = NO;
    reloadclip = YES;
    restore_virtual_ints(block);
}

static void NeXTClip(Rect *rp)
{
    virtual_int_state_t block;

    block = block_virtual_ints();
    /* we used to do a `grestore' `gsave' sequence here, to get the clip
     path before the previous call to `NeXTClip ()', and then add the
     new `rectclip'.  but this causes serious problems, because the
     `grestore' `gsave' sequence undoes more than just the previous
     rectclip, it also undoes `translate's and other graphics state
     modifications that can't be undone here.
     
     as far as i can tell, the `grestore' `gsave' sequence just got
     you back to the intial (unbounded) clip path */
    DPSPrintf(DPSGetCurrentContext(), "initclip\n");
    if((rp->left != CWC(-32767) && rp->left != CWC(-32768)) || (rp->top != CWC(-32767) && rp->top != CWC(-32768)) || rp->right != CWC(32767) || rp->bottom != CWC(32767))
        PSrectclip(CW(rp->left), CW(rp->top),
                   CW(rp->right) - CW(rp->left), CW(rp->bottom) - CW(rp->top));
    restore_virtual_ints(block);
}

static int myEqualRect(Rect *r1, Rect *r2)
{
    return r1->top == r2->top && r1->bottom == r2->bottom && r1->left == r2->left && r1->right == r2->right;
}

/*
 * updates the fields that *all* drawing routines care about
 */

static void commonupdate(GrafPtr thePortp)
{
    int dx, dy;

    if(thePortp->portBits.bounds.top != printport.portBits.bounds.top || thePortp->portBits.bounds.left != printport.portBits.bounds.left)
    {
        dx = CW(printport.portBits.bounds.left) - CW(thePortp->portBits.bounds.left);
        dy = CW(printport.portBits.bounds.top) - CW(thePortp->portBits.bounds.top);
        PStranslate(dx, dy);
        printport.portBits.bounds = thePortp->portBits.bounds;
    }
    if(!ROMlib_suppressclip && (reloadclip || !myEqualRect(&MR(*MR(thePortp->clipRgn))->rgnBBox, &MR(*MR(printport.clipRgn))->rgnBBox)))
    {
        NeXTClip(&MR(*MR(thePortp->clipRgn))->rgnBBox);
        MR(*MR(printport.clipRgn))->rgnBBox = MR(*MR(thePortp->clipRgn))->rgnBBox;
        printport.pnLoc.h = CWC(-32768); /* force reload */
        printport.txFont = CWC(-32768); /* force reload */
    }

    if(thePortp->pnLoc.h != printport.pnLoc.h || thePortp->pnLoc.v != printport.pnLoc.v)
    {
        PSmoveto(CW(thePortp->pnLoc.h), CW(thePortp->pnLoc.v));
        printport.pnLoc = thePortp->pnLoc;
    }

#if 0
    if (thePortp->fgColor != printport.fgColor) {
    }

    if (thePortp->bkColor != printport.bkColor) {
    }
#endif
}

#define bold 1
#define italic 2
#define underline 4
#define outline 8

/*
 * TODO: tons of this better... (pay attention to spExtra)
 */

/*
 * TODO: Barlow wants more fonts
 */

/*
 * findprefered looks for a match in fonttable and constructs the
 * appropriate fontname.  If no match is found, fname is copied
 * over to retval.
 */

static char match255c(StringPtr str, char *p)
{
    int n;

    n = strlen(p);
    if(n != str[0])
        return NO;
    else
        return strncmp((char *)str + 1, p, n) == 0;
}

PRIVATE void
substitute_chars(char *string, char find, char replace)
{
    while(*string)
    {
        if(*string == find)
            *string = replace;
        ++string;
    }
}

static void findpreferred(StringPtr fname, int index, char *retval)
{
    int i;

    for(i = 0; i < (int)NELEM(fonttable); ++i)
    {
        if(match255c(fname, (char *)fonttable[i].macfontname))
            /*-->*/ break;
    }
    if(i < (int)NELEM(fonttable))
        sprintf(retval, "%s%s", fonttable[i].nextfontname,
                fonttable[i].suffix[index]);
    else
    {
        strncpy(retval, (char *)fname + 1, (unsigned char)fname[0]);
        retval[(unsigned char)fname[0]] = 0;
        substitute_chars(retval, ' ', '-');
    }
}

#if defined(MACOSX_)

/*
 * Trytomatch looks at the fonts that are actually present and verifies
 * that what we're trying to do is legit.  If it isn't, a substitution
 * is made.
 */

static const char *normals[] = {
    "UltraLight",
    "Thin",
    "Light",
    "ExtraLight",
    "Book",
    "Regular",
    "Plain",
    "Roman"
    "Medium",
};

static const char *bolds[] = { "Demi",
                               "DemiBold",
                               "SemiBold",
                               "Bold",
                               "ExtraBold",
                               "Heavy",
                               "Heavyface",
                               "Black",
                               "Ultra",
                               "UltraBlack",
                               "Fat",
                               "ExtraBlack",
                               "Obese" };
static const char *italics[] = { "Italic", "Oblique" };

static int lookfor(char *str, char *table[], int tablelen, char startatleft)
{
    int i, retval, n1, n2;
    char *start;

    retval = 0;
    n1 = strlen(str);
    start = str;
    for(i = 0; i < tablelen; ++i)
    {
        n2 = strlen(table[i]);
        if(n1 >= n2)
        {
            if(!startatleft)
                start = str + n1 - n2;
            if(strncmp(table[i], start, n2) == 0)
            {
                retval = 1;
                break;
            }
        }
    }
    return retval;
}

#if 0
static int ndashes(char *name)
{
    int retval;

    retval = 0;
    while (*name)
	if (*name++ == '-')
	    ++retval;
    return retval;
}
#endif

static int matchpercentage(char *value, int indx, const char *tomatch)
{
    char *dash, isnormal, isbold, isitalic;
    int retval;

    dash = rindex(tomatch, '-');
    retval = 0;
    if((dash && strncmp(value, tomatch, dash - tomatch) == 0) || (!dash && strcmp(value, tomatch) == 0))
    {
        ++dash;
        isnormal = lookfor(dash, (char **)normals, NELEM(normals), YES);
        isbold = lookfor(dash, (char **)bolds, NELEM(bolds), YES);
        isitalic = lookfor(dash, (char **)italics, NELEM(italics), NO);
        switch(indx)
        {
            case 0:
                if(isnormal)
                    retval += 50;
                if(!isbold)
                    retval += 25;
                if(!isitalic)
                    retval += 25;
                break;
            case bold:
                if(isbold)
                    retval += 50;
                if(!isnormal)
                    retval += 25;
                if(!isitalic)
                    retval += 25;
                break;
            case italic:
                if(isitalic)
                    retval += 75;
                if(!isbold)
                    retval += 25;
                break;
            case bold | italic:
                if(isitalic)
                    retval += 75;
                if(isbold)
                    retval += 25;
                break;
        }
    }
    return retval;
}

void Executor::ROMlib_trytomatch(char *retval, LONGINT index)
{
    @autoreleasepool
    {
        const char *bestp;
        int bestn;
        int n;
        NSArray *font_list;
        int i;
        virtual_int_state_t block;

        block = block_virtual_ints();
        font_list = [[NSFontManager sharedFontManager] availableFonts];
        bestn = 0;

#if !defined(LETGCCWAIL)
        bestp = 0;
#endif
        for(i = 0; i < [font_list count] && bestn != 100; ++i)
        {
            const char *font_name;

            font_name = [[font_list objectAtIndex:i] cStringUsingEncoding:NSMacOSRomanStringEncoding];
            n = matchpercentage(retval, index, font_name);
            if(n > bestn)
            {
                bestn = n;
                bestp = font_name;
            }
        }
        if(bestn > 0)
            strcpy(retval, bestp);
        else
        {
            switch(index)
            {
                default:
                    strcpy(retval, "Times-Roman");
                    break;
                case bold:
                    strcpy(retval, "Times-Bold");
                    break;
                case italic:
                    strcpy(retval, "Times-Italic");
                    break;
                case bold | italic:
                    strcpy(retval, "Times-BoldItalic");
                    break;
            }
        }
        restore_virtual_ints(block);
    }
}
#endif

static char *fnametofont(StringPtr fname, LONGINT txFace)
{
    int index;
    static char retval[80];

    index = txFace & (bold | italic);
    findpreferred(fname, index, retval);
#ifdef MACOSX_
    ROMlib_trytomatch(retval, index);
#endif
    if(txFace & outline)
        strcat(retval, "-Outline");

    if(txFace & outline)
    {
        PSFontDirectory();
        PSsendchararray(retval, strlen(retval));
        DPSPrintf(DPSGetCurrentContext(), "known not {\n");
        PSsendchararray(retval, strlen(retval));
        PSsendchararray(retval, strlen(retval) - 8);
        DPSPrintf(DPSGetCurrentContext(), "findfont2\n");
        DPSPrintf(DPSGetCurrentContext(), "dup length 1 add dict\n");
        PSbegin();
        DPSPrintf(DPSGetCurrentContext(), "{ 1 index /FID ne {def} {pop pop} ifelse } forall\n");
        DPSPrintf(DPSGetCurrentContext(), "(PaintType) 2 def\n");
        DPSPrintf(DPSGetCurrentContext(), "(StrokeWidth) 0 def\n");
        PScurrentdict();
        PSend();
        PSdefinefont();
        PSpop();
        DPSPrintf(DPSGetCurrentContext(), "} \nif\n");
    }
    return retval;
}

typedef struct
{
    const char *old;
    const char *new1;
    float multiplier;
} substitute_t;

PUBLIC bool Executor::substitute_fonts_p = false;

PRIVATE float
substitute_font_if_needed(char **fontp, LONGINT orig_size,
                          bool *need_to_freep)
{
    float retval;
    static substitute_t substitutions[] = {
        {
            "Geneva", "Helvetica", 1.0,
        },
        {
            "Monaco", "Courier", 1.0,
        },
        {
            "New York", "Times", 1.0,
        },
    };

    retval = orig_size;
    *need_to_freep = false;
    if(substitute_fonts_p)
    {
        int i;
        char *font;

        font = *fontp;
        for(i = 0; i < (int)NELEM(substitutions); ++i)
        {
            int len;

            len = strlen(substitutions[i].old);
            if(strncmp(substitutions[i].old, font, len) == 0)
            {
                char *newname;

                newname = (char *)malloc(strlen(font) - len + strlen(substitutions[i].new1) + 1);
                sprintf(newname, "%s%s", substitutions[i].new1, font + len);
                *fontp = newname;
                *need_to_freep = true;
                retval = orig_size * substitutions[i].multiplier;
                /*-->*/ break;
            }
        }
    }

    return retval;
}

void NeXTSetText(StringPtr fname, LONGINT txFace, LONGINT txSize,
                 LONGINT spExtra)
{
    char *font;
    float matrix[6];
    virtual_int_state_t block;
    float font_size;
    bool need_to_free;

    block = block_virtual_ints();
    font = fnametofont(fname, txFace);
    if(txSize < 0)
        txSize = 1;
    else if(txSize == 0)
        txSize = 12;

    font_size = substitute_font_if_needed(&font, txSize, &need_to_free);

#ifdef MACOSX_
    ROMlib_newFont(font, font_size);
#endif
    PSsendchararray(font, strlen(font));
    matrix[0] = font_size;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = -font_size;
    matrix[4] = 0;
    matrix[5] = 0;
    PSsendfloatarray(matrix, 6);
    DPSPrintf(DPSGetCurrentContext(), "selectfont\n");
    restore_virtual_ints(block);
    if(need_to_free)
        free(font);
}

PRIVATE void
SmartGetFontName(GrafPtr thePortp, StringPtr fname)
{
    C_GetFontName(CW(thePortp->txFont), fname);
    if(!fname[0])
        C_GetFontName(CW(ApFontID), fname);
    if(!fname[0])
        C_GetFontName(CW(SysFontFam), fname);
}

/*
 * updates the fields that text routines care about
 */

static void txupdate(GrafPtr thePortp)
{
    unsigned char fname[256];

    SETUPA5;
    commonupdate(thePortp);
    if(thePortp->txMode != printport.txMode)
    {
    }
    if(thePortp->txFont != printport.txFont || thePortp->txFace != printport.txFace || thePortp->txSize != printport.txSize || thePortp->spExtra != printport.spExtra)
    {
        SmartGetFontName(thePortp, fname);
        NeXTSetText(fname, thePortp->txFace, CW(thePortp->txSize),
                    CL(thePortp->spExtra));
        printport.txFont = thePortp->txFont;
        printport.txFace = thePortp->txFace;
        printport.txSize = thePortp->txSize;
    }
    RESTOREA5;
}

void pnupdate(GrafPtr thePortp)
{
    commonupdate(thePortp);
#if 0
    if (thePortp->pnSize.h != printport.pnSize.h ||
	thePortp->pnSize.v != printport.pnSize.v) {
	NeXTSetWidth(CW(thePortp->pnSize));
	printport.pnSize = thePortp->pnSize;
    }
#endif
    /* TODO: more stuff here */
}

void Executor::NeXTPrArc(LONGINT verb, Rect *rp, LONGINT starta, LONGINT arca,
                         GrafPtr thePortp)
{
    if(rp->left != rp->right && rp->top != rp->bottom) /* ignore empty rectangles */
    {
        short psh, psv;
        float xdiam, ydiam, midx, midy;
        LONGINT froma, toa;
        virtual_int_state_t block;

        block = block_virtual_ints();
        pnupdate(thePortp);
        PSgsave();
        if(arca > 0)
        {
            froma = -90 + starta;
            toa = -90 + starta + arca;
        }
        else
        {
            froma = -90 + starta + arca;
            toa = -90 + starta;
        }
        midx = ((float)CW(rp->left) + CW(rp->right)) / 2;
        midy = ((float)CW(rp->top) + CW(rp->bottom)) / 2;
        xdiam = CW(rp->right) - CW(rp->left);
        ydiam = CW(rp->bottom) - CW(rp->top);
        PStranslate(midx, (midy));
        PSnewpath();
        PSscale(1, ydiam / xdiam);
        PSarc(0, 0, xdiam / 2, froma, toa);

        if(verb == frameVerb)
        {
            psh = CW(thePortp->pnSize.h);
            psv = CW(thePortp->pnSize.v);
            if(ydiam > (2 * psv) && xdiam > (2 * psh))
            {
                PSscale(1, xdiam / ydiam * (ydiam - 2 * psv) / (xdiam - 2 * psh));
                PSarcn(0, 0, xdiam / 2 - psh, toa, froma);
                PSclosepath();
                PSscale(1, ydiam / xdiam * (xdiam - 2 * psh) / (ydiam - 2 * psv));
            }
        }
        else
            PSlineto(0, 0);
        PSclosepath();
        PSclip();
        PSscale(1, xdiam / ydiam);
        PStranslate(-midx, -(midy));

        doimage(verb, rp, thePortp);
        PSgrestore();
        restore_virtual_ints(block);
    }
}

PRIVATE int
num_image_bytes(int numbytes, int pixelsize, int direct_color_p)
{
    int retval;

    if(!direct_color_p)
        retval = numbytes;
    else
    {
        switch(pixelsize)
        {
            case 16:
                retval = numbytes / 2 * 3;
                break;
            default:
            case 24:
                retval = numbytes;
                break;
            case 32:
                retval = numbytes / 4 * 3;
                break;
        }
    }

    return retval;
}

#define srcCopy 0
#define srcBic 3

#if !defined(ROWMASK)
#define ROWMASK 0x1FFF
#endif

void Executor::NeXTPrBits(BitMap *srcbmp, Rect *srcrp, Rect *dstrp,
                          LONGINT mode, RgnHandle mask, GrafPtr thePortp)
{
    float scalex, scaley;
    float srcwidth, srcheight, dstwidth, dstheight;
    float matrix[6];
    unsigned char *p1, *p2, *ep, *bytes, *baseaddr;
    int numbytesneeded;
    PixMap *srcpmp;
    short rowbytes, numbytes, pixelsize;
    virtual_int_state_t block;
    bool direct_color_p;
    bool indexed_color_p;
    TEMP_ALLOC_DECL(temp_alloc_space);

    direct_color_p = false;
    indexed_color_p = false;
    block = block_virtual_ints();
    commonupdate(thePortp);
    srcwidth = CW(srcrp->right) - CW(srcrp->left);
    srcheight = CW(srcrp->bottom) - CW(srcrp->top);
    dstwidth = CW(dstrp->right) - CW(dstrp->left);
    dstheight = CW(dstrp->bottom) - CW(dstrp->top);
    if(srcwidth && srcheight && dstwidth && dstheight) /* put in for output */
    { /* from Tex-Edit 2.5 */
        /* see the comment at the grestore below */
        PSgsave();

        scalex = dstwidth / srcwidth;
        scaley = dstheight / srcheight;

        if(CW(srcbmp->rowBytes) & 0x8000)
        {
            srcpmp = (PixMap *)srcbmp;
            pixelsize = CW(srcpmp->pixelSize);
            if(pixelsize != 1 && mode != srcCopy)
                /*-->*/ goto DONE;
            direct_color_p = pixelsize > 8;
            if(!direct_color_p)
                indexed_color_p = true;
        }
        else
        {
            srcpmp = 0;
            pixelsize = 1;
        }
        rowbytes = CW(srcbmp->rowBytes) & ROWMASK;

        PStranslate(CW(dstrp->left), CW(dstrp->top));
        baseaddr = (unsigned char *)MR(srcbmp->baseAddr)
            + (CW(srcrp->top) - CW(srcbmp->bounds.top)) * (LONGINT)rowbytes;

        /* NOTE: now that we don't send big arrays, I believe that doing
	 the transformation below is a memory waste... Shouldn't we
	 be able to do that on the fly? -- I don't want to mess with
	 this now, but it does make sense to fix it sometime.  */

        if(mode == srcCopy && !direct_color_p && !indexed_color_p)
        {
            numbytesneeded = rowbytes * srcheight;
            TEMP_ALLOC_ALLOCATE(bytes, temp_alloc_space, numbytesneeded);
            ep = bytes + numbytesneeded;
            for(p1 = bytes, p2 = baseaddr; p1 < ep;)
                *p1++ = ~*p2++;
            baseaddr = bytes;
        }
#if !defined(DONTSENDBIGARRAYS)
        PSsendchararray((char *)baseaddr, rowbytes * srcheight);
#endif
        matrix[0] = 1 / scalex;
        matrix[1] = 0;
        matrix[2] = 0;
        matrix[3] = 1 / scaley;
        matrix[4] = CW(srcrp->left) - CW(srcbmp->bounds.left);
        matrix[5] = 0;

        numbytes = ((int)srcwidth * pixelsize + 7) / 8;
        if(!indexed_color_p)
        {
            PSsendint((int)srcwidth);
            PSsendint(srcheight);
            if(mode != srcCopy)
                PSsendboolean(YES);
            else
            {
                if(direct_color_p)
                    PSsendint(8);
                else
                    PSsendint(pixelsize);
            }
            PSsendfloatarray(matrix, 6);
#if !defined(DONTSENDBIGARRAYS)
            PSsendintarray((int *)0, 0);
            PScvx();
#else
            ourimageproc(num_image_bytes(numbytes, pixelsize, direct_color_p));
#endif
            if(mode == srcBic)
                PSsetgray(1);
            if(mode != srcCopy)
                PSimagemask();
            else
            {
                if(!direct_color_p)
                    PSimage();
                else
                {
                    PSsendboolean(false);
                    PSsendint(3);
                    PScolorimage();
                }
            }
        }
        else
        {
            Executor::ColorSpec *ctab;
            int i;
            GUEST<PixMapPtr> pxp;
            bool has_warned_p;

            pxp = RM((Executor::PixMapPtr)srcpmp);
            DPSPrintf(DPSGetCurrentContext(),
                      "[/Indexed /DeviceRGB %d <\n", (1 << pixelsize) - 1);

            ctab = CTAB_TABLE(PIXMAP_TABLE(&pxp));

            /* NOTE: we're ignoring the value field below.  We could do
	     a quick test to see if it's sorted and if it is use it
	     directly and if not, copy and qsort it and then use
	     the result.  It's not clear when that's necessary.  TODO
	     look into this further. */

            has_warned_p = false;
            for(i = 0; i < (1 << pixelsize); ++i)
            {
                unsigned char r;
                unsigned char g;
                unsigned char b;

                if(CW(ctab[i].value) != i && !has_warned_p)
                {
                    warning_unexpected("value = %d, i = %d",
                                       CW(ctab[i].value), i);
                    has_warned_p = true;
                }
                r = CW(ctab[i].rgb.red) >> 8;
                g = CW(ctab[i].rgb.green) >> 8;
                b = CW(ctab[i].rgb.blue) >> 8;
                DPSPrintf(DPSGetCurrentContext(),
                          "%02x%02x%02x%c", r, g, b, (i % 8) == 7 ? '\n' : ' ');
            }

            DPSPrintf(DPSGetCurrentContext(),
                      ">]\nsetcolorspace\n<<\n/ImageType 1\n/Width %d\n"
                      "/Height %d\n"
                      "/BitsPerComponent %d\n/Decode [0 %d]\n"
                      "/ImageMatrix [%f %f %f %f %f %f]\n"
                      "/DataSource currentfile /ASCIIHexDecode filter\n"
                      ">>\nimage\n",
                      (int)srcwidth, (int)srcheight, pixelsize,
                      (1 << pixelsize) - 1,
                      matrix[0], matrix[1], matrix[2],
                      matrix[3], matrix[4], matrix[5]);
        }
#if defined(DONTSENDBIGARRAYS)
        if(direct_color_p)
            dumpcolorimage(baseaddr, srcheight, numbytes, rowbytes, pixelsize);
        else
            dumpimage(baseaddr, srcheight, numbytes, rowbytes);
#endif
    /* #### this grestore used to be commented out with the
	 following comment:
	 
	 `This seems to mess up Compact Pro's output'
	 
	 but there was no corresponding `gsave' earlier in this
	 function.  i added it, and uncommented this grestore, because
	 it is clear someone has to undo the `translate' done to set
	 the location for the `image' command */
    DONE:
        PSgrestore();
    }
    restore_virtual_ints(block);

    TEMP_ALLOC_FREE(temp_alloc_space);
}

void Executor::NeXTPrLine(Point to, GrafPtr thePortp)
{
    float temp, fromh, fromv, toh, tov;
    short psh, psv;
    Rect r;
    virtual_int_state_t block;

    block = block_virtual_ints();
    pnupdate(thePortp);
    if(CW(thePortp->pnSize.h) || CW(thePortp->pnSize.v))
    {
        fromh = CW(thePortp->pnLoc.h);
        fromv = CW(thePortp->pnLoc.v);
        toh = to.h;
        tov = to.v;

        if(fromh > toh)
        {
            temp = fromh;
            fromh = toh;
            toh = temp;
            temp = fromv;
            fromv = tov;
            tov = temp;
        }
        psh = CW(thePortp->pnSize.h);
        psv = CW(thePortp->pnSize.v);
        r.right = CW(toh + psh);
        r.left = CW(fromh);
        PSgsave();
        PSnewpath();
        PSmoveto(fromh, fromv);
        if(fromv < tov)
        {
            r.top = CW(fromv);
            r.bottom = CW(tov + psv);
            PSlineto(fromh + psh, fromv);
            PSlineto(toh + psh, tov);
            PSlineto(toh + psh, tov + psv);
            PSlineto(toh, tov + psv);
            PSlineto(fromh, fromv + psv);
        }
        else
        {
            r.top = CW(tov);
            r.bottom = CW(fromv + psv);
            PSlineto(toh, tov);
            PSlineto(toh + psh, tov);
            PSlineto(toh + psh, tov + psv);
            PSlineto(fromh + psh, fromv + psv);
            PSlineto(fromh, fromv + psv);
        }
        PSclosepath();
        PSclip();

        doimage((LONGINT)paintVerb, &r, thePortp);
        PSgrestore();
    }
    restore_virtual_ints(block);
}

void Executor::NeXTPrOval(LONGINT verb, Rect *rp, GrafPtr thePortp)
{
    NeXTPrArc(verb, rp, 0, 360, thePortp);
}

#if 1
void Executor::NeXTPrGetPic(Ptr dp, LONGINT bc, GrafPtr thePortp)
{
    gui_abort();
}
#endif

void Executor::NeXTPrPutPic(Ptr sp, LONGINT bc, GrafPtr thePortp)
{
}

void Executor::NeXTPrPoly(LONGINT verb, PolyHandle ph, GrafPtr thePortp)
{
    GUEST<Point> *pp, *ep;
    Point firstp;
    virtual_int_state_t block;
    Point pt;

    block = block_virtual_ints();
    pnupdate(thePortp);
    pp = MR(*ph)->polyPoints;
    ep = (GUEST<Point> *)((char *)MR(*ph) + CW((MR(*ph))->polySize));
    firstp.h = CW(pp[0].h);
    firstp.v = CW(pp[0].v);
    thePortp->pnLoc = pp[0];
    if(CW(ep[-1].h) == firstp.h && CW(ep[-1].v) == firstp.v)
        ep--;
    if(ep > pp)
    {
        if(verb == frameVerb)
        {
            PSmoveto(firstp.h, firstp.v);
            for(++pp; pp < ep; pp++)
            {
                pt.h = CW(pp[0].h);
                pt.v = CW(pp[0].v);
                NeXTPrLine(pt, thePortp);
                thePortp->pnLoc = pp[0];
            }
            NeXTPrLine(firstp, thePortp);
        }
        else
        {
            PSgsave();
            PSnewpath();
            PSmoveto(firstp.h, firstp.v);
            for(++pp; pp < ep; pp++)
            {
                PSlineto(CW(pp->h), CW(pp->v));
            }
            PSlineto(firstp.h, firstp.v);
            PSclosepath();
            PSclip();
            doimage(verb, &(MR(*ph))->polyBBox, thePortp);

            PSgrestore();
        }
    }
    restore_virtual_ints(block);
}

void Executor::NeXTPrRRect(LONGINT verb, Rect *rp, LONGINT width, LONGINT height,
                           GrafPtr thePortp)
{
    float sfactor, midy, rt, rb, rl, rr, sfactor2;
    short psh, psv;
    virtual_int_state_t block;

    if(width <= 0 || height <= 0)
        NeXTPrRect(verb, rp, thePortp);
    else
    {
        block = block_virtual_ints();
        pnupdate(thePortp);
        sfactor = (float)height / width;
        midy = ((float)CW(rp->top) + CW(rp->bottom)) / 2;

        PSgsave();
        PSnewpath();
        PSscale(1, sfactor);
        PSmoveto(CW(rp->left), midy / sfactor);
        rl = CW(rp->left);
        rr = CW(rp->right);
        rt = CW(rp->top) / sfactor;
        rb = CW(rp->bottom) / sfactor;
        PSarct(rl, rb, rr, rb, (float)width / 2);
        PSarct(rr, rb, rr, rt, (float)width / 2);
        PSarct(rr, rt, rl, rt, (float)width / 2);
        PSarct(rl, rt, rl, rb, (float)width / 2);
        PSclosepath();
        if(verb == frameVerb)
        {
            psh = CW(thePortp->pnSize.h);
            psv = CW(thePortp->pnSize.v);
            sfactor2 = ((float)height - 2 * psv) / (width - 2 * psh) / sfactor;
            rl = CW(rp->left) + psh;
            rr = CW(rp->right) - psh;
            rt = CW(rp->top) + psv / sfactor2;
            rb = CW(rp->bottom) - psv / sfactor2;

            PSscale(1, sfactor2);
            PSmoveto(rl, midy / sfactor);
            PSarct(rl, rt, rr, rt, (float)width / 2 - psh);
            PSarct(rr, rt, rr, rb, (float)width / 2 - psh);
            PSarct(rr, rb, rl, rb, (float)width / 2 - psh);
            PSarct(rl, rb, rl, rt, (float)width / 2 - psh);
            PSscale(1, 1 / sfactor2);
            PSclosepath();
        }
        PSscale(1, 1 / sfactor);
        PSclip();

        doimage(verb, rp, thePortp);
        PSgrestore();
        restore_virtual_ints(block);
    }
}

void Executor::NeXTPrRect(LONGINT verb, Rect *rp, GrafPtr thePortp)
{
    short psh, psv;
    virtual_int_state_t block;

    block = block_virtual_ints();
    pnupdate(thePortp);
    PSgsave();
    PSnewpath();
    PSmoveto(CW(rp->left), CW(rp->top));
    PSlineto(CW(rp->left), CW(rp->bottom));
    PSlineto(CW(rp->right), CW(rp->bottom));
    PSlineto(CW(rp->right), CW(rp->top));
    PSclosepath();
    if(verb == frameVerb)
    {
        psh = CW(thePortp->pnSize.h);
        psv = CW(thePortp->pnSize.v);
        PSmoveto(CW(rp->left) + psh, CW(rp->top) + psv);
        PSlineto(CW(rp->right) - psh, CW(rp->top) + psv);
        PSlineto(CW(rp->right) - psh, CW(rp->bottom) - psv);
        PSlineto(CW(rp->left) + psh, CW(rp->bottom) - psv);
        PSclosepath();
    }
    PSclip();

    doimage(verb, rp, thePortp);

    PSgrestore();
    restore_virtual_ints(block);
}

void Executor::NeXTPrRgn(LONGINT verb, RgnHandle rgn, GrafPtr thePortp)
{
    /* NOP */
}

short Executor::NeXTPrTxMeas(LONGINT n, Ptr p, GUEST<Point> *nump, GUEST<Point> *denp,
                             FontInfo *finfop, GrafPtr thePortp)
{
    GUEST<Point> num, den;
    virtual_int_state_t block;
    short retval;

    SETUPA5;
    block = block_virtual_ints();
    num.h = num.v = den.h = den.v = CWC(0x100);
    retval = ROMlib_StdTxMeas(n,
                              (Ptr)p, &num, &den, NULL);
    restore_virtual_ints(block);
    RESTOREA5;
    return (float)retval * CW(num.h) / CW(den.h);
}

static int numspacesin(const char *str)
{
    int retval;

    retval = 0;
    while(*str)
        if(*str++ == ' ')
            ++retval;

    return retval;
}

static void dopsunderline(GrafPtr thePortp, short total,
                          bool substitute_p, char *translated, LONGINT n)
{
    unsigned char fname[256];
    char *font;

    SETUPA5;

    /* If we are substituting fonts, then we need to use PS's idea of
       how long a string is rather than use "total".  So before we do
       a findfond we need to measure the string and leave that width
       on the stack */

    if(substitute_p)
    {
        PSsendchararray(translated, n);
        DPSPrintf(DPSGetCurrentContext(), "stringwidth pop\n");
    }

    SmartGetFontName(thePortp, fname);
    font = fnametofont(fname, thePortp->txFace);
    {
        bool need_to_free;

        substitute_font_if_needed(&font, 0, &need_to_free);
        PSgsave();
        PSnewpath();
        PSsendfloat(CW(thePortp->pnLoc.h));
        PSsendfloat(CW(thePortp->pnLoc.v));
        PSsendfloat(CW(thePortp->txSize));
        PSsendchararray(font, strlen(font));
        if(need_to_free)
            free(font);
    }

    DPSPrintf(DPSGetCurrentContext(), "findfont2\n");
    PSdup();
    PSsendchararray("FontInfo", 8);
    DPSPrintf(DPSGetCurrentContext(), "known {\n");
    PSdup();
    PSsendchararray("FontMatrix", 10);
    PSget();
    PSsendint(3);
    PSget();
    PSindex(2);
    PSmul();
    PSindex(1);
    PSsendchararray("FontInfo", 8);
    PSget();
    PSdup();
    PSsendchararray("UnderlineThickness", 18);
    PSget();
    PSindex(2);
    PSmul();
    DPSPrintf(DPSGetCurrentContext(), "setlinewidth\n");
    PSsendchararray("UnderlinePosition", 17);
    PSget();
    PSmul();
    PSroll(3, 1);
    PSpop();
    PSpop();
    PSsub();
    DPSPrintf(DPSGetCurrentContext(), "}\n{\n");
    PSpop();
    PSpop();
    DPSPrintf(DPSGetCurrentContext(), "}\nifelse\n");
    DPSPrintf(DPSGetCurrentContext(), "moveto\n");
    if(!substitute_p)
        PSrlineto(total, 0);
    else
    {
        /* The width is already on the stack.  See above */
        DPSPrintf(DPSGetCurrentContext(), "0 rlineto\n");
    }
    PSstroke();
    PSgrestore();
    RESTOREA5;
}

static void doshow(char *translated, LONGINT n)
{
    PSsendchararray(translated, n);
    DPSPrintf(DPSGetCurrentContext(), "show\n");
}

static void dowidthshow(char *translated, LONGINT n, int i, short total)
{
    PSsendint(n);
    PSsendint(i);
    PSsendint(total);
    PSsendchararray(translated, n);
    DPSPrintf(DPSGetCurrentContext(), "__char_spaces_width_show\n");
}

static void doashow(char *translated, LONGINT n, int i, short total)
{
    PSsendchararray(translated, n);
    PSdup();
    DPSPrintf(DPSGetCurrentContext(), "stringwidth\n");
    PSpop();
    PSsendint(total);
    PSexch();
    PSsub();
    PSsendint(n);
    PSdiv();
    PSexch();
    PSsendint(0);
    PSexch();
    DPSPrintf(DPSGetCurrentContext(), "ashow\n");
}

void Executor::NeXTsendps(LONGINT n, Ptr textbufp)
{
    virtual_int_state_t block;

    block = block_virtual_ints();
    DPSWritePostScript(DPSGetCurrentContext(), textbufp, n);
    restore_virtual_ints(block);
}

enum
{
    BULLET = 0xa5
};

/* beginning of ugly parallel enums and parallel switches */

enum mac_char
{
    mac_char_notequal = 0xad,

    mac_char_infinity = 0xb0,
    mac_char_plusminus = 0xb1,
    mac_char_lessequal = 0xb2,
    mac_char_greaterequal = 0xb3,
    mac_char_mu = 0xb5,
    mac_char_partialdiff = 0xb6,
    mac_char_Sigma = 0xb7,
    mac_char_Pi = 0xb8,
    mac_char_pi = 0xb9,
    mac_char_integral = 0xba,
    mac_char_Omega = 0xbd,

    mac_char_logicalnot = 0xc2,
    mac_char_radical = 0xc3,
    mac_char_florin = 0xc4,
    mac_char_approxequal = 0xc5,
    mac_char_Delta = 0xc6,

    mac_char_divide = 0xd6,
};

enum symbol_char
{
    symbol_char_notequal = 0271,

    symbol_char_infinity = 0245,
    symbol_char_plusminus = 0261,
    symbol_char_lessequal = 0243,
    symbol_char_greaterequal = 0263,
    symbol_char_mu = 0155,
    symbol_char_partialdiff = 0266,
    symbol_char_Sigma = 0123,
    symbol_char_Pi = 0120,
    symbol_char_pi = 0160,
    symbol_char_integral = 0362,
    symbol_char_Omega = 0127,

    symbol_char_logicalnot = 0330,
    symbol_char_radical = 0326,
    symbol_char_florin = 0246,
    symbol_char_approxequal = 0273,
    symbol_char_Delta = 0104,

    symbol_char_divide = 0270,
};

#define CHAR_REPLACE(s)           \
    case mac_char_##s:            \
        retval = symbol_char_##s; \
        is_symbol = true;         \
        break

PRIVATE unsigned char
symbol_translate_char(unsigned char c, bool *is_symbolp)
{
    unsigned char retval;
    bool is_symbol;

    retval = c;
    is_symbol = false;
    switch((enum mac_char)c)
    {
        CHAR_REPLACE(notequal);
        CHAR_REPLACE(infinity);
        CHAR_REPLACE(plusminus);
        CHAR_REPLACE(lessequal);
        CHAR_REPLACE(greaterequal);
        CHAR_REPLACE(mu);
        CHAR_REPLACE(partialdiff);
        CHAR_REPLACE(Sigma);
        CHAR_REPLACE(Pi);
        CHAR_REPLACE(pi);
        CHAR_REPLACE(integral);
        CHAR_REPLACE(Omega);

        CHAR_REPLACE(logicalnot);
        CHAR_REPLACE(radical);
        CHAR_REPLACE(florin);
        CHAR_REPLACE(approxequal);
        CHAR_REPLACE(Delta);

        CHAR_REPLACE(divide);
    }
    if(is_symbolp)
        *is_symbolp = is_symbol;

    return retval;
}

#undef CHAR_REPLACE

/* end of ugly parallel enums and parallel switches */

PRIVATE bool
is_symbol(unsigned char c)
{
    bool retval;

    symbol_translate_char(c, &retval);
    return retval;
}

PRIVATE void
find_run_of_symbol_chars(LONGINT n, Ptr textbufp, int *run_startp,
                         int *run_stopp)
{
    int start, stop;
    int i;

    start = -1;
    stop = -1;

    for(i = 0; i < n && !is_symbol(textbufp[i]); ++i)
        ;
    if(i < n)
    {
        start = i;
        for(; i < n && is_symbol(textbufp[i]); ++i)
            ;
        stop = i;
    }

    *run_startp = start;
    *run_stopp = stop;
}

void Executor::NeXTPrText(LONGINT n, Ptr textbufp, Point num, Point den,
                          GrafPtr thePortp)
{
    virtual_int_state_t block;
    /* right now blow off num and den */
    char *translated;
    short total;
    int i;
    int n_leading_spaces;
    int run_start, run_stop;

    if(thePortp->txFont != CWC(symbol))
    {
        find_run_of_symbol_chars(n, textbufp, &run_start, &run_stop);
        if(run_start >= 0)
        {
            decltype(thePortp->txFont) save_font;

            if(run_start > 0)
                NeXTPrText(run_start, textbufp, num, den, thePortp);
            save_font = thePortp->txFont;
            thePortp->txFont = CWC(symbol);
            NeXTPrText(run_stop - run_start, textbufp + run_start, num, den,
                       thePortp);
            thePortp->txFont = save_font;
            if(run_stop < n)
                NeXTPrText(n - run_stop, textbufp + run_stop, num, den,
                           thePortp);
            return;
        }
    }

    for(n_leading_spaces = 0;
        n_leading_spaces < n && (textbufp[n_leading_spaces] == ' '
                                 || (unsigned char)textbufp[n_leading_spaces]
                                     == BULLET);
        ++n_leading_spaces)
        ;
    if(n_leading_spaces && n_leading_spaces != n)
    {
        NeXTPrText(n_leading_spaces, textbufp, num, den, thePortp);
        NeXTPrText(n - n_leading_spaces, textbufp + n_leading_spaces,
                   num, den, thePortp);
    }
    else
    {
        if(n != 0)
        {
            block = block_virtual_ints();
            txupdate(thePortp);
            if(rotation.rotated_p)
            {
                PSgsave();
                DPSPrintf(DPSGetCurrentContext(), "initclip\n");
                PStranslate(CW(thePortp->pnLoc.h) + rotation.center_x,
                            CW(thePortp->pnLoc.v) + rotation.center_y);
                PSrotate(rotation.angle);
                PSmoveto(-rotation.center_x, -rotation.center_y);
            }
            GUEST<Point> num_s, den_s;
            num_s.set(num);
            den_s.set(den);
            total = NeXTPrTxMeas(n, textbufp, &num_s, &den_s, nullptr,
                                 thePortp);
            translated = (char *)alloca(n + 1);
            memcpy(translated, textbufp, n);
            /* strip trailing <CR>s */
            while(n > 0 && translated[n - 1] == '\r')
                --n;
            translated[n] = 0;
            if(n)
            {
                if(thePortp->txFont == CWC(symbol))
                {
                    int i;

                    for(i = 0; i < n; ++i)
                        translated[i]
                            = symbol_translate_char(translated[i], NULL);
                }

#if 0
		PSxshow(translated, fwidths, n);
#else
                if(substitute_fonts_p && (thePortp->txFont == CWC(geneva)))
                    doshow(translated, n);
                else if((i = numspacesin(translated)))
                    dowidthshow(translated, n, i, total);
                else
                    doashow(translated, n, i, total);
#endif
                if(thePortp->txFace & underline)
                    dopsunderline(thePortp, total,
                                  substitute_fonts_p
                                      && (thePortp->txFont == CWC(geneva)),
                                  translated, n);
                thePortp->pnLoc.h = CW(CW(thePortp->pnLoc.h) + total);
                printport.pnLoc.h = thePortp->pnLoc.h;
            }
            if(rotation.rotated_p)
                PSgrestore();
            restore_virtual_ints(block);
        }
    }
}

void Executor::NeXTOpenPage(void)
{
    /*
 * TODO: make sure that the open page really conforms to what InitPort
 *       gave us.
 */
}

PRIVATE bool text_state;

PRIVATE void
disable_copybits(void)
{
    if(!ROMlib_text_output_disabled_p)
        text_state = disable_text_printing();
}

PRIVATE void
enable_copybits(void)
{
    set_text_printing(text_state);
}

#define FIX_TO_FLOAT(x) ((float)(x) / (1 << 16))

PUBLIC void
Executor::do_textbegin(TTxtPicHdl h)
{
    disable_copybits();
    rotation.angle = Executor::GetHandleSize((Handle)h) >= 10
        ? FIX_TO_FLOAT(TEXTPIC_ANGLE_FIXED(h))
        : TEXTPIC_ANGLE(h);
    rotation.rotated_p = true;
}

PUBLIC void
Executor::do_textcenter(TCenterRecHdl h)
{
    rotation.center_x = FIX_TO_FLOAT(TEXTCENTER_X(h));
    rotation.center_y = FIX_TO_FLOAT(TEXTCENTER_Y(h));
}

PUBLIC void
Executor::do_textend(void)
{
    if(rotation.rotated_p)
    {
        rotation.rotated_p = false;
        enable_copybits();
    }
}
