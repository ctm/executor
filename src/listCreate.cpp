/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_listCreate[] =
	    "$Id: listCreate.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in ListMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "MemoryMgr.h"
#include "ResourceMgr.h"
#include "ListMgr.h"
#include "rsys/list.h"

using namespace Executor;

#define STEF_lActivefix

A3(PUBLIC, void, ROMlib_vminmax, INTEGER *, minp,		/* INTERNAL */
						 INTEGER *, maxp, ListPtr, lp)
{
    *minp = CW(lp->dataBounds.top);
    *maxp = *minp + CW(lp->dataBounds.bottom) - CW(lp->visible.bottom) +
                                                CW(lp->visible.top);
    if (CW(lp->cellSize.v) * (CW(lp->visible.bottom) - CW(lp->visible.top)) > 
				      CW(lp->rView.bottom) - CW(lp->rView.top))
	++*maxp;
}

A3(PUBLIC, void, ROMlib_hminmax, INTEGER *, minp,		/* INTERNAL */
						 INTEGER *, maxp, ListPtr, lp)
{
    *minp = CW(lp->dataBounds.left);
    *maxp = *minp + CW(lp->dataBounds.right) - CW(lp->visible.right) +
                                               CW(lp->visible.left);
    if (CW(lp->cellSize.h) * (CW(lp->visible.right) - CW(lp->visible.left)) > 
				      CW(lp->rView.right) - CW(lp->rView.left))
	++*maxp;
}

/* #define LIST_DEBUG */
#if defined (LIST_DEBUG)
typedef struct ll_elem_str
{
  ListHandle list;
  GrafPtr orig_port;
  pascal trap void (*lastTextProc)(INTEGER bc, Ptr p, Point num, Point den);
  struct ll_elem_str *next;
}
ll_elem;

PRIVATE ll_elem *ll_head;

PRIVATE void
add_list (ListHandle list)
{
  ll_elem *new_elemp;

  new_elemp = malloc (sizeof *new_elemp);
  new_elemp->list = list;
  new_elemp->orig_port = HxP(list, port);
  new_elemp->next = ll_head;
  ll_head = new_elemp;
}

PRIVATE void
delete_list (ListHandle list)
{
  ll_elem **pp;

  for (pp = &ll_head; *pp && (*pp)->list != list; pp = &(*pp)->next)
    ;
  if (*pp)
    {
      ll_elem *to_freep;

      to_freep = *pp;
      *pp = (*pp)->next;
      free (to_freep);
    }
}

PUBLIC void
check_lists (void)
{
  ll_elem **pp;

  for (pp = &ll_head; *pp; pp = &(*pp)->next)
    {
      GrafPtr gp;

      gp = HxP((*pp)->list, port);
      if (gp != (*pp)->orig_port)
	(*pp)->lastTextProc = MR(gp->grafProcs)->textProc;
    }
}
#endif

/*
 * NOTE:  I really don't know what "grow" is for below.
 * This doesn't worry me too much because I suspect it was put in there
 * when people were thinking about having the scroll bars within "rview".
 */

