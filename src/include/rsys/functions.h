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

using A0 = A<0>;
using A1 = A<1>;
using A2 = A<2>;
using D0 = D<0>;
using D1 = D<1>;
using D2 = D<2>;

struct D0HighWord;
struct D0LowWord;

template <int mask> struct TrapBit;
template<typename loc, typename T> struct Out;
template<typename loc, typename T> struct InOut;

template<typename Loc> struct ReturnMemErr;
struct CCFromD0;

struct ClearD0;
struct SaveA1D1D2;
struct MoveA1ToA0;


}
using namespace callconv;

namespace callfrom68K
{
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
}

namespace callto68K
{
    template<typename F, typename CallConv>
    struct Invoker;

    template<typename F, typename... Args>
    struct InvokerRec;

    template<>
    struct InvokerRec<void ()>
    {
        static void invoke68K(void *ptr)
        {
            CALL_EMULATOR(US_TO_SYN68K(ptr));
        }
    };

    template<typename Arg, typename... Args, typename ArgConv, typename... ArgConvs>
    struct InvokerRec<void (Arg, Args...), ArgConv, ArgConvs...>
    {
        static void invoke68K(void *ptr, Arg arg, Args... args)
        {
            ArgConv::set(arg);
            InvokerRec<void (Args...), ArgConvs...>::invoke68K(ptr, args...);
            ArgConv::afterCall(arg);
        }
    };
   
    template<typename Ret, typename... Args, typename RetConv, typename... ArgConvs, typename... Extras>
    struct Invoker<Ret (Args...), callconv::Register<RetConv(ArgConvs...), Extras...>>
    {
        static Ret invoke68K(void *ptr, Args... args)
        {
            InvokerRec<void (Args...), ArgConvs...>::invoke68K(ptr, args...);
            return RetConv();
        }
    };

    template<typename... Args, typename... ArgConvs, typename... Extras>
    struct Invoker<void (Args...), callconv::Register<void(ArgConvs...), Extras...>>
    {
        static void invoke68K(void *ptr, Args... args)
        {
            InvokerRec<void (Args...), ArgConvs...>::invoke68K(ptr, args...);
        }
    };


    template<typename Ret, typename... Args, typename RetArgConv, typename... Extras>
    struct Invoker<Ret (Args...), callconv::Register<RetArgConv, Extras...>>
    {
        static Ret invoke68K(void *ptr, Args... args)
        {
            return (Ret)CToPascalCall(ptr, ctop<Ret(Args...)>(), args...);
        }
    };

    template<typename Ret, typename... Args>
    struct Invoker<Ret (Args...), callconv::Pascal>
    {
        static Ret invoke68K(void *ptr, Args... args)
        {
            return (Ret)CToPascalCall(ptr, ctop<Ret(Args...)>(), args...);
        }
    };

    template<>
    struct Invoker<syn68k_addr_t (syn68k_addr_t, void*), callconv::Raw>
    {
        static syn68k_addr_t invoke68K(void *ptr, syn68k_addr_t, void *)
        {
            CALL_EMULATOR(US_TO_SYN68K(ptr));
            return POPADDR();   // ###
        }
    };

}

struct OpaqueUntyped68KProc;
typedef OpaqueUntyped68KProc* ProcPtr;

template<typename F, typename CallConv = callconv::Pascal>
struct UPP {};

template<typename Ret, typename... Args, typename CallConv>
struct UPP<Ret(Args...), CallConv>
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

    template<class T>   
    explicit operator T *() const { return (T*)ptr; }
    explicit operator ProcPtr() const { return (ProcPtr)ptr; }

    explicit operator bool() const { return ptr != nullptr; }
    bool operator==(UPP<Ret(Args...)> other) const { return ptr == other.ptr; }
    bool operator!=(UPP<Ret(Args...)> other) const { return ptr != other.ptr; }

    Ret operator()(Args... args) const
    {
        return callto68K::Invoker<Ret(Args...), CallConv>::invoke68K(ptr, args...);
    }
};


