#ifndef PASCAL_H_
#define PASCAL_H_

#include <stdint.h>
#include <type_traits>

namespace Executor
{

namespace ptoc_internal
{
    template<typename T>
    struct PTOCElement
    {
        constexpr static
        std::enable_if<std::is_integral<T>::value && sizeof(T) <= 4, int>
        code = sizeof(T);
    };

    template<typename T>
    struct PTOCElement<T*>
    {
        constexpr static int code = 5;
    };

    // TODO: POINT
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
        return (*f)(args...);
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