P9(PUBLIC pascal trap, ListHandle,  LNew, Rect *, rview,	/* IMIV-270 */
	    Rect *, bounds, Point, csize, INTEGER, proc, WindowPtr, wind,
	      BOOLEAN, draw, BOOLEAN, grow, BOOLEAN, scrollh, BOOLEAN, scrollv)
{
    ListHandle retval;
    ListPtr lp;
    INTEGER noffs, min, max;
    INTEGER *ip;
    Rect r;
    int i;
    GUEST<DataHandle> tempdatah;
    GUEST<Handle> temph;
    LISTDECL();

    noffs = (CW(bounds->right) - CW(bounds->left)) *
				     (CW(bounds->bottom) - CW(bounds->top)) +1;
    retval = (ListHandle) NewHandle(sizeof(ListRec) -
	         sizeof(HxX(retval, cellArray)) + (noffs+1) * sizeof(INTEGER));
    if (!retval)
/*-->*/	return 0;	/* couldn't allocate memory */

    temph = RM(GetResource(TICK("LDEF"), proc));
    if (!(HxX(retval, listDefProc) = temph)) {
	DisposHandle((Handle) retval);
/*-->*/	return 0;	/* spooey list definition proc */
    }

    TRAPBEGIN();
    tempdatah = RM((DataHandle) NewHandle(0));
    HxX(retval, cells) = tempdatah;
    HLock((Handle) retval);
    lp = STARH(retval);

    lp->dataBounds  = *bounds;
    lp->rView       = *rview;
    lp->port        = RM(wind);
    lp->indent.h    = 0;
    lp->indent.v    = 0;
    lp->selFlags    = 0;
#if defined (STEF_lActivefix)
    lp->lActive     = TRUE;
#else
    lp->lActive     = FrontWindow() == wind;
#endif
    lp->lReserved   = 0;
    lp->clikTime    = 0;
    lp->clikLoc.h   = CWC(-32768);
    lp->clikLoc.v   = CWC(-32768);
    lp->mouseLoc.h  = -1;
    lp->mouseLoc.v  = -1;
    lp->lClikLoop   = 0;
    lp->lastClick.h = -1;
    lp->lastClick.v = -1;
    lp->refCon      = 0;
    lp->userHandle  = (Handle) 0;
    lp->maxIndex    = -1;	/* What is this anyway? */
    ip = (INTEGER *) lp->cellArray;
    for (i = 0; i <= noffs; i++)
        *ip++ = 0;

    lp->visible.top  = bounds->top;
    lp->visible.left = bounds->left;
    lp->vScroll = 0;
    lp->hScroll = 0;
    C_LCellSize(csize, retval);	/* sets cellSize and visible */

    lp->listFlags = draw ? DODRAW : 0;
    if (scrollv) {
	r = lp->rView;
	r.top = CW(CW(r.top) - 1);
	r.left = r.right;
	r.right = CW(CW(r.right) + (16));
	r.bottom = CW(CW(r.bottom) + 1);
	ROMlib_vminmax(&min, &max, lp);
	lp->vScroll = RM(NewControl((WindowPtr) wind, &r, (StringPtr) "",
	       draw && lp->lActive, min, min, max, scrollBarProc, (LONGINT) 0));
	STARH(MR(lp->vScroll))->contrlRfCon = guest_cast<LONGINT>(RM(retval));
	lp->listFlags |= lDoVAutoscroll;
    }

    if (scrollh) {
	r = lp->rView;
	r.left = CW(CW(r.left) - 1);
	r.top = r.bottom;
	r.bottom = CW(CW(r.bottom) + (16));
	r.right = CW(CW(r.right) + 1);
	ROMlib_hminmax(&min, &max, lp);
	lp->hScroll = RM(NewControl((WindowPtr) wind, &r, (StringPtr) "",
	       draw && lp->lActive, min, min, max, scrollBarProc, (LONGINT) 0));
	STARH(MR(lp->hScroll))->contrlRfCon = guest_cast<LONGINT>(RM(retval));
	lp->listFlags |= lDoHAutoscroll;
    }

    HUnlock((Handle) retval);
    LISTBEGIN(retval);
    LISTCALL(lInitMsg, FALSE, (Rect *)0, * (Cell *)&lp->clikLoc, 0, 0, retval);
    LISTEND(retval);
    TRAPEND();
#if defined (LIST_DEBUG)
    add_list (retval);
#endif
    return retval;
}

P1(PUBLIC pascal trap, void, LDispose, ListHandle, list)	/* IMIV-271 */
{
  if (list)
    {
      LISTDECL();
      LISTBEGIN(list);
      LISTCALL(lCloseMsg, FALSE, (Rect *)0, * (Cell *) &HxX(list, clikLoc), 0,
	       0, list);
      LISTEND(list);

      DisposHandle((Handle) HxP(list, cells));
      if (HxP(list, hScroll))
	DisposeControl(HxP(list, hScroll));
      if (HxP(list, vScroll))
	DisposeControl(HxP(list, vScroll));
    }
#if defined (LIST_DEBUG)
  delete_list (list);
#endif  
}
