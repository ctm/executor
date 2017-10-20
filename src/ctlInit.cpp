/* Copyright 1986, 1988, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_ctlInit[] =
		"$Id: ctlInit.c 86 2005-05-25 00:47:12Z ctm $";
#endif

/* Forward declarations in ControlMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "WindowMgr.h"
#include "ControlMgr.h"
#include "ToolboxUtil.h"
#include "ResourceMgr.h"
#include "MemoryMgr.h"
#include "OSUtil.h"

#include "rsys/ctl.h"
#include "rsys/wind.h"
#include "rsys/cquick.h"
#include "rsys/resource.h"

using namespace Executor;

PUBLIC BOOLEAN Executor::ROMlib_cdef0_is_rectangular = FALSE;

P9(PUBLIC pascal trap, ControlHandle, NewControl, WindowPtr, wst, Rect *, r,
		  StringPtr, title, BOOLEAN, vis, INTEGER, value, INTEGER, min,
		  INTEGER, max, INTEGER, procid, LONGINT, rc)	/* IMI-319 */
{
    ControlHandle retval;
    GrafPtr gp;
    AuxCtlHandle tmp, auxctlhead;
    Handle temph;
    
    retval = (ControlHandle) NewHandle((Size) sizeof(ControlRecord));
    temph = RM(GetResource(TICK("CDEF"), procid>>4));
    if (!(HxX(retval, contrlDefProc) = temph)) {
        DisposHandle((Handle) retval);
        return 0;
    }

    if ((procid>>4) == 0)
      {
	GUEST<INTEGER> id;
	GUEST<ResType> type;
	Str255 name;

	GetResInfo (MR (temph), &id, &type, name);
	ROMlib_cdef0_is_rectangular =
	  (EqualString (name,
			(StringPtr) "\023rectangular buttons", FALSE, FALSE));
      }

    if (!title)
	title = (StringPtr) "";	/* nothing ("\p" == "") */
    gp = thePort;
    SetPort ((GrafPtr) wst);
    
    tmp = MR(AuxCtlHead);
    auxctlhead = (AuxCtlHandle) NewHandle(sizeof(AuxCtlRec));
    AuxCtlHead = RM(auxctlhead);

    HASSIGN_6
      (auxctlhead,
       acNext, RM (tmp),
       acOwner, RM (retval),
       acCTable, (CCTabHandle) RM (GetResource (TICK("cctb"), 0)),
       acFlags, CW (procid & 0x0F),
       acReserved, CLC (0),
       acRefCon, CLC (0));
    str255assign (CTL_TITLE (retval), title);

    if (!HxX(auxctlhead, acCTable))
      {
	warning_unexpected ("no 'cctb', 0, probably old system file ");
	HxX(auxctlhead, acCTable) = (CCTabHandle) RM (default_ctl_ctab);
      }

    HASSIGN_11
      (retval,
       contrlRect, *r,
       nextControl, WINDOW_CONTROL_LIST_X (wst),
       contrlOwner, RM (wst),
       contrlVis, vis ? 255 : 0,
       contrlHilite, 0,
       contrlValue, CW (value),
       contrlMin, CW (min),
       contrlMax, CW (max),
       contrlData, CLC_NULL,
       contrlAction, CLC_NULL,
       contrlRfCon, CL (rc));
    
    WINDOW_CONTROL_LIST_X (wst) = RM (retval);
    
    
    CTL_CALL_EXCURSION
      (retval,
       {
	 CTLCALL (retval, initCntl, 0);
	 if (WINDOW_VISIBLE_X (wst) && vis)
	   CTLCALL (retval, drawCntl, 0);
       });
    
    SetPort (gp);
    return retval;
}

P2(PUBLIC pascal trap, ControlHandle, GetNewControl,		/* IMI-321 */
					        INTEGER, cid, WindowPtr, wst)
{
    typedef contrlrestype *wp;
    
    GUEST<wp> *wh;
    ControlHandle retval;
    Handle ctab_res_h;

    wh = (GUEST<wp> *) GetResource(TICK("CNTL"), cid);
    if (!wh)
        return 0;
    if (!(*wh))
	LoadResource((Handle) wh);
    ctab_res_h = ROMlib_getrestid (TICK ("cctb"), cid);
    retval = NewControl(wst, &(HxX(wh, _crect)),
	    (StringPtr) ((char *) &HxX(wh, _crect) + 22),	/* _ctitle */
            Hx(wh, _cvisible) ? 255 : 0, Hx(wh, _cvalue), Hx(wh, _cmin),
	    Hx(wh, _cmax), Hx(wh, _cprocid),
	    CL(*(LONGINT *)((char *)&HxX(wh, _crect) + 18)));	/* _crefcon */
    if (ctab_res_h)
      SetCtlColor (retval, (CCTabHandle) ctab_res_h);
    return retval;
}

P2 (PUBLIC pascal trap, void, SetCtlColor, ControlHandle, ctl, CCTabHandle, ctab)
{
  AuxCtlHandle aux_c;

  if (ctl)
    {
      aux_c = MR (*lookup_aux_ctl (ctl));
      if (!aux_c)
	{
	  AuxCtlHandle t_aux_c;
	  
	  /* allocate one */
	  t_aux_c = MR (AuxCtlHead);
	  aux_c = (AuxCtlHandle) NewHandle (sizeof (AuxCtlRec));
	  AuxCtlHead = RM (aux_c);
	  HxX(aux_c, acNext) = RM (t_aux_c);
	  HxX(aux_c, acOwner) = RM (ctl);
	  
	  HxX (aux_c, acCTable) = RM (ctab);
	  
	  HxX(aux_c, acFlags) = 0;
	  HxX(aux_c, acReserved) = 0;
	  HxX(aux_c, acRefCon) = 0;
	}
      else
	HxX (aux_c, acCTable) = RM (ctab);

      if (CTL_VIS (ctl))
	Draw1Control (ctl);
    }
  else
    {
      abort ();
    }
}

P1(PUBLIC pascal trap, void, DisposeControl, ControlHandle, c)	/* IMI-321 */
{
    
    GUEST<ControlHandle> *t;
    
    GUEST<AuxCtlHandle> *auxhp;
    AuxCtlHandle saveauxh;
    
    HideControl(c);
    CTL_CALL_EXCURSION
      (c,
       {
	 CTLCALL(c, dispCntl, 0);
       });
    
    for (t = (GUEST<ControlHandle> *) &(((WindowPeek)(HxP(c, contrlOwner)))->controlList);
	      (*t) && STARH(t) != c; t = (GUEST<ControlHandle> *) &((STARH(STARH(t)))->nextControl))
        ;
    if ((*t))
        (*t) = HxX(c, nextControl);
    for (auxhp = (GUEST<AuxCtlHandle> *) &AuxCtlHead; (*auxhp) && MR((STARH(STARH(auxhp)))->acOwner) != c;
					auxhp = (GUEST<AuxCtlHandle> *) &(STARH(STARH(auxhp)))->acNext)
	;
    if ((*auxhp)) {
	saveauxh = STARH(auxhp);
	(*auxhp) = STARH(STARH(auxhp))->acNext;
	DisposHandle((Handle) saveauxh);
    }

    DisposHandle((Handle) c);
}

P1(PUBLIC pascal trap, void, KillControls, WindowPtr, w)	/* IMI-321 */
{
    ControlHandle c, t;
    
    for (c = MR(((WindowPeek)w)->controlList); c ; ) {
        t = c;
        c = HxP(c, nextControl);
        DisposeControl(t);
    }
}
