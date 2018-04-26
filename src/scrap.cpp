/* Copyright 1987, 1989, 1990, 1999 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ScrapMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "FileMgr.h"
#include "ScrapMgr.h"
#include "MemoryMgr.h"
#include "ResourceMgr.h"
#include "OSUtil.h"
#include "CQuickDraw.h"

#include "rsys/notmac.h"
#include "rsys/file.h"
#include "rsys/scrap.h"
#include "rsys/executor.h"
#include "rsys/cquick.h"

#if defined(CYGWIN32) && defined(SDL)
#include "sdlscrap.h"
#include "win_clip.h"
#endif

using namespace Executor;

PScrapStuff Executor::C_InfoScrap()
{
    if(Cx(LM(ScrapState)) < 0)
        ZeroScrap();
    return ((PScrapStuff)&LM(ScrapSize));
}

static OSErr cropen(INTEGER *fp)
{
    OSErr retval;

    retval = FSOpen(MR(LM(ScrapName)), CW(LM(BootDrive)), fp);
    if(retval == fnfErr)
    {
        retval = Create(MR(LM(ScrapName)), CW(LM(BootDrive)), TICK("MACS"), TICK("CLIP"));
        if(retval != noErr)
            return (retval);
        return (FSOpen(MR(LM(ScrapName)), CW(LM(BootDrive)), fp));
    }
    return (retval);
}

LONGINT Executor::C_UnloadScrap()
{
    OSErr retval;
    INTEGER f;
    LONGINT l = Cx(LM(ScrapSize));

    if(Cx(LM(ScrapState)) > 0)
    {
        retval = cropen(&f);
        if(retval != noErr)
            /*-->*/ return (retval);
        HLock(MR(LM(ScrapHandle)));
        retval = FSWriteAll(f, &l, STARH(MR(LM(ScrapHandle))));
        HUnlock(MR(LM(ScrapHandle)));
        if(retval != noErr)
            /*-->*/ return (retval);
        retval = FSClose(f);
        if(retval != noErr)
            /*-->*/ return (retval);
        LM(ScrapState) = CWC(0);
    }
    return noErr;
}

LONGINT Executor::C_LoadScrap()
{
    OSErr retval;
    INTEGER f;
    LONGINT l = Cx(LM(ScrapSize));

    if(LM(ScrapState) == CWC(0))
    {
        retval = FSOpen(MR(LM(ScrapName)), CW(LM(BootDrive)), &f);
        if(retval != noErr)
            return (retval);

        HUnlock(MR(LM(ScrapHandle)));
        ReallocHandle(MR(LM(ScrapHandle)), (Size)Cx(LM(ScrapSize)));
        if(LM(MemErr) != CWC(noErr))
            /*-->*/ return Cx(LM(MemErr));
        HLock(MR(LM(ScrapHandle)));
        retval = FSReadAll(f, &l, STARH(MR(LM(ScrapHandle))));
        HUnlock(MR(LM(ScrapHandle)));
        if(retval != noErr)
            return (retval);
        SetEOF(f, (LONGINT)0);
        FSClose(f);
        LM(ScrapState) = CWC(1);
    }
    return (Cx(LM(ScrapState)) > 0 ? noErr : noScrapErr);
}

LONGINT Executor::ROMlib_ZeroScrap()
{
    OSErr retval;
    INTEGER f;
    GUEST<THz> saveZone;

    if(Cx(LM(ScrapState)) < 0)
    {
        LM(ScrapCount) = 0;
        saveZone = LM(TheZone);
        LM(TheZone) = LM(SysZone);
        LM(ScrapHandle) = RM(NewHandle((Size)0));
        LM(TheZone) = saveZone;
        LM(ScrapState) = CWC(1);
        LM(ScrapName) = RM((StringPtr) "\016Clipboard File");
    }
    else if(Cx(LM(ScrapState)) == 0)
    {
        retval = cropen(&f);
        if(retval != noErr)
            return retval;
        retval = SetEOF(f, (LONGINT)0);
        if(retval != noErr)
            return retval;
        FSClose(f);
    }
    else if(Cx(LM(ScrapState)) > 0)
        SetHandleSize(MR(LM(ScrapHandle)), (Size)0);
    LM(ScrapSize) = 0;
    LM(ScrapCount) = CW(CW(LM(ScrapCount)) + 1);
    return noErr;
}

LONGINT Executor::C_ZeroScrap()
{
    return ROMlib_ZeroScrap();
}

