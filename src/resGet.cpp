/* Copyright 1986, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ResourceMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "FileMgr.h"
#include "rsys/resource.h"
#include "MemoryMgr.h"
#include "OSUtil.h"
#include "rsys/glue.h"
#include "rsys/mman.h"

#include "rsys/ctl.h"
#include "rsys/list.h"
#include "rsys/menu.h"
#include "rsys/wind.h"
#include "rsys/soundopts.h"
#include "rsys/file.h"
#include "rsys/pstuff.h"
#include "rsys/aboutbox.h"

#include <ctype.h>

namespace Executor
{
PRIVATE INTEGER countmapresources(resmaphand, ResType);
PRIVATE Handle getindmapresource(resmaphand, ResType,
                                 INTEGER *);
PRIVATE Handle getnamedmapresource(resmaphand, ResType, StringPtr);
}

using namespace Executor;

P1(PUBLIC pascal trap, void, SetResLoad, BOOLEAN, load)
{
    ResLoad = load;
}

A2(PRIVATE, INTEGER, countmapresources, resmaphand, map, ResType, typ)
{
    INTEGER i, retval;
    typref *tr;

    retval = 0;
    WALKTR(map, i, tr)
    if(CL(tr->rtyp) == typ)
    {
        retval += 1 + Cx(tr->nres);
        break;
    }
    EWALKTR(tr)
    return (retval);
}

P1(PUBLIC pascal trap, INTEGER, CountResources, ResType, typ)
{
    resmaphand map;
    INTEGER n;

    ROMlib_setreserr(noErr);
    n = 0;
    WALKMAPTOP(map)
    n += countmapresources(map, typ);
    EWALKMAP()
    return (n);
}

P1(PUBLIC pascal trap, INTEGER, Count1Resources, /* IMIV-15 */
   ResType, typ)
{
    resmaphand map;

    map = ROMlib_rntohandl(Cx(CurMap), (Handle *)0);
    if(!map)
    {
        ROMlib_setreserr(resFNotFound);
        /*-->*/ return 0;
    }
    return (countmapresources(map, typ));
}

A3(PRIVATE, Handle, getindmapresource, resmaphand, map, ResType, typ,
   INTEGER *, indx)
{
    INTEGER i, j, nr;
    typref *tr;
    resref *rr;

    WALKTR(map, i, tr)
    if(CL(tr->rtyp) == typ)
    {
        nr = Cx(tr->nres) + 1;
        WALKRR(map, tr, j, rr)
        if(--*indx == 0)
            /*-->*/ return ROMlib_mgetres(map, rr);
        EWALKRR(rr)
    }
    EWALKTR(tr)
    return ((Handle)0);
}

P2(PUBLIC pascal trap, Handle, GetIndResource, ResType, typ, INTEGER, indx)
{
    resmaphand map;
    Handle retval;

    if(indx <= 0)
    {
        ROMlib_setreserr(resNotFound);
        /*-->*/ return 0;
    }
    WALKMAPTOP(map)
    retval = getindmapresource(map, typ, &indx);
    if(retval)
    {
        ROMlib_setreserr(noErr);
        /*-->*/ return retval;
    }
    EWALKMAP()
    ROMlib_setreserr(resNotFound);
    return 0;
}

P2(PUBLIC pascal trap, Handle, Get1IndResource, ResType, typ, /* IMIV-15 */
   INTEGER, i)
{
    resmaphand map;
    Handle retval;
    INTEGER tempi;

    map = ROMlib_rntohandl(Cx(CurMap), (Handle *)0);
    if(!map)
    {
        ROMlib_setreserr(resFNotFound);
        /*-->*/ return 0;
    }
    tempi = i;
    if(i <= 0 || !(retval = getindmapresource(map, typ, &tempi)))
    {
        ROMlib_setreserr(resNotFound);
        /*-->*/ return 0;
    }
    else
    {
        ROMlib_setreserr(noErr);
        return retval;
    }
}

/* ROMlib_maptypidtop: given a map, typ and an id,
				    *ptr is filled in appropriately */

