/* Copyright 1992 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "rsys/hfs.h"
#include "rsys/file.h"
#include "MemoryMgr.h"

using namespace Executor;

/*
 * TODO: use this working directory stuff in ROMlib
 */

OSErr Executor::ROMlib_dirbusy(LONGINT dirid, HVCB *vcbp)
{
#if defined(MAC)
    wdentry *wdp, *ewdp;

    for(wdp = (wdentry *)(CL(LM(WDCBsPtr)) + sizeof(INTEGER)),
    ewdp = (wdentry *)(CL(LM(WDCBsPtr)) + CW(*(INTEGER *)CL(LM(WDCBsPtr))));
        wdp != ewdp; wdp++)
        ;
    return wdp == ewdp ? noErr : fBsyErr;
#else
    return noErr;
#endif
}

OSErr Executor::ROMlib_mkwd(WDPBPtr pb, HVCB *vcbp, LONGINT dirid,
                            LONGINT procid)
{
    wdentry *wdp, *ewdp, *firstfreep;
    OSErr retval;
    INTEGER n_wd_bytes, new_n_wd_bytes;
    Ptr newptr;
    GUEST<THz> saveZone;

    firstfreep = 0;
    for(wdp = (wdentry *)(MR(LM(WDCBsPtr)) + sizeof(INTEGER)),
    ewdp = (wdentry *)(MR(LM(WDCBsPtr)) + CW(*(GUEST<INTEGER> *)MR(LM(WDCBsPtr))));
        wdp != ewdp; wdp++)
    {
        if(!firstfreep && !wdp->vcbp)
            firstfreep = wdp;
        if(MR(wdp->vcbp) == vcbp && CL(wdp->dirid) == dirid && CL(wdp->procid) == procid)
        {
            pb->ioVRefNum = CW(WDPTOWDNUM(wdp));
            /*-->*/ return noErr;
        }
    }
    if(!firstfreep)
    {
        n_wd_bytes = CW(*(GUEST<INTEGER> *)MR(LM(WDCBsPtr)));
        new_n_wd_bytes = (n_wd_bytes - sizeof(INTEGER)) * 2 + sizeof(INTEGER);
        saveZone = LM(TheZone);
        LM(TheZone) = LM(SysZone);
        newptr = NewPtr(new_n_wd_bytes);
        LM(SysZone) = LM(TheZone);
        if(!newptr)
            retval = tmwdoErr;
        else
        {
            BlockMoveData(MR(LM(WDCBsPtr)), newptr, n_wd_bytes);
            DisposPtr(MR(LM(WDCBsPtr)));
            LM(WDCBsPtr) = RM(newptr);
            *(GUEST<INTEGER> *)newptr = CW(new_n_wd_bytes);
            firstfreep = (wdentry *)(newptr + n_wd_bytes);
            retval = noErr;
        }
    }
    else
        retval = noErr;
    if(retval == noErr)
    {
        firstfreep->vcbp = RM(vcbp);
        firstfreep->dirid = CL(dirid);
        firstfreep->procid = CL(procid);
        pb->ioVRefNum = CW(WDPTOWDNUM(firstfreep));
        retval = noErr;
    }
    return retval;
}

OSErr Executor::hfsPBOpenWD(WDPBPtr pb, BOOLEAN async)
{
    LONGINT dirid;
    OSErr retval;
    filekind kind;
    btparam btparamrec;
    HVCB *vcbp;
    StringPtr namep;

    kind = (filekind)(regular | directory);
    retval = ROMlib_findvcbandfile((IOParam *)pb, Cx(pb->ioWDDirID),
                                   &btparamrec, &kind, false);
    if(retval != noErr)
        PBRETURN(pb, retval);
    vcbp = btparamrec.vcbp;
    retval = ROMlib_cleancache(vcbp);
    if(retval != noErr)
        PBRETURN(pb, retval);
    namep = MR(pb->ioNamePtr);
    if(kind == directory && namep && namep[0])
        dirid = CL(((directoryrec *)DATAPFROMKEY(btparamrec.foundp))->dirDirID);
    else
        dirid = CL(pb->ioWDDirID);
    retval = ROMlib_mkwd(pb, vcbp, dirid, CL(pb->ioWDProcID));

    PBRETURN(pb, retval);
}

