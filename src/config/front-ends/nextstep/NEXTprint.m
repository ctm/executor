#include "rsys/common.h"

#ifndef OPENSTEP

#import <appkit/View.h>
#import <appkit/Window.h>
#import <appkit/NXBitmapImageRep.h>
#import <appkit/Application.h>   // For NXApp
#import <appkit/PrintInfo.h>
#import <appkit/Font.h>
#import <appkit/FontManager.h>
#import <dpsclient/wraps.h>

#else /* OPENSTEP */

#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSBitmapImageRep.h>
#import <AppKit/NSApplication.h>   // For NXApp
#import <AppKit/NSPrintInfo.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSFontManager.h>
#import <AppKit/psops.h>

#endif /* OPENSTEP */


#include "rsys/mactype.h"
#include "rsys/nextprint.h"
#include "rsys/blockinterrupts.h"
#include "rsys/next.h"
#include "rsys/syn68k_public.h"

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_NEXTprint[] =
	    "$Id: NEXTprint.m,v 2.5 1997/07/19 01:17:09 ctm Exp $";
#endif

/*
 * Copyright 1992 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 */

#ifndef OPENSTEP
char **ROMlib_availableFonts(void)
{
    return [[FontManager new] availableFonts];
}
#endif /* not OPENSTEP */

void ROMlib_newFont(char *font, float txSize)
{
#ifndef OPENSTEP
    [Font newFont:font size:txSize];
#else /* OPENSTEP */
    [NSFont fontWithName:[NSString stringWithCString:font] size:txSize];
#endif /* OPENSTEP */
}

void ROMlib_updatenextpagerect(comRect *rp)
{
#ifndef OPENSTEP
    NXRect tmpr;
#else /* OPENSTEP */
    NSSize new_paper_size;
#endif /* OPENSTEP */
    virtual_int_state_t block;

    block = block_virtual_ints ();
#ifndef OPENSTEP
    tmpr.origin.y = CW(rp->top) ;
    tmpr.size.height = CW(rp->bottom) - CW(rp->top);
    tmpr.origin.x = CW(rp->left);
    tmpr.size.width = CW(rp->right) - CW(rp->left);
    [[NXApp printInfo] setPaperRect:&tmpr andAdjust:YES];
#else /* OPENSTEP */
    new_paper_size.height = CW(rp->bottom) - CW(rp->top);
    new_paper_size.width = CW(rp->right) - CW(rp->left);
    [[NSPrintInfo sharedPrintInfo] setPaperSize:new_paper_size];
#endif /* OPENSTEP */
    restore_virtual_ints (block);
}

#define HALFINCH	36

#ifdef OPENSTEP
static float printer_scaling_factor (void)
{
  id key_val;
  float retval;

  key_val =
    [[[NSPrintInfo sharedPrintInfo] dictionary] objectForKey:NSPrintScalingFactor];

  if (key_val)
    retval = [key_val floatValue];
  else
    retval = 1;

  return retval;
}
#endif /* OPENSTEP */

void ROMlib_updatemacpagerect(comRect *paperp, comRect *page1p,
							       comRect *page2p)
{
#ifndef OPENSTEP
    const NXRect *tmpr;
#else /* OPENSTEP */
    NSSize new_size;
#endif /* OPENSTEP */
    virtual_int_state_t block;
#ifdef OPENSTEP
    float scaling_factor;
#endif /* OPENSTEP */

    block = block_virtual_ints ();
#ifndef OPENSTEP
    tmpr = [[NXApp printInfo] paperRect];
    paperp->top    = CW(tmpr->origin.y - HALFINCH);
    paperp->left   = CW(tmpr->origin.x - HALFINCH);

    paperp->bottom = CW(tmpr->origin.y +
       tmpr->size.height * (1 / [[NXApp printInfo] scalingFactor]) - HALFINCH);
    paperp->right  = CW(tmpr->origin.x +
       tmpr->size.width  * (1 / [[NXApp printInfo] scalingFactor]) - HALFINCH);
#else /* OPENSTEP */
    new_size = [[NSPrintInfo sharedPrintInfo] paperSize];
    paperp->top    = CW(0 - HALFINCH);
    paperp->left   = CW(0 - HALFINCH);

    scaling_factor = printer_scaling_factor ();
    paperp->bottom = CW(new_size.height * (1 / scaling_factor) - HALFINCH);
    paperp->right  = CW(new_size.width  * (1 / scaling_factor) - HALFINCH);
#endif /* OPENSTEP */

    page1p->top    = page2p->top    = 0;
    page1p->left   = page2p->left   = 0;
    page1p->bottom = page2p->bottom = CW(CW(paperp->bottom) - HALFINCH);
    page1p->right  = page2p->right  = CW(CW(paperp->right)  - HALFINCH);
    restore_virtual_ints (block);
}
