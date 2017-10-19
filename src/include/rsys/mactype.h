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
#include <syn68k_public.h> /* for ROMlib_offset */

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


// Define alignment.

// Most things are two-byte aligned.
template<typename T>
struct Aligner
{
    uint16_t align;
};

// Except for chars
template<> struct Aligner<char> { uint8_t align; };
template<> struct Aligner<unsigned char> { uint8_t align; };
template<> struct Aligner<signed char> { uint8_t align; };


// For now, SwapTyped is exactly the same as Cx.
// However, Cx is used all over Executor, should learn to handle GUEST<> types,
// and eventually go away. SwapTyped is used internally here, and nowhere else.

inline unsigned char SwapTyped(unsigned char x) { return x; }
inline signed char SwapTyped(signed char x) { return x; }
inline char SwapTyped(char x) { return x; }

inline uint16_t SwapTyped(uint16_t x) { return swap16(x); }
inline int16_t SwapTyped(int16_t x) { return swap16((uint16_t)x); }

inline uint32_t SwapTyped(uint32_t x) { return swap32(x); }
inline int32_t SwapTyped(int32_t x) { return swap32((uint32_t)x); }


// USE_PACKED_HIDDENVALUE - control which one of two versions
// of template struct/union HiddenValue to use.
// Only the PACKED version currently works reliably,
// the union version requires -fno-strict-aliasing.
// ... and I haven't quite figured out why.
// Would have been nice to do this without any non-standard __attribute__s.

#define USE_PACKED_HIDDENVALUE


template<typename ActualType, typename FakeType = ActualType>
#ifdef USE_PACKED_HIDDENVALUE
struct  __attribute__((packed, align(2))) HiddenValue
#else
union HiddenValue
#endif
{
#ifdef USE_PACKED_HIDDENVALUE
    ActualType packed;
#else
    uint8_t data[sizeof(ActualType)];
    Aligner<ActualType> align;
#endif
public:
    HiddenValue() = default;
    HiddenValue(const HiddenValue<ActualType,FakeType>& y) = default;
    HiddenValue<ActualType,FakeType>& operator=(const HiddenValue<ActualType,FakeType>& y) = default;
    
    ActualType raw() const
    {
#ifdef USE_PACKED_HIDDENVALUE
        return packed;
#else
        return *(const ActualType*)data;
#endif
    }

    void raw(ActualType x)
    {
#ifdef USE_PACKED_HIDDENVALUE
        packed = x;
#else
        *(ActualType*)data = x;         
#endif
    }


    HiddenValue(FakeType x) { raw( (ActualType)x ); }
    HiddenValue<ActualType,FakeType>& operator=(FakeType y) { raw( (ActualType)y ); return *this; }
    operator FakeType() const { return (FakeType) raw(); }

    template<class T2>
    explicit operator T2() const { return (T2) (FakeType) raw(); }
};


template<typename TT>
struct GuestWrapperBase
{
    HiddenValue<TT> hidden;
    
    using WrappedType = TT;
    using RawGuestType = TT;

    WrappedType get() const
    {
        return SwapTyped(hidden.raw());
    }

    void set(WrappedType x)
    {
        hidden.raw(SwapTyped(x));
    }

    RawGuestType raw() const
    {
        return hidden.raw();
    }

    void raw(RawGuestType x)
    {
        hidden.raw(x); 
    }

    void raw_and(RawGuestType x)
    {
        hidden.raw(hidden.raw() & x);
    }

    void raw_or(RawGuestType x)
    {
        hidden.raw(hidden.raw() | x);
    }
};

template<typename TT>
struct GuestWrapperBase<TT*>
{
private:
    HiddenValue<uint32_t, TT*> p;
public:
    using WrappedType = TT*;
    using RawGuestType = uint32_t;
    
    WrappedType get() const
    {
        uint32 rawp = this->raw();
        if(rawp)
            return (TT*) (swap32(rawp) + ROMlib_offset);
        else
            return nullptr;
    }

