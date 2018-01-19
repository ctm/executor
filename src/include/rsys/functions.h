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

template<typename T>
class Register { };

template<int n> struct A
{
    static uint32_t get() { return EM_AREG(n); }
    static void set(uint32_t x) { EM_AREG(n) = x; }
};

template<int n> struct D
{
    static uint32_t get() { return EM_DREG(n); }
    static void set(uint32_t x) { EM_DREG(n) = x; }
};


template<int mask> struct TrapBit
{
    static uint32_t get() { return !!(EM_D1 & mask); }
};
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

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void *)>
class Raw68KFunction
{
public:
    ProcPtr operator&() const
    {
        return guestFP;
    }

    void init();
protected:
    static ProcPtr guestFP;
};

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void *), int trapno>
class Raw68KTrap : public Raw68KFunction<fptr>
{
public:
    void init();
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

    void init();
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
    void init();
};

#if !defined(FUNCTION_WRAPPER_IMPLEMENTATION) /* defined in functions.cpp */

#define CREATE_FUNCTION_WRAPPER(TYPE, NAME, FPTR, DISPLAYNAME) \
    extern Executor::functions::TYPE NAME;   \
    extern template class Executor::functions::TYPE

#else

#define CREATE_FUNCTION_WRAPPER(TYPE, NAME, FPTR, DISPLAYNAME) \
    Executor::functions::TYPE NAME;   \
    template class Executor::functions::TYPE; \
    Executor::functions::InitAction init_##NAME([]{NAME.init();}); \
    template<> const char * NamedThing<decltype(FPTR),FPTR>::name = DISPLAYNAME


#endif

#define COMMA ,
#define PASCAL_TRAP(NAME, TRAP) \
    CREATE_FUNCTION_WRAPPER(PascalTrap<decltype(C_##NAME) COMMA &C_##NAME COMMA TRAP>, NAME, &C_##NAME, #NAME)
#define PASCAL_SUBTRAP(NAME, TRAP, TRAPNAME) \
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

class InitAction
{
public:
    InitAction(void (*f)());
    static void execute();
};


void resetNestingLevel();

}

}

#endif