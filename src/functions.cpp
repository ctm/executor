#include <vector>
#include <unordered_map>
#include <syn68k_public.h>

#define FUNCTION_WRAPPER_IMPLEMENTATION
#include <rsys/functions.h>
#include <rsys/mactype.h>
#include <rsys/byteswap.h>

namespace
{
    std::vector<Executor::functions::internal::DeferredInit*> initObjects;
    std::unordered_map<void*, const char*> namedThings;
}

#include <iostream>
#include <cctype>
#include <iomanip>

using namespace Executor;
using namespace Executor::functions;

bool toolflags[0x400];
bool osflags[0x100];

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

namespace Executor
{
namespace callconv
{
template<int n> struct A
{
    template<typename T>
    operator T() { return EM_AREG(n); }
    template<typename T>
    operator T*() { return ptr_from_longint<T*>(EM_AREG(n)); }
    template<typename T>
    operator UPP<T>() { return UPP<T>(ptr_from_longint<ProcPtr>(EM_AREG(n))); }

    static void set(uint32_t x) { EM_AREG(n) = x; }
    static void set(void *x) { EM_AREG(n) = ptr_to_longint(x); }
};

template<int n> struct D
{
    template<typename T>
    operator T() { return EM_DREG(n); }
    static void set(uint32_t x) { EM_DREG(n) = x; }
};

struct D0HighWord
{
    operator uint16_t() { return EM_D0 >> 16; }
};

template<typename Loc, typename T> struct Out
{
    T temp;

    operator T*() { return &temp; }

    ~Out() { Loc::set(temp); }
};

template<typename Loc, typename T> struct InOut
{
    T temp;

    operator T*() { return &temp; }

    InOut() { temp = Loc(); }
    ~InOut() { Loc::set(temp); }
};


template<int mask> struct TrapBit
{
    operator bool() { return !!(EM_D1 & mask); }
};

struct CCFromD0
{
    syn68k_addr_t afterwards(syn68k_addr_t ret)
    {
        cpu_state.ccc = 0;
        cpu_state.ccn = (cpu_state.regs[0].sw.n < 0);
        cpu_state.ccv = 0;
        cpu_state.ccx = cpu_state.ccx; /* unchanged */
        cpu_state.ccnz = (cpu_state.regs[0].sw.n != 0);
        return ret;
    }
};

struct ClearD0
{
    syn68k_addr_t afterwards(syn68k_addr_t ret)
    {
        EM_D0 = 0;
        return ret;
    }  
};

struct SaveA1D1D2
{
    syn68k_addr_t a1, d1, d2;
    SaveA1D1D2()
    {
        a1 = EM_A1;
        d1 = EM_D1;
        d2 = EM_D2;
    }
    syn68k_addr_t afterwards(syn68k_addr_t ret)
    {
        EM_A1 = a1;
        EM_D1 = d1;
        EM_D2 = d2;
        return ret;
    }  
};

struct MoveA1ToA0
{
    syn68k_addr_t a1;
    MoveA1ToA0()
    {
        a1 = EM_A1;
    }
    syn68k_addr_t afterwards(syn68k_addr_t ret)
    {
        EM_A0 = a1;
        return ret;
    }
};

}
}

template<typename F, F *fptr, typename CallConv>
struct Invoker;

template<typename Ret, typename... Args, Ret (*fptr)(Args...)>
struct Invoker<Ret (Args...), fptr, callconv::Pascal>
{
    static syn68k_addr_t invokeFrom68K(syn68k_addr_t addr, void *)
    {
        static ptocblock_t ptocblock { (void*)fptr, ptoc(fptr) };
        return PascalToCCall(addr, &ptocblock);
    }
};


template<syn68k_addr_t (*fptr)(syn68k_addr_t, void*)>
struct Invoker<syn68k_addr_t (syn68k_addr_t addr, void *), fptr, callconv::Raw>
{
    static syn68k_addr_t invokeFrom68K(syn68k_addr_t addr, void * refcon)
    {
        return fptr(addr, refcon);
    }
};
template<typename... Xs> struct List;

