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
struct PACKED GuestWrapper
{
    union
    {
        uint16_t align;
        uint8_t data[sizeof(TT)];
    } x;
    
public:
    using WrappedType = TT;
    
    const TT unwrap() const { return *(const TT*)x.data; }
    TT& unwrap() { return *(TT*)x.data; }

    GuestWrapper() {}
    GuestWrapper(TT x) { unwrap() = x; }
    GuestWrapper(const GuestWrapper<TT>& y) = default;

    operator TT() const { return unwrap(); }
    GuestWrapper<TT>& operator=(TT y) { unwrap() = y; return *this; }
    GuestWrapper<TT>& operator=(const GuestWrapper<TT>& y) = default;
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



}

#endif /* _MACTYPE_H_ */
