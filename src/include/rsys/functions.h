#ifndef PASCAL_H_
#define PASCAL_H_

#include <stdint.h>
#include <type_traits>
#include <utility>
#include <vector>
#include "rsys/trapglue.h"
#include "rsys/ctop_ptoc.h"
#include <syn68k_public.h>

namespace Executor
{

namespace callconv
{
class Pascal { };

template<typename T, typename... Extras>
class Register { };
template <int n> struct A;
template <int n> struct D;
template <int mask> struct TrapBit;
template<typename loc> struct Out;
template<typename loc> struct InOut;

template<typename Loc> struct ReturnMemErr;
struct CCFromD0;

}


template<typename F>
struct UPP;

struct OpaqueUntyped68KProc;
typedef OpaqueUntyped68KProc* ProcPtr;

template<typename F>
struct UPP;

template<typename Ret, typename... Args>
struct UPP<Ret(Args...)>
{
    void *ptr;

    constexpr UPP() = default;
    constexpr UPP(std::nullptr_t)
        : ptr(nullptr)
    {
    }
    explicit constexpr UPP(void *ptr)
        : ptr(ptr)
    {
    }
    explicit constexpr UPP(intptr_t ptr)
        : ptr((void*)ptr)
    {
    }

    explicit operator void *() const { return ptr; }
    explicit operator ProcPtr() const { return (ProcPtr)ptr; }

    explicit operator bool() const { return ptr != nullptr; }
    bool operator==(UPP<Ret(Args...)> other) const { return ptr == other.ptr; }
    bool operator!=(UPP<Ret(Args...)> other) const { return ptr != other.ptr; }

    Ret operator()(Args... args) const
    {
        return (Ret)CToPascalCall(ptr, ctop<Ret(Args...)>(), args...);
    }
};


namespace functions
{

namespace selectors
{
    class D0W;
    class D0L;
    class StackW;
    class StackL;
    class StackWLookahead;
}

template<class SelectorConvention>
class DispatcherTrap
{
    //std::unordered_map<uint32_t, callback_handler_t> selectors;

    syn68k_addr_t invokeFrom68K(syn68k_addr_t addr, void* extra);
public:
    void addSelector(uint32_t sel, callback_handler_t handler);

    void init();
};

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void *)>
class Raw68KFunction
{
public:
    ProcPtr operator&() const
    {
        return guestFP;
    }

    Raw68KFunction();
protected:
    static ProcPtr guestFP;
};

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void *), int trapno>
class Raw68KTrap : public Raw68KFunction<fptr>
{
public:
    Raw68KTrap();
};


template<typename F, F* fptr, typename CallConv = callconv::Pascal>
class WrappedFunction {};

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename CallConv>
class WrappedFunction<Ret (Args...), fptr, CallConv>
{
public:
    Ret operator()(Args... args) const
    {
        return (*fptr)(args...);
    }

    UPP<Ret (Args...)> operator&() const
    {
        return guestFP;
    }

    WrappedFunction();    
protected:
    static UPP<Ret (Args...)> guestFP;
    static const char *name;
};

template<typename F, F* fptr, int trapno, typename CallConv = callconv::Pascal>
class PascalTrap {};

template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno, typename CallConv>
class PascalTrap<Ret (Args...), fptr, trapno, CallConv> : public WrappedFunction<Ret (Args...), fptr, CallConv>
{
public:
    Ret operator()(Args... args) const;
    
    PascalTrap();
};

#if !defined(FUNCTION_WRAPPER_IMPLEMENTATION) /* defined in functions.cpp */

#define CREATE_FUNCTION_WRAPPER(TYPE, NAME, FPTR, DISPLAYNAME) \
    extern Executor::functions::TYPE NAME

#define DISPATCHER_TRAP(NAME, TRAP, SELECTOR) \
    extern Executor::functions::DispatcherTrap<Executor::functions::selectors::SELECTOR> NAME

#else

#define CREATE_FUNCTION_WRAPPER(TYPE, NAME, FPTR, DISPLAYNAME) \
    Executor::functions::TYPE NAME;   \
    template class Executor::functions::TYPE; \
    template<> const char * NamedThing<decltype(FPTR),FPTR>::name = DISPLAYNAME

#define DISPATCHER_TRAP(NAME, TRAP, SELECTOR) \
    Executor::functions::DispatcherTrap<Executor::functions::selectors::SELECTOR> NAME

#endif

#define COMMA ,
#define PASCAL_TRAP(NAME, TRAP) \
    CREATE_FUNCTION_WRAPPER(PascalTrap<decltype(C_##NAME) COMMA &C_##NAME COMMA TRAP>, NAME, &C_##NAME, #NAME)
#define PASCAL_SUBTRAP(NAME, TRAP, SELECTOR, TRAPNAME) \
    CREATE_FUNCTION_WRAPPER(PascalTrap<decltype(C_##NAME) COMMA &C_##NAME COMMA 0>, NAME, &C_##NAME, #NAME)
#define NOTRAP_FUNCTION(NAME) \
    CREATE_FUNCTION_WRAPPER(WrappedFunction<decltype(C_##NAME) COMMA &C_##NAME>, NAME, &C_##NAME, #NAME)
#define PASCAL_FUNCTION(NAME) \
    CREATE_FUNCTION_WRAPPER(WrappedFunction<decltype(C_##NAME) COMMA &C_##NAME>, NAME, &C_##NAME, #NAME)
#define RAW_68K_FUNCTION(NAME) \
    syn68k_addr_t _##NAME(syn68k_addr_t, void *); \
    CREATE_FUNCTION_WRAPPER(Raw68KFunction<&_##NAME>, stub_##NAME, &_##NAME, #NAME)
#define RAW_68K_TRAP(NAME, TRAP) \
    syn68k_addr_t _##NAME(syn68k_addr_t, void *); \
    CREATE_FUNCTION_WRAPPER(Raw68KTrap<&_##NAME COMMA TRAP>, stub_##NAME, &_##NAME, #NAME)

    
void resetNestingLevel();
void init();

}

}

#endif