#ifndef PASCAL_H_
#define PASCAL_H_

#include <stdint.h>
#include <type_traits>
#include <utility>
#include <vector>
#include "rsys/trapglue.h"
#include "rsys/mactype.h"

namespace Executor
{

template<typename F>
struct UPP;

using CTOPCode = uint64_t;

namespace ptoc_internal
{
    template<typename A, typename... Types>
    struct CTOP
    {
        constexpr static CTOPCode value = CTOP<A>::value | (CTOP<Types...>::value << 3);
    };

    template<typename T>
    struct CTOP<T>
    {
        constexpr static
            typename std::enable_if<std::is_integral<T>::value && sizeof(T) <= 4, CTOPCode>::type
                value
            = sizeof(T);
    };

    template<typename T>
    struct CTOP<T *>
    {
        constexpr static CTOPCode value = 5;
    };

    template<typename T>
    struct CTOP<UPP<T>>
    {
        constexpr static CTOPCode value = 5;
    };

    template<>
    struct CTOP<Point>
    {
        constexpr static CTOPCode value = 3;
    };
    template<>
    struct CTOP<void>
    {
        constexpr static CTOPCode value = 0;
    };

    template<typename A, typename... Types>
    struct PTOC
    {
        using Rec = PTOC<Types...>;
        constexpr static CTOPCode value = (PTOC<A>::value << Rec::bits) | (Rec::value);
        constexpr static int bits = 3 + Rec::bits;
    };

    template<typename T>
    struct PTOC<T>
    {
        constexpr static CTOPCode value = CTOP<T>::value;
        constexpr static int bits = 3;
    };

    template<typename T>
    struct helper
    {
        constexpr static bool enable = false;
    };
    template<typename Ret>
    struct helper<Ret()>
    {
        constexpr static bool enable = true;
        constexpr static CTOPCode ctop = CTOP<Ret>::value;
        constexpr static CTOPCode ptoc = PTOC<Ret>::value;
    };

    template<typename Ret, typename... Args>
    struct helper<Ret(Args...)>
    {
        constexpr static bool enable = true;
        constexpr static CTOPCode ctop = CTOP<Ret, Args...>::value;
        constexpr static CTOPCode ptoc = PTOC<Ret>::value | (PTOC<Args...>::value << 3);
    };
}

template<typename F, typename = std::enable_if<ptoc_internal::helper<F>::enable>>
constexpr CTOPCode ptoc(F * = nullptr)
{
    return ptoc_internal::helper<F>::ptoc;
}
template<typename F, typename = std::enable_if<ptoc_internal::helper<F>::enable>>
constexpr CTOPCode ctop(F * = nullptr)
{
    return ptoc_internal::helper<F>::ctop;
}

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
    Ret operator()(Args... args) const
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
                return (Ret)CToPascalCall(SYN68K_TO_US(new_addr), ctop(fptr), args...);
        }
    }
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
#define RAW_68K_FUNCTION(NAME) CREATE_FUNCTION_WRAPPER(Raw68KFunction<&_##NAME>, stub_##NAME)

class InitAction
{
public:
    InitAction(void (*f)());
    static void execute();
};

}

}

#endif