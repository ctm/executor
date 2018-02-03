#include <vector>
#include <unordered_map>
#include <syn68k_public.h>

#define FUNCTION_WRAPPER_IMPLEMENTATION
#include <rsys/functions.h>
#include <rsys/mactype.h>
#include <rsys/byteswap.h>
#include <rsys/functions.impl.h>

namespace
{
    std::vector<Executor::functions::internal::DeferredInit*> initObjects;
    std::unordered_map<void*, const char*> namedThings;
}

#include <iostream>
#include <cctype>
#include <iomanip>

#include <assert.h>

using namespace Executor;
using namespace Executor::functions;

template<typename F, F* fptr>
struct LoggedFunction;

static int nestingLevel = 0;
static bool loggingEnabled = false;

void functions::resetNestingLevel()
{
    nestingLevel  = 0;
}
static void indent()
{
    for(int i = 0; i < nestingLevel; i++)
        std::cout << "  ";
}
static bool loggingActive()
{
    return nestingLevel == 0;
}

void logEscapedChar(unsigned char c)
{
    if(c == '\'' || c == '\"' || c == '\\')
        std::cout << '\\' << c;
    else if(std::isprint(c))
        std::cout << c;
    else
        std::cout << "\\0" << std::oct << (unsigned)c << std::dec;
}

bool canConvertBack(const void* p)
{
    if(!p || p == (const void*) -1)
        return true;
#if SIZEOF_VOID_P == 4
    bool valid = true;
#else
    bool valid = false;
    for(int i = 0; i < 4; i++)
    {
        if((uintptr_t)p >= ROMlib_offsets[i] &&
            (uintptr_t)p < ROMlib_offsets[i] + ROMlib_sizes[i])
            valid = true;
    }
#endif
    return valid;
}

bool validAddress(const void* p)
{
    if(!p)
        return false;
    if( (uintptr_t)p & 1 )
        return false;
#if SIZEOF_VOID_P == 4
    bool valid = true;
#else
    bool valid = false;
    for(int i = 0; i < 4; i++)
    {
        if((uintptr_t)p >= ROMlib_offsets[i] &&
            (uintptr_t)p < ROMlib_offsets[i] + ROMlib_sizes[i])
            valid = true;
    }
#endif
    if(!valid)
        return false;

    return true;
}

bool validAddress(syn68k_addr_t p)
{
    if(p == 0 || (p & 1))
        return false;
    return validAddress(SYN68K_TO_US(p));
}

template<class T>
bool validAddress(GUEST<T*> p)
{
    return validAddress(p.raw_host_order());
}

template<typename T>
void logValue(const T& arg)
{
    std::cout << "?";
}
template<class T>
void logValue(const GuestWrapper<T>& p)
{
    logValue(p.get());
}

void logValue(char x)
{
    std::cout << (int)x;
    std::cout << " = '";
    logEscapedChar(x);
    std::cout << '\'';
}
void logValue(unsigned char x)
{
    std::cout << (int)x;
    if(std::isprint(x))
        std::cout << " = '" << x << '\'';
}
void logValue(signed char x)
{
    std::cout << (int)x;
    if(std::isprint(x))
        std::cout << " = '" << x << '\'';
}
void logValue(int16_t x) { std::cout << x; }
void logValue(uint16_t x) { std::cout << x; }
void logValue(int32_t x)
{
    std::cout << x << " = '";
    logEscapedChar((x >> 24) & 0xFF);
    logEscapedChar((x >> 16) & 0xFF);
    logEscapedChar((x >> 8) & 0xFF);
    logEscapedChar(x & 0xFF);
    std::cout << "'";
}
void logValue(uint32_t x)
{
    std::cout << x << " = '";
    logEscapedChar((x >> 24) & 0xFF);
    logEscapedChar((x >> 16) & 0xFF);
    logEscapedChar((x >> 8) & 0xFF);
    logEscapedChar(x & 0xFF);
    std::cout << "'";
}
void logValue(unsigned char* p)
{
    std::cout << "0x" << std::hex << US_TO_SYN68K_CHECK0_CHECKNEG1(p) << std::dec;
    if(validAddress(p) && validAddress(p+256))
    {
        std::cout << " = \"\\p";
        for(int i = 1; i <= p[0]; i++)
            logEscapedChar(p[i]);
        std::cout << '"';
    }
}
void logValue(const void* p)
{
    if(canConvertBack(p))
        std::cout << "0x" << std::hex << US_TO_SYN68K_CHECK0_CHECKNEG1(p) << std::dec;
    else
        std::cout << "?";
}
void logValue(void* p)
{
    if(canConvertBack(p))
        std::cout << "0x" << std::hex << US_TO_SYN68K_CHECK0_CHECKNEG1(p) << std::dec;
    else
        std::cout << "?";
}
void logValue(ProcPtr p)
{
    if(canConvertBack(p))
        std::cout << "0x" << std::hex << US_TO_SYN68K_CHECK0_CHECKNEG1(p) << std::dec;
    else
        std::cout << "?";
}

