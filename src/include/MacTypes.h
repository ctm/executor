#if !defined (_MACTYPES_H_)
#define _MACTYPES_H_

/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: MacTypes.h 86 2005-05-25 00:47:12Z ctm $
 */

#include "rsys/mactype.h"

#ifdef __cplusplus
namespace Executor {
#endif
	
#define a5	(EM_A5)
	
#if !defined(__MACTYPES__) || defined(__cplusplus)
typedef int8 SignedByte;
#endif
typedef uint8 Byte;
#if !defined(__MACTYPES__) || defined(__cplusplus)
typedef int8 *Ptr;
#endif

#if (SIZEOF_CHAR_P == 4) && !FORCE_EXPERIMENTAL_PACKED_MACROS
#define MAKE_HIDDEN(typ) typedef struct { typ p; } HIDDEN_ ## typ
#else
#define MAKE_HIDDEN(typ) \
  typedef union PACKED { uint32 pp; typ type[0];} HIDDEN_ ## typ
#endif
MAKE_HIDDEN(Ptr);

#if !defined(__MACTYPES__) || defined(__cplusplus)
typedef HIDDEN_Ptr *Handle;
#endif

MAKE_HIDDEN(Handle);
typedef BOOLEAN Boolean;

typedef Byte Str15[16];
typedef Byte Str31[32];
typedef Byte Str32[33];
typedef Byte Str63[64];
typedef Byte Str255[256];
#if !defined(__MACTYPES__) || defined(__cplusplus)
typedef Byte *StringPtr;
#endif
	
MAKE_HIDDEN(StringPtr);

typedef HIDDEN_StringPtr *StringHandle;
#if !defined(__MACTYPES__) || defined(__cplusplus)

typedef int (*ProcPtr)();
#endif
MAKE_HIDDEN(ProcPtr);
#if !defined(__MACTYPES__) || defined(__cplusplus)
typedef LONGINT Fixed, Fract;
#endif
/* SmallFract represnts values between 0 and 65535 */
typedef unsigned short SmallFract;

#define MaxSmallFract 0xFFFF

typedef double Extended;

typedef LONGINT Size;

typedef INTEGER OSErr;
typedef ULONGINT OSType;
typedef	ULONGINT ResType;

typedef	LONGINT	OSErrRET;	/* for smashing d0 just like the Mac */
typedef	LONGINT	INTEGERRET;
typedef	LONGINT	BOOLEANRET;
typedef	LONGINT	SignedByteRET;


#if (SIZEOF_CHAR_P == 4) && !FORCE_EXPERIMENTAL_PACKED_MACROS
#  define PACKED_MEMBER(typ, name) typ name
#else
#  define PACKED_MEMBER(typ, name) \
                                union PACKED { uint32 pp; typ type[0]; } name
#endif

typedef struct PACKED {
  INTEGER qFlags;
  PACKED_MEMBER(union __qe *, qHead);	/* actually QElemPtr */
  PACKED_MEMBER(union __qe *, qTail);	/* actually QElemPtr */
} QHdr;
typedef QHdr *QHdrPtr;
typedef union __qe *QElemPtr;

MAKE_HIDDEN(QElemPtr);

	enum {
		noErr = 0
	};

/* from Quickdraw.h */
typedef struct PACKED Point
{
  INTEGER v;
  INTEGER h;
} Point;

#define NULL_POINTP ((Point *) NULL)

#define ZEROPOINT(p) (p.v = CWC (0), p.h = CWC (0))

typedef struct PACKED Rect
{
  INTEGER top;
  INTEGER left;
  INTEGER bottom;
  INTEGER right;
} Rect;

typedef Rect *RectPtr;

#define RECT_WIDTH(r)				\
({						\
  const Rect *__r = (r);			\
  CW (__r->right) - CW (__r->left);		\
})
#define RECT_HEIGHT(r)				\
({						\
  const Rect *__r = (r);			\
  CW (__r->bottom) - CW (__r->top);		\
})

#define NULL_RECTP	((Rect *) NULL)

#define RECT_ZERO(r)				\
do						\
  memset (r, 0, sizeof (Rect));			\
while (FALSE)

#define RECT_EQUAL_P(r1, r2)			\
({						\
  const uint32 *__p1 = (const uint32 *) (r1);	\
  const uint32 *__p2 = (const uint32 *) (r2);	\
  __p1[0] == __p2[0] && __p1[1] == __p2[1];	\
})

/* from IntlUtil.h */
typedef INTEGER ScriptCode;
typedef INTEGER LangCode;

/* DO NOT DELETE THIS LINE */
extern INTEGER 	ROM85;
extern INTEGER 	DSErrCode;
#ifdef __cplusplus
}
#endif
#endif /* _MACTYPES_H_ */
