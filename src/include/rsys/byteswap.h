#if !defined(_BYTESWAP_H_)
#define _BYTESWAP_H_

#if !defined(BIGENDIAN) && !defined(LITTLEENDIAN)
#error "One of BIGENDIAN or LITTLEENDIAN must be #defined"
#endif

#include <stdint.h>
#include <cstddef>
#include <type_traits>
#include "rsys/mactype.h"
#include <syn68k_public.h> /* for ROMlib_offset */

namespace Executor
{

#if defined(BIGENDIAN)

#define CW_RAW(rhs) (rhs)
#define CL_RAW(rhs) (rhs)

#else /* !defined (BIGENDIAN) */

template<class TT>
inline TT CW_RAW(TT n)
{
    return swap16((uint16_t)n);
}
template<class TT>
inline TT CL_RAW(TT n)
{
    return swap32((uint32_t)n);
}

#endif

template<class TT>
inline GUEST<int16_t> CW(TT x)
{
    return GUEST<int16_t>::fromHost(x);
}
inline GUEST<uint16_t> CW(uint16_t x) { return GUEST<uint16_t>::fromHost(x); }
inline GUEST<int16_t> CW(int16_t x) { return GUEST<int16_t>::fromHost(x); }

inline GUEST<uint16_t> CW(unsigned int x) { return GUEST<uint16_t>::fromHost(x); }

template<class TT, typename = typename std::enable_if<sizeof(TT) == 2, void>::type>
TT CW(GuestWrapper<TT> x) { return x.get(); }

template<class TT>
inline GUEST<int32_t> CL(TT x) { return GUEST<int32_t>::fromHost(x); }
inline GUEST<uint32_t> CL(uint32_t x) { return GUEST<uint32_t>::fromHost(x); }
inline GUEST<int32_t> CL(int32_t x) { return GUEST<int32_t>::fromHost(x); }

template<class TT, typename = typename std::enable_if<sizeof(TT) == 4, void>::type>
TT CL(GuestWrapper<TT> x) { return x.get(); }

inline unsigned char Cx(unsigned char x) { return x; }
inline signed char Cx(signed char x) { return x; }
inline char Cx(char x) { return x; }

inline GUEST<uint16_t> Cx(uint16_t x) { return CW(x); }
inline GUEST<int16_t> Cx(int16_t x) { return CW(x); }

inline GUEST<uint32_t> Cx(uint32_t x) { return CL(x); }
inline GUEST<int32_t> Cx(int32_t x) { return CL(x); }

template<class TT>
TT Cx(GuestWrapper<TT> x) { return x.get(); }

template<class TT>
TT Cx(TT x); // no definition. Make sure we get a linker error if an unexpected version of Cx is used.

inline unsigned char Cx_RAW(unsigned char x) { return x; }
inline signed char Cx_RAW(signed char x) { return x; }
inline char Cx_RAW(char x) { return x; }

inline uint16_t Cx_RAW(uint16_t x) { return CW_RAW(x); }
inline int16_t Cx_RAW(int16_t x) { return CW_RAW(x); }

inline uint32_t Cx_RAW(uint32_t x) { return CL_RAW(x); }
inline int32_t Cx_RAW(int32_t x) { return CL_RAW(x); }

template<class TT>
TT Cx_RAW(TT x); // no definition. Make sure we get a linker error if an unexpected version of Cx is used.

template<typename TT>
GUEST<TT *> RM(TT *p)
{
    return GUEST<TT *>::fromHost(p);
}

template<typename TT>
TT *MR(GuestWrapper<TT *> p)
{
    return p.get();
}

inline std::nullptr_t RM(std::nullptr_t p) { return nullptr; }

#define PPR(n) MR(n)

#define PTR_MINUSONE ((void *)-1)

#if defined(BIGENDIAN)

#define CW_RAW(rhs) (rhs)
#define CL_RAW(rhs) (rhs)
#define CWC_RAW(rhs) (rhs)
#define CLC_RAW(rhs) (rhs)

#define CWC(rhs) CW(rhs)
#define CLC(rhs) CL(rhs)
#define CWV(rhs) CW(rhs)
#define CLV(rhs) CL(rhs)

#else /* !defined (BIGENDIAN) */

namespace internal
{
    template<typename T0, typename TT, typename T2 = typename std::conditional<std::is_signed<T0>::value, int16_t, uint16_t>::type>
    inline GUEST<T2>
    wordFromRaw(TT x) { return GUEST<T2>::fromRaw(x); }
    template<typename T0, typename TT, typename T2 = typename std::conditional<std::is_signed<T0>::value, int32_t, uint32_t>::type>
    inline GUEST<T2>
    longwordFromRaw(TT x) { return GUEST<T2>::fromRaw(x); }
}

#define CWC(n) (internal::wordFromRaw<decltype(n)>(      \
    (signed short)(((((unsigned short)n) << 8) & 0xFF00) \
                   | ((((unsigned short)n) >> 8) & 0x00FF))))
#define CLC(n) (internal::longwordFromRaw<decltype(n)>(  \
    (int)((((unsigned int)((n) | 0) & 0x000000FF) << 24) \
          | (((unsigned int)(n)&0x0000FF00) << 8)        \
          | (((unsigned int)(n)&0x00FF0000) >> 8)        \
          | (((unsigned int)(n)&0xFF000000)              \
             >> 24))))

#define CWC_RAW(n) ((decltype(n))(                       \
    (signed short)(((((unsigned short)n) << 8) & 0xFF00) \
                   | ((((unsigned short)n) >> 8) & 0x00FF))))
#define CLC_RAW(n) ((decltype(n))(                       \
    (int)((((unsigned int)((n) | 0) & 0x000000FF) << 24) \
          | (((unsigned int)(n)&0x0000FF00) << 8)        \
          | (((unsigned int)(n)&0x00FF0000) >> 8)        \
          | (((unsigned int)(n)&0xFF000000)              \
             >> 24))))

/* These are better versions of CW and CL, but should not be nested
 * because of exponential growth in the preprocessed code size.
 */
#define CWV(n) \
    ((__builtin_constant_p((n)) ? (decltype(CW(n)))CWC(n) : CW(n)))
#define CLV(n) \
    ((__builtin_constant_p((n)) ? (decltype(CL(n)))CLC(n) : CL(n)))

#endif /* !defined (BIGENDIAN) */

#define CLC_NULL nullptr

#define CB(rhs) (rhs)
#define CBC(rhs) (rhs)
#define CBV(rhs) (rhs)

#define STARH(h) MR(*h)
#define HxP(handle, field) MR(STARH(handle)->field)
#define HxX(handle, field) (STARH(handle)->field)
#define HxZ(handle, field) HxX(handle, field)
// HxZ is a handle dereference where the member selected is itself some form
// of packed pointer, but we're only checking to see if it's zero or non-zero
// (e.g. if (HxZ(hand)) )

#define Hx(handle, field) Cx(STARH(handle)->field)

#define SWAPPED_OPW(bevalue, op, v) (bevalue).set((bevalue).get() op(v))
#define SWAPPED_OPL(bevalue, op, v) (bevalue).set((bevalue).get() op(v))

template<typename TT>
TT ptr_from_longint(int32_t l)
{
    // FIXME: needless back-and-forth endian conversion
    return MR(guest_cast<TT>(CL(l)));
}

template<typename TT>
int32_t ptr_to_longint(TT p)
{
    // FIXME: needless back-and-forth endian conversion
    return CL(guest_cast<int32_t>(RM(p)));
}
}

#endif /* !_BYTESWAP_H_ */