template<class T>
void logValue(T* p)
{
    if(canConvertBack(p))
        std::cout << "0x" << std::hex << US_TO_SYN68K_CHECK0_CHECKNEG1(p) << std::dec;
    else
        std::cout << "?";
    if(validAddress(p))
    {
        std::cout << " => ";
        logValue(*p);
    }
}
template<class T>
void logValue(GuestWrapper<T*> p)
{
    std::cout << "0x" << std::hex << p.raw() << std::dec;
    if(validAddress(p.raw_host_order()))
    {
        std::cout << " => ";
        logValue(*(p.get()));
    }
}

template<typename Arg>
void logList(Arg a)
{
    logValue(a);
}
void logList()
{
}
template<typename Arg1, typename Arg2, typename... Args>
void logList(Arg1 a, Arg2 b, Args... args)
{
    logValue(a);
    std::cout << ", ";
    logList(b,args...);
}


template<typename... Args>
void logTrapCall(const char* trapname, Args... args)
{
    if(!loggingActive())
        return;
    std::cout.clear();
    indent();
    std::cout << trapname << "(";
    logList(args...);
    std::cout << ")\n" << std::flush;
}

template<typename Ret, typename... Args>
void logTrapValReturn(const char* trapname, Ret ret, Args... args)
{
    if(!loggingActive())
        return;
    indent();
    std::cout << "returning: " << trapname << "(";
    logList(args...);
    std::cout << ") => ";
    logValue(ret);
    std::cout << std::endl << std::flush;
}
template<typename... Args>
void logTrapVoidReturn(const char* trapname, Args... args)
{
    if(!loggingActive())
        return;
    indent();
    std::cout << "returning: " << trapname << "(";
    logList(args...);
    std::cout << ")\n" << std::flush;
}


template<typename... Args, void (*fptr)(Args...)> 
struct LoggedFunction<void (Args...), fptr>
{
    static void call(Args... args)
    {
        const char *fname = namedThings.at((void*)fptr);
        logTrapCall(fname, args...);
        nestingLevel++;
        fptr(args...);
        nestingLevel--;
        logTrapVoidReturn(fname, args...);
    }
};

template<typename Ret, typename... Args, Ret (*fptr)(Args...)> 
struct LoggedFunction<Ret (Args...), fptr>
{
    static Ret call(Args... args)
    {
        const char *fname = namedThings.at((void*)fptr);
        logTrapCall(fname, args...);
        nestingLevel++;
        Ret retval = fptr(args...);
        nestingLevel--;
        logTrapValReturn(fname, retval, args...);
        return retval;
    }
};

void dumpRegsAndStack()
{
    std::cout << std::hex << /*std::showbase <<*/ std::setfill('0');
    std::cout << "D0=" << std::setw(8) << EM_D0 << " ";
    std::cout << "D1=" << std::setw(8) << EM_D1 << " ";
    std::cout << "A0=" << std::setw(8) << EM_A0 << " ";
    std::cout << "A1=" << std::setw(8) << EM_A1 << " ";
    //std::cout << std::noshowbase;
    std::cout << "Stack: ";
    uint8_t *p = (uint8_t*)SYN68K_TO_US(EM_A7);
    for(int i = 0; i < 12; i++)
        std::cout << std::setfill('0') << std::setw(2) << (unsigned)p[i] << " ";
    std::cout << std::dec;
}

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void *)>
syn68k_addr_t untypedLoggedFunction(syn68k_addr_t addr, void * param)
{
    const char *fname = namedThings.at((void*)fptr);
    if(loggingActive())
    {
        std::cout.clear();
        indent();
        std::cout << fname << " ";
        dumpRegsAndStack();
        std::cout << std::endl;
    }
    nestingLevel++;
    syn68k_addr_t retaddr = (*fptr)(addr, param);
    nestingLevel--;
    if(loggingActive())
    {
        indent();
        std::cout << "returning: " << fname << " ";
        dumpRegsAndStack();
        std::cout << std::endl << std::flush;
    }
    return retaddr;
}

template<typename F, F* fptr, typename CallConv>
struct LoggedFunctionCC;

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void *)> 
struct LoggedFunctionCC<syn68k_addr_t (syn68k_addr_t, void *), fptr, callconv::Raw>
{
    static syn68k_addr_t call(syn68k_addr_t addr, void * param)
    {
        return untypedLoggedFunction<fptr>(addr, param);
    }
};

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename CallConv> 
struct LoggedFunctionCC<Ret (Args...), fptr, CallConv>
{
    static Ret call(Args... args)
    {
        return LoggedFunction<Ret(Args...),fptr>::call(args...);
    }
};

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename CallConv>
WrappedFunction<Ret (Args...), fptr, CallConv>::WrappedFunction(const char* name)
    : name(name)
{
    namedThings[(void*) fptr] = name;
}

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename CallConv>
void WrappedFunction<Ret (Args...), fptr, CallConv>::init()
{
    if(loggingEnabled)
        guestFP = (UPP<Ret (Args...),CallConv>)SYN68K_TO_US(callback_install(
            callfrom68K::Invoker<Ret (Args...), &LoggedFunctionCC<Ret (Args...),fptr,CallConv>::call, CallConv>
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
    if(loggingEnabled)
        dispatcher.addSelector(selector, callfrom68K::Invoker<Ret (Args...), &LoggedFunction<Ret (Args...),fptr>::call, CallConv>
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
    loggingEnabled = log;
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
