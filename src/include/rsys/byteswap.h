#if !defined (_BYTESWAP_H_)
#define _BYTESWAP_H_

#if !defined (BIGENDIAN) && !defined (LITTLEENDIAN)
# error "One of BIGENDIAN or LITTLEENDIAN must be #defined"
#endif

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

#define CW(n)  ((typeof (n)) swap16 ((unsigned short)(n)))
#define CL(n)  ((typeof (n)) swap32 ((unsigned int) ((n)|0)))

#if 0
#define MR(n)  ((typeof (n))(n ? ((swap32 ((unsigned long) (n))) + ROMlib_offset) : 0))
#define RM(n)  ((typeof (n))(n ? ((swap32 ((unsigned long) (n)- ROMlib_offset)) ) : 0))
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

#define CWC(n) ((typeof (n)) (signed short) ((((n) << 8) & 0xFF00) \
					     | (((n) >> 8) & 0x00FF)))
#define CLC(n) ((typeof (n)) (int) (  (((int) ((n)|0) & 0x000000FF) << 24)   \
				     | (((int) (n) & 0x0000FF00) <<  8)   \
				     | (((int) (n) & 0x00FF0000) >>  8)   \
				     | (((unsigned int) (n) & 0xFF000000) \
					>> 24)))

#define CLC_NULL NULL

/* These are better versions of CW and CL, but should not be nested
 * because of exponential growth in the preprocessed code size.
 */
#define CWV(n) \
((typeof (n)) (__builtin_constant_p ((long) (n)) ? CWC (n) : CW (n)))
#define CLV(n) \
((typeof (n)) (__builtin_constant_p ((long) (n)) ? CLC (n) : CL (n)))

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


#endif  /* !defined (BIGENDIAN) */

#define CB(rhs)		(rhs)
#define CBC(rhs)	(rhs)
#define CBV(rhs)	(rhs)

#if (SIZEOF_CHAR_P == 4) && !FORCE_EXPERIMENTAL_PACKED_MACROS
#  define STARH(h)		MR ((h)->p)
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

#endif /* !_BYTESWAP_H_ */
