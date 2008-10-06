
#if !defined (_MMAN_PUBLIC_H_)
#define _MMAN_PUBLIC_H_

#include "MemoryMgr.h"

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: mman.h 63 2004-12-24 18:19:43Z ctm $
 */

/* common case handle state accessor; lock the handle and return the
   pre-locked handle state */
extern SignedByte hlock_return_orig_state (Handle h);
extern Size zone_size (THz zone);

extern void BlockMove_the_trap (Ptr src, Ptr dst, Size cnt, boolean_t flush_p);

/* Helper function that prints a useful error message when out of memory. */
extern void print_mem_full_message (void);

/* We'll allow the user to specify zones up to this size. */
#define MAX_ZONE_SIZE (2047L * 1024 * 1024)

/* Always malloc Mac memory for now; someday we may return to
 * support mmap'd memory.
 */
#define MALLOC_MAC_MEMORY

extern int ROMlib_applzone_size, ROMlib_syszone_size, ROMlib_stack_size;

#if !defined (NDEBUG) && defined (HAVE_MMAP)
#define MM_MANY_APPLZONES
#endif /* !NDEBUG && HAVE_MMAP */

#if defined (MM_MANY_APPLZONES)
extern int mm_n_applzones;
#endif

extern Handle _NewHandle_copy_ptr_flags (Size size, const void *data_to_copy,
					 boolean_t sys_p);
extern Handle _NewHandle_copy_handle_flags (Size size, Handle data_to_copy,
					    boolean_t sys_p);
extern Ptr _NewPtr_copy_ptr_flags (Size size, const void *data_to_copy,
				   boolean_t sys_p);
extern Ptr _NewPtr_copy_handle_flags (Size size, Handle data_to_copy,
				      boolean_t sys_p);

#define NewHandle_copy_ptr(s, p)      _NewHandle_copy_ptr_flags(s, p, FALSE)
#define NewHandleSys_copy_ptr(s, p)   _NewHandle_copy_ptr_flags(s, p, TRUE)
#define NewHandle_copy_handle(s, h)   _NewHandle_copy_handle_flags(s, h, FALSE)
#define NewHandleSys_copy_handle(s, h) _NewHandle_copy_handle_flags(s, h, TRUE)
#define NewPtr_copy_ptr(s, p)	    _NewPtr_copy_ptr_flags (s, p, FALSE)
#define NewPtrSys_copy_ptr(s, p)    _NewPtr_copy_ptr_flags (s, p, TRUE)
#define NewPtr_copy_handle(s, h)    _NewPtr_copy_handle_flags (s, h, FALSE)
#define NewPtrSys_copy_handle(s, h) _NewPtr_copy_handle_flags (s, h, TRUE)

/* spewy flags */
extern boolean_t ROMlib_memnomove_p;

#define LOCKBIT		(1 << 7)
#define PURGEBIT	(1 << 6)
#define RSRCBIT		(1 << 5)

#define SYSBIT	(1 << 10)
#define CLRBIT	(1 << 9)

/*
 * The to_look_for stuff below is a guess, but I couldn't get ThinkC to
 * compile my test case on Brute.  The problem is that when running
 * "Droit Romain" (the compiled Hypercard stack from Belgium), we see
 * a patched out NewHandle that actually stomps d1, which was causing us
 * to allocate memory from the SysZone instead of the ApplZone.  I don't
 * know why this doesn't happen on a real Mac, although they were putting
 * the value of the original NewHandle address in d1, so perhaps Macs get
 * lucky, or perhaps their SysZones grow nicer than ours.  Other possibilities
 * include them doing a check like we're doing below or that they use some
 * sort of hidden state to decide whether or not to put stuff into SysZone.
 */

enum { TRAP_MASK = 0xF9FF };

#define SYS_P(trapno, to_look_for) \
((((trapno) & TRAP_MASK) == ((to_look_for) & TRAP_MASK)) \
 && (((trapno) & SYSBIT) != 0))