OSErr Executor::hfsPBCloseWD(WDPBPtr pb, BOOLEAN async)
{
    wdentry *wdp;
    OSErr retval;

    retval = noErr;
    if(ISWDNUM(Cx(pb->ioVRefNum)))
    {
        wdp = WDNUMTOWDP(Cx(pb->ioVRefNum));
        if(wdp)
            wdp->vcbp = 0;
        else
            retval = nsvErr;
    }
    PBRETURN(pb, retval);
}

OSErr Executor::hfsPBGetWDInfo(WDPBPtr pb, BOOLEAN async)
{
    OSErr retval;
    wdentry *wdp, *ewdp;
    INTEGER i;
    BOOLEAN foundelsewhere;
    HVCB *vcbp;

    foundelsewhere = false;
    retval = noErr;
    wdp = 0;
    if(Cx(pb->ioWDIndex) > 0)
    {
        i = Cx(pb->ioWDIndex);
        wdp = (wdentry *)(MR(LM(WDCBsPtr)) + sizeof(INTEGER));
        ewdp = (wdentry *)(MR(LM(WDCBsPtr)) + CW(*(GUEST<INTEGER> *)MR(LM(WDCBsPtr))));
        if(Cx(pb->ioVRefNum) < 0)
        {
            for(; wdp != ewdp; wdp++)
                if(wdp->vcbp && MR(wdp->vcbp)->vcbVRefNum == pb->ioVRefNum && --i <= 0)
                    break;
        }
        else if(pb->ioVRefNum == CWC(0))
        {
            for(; wdp != ewdp && i > 1; ++wdp)
                if(wdp->vcbp)
                    --i;
        }
        else /* if (Cx(pb->ioVRefNum) > 0 */
        {
            for(; wdp != ewdp; wdp++)
                if(MR(wdp->vcbp)->vcbDrvNum == pb->ioVRefNum && --i <= 0)
                    break;
        }
        if(wdp == ewdp || !wdp->vcbp)
            wdp = 0;
    }
    else if(ISWDNUM(Cx(pb->ioVRefNum)))
        wdp = WDNUMTOWDP(Cx(pb->ioVRefNum));
    else
    {
        vcbp = ROMlib_findvcb(Cx(pb->ioVRefNum), (StringPtr)0, (LONGINT *)0,
                              true);
        if(vcbp)
        {
            if(pb->ioNamePtr)
                str255assign(MR(pb->ioNamePtr), (StringPtr)vcbp->vcbVN);
            pb->ioWDProcID = 0;
            pb->ioVRefNum = pb->ioWDVRefNum = vcbp->vcbVRefNum;
            pb->ioWDDirID = (vcbp == MR(LM(DefVCBPtr))) ? DefDirID : CLC(2);
            foundelsewhere = true;
        }
    }

    if(!foundelsewhere)
    {
        if(wdp)
        {
            if(pb->ioNamePtr)
                str255assign(MR(pb->ioNamePtr),
                             (StringPtr)MR(wdp->vcbp)->vcbVN);
            if(Cx(pb->ioWDIndex) > 0)
                pb->ioVRefNum = MR(wdp->vcbp)->vcbVRefNum;
            pb->ioWDProcID = wdp->procid;
            pb->ioWDVRefNum = MR(wdp->vcbp)->vcbVRefNum;
            pb->ioWDDirID = wdp->dirid;
        }
        else
            retval = nsvErr;
    }

    PBRETURN(pb, retval);
}

OSErr
Executor::GetWDInfo(INTEGER wd, GUEST<INTEGER> *vrefp, GUEST<LONGINT> *dirp, GUEST<LONGINT> *procp)
{
    OSErr retval;

    WDPBRec wdp;
    memset(&wdp, 0, sizeof wdp);
    wdp.ioVRefNum = CW(wd);
    retval = PBGetWDInfo(&wdp, false);
    if(retval == noErr)
    {
        *vrefp = wdp.ioVRefNum;
        *dirp = wdp.ioWDDirID;
        *procp = wdp.ioWDProcID;
    }
    return retval;
}

void Executor::ROMlib_adjustdirid(LONGINT *diridp, HVCB *vcbp, INTEGER vrefnum)
{
    wdentry *wdp;

    if(*(ULONGINT *)diridp <= 1 && ISWDNUM(vrefnum))
    {
        wdp = WDNUMTOWDP(vrefnum);
        if(MR(wdp->vcbp) == vcbp)
            *diridp = CL(wdp->dirid);
    }
    else if(*diridp == 0 && !vrefnum /* vcbp == CL(LM(DefVCBPtr)) */)
        *diridp = CL(DefDirID);
    if(*diridp == 0)
        *diridp = 2;
}