LONGINT Executor::C_PutScrap(LONGINT len, ResType rest, Ptr p)
{
    OSErr retval;
    LONGINT l;
    GUEST<LONGINT> swappedlen;
    INTEGER f;

    GUEST<LONGINT> *lp;

    if(Cx(LM(ScrapState)) < 0)
    {
        retval = ZeroScrap();
        if(retval != noErr)
            /*-->*/ return (retval);
    }
#if defined(X) || defined(MACOSX_) || defined(SDL)
    PutScrapX(rest, len, (char *)p, CW(LM(ScrapCount)));
#endif /* defined(X) */
    if(Cx(LM(ScrapState)) == 0)
    {
        retval = FSOpen(MR(LM(ScrapName)), CW(LM(BootDrive)), &f);
        if(retval != noErr)
            /*-->*/ return (retval);
        SetFPos(f, fsFromStart, (LONGINT)Cx(LM(ScrapSize)));
        l = 4;
        GUEST<ResType> rest_s = CL(rest);
        FSWriteAll(f, &l, (Ptr)&rest_s);
        l = 4;
        swappedlen = CL(len);
        FSWriteAll(f, &l, (Ptr)&swappedlen);
        l = len = (len + 1) & -2L;
        FSWriteAll(f, &len, p);
        FSClose(f);
    }
    else
    {
        SetHandleSize(MR(LM(ScrapHandle)), (Size)Cx(LM(ScrapSize)) + 8);
        if(LM(MemErr) != CWC(noErr))
            /*-->*/ return CW(LM(MemErr));
        /* alignment stuff */
        lp = (GUEST<LONGINT> *)((char *)STARH(MR(LM(ScrapHandle))) + Cx(LM(ScrapSize)));
        *lp++ = CL(rest);
        *lp++ = CL(len);
        len = (len + 1) & -2L;
        PtrAndHand(p, MR(LM(ScrapHandle)), (Size)len);
    }
    LM(ScrapSize) = CL(CL(LM(ScrapSize)) + 8 + len);
    return noErr;
}

#if defined(CYGWIN32)
int
count_char(const char *p, int len, char c)
{
    int retval;

    retval = 0;
    while(--len >= 0)
        if(*p++ == c)
            ++retval;
    return retval;
}

static void
memcpy_but_delete_char(char *destp, const char *srcp, int len, char to_del)
{
    while(--len >= 0)
    {
        char c;

        c = *srcp++;
        if(c != to_del)
            *destp++ = c;
    }
}

int
get_scrap_helper(void *vh, void *lp, int len, bool convert_text)
{
    int retval;
    int new_len;
    Handle h;

    if(convert_text)
        new_len = len - count_char(lp, len, '\n'); /* won't copy linefeeds */
    else
        new_len = len;
    h = (Handle)vh;
    ReallocHandle(h, new_len);
    if(LM(MemErr) != CWC(noErr))
        retval = -1;
    else
    {
        if(convert_text)
            memcpy_but_delete_char(STARH(h), lp, len, '\n');
        else
            memcpy(STARH(h), lp, len);
        retval = new_len;
    }
    return retval;
}
#endif

#define RETURN(x) return (temph ? (DisposHandle(temph), 0) : 0), x

