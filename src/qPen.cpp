/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/picture.h"

using namespace Executor;

void Executor::C_HidePen()
{
    if(thePortX)
        PORT_PEN_VIS_X(thePort) = CW(PORT_PEN_VIS(thePort) - 1);
}

void Executor::C_ShowPen()
{
    if(thePortX)
        PORT_PEN_VIS_X(thePort) = CW(PORT_PEN_VIS(thePort) + 1);
}

void Executor::C_GetPen(GUEST<Point> *ptp)
{
    if(thePortX)
        *ptp = PORT_PEN_LOC(thePort);
}

void Executor::C_GetPenState(PenState *ps)
{
    if(!thePortX)
        return;

    if(CGrafPort_p(thePort))
    {
        PixPatHandle pen_pixpat;

        ps->pnLoc = PORT_PEN_LOC(thePort);
        ps->pnSize = PORT_PEN_SIZE(thePort);
        ps->pnMode = PORT_PEN_MODE_X(thePort);

        pen_pixpat = CPORT_PEN_PIXPAT(theCPort);
        /*
 * NOTE: it's not clear what the Mac does here.  Cotton has been
 *	 wrong about this stuff before.
 */
        if(PIXPAT_TYPE_X(pen_pixpat) == CWC(pixpat_type_orig))
            /* #warning GetPenState not necessarily implemented correctly... */
            PATASSIGN(ps->pnPat, PIXPAT_1DATA(pen_pixpat));
        else
        {
            /* high bit indicates there is a pixpat (not a pattern)
	     stored in the pnPat field */
            ps->pnMode.raw_or(CWC(0x8000));
            *(PixPatHandle *)&ps->pnPat[0] = pen_pixpat;
        }
    }
    else
        *ps = *(PenState *)&PORT_PEN_LOC(thePort);
}

void Executor::C_SetPenState(PenState *ps)
{
    if(!thePortX)
        return;

    PORT_PEN_LOC(thePort) = ps->pnLoc;
    PORT_PEN_SIZE(thePort) = ps->pnSize;

    if(ps->pnMode & CWC(0x8000))
    {
        PORT_PEN_MODE_X(thePort) = ps->pnMode & CWC(~0x8000);
        PenPixPat(*(PixPatHandle *)&ps->pnPat[0]);
    }
    else
    {
        PORT_PEN_MODE_X(thePort) = ps->pnMode;
        PenPat(ps->pnPat);
    }
}

void Executor::draw_state_save(draw_state_t *draw_state)
{
    GrafPtr current_port;

    current_port = thePort;

    GetPenState(&draw_state->pen_state);
    if(CGrafPort_p(current_port))
    {
        draw_state->fg_color = CPORT_RGB_FG_COLOR(current_port);
        draw_state->bk_color = CPORT_RGB_BK_COLOR(current_port);
    }
    draw_state->fg = PORT_FG_COLOR_X(current_port);
    draw_state->bk = PORT_BK_COLOR_X(current_port);

    draw_state->tx_font = PORT_TX_FONT_X(current_port);
    draw_state->tx_face = PORT_TX_FACE_X(current_port);
    draw_state->tx_size = PORT_TX_SIZE_X(current_port);
    draw_state->tx_mode = PORT_TX_MODE_X(current_port);
}

void Executor::draw_state_restore(draw_state_t *draw_state)
{
    GrafPtr current_port;

    current_port = thePort;

    SetPenState(&draw_state->pen_state);
    if(CGrafPort_p(current_port))
    {
        CPORT_RGB_FG_COLOR(current_port) = draw_state->fg_color;
        CPORT_RGB_BK_COLOR(current_port) = draw_state->bk_color;
    }
    PORT_FG_COLOR_X(current_port) = draw_state->fg;
    PORT_BK_COLOR_X(current_port) = draw_state->bk;

    PORT_TX_FONT_X(current_port) = draw_state->tx_font;
    PORT_TX_FACE_X(current_port) = draw_state->tx_face;
    PORT_TX_SIZE_X(current_port) = draw_state->tx_size;
    PORT_TX_MODE_X(current_port) = draw_state->tx_mode;
}

void Executor::C_PenSize(INTEGER w, INTEGER h)
{
    if(thePortX)
    {
        PORT_PEN_SIZE(thePort).h = CW(w);
        PORT_PEN_SIZE(thePort).v = CW(h);
    }
}

void Executor::C_PenMode(INTEGER m)
{
    if(thePortX)
        PORT_PEN_MODE_X(thePort) = CW(m);
}

void Executor::C_PenPat(Pattern pp)
{
    if(thePortX)
    {
        if(CGrafPort_p(thePort))
        {
            PixPatHandle old_pen;

            old_pen = CPORT_PEN_PIXPAT(theCPort);
            if(PIXPAT_TYPE_X(old_pen) == CWC(pixpat_type_orig))
                PATASSIGN(PIXPAT_1DATA(old_pen), pp);
            else
            {
                PixPatHandle new_pen = NewPixPat();

                PIXPAT_TYPE_X(new_pen) = CWC(0);
                PATASSIGN(PIXPAT_1DATA(new_pen), pp);

                PenPixPat(new_pen);
            }
            /* #warning PenPat not currently implemented correctly... */
        }
        else
            PATASSIGN(PORT_PEN_PAT(thePort), pp);
    }
}

void Executor::C_PenNormal()
{
    if(thePortX)
    {
        PenSize(1, 1);
        PenMode(patCopy);
        PenPat(black);
    }
}

void Executor::C_MoveTo(INTEGER h, INTEGER v)
{
    if(thePortX)
    {
        PORT_PEN_LOC(thePort).h = CW(h);
        PORT_PEN_LOC(thePort).v = CW(v);
    }
}

void Executor::C_Move(INTEGER dh, INTEGER dv)
{
    if(thePortX)
    {
        PORT_PEN_LOC(thePort).h = CW(CW(PORT_PEN_LOC(thePort).h) + (dh));
        thePort->pnLoc.v = CW(CW(PORT_PEN_LOC(thePort).v) + (dv));
    }
}

void Executor::C_LineTo(INTEGER h, INTEGER v)
{
    Point p;

    if(thePortX)
    {
        p.h = h;
        p.v = v;
        CALLLINE(p);
        PORT_PEN_LOC(thePort).h = CW(p.h);
        PORT_PEN_LOC(thePort).v = CW(p.v);
    }
}

void Executor::C_Line(INTEGER dh, INTEGER dv)
{
    Point p;

    if(thePortX)
    {
        p.h = CW(PORT_PEN_LOC(thePort).h) + dh;
        p.v = CW(PORT_PEN_LOC(thePort).v) + dv;
        CALLLINE(p);
        PORT_PEN_LOC(thePort).h = CW(p.h);
        PORT_PEN_LOC(thePort).v = CW(p.v);
    }
}