    void set(TT* ptr)
    {
        if(ptr)
            this->raw( swap32( (uint32_t) ((uintptr_t)ptr - ROMlib_offset) ) );
        else
            this->raw( 0 );
    }

    RawGuestType raw() const
    {
        return p.raw();
    }

    void raw(RawGuestType x)
    {
        p.raw(x);
    }

    
};


template<typename TT>
struct GuestWrapper : GuestWrapperBase<TT>
{
    GuestWrapper() = default;
    GuestWrapper(const GuestWrapper<TT>& y) = default;

    GuestWrapper<TT>& operator=(const GuestWrapper<TT>& y) = default;

    // Map implicit operations to *raw* access.
    // This should go away, and once we're sure it's gone,
    // we can wrap it to proper converted access.
    GuestWrapper(TT x) { this->raw((typename GuestWrapper<TT>::RawGuestType)x); }
    GuestWrapper<TT>& operator=(TT y) { this->raw((typename GuestWrapper<TT>::RawGuestType)y); return *this; }
    operator TT() const { return (TT)this->raw(); }

    explicit operator bool()
    {
        return this->raw() != 0;
    }
};

template<typename TT>
bool operator==(GuestWrapper<TT> a, GuestWrapper<TT> b)
{
        return a.raw() == b.raw();
}

template<typename TT>
bool operator!=(GuestWrapper<TT> a, GuestWrapper<TT> b)
{
        return a.raw() != b.raw();
}

#define GUEST_STRUCT struct is_guest_struct {}

struct Point {
    INTEGER v;
    INTEGER h;
};

template<>
struct GuestWrapper<Point> { GUEST_STRUCT;
    GuestWrapper< INTEGER> v;
    GuestWrapper< INTEGER> h;

    using WrappedType = Point;
    using RawGuestType = Point;

    Point get() const
    {
        return Point { v.get(), h.get() };
    }

    void set(Point x)
    {
        v.set(x.v);
        h.set(x.h);
    }

    Point raw() const
    {
        return Point { v.raw(), h.raw() };
    }

    void raw(Point x)
    {
        v.raw(x.v);
        h.raw(x.h);
    }

};

template <typename TT, typename SFINAE = void>
struct GuestType
{
    using type = GuestWrapper<TT>;
    /* typename std::conditional<
            std::is_base_of<GuestStruct, TT>::value,
            TT,
            GuestWrapper<TT>
        >::type;*/
};


template <typename TT>
struct GuestType<TT, std::void_t<typename TT::is_guest_struct> >
{
    using type = TT;
};

// forward declare.
// uses template specialization to bypass the above,
// so a GUEST_STRUCT on the actual declaration is redundant (but still fine)
#define FORWARD_GUEST_STRUCT(CLS)   \
    struct CLS;\
    template<>  \
    struct GuestType<CLS>    \
    {   \
        using type = CLS;    \
    }
    

template<typename TT>
using GUEST = typename GuestType<TT>::type;

template<>
struct GuestType<char>
{
    using type = char;
};

template<>
struct GuestType<signed char>
{
    using type = signed char;
};

template<>
struct GuestType<unsigned char>
{
    using type = unsigned char;
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

template<typename TT>
struct GuestType<TT[0]>
{
    using type = GUEST<TT>[0];
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
    //return GUEST<TO>((TO)(FROM)p);
    GUEST<TO> result;
    result.raw( p.raw() );
    return result;
}

//#define MAKE_HIDDEN(typ) struct  HIDDEN_ ## typ { GUEST_STRUCT; using HiddenType = typ; typ p; }
#define MAKE_HIDDEN(typ) using  HIDDEN_ ## typ = GUEST<typ>
// Roadmap:
// 1. switch to
// [done] #define MAKE_HIDDEN(typ) using  HIDDEN_ ## typ = GUEST<typ>
//  (adapt STARH, other uses of ->P)
// 2. remove

#  define PACKED_MEMBER(typ, name) typ name
// Roadmap:
// 1. remove

}

#endif /* _MACTYPE_H_ */
