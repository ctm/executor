#pragma once

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

    Ret operator()(Args... args) const;
};

}
