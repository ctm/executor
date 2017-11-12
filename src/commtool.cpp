/* Copyright 1999 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_commtool[] = "$Id: commtool.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#include "CommTool.h"
#include "MemoryMgr.h"
#include "OSUtil.h"

#include "rsys/blockinterrupts.h"
#include "rsys/string.h"
#include "rsys/mman.h"

using namespace Executor;

PUBLIC INTEGER
Executor::CRMGetCRMVersion(void)
{
    INTEGER retval;

    retval = curCRMVersion;
    return retval;
}

PRIVATE QHdr commtool_head;

PUBLIC QHdrPtr
Executor::CRMGetHeader(void)
{
    QHdrPtr retval;

    retval = &commtool_head;
    return retval;
}

PUBLIC void
Executor::CRMInstall(QElemPtr qp)
{
    virtual_int_state_t block;

    block = block_virtual_ints();
    if(!commtool_head.qTail)
        ((CRMRecPtr)qp)->crmDeviceID = CLC(1);
    else
    {
        CRMRecPtr prevp;

        prevp = (CRMRecPtr)MR(commtool_head.qTail);
        ((CRMRecPtr)qp)->crmDeviceID = CL(CL(prevp->crmDeviceID) + 1);
    }

    Enqueue(qp, &commtool_head);
    restore_virtual_ints(block);
}

PUBLIC OSErr
Executor::CRMRemove(QElemPtr qp)
{
    OSErr retval;

    retval = Dequeue(qp, &commtool_head);
    return retval;
}

PUBLIC QElemPtr
Executor::CRMSearch(QElemPtr qp)
{
    QElemPtr retval;
    virtual_int_state_t block;
    CRMRecPtr p;
    LONGINT min;

    min = CL(((CRMRecPtr)qp)->crmDeviceID) + 1;
    block = block_virtual_ints();

    for(p = (CRMRecPtr)MR(commtool_head.qHead);
        p && CL(p->crmDeviceID) < min;
        p = (CRMRecPtr)MR(p->qLink))
        ;
    restore_virtual_ints(block);
    retval = (QElemPtr)p;
    return retval;
}

PRIVATE CRMErr
serial_insert(const char *input, const char *output, const char *name)
{
    CRMErr retval;
    CRMSerialPtr p;

    p = (CRMSerialPtr)NewPtrSys(sizeof *p);
    if(!p)
        retval = MemError();
    else
    {
        CRMRecPtr qp;

        qp = (CRMRecPtr)NewPtrSys(sizeof *qp);
        if(!qp)
            retval = MemError();
        else
        {
            p->version = CWC(curCRMSerRecVer);
            p->inputDriverName = RM(stringhandle_from_c_string(input));
            p->outputDriverName = RM(stringhandle_from_c_string(output));
            p->name = RM(stringhandle_from_c_string(name));
            p->deviceIcon = NULL;
            p->ratedSpeed = CLC(19200);
            p->maxSpeed = CLC(57600);
            p->reserved = CLC(0);
            qp->qLink = nullptr;
            qp->qType = CWC(crmType);
            qp->crmVersion = CWC(crmRecVersion);
            qp->crmPrivate = CLC(0);
            qp->crmReserved = CWC(0);
            qp->crmDeviceType = CLC(crmSerialDevice);
            qp->crmDeviceID = CLC(0);
            qp->crmAttributes = guest_cast<LONGINT>(RM(p));
            qp->crmStatus = CLC(0);
            qp->crmRefCon = CLC(0);
            CRMInstall((QElemPtr)qp);
            retval = noErr;
        }
    }
    return retval;
}

PUBLIC CRMErr
Executor::InitCRM(void)
{
    static bool beenhere;
    CRMErr retval;

    if(beenhere)
        retval = noErr;
    else
    {
        TheZoneGuard guard(SysZone);
        retval = serial_insert(".AIn", ".AOut", "COM1");
        if(retval == noErr)
            retval = serial_insert(".BIn", ".BOut", "COM2");
        beenhere = true;
    }
    return retval;
}