#define CLEAR_P(trapno, to_look_for) \
((((trapno) & TRAP_MASK) == ((to_look_for) & TRAP_MASK)) \
 && (((trapno) & CLRBIT) != 0))

#define ZONE_SAVE_EXCURSION(zone, body)		\
  do {						\
      THz save_zone;				\
						\
      save_zone = TheZone;			\
      TheZone = zone;				\
      { body }					\
      TheZone = save_zone;			\
  } while (FALSE)

/* These macros assign values to fields of a structure referred to
 * by a handle.  They perform no byte swapping.  There is no need to
 * lock the handle before invoking these macros.  For example,
 * HASSIGN_2 (gdhandle,
 *	      gdRefNum, CWC (9),
 *	      gdID,     CWC (14));
 */
#define HASSIGN_1(h, f1, v1)			\
({						\
  typeof (v1) _v1 = (v1);			\
  typeof (STARH (h)) _hp = STARH (h);		\
  _hp->f1 = _v1;				\
})

#define HASSIGN_2(h, f1, v1, f2, v2)		\
({						\
  typeof (v1) _v1 = (v1);			\
  typeof (v2) _v2 = (v2);			\
  typeof (STARH (h)) _hp = STARH (h);		\
  _hp->f1 = _v1;				\
  _hp->f2 = _v2;				\
})

#define HASSIGN_3(h, f1, v1, f2, v2, f3, v3)	\
({						\
  typeof (v1) _v1 = (v1);			\
  typeof (v2) _v2 = (v2);			\
  typeof (v3) _v3 = (v3);			\
  typeof (STARH (h)) _hp = STARH (h);		\
  _hp->f1 = _v1;				\
  _hp->f2 = _v2;				\
  _hp->f3 = _v3;				\
})

#define HASSIGN_4(h, f1, v1, f2, v2, f3, v3, f4, v4)	\
({							\
  typeof (v1) _v1 = (v1);				\
  typeof (v2) _v2 = (v2);				\
  typeof (v3) _v3 = (v3);				\
  typeof (v4) _v4 = (v4);				\
  typeof (STARH (h)) _hp = STARH (h);			\
  _hp->f1 = _v1;					\
  _hp->f2 = _v2;					\
  _hp->f3 = _v3;					\
  _hp->f4 = _v4;					\
})

#define HASSIGN_5(h, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5)	\
({								\
  typeof (v1) _v1 = (v1);					\
  typeof (v2) _v2 = (v2);					\
  typeof (v3) _v3 = (v3);					\
  typeof (v4) _v4 = (v4);					\
  typeof (v5) _v5 = (v5);					\
  typeof (STARH (h)) _hp = STARH (h);				\
  _hp->f1 = _v1;						\
  _hp->f2 = _v2;						\
  _hp->f3 = _v3;						\
  _hp->f4 = _v4;						\
  _hp->f5 = _v5;						\
})

#define HASSIGN_6(h, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6)	\
({									\
  typeof (v1) _v1 = (v1);						\
  typeof (v2) _v2 = (v2);						\
  typeof (v3) _v3 = (v3);						\
  typeof (v4) _v4 = (v4);						\
  typeof (v5) _v5 = (v5);						\
  typeof (v6) _v6 = (v6);						\
  typeof (STARH (h)) _hp = STARH (h);					\
  _hp->f1 = _v1;							\
  _hp->f2 = _v2;							\
  _hp->f3 = _v3;							\
  _hp->f4 = _v4;							\
  _hp->f5 = _v5;							\
  _hp->f6 = _v6;							\
})

#define HASSIGN_7(h, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5,	\
		  f6, v6, f7, v7)				\
({								\
  typeof (v1) _v1 = (v1);					\
  typeof (v2) _v2 = (v2);					\
  typeof (v3) _v3 = (v3);					\
  typeof (v4) _v4 = (v4);					\
  typeof (v5) _v5 = (v5);					\
  typeof (v6) _v6 = (v6);					\
  typeof (v7) _v7 = (v7);					\
  typeof (STARH (h)) _hp = STARH (h);				\
  _hp->f1 = _v1;						\
  _hp->f2 = _v2;						\
  _hp->f3 = _v3;						\
  _hp->f4 = _v4;						\
  _hp->f5 = _v5;						\
  _hp->f6 = _v6;						\
  _hp->f7 = _v7;						\
})