A4(PUBLIC, OSErr, ROMlib_maptypidtop, resmaphand, map, /* INTERNAL */
   ResType, typ, INTEGER, id, resref **, ptr)
{
    INTEGER i, j;
    typref *tr;
    resref *rr;

    WALKTR(map, i, tr)
    if(CL(tr->rtyp) == typ)
    {
        WALKRR(map, tr, j, rr)
        if(Cx(rr->rid) == id)
        {
            *ptr = rr;
            return (noErr);
        }
        EWALKRR(rr)
    }
    EWALKTR(tr)
    return resNotFound;
}

/* ROMlib_typidtop:  given a type and an id,
		ROMlib_typidtop fills in *pth and *ptr
                with a resmap handle and a resref pointer */

A4(PUBLIC, OSErr, ROMlib_typidtop, ResType, typ, INTEGER, id, /* INTERNAL */
   resmaphand *, pth, resref **, ptr)
{
    resmaphand map;

    WALKMAPCUR(map)
    if(ROMlib_maptypidtop(map, typ, id, ptr) == noErr)
    {
        *pth = map;
        return noErr;
    }
    EWALKMAP()

    warn_resource_not_found(typ, id);
    return resNotFound;
}

#define MAELSTROM_HACK

#define NUM_ROMLIB_DEFS 10

#if !defined(MAELSTROM_HACK)
PRIVATE GUEST<LONGINT> ROMlib_defs[NUM_ROMLIB_DEFS];
#endif

/*
 * Don't change the order of the routines below.
 * We have made stubs of the form:
 *
 *	0x20780058	movel	0x58:w,		a0
 *	0x206800nn	movel	a0@(nn),	a0
 *	0x4ED0		jmp			a0@
 * 
 * nn is the offset into ROMlib_defs (0, 4, 8, ...)
 */

PRIVATE BOOLEAN acceptable(unsigned long addr)
{
    return !(islower(addr & 0xFF) || islower((addr >> 8) & 0xFF) || islower((addr >> 16) & 0xFF) || islower((addr >> 24) & 0xFF));
}

PRIVATE void ROMlib_init_xdefs(void)
{
#if !defined(MAELSTROM_HACK)
    ROMlib_defs[0] = guest_cast<LONGINT>(RM(P_cdef0));
    ROMlib_defs[1] = guest_cast<LONGINT>(RM(P_cdef16));
    ROMlib_defs[2] = guest_cast<LONGINT>(RM(P_wdef0));
    ROMlib_defs[3] = guest_cast<LONGINT>(RM(P_wdef16));
    ROMlib_defs[4] = guest_cast<LONGINT>(RM(P_mdef0));
    ROMlib_defs[5] = guest_cast<LONGINT>(RM(P_ldef0));
    ROMlib_defs[6] = guest_cast<LONGINT>(RM(P_mbdf0));
    ROMlib_defs[7] = guest_cast<LONGINT>(RM(P_snth5));
    ROMlib_defs[8] = guest_cast<LONGINT>(RM(P_unixmount));
    ROMlib_defs[9] = guest_cast<LONGINT>(RM(P_cdef1008));

    *(LONGINT *)SYN68K_TO_US(0x58) = RM((LONGINT)ROMlib_defs);
#else
    GUEST<THz> save_zone;
    Handle oldhandle, newhandle;
    long timeout;
    GUEST<LONGINT> *ROMlib_defs;

    save_zone = TheZone;

    TheZone = SysZone;
    newhandle = 0;
    timeout = 64000;
    do
    {
        oldhandle = newhandle;
        newhandle = NewHandle(NUM_ROMLIB_DEFS * sizeof(ROMlib_defs[0]));
        if(oldhandle)
            DisposHandle(oldhandle);
    } while(!acceptable((*newhandle).raw()) && --timeout);
#if !defined(NDEBUG)
    if(!timeout)
        warning_unexpected("Maelstrom hack didn't work");
#endif
    HLock(newhandle);
    ROMlib_defs = (GUEST<LONGINT> *)STARH(newhandle);
    ROMlib_defs[0] = guest_cast<LONGINT>(RM(P_cdef0));
    ROMlib_defs[1] = guest_cast<LONGINT>(RM(P_cdef16));
    ROMlib_defs[2] = guest_cast<LONGINT>(RM(P_wdef0));
    ROMlib_defs[3] = guest_cast<LONGINT>(RM(P_wdef16));
    ROMlib_defs[4] = guest_cast<LONGINT>(RM(P_mdef0));
    ROMlib_defs[5] = guest_cast<LONGINT>(RM(P_ldef0));
    ROMlib_defs[6] = guest_cast<LONGINT>(RM(P_mbdf0));
    ROMlib_defs[7] = guest_cast<LONGINT>(RM(P_snth5));
    ROMlib_defs[8] = guest_cast<LONGINT>(RM(P_unixmount));
    ROMlib_defs[9] = guest_cast<LONGINT>(RM(P_cdef1008));
    *(LONGINT *)SYN68K_TO_US(0x58) = (LONGINT)(*newhandle).raw(); // ### use standard low mem access method
    TheZone = save_zone;
#endif
}

