
#pragma once

#include <rsys/functions.h>
#include <string.h>

//#define USE_DYNAMIC_CTOP_PTOC

#ifdef USE_DYNAMIC_CTOP_PTOC
#include <rsys/ctop_ptoc.h>
#endif

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
    template<class T, class CC>
    static void set(UPP<T,CC> x) { EM_AREG(n) = ptr_to_longint((void*)x); }

    template<class T>
    static void afterCall(T) {}
};

template<int n> struct D
{
    template<typename T>
    operator T() { return EM_DREG(n); }
    static void set(uint32_t x) { EM_DREG(n) = x; }

    template<class T>
    static void afterCall(T) {}
};

struct D0HighWord
{
    operator uint16_t() { return EM_D0 >> 16; }

    static void set(uint16_t x)
    {
        EM_D0 = (EM_D0 & 0xFFFF) | (x << 16);
    }

    template<class T>
    static void afterCall(T) {}    
};
struct D0LowWord
{
    operator uint16_t() { return EM_D0 & 0xFFFF; }

    static void set(uint16_t x)
    {
        EM_D0 = (EM_D0 & 0xFFFF0000) | x;
    }

    template<class T>
    static void afterCall(T) {}    
};

template<typename Loc, typename T> struct Out
{
    T temp;

    operator T*() { return &temp; }

    ~Out() { Loc::set(temp); }

    static void set(T* p) {}
    static void afterCall(T* p) { *p = Loc(); }
};

template<typename Loc, typename T> struct InOut
{
    T temp;

    operator T*() { return &temp; }

    InOut() { temp = Loc(); }
    ~InOut() { Loc::set(temp); }

    static void set(T* p) { Loc::set(*p); }
    static void afterCall(T* p) { *p = Loc(); }
};


template<int mask> struct TrapBit
{
    operator bool() { return !!(EM_D1 & mask); }

    static void set(bool b)
    {
        if(b)
            EM_D1 |= mask;
        else
            EM_D1 &= ~mask;
    }
    template<class T>
    static void afterCall(T) {}
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

namespace stack
{
    template<class T>
    void push(T x)
    {
        EM_A7 -= (sizeof(GUEST<T>) + 1) & ~1;
        *(ptr_from_longint<GUEST<T>*>(EM_A7)) = RM(x);
    }
    template<class T>
    T pop()
    {
        T retval = MR(*ptr_from_longint<GUEST<T>*>(EM_A7));
        EM_A7 += (sizeof(GUEST<T>) + 1) & ~1;
        return retval;
    }
}
}

namespace callfrom68K
{
    template<typename F, F *fptr, typename CallConv>
    struct Invoker;

    template<typename... Xs> struct List;

#ifdef USE_DYNAMIC_CTOP_PTOC
    template<typename Ret, typename... Args, Ret (*fptr)(Args...)>
    struct Invoker<Ret (Args...), fptr, callconv::Pascal>
    {
        static syn68k_addr_t invokeFrom68K(syn68k_addr_t addr, void *)
        {
            printf("---> %x\n", EM_A7);
            static ptocblock_t ptocblock { (void*)fptr, ptoc(fptr) };
            syn68k_addr_t ret = PascalToCCall(addr, &ptocblock);
            printf("|--> %x\n", EM_A7);
            return ret;
        }
    };
#else

    template<typename F, typename Args, typename ToDoArgs>
    struct PascalInvoker;

    template<typename Ret, typename... Args>
    struct PascalInvoker<Ret (Args...), List<Args...>, List<>>
    {
        static void invokeFrom68K(Ret (*fptr)(Args...), Args... args)
        {
            Ret retval = fptr(args...);
            *ptr_from_longint<GUEST<Ret>*>(EM_A7) = RM(retval);
        }
    };

    template<typename... Args>
    struct PascalInvoker<void (Args...), List<Args...>, List<>>
    {
        static void invokeFrom68K(void (*fptr)(Args...), Args... args)
        {
            fptr(args...);
        }
    };

    template<typename F, typename... Args, typename Arg, typename... ToDoArgs>
    struct PascalInvoker<F, List<Args...>, List<Arg, ToDoArgs...>>
    {
        static void invokeFrom68K(F* fptr, Args... args)
        {
            Arg arg = callconv::stack::pop<Arg>();
            PascalInvoker<F, List<Arg, Args...>, List<ToDoArgs...>>::invokeFrom68K(fptr, arg, args...);
        }
    };

    template<typename SrcList, typename DstList = List<>>
    struct Reverse;

    template<typename... Xs, typename X, typename... Ys>
    struct Reverse<List<X, Xs...>, List<Ys...>>
    {
        using type = typename Reverse<List<Xs...>, List<X,Ys...>>::type;
    };