#define HASSIGN_10(h, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5,	\
		   f6, v6, f7, v7, f8, v8, f9, v9, f10, v10)	\
({								\
  typeof (v1) _v1 = (v1);					\
  typeof (v2) _v2 = (v2);					\
  typeof (v3) _v3 = (v3);					\
  typeof (v4) _v4 = (v4);					\
  typeof (v5) _v5 = (v5);					\
  typeof (v6) _v6 = (v6);					\
  typeof (v7) _v7 = (v7);					\
  typeof (v8) _v8 = (v8);					\
  typeof (v9) _v9 = (v9);					\
  typeof (v10) _v10 = (v10);					\
  typeof (STARH (h)) _hp = STARH (h);				\
  _hp->f1 = _v1;						\
  _hp->f2 = _v2;						\
  _hp->f3 = _v3;						\
  _hp->f4 = _v4;						\
  _hp->f5 = _v5;						\
  _hp->f6 = _v6;						\
  _hp->f7 = _v7;						\
  _hp->f8 = _v8;						\
  _hp->f9 = _v9;						\
  _hp->f10 = _v10;						\
})

#define HASSIGN_11(h, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5,	\
		   f6, v6, f7, v7, f8, v8, f9, v9, f10, v10,	\
		   f11, v11)					\
({								\
  typeof (v1) _v1 = (v1);					\
  typeof (v2) _v2 = (v2);					\
  typeof (v3) _v3 = (v3);					\
  typeof (v4) _v4 = (v4);					\
  typeof (v5) _v5 = (v5);					\
  typeof (v6) _v6 = (v6);					\
  typeof (v7) _v7 = (v7);					\
  typeof (v8) _v8 = (v8);					\
  typeof (v9) _v9 = (v9);					\
  typeof (v10) _v10 = (v10);					\
  typeof (v11) _v11 = (v11);					\
  typeof (STARH (h)) _hp = STARH (h);				\
  _hp->f1 = _v1;						\
  _hp->f2 = _v2;						\
  _hp->f3 = _v3;						\
  _hp->f4 = _v4;						\
  _hp->f5 = _v5;						\
  _hp->f6 = _v6;						\
  _hp->f7 = _v7;						\
  _hp->f8 = _v8;						\
  _hp->f9 = _v9;						\
  _hp->f10 = _v10;						\
  _hp->f11 = _v11;						\
})

#define HASSIGN_12(h, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5,	\
		   f6, v6, f7, v7, f8, v8, f9, v9, f10, v10,	\
		   f11, v11, f12, v12)				\
({								\
  typeof (v1) _v1 = (v1);					\
  typeof (v2) _v2 = (v2);					\
  typeof (v3) _v3 = (v3);					\
  typeof (v4) _v4 = (v4);					\
  typeof (v5) _v5 = (v5);					\
  typeof (v6) _v6 = (v6);					\
  typeof (v7) _v7 = (v7);					\
  typeof (v8) _v8 = (v8);					\
  typeof (v9) _v9 = (v9);					\
  typeof (v10) _v10 = (v10);					\
  typeof (v11) _v11 = (v11);					\
  typeof (v12) _v12 = (v12);					\
  typeof (STARH (h)) _hp = STARH (h);				\
  _hp->f1 = _v1;						\
  _hp->f2 = _v2;						\
  _hp->f3 = _v3;						\
  _hp->f4 = _v4;						\
  _hp->f5 = _v5;						\
  _hp->f6 = _v6;						\
  _hp->f7 = _v7;						\
  _hp->f8 = _v8;						\
  _hp->f9 = _v9;						\
  _hp->f10 = _v10;						\
  _hp->f11 = _v11;						\
  _hp->f12 = _v12;						\
})