typedef struct
{
    ResType type;
    INTEGER id;
} pseudo_rom_entry_t;

PRIVATE Handle
pseudo_get_rom_resource(ResType typ, INTEGER id)
{
    Handle retval;
    int i;
    static pseudo_rom_entry_t pseudo_rom_table[] = {
        { T('F', 'O', 'N', 'D'), 3 }
    };

    for(i = 0; (i < (int)NELEM(pseudo_rom_table) && (pseudo_rom_table[i].type != typ
                                                     || pseudo_rom_table[i].id != id));
        ++i)
        ;
    if(i < (int)NELEM(pseudo_rom_table))
    {
        GUEST<INTEGER> save_map;

        save_map = CurMap;
        CurMap = SysMap;
        retval = C_Get1Resource(typ, id);
        CurMap = save_map;
    }
    else
        retval = 0;
    return retval;
}

#if defined(ULTIMA_III_HACK)
PUBLIC bool Executor::ROMlib_ultima_iii_hack;
#endif

P2(PUBLIC pascal trap, Handle, GetResource, ResType, typ, INTEGER, id)
{
    resmaphand map;
    resref *rr;
    static int beenhere = 0;
    Handle retval;

    if(!beenhere)
    {
        beenhere = 1;
        ROMlib_init_xdefs();
    }

    retval = pseudo_get_rom_resource(typ, id);
    if(retval)
        /*-->*/ return retval;

#define BOBSEYESHACK
#if defined(BOBSEYESHACK)
    /*
 * This gets around a bug in Bob's Eyes that references stray memory.
 * I'm trying to find the author of the program and get him to fix it.
 *  	--Cliff Sat Sep  3 08:02:51 MDT 1994
 */
    if(typ == TICK("rAnd") && id == 128)
        return 0;
#endif

#if !defined(MAC)
    switch(typ)
    { /* fake out code resources */
#define ICKYHACK
#if defined(ICKYHACK)
        case T('P', 'A', 'C', 'K'):
            return GetResource(TICK("ALRT"), -3995);
#endif /* ICKYHACK */
    }
#endif /* !defined(MAC) */

    ROMlib_setreserr(ROMlib_typidtop(typ, id, &map, &rr));
    if(Cx(ResErr) == resNotFound)
    {
        ROMlib_setreserr(noErr);
        retval = 0; /* IMIV */
    }
    else
    {
        if(ResErr == CWC(noErr))
            retval = ROMlib_mgetres(map, rr);
        else
            retval = 0;
    }
#if defined(ULTIMA_III_HACK)
    if(ROMlib_ultima_iii_hack && typ == TICK("PREF") && retval)
    {
        *((char *)STARH(retval) + 1) = 0; /* turn off Music prefs */
    }
#endif
    return retval;
}

P2(PUBLIC pascal trap, Handle, Get1Resource, ResType, typ, /* IMIV-16 */
   INTEGER, id)
{
    Handle retval;
    resmaphand map;
    resref *rr;

    map = ROMlib_rntohandl(Cx(CurMap), NULL);
    if(!map)
    {
        ROMlib_setreserr(resFNotFound);
        return NULL;
    }
    ROMlib_setreserr(ROMlib_maptypidtop(map, typ, id, &rr));
    if(ResErr == CWC(noErr))
        retval = ROMlib_mgetres(map, rr);
    else
    {
        warn_resource_not_found(typ, id);
        retval = NULL;
    }
    if(ResErr == CWC(resNotFound))
        /* IMIV */
        ResErr = CWC(noErr);
    return retval;
}

