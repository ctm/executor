#include <vector>
#include <syn68k_public.h>
namespace
{
    std::vector<void (*)()> initFunctions;
}

#define FUNCTION_WRAPPER_IMPLEMENTATION

#include "rsys/everything.h"
#include "rsys/emustubs.h"
#include "rsys/ctl.h"
#include "rsys/list.h"
#include "rsys/menu.h"
#include "rsys/wind.h"
#include "rsys/print.h"
#include "rsys/vbl.h"
#include "rsys/stdfile.h"
#include "rsys/tesave.h"
#include "rsys/osutil.h"
#include "rsys/refresh.h"
#include "rsys/gestalt.h"

#include <iostream>

using namespace Executor;
using namespace Executor::functions;

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void **)>
void Raw68KFunction<fptr>::init()
{
    guestFP = (ProcPtr)SYN68K_TO_US(callback_install((callback_handler_t)fptr, nullptr));    
}

template<typename Ret, typename... Args, Ret (*fptr)(Args...)>
void
WrappedFunction<Ret (Args...), fptr>::init()
{
    static ptocblock_t ptocblock { (void*)fptr, ptoc(fptr) };
    guestFP = (UPP<Ret (Args...)>)SYN68K_TO_US(callback_install((callback_handler_t)&PascalToCCall, &ptocblock));    
}

/*template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno>
void
PascalTrap<Ret (Args...), fptr, trapno>::init()
{
}
*/

InitAction::InitAction(void (*f)())
{
    initFunctions.push_back(f);
}

void InitAction::execute()
{
    for(auto f : initFunctions)
        (*f)();
}