template<typename F, F *fptr, typename DoneArgs, typename ToDoArgs, typename ToDoCC>
struct InvokerRec;

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename... DoneArgs>
struct InvokerRec<Ret (Args...), fptr, List<DoneArgs...>, List<>, List<>>
{
    static Ret invokeFrom68K(syn68k_addr_t addr, void *refcon, DoneArgs... args)
    {
        return fptr(args...);
    }
};

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename... DoneArgs, typename Arg, typename... TodoArgs, typename CC, typename... TodoCC>
struct InvokerRec<Ret (Args...), fptr, List<DoneArgs...>, List<Arg, TodoArgs...>, List<CC, TodoCC...>>
{
    static Ret invokeFrom68K(syn68k_addr_t addr, void *refcon, DoneArgs... args)
    {
        CC newarg;
        return InvokerRec<Ret (Args...),fptr,List<DoneArgs...,Arg>,List<TodoArgs...>,List<TodoCC...>>
            ::invokeFrom68K(addr, refcon, args..., newarg);
    }
};

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename RetConv, typename... ArgConvs>
struct Invoker<Ret (Args...), fptr, callconv::Register<RetConv (ArgConvs...)>>
{
    static syn68k_addr_t invokeFrom68K(syn68k_addr_t addr, void *refcon)
    {
        syn68k_addr_t retaddr = POPADDR();
        Ret retval = InvokerRec<Ret (Args...), fptr, List<>, List<Args...>, List<ArgConvs...>>::invokeFrom68K(addr, refcon);
        RetConv::set(retval);  // ### double conversion?
        return retaddr;
    }
};
template<typename... Args, void (*fptr)(Args...), typename RetConv, typename... ArgConvs>
struct Invoker<void (Args...), fptr, callconv::Register<RetConv (ArgConvs...)>>
{
    static syn68k_addr_t invokeFrom68K(syn68k_addr_t addr, void *refcon)
    {
        syn68k_addr_t retaddr = POPADDR();
        InvokerRec<void (Args...), fptr, List<>, List<Args...>, List<ArgConvs...>>::invokeFrom68K(addr, refcon);
        return retaddr;
    }
};

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename RetArgConv, typename Extra1, typename... Extras>
struct Invoker<Ret (Args...), fptr, callconv::Register<RetArgConv, Extra1, Extras...>>
{
    static syn68k_addr_t invokeFrom68K(syn68k_addr_t addr, void *refcon)
    {
        Extra1 state;
        syn68k_addr_t retval = Invoker<Ret(Args...), fptr, callconv::Register<RetArgConv, Extras...>>::invokeFrom68K(addr,refcon);
        return state.afterwards(retval);
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
        guestFP = (UPP<Ret (Args...)>)SYN68K_TO_US(callback_install(
            Invoker<Ret (Args...), &LoggedFunction<Ret (Args...),fptr>::call, CallConv>
                ::invokeFrom68K, nullptr));    
    else
        guestFP = (UPP<Ret (Args...)>)SYN68K_TO_US(callback_install(
            Invoker<Ret (Args...), fptr, CallConv>
                ::invokeFrom68K, nullptr));    
}

template<syn68k_addr_t (*fptr)(syn68k_addr_t,void*)>
WrappedFunction<syn68k_addr_t (syn68k_addr_t,void*), fptr, callconv::Raw>::WrappedFunction(const char* name)
    : name(name)
{
    namedThings[(void*) fptr] = name;
}

template<syn68k_addr_t (*fptr)(syn68k_addr_t,void*)>
void WrappedFunction<syn68k_addr_t (syn68k_addr_t,void*), fptr, callconv::Raw>::init()
{
    if(loggingEnabled)
        guestFP = (ProcPtr)SYN68K_TO_US(callback_install(&untypedLoggedFunction<fptr>, nullptr));
    else
        guestFP = (ProcPtr)SYN68K_TO_US(callback_install(fptr, nullptr));
}



template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno, typename CallConv>
void TrapFunction<Ret (Args...), fptr, trapno, CallConv>::init()
{
    WrappedFunction<Ret (Args...), fptr, CallConv>::init();
    if(trapno)
    {
        if(trapno & TOOLBIT)
        {
            if(!toolflags[trapno & 0x3FF])
            {
                toolflags[trapno & 0x3FF] = true;
                tooltraptable[trapno & 0x3FF] = US_TO_SYN68K(((void*)this->guestFP));
            }
            else
                std::cout << "Not replacing " << std::hex << trapno << " : " << this->name << std::endl;
        }
        else
        {
            if(!osflags[trapno & 0xFF])
            {
                osflags[trapno & 0xFF] = true;
                ostraptable[trapno & 0xFF] = US_TO_SYN68K(((void*)this->guestFP));
            }
            else
                std::cout << "Not replacing " << std::hex << trapno << " : " << this->name << std::endl;
        }
    }
}

template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno, uint32_t selector, typename CallConv>
SubTrapFunction<Ret (Args...), fptr, trapno, selector, CallConv>::SubTrapFunction(const char* name, GenericDispatcherTrap& dispatcher)
    : WrappedFunction<Ret(Args...),fptr,CallConv>(name), dispatcher(dispatcher)
{
    if(loggingEnabled)
        dispatcher.addSelector(selector, Invoker<Ret (Args...), &LoggedFunction<Ret (Args...),fptr>::call, CallConv>
                ::invokeFrom68K);
    else
        dispatcher.addSelector(selector, Invoker<Ret (Args...), fptr, CallConv>
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
            toolflags[trapno & 0x3FF] = true;
            tooltraptable[trapno & 0x3FF] = US_TO_SYN68K(((void*)guestFP));
        }
        else
        {
            osflags[trapno & 0xFF] = true;
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

syn68k_addr_t Executor::tooltraptable[0x400]; /* Gets filled in at run time */
syn68k_addr_t Executor::ostraptable[0x100]; /* Gets filled in at run time */


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
}
