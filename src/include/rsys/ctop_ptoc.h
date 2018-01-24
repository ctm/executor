#pragma once

namespace Executor
{

template<typename F>
struct UPP;

struct Point;

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

}