    template<typename... Ys>
    struct Reverse<List<>, List<Ys...>>
    {
        using type = List<Ys...>;
    };
    
    template<typename Ret, typename... Args, Ret (*fptr)(Args...)>
    struct Invoker<Ret (Args...), fptr, callconv::Pascal>
    {
        static syn68k_addr_t invokeFrom68K(syn68k_addr_t addr, void *)
        {
            syn68k_addr_t retaddr = callconv::stack::pop<syn68k_addr_t>();
            PascalInvoker<Ret (Args...), List<>, typename Reverse<List<Args...>>::type>::invokeFrom68K(fptr);
            return retaddr;
        }
    };
#endif

    template<syn68k_addr_t (*fptr)(syn68k_addr_t, void*)>
    struct Invoker<syn68k_addr_t (syn68k_addr_t addr, void *), fptr, callconv::Raw>
    {
        static syn68k_addr_t invokeFrom68K(syn68k_addr_t addr, void * refcon)
        {
            return fptr(addr, refcon);
        }
    };

    template<typename F, F *fptr, typename DoneArgs, typename ToDoArgs, typename ToDoCC>
    struct RegInvoker;

    template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename... DoneArgs>
    struct RegInvoker<Ret (Args...), fptr, List<DoneArgs...>, List<>, List<>>
    {
        static Ret invokeFrom68K(syn68k_addr_t addr, void *refcon, DoneArgs... args)
        {
            return fptr(args...);
        }
    };

    template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename... DoneArgs, typename Arg, typename... TodoArgs, typename CC, typename... TodoCC>
    struct RegInvoker<Ret (Args...), fptr, List<DoneArgs...>, List<Arg, TodoArgs...>, List<CC, TodoCC...>>
    {
        static Ret invokeFrom68K(syn68k_addr_t addr, void *refcon, DoneArgs... args)
        {
            CC newarg;
            return RegInvoker<Ret (Args...),fptr,List<DoneArgs...,Arg>,List<TodoArgs...>,List<TodoCC...>>
                ::invokeFrom68K(addr, refcon, args..., newarg);
        }
    };

    template<typename Ret, typename... Args, Ret (*fptr)(Args...), typename RetConv, typename... ArgConvs>
    struct Invoker<Ret (Args...), fptr, callconv::Register<RetConv (ArgConvs...)>>
    {
        static syn68k_addr_t invokeFrom68K(syn68k_addr_t addr, void *refcon)
        {
            syn68k_addr_t retaddr = POPADDR();
            Ret retval = RegInvoker<Ret (Args...), fptr, List<>, List<Args...>, List<ArgConvs...>>::invokeFrom68K(addr, refcon);
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
            RegInvoker<void (Args...), fptr, List<>, List<Args...>, List<ArgConvs...>>::invokeFrom68K(addr, refcon);
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

#ifdef USE_DYNAMIC_CTOP_PTOC
    template<typename Ret, typename... Args>
    struct Invoker<Ret (Args...), callconv::Pascal>
    {
        static Ret invoke68K(void *ptr, Args... args)
        {
            return (Ret)CToPascalCall(ptr, ctop<Ret(Args...)>(), args...);
        }
    };
#else
    template<>
    struct Invoker<void (), callconv::Pascal>
    {
        static void invoke68K(void *ptr)
        {
            M68kReg saveregs[14];
            memcpy(saveregs, &EM_D1, sizeof(saveregs)); /* d1-d7/a0-a6 */
            CALL_EMULATOR(US_TO_SYN68K(ptr));
            memcpy(&EM_D1, saveregs, sizeof(saveregs)); /* d1-d7/a0-a6 */
        }
    };
    template<typename Arg, typename... Args>
    struct Invoker<void (Arg, Args...), callconv::Pascal>
    {
        static void invoke68K(void *ptr, Arg arg, Args... args)
        {
            callconv::stack::push(arg);
            Invoker<void (Args...), callconv::Pascal>::invoke68K(ptr, args...);
        }
    };
    template<typename Ret, typename... Args>
    struct Invoker<Ret (Args...), callconv::Pascal>
    {
        static Ret invoke68K(void *ptr, Args... args)
        {
            callconv::stack::push(Ret());
            Invoker<void (Args...), callconv::Pascal>::invoke68K(ptr, args...);
            return callconv::stack::pop<Ret>();
        }
    };
#endif
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

template<typename Ret, typename... Args, typename CallConv>
Ret UPP<Ret(Args...), CallConv>::operator()(Args... args) const
{
    return callto68K::Invoker<Ret(Args...), CallConv>::invoke68K(ptr, args...);
}

template<typename Ret, typename... Args, typename CallConv>
Ret GuestWrapperBase<UPP<Ret(Args...),CallConv>>::operator()(Args... args)
{
    return (this->get())(args...);
}

}