A3(PRIVATE, Handle, getnamedmapresource, resmaphand, map, ResType, typ,
   StringPtr, nam)
{
    INTEGER i, j;
    typref *tr;
    resref *rr;

    WALKTR(map, i, tr)
    if(CL(tr->rtyp) == typ)
    {
        WALKRR(map, tr, j, rr)
        if(Cx(rr->noff) != -1 && EqualString((StringPtr)(
                                                 (char *)STARH(map) + Hx(map, namoff) + Cx(rr->noff)),
                                             (StringPtr)nam, 0, 1))
            /*-->*/ return ROMlib_mgetres(map, rr);
        EWALKRR(rr)
    }
    EWALKTR(tr)
    ROMlib_setreserr(resNotFound);
    return (0);
}

P2(PUBLIC pascal trap, Handle, GetNamedResource, ResType, typ, StringPtr, nam)
{
    Handle retval;

    retval = NULL;
    if(EqualString(nam, about_box_menu_name_pstr, true, true))
    {
        static Handle phoney_hand;

        if(!phoney_hand)
            phoney_hand = NewHandleSys(0);
        retval = phoney_hand;
    }
    else
    {
        resmaphand map;

        WALKMAPCUR(map)
        if((retval = getnamedmapresource(map, typ, nam)))
        {
            ROMlib_setreserr(noErr);
            goto DONE;
        }
        EWALKMAP()
        warn_resource_not_found_name(typ, nam);
        ROMlib_setreserr(resNotFound);
    }
DONE:
    return retval;
}

P2(PUBLIC pascal trap, Handle, Get1NamedResource, ResType, typ, /* IMIV-16 */
   StringPtr, s)
{
    resmaphand map;

    map = ROMlib_rntohandl(Cx(CurMap), (Handle *)0);
    if(!map)
    {
        ROMlib_setreserr(resFNotFound);
        /*-->*/ return 0;
    }
    return (getnamedmapresource(map, typ, s));
}

P1(PUBLIC pascal trap, void, LoadResource, Handle volatile, res)
{
    resmaphand map;
    typref *tr;
    resref *rr;
    GUEST<int16_t> savemap;

    volatile LONGINT savea0d0[2];
    savea0d0[0] = EM_D0;
    savea0d0[1] = EM_A0;

    if(!res)
    {
        ROMlib_setreserr(nilHandleErr);
        return;
    }

    if(*res)
    {
        ROMlib_setreserr(noErr);
    }
    else
    {
        savemap = CurMap;
        CurMap = ((resmap *)STARH(MR(TopMapHndl)))->resfn;
        ROMlib_setreserr(ROMlib_findres(res, &map, &tr, &rr));
        CurMap = savemap;
        if(ResErr == CWC(noErr))
        {
            BOOLEAN save_resload;

            save_resload = ResLoad;
            SetResLoad(true);
            ROMlib_mgetres(map, rr);
            ResLoad = save_resload;
        }
    }

    EM_D0 = savea0d0[0];
    EM_A0 = savea0d0[1];
}

P1(PUBLIC pascal trap, void, ReleaseResource, Handle, res)
{
    resmaphand map;
    typref *tr;
    resref *rr;
    Handle h;

    if(ROMlib_setreserr(ROMlib_findres(res, &map, &tr, &rr)))
        /*-->*/ return;
    if(Cx(rr->ratr) & resChanged)
        ROMlib_wr(map, rr);
    if(ResErr != CWC(noErr) || !rr->rhand)
        return;
    h = MR(rr->rhand);
    if(*h)
        HClrRBit(h);
    DisposHandle(h);
    rr->rhand = 0;
}

P1(PUBLIC pascal trap, void, DetachResource, Handle, res)
{
    resmaphand map;
    typref *tr;
    resref *rr;
    Handle h;

    if(ROMlib_setreserr(ROMlib_findres(res, &map, &tr, &rr)))
        /*-->*/ return;
    if(ResErr != CWC(noErr) || !(h = (Handle)MR(rr->rhand)))
        return;
    if(Cx(rr->ratr) & resChanged)
    {
        ROMlib_setreserr(resAttrErr); /* IV-18 */
        return;
    }
    rr->rhand = 0;
    if(*h)
        HClrRBit(h);
}
