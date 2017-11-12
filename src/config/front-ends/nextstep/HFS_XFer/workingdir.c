#if defined(OUTDATEDCODE)
#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "myhfs.h"
#include <string.h>

/*
 * TODO: use this working directory stuff in ROMlib
 */

PUBLIC OSErr dirbusy(LONGINT dirid, HVCB *vcbp)
{
#if !defined(UNIX)
    wdentry *wdp, *ewdp;

    for(wdp = (wdentry *)(WDCBsPtr + sizeof(INTEGER)),
    ewdp = (wdentry *)(WDCBsPtr + *(INTEGER *)WDCBsPtr);
        wdp != ewdp; wdp++)
        ;
    return wdp == ewdp ? noErr : fBsyErr;
#else
    return noErr;
#endif
}

PUBLIC OSErr myPBOpenWD(WDPBPtr pb, BOOLEAN async)
{
    LONGINT dirid, procid;
    wdentry *wdp, *ewdp, *firstfreep;
    OSErr retval;
    filekind kind;
    btparam btparamrec;
    HVCB *vcbp;

    retval = findvcbandfile((ioParam *)pb, pb->ioWDDirID, &btparamrec, &kind,
                            false);
    if(retval != noErr)
        PBRETURN(pb, retval);
    vcbp = btparamrec.vcbp;
    retval = cleancache(vcbp);
    if(retval != noErr)
        PBRETURN(pb, retval);
    if(kind == directory)
        dirid = ((directoryrec *)DATAPFROMKEY(btparamrec.foundp))->dirDirID;
    else
        dirid = pb->ioWDDirID;
    procid = pb->ioWDProcID;

    firstfreep = 0;
    for(wdp = (wdentry *)(WDCBsPtr + sizeof(INTEGER)),
    ewdp = (wdentry *)(WDCBsPtr + *(INTEGER *)WDCBsPtr);
        wdp != ewdp; wdp++)
    {
        if(!firstfreep && !wdp->vcbp)
            firstfreep = wdp;
        if(wdp->vcbp == vcbp && wdp->dirid == dirid && wdp->procid == procid)
        {
            pb->ioVRefNum = WDPTOWDNUM(wdp);
            PBRETURN(pb, noErr);
        }
    }
    if(!firstfreep)
        retval = tmwdoErr;
    else
    {
        firstfreep->vcbp = vcbp;
        firstfreep->dirid = dirid;
        firstfreep->procid = procid;
        pb->ioVRefNum = WDPTOWDNUM(firstfreep);
        retval = noErr;
    }
    PBRETURN(pb, retval);
}

PUBLIC OSErr myPBCloseWD(WDPBPtr pb, BOOLEAN async)
{
    wdentry *wdp;
    OSErr retval;

    retval = noErr;
    if(ISWDNUM(pb->ioVRefNum))
    {
        wdp = WDNUMTOWDP(pb->ioVRefNum);
        if(wdp)
            wdp->vcbp = 0;
        else
            retval = nsvErr;
    }
    PBRETURN(pb, retval);
}

PUBLIC OSErr myGetWDInfo(WDPBPtr pb, BOOLEAN async)
{
    OSErr retval;
    wdentry *wdp, *ewdp;
    INTEGER i;

    retval = noErr;
    wdp = 0;
    if(pb->ioWDIndex > 0)
    {
        i = pb->ioWDIndex;
        wdp = (wdentry *)(WDCBsPtr + sizeof(INTEGER));
        ewdp = (wdentry *)(WDCBsPtr + *(INTEGER *)WDCBsPtr);
        if(pb->ioVRefNum < 0)
        {
            for(; wdp != ewdp; wdp++)
                if(wdp->vcbp->vcbVRefNum == pb->ioVRefNum && --i <= 0)
                    break;
        }
        else if(pb->ioVRefNum == 0)
        {
            for(; wdp != ewdp && --i > 0; wdp++)
                ;
        }
        else /* if (pb->ioVRefNum > 0 */
        {
            for(; wdp != ewdp; wdp++)
                if(wdp->vcbp->vcbDrvNum == pb->ioVRefNum && --i <= 0)
                    break;
        }
        if(wdp == ewdp)
            wdp = 0;
    }
    else if(ISWDNUM(pb->ioVRefNum))
        wdp = WDNUMTOWDP(pb->ioVRefNum);

    if(wdp)
    {
        if(pb->ioNamePtr)
            str255assign(pb->ioNamePtr, (StringPtr)wdp->vcbp->vcbVN);
        if(pb->ioWDIndex > 0)
            pb->ioVRefNum = wdp->vcbp->vcbVRefNum;
        pb->ioWDProcID = wdp->procid;
        pb->ioWDVRefNum = wdp->vcbp->vcbVRefNum;
        pb->ioWDDirID = wdp->dirid;
    }
    else
        retval = nsvErr;

    PBRETURN(pb, retval);
}

PUBLIC void adjustdirid(LONGINT *diridp, HVCB *vcbp, INTEGER vrefnum)
{
    wdentry *wdp;

    if(*(unsigned long *)diridp <= 1 && ISWDNUM(vrefnum))
    {
        wdp = WDNUMTOWDP(vrefnum);
        if(wdp->vcbp == vcbp)
            *diridp = wdp->dirid;
    }
}
#endif