LONGINT Executor::C_GetScrap(Handle h, ResType rest, GUEST<LONGINT> *off)
{
    OSErr retval;
    LONGINT l = 0, incr, s, ltoread;
    GUEST<LONGINT> restlen[2];
    unsigned char *p;
    int found;
    INTEGER f;
    Handle temph;

#if !defined(LETGCCWAIL)
    s = 0;
#endif /* LETGCCWAIL */
    if(h)
        temph = 0;
    else
    {
        temph = NewHandle((Size)0);
        h = temph;
    }

#if defined(X) || defined(MACOSX_) || defined(SDL)
    s = GetScrapX(rest, h);
    if(s >= 0)
    {
        *off = 0; /* ack... could mess people up */
        /*-->*/ RETURN(s);
    }
#endif /* defined(X) */
    if(Cx(LM(ScrapState)) < 0)
    {
        retval = ZeroScrap();
        if(retval != noErr)
            /*-->*/ RETURN(retval);
    }
    if(LM(ScrapState) == CWC(0))
    {
        retval = FSOpen(MR(LM(ScrapName)), CW(LM(BootDrive)), &f);
        if(retval != noErr)
            /*-->*/ RETURN(retval);
        found = false;
        while(l < Cx(LM(ScrapSize)) && !found)
        {
            ltoread = 8;
            FSReadAll(f, &ltoread, (Ptr)restlen);
            s = CL(restlen[1]);
            if(rest == CL(restlen[0]))
                found = true;
            else
            {
                incr = (8 + s + 1) & ~1L;
                l += incr;
                SetFPos(f, fsFromMark, incr);
            }
        }
        if(l >= Cx(LM(ScrapSize)))
        {
            FSClose(f);
            /*-->*/ RETURN(noTypeErr);
        }
        ReallocHandle(h, s);
        if(LM(MemErr) != CWC(noErr))
            /*-->*/ RETURN(CW(LM(MemErr)));
        HLock(h);
        ltoread = s;
        FSReadAll(f, &ltoread, STARH(h));
        HUnlock(h);
        FSClose(f);
    }
    else
    {
        HLock(MR(LM(ScrapHandle)));
        p = (unsigned char *)MR(*MR(LM(ScrapHandle)));
#if 1 || !defined(QUADALIGN)
        while(l < Cx(LM(ScrapSize)) && rest != CL(*(GUEST<LONGINT> *)p))
        {
            s = CL(*((GUEST<LONGINT> *)p + 1));
            incr = (8 + s + 1) & ~1L;
            l += incr;
            p += incr;
        }
        if(l >= Cx(LM(ScrapSize)))
        {
            HUnlock(MR(LM(ScrapHandle)));
            /*-->*/ RETURN(noTypeErr);
        }
        s = CL(*((GUEST<LONGINT> *)p + 1));
#else /* QUADALIGN */
        while(l < Cx(LM(ScrapSize)) && rest != SNAGLONG(p))
        {
            incr = 8 + ((s = SNAGLONG(p + sizeof(LONGINT))) + 1) & -2L;
            l += incr;
            p += incr;
        }
        if(l >= Cx(LM(ScrapSize)))
        {
            HUnlock(MR(LM(ScrapHandle)));
            /*-->*/ RETURN(noTypeErr);
        }
        s = *((LONGINT *)p + 1);
#endif /* QUADALIGN */
        PtrToXHand((Ptr)(p + 8), h, s);
        HUnlock(MR(LM(ScrapHandle)));
    }
    *off = CL(l + 8);
    RETURN(s);
}

#if defined(CYGWIN32) && defined(SDL)

#include "SDL/SDL.h"

static int
SDL_Surface_depth(const SDL_Surface *surfp)
{
    int retval;
    retval = surfp->format->BitsPerPixel;
    return retval;
}

static int
SDL_Surface_width(const SDL_Surface *surfp)
{
    int retval;
    retval = surfp->w;
    return retval;
}

static int
SDL_Surface_height(const SDL_Surface *surfp)
{
    int retval;
    retval = surfp->h;
    return retval;
}

void *
SDL_Surface_pixels(const SDL_Surface *surfp)
{
    void *retval;
    retval = surfp->pixels;
    return retval;
}

int
SDL_Surface_pitch(const SDL_Surface *surfp)
{
    int retval;
    retval = surfp->pitch;
    return retval;
}

int
SDL_n_colors(const SDL_Surface *surfp)
{
    int retval;
    retval = surfp->format->palette->ncolors;
    return retval;
}

SDL_Color *
SDL_colors(const SDL_Surface *surfp)
{
    SDL_Color *retval;
    retval = surfp->format->palette->colors;
    return retval;
}

typedef struct
{
    uint8_t blue PACKED;
    uint8_t red PACKED;
    uint8_t green PACKED;
} sdl_pixel24;

typedef struct
{
    uint8_t zero PACKED;
    uint8_t red PACKED;
    uint8_t green PACKED;
    uint8_t blue PACKED;
} mac_pixel32;

#define advance_n_bytes(ptrp, n_bytes)                             \
    ({                                                             \
        decltype(ptrp) _ptrp;                                      \
                                                                   \
        _ptrp = (ptrp);                                            \
        *(_ptrp) = (decltype(*_ptrp))((char *)*(_ptrp) + n_bytes); \
    })

#define MAC_COLOR_COMPONENT_FROM_SDL_CC(x) \
    ({                                     \
        decltype(x) _x;                    \
                                           \
        _x = (x);                          \
        (_x << 8) | (uint8_t)_x;             \
    })