#define HASSIGN_13(h, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5,	\
		   f6, v6, f7, v7, f8, v8, f9, v9, f10, v10,	\
		   f11, v11, f12, v12, f13, v13)		\
({								\
  typeof (v1) _v1 = (v1);					\
  typeof (v2) _v2 = (v2);					\
  typeof (v3) _v3 = (v3);					\
  typeof (v4) _v4 = (v4);					\
  typeof (v5) _v5 = (v5);					\
  typeof (v6) _v6 = (v6);					\
  typeof (v7) _v7 = (v7);					\
  typeof (v8) _v8 = (v8);					\
  typeof (v9) _v9 = (v9);					\
  typeof (v10) _v10 = (v10);					\
  typeof (v11) _v11 = (v11);					\
  typeof (v12) _v12 = (v12);					\
  typeof (v13) _v13 = (v13);					\
  typeof (STARH (h)) _hp = STARH (h);				\
  _hp->f1 = _v1;						\
  _hp->f2 = _v2;						\
  _hp->f3 = _v3;						\
  _hp->f4 = _v4;						\
  _hp->f5 = _v5;						\
  _hp->f6 = _v6;						\
  _hp->f7 = _v7;						\
  _hp->f8 = _v8;						\
  _hp->f9 = _v9;						\
  _hp->f10 = _v10;						\
  _hp->f11 = _v11;						\
  _hp->f12 = _v12;						\
  _hp->f13 = _v13;						\
})

#define HASSIGN_14(h, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5,	\
		   f6, v6, f7, v7, f8, v8, f9, v9, f10, v10,	\
		   f11, v11, f12, v12, f13, v13, f14, v14)	\
({								\
  typeof (v1) _v1 = (v1);					\
  typeof (v2) _v2 = (v2);					\
  typeof (v3) _v3 = (v3);					\
  typeof (v4) _v4 = (v4);					\
  typeof (v5) _v5 = (v5);					\
  typeof (v6) _v6 = (v6);					\
  typeof (v7) _v7 = (v7);					\
  typeof (v8) _v8 = (v8);					\
  typeof (v9) _v9 = (v9);					\
  typeof (v10) _v10 = (v10);					\
  typeof (v11) _v11 = (v11);					\
  typeof (v12) _v12 = (v12);					\
  typeof (v13) _v13 = (v13);					\
  typeof (v14) _v14 = (v14);					\
  typeof (STARH (h)) _hp = STARH (h);				\
  _hp->f1 = _v1;						\
  _hp->f2 = _v2;						\
  _hp->f3 = _v3;						\
  _hp->f4 = _v4;						\
  _hp->f5 = _v5;						\
  _hp->f6 = _v6;						\
  _hp->f7 = _v7;						\
  _hp->f8 = _v8;						\
  _hp->f9 = _v9;						\
  _hp->f10 = _v10;						\
  _hp->f11 = _v11;						\
  _hp->f12 = _v12;						\
  _hp->f13 = _v13;						\
  _hp->f14 = _v14;						\
})

#define HASSIGN_15(h, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5,	\
		   f6, v6, f7, v7, f8, v8, f9, v9, f10, v10,	\
		   f11, v11, f12, v12, f13, v13, f14, v14,	\
		   f15, v15)					\
