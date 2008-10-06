#include "rsys/common.h"
#include "FileMgr.h"

main()
{
    short rn;
    long count;
    
    Create("\pCirrus 80D:FSWrite", 0, 'xxxx', 'yyyy');
    FSOpen("\pCirrus 80D:FSWrite", 0, &rn);
    count = 1;
    FSWrite(rn, &count, "a");
    FSClose(rn);
    
    Create("\pCirrus 80D:SetEOF", 0, 'xxxx', 'yyyy');
    FSOpen("\pCirrus 80D:SetEOF", 0, &rn);
    SetEOF(rn, 1L);
    FSClose(rn);
    
    Create("\pCirrus 80D:Allocate", 0, 'xxxx', 'yyyy');
    FSOpen("\pCirrus 80D:Allocate", 0, &rn);
    count = 1;
    Allocate(rn, &count);
    FSClose(rn);
}