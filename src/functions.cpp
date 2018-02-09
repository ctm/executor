#include <vector>
#include <unordered_map>
#include <syn68k_public.h>

#define FUNCTION_WRAPPER_IMPLEMENTATION
#include <rsys/functions.h>
#include <rsys/mactype.h>
#include <rsys/byteswap.h>
#include <rsys/functions.impl.h>
#include <rsys/logging.h>

namespace
{
    std::vector<Executor::functions::internal::DeferredInit*> initObjects;
}

#include <iostream>
#include <cctype>
#include <iomanip>

#include <assert.h>

using namespace Executor;
using namespace Executor::functions;

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename CallConv>
WrappedFunction<Ret (Args...), fptr, CallConv>::WrappedFunction(const char* name)
    : name(name)
{
}

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename CallConv>
void WrappedFunction<Ret (Args...), fptr, CallConv>::init()
{
    logging::namedThings[(void*) fptr] = name;
    if(logging::enabled())
        guestFP = (UPP<Ret (Args...),CallConv>)SYN68K_TO_US(callback_install(
            callfrom68K::Invoker<Ret (Args...), &logging::LoggedFunction<Ret (Args...),fptr,CallConv>::call, CallConv>
                ::invokeFrom68K, nullptr));    
    else
        guestFP = (UPP<Ret (Args...),CallConv>)SYN68K_TO_US(callback_install(
            callfrom68K::Invoker<Ret (Args...), fptr, CallConv>
                ::invokeFrom68K, nullptr));    
}

template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno, typename CallConv>
Ret TrapFunction<Ret (Args...), fptr, trapno, CallConv>::invokeViaTrapTable(Args... args) const
{
    return (UPP<Ret (Args...),CallConv>(SYN68K_TO_US(tableEntry())))(args...);
}

template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno, typename CallConv>
void TrapFunction<Ret (Args...), fptr, trapno, CallConv>::init()
{
    WrappedFunction<Ret (Args...), fptr, CallConv>::init();
    originalFunction = US_TO_SYN68K(((void*)this->guestFP));
    assert(trapno);
    assert(!tableEntry());
    tableEntry() = originalFunction;
}

template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno, uint32_t selector, typename CallConv>
SubTrapFunction<Ret (Args...), fptr, trapno, selector, CallConv>::SubTrapFunction(const char* name, GenericDispatcherTrap& dispatcher)
    : WrappedFunction<Ret(Args...),fptr,CallConv>(name), dispatcher(dispatcher)
{
    if(logging::enabled())
        dispatcher.addSelector(selector, callfrom68K::Invoker<Ret (Args...), &logging::LoggedFunction<Ret (Args...),fptr,CallConv>::call, CallConv>
                ::invokeFrom68K);
    else
        dispatcher.addSelector(selector, callfrom68K::Invoker<Ret (Args...), fptr, CallConv>
                ::invokeFrom68K);
}

template<class SelectorConvention>
syn68k_addr_t DispatcherTrap<SelectorConvention>::invokeFrom68K(syn68k_addr_t addr, void* extra)
{
    DispatcherTrap<SelectorConvention>* self = (DispatcherTrap<SelectorConvention>*)extra;
    uint32 sel = SelectorConvention::get();
    auto it = self->selectors.find(sel);
    if(it != self->selectors.end())
        return it->second(addr, nullptr);
    else
    {
        std::cerr << "Unknown selector 0x" << std::hex << sel << " for trap " << self->name << std::endl;
        std::abort();
    }
}

template<class SelectorConvention>
void DispatcherTrap<SelectorConvention>::addSelector(uint32_t sel, callback_handler_t handler)
{
    selectors[sel & SelectorConvention::selectorMask] = handler;
}

template<class SelectorConvention>
void DispatcherTrap<SelectorConvention>::init()
{
    if(trapno)
    {
        ProcPtr guestFP = (ProcPtr)SYN68K_TO_US(callback_install(&invokeFrom68K, this));
        if(trapno & TOOLBIT)
        {
            tooltraptable[trapno & 0x3FF] = US_TO_SYN68K(((void*)guestFP));
        }
        else
        {
            ostraptable[trapno & 0xFF] = US_TO_SYN68K(((void*)guestFP));
        }
    }
}

functions::internal::DeferredInit::DeferredInit()
{
    initObjects.push_back(this);
}

namespace Executor
{
namespace functions
{
namespace selectors
{
template <uint32_t mask>
struct D0
{
    static const uint32_t selectorMask = mask;
    static uint32_t get() { return EM_D0 & mask; }
};

template <uint32_t mask>
struct StackWMasked
{
    static const uint32_t selectorMask = mask;
    static uint32_t get()
    {
        auto ret = POPADDR();
        auto sel = POPUW();
        PUSHADDR(ret);
        return sel & mask;
    }
};

template <uint32_t mask>
struct StackLMasked
{
    static const uint32_t selectorMask = mask;
    static uint32_t get()
    {
        auto ret = POPADDR();
        auto sel = POPUL();
        PUSHADDR(ret);
        return sel & mask;
    }
};
template <uint32_t mask>
struct StackWLookahead
{
    static const uint32_t selectorMask = mask;
    static uint32_t get()
    {
        return READUW(EM_A7 + 4) & mask;
    }
};

}
}
}

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

syn68k_addr_t Executor::tooltraptable[NTOOLENTRIES]; /* Gets filled in at run time */
syn68k_addr_t Executor::ostraptable[NOSENTRIES]; /* Gets filled in at run time */


void functions::init(bool log)
{
    logging::setEnabled(log);
    for(auto init : initObjects)
        init->init();
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
