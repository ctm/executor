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
#include <iomanip>

using namespace Executor;
using namespace Executor::functions;

bool toolflags[0x400];
bool osflags[0x100];

template<typename F, F* fptr>
struct LoggedFunction;

static thread_local int nestingLevel = 0;

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
    printf("Hello, world.\n");
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
        const char *fname = NamedThing<void (*)(Args...), fptr>::name;
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
        const char *fname = NamedThing<Ret (*)(Args...), fptr>::name;
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
    std::cout << std::hex << std::showbase << std::setfill('0');
    std::cout << "D0=" << std::setw(8) << EM_A0 << " ";
    std::cout << "D1=" << std::setw(8) << EM_A0 << " ";
    std::cout << "A0=" << std::setw(8) << EM_A0 << " ";
    std::cout << "A1=" << std::setw(8) << EM_A0 << " ";
    std::cout << std::noshowbase;
    std::cout << "Stack: ";
    uint8_t *p = (uint8_t*)SYN68K_TO_US(EM_A7);
    for(int i = 0; i < 12; i++)
        std::cout << std::setfill('0') << std::setw(2) << (unsigned)p[i] << " ";
    std::cout << std::dec;
}

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void *)>
syn68k_addr_t untypedLoggedFunction(syn68k_addr_t addr, void * param)
{
    const char *fname = NamedThing<syn68k_addr_t (*)(syn68k_addr_t, void *), fptr>::name;
    if(loggingActive())
    {
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

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void *)>
ProcPtr Raw68KFunction<fptr>::guestFP;

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void *)>
void Raw68KFunction<fptr>::init()
{
    guestFP = (ProcPtr)SYN68K_TO_US(callback_install(&untypedLoggedFunction<fptr>, nullptr));    
}

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename CallConv>
UPP<Ret (Args...)> WrappedFunction<Ret (Args...), fptr, CallConv>::guestFP;


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

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename RetConv, typename ArgConvs>
struct Invoker<Ret (Args...), fptr, callconv::Register<RetConv (ArgConvs...)>>
{
    static syn68k_addr_t invokeFrom68K(syn68k_addr_t addr, void *)
    {
        return 0;
    }
};


template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename CallConv>
void
WrappedFunction<Ret (Args...), fptr, CallConv>::init()
{
    guestFP = (UPP<Ret (Args...)>)SYN68K_TO_US(callback_install(
        Invoker<Ret (Args...), LoggedFunction<Ret (Args...),fptr>::call, CallConv>
            ::invokeFrom68K, nullptr));    
}

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void *), int trapno>
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


template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno, typename CallConv>
void
PascalTrap<Ret (Args...), fptr, trapno, CallConv>::init()
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


template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno, typename CallConv>
Ret
PascalTrap<Ret (Args...), fptr, trapno, CallConv>::operator()(Args... args) const
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