static CTabHandle
ctab_from_surface(SDL_Surface *surfp)
{
    CTabHandle retval;
    int n_colors;
    SDL_Color *ip;
    ColorSpec *op;
    int i;

    retval = NULL;

    n_colors = SDL_n_colors(surfp);
    retval = (CTabHandle)NewHandle(CTAB_STORAGE_FOR_SIZE(n_colors - 1));
    CTAB_SIZE_X(retval) = CW(n_colors - 1);
    CTAB_SEED_X(retval) = CL(GetCTSeed());
    CTAB_FLAGS_X(retval) = CTAB_GDEVICE_BIT_X;

    for(i = 0, ip = SDL_colors(surfp), op = CTAB_TABLE(retval);
        i < n_colors;
        ++i, ++ip, ++op)
    {
        op->value = CWC(0);
        op->rgb.red = MAC_COLOR_COMPONENT_FROM_SDL_CC(ip->r);
        op->rgb.green = MAC_COLOR_COMPONENT_FROM_SDL_CC(ip->g);
        op->rgb.blue = MAC_COLOR_COMPONENT_FROM_SDL_CC(ip->b);
    }

    return retval;
}

static GWorldPtr
gworld_from_surface(SDL_Surface *surfp)
{
    GWorldPtr retval;

    retval = NULL;

    if(surfp)
    {
        QDErr err;
        int surf_depth;
        CTabHandle ctab;

        ctab = NULL;

        surf_depth = SDL_Surface_depth(surfp);
        switch(surf_depth)
        {
            case 8:
                ctab = ctab_from_surface(surfp);
                break;
            case 32:
                break;
            default:
                warning_unexpected("surf_depth = %d", surf_depth);
                surf_depth = 0;
                break;
        }

        if(surf_depth)
        {
            int n_lines;
            int pixels_per_line;
            Rect r;

            n_lines = SDL_Surface_height(surfp);
            pixels_per_line = SDL_Surface_width(surfp);

            r.top = CWC(0);
            r.left = CWC(0);
            r.bottom = CW(n_lines);
            r.right = CW(pixels_per_line);
            {
                CGrafPtr save_port;
                GDHandle save_device;

                GetGWorld(&save_port, &save_device);
                save_port = MR(save_port);
                save_device = MR(save_device);
                err = NewGWorld(&retval, surf_depth, &r, ctab, NULL, keepLocal);
                SetGWorld(save_port, save_device);
            }
            if(retval)
            {
                PixMapHandle pm;

                retval = MR(retval);
                pm = GetGWorldPixMap(retval);
                LockPixels(pm);
                SDL_LockSurface(surfp);

                switch(surf_depth)
                {
                    case 8:
                    {
                        uint8_t *ip, *eip;
                        uint8_t *op;
                        int rowbytes;
                        int pitch;

                        pitch = SDL_Surface_pitch(surfp);
                        rowbytes = PIXMAP_ROWBYTES(pm);

                        ip = SDL_Surface_pixels(surfp);
                        op = (decltype(op))GetPixBaseAddr(pm);
                        eip = ip + n_lines * pitch;
                        for(; ip != eip; ip += pitch, op += rowbytes)
                            memcpy(op, ip, rowbytes);
                        break;
                    }

                    case 32:
                    {
                        sdl_pixel24 *ip;
                        mac_pixel32 *op;

                        op = (decltype(op))GetPixBaseAddr(pm);
                        ip = SDL_Surface_pixels(surfp);

                        memcpy(op, ip, n_lines * pixels_per_line * sizeof *op);

                        break;
                    }
                    default:
                        warning_unexpected("surf_depth = %d", surf_depth);
                        break;
                }
                SDL_UnlockSurface(surfp);
                UnlockPixels(pm);
            }
        }
    }
    return retval;
}

static PicHandle
pict_from_gworld(GWorldPtr gp, int *lenp)
{
    PicHandle retval;

    if(!gp)
        retval = NULL;
    else
    {
        Rect pict_frame;
        PixMapHandle pm;

        pm = GetGWorldPixMap(gp);
        pict_frame = PIXMAP_BOUNDS(pm);
        retval = OpenPicture(&pict_frame);
        if(retval)
        {
            ClipRect(&pict_frame);
            HLock((Handle)pm);
            CopyBits((BitMap *)STARH(pm), PORT_BITS_FOR_COPY(thePort),
                     &pict_frame, &pict_frame, srcCopy, NULL);
            HUnlock((Handle)pm);
            ClosePicture();
        }
    }
    return retval;
}

