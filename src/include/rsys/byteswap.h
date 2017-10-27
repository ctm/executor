#if !defined (_BYTESWAP_H_)
#define _BYTESWAP_H_

#if !defined (BIGENDIAN) && !defined (LITTLEENDIAN)
# error "One of BIGENDIAN or LITTLEENDIAN must be #defined"
#endif

#include <stdint.h>
#include <cstddef>
#include <type_traits>
#include "rsys/mactype.h"
#include <syn68k_public.h> /* for ROMlib_offset */


namespace Executor
{

#if defined (BIGENDIAN)

#define CW(rhs)		(rhs)
#define CL(rhs)		(rhs)
#define CWC(rhs)	(rhs)
#define CLC(rhs)	(rhs)
#define CWV(rhs)	(rhs)
#define CLV(rhs)	(rhs)
#define Cx(rhs)		(rhs)

/*
 * Do not use these blindly on big-endian machines.  There's no longer a
 * guarantee that we can get low-memory on all big-endian machines.  It doesn't
 * work, for example, on Mac OS X PPC.
 *
 * #define MR(rhs)		(rhs) / * Mac to ROMlib * /
 * #define RM(rhs)		(rhs) / * ROMlib to Mac * /
 */

#define MR(n)  ((typeof (n))({ typeof (n) _t = n; _t ? ((((unsigned int) (_t))) + ROMlib_offset) : 0;}))
#define RM(n)  ((typeof (n))({ typeof (n) _t = n; _t ? ((((unsigned int) (_t)- ROMlib_offset)) ) : 0;}))

#define CLC_NULL NULL

#else /* !defined (BIGENDIAN) */

#define CW_RAW(n)  ((typeof (n)) swap16 ((unsigned short)(n)))
#define CL_RAW(n)  ((typeof (n)) swap32 ((unsigned int) ((n)|0)))


template<class TT> inline GUEST<int16_t> CW(TT x) { return GUEST<int16_t>::fromRaw(swap16((uint16_t)x)); }
inline GUEST<uint16_t> CW(uint16_t x) { return GUEST<uint16_t>::fromRaw(swap16(x)); }
inline GUEST<int16_t> CW(int16_t x) { return GUEST<int16_t>::fromRaw(swap16((uint16_t)x)); }

inline GUEST<uint16_t> CW(unsigned int x) { return GUEST<uint16_t>::fromRaw(swap16((uint16_t)x)); }

template<class TT, typename = typename std::enable_if<sizeof(TT) == 2,void>::type>
TT CW(GuestWrapper<TT> x) { return x.get(); }


template<class TT> inline GUEST<int32_t> CL(TT x) { return GUEST<int32_t>::fromRaw(swap32((uint32_t)x)); }
inline GUEST<uint32_t> CL(uint32_t x) { return GUEST<uint32_t>::fromRaw(swap32(x)); }
inline GUEST<int32_t> CL(int32_t x) { return GUEST<int32_t>::fromRaw(swap32((uint32_t)x)); }

template<class TT, typename = typename std::enable_if<sizeof(TT) == 4,void>::type>
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
TT Cx(TT x);    // no definition. Make sure we get a linker error if an unexpected version of Cx is used.


inline unsigned char Cx_RAW(unsigned char x) { return x; }
inline signed char Cx_RAW(signed char x) { return x; }
inline char Cx_RAW(char x) { return x; }

inline uint16_t Cx_RAW(uint16_t x) { return CW_RAW(x); }
inline int16_t Cx_RAW(int16_t x) { return CW_RAW(x); }

inline uint32_t Cx_RAW(uint32_t x) { return CL_RAW(x); }
inline int32_t Cx_RAW(int32_t x) { return CL_RAW(x); }

template<class TT>
TT Cx_RAW(TT x);    // no definition. Make sure we get a linker error if an unexpected version of Cx is used.


#if 0
#define MR(n)  ((typeof (n))(n ? ((swap32 ((unsigned long) (n))) + ROMlib_offset) : 0))
#define RM(n)  ((typeof (n))(n ? ((swap32 ((unsigned long) (n)- ROMlib_offset)) ) : 0))
#elif 1

template<typename TT>
GUEST<TT*> RM(TT* p)
{
    return GUEST<TT*>::fromHost(p);
}


template<typename TT>
TT* MR(GuestWrapper<TT*> p)
{
    return p.get();
}

template<typename TT>
TT* MR(HiddenValue<uint32_t, TT*> p)
{
    return MR((TT*) p);
}

//template<typename TT>
//TT* MR(TT* p); // unavailable

inline std::nullptr_t RM(std::nullptr_t p) { return nullptr; }

//inline void* MR(uint32_t p) { return MR((void*)p); }
//inline void* MR(int32_t p) { return MR((void*)p); }


#  define PPR(n) MR(n)

#define PTR_MINUSONE ((void*)-1)

#else

#if (SIZEOF_CHAR_P == 4) && !FORCE_EXPERIMENTAL_PACKED_MACROS

#  define MR(n)  ((typeof (n))({ typeof (n) _t = n; _t ? ((swap32 ((unsigned int) (_t))) + ROMlib_offset) : 0;}))

#else

/* YY needs a better name, right now it's just a helper for MR */
#  define YY(n) (swap32((uint32)(n)) + ROMlib_offset)

#  define MR(n)  ((typeof (n))({ typeof (n) _t = n; _t ? (YY(_t)) : 0;}))
#endif


#define RM(n)  ((typeof (n))({ typeof (n) _t = n; _t ? ((swap32 ((unsigned long) (_t)- ROMlib_offset)) ) : 0;}))

/* Packed Pointer to ROMlib  */
#if (SIZEOF_CHAR_P == 4) && !FORCE_EXPERIMENTAL_PACKED_MACROS
#  define PPR(n) MR(n)
#else
#  define PPR(n) ((typeof (n.type[0]))({ typeof (n) _t = n; _t.pp ? (YY(_t.pp)) : 0;}))
#endif

#endif

namespace internal
{
        template<typename T0, typename TT, typename T2 = std::conditional_t<std::is_signed<T0>::value,int16_t,uint16_t>>
        inline GUEST<T2>
        wordFromRaw(TT x) { return GUEST<T2>::fromRaw(x); }
        template<typename T0, typename TT, typename T2 = std::conditional_t<std::is_signed<T0>::value,int32_t,uint32_t>>
        inline GUEST<T2>
        longwordFromRaw(TT x) { return GUEST<T2>::fromRaw(x); }
}

#define CWC(n) (internal::wordFromRaw<decltype(n)>( \
                                                (signed short) (((((unsigned short)n) << 8) & 0xFF00) \
					     | ((((unsigned short)n) >> 8) & 0x00FF))))
