
#if !defined (_MMAN_PRIVATE_H_)
#define _MMAN_PRIVATE_H_


/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: mman_private.h 74 2004-12-30 03:38:55Z ctm $
 */

#include "rsys/mman.h"
namespace Executor {
    /* the bogo new IM books implies (via a picture) that the field
     order is `location, size, flags'; and it says that the 24bit
     header is `location, size'.  but IMII implies that the 24bit
     header order is `size, location'; testing shows this is the
     correct order */
    /* or, as mat would like me to say: `testing shows that good things
     are better than bad things' */


    /* various flags */





    /* data contained in the block */
typedef struct block_header { GUEST_STRUCT;
#if defined (MM_BLOCK_HEADER_SENTINEL)
    GUEST< uint8[SENTINEL_SIZE]> pre_sentinel;
#endif
    GUEST< uint8> flags;
    GUEST< uint8> master_ptr_flags;
    GUEST< uint8> reserved;
    GUEST< uint8> size_correction;
    GUEST< uint32> size;
    GUEST< uint32> location_u;    /* sometimes it's a pointer (the zone),
					   sometimes it's an offset */
#if defined (MM_RECORD_ALLOCATION_STACK_TRACES)
    GUEST< int> alloc_debug_number;
    GUEST< void*[MM_MAX_STACK_TRACE_DEPTH]> alloc_pcs;
#endif
#if defined (MM_BLOCK_HEADER_SENTINEL)
    GUEST< uint8[SENTINEL_SIZE]> post_sentinel;
#endif
    GUEST< uint32[0]> data;
} block_header_t;

#define BLOCK_LOCATION_OFFSET_X(block)	((block)->location_u)
#define BLOCK_LOCATION_ZONE_X(block)	((block)->location_u)
#define BLOCK_DATA_X(block)	(RM ((Ptr) (block)->data))

#define BLOCK_LOCATION_OFFSET(block)	(CL (BLOCK_LOCATION_OFFSET_X (block)))
#define BLOCK_LOCATION_ZONE(block)	(MR (BLOCK_LOCATION_ZONE_X (block)))
#define BLOCK_DATA(block)	((Ptr) (block)->data)

#define USE(block)		(((block)->flags >> 6) & 0x3)
#define PSIZE(block)		(CL ((block)->size))
#define SIZEC(block)		((block)->size_correction)

#define SETUSE(block, use)	((block)->flags = (  ((block)->flags & 0x3F) \
						   | (use) << 6))
#define SETPSIZE(block, _size)	((block)->size = CLV (_size))
#define SETSIZEC(block, sizec)	((block)->size_correction = (sizec))

/* ### fixme; we also need to set the other reserved bits (in the
   flags and master_ptr_flags fields) to zero */
#define SETZERO(block)		((block)->reserved = 0)

#define HEADER_SIZE		(sizeof (block_header_t))

/* ### bogo compat */
#define HDRSIZE HEADER_SIZE

extern unsigned long ROMlib_syszone;
extern unsigned long ROMlib_memtop;

#define VALID_ADDRESS(p) ((unsigned long)(p) >= ROMlib_syszone && \
			  (unsigned long)(p) <  ROMlib_memtop)

/* handle to block pointer */
#define HANDLE_TO_BLOCK(handle)					\
  (VALID_ADDRESS(handle) && VALID_ADDRESS(MR((handle)->p))	\
   ? (block_header_t *) ((char *) STARH (handle)		\
		         - HDRSIZE)				\
   : NULL)

#define BLOCK_TO_HANDLE(zone, block)			\
  ((Handle) ((char *) (zone) + BLOCK_LOCATION_OFFSET (block)))

#define BLOCK_TO_POINTER(block)				\
  ((Ptr) ((char *) (block) + HDRSIZE))

#define POINTER_TO_BLOCK(pointer)			\
  ((block_header_t *) ((char *) (pointer) - HDRSIZE))

/* extract the handle state bits */
#define HANDLE_STATE(handle, block)			\
  ((block)->master_ptr_flags)

#define FREE_BLOCK_STATE	(0x1A)
#define STATE_BITS		(LOCKBIT | PURGEBIT | RSRCBIT)
#define EMPTY_STATE		(0)
#define MIN_BLOCK_SIZE		(12)

#define BLOCK_STATE(block)				\
  ((block)->master_ptr_flags)

#define SET_HANDLE_STATE(handle, block, state)		\
  ((block)->master_ptr_flags = (state))

#define SET_BLOCK_STATE(block, state)			\
  ((block)->master_ptr_flags = (state))

/* set the master pointer of a handle to a given value */
#define SETMASTER(handle, ptr)	((handle)->p = (Ptr)RM (ptr))

#define BLOCK_NEXT(block)			\
  ((block_header_t *) ((char *) (block) + PSIZE (block)))

#define LSIZE(block)				\
  (PSIZE (block) - SIZEC (block) - HDRSIZE)

/* ### bogo compat */
#define BLOCK_SET_STATE(block, state)	SET_BLOCK_STATE (block, state)
#define BLOCK_SET_USE(block, use)	SETUSE (block, use)
#define BLOCK_SET_SIZEC(block, sizec)	SETSIZEC (block, sizec)
#define BLOCK_SET_PSIZE(block, psize)	SETPSIZE (block, psize)
#define BLOCK_SET_LOCATION_OFFSET(block, loc)	\
  ((block)->location_u = CL (loc))
#define BLOCK_SET_LOCATION_ZONE(block, loc)	\
  ((block)->location_u = (uint32_t) RM (loc))
#define BLOCK_SET_RESERVED(block)

/* Zone record accessor macros */
#define ZONE_HEAP_DATA(zone)	((block_header_t *) &(zone)->heapData)

