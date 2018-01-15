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

#include "rsys/ctop_ptoc.h"

#include <iostream>

using namespace Executor;
using namespace Executor::functions;

bool toolflags[0x400];
bool osflags[0x100];

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void **)>
ProcPtr Raw68KFunction<fptr>::guestFP;

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void **)>
void Raw68KFunction<fptr>::init()
{
    guestFP = (ProcPtr)SYN68K_TO_US(callback_install((callback_handler_t)fptr, nullptr));    
}

template<typename Ret, typename... Args, Ret (*fptr)(Args...)>
UPP<Ret (Args...)> WrappedFunction<Ret (Args...), fptr>::guestFP;

template<typename Ret, typename... Args, Ret (*fptr)(Args...)>
void
WrappedFunction<Ret (Args...), fptr>::init()
{
//    static ptocblock_t ptocblock { (void*)fptr, ptoc(fptr) };
//    guestFP = (UPP<Ret (Args...)>)SYN68K_TO_US(callback_install((callback_handler_t)&PascalToCCall, &ptocblock));    
    guestFP = (UPP<Ret (Args...)>)SYN68K_TO_US(callback_install(
        (callback_handler_t)WrappedFunction<Ret (Args...), fptr>::invokeFrom68K, nullptr));    
}

template<typename Ret, typename... Args, Ret (*fptr)(Args...)>
syn68k_addr_t
WrappedFunction<Ret (Args...), fptr>::invokeFrom68K(syn68k_addr_t addr, void **)
{
    static ptocblock_t ptocblock { (void*)fptr, ptoc(fptr) };
    return PascalToCCall(addr, &ptocblock);
}




template<syn68k_addr_t (*fptr)(syn68k_addr_t, void **), int trapno>
void
Raw68KTrap<fptr, trapno>::init()
{
    Raw68KFunction<fptr>::init();
    if(trapno & TOOLBIT)
    {
        toolflags[trapno & 0x3FF] = true;
        tooltraptable[trapno & 0x3FF] = US_TO_SYN68K((void*)this->guestFP);
    }
    else
        osflags[trapno & 0xFF] = true;
}


template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno>
void
PascalTrap<Ret (Args...), fptr, trapno>::init()
{
    WrappedFunction<Ret (Args...), fptr>::init();
    if(trapno & TOOLBIT)
    {
        toolflags[trapno & 0x3FF] = true;
        tooltraptable[trapno & 0x3FF] = US_TO_SYN68K((void*)this->guestFP);
    }
    else
        osflags[trapno & 0xFF] = true;
}


template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno>
Ret
PascalTrap<Ret (Args...), fptr, trapno>::operator()(Args... args) const
{
    if((trapno & TOOLBIT) == 0)
        return (*fptr)(args...);
    else
    {
        // TODO
        syn68k_addr_t new_addr = tooltraptable[trapno & (NTOOLENTRIES - 1)];

        if(new_addr == toolstuff[trapno & (NTOOLENTRIES - 1)].orig)
            return (*fptr)(args...);
        else
            return UPP<Ret(Args...)>(SYN68K_TO_US(new_addr))(args...);
    }
}


InitAction::InitAction(void (*f)())
{
    initFunctions.push_back(f);
}

void InitAction::execute()
{
    for(auto f : initFunctions)
        (*f)();
    for(int i = 0; i < NTOOLENTRIES; i++)
        if(tooltraptable[i] == 0)
            tooltraptable[i] = US_TO_SYN68K((void*)&stub_Unimplemented);

    /*for(int i = 0; i < 0x400; i++)
    {
        bool shouldhave = toolstuff[i].ptoc.wheretogo != (void*)&_Unimplemented;
        if(toolflags[i] != shouldhave)
        {
            if(shouldhave)
                std::cout << "Missing " << std::hex << (0xA800 | i) << std::endl;
            else
                std::cout << "Surprising " << std::hex << (0xA800 | i) << std::endl;
        }
    }*/
    for(int i = 0; i < 0x100; i++)
    {
        bool shouldhave = osstuff[i].func != (void*)&_Unimplemented;
        if(osflags[i] != shouldhave)
        {
            if(shouldhave)
                std::cout << "Missing " << std::hex << (0xA000 | i) << std::endl;
            else
                std::cout << "Surprising " << std::hex << (0xA000 | i) << std::endl;
        }
    }

}
