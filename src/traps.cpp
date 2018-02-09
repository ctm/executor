#include <vector>
#include <syn68k_public.h>

#include <rsys/functions.h>
#include <rsys/mactype.h>
#include <rsys/byteswap.h>
#include <rsys/functions.impl.h>
#include <rsys/logging.h>

#include <assert.h>

using namespace Executor;
using namespace Executor::functions;
Executor::functions::internal::DeferredInit *Executor::functions::internal::DeferredInit::first = nullptr;
Executor::functions::internal::DeferredInit *Executor::functions::internal::DeferredInit::last = nullptr;

functions::internal::DeferredInit::DeferredInit()
    : next(nullptr)
{
    if(!first)
        first = last = this;
    else
    {
        last->next = this;
        last = this;
    }
}

void functions::internal::DeferredInit::initAll()
{
    for(DeferredInit *p = first; p; p = p->next)
        p->init();
}

syn68k_addr_t Executor::tooltraptable[NTOOLENTRIES]; /* Gets filled in at run time */
syn68k_addr_t Executor::ostraptable[NOSENTRIES]; /* Gets filled in at run time */

namespace Executor
{
RAW_68K_TRAP(Unimplemented, 0xA89F);
}

void functions::init(bool log)
{
    logging::setEnabled(log);
    internal::DeferredInit::initAll();
    for(int i = 0; i < NTOOLENTRIES; i++)
    {
        if(tooltraptable[i] == 0)
            tooltraptable[i] = US_TO_SYN68K((void*)&stub_Unimplemented);
    }
    for(int i = 0; i < NOSENTRIES; i++)
    {
        if(ostraptable[i] == 0)
            ostraptable[i] = US_TO_SYN68K((void*)&stub_Unimplemented);
    }
}