#define CLC(n) (internal::longwordFromRaw<decltype(n)>( \
                                         (int) (  (((unsigned int) ((n)|0) & 0x000000FF) << 24)   \
				     | (((unsigned int) (n) & 0x0000FF00) <<  8)   \
				     | (((unsigned int) (n) & 0x00FF0000) >>  8)   \
				     | (((unsigned int) (n) & 0xFF000000) \
					>> 24))))


#define CWC_RAW(n) ((decltype(n))( \
                                                (signed short) (((((unsigned short)n) << 8) & 0xFF00) \
					     | ((((unsigned short)n) >> 8) & 0x00FF))))
#define CLC_RAW(n) ((decltype(n))( \
                                         (int) (  (((unsigned int) ((n)|0) & 0x000000FF) << 24)   \
				     | (((unsigned int) (n) & 0x0000FF00) <<  8)   \
				     | (((unsigned int) (n) & 0x00FF0000) >>  8)   \
				     | (((unsigned int) (n) & 0xFF000000) \
					>> 24))))

#define CLC_NULL nullptr

/* These are better versions of CW and CL, but should not be nested
 * because of exponential growth in the preprocessed code size.
 */
#define CWV(n) \
((__builtin_constant_p ((n)) ? (typeof (CW(n)))  CWC (n) : CW (n)))
#define CLV(n) \
((__builtin_constant_p ((n)) ? (typeof (CL(n))) CLC (n) : CL (n)))

#if 0
/* This will cause a link error for an invalid Cx. */
extern int bad_cx_splosion;

#define Cx(rhs) (sizeof(rhs) == 1 ?					\
		    (rhs)						\
		:							\
		    sizeof(rhs) == 2 ?					\
			 CW (rhs)					\
		    :							\
		        (sizeof(rhs) == 4 || sizeof(rhs) == 8) ?	\
			    CL ((rhs)|0)				\
			:						\
			    (typeof (rhs)) bad_cx_splosion)
#endif

#endif  /* !defined (BIGENDIAN) */

#define CB(rhs)		(rhs)
#define CBC(rhs)	(rhs)
#define CBV(rhs)	(rhs)

#if 1
//#  define STARH(h)		MR ((h)->p)
#  define STARH(h)		MR (*h)
#  define HxP(handle, field)	MR (STARH(handle)->field)
#  define HxX(handle, field)	(STARH(handle)->field)
#  define HxZ(handle, field) HxX(handle, field)
#else
#  define STARH(h)		((typeof ((h)->type[0])) (YY ((h)->pp)))
#  define HxP(handle, field)	MR ((STARH(handle)->field).pp)
#  define HxX(handle, field)	((STARH(handle))->field)

