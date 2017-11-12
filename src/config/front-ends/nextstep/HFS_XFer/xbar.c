#include "rsys/common.h"
#include "myhfs.h"

/* #define ORIG */

#if defined(OUTDATEDCODE)
#if defined(CACHECHECK)
void cachecheck(HVCB *vcbp)
{
    cacheentry *cachep;
    cachehead *headp;
    INTEGER i;

    headp = (cachehead *)vcbp->vcbCtlBuf;
    for(i = headp->nitems, cachep = headp->flink; --i >= 0;
        cachep = cachep->flink)
        if(cachep->flags & CACHEBUSY)
            DebugStr((StringPtr) "\pbusy");
}
#endif /* defined(CACHECHECK) */

BOOLEAN myvol(ioParam *pb)
{
    HVCB *vcbp;
    LONGINT dir;

#if defined(ORIG)
    return false;
#endif
    vcbp = findvcb(pb->ioVRefNum, pb->ioNamePtr, &dir);
#if !defined(UNIX)
#if defined(CACHECHECK)
    cachecheck(vcbp);
#endif /* defined(CACHECHECK) */
    return strncmp((char *)vcbp->vcbVN, "\pMyVol", vcbp->vcbVN[0] + 1) == 0;
#else
    if(!vcbp)
        return false; /* hopefully is a messed up working dir reference */
    if(vcbp->vcbCTRef)
    {
#if defined(CACHECHECK)
        cachecheck(vcbp);
#endif /* defined(CACHECHECK) */
        return true;
    }
    else
        return false;
#endif
}

BOOLEAN myfil(ioParam *pb)
{
    filecontrolblock *fcbp;
    HVCB *vcbp;

#if defined(ORIG)
    return false;
#endif
    fcbp = refnumtofcbp(pb->ioRefNum);
    if(fcbp)
    {
        vcbp = fcbp->fcbVPtr;
#if !defined(UNIX)
#if defined(CACHECHECK)
        cachecheck(vcbp);
#endif /* defined(CACHECHECK) */
        return strncmp((char *)vcbp->vcbVN, "\pMyVol", vcbp->vcbVN[0] + 1) == 0;
#else
        if(vcbp->vcbCTRef)
        {
#if defined(CACHECHECK)
            cachecheck(vcbp);
#endif /* defined(CACHECHECK) */
            return true;
        }
        else
            return false;
#endif
    }
    else
        return false;
}
#endif

PUBLIC OSErr xPBHRename(HFileParam *pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myvol((ioParam *)pb))
        return myPBHRename(pb, async);
    else
#endif
        return PBHRename((HParmBlkPtr)pb, async);
}

PUBLIC OSErr xPBHCreate(HFileParam *pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myvol((ioParam *)pb))
        return myPBHCreate(pb, async);
    else
#endif
        return PBHCreate((HParmBlkPtr)pb, async);
}

PUBLIC OSErr xPBDirCreate(HFileParam *pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myvol((ioParam *)pb))
        return myPBDirCreate(pb, async);
    else
#endif
        return PBDirCreate((HParmBlkPtr)pb, async);
}

PUBLIC OSErr xPBHDelete(HFileParam *pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myvol((ioParam *)pb))
        return myPBHDelete(pb, async);
    else
#endif
        return PBHDelete((HParmBlkPtr)pb, async);
}

PUBLIC OSErr xPBRead(ioParam *pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myfil(pb))
        return myPBRead(pb, async);
    else
#endif
        return PBRead((ParmBlkPtr)pb, async);
}

PUBLIC OSErr xPBWrite(ioParam *pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myfil(pb))
        return myPBWrite(pb, async);
    else
#endif
        return PBWrite((ParmBlkPtr)pb, async);
}

PUBLIC OSErr xPBClose(ioParam *pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myfil(pb))
        return myPBClose(pb, async);
    else
#endif
        return PBClose((ParmBlkPtr)pb, async);
}

PUBLIC OSErr xPBHOpen(HFileParam *pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myvol((ioParam *)pb))
        return myPBHOpen(pb, async);
    else
#endif
        return PBHOpen((HParmBlkPtr)pb, async);
}

PUBLIC OSErr xPBHOpenRF(HFileParam *pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myvol((ioParam *)pb))
        return myPBHOpenRF(pb, async);
    else
#endif
        return PBHOpenRF((HParmBlkPtr)pb, async);
}

PUBLIC OSErr xPBGetCatInfo(CInfoPBPtr pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myvol((ioParam *)pb))
        return myPBGetCatInfo(pb, async);
    else
#endif
        return PBGetCatInfo(pb, async);
}

PUBLIC OSErr xPBSetCatInfo(CInfoPBPtr pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myvol((ioParam *)pb))
        return myPBSetCatInfo(pb, async);
    else
#endif
        return PBSetCatInfo(pb, async);
}

PUBLIC OSErr xPBCatMove(CMovePBPtr pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myvol((ioParam *)pb))
        return myPBCatMove(pb, async);
    else
#endif
        return PBCatMove(pb, async);
}

PUBLIC OSErr xPBGetVInfo(volumeParam *pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myvol((ioParam *)pb))
        return myPBGetVInfo(pb, async);
    else
#endif
        return PBGetVInfo((ParmBlkPtr)pb, async);
}

PUBLIC OSErr xPBUnmountVol(volumeParam *pb)
{
#if defined(OUTDATEDCODE)
    if(myvol((ioParam *)pb))
        return myPBUnmountVol(pb);
    else
#endif
        return PBUnmountVol((ParmBlkPtr)pb);
}

PUBLIC OSErr xPBEject(volumeParam *pb)
{
#if defined(OUTDATEDCODE)
    if(myvol((ioParam *)pb))
        return myPBEject(pb);
    else
#endif
        return PBEject((ParmBlkPtr)pb);
}

PUBLIC OSErr xPBAllocate(ParmBlkPtr pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myfil((ioParam *)pb))
        return myPBAllocate((ioParam *)pb, async);
    else
#endif
        return PBAllocate((ParmBlkPtr)pb, async);
}

PUBLIC OSErr xPBAllocContig(ParmBlkPtr pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myfil((ioParam *)pb))
        return myPBAllocContig((ioParam *)pb, async);
    else
#endif
        return PBAllocContig((ParmBlkPtr)pb, async);
}

PUBLIC OSErr xPBHGetFInfo(ParmBlkPtr pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myvol((ioParam *)pb))
        return myPBHGetFInfo((HFileParam *)pb, async);
    else
#endif
        return PBHGetFInfo((HParmBlkPtr)pb, async);
}

PUBLIC OSErr xPBSetEOF(ParmBlkPtr pb, BOOLEAN async)
{
#if defined(OUTDATEDCODE)
    if(myfil((ioParam *)pb))
        return myPBSetEOF((ioParam *)pb, async);
    else
#endif
        return PBSetEOF((ParmBlkPtr)pb, async);
}
