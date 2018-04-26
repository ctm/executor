#pragma once
#include <rsys/mactype.h>
#include <syn68k_public.h>

#include <iostream>
#include <unordered_map>

namespace Executor
{
namespace logging
{

template<typename F, F* fptr, typename CallConv>
struct LoggedFunction;

bool enabled();
void setEnabled(bool e);
void resetNestingLevel();

bool loggingActive();
void indent();

extern int nestingLevel;
extern std::unordered_map<void*, const char*> namedThings;

syn68k_addr_t untypedLoggedFunction(syn68k_addr_t (*fptr)(syn68k_addr_t, void *), syn68k_addr_t addr, void * param);


void logEscapedChar(unsigned char c);
bool canConvertBack(const void* p);
bool validAddress(const void* p);
bool validAddress(syn68k_addr_t p);
void logValue(char x);
void logValue(unsigned char x);
void logValue(signed char x);
void logValue(int16_t x);
void logValue(uint16_t x);
void logValue(int32_t x);
void logValue(uint32_t x);
void logValue(unsigned char* p);
void logValue(const void* p);
void logValue(void* p);
void logValue(ProcPtr p);
void dumpRegsAndStack();

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
inline void logList(Arg a)
{
    logValue(a);
}
inline void logList()
{
}
template<typename Arg1, typename Arg2, typename... Args>
void logList(Arg1 a, Arg2 b, Args... args)
{
    logValue(a);
    std::cout << ", ";
    logList(b,args...);
}

template<class T>
bool validAddress(GUEST<T*> p)
{
    return validAddress(p.raw_host_order());
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


template<typename... Args, void (*fptr)(Args...), typename CallConv> 
struct LoggedFunction<void (Args...), fptr, CallConv>
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

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename CallConv> 
struct LoggedFunction<Ret (Args...), fptr, CallConv>
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

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void *)> 
struct LoggedFunction<syn68k_addr_t (syn68k_addr_t, void *), fptr, callconv::Raw>
{
    static syn68k_addr_t call(syn68k_addr_t addr, void * param)
    {
        return untypedLoggedFunction(fptr, addr, param);
    }
};


}
}
