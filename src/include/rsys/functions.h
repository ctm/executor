#ifndef PASCAL_H_
#define PASCAL_H_

#include <stdint.h>
#include <type_traits>
#include <utility>
#include <vector>
#include "rsys/trapglue.h"
#include "rsys/mactype.h"

#include "rsys/ctop_ptoc.h"
namespace Executor
{

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

template<typename Ret, typename... Args>
struct GuestWrapperBase<UPP<Ret(Args...)>>
{
private:
    HiddenValue<uint32_t> p;

public:
    using WrappedType = UPP<Ret(Args...)>;
    using RawGuestType = uint32_t;

    WrappedType get() const
    {
        uint32_t rawp = this->raw();
        if(rawp)
            return WrappedType(SYN68K_TO_US((uint32_t)swap32(rawp)));
        else
            return nullptr;
    }

    void set(WrappedType ptr)
    {
        if(ptr)
            this->raw(swap32(US_TO_SYN68K(ptr.ptr)));
        else
            this->raw(0);
    }

    RawGuestType raw() const
    {
        return p.raw();
    }

    void raw(RawGuestType x)
    {
        p.raw(x);
    }

    Ret operator()(Args... args)
    {
        return (this->get())(args...);
    }
};

template<typename TT>
GUEST<UPP<TT>> RM(UPP<TT> p)
{
    return GUEST<UPP<TT>>::fromHost(p);
}

template<typename TT>
UPP<TT> MR(GuestWrapper<UPP<TT>> p)
{
    return p.get();
}

namespace functions
{

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void **)>
class Raw68KFunction
{
public:
    ProcPtr operator&() const
    {
        return guestFP;
    }

    void init();
protected:
    ProcPtr guestFP;
};

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void **), int trapno>
class Raw68KTrap : public Raw68KFunction<fptr>
{
public:
    void init();
};


template<typename F, F* fptr>
class WrappedFunction {};

template<typename Ret, typename... Args, Ret (*fptr)(Args...)>
class WrappedFunction<Ret (Args...), fptr>
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

    syn68k_addr_t invokeFrom68K(syn68k_addr_t, void **);

    void init();
protected:
    UPP<Ret (Args...)> guestFP;
};

template<typename F, F* fptr, int trapno>
class PascalTrap {};

template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno>
class PascalTrap<Ret (Args...), fptr, trapno> : public WrappedFunction<Ret (Args...), fptr>
{
public:
    Ret operator()(Args... args) const;
    void init();
};

#if !defined(FUNCTION_WRAPPER_IMPLEMENTATION) /* defined in functions.cpp */

#define CREATE_FUNCTION_WRAPPER(TYPE, NAME) \
    extern Executor::functions::TYPE NAME;   \
    extern template class Executor::functions::TYPE



#else

#define CREATE_FUNCTION_WRAPPER(TYPE, NAME) \
    Executor::functions::TYPE NAME;   \
    template class Executor::functions::TYPE; \
    Executor::functions::InitAction init_##NAME([]{NAME.init();})


#endif

#define COMMA ,
#define PASCAL_TRAP(NAME, TRAP) CREATE_FUNCTION_WRAPPER(PascalTrap<decltype(C_##NAME) COMMA &C_##NAME COMMA TRAP>, NAME)
#define PASCAL_SUBTRAP(NAME, TRAP, TRAPNAME) CREATE_FUNCTION_WRAPPER(PascalTrap<decltype(C_##NAME) COMMA &C_##NAME COMMA 0>, NAME)
#define NOTRAP_FUNCTION(NAME) CREATE_FUNCTION_WRAPPER(WrappedFunction<decltype(C_##NAME) COMMA &C_##NAME>, NAME)
#define PASCAL_FUNCTION(NAME) CREATE_FUNCTION_WRAPPER(WrappedFunction<decltype(C_##NAME) COMMA &C_##NAME>, NAME)
#define RAW_68K_FUNCTION(NAME) \
    syn68k_addr_t _##NAME(syn68k_addr_t, void **); \
    CREATE_FUNCTION_WRAPPER(Raw68KFunction<&_##NAME>, stub_##NAME)
#define RAW_68K_TRAP(NAME, TRAP) \
    syn68k_addr_t _##NAME(syn68k_addr_t, void **); \
    CREATE_FUNCTION_WRAPPER(Raw68KTrap<&_##NAME COMMA TRAP>, stub_##NAME)

class InitAction
{
public:
    InitAction(void (*f)());
    static void execute();
};

}

}

#endif