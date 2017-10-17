#if !defined (_MACTYPE_H_)
#define _MACTYPE_H_

/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: mactype.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "rsys/macros.h"
#include "rsys/types.h"
#include <type_traits>

#ifndef __cplusplus
#error C++ required
#endif

namespace Executor {

typedef int16 INTEGER;
typedef int32 LONGINT;
typedef uint32 ULONGINT;

#if !defined (USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)
typedef int8 BOOLEAN;
typedef int16 CHAR; /* very important not to use this as char */
#endif

typedef struct { int32 l PACKED; } HIDDEN_LONGINT;
typedef struct { uint32 u PACKED; } HIDDEN_ULONGINT;

template<typename TT>
struct GuestWrapper
{
    union
    {
        uint16_t align;
        uint8_t data[sizeof(TT)];
    } x;
    
    using WrappedType = TT;
    using RawGuestType = TT;
    
    const TT unwrap() const { return *(const TT*)x.data; }
    TT& unwrap() { return *(TT*)x.data; }

    GuestWrapper() = default;
    GuestWrapper(const GuestWrapper<TT>& y) = default;

    GuestWrapper<TT>& operator=(const GuestWrapper<TT>& y) = default;

    /*WrappedType get() const
    {
    }

    void set(WrappedType x)
    {
    }

    RawGuestType raw() const
    {

    }

    void raw(RawGuestType)
    {

    }*/



        // Things that should go away at some point
    GuestWrapper(TT x) { unwrap() = x; }
    GuestWrapper<TT>& operator=(TT y) { this->unwrap() = y; return *this; }
    operator TT() const { return this->unwrap(); }
};

template<typename TT>
struct GuestWrapper<TT*>
{
    union
    {
        uint16_t align;
        uint8_t data[4];
    } x;
    
    using WrappedType = TT*;
    using RawGuestType = uint32_t;

    const RawGuestType unwrap() const { return *(const RawGuestType*)x.data; }
    RawGuestType& unwrap() { return *(RawGuestType*)x.data; }

    GuestWrapper() = default;
    GuestWrapper(const GuestWrapper<WrappedType>& y) = default;

    GuestWrapper<WrappedType>& operator=(const GuestWrapper<WrappedType>& y) = default;

        // Things that should go away at some point
    GuestWrapper(TT* x) { unwrap() = (uint32_t)x; }
    GuestWrapper<WrappedType>& operator=(TT* y) { this->unwrap() = (uint32_t)y; return *this; }
    operator TT*() const { return (TT*)this->unwrap(); }

    GuestWrapper(std::nullptr_t) { unwrap() = 0; }    
};

struct GuestStruct
{
};

template <typename TT>
struct GuestType
{
    using type = typename std::conditional<
            std::is_base_of<GuestStruct, TT>::value,
            TT,
            GuestWrapper<TT>
        >::type;
};

// forward declare.
// uses template specialization to bypass the above,
// so a ": GuestStruct" on the actual declaration is redundant (but still fine)
#define GUEST_STRUCT(CLS)   \
    struct CLS;\
    template<>  \
    struct GuestType<CLS>    \
    {   \
        using type = CLS;    \
    }
    

template<typename TT>
using GUEST = typename GuestType<TT>::type;

template<>
struct GuestType<int8_t>
{
    using type = int8_t;
};

template<>
struct GuestType<uint8_t>
{
    using type = uint8_t;
};

/*
template<typename TT>
struct GuestType<TT*>
{
    using type = GuestPointerWrapper<GUEST<TT>>;
};


template<typename RT, typename... Args>
struct GuestType<RT (*)(Args...)>
{
    using type = GuestPointerWrapper<void>;
};
*/

template<typename TT, int n>
struct GuestType<TT[n]>
{
    using type = GUEST<TT>[n];
};

/*
template<typename TO, typename FROM>
GUEST<TO*> guest_ptr_cast(GUEST<FROM*> p)
{
    return GUEST<TO*>((FROM*)p);
}*/
template<typename TO, typename FROM>
GUEST<TO> guest_cast(GuestWrapper<FROM> p)
{
    return GUEST<TO>((TO)(FROM)p);
}

#define MAKE_HIDDEN(typ) struct  HIDDEN_ ## typ : GuestStruct { typ p; }
// Roadmap:
// 1. switch to
//  #define MAKE_HIDDEN(typ) using  HIDDEN_ ## typ = GUEST<p>
//  (adapt STARH, other uses of ->P)
// 2. remove

#  define PACKED_MEMBER(typ, name) typ name
// Roadmap:
// 1. remove

}

#endif /* _MACTYPE_H_ */
