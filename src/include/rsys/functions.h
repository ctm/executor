#ifndef PASCAL_H_
#define PASCAL_H_

#include <stdint.h>
#include <type_traits>
#include <utility>
#include <vector>
#include <unordered_map>
#include "rsys/trapglue.h"
#include "rsys/ctop_ptoc.h"
#include <syn68k_public.h>

namespace Executor
{

namespace callconv
{
class Pascal { };
class Raw { };

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
    struct D0W;
    struct D0L;
    struct StackW;
    struct StackL;
    struct StackWLookahead;
}

namespace internal
{
    class DeferredInit
    {
    public:
        DeferredInit();
        virtual void init() = 0;
    };
}

class GenericDispatcherTrap : public internal::DeferredInit
{
public:
    void addSelector(uint32_t sel, callback_handler_t handler);
protected:
    std::unordered_map<uint32_t, callback_handler_t> selectors;
};

template<class SelectorConvention>
class DispatcherTrap : public GenericDispatcherTrap
{
    static syn68k_addr_t invokeFrom68K(syn68k_addr_t addr, void* extra);
    const char *name;
    uint16_t trapno;
public:
    void init();
    DispatcherTrap(const char* name, uint16_t trapno) : name(name), trapno(trapno) {}
};

template<typename F, F* fptr, typename CallConv = callconv::Pascal>
class WrappedFunction {};

template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename CallConv>
class WrappedFunction<Ret (Args...), fptr, CallConv> : public internal::DeferredInit
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

    virtual void init() override;

    WrappedFunction(const char* name);
protected:
    UPP<Ret (Args...)> guestFP;
    const char *name;
};

template<syn68k_addr_t (*fptr)(syn68k_addr_t, void*)>
class WrappedFunction<syn68k_addr_t (syn68k_addr_t,void*), fptr, callconv::Raw> : public internal::DeferredInit
{
public:
    ProcPtr operator&() const
    {
        return guestFP;
    }

    virtual void init() override;

    WrappedFunction(const char* name);
protected:
    ProcPtr guestFP;
    const char *name;
};

template<typename F, F* fptr, int trapno, typename CallConv = callconv::Pascal>
class TrapFunction {};

template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno, typename CallConv>
class TrapFunction<Ret (Args...), fptr, trapno, CallConv> : public WrappedFunction<Ret (Args...), fptr, CallConv>
{
public:
    Ret operator()(Args... args) const { return fptr(args...); }
    
    virtual void init() override;

    TrapFunction(const char* name) : WrappedFunction<Ret(Args...),fptr,CallConv>(name) {}
};

template<typename F, F* fptr, int trapno, uint32_t selector, typename CallConv = callconv::Pascal>
class SubTrapFunction {};

template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno, uint32_t selector, typename CallConv>
class SubTrapFunction<Ret (Args...), fptr, trapno, selector, CallConv> : public WrappedFunction<Ret (Args...), fptr, CallConv>
{
public:
    Ret operator()(Args... args) const { return fptr(args...); }
    SubTrapFunction(const char* name, GenericDispatcherTrap& dispatcher);
private:
    GenericDispatcherTrap& dispatcher;
};


#if !defined(FUNCTION_WRAPPER_IMPLEMENTATION) /* defined in functions.cpp */

#define CREATE_FUNCTION_WRAPPER(TYPE, NAME, FPTR, ...) \
    extern Executor::functions::TYPE NAME

#define DISPATCHER_TRAP(NAME, TRAP, SELECTOR) \
    extern Executor::functions::DispatcherTrap<Executor::functions::selectors::SELECTOR> NAME

#else

#define CREATE_FUNCTION_WRAPPER(TYPE, NAME, FPTR, ...) \
    Executor::functions::TYPE NAME(__VA_ARGS__);   \
    template class Executor::functions::TYPE;

#define DISPATCHER_TRAP(NAME, TRAP, SELECTOR) \
    Executor::functions::DispatcherTrap<Executor::functions::selectors::SELECTOR> NAME { #NAME, TRAP }

#endif

#define EXTERN_DISPATCHER_TRAP(NAME, TRAP, SELECTOR) \
    extern Executor::functions::DispatcherTrap<Executor::functions::selectors::SELECTOR> NAME


#define COMMA ,
#define PASCAL_TRAP(NAME, TRAP) \
    CREATE_FUNCTION_WRAPPER(TrapFunction<decltype(C_##NAME) COMMA &C_##NAME COMMA TRAP>, NAME, &C_##NAME, #NAME)
#define PASCAL_SUBTRAP(NAME, TRAP, SELECTOR, TRAPNAME) \
    CREATE_FUNCTION_WRAPPER(SubTrapFunction<decltype(C_##NAME) COMMA &C_##NAME COMMA TRAP COMMA SELECTOR>, NAME, &C_##NAME, #NAME, TRAPNAME)
#define NOTRAP_FUNCTION(NAME) \
    CREATE_FUNCTION_WRAPPER(WrappedFunction<decltype(C_##NAME) COMMA &C_##NAME>, NAME, &C_##NAME, #NAME)
#define PASCAL_FUNCTION(NAME) \
    CREATE_FUNCTION_WRAPPER(WrappedFunction<decltype(C_##NAME) COMMA &C_##NAME>, NAME, &C_##NAME, #NAME)
#define RAW_68K_FUNCTION(NAME) \
    syn68k_addr_t _##NAME(syn68k_addr_t, void *); \
    CREATE_FUNCTION_WRAPPER(WrappedFunction<decltype(_##NAME) COMMA &_##NAME COMMA callconv::Raw>, stub_##NAME, &_##NAME, #NAME)
#define RAW_68K_TRAP(NAME, TRAP) \
    syn68k_addr_t _##NAME(syn68k_addr_t, void *); \
    CREATE_FUNCTION_WRAPPER(TrapFunction<decltype(_##NAME) COMMA &_##NAME COMMA TRAP COMMA callconv::Raw>, stub_##NAME, &_##NAME, #NAME)

    
void resetNestingLevel();
void init();

}

}

#endif