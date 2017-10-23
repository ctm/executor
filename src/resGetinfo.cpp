/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_resGetinfo[] =
	    "$Id: resGetinfo.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in ResourceMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "FileMgr.h"
#include "rsys/resource.h"
#include "rsys/glue.h"
#include "rsys/file.h"
#include "rsys/prefs.h"

#define STEF_GetResInfoFix

using namespace Executor;

P1(PUBLIC pascal trap, INTEGER, UniqueID, ResType, typ)
{
    static INTEGER startid = 0;
    GUEST<INTEGER> curmap;
    resmaphand map;
    resref *rr;
    
    curmap = CurMap;
    CurMap = ((resmap *) STARH(MR(TopMapHndl)))->resfn;
    while (ROMlib_typidtop(typ, ++startid, &map, &rr) != resNotFound)
        ;
    CurMap = curmap;
    return(startid);
}

P1(PUBLIC pascal trap, INTEGER, Unique1ID, ResType, typ)  /* IMIV-16 */
{
    resmaphand map;
    static INTEGER startid = 0;
    resref *rr;
    
    map = ROMlib_rntohandl(Cx(CurMap), (Handle *)0);
    if (!map) {
        ROMlib_setreserr(resFNotFound);
/*-->*/ return 0;
    }
    while (ROMlib_maptypidtop(map, typ, ++startid, &rr) != resNotFound)
        ;
    return(startid);
}

P4(PUBLIC pascal trap, void, GetResInfo, Handle, res, GUEST<INTEGER> *, id,
					       GUEST<ResType> *, typ, StringPtr, name)
{
    resmaphand map;
    typref *tr;
    resref *rr;

    ROMlib_setreserr(ROMlib_findres(res, &map, &tr, &rr));

#if !defined (STEF_GetResInfoFix)
    if (ResErr != CWC(noErr))
        return;
#else
    if (ResErr != CWC(noErr))
      {
	if (id)
	  *id = CWC(-1);
	if (typ)
	  *typ = (ResType) 0;
	if (name)
	    name[0] = 0;
/*-->*/ return;
      }
#endif
    if (id)
      *id = rr->rid;
    if (typ)
      *typ = tr->rtyp;
    if (name) {
	if (rr->noff != CWC(-1))
	    str255assign(name,
			 (StringPtr)((char *)STARH(map) +Hx(map, namoff) + Cx(rr->noff)));
	else
	    name[0] = 0;
    }
}

P1(PUBLIC pascal trap, INTEGER, GetResAttrs, Handle, res)
{
    resmaphand map;
    typref *tr;
    resref *rr;

    ROMlib_setreserr(ROMlib_findres(res, &map, &tr, &rr));
    if (ResErr != CWC(noErr))
/*-->*/ return 0;
    return(Cx(rr->ratr));
}

A2(PUBLIC, LONGINT, ROMlib_SizeResource, Handle, res, BOOLEAN, usehandle)
{
    resmaphand map;
    typref *tr;
    resref *rr;
    Size retval;
    LONGINT loc, lc;

    ROMlib_setreserr(ROMlib_findres(res, &map, &tr, &rr));
    if (ResErr != CWC(noErr))
/*-->*/ return -1;

    if (usehandle && *res)	/* STARH is overkill */
	retval = GetHandleSize(res);
    else {
	loc = Hx(map, rh.rdatoff) + B3TOLONG(rr->doff);
	ROMlib_setreserr(SetFPos(Hx(map, resfn), fsFromStart, loc));
	if (ResErr != CWC(noErr))
/*-->*/	    return -1;

	/* If the resource is compressed and we want the memory size, not
	   the disk size, we have to look at more than the length on the 
	   disk */

	if (usehandle && (rr->ratr & resCompressed) && system_version >= 0x700)
	  {
	    GUEST<LONGINT> l[4]; /* [0] == diskSize,
			     [1] == compressedResourceTag, 
			     [2] == typeFlags,
			     [3] == uncompressedSize */
	    LONGINT master_save_pos;

	    lc = sizeof (l);
	    GetFPos (Hx(map, resfn), &master_save_pos);
	    ROMlib_setreserr(FSReadAll(Hx(map, resfn), &lc, (Ptr) l));
	    if (ResErr != CWC(noErr) || l[1] != CLC (COMPRESSED_TAG))
	      {
		SetFPos (Hx(map, resfn), fsFromStart, master_save_pos);
		goto not_compressed_after_all;
	      }
	    else
	      {
		if (l[2] != CLC (COMPRESSED_FLAGS))
		  {
		    ROMlib_setreserr (CantDecompress);
		    return -1;
		  }
		else
		  retval = CL (l[3]);
	      }
	  }
	else
	  {
not_compressed_after_all:	    
            lc = sizeof(retval);
            GUEST<Size> tmpRet;
	    ROMlib_setreserr(FSReadAll(Hx(map, resfn), &lc, (Ptr) &tmpRet));
	    retval = CL(tmpRet);
	    if (ResErr != CWC(noErr))
/*-->*/	      return -1;
	  }
    }

    return retval;
}

P1(PUBLIC pascal trap, LONGINT, SizeResource, Handle, res)
{
    return ROMlib_SizeResource(res, TRUE);
}
