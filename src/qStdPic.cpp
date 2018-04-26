/* Copyright 1986-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "MemoryMgr.h"
#include "OSUtil.h"

#include "rsys/cquick.h"
#include "rsys/picture.h"
#include "rsys/print.h"
#include "rsys/text.h"

using namespace Executor;

void Executor::C_StdComment(INTEGER kind, INTEGER size, Handle hand)
{
    SignedByte state;
    GUEST<INTEGER> swappedsize;

    switch(kind)
    {
        case textbegin:
            disable_stdtext();
            break;
        case textend:
            enable_stdtext();
            break;
    }

    GUEST<INTEGER> kind_s = CW(kind);
    if(size)
    {
        PICSAVEBEGIN(OP_LongComment);
        PICWRITE(&kind_s, sizeof(kind_s));
        swappedsize = CW(size);
        PICWRITE(&swappedsize, sizeof(swappedsize));
        state = HGetState(hand);
        HLock(hand);
        PICWRITE(STARH(hand), size);
        if(size & 1)
            PICWRITE("", 1);
        HSetState(hand, state);
        PICSAVEEND;
    }
    else
    {
        PICSAVEBEGIN(OP_ShortComment);
        PICWRITE(&kind_s, sizeof(kind_s));
        PICSAVEEND;
    }
}

void Executor::C_StdGetPic(void *dp, INTEGER bc) /* TODO */
{
    warning_unimplemented(NULL_STRING);
}

void Executor::C_StdPutPic(const void  *sp, INTEGER bc)
{
    piccachehand pch;
    PicHandle ph;
    LONGINT oldhowfar, newhowfar;
    Size newsize;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);

    if(pch)
    {
        oldhowfar = Hx(pch, pichowfar);
        ph = HxP(pch, pichandle);
        newhowfar = Hx(pch, pichowfar) + bc;
        HxX(pch, pichowfar) = CL(newhowfar);
        if(newhowfar > 32766)
            HxX(ph, picSize) = CWC(32766);
        else
            HxX(ph, picSize) = CW(newhowfar);

        if(Hx(pch, pichowfar) > Hx(pch, picsize))
        {
            newsize = (Hx(pch, pichowfar) + 0xFF) & ~(LONGINT)0xFF;
            SetHandleSize((Handle)ph, newsize);
            HxX(pch, picsize) = CL(newsize);
        }
        memmove((char *)STARH(ph) + oldhowfar, sp, bc);
    }
}