namespace functions
{

namespace selectors
{
    template <uint32_t mask> struct D0;
    using D0W = D0<0xFFFF>;
    using D0L = D0<0xFFFFFFFF>;
    template <uint32_t mask = 0xFFFF> struct StackWMasked;
    template <uint32_t mask = 0xFFFFFFFF> struct StackLMasked;
    using StackW = StackWMasked<>;
    using StackL = StackLMasked<>;
    template <uint32_t mask = 0xFFFF> struct StackWLookahead;
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
    virtual void addSelector(uint32_t sel, callback_handler_t handler) = 0;
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
    virtual void init() override;
    virtual void addSelector(uint32_t sel, callback_handler_t handler) override;
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

    UPP<Ret (Args...), CallConv> operator&() const
    {
        return guestFP;
    }

    virtual void init() override;

    WrappedFunction(const char* name);

    using UPPType = UPP<Ret (Args...), CallConv>;
protected:
    UPPType guestFP;
    const char *name;
};

template<typename F, F* fptr, int trapno, typename CallConv = callconv::Pascal>
class TrapFunction {};

template<typename Ret, typename... Args, Ret (*fptr)(Args...), int trapno, typename CallConv>
class TrapFunction<Ret (Args...), fptr, trapno, CallConv> : public WrappedFunction<Ret (Args...), fptr, CallConv>
{
public:
    using UPPType = typename WrappedFunction<Ret (Args...), fptr, CallConv>::UPPType;

    Ret operator()(Args... args) const
    {
        if(isPatched())
            return invokeViaTrapTable(args...);
        else
            return fptr(args...);
    }
    
    virtual void init() override;

    TrapFunction(const char* name) : WrappedFunction<Ret(Args...),fptr,CallConv>(name) {}

    bool isPatched() const { return tableEntry() != originalFunction; }
    Ret invokeViaTrapTable(Args...) const;
private:
    syn68k_addr_t originalFunction;

