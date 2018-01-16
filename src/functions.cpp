#include <vector>
#include <syn68k_public.h>
namespace
{
    std::vector<void (*)()> initFunctions;
}

template<typename F, F thing>
struct NamedThing
{
    static const char* name;
};

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
#include <cctype>

using namespace Executor;
using namespace Executor::functions;

bool toolflags[0x400];
bool osflags[0x100];

template<typename F, F* fptr>
struct LoggedFunction;

struct Nest
{
    Nest() { ++level; }
    ~Nest() { --level; }
    static thread_local int level;
};
thread_local int Nest::level = 0;

void logEscapedChar(unsigned char c)
{
    if(c == '\'' || c == '\"' || c == '\\')
        std::cout << '\\' << c;
    else if(std::isprint(c))
        std::cout << c;
    else
        std::cout << "\\0" << std::oct << (unsigned)c << std::dec;
}

template<typename T>
void logValue(T arg)
{
    std::cout << "?";
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
    std::cout << "0x" << std::hex << US_TO_SYN68K(p) << std::dec;
    if(p && p != (unsigned char*)-1)
    {
        std::cout << " = \"\\p";
        for(int i = 1; i <= p[0]; i++)
            logEscapedChar(p[i]);
        std::cout << '"';
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
    if(Nest::level > 2)
        return;
    for(int i = 1; i < Nest::level; i++)
        std::cout << "  ";
    std::cout << trapname << "(";
    logList(args...);
    std::cout << ")\n";
}

template<typename Ret, typename... Args>
void logTrapValReturn(const char* trapname, Ret ret, Args... args)
{
    if(Nest::level > 2)
        return;
    for(int i = 1; i < Nest::level; i++)
        std::cout << "  ";
    std::cout << "returning: " << trapname << "(";
    logList(args...);
    std::cout << ") => ";
    logValue(ret);
    std::cout << std::endl;
}
template<typename... Args>
void logTrapVoidReturn(const char* trapname, Args... args)
{
    if(Nest::level > 2)
        return;
    for(int i = 1; i < Nest::level; i++)
        std::cout << "  ";
    std::cout << "returning: " << trapname << "(";
    logList(args...);
    std::cout << ")\n";
}


template<typename... Args, void (*fptr)(Args...)> 
struct LoggedFunction<void (Args...), fptr>
{
    static void call(Args... args)
    {
        const char *fname = NamedThing<void (*)(Args...), fptr>::name;
        logTrapCall(fname, args...);
        fptr(args...);
        logTrapVoidReturn(fname, args...);
    }
};

template<typename Ret, typename... Args, Ret (*fptr)(Args...)> 
struct LoggedFunction<Ret (Args...), fptr>
{
    static Ret call(Args... args)
    {
        const char *fname = NamedThing<Ret (*)(Args...), fptr>::name;
        logTrapCall(fname, args...);
        Ret retval = fptr(args...);
        logTrapValReturn(fname, retval, args...);
        return retval;
    }
};


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
    Nest nest;
    Ret (*fptr2)(Args...);
    fptr2 = LoggedFunction<Ret (Args...),fptr>::call;
    static ptocblock_t ptocblock { (void*)fptr2, ptoc(fptr) };
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
    {
        osflags[trapno & 0xFF] = true;
        ostraptable[trapno & 0xFF] = US_TO_SYN68K((void*)this->guestFP);
    }
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
