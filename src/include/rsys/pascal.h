#ifndef PASCAL_H_
#define PASCAL_H_

#include <stdint.h>
#include <type_traits>
#include "rsys/trapglue.h"
#include "rsys/mactype.h"

namespace Executor
{

using CTOPCode = uint64_t;

namespace ptoc_internal
{
    template<typename A, typename... Types>
    struct CTOP
    {
        constexpr static CTOPCode value = 
            CTOP<A>::value | (CTOP<Types...>::value << 3);
    };

    template<typename T>
    struct CTOP<T>
    {
        constexpr static
        typename std::enable_if<std::is_integral<T>::value && sizeof(T) <= 4, CTOPCode>::type
        value = sizeof(T);
    };

    template<typename T>
    struct CTOP<T*>
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
        constexpr static CTOPCode value = 
            (PTOC<A>::value << Rec::bits) | (Rec::value);
        constexpr static int bits = 3 + Rec::bits;
    };

    template<typename T>
    struct PTOC<T>
    {
        constexpr static CTOPCode value = CTOP<T>::value;
        constexpr static int bits = 3;
    };


}


template<typename Ret, typename... Args>
constexpr CTOPCode ptoc(Ret (*)(Args...) = nullptr)
{
    return ptoc_internal::PTOC<Ret>::value | (ptoc_internal::PTOC<Args...>::value << 3);
}
template<typename Ret>
constexpr CTOPCode ptoc(Ret (*)() = nullptr)
{
    return ptoc_internal::PTOC<Ret>::value;
}

template<typename Ret, typename... Args>
constexpr CTOPCode ctop(Ret (*)(Args...) = nullptr)
{
    return ptoc_internal::CTOP<Ret, Args...>::value;
}

template<typename Ret, typename... Args>
class PascalTrap
{
public:
    using ProcPtr = Ret (*)(Args...);

    constexpr PascalTrap(ProcPtr impl, uint16_t trapno)
        : f(impl), trapno(trapno)
    {
    }

    Ret operator()(Args... args) const
    {
        if((trapno & TOOLBIT) == 0)
            return (*f)(args...);
        else
        {
            syn68k_addr_t new_addr = tooltraptable[trapno & (NTOOLENTRIES-1)];
        
            if(new_addr == toolstuff[trapno & (NTOOLENTRIES-1)].orig)
                return (*f)(args...);
            else
                return (Ret)CToPascalCall(SYN68K_TO_US(new_addr), ctop(f), args...);
        }
    }

private:
    ProcPtr f;
    uint16_t trapno;
};

template<typename Ret, typename... Args> constexpr PascalTrap<Ret, Args...> pascalTrap(Ret (*impl)(Args...), uint16_t trapno)
{
    return PascalTrap<Ret, Args...>(impl, trapno);
}

#define PASCAL_TRAP(NAME, TRAP) constexpr auto NAME = pascalTrap(&C_ ## NAME, TRAP)
#define PASCAL_FUNCTION(NAME) constexpr auto NAME = pascalTrap(&C_ ## NAME, 0)
}

#endif