// HxZ is a handle dereference where the member selected is itself some form
// of packed pointer, but we're only checking to see if it's zero or non-zero
// (e.g. if (HxZ(hand)) )

#  define HxZ(handle, field)	((STARH(handle))->field.pp)
#endif

#define Hx(handle, field)	Cx (STARH(handle)->field)

#if defined (BIGENDIAN)

#define SWAP_POINT(pt) (pt)

#define SWAPPED_OPW(big_endian_lvalue, op, v)				  \
 ((void) ((big_endian_lvalue) op##= (v)))
#define SWAPPED_OPL(big_endian_lvalue, op, v)				  \
 ((void) ((big_endian_lvalue) op##= (v)))
#define CMPW_P(big_endian_v, op, native_endian_v)			\
 ((big_endian_v) op (native_endian_v))
#define CMPL_P(big_endian_v, op, native_endian_v)			\
 ((big_endian_v) op (native_endian_v))

#else /* !BIGENDIAN */

#define SWAP_POINT(pt)				\
({						\
   Point __pt_ = (pt);				\
						\
   __pt_.h = CW (__pt_.h);			\
   __pt_.v = CW (__pt_.v);			\
						\
   __pt_;					\
})

#if 0
#define SWAPPED_OPW(big_endian_lvalue, op, v)				  \
  ((void)								  \
   (((#op[0] == '&' || #op[0] == '|' || #op[0] == '^') && #op[1] == '\0') \
    ? ((big_endian_lvalue) op##= CWV (v))				  \
    : ((big_endian_lvalue) = CW (CW (big_endian_lvalue) op (v)))))
#define SWAPPED_OPL(big_endian_lvalue, op, v)				  \
  ((void)								  \
   (((#op[0] == '&' || #op[0] == '|' || #op[0] == '^') && #op[1] == '\0') \
    ? ((big_endian_lvalue) op##= CLV (v))				  \
    : ((big_endian_lvalue) = CL (CL (big_endian_lvalue) op (v)))))
#else

#define SWAPPED_OPW(bevalue, op, v) (bevalue).set((bevalue).get() op (v))
#define SWAPPED_OPL(bevalue, op, v) (bevalue).set((bevalue).get() op (v))

#endif
/* This compares a big endian 16 bit value to a native endian value,
 * using the specified operator.  The operator must be one of
 * ==, !=, <, >, <=, >=.
 */
#define CMPW_P(big_endian_v, op, native_endian_v)		\
((__builtin_constant_p ((long) (native_endian_v))		\
  && (native_endian_v) == 0)					\
 ? ((#op[0] == '>' && #op[1] == '=')				\
    ? !((big_endian_v) & CWC (0x8000))				\
    : ((#op[0] == '<' && #op[1] == '\0')			\
       ? (((big_endian_v) & CWC (0x8000)) != 0)			\
       : (((#op[0] == '=' || #op[0] == '!') && #op[1] == '=')	\
	  ? ((big_endian_v) op CWC (0))				\
	  : CWV (big_endian_v) op 0)))				\
 : (((#op[0] == '=' || #op[0] == '!') && #op[1] == '=')		\
    ? ((big_endian_v) op CWV (native_endian_v))			\
    : CWV (big_endian_v) op (native_endian_v)))

/* This compares a big endian 32 bit value to a native endian value,
 * using the specified operator.  The operator must be one of
 * ==, !=, <, >, <=, >=.
 */
#define CMPL_P(big_endian_v, op, native_endian_v)		\
((__builtin_constant_p ((long) (native_endian_v))		\
  && (native_endian_v) == 0)					\
 ? ((#op[0] == '>' && #op[1] == '=')				\
    ? !((big_endian_v) & CLC (0x80000000))			\
    : ((#op[0] == '<' && #op[1] == '\0')			\
       ? (((big_endian_v) & CLC (0x80000000)) != 0)		\
       : (((#op[0] == '=' || #op[0] == '!') && #op[1] == '=')	\
	  ? ((big_endian_v) op CLC (0))				\
	  : CLV (big_endian_v) op 0)))				\
 : (((#op[0] == '=' || #op[0] == '!') && #op[1] == '=')		\
    ? ((big_endian_v) op CLV (native_endian_v))			\
    : CLV (big_endian_v) op (native_endian_v)))

#endif /* !BIGENDIAN */



template<typename TT>
TT ptr_from_longint(int32_t l)
{
        // FIXME: needless back-and-forth endian conversion
    return MR( guest_cast<TT>( CL(l) ) );
}

template<typename TT>
int32_t ptr_to_longint(TT p)
{
        // FIXME: needless back-and-forth endian conversion
    return CL( guest_cast<int32_t>( RM(p) ) );
}

}


#endif /* !_BYTESWAP_H_ */
