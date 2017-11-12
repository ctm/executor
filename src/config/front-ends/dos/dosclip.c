/* Copyright 1994 - 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include <pc.h>
#include "rsys/myabort.h"
#include "vga.h"
#include "dosdisk.h"
#include <dos.h>
#include <dpmi.h>
#include <go32.h>

#warning "initclip(), PutScrapX(), GetScrapX() not implemented"

PRIVATE void initclip(void)
{
    static int been_here = 0;

    if(!been_here)
    {

        been_here = true;
    }
}

void PutScrapX(LONGINT type, LONGINT length, char *p, int scrap_count)
{
    initclip();

    if(type == TICKX("TEXT"))
    {
        /* TODO */
    }
    else if(type == TICKX("TIFF"))
    {
        /* TODO */
    }
    else if(type == TICKX("PICT"))
    {
        /* TODO */
    }
    else if(type == TICKX("RTF "))
    {
        /* TODO */
    }
}

LONGINT GetScrapX(LONGINT type, char **h)
{
    initclip();

    if(type == TICKX("TEXT"))
    {
        /* TODO */
    }
    else if(type == TICKX("TIFF"))
    {
        /* TODO */
    }
    else if(type == TICKX("PICT"))
    {
        /* TODO */
    }
    else if(type == TICKX("RTF "))
    {
        /* TODO */
    }
    return 0;
}