int
get_scrap_helper_dib(void *vh, void *lp)
{
    SDL_Surface *surfp;
    GWorldPtr gp;
    PicHandle pich;
    Handle h;
    int retval;
    int len;

    surfp = surface_from_dib(lp);
    gp = gworld_from_surface(surfp);
    SDL_FreeSurface(surfp);
    pich = pict_from_gworld(gp, &len);
    DisposeGWorld(gp);
    h = (Handle)vh;
    len = GetHandleSize((Handle)pich);
    ReallocHandle(h, len);
    if(LM(MemErr) != CWC(noErr))
        retval = -1;
    else
    {
        memcpy(STARH(h), STARH(pich), len);
        retval = len;
    }
    DisposHandle((Handle)pich);
    return retval;
}

static PicHandle
pict_from_lp(const void *lp)
{
    PicHandle retval;

    if(!lp)
        retval = NULL;
    else
    {
        int len;

        len = *(int *)lp;
        retval = (PicHandle)NewHandle(len);
        if(retval)
        {
            char *p;

            p = (char *)STARH(retval);
            if(p)
                memcpy(p, (char *)lp + sizeof(int), len);
            else
            {
                warning_unexpected(NULL_STRING);
                DisposHandle((Handle)retval);
                retval = NULL;
            }
        }
    }

    return retval;
}

static GWorldPtr
gworld_from_pict(PicHandle ph)
{
    GWorldPtr retval;

    retval = NULL;
    if(ph)
    {
        CGrafPtr save_port;
        GDHandle save_device;
        Rect r;
        OSErr err;

        GetGWorld(&save_port, &save_device);
        save_port = MR(save_port);
        save_device = MR(save_device);
        r = HxX(ph, picFrame);
        err = NewGWorld(&retval, 32, &r, NULL, NULL, keepLocal);
        if(retval)
        {
            PixMapHandle pm;

            retval = MR(retval);
            SetGWorld(retval, NULL);
            pm = GetGWorldPixMap(retval);
            LockPixels(pm);
            DrawPicture(ph, &r);
#if 0
// FIXME: #warning THIS INTERFERES WITH PICT PASTING
	  {
	    char *p;

	    EraseRect (&r);
	    p = GetPixBaseAddr (pm);
	    memset (p, 0x00, 4 * RECT_HEIGHT(&r) * RECT_WIDTH (&r));
	    memset (p, 0xFF, 4 * RECT_HEIGHT(&r) * RECT_WIDTH (&r) / 2);
	  }
#endif
            UnlockPixels(pm);
        }
        SetGWorld(save_port, save_device);
    }
    return retval;
}

static SDL_Surface *
surface_from_gworld(GWorldPtr gp)
{
    SDL_Surface *retval;

    if(!gp)
        retval = NULL;
    else
    {
        int pixels_per_line;
        int n_lines;
        PixMapHandle pm;
        enum
        {
            A = 0x00000000,
            R = 0x0000FF00,
            G = 0x00FF0000,
            B = 0xFF000000
        };
        mac_pixel32 *ip;
        sdl_pixel24 *op;
        Rect r;

        pm = GetGWorldPixMap(gp);
        LockPixels(pm);

        r = PIXMAP_BOUNDS(pm);
        n_lines = RECT_HEIGHT(&r);
        pixels_per_line = RECT_WIDTH(&r);
        retval = SDL_AllocSurface(SDL_SWSURFACE, pixels_per_line, n_lines, 32,
                                  R, G, B, A);
        SDL_LockSurface(retval);
        op = SDL_Surface_pixels(retval);
        ip = (decltype(ip))GetPixBaseAddr(pm);
        memcpy(op, ip, n_lines * pixels_per_line * sizeof *ip);
#if 0
// FIXME: #warning THIS IS BROKEN
      memset (op, 0x00, 4 * n_lines * pixels_per_line);
      memset (op, 0xFF, 4 * n_lines * pixels_per_line / 2);
#endif
        SDL_UnlockSurface(retval);
        UnlockPixels(pm);
    }

    return retval;
}

/* write_surfp_to_clipboard */

void
put_scrap_helper_dib(void *lp)
{
    PicHandle pich;

    pich = pict_from_lp(lp);
    if(pich)
    {
        GWorldPtr gp;

        gp = gworld_from_pict(pich);
        if(gp)
        {
            SDL_Surface *surfp;

            surfp = surface_from_gworld(gp);
            if(surfp)
            {
                write_surfp_to_clipboard(surfp);
                SDL_FreeSurface(surfp);
            }
            DisposeGWorld(gp);
        }
        DisposHandle((Handle)pich);
    }
}

#endif