#define ZONE_BK_LIM_X(zone)	(guest_cast<block_header_t*> ((zone)->bkLim))
#define ZONE_PURGE_PTR_X(zone)	((zone)->purgePtr)
#define ZONE_HFST_FREE_X(zone)	((zone)->hFstFree)
#define ZONE_ZCB_FREE_X(zone)	((zone)->zcbFree)
#define ZONE_GZ_PROC_X(zone)	((zone)->gzProc)
#define ZONE_MORE_MAST_X(zone)	((zone)->moreMast)
#define ZONE_PURGE_PROC_X(zone)	((zone)->purgeProc)
#define ZONE_ALLOC_PTR_X(zone)	((zone)->allocPtr)

#define ZONE_BK_LIM(zone)	(MR (ZONE_BK_LIM_X (zone)))
#define ZONE_PURGE_PTR(zone)	(MR (ZONE_PURGE_PTR_X (zone)))
#define ZONE_HFST_FREE(zone)	(MR (ZONE_HFST_FREE_X (zone)))
#define ZONE_ZCB_FREE(zone)	(CL (ZONE_ZCB_FREE_X (zone)))
#define ZONE_GZ_PROC(zone)	(MR (ZONE_GZ_PROC_X (zone)))
#define ZONE_MORE_MAST(zone)	(CW (ZONE_MORE_MAST_X (zone)))
#define ZONE_PURGE_PROC(zone)	(MR (ZONE_PURGE_PROC_X (zone)))
#define ZONE_ALLOC_PTR(zone)	((block_header_t *) MR (ZONE_ALLOC_PTR_X (zone)))

#define MEM_DEBUG_P()	ERROR_ENABLED_P (ERROR_TRAP_FAILURE)

extern OSErr ROMlib_relalloc (Size, block_header_t **);
extern void ROMlib_setupblock (block_header_t *, uint32, short, Handle, ...);
extern void ROMlib_freeblock (block_header_t *);
extern boolean_t ROMlib_makespace (block_header_t **, uint32);
extern boolean_t ROMlib_locked (block_header_t *);
extern void ROMlib_moveblock (block_header_t *, block_header_t *, uint32);
extern int32 ROMlib_amtfree (block_header_t *);
extern boolean_t ROMlib_pushblock (block_header_t *, block_header_t *);
extern void ROMlib_coalesce (block_header_t *blk);

void mm_set_block_fields_offset (block_header_t *block,
			  unsigned state, unsigned use,
			  unsigned size_correction,
			  uint32 physical_size, uint32 location);

void mm_set_block_fields_zone (block_header_t *block,
			  unsigned state, unsigned use,
			  unsigned size_correction,
			  uint32 physical_size, THz location);

extern void checkallocptr (void);

#if ERROR_SUPPORTED_P (ERROR_MEMORY_MANAGER_SLAM)

typedef struct
{
  int32 n_rel;
  int32 n_nrel;
  int32 n_free;
  int32 largest_free;
  int32 total_free;
}
zone_info_t;


typedef Zone *ZonePtr;

struct pblock_t { GUEST_STRUCT;
    GUEST< ZonePtr> sp;
    GUEST< Ptr> lp;
    GUEST< INTEGER> mm;
    GUEST< ProcPtr> gz;
};

extern void ROMlib_sledgehammer_zone (THz zone, boolean_t print_p,
				      const char *fn, const char *file,
				      int lineno, const char *where,
				      zone_info_t *infop);
extern void ROMlib_sledgehammer_zones (const char *fn,
				       const char *file, int lineno,
				       const char *where,
				       zone_info_t *info_array);

#define MM_SLAM(where)						\
  do {								\
    if (ERROR_ENABLED_P (ERROR_MEMORY_MANAGER_SLAM))		\
      ROMlib_sledgehammer_zones (__PRETTY_FUNCTION__,		\
				 __FILE__, __LINE__,		\
				 where, NULL);			\
  } while (FALSE)

#define MM_SLAM_ZONE(zone, where)				\
  do {								\
    if (ERROR_ENABLED_P (ERROR_MEMORY_MANAGER_SLAM))		\
      ROMlib_sledgehammer_zone (zone, FALSE,			\
                                __PRETTY_FUNCTION__,		\
                                __FILE__, __LINE__,		\
                                where, NULL);				\
  } while (FALSE)

#else /* No ERROR_MEMORY_MANAGER_SLAM */

#define MM_SLAM(where)
#define MM_SLAM_ZONE(zone, where)

#endif /* No ERROR_MEMORY_MANAGER_SLAM */

#if ERROR_SUPPORTED_P (ERROR_TRAP_FAILURE)
#define GEN_MEM_ERR(err)					\
  do {								\
    if ((err) != noErr)						\
      warning_trap_failure ("returning err %ld", (long) (err));	\
  } while (FALSE)
#else
#define GEN_MEM_ERR(err)
#endif

/* block types, or `use' */
#define FREE 0
#define NREL 1
#define REL 2

extern void mman_heap_death (const char *func, const char *where);

/* Preprocessor sludge to get __LINE__ as a string for HEAP_DEATH macro. */
#define __HEAP_DEATH(func, file, l) mman_heap_death (func, file #l)
#define _HEAP_DEATH(func, file, l) __HEAP_DEATH (func, file, l)
#define HEAP_DEATH() \
  _HEAP_DEATH (__PRETTY_FUNCTION__, " in " __FILE__ ":", __LINE__)

#define HEAPEND	(MR(HeapEnd) + MIN_BLOCK_SIZE)	/* temporary ctm hack */
}
#endif /* _MMAN_PRIVATE_H */