    syn68k_addr_t& tableEntry() const
    {
        if(trapno & TOOLBIT)
            return tooltraptable[trapno & 0x3FF];
        else
            return ostraptable[trapno & 0xFF];
    }
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

#define CREATE_FUNCTION_WRAPPER(NAME, FPTR, INIT, ...) \
    extern Executor::functions::__VA_ARGS__ NAME

#define DISPATCHER_TRAP(NAME, TRAP, SELECTOR) \
    extern Executor::functions::DispatcherTrap<Executor::functions::selectors::SELECTOR> NAME

#else

#define CREATE_FUNCTION_WRAPPER(NAME, FPTR, INIT, ...) \
    Executor::functions::__VA_ARGS__ NAME INIT;   \
    template class Executor::functions::__VA_ARGS__;

#define DISPATCHER_TRAP(NAME, TRAP, SELECTOR) \
    Executor::functions::DispatcherTrap<Executor::functions::selectors::SELECTOR> NAME { #NAME, TRAP }

#endif

#define EXTERN_DISPATCHER_TRAP(NAME, TRAP, SELECTOR) \
    extern Executor::functions::DispatcherTrap<Executor::functions::selectors::SELECTOR> NAME


#define COMMA ,
#define PASCAL_TRAP(NAME, TRAP) \
    CREATE_FUNCTION_WRAPPER(NAME, &C_##NAME, (#NAME), TrapFunction<decltype(C_##NAME) COMMA &C_##NAME COMMA TRAP>)
#define REGISTER_TRAP(NAME, TRAP, ...) \
    CREATE_FUNCTION_WRAPPER(NAME, &C_##NAME, (#NAME), TrapFunction<decltype(C_##NAME) COMMA &C_##NAME COMMA TRAP COMMA callconv::Register<__VA_ARGS__>>)
#define REGISTER_TRAP2(NAME, TRAP, ...) \
    CREATE_FUNCTION_WRAPPER(stub_##NAME, &NAME, (#NAME), TrapFunction<decltype(NAME) COMMA &NAME COMMA TRAP COMMA callconv::Register<__VA_ARGS__>>)

#define PASCAL_SUBTRAP(NAME, TRAP, SELECTOR, TRAPNAME) \
    CREATE_FUNCTION_WRAPPER(NAME, &C_##NAME, (#NAME, TRAPNAME), SubTrapFunction<decltype(C_##NAME) COMMA &C_##NAME COMMA TRAP COMMA SELECTOR>)
#define REGISTER_SUBTRAP(NAME, TRAP, SELECTOR, TRAPNAME, ...) \
    CREATE_FUNCTION_WRAPPER(NAME, &C_##NAME, (#NAME, TRAPNAME), SubTrapFunction<decltype(C_##NAME) COMMA &C_##NAME COMMA TRAP COMMA SELECTOR COMMA callconv::Register<__VA_ARGS__>>)
#define REGISTER_SUBTRAP2(NAME, TRAP, SELECTOR, TRAPNAME, ...) \
    CREATE_FUNCTION_WRAPPER(stub_##NAME, &NAME, (#NAME, TRAPNAME), SubTrapFunction<decltype(NAME) COMMA &NAME COMMA TRAP COMMA SELECTOR COMMA callconv::Register<__VA_ARGS__>>)

#define NOTRAP_FUNCTION(NAME) \
    CREATE_FUNCTION_WRAPPER(NAME, &C_##NAME, (#NAME), WrappedFunction<decltype(C_##NAME) COMMA &C_##NAME>)
#define PASCAL_FUNCTION(NAME) \
    CREATE_FUNCTION_WRAPPER(NAME, &C_##NAME, (#NAME), WrappedFunction<decltype(C_##NAME) COMMA &C_##NAME>)
#define RAW_68K_FUNCTION(NAME) \
    syn68k_addr_t _##NAME(syn68k_addr_t, void *); \
    CREATE_FUNCTION_WRAPPER(stub_##NAME, &_##NAME, (#NAME), WrappedFunction<decltype(_##NAME) COMMA &_##NAME COMMA callconv::Raw>)
#define RAW_68K_TRAP(NAME, TRAP) \
    syn68k_addr_t _##NAME(syn68k_addr_t, void *); \
    CREATE_FUNCTION_WRAPPER(stub_##NAME, &_##NAME, (#NAME), TrapFunction<decltype(_##NAME) COMMA &_##NAME COMMA TRAP COMMA callconv::Raw>)


#define ASYNCBIT (1 << 10)
#define HFSBIT (1 << 9)

#define FILE_TRAP(NAME, TRAP) \
    REGISTER_TRAP2(NAME, TRAP, D0 (A0, TrapBit<ASYNCBIT>))

#define FILE_SUBTRAP(NAME, TRAP, SELECTOR, TRAPNAME) \
    REGISTER_SUBTRAP2(NAME, TRAP, SELECTOR, TRAPNAME, D0 (A0, TrapBit<ASYNCBIT>))

#define HFS_TRAP(NAME, HNAME, PBTYPE, TRAP) \
    inline OSErr NAME##_##HNAME(ParmBlkPtr pb, Boolean async, Boolean hfs) \
    { \
        return hfs ? HNAME((PBTYPE)pb, async) : NAME(pb, async); \
    } \
    CREATE_FUNCTION_WRAPPER(stub_##NAME, &NAME##_##HNAME, \
        (#NAME "/" #HNAME), \
        TrapFunction<decltype(NAME##_##HNAME), \
            &NAME##_##HNAME, TRAP, \
            callconv::Register<D0 (A0, TrapBit<ASYNCBIT>, TrapBit<HFSBIT>)>>)

void resetNestingLevel();
void init(bool enableLogging);

}

}

#endif