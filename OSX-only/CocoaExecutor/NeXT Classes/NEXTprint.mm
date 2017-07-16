#include "rsys/common.h"

#import <Cocoa/Cocoa.h>


#include "rsys/mactype.h"
#include "rsys/nextprint.h"
#include "rsys/blockinterrupts.h"
#include "rsys/next.h"
#include <syn68k_public.h>

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_NEXTprint[] =
	    "$Id: NEXTprint.m,v 2.5 1997/07/19 01:17:09 ctm Exp $";
#endif

/*
 * Copyright 1992 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 */

namespace Executor {
using namespace ByteSwap;

void ROMlib_newFont(char *font, float txSize)
{
    [[NSFont fontWithName:[NSString stringWithCString:font encoding:NSMacOSRomanStringEncoding] size:txSize] set];
}

void ROMlib_updatenextpagerect(comRect *rp)
{
    NSSize new_paper_size;
    virtual_int_state_t block = block_virtual_ints();

    new_paper_size.height = BigEndianValue(rp->bottom) - BigEndianValue(rp->top);
    new_paper_size.width = BigEndianValue(rp->right) - BigEndianValue(rp->left);
    [[NSPrintInfo sharedPrintInfo] setPaperSize:new_paper_size];
    restore_virtual_ints(block);
}

#define HALFINCH	36

static CGFloat printer_scaling_factor (void)
{
  CGFloat retval = [[NSPrintInfo sharedPrintInfo] scalingFactor];

  return retval;
}

void ROMlib_updatemacpagerect(comRect *paperp, comRect *page1p,
                                        comRect *page2p)
{
    NSSize new_size;
    virtual_int_state_t block = block_virtual_ints();
    CGFloat scaling_factor;

    new_size = [[NSPrintInfo sharedPrintInfo] paperSize];
    paperp->top    = BigEndianValue(0 - HALFINCH);
    paperp->left   = BigEndianValue(0 - HALFINCH);

    scaling_factor = printer_scaling_factor ();
    paperp->bottom = BigEndianValue(new_size.height * (1 / scaling_factor) - HALFINCH);
    paperp->right  = BigEndianValue(new_size.width  * (1 / scaling_factor) - HALFINCH);

    page1p->top    = page2p->top    = 0;
    page1p->left   = page2p->left   = 0;
    page1p->bottom = page2p->bottom = BigEndianValue(BigEndianValue(paperp->bottom) - HALFINCH);
    page1p->right  = page2p->right  = BigEndianValue(BigEndianValue(paperp->right)  - HALFINCH);
    restore_virtual_ints(block);
}
}