({								\
  typeof (v1) _v1 = (v1);					\
  typeof (v2) _v2 = (v2);					\
  typeof (v3) _v3 = (v3);					\
  typeof (v4) _v4 = (v4);					\
  typeof (v5) _v5 = (v5);					\
  typeof (v6) _v6 = (v6);					\
  typeof (v7) _v7 = (v7);					\
  typeof (v8) _v8 = (v8);					\
  typeof (v9) _v9 = (v9);					\
  typeof (v10) _v10 = (v10);					\
  typeof (v11) _v11 = (v11);					\
  typeof (v12) _v12 = (v12);					\
  typeof (v13) _v13 = (v13);					\
  typeof (v14) _v14 = (v14);					\
  typeof (v15) _v15 = (v15);					\
  typeof (STARH (h)) _hp = STARH (h);				\
  _hp->f1 = _v1;						\
  _hp->f2 = _v2;						\
  _hp->f3 = _v3;						\
  _hp->f4 = _v4;						\
  _hp->f5 = _v5;						\
  _hp->f6 = _v6;						\
  _hp->f7 = _v7;						\
  _hp->f8 = _v8;						\
  _hp->f9 = _v9;						\
  _hp->f10 = _v10;						\
  _hp->f11 = _v11;						\
  _hp->f12 = _v12;						\
  _hp->f13 = _v13;						\
  _hp->f14 = _v14;						\
  _hp->f15 = _v15;						\
})

#define LOCK_HANDLE_EXCURSION_1(handle1_expr, body)	\
  do {							\
    Handle handle1;					\
    SignedByte handle1_state;				\
    							\
    handle1 = (Handle) (handle1_expr);			\
    handle1_state = hlock_return_orig_state (handle1);	\
    							\
    { body }						\
    							\
    HSetState (handle1, handle1_state);			\
  } while (FALSE)

#define LOCK_HANDLE_EXCURSION_2(handle1_expr,		\
				handle2_expr, body)	\
  do {							\
    Handle handle1;					\
    Handle handle2;					\
    SignedByte handle1_state;				\
    SignedByte handle2_state;				\
    							\
    handle1 = (Handle) (handle1_expr);			\
    handle2 = (Handle) (handle2_expr);			\
    handle1_state = hlock_return_orig_state (handle1);	\
    handle2_state = hlock_return_orig_state (handle2);	\
    							\
    { body }						\
    							\
    HSetState (handle2, handle2_state);			\
    HSetState (handle1, handle1_state);			\
  } while (FALSE)

#define LOCK_HANDLE_EXCURSION_3(handle1_expr,		\
				handle2_expr,		\
				handle3_expr, body)	\
							\
  do {							\
    Handle handle1;					\
    Handle handle2;					\
    Handle handle3;					\
    SignedByte handle1_state;				\
    SignedByte handle2_state;				\
    SignedByte handle3_state;				\
    							\
    handle1 = (Handle) (handle1_expr);			\
    handle2 = (Handle) (handle2_expr);			\
    handle3 = (Handle) (handle3_expr);			\
    handle1_state = hlock_return_orig_state (handle1);	\
    handle2_state = hlock_return_orig_state (handle2);	\
    handle3_state = hlock_return_orig_state (handle3);	\
    							\
    { body }						\
    							\
    HSetState (handle3, handle3_state);			\
    HSetState (handle2, handle2_state);			\
    HSetState (handle1, handle1_state);			\
  } while (FALSE)

#define LOCK_HANDLE_EXCURSION_4(handle1_expr,		\
				handle2_expr,		\
				handle3_expr,		\
				handle4_expr, body)	\
  do {							\
    Handle handle1;					\
    Handle handle2;					\
    Handle handle3;					\
    Handle handle4;					\
    SignedByte handle1_state;				\
    SignedByte handle2_state;				\
    SignedByte handle3_state;				\
    SignedByte handle4_state;				\
    							\
    handle1 = (Handle) (handle1_expr);			\
    handle2 = (Handle) (handle2_expr);			\
    handle3 = (Handle) (handle3_expr);			\
    handle4 = (Handle) (handle4_expr);			\
    handle1_state = hlock_return_orig_state (handle1);	\
    handle2_state = hlock_return_orig_state (handle2);	\
    handle3_state = hlock_return_orig_state (handle3);	\
    handle4_state = hlock_return_orig_state (handle4);	\
    							\
    { body }						\
    							\
    HSetState (handle4, handle4_state);			\
    HSetState (handle3, handle3_state);			\
    HSetState (handle2, handle2_state);			\
    HSetState (handle1, handle1_state);			\
  } while (FALSE)

#endif /* !_MMAN_PUBLIC_H_ */
