#if !defined(__RSYS_LIST__)
#define __RSYS_LIST__

/*
 * Copyright 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "ListMgr.h"
#include "MemoryMgr.h"
#include "ResourceMgr.h"
#include "rsys/mman.h"

namespace Executor
{
extern void
C_ldef0(INTEGER, BOOLEAN, Rect *, Cell, INTEGER, INTEGER, ListHandle);
PASCAL_FUNCTION(ldef0);

#define DODRAW 0x8 /* whether or not we should really draw */


using listprocp = UPP<void (INTEGER mess, BOOLEAN sel, Rect *rectp,
                                 Cell cell, INTEGER off, INTEGER len, ListHandle lhand)>;

extern void ROMlib_listcall(INTEGER mess, BOOLEAN sel, Rect *rp, Cell cell,
                            INTEGER off, INTEGER len, ListHandle lhand);

#define LISTCALL(msg, sel, rect, cell, doff, dlen, list) \
    ROMlib_listcall((msg), (sel), (rect), (cell), (doff), (dlen), (list))


#define LISTDECL() INTEGER liststate

#define LISTBEGIN(l) (liststate = HGetState(HxP(l, listDefProc)), \
                      HSetState(HxP(l, listDefProc), liststate | LOCKBIT))

#define LISTEND(l) HSetState(HxP(l, listDefProc), liststate)

extern void ROMlib_vminmax(INTEGER *minp, INTEGER *maxp, ListPtr lp);
extern void ROMlib_hminmax(INTEGER *minp, INTEGER *maxp, ListPtr lp);
extern GUEST<INTEGER> *ROMlib_getoffp(Cell cell, ListHandle list);
}
#endif /* !defined(__RSYS_LIST__) */
