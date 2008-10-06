/* Copyright 1990, 1991, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_mmansubr[] =
  "$Id: mmansubr.c 88 2005-05-25 03:59:37Z ctm $";
#endif

/* Miscellaneous subroutines for memory management */

#include "rsys/common.h"

#include "MemoryMgr.h"

/* ### this probably shouldn't be here */
#include "ResourceMgr.h"

#include "rsys/mman_private.h"
#include "rsys/assert.h"
#include "rsys/fatal.h"
#include "rsys/hook.h"
#include "rsys/system_error.h"
#include "rsys/vdriver.h"

#include <stdarg.h>

/* Attempts to notify the user of a catastrophic heap failure then exits. */
void
mman_heap_death (const char *func, const char *where)
{
  static boolean_t already_dead_p = FALSE;
  char err_msg[256];

  sprintf (err_msg,
	   "Executor has encountered a fatal heap error and must exit. "
	   "(%s%s, most recent trap = %04X)",
	   func, where, (unsigned) mostrecenttrap);

  if (!already_dead_p)  /* Avoid recursion. */
    {
      already_dead_p = TRUE;

      warning_unexpected ("%s", err_msg);

      /* Operating on the theory that the application zone is most likely
       * to get smashed, switch to the SysZone and bring up a window
       * explaining the death.  If the SysZone is (also?) smashed, we
       * may crash or try to recurse here.  But at least we got the
       * warning_unexpected out.
       */
      TheZone = SysZone;
      system_error (err_msg, 0, "Exit", NULL, NULL, NULL, NULL, NULL);
    }

  /* Don't bother calling ExitToShell; things are too smashed. */
  vdriver_shutdown ();
  puts (err_msg);
  exit (-2);
}

#if ERROR_SUPPORTED_P (ERROR_MEMORY_MANAGER_SLAM)
static void
mm_slam_hook (void)
{
}

#define mm_fatal(fmt, args...)						\
  do {									\
    if (fn && file && where)						\
      {									\
	fprintf (stderr, "            `%s' in %s:%d;\n"			\
		         "called from `%s' in %s:%d on %s;\n"		\
	                 "fatal error: " fmt "\n",			\
		 __PRETTY_FUNCTION__, __FILE__, __LINE__,		\
		 fn, file, lineno, where , ## args);			\
									\
      }									\
    else								\
      {									\
	fprintf (stderr, "`%s' in %s:%d;\n"				\
	                 "fatal error: " fmt "\n",			\
		 __PRETTY_FUNCTION__, __FILE__, __LINE__ , ## args);	\
									\
      }									\
    mm_slam_hook ();							\
    /* d'oh! */								\
    *(volatile uint32 *) -1;						\
  } while (FALSE)
#endif


#if ERROR_SUPPORTED_P (ERROR_MEMORY_MANAGER_SLAM)

void
dump_zone (THz zone)
{
  ROMlib_sledgehammer_zone (zone, TRUE, NULL, NULL, -1, NULL, NULL);
}

#endif /* ERROR_SUPPORTED_P (ERROR_MEMORY_MANAGER_SLAM) */


#if ERROR_SUPPORTED_P (ERROR_MEMORY_MANAGER_SLAM)

block_header_t *
addr_block (THz zone, char *addr)
{
  block_header_t *t;

  for (t = ZONE_HEAP_DATA (zone);
       t != ZONE_BK_LIM (zone);
       t = BLOCK_NEXT (t))
    {
      if (PSIZE (t) < MIN_BLOCK_SIZE)
	HEAP_DEATH ();
      if ((char *) t <= addr
	  && addr <= (char *) t + PSIZE (t))
	return t;
    }
  return NULL;
}

boolean_t
addr_in_zone_p (THz zone, char *addr)
{
  return ((char *) ZONE_HEAP_DATA (zone) <= addr
	  && addr <= (char *) ZONE_BK_LIM (zone));
}

void
addr_info (char *addr)
{
  THz zones[3], addr_zone;
  Handle handle;
  block_header_t *block, *ptr_block;
  boolean_t addr_is_handle_p;
  char *ptr, *ptr_data_start;
  int ptr_data_size;
  int i;
  
  /* get the zone for `addr' */
  zones[0] = MR (ApplZone);
  zones[1] = MR (SysZone);
  zones[2] = MR (TheZone);
  
  for (i = 0, addr_zone = NULL; i < (int) NELEM (zones); i ++)
    {
      if (addr_in_zone_p (zones[i], addr))
	{
	  addr_zone = zones[i];
	  break;
	}
    }
  if (addr_zone == NULL)
    {
      fprintf (stderr, "cannot determine zone for addr `%p'\n",
	       addr);
      return;
    }
  
  fprintf (stderr, "addr `%p' is located in zone %s: `%p', `%p:%p'\n",
	   addr, ((addr_zone == zones[0])
		  ? "ApplicZone"
		  : ((addr_zone == zones[1])
		     ? "SystemZone"
		     : "TheZone")), addr_zone,
	   ZONE_HEAP_DATA (addr_zone), ZONE_BK_LIM (addr_zone));
  
  block = addr_block (addr_zone, addr);
  if (block == NULL)
    {
      fprintf (stderr, "cannot determine block for addr `%p'\n",
	       addr);
      return;
    }
  
  if (USE (block) == NREL)
    {
      /* `addr' might be a handle, and `block' might be a block of
         master pointers for this handle */
      handle = (Handle) addr;
      ptr = (char *) STARH (handle);
      
      ptr_block = addr_block (addr_zone, ptr);
      if (ptr_block != NULL
	  && USE (ptr_block) == REL
	  && (char *) BLOCK_DATA (ptr_block) == ptr
	  && BLOCK_TO_HANDLE (addr_zone, ptr_block) == handle)
	{
	  addr_is_handle_p = TRUE;
	  
	  goto handle_addr_info;
	}
      else
	{
	  ptr_data_start = (char *) BLOCK_DATA (block);
	  ptr_data_size = LSIZE (block);
	  
	  fprintf (stderr,
		   "addr `%p' points `%d' bytes into NREL block `%p'\n"
		   "which has data start `%p', lsize `%d' and end `%p'\n"
		   "`%d' bytes remain after addr in block\n",
		   addr, (int) (addr - ptr_data_start), block,
		   ptr_data_start, ptr_data_size,
		   ptr_data_size + ptr_data_start,
		   (int) (ptr_data_size - (addr - ptr_data_start)));
	  return;
	}
    }
  else if (USE (block) == REL)
    {
      /* relocatable block */
      handle = BLOCK_TO_HANDLE (addr_zone, block);
      addr_is_handle_p = FALSE;
      
      goto handle_addr_info;
    }
  else if (USE (block) == FREE)
    {
      fprintf (stderr,
	       "addr `%p' points `%d' bytes into FREE block `%p'\n"
	       "which has a physical size of `%d'\n",
	       addr, (int) (addr - (char *) block), block,
	       PSIZE (block));
      return;
    }
  
  fprintf (stderr, "danger will robinson, shouldn't get here");
  return;
  
 handle_addr_info:
  {
    block_header_t *handle_block;
    SignedByte state;
    
    /* print out handle address information.  `addr_is_handle_p' is
       true if the address is the actual handle, false if the address
       points into the relocatable block.
       
       to call this code `handle', and `addr_is_handle_p' must be set
       up */
    
    ptr = (char *) STARH (handle);
    ptr_block = HANDLE_TO_BLOCK (handle);
    
    handle_block = addr_block (addr_zone, (char *) handle);
    if (handle_block == NULL)
      {
	fprintf (stderr,
		 "cannot determine master pointer block for handle `%p'\n",
		 handle);
	return;
      }
    
    state = BLOCK_STATE (ptr_block);
    
    ptr_data_start = (char *) BLOCK_DATA (ptr_block);
    ptr_data_size = LSIZE (ptr_block);
    
    if (addr_is_handle_p)
      fprintf (stderr,
	       "addr is a handle `%p' allocated from master pointer block `%p'\n"
	       "pointing to REL block `%p'\n"
	       "which has data start `%p', lsize `%d' and end `%p'\n"
	       "with state %c%c%c\n",
	       handle, handle_block,
	       ptr_block,
	       ptr_data_start, ptr_data_size, ptr_data_start + ptr_data_size,
	       state & LOCKBIT  ? 'L' : ' ',
	       state & PURGEBIT ? 'P' : ' ',
	       state & RSRCBIT  ? 'R' : ' ');
    else
      fprintf (stderr,
	       "addr `%p' points `%ld' bytes into REL block `%p'\n"
	       "as handle `%p' allocated from master pointer block `%p'\n"
	       "which has a data start `%p', lsize `%d' and end `%p'\n"
	       "with state %c%c%c\n"
	       "`%d' bytes remain after addr in block\n",
	       addr, (long) (addr - ptr), ptr_block,
	       handle, handle_block,
	       ptr_data_start, ptr_data_size, ptr_data_start + ptr_data_size,
	       state & LOCKBIT  ? 'L' : ' ',
	       state & PURGEBIT ? 'P' : ' ',
	       state & RSRCBIT  ? 'R' : ' ',
	       (int) (ptr_data_size - (addr - ptr)));
    
    if (state & RSRCBIT)
      {
	INTEGER id;
	ResType type;
	char res_name[257];
	
	GetResInfo (handle, &id, &type, (StringPtr) res_name);
	
	if (ResErr)
	  return;
	
	/* blah */
	id = CW (id);
	type = MR (type);
	
	res_name[res_name[0] + 1] = '\0';
	
	fprintf (stderr,
		 "res handle `%p' has type `%c%c%c%c', id `%d', name `%s'\n",
		 handle,
		 (type >> 24) & 0xFF,
		 (type >> 16) & 0xFF,
		 (type >>  8) & 0xFF,
		 (type >>  0) & 0xFF,
		 id, &res_name[1]);
      }
    return;
  }

}

void
ROMlib_sledgehammer_zone (THz zone, boolean_t print_p,
			  const char *fn, const char *file, int lineno,
			  const char *where, zone_info_t *infop)
{
  block_header_t *block, *zone_start, *zone_end, *alloc_ptr;
  boolean_t found_alloc_ptr_p = FALSE;
  int total_size;
  
  if (infop)
    memset (infop, 0, sizeof *infop);

  if (zone == NULL)
    return;
  
  zone_start = ZONE_HEAP_DATA (zone);
  zone_end = ZONE_BK_LIM (zone);
  
  alloc_ptr = ZONE_ALLOC_PTR (zone);
  if (alloc_ptr
      && (alloc_ptr < zone_start
	  || alloc_ptr > zone_end))
    mm_fatal ("alloc pointer `%p' out of bounds of zone %p; `%p:%p'",
	       alloc_ptr, zone, zone_start, zone_end);
  
  total_size = 0;
  for (block = zone_start;
       block != zone_end;
       block = BLOCK_NEXT (block))
    {
      if (block < zone_start
	  || block > zone_end)
	mm_fatal ("block `%p' outside of zone %p; `%p:%p'",
		  block,
		  zone, zone_start, zone_end);
      
      if (PSIZE (block) < MIN_BLOCK_SIZE)
	mm_fatal ("bad physical block size `%d' in block `%p'",
		  (int) PSIZE (block), block);
      
      if (SIZEC (block) > MIN_BLOCK_SIZE)
	mm_fatal ("bad size correction `%d' in block `%p'",
		  (int) SIZEC (block), block);
  
      if ((int32) LSIZE (block) < 0)
	mm_fatal ("bad logical block size `%d' in block `%p'",
		  (int) LSIZE (block), block);
      
      switch (USE (block))
	{
	case REL:
	  {
	    Handle handle;
	    
	    if (infop)
	      ++infop->n_rel;
	    handle = BLOCK_TO_HANDLE (zone, block);
	    if (block != HANDLE_TO_BLOCK (handle))
	      mm_fatal ("REL block `%p'; bad master pointer for handle `%p'",
			block, handle);
	    
	    if ((BLOCK_STATE (block) & ~STATE_BITS) == FREE_BLOCK_STATE)
	      mm_fatal ("bogus unused block state bits `%d'\n",
			BLOCK_STATE (block));
	    
	    if (print_p)
	      {
		SignedByte state;

		state = HANDLE_STATE (handle, block);
		fprintf (stderr, "REL  %p; H:%p P:%p S:%08lx %c%c%c\n",
			 block,
			 handle, MR (handle->p),
			 (unsigned long) LSIZE (block),
			 state & LOCKBIT  ? 'L' : ' ',
			 state & PURGEBIT ? 'P' : ' ',
			 state & RSRCBIT  ? 'R' : ' ');
	      }
	    break;
	  }
	case NREL:
	  if (infop)
	    ++infop->n_nrel;
	  if ((uint32) BLOCK_LOCATION_ZONE (block) != (uint32) zone)
	    mm_fatal ("NREL block `%p' bad location; %p not zone %p",
		      block, (char *) BLOCK_LOCATION_ZONE (block), zone);

	  if ((BLOCK_STATE (block) & ~STATE_BITS) == FREE_BLOCK_STATE)
	    mm_fatal ("bogus unused block state bits `%d'\n",
		      BLOCK_STATE (block));
	  
	  if (print_p)
	    fprintf (stderr, "NREL %p;            P:%p S:%08lx\n",
		     block, BLOCK_TO_POINTER (block),
		     (unsigned long) LSIZE (block));
	  break;
	case FREE:
	  if (infop)
	    ++infop->n_free;
	  if (BLOCK_LOCATION_OFFSET (block))
	    mm_fatal ("free block `%p' has non-zero location `%p'",
		       block, (char *) BLOCK_LOCATION_OFFSET (block));
	  total_size += PSIZE (block);
	  if (infop)
	    {
	      infop->total_free += PSIZE (block);
	      if (PSIZE (block) > infop->largest_free)
		infop->largest_free = PSIZE (block);
	    }
	  
	  if (BLOCK_STATE (block) != FREE_BLOCK_STATE)
	    mm_fatal ("bogus free block state bits `%d'\n",
		      BLOCK_STATE (block));
	  
	  if (print_p)
	    fprintf (stderr, "FREE %p;                       S:%08x %c\n",
		     block,
		     PSIZE (block),
		     block == alloc_ptr ? 'A' : ' ');
	  break;
	default:
	  mm_fatal ("unknown use `%d' in block `%p'",
		    USE (block), block);
	}
      
      if (block == alloc_ptr)
	found_alloc_ptr_p = TRUE;
    }
  if (alloc_ptr
      && ! found_alloc_ptr_p)
    mm_fatal ("alloc pointer `%p' does not point to heap block",
	      alloc_ptr);
}

void
ROMlib_sledgehammer_zones (const char *fn, const char *file, int lineno,
			   const char *where, zone_info_t *info_array)
{
  ROMlib_sledgehammer_zone (MR (SysZone), FALSE,
			    fn, file, lineno, where,
			    info_array ? &info_array[0] : NULL);
  ROMlib_sledgehammer_zone (MR (ApplZone), FALSE,
			    fn, file, lineno, where,
			    info_array ? &info_array[1] : NULL);
  if (TheZone != SysZone
      && TheZone != ApplZone)
    ROMlib_sledgehammer_zone (MR (TheZone), FALSE,
			      fn, file, lineno, where,
			      info_array ? &info_array[2] : NULL);
}

#endif

PRIVATE void
mm_set_block_fields_common (block_header_t *block,
		     unsigned state, unsigned use,
		     unsigned size_correction,
		     uint32 physical_size)
{
  BLOCK_SET_STATE (block, state);
  BLOCK_SET_USE (block, use);
  BLOCK_SET_SIZEC (block, size_correction);
  BLOCK_SET_PSIZE (block, physical_size);

  BLOCK_SET_RESERVED (block);
}

void
mm_set_block_fields_offset (block_header_t *block,
		     unsigned state, unsigned use,
		     unsigned size_correction,
		     uint32 physical_size, uint32 location)
{
  mm_set_block_fields_common (block, state, use, size_correction,
			      physical_size);
  BLOCK_SET_LOCATION_OFFSET (block, location);
}

void
mm_set_block_fields_zone (block_header_t *block,
		     unsigned state, unsigned use,
		     unsigned size_correction,
		     uint32 physical_size, uint32 location)
{
  mm_set_block_fields_common (block, state, use, size_correction,
			      physical_size);
  BLOCK_SET_LOCATION_ZONE (block, location);
}

/* Given a block BLOCK big enough to hold a block of size SIZE, split
   it and set the fields up.  SIZE is the user size plus header.  USE
   goes into the use field of the header and masterp is used to
   calculate the offset of the master pointer in the location
   field. */
void
ROMlib_setupblock (block_header_t *block,
		   uint32 size, int16 use, Handle master_ptr, ...)
{
  int32 oldsize;
  /* size actually allocated (inc align byte and header) */
  int32 asize;
  block_header_t *newfree;
  int olduse;
  THz current_zone;
  uint32 physical_size;
  unsigned size_correction;
  
  current_zone = MR (TheZone);
  
  asize = size;
  if (asize < MIN_BLOCK_SIZE)
    asize = MIN_BLOCK_SIZE;
  else
    /* align mod 4 */
    asize = (asize + 3) & ~3;
  
  olduse  = USE (block);
  oldsize = PSIZE (block);
  
  if (oldsize - asize < MIN_BLOCK_SIZE)
    {
      /* too small to split.  Use it all. */
      size_correction = oldsize - size;
      physical_size = PSIZE (block);
      
      /* NOTE: newfree is a misnomer here. */
      newfree = (block_header_t *) ((char *) block + oldsize);
      if (newfree == ZONE_BK_LIM (current_zone))
	newfree = ZONE_HEAP_DATA (current_zone);
    }
  else
    {
      /* split the block. */
      newfree = (block_header_t *) ((char *) block + asize);
      
      mm_set_block_fields_offset (newfree,
			   FREE_BLOCK_STATE, FREE,
			   0, oldsize - asize, 0);
      
      size_correction = asize - size;
      physical_size = asize;
    }
  
  if ((char *) ZONE_ALLOC_PTR (current_zone) < (char *) block + oldsize)
    ZONE_ALLOC_PTR_X (current_zone) = (Ptr) RM (newfree);


#if 0
  mm_set_block_fields (block,
		       EMPTY_STATE, use,
		       size_correction, physical_size,
		       ((use == NREL)
			? (uint32) current_zone
			: ((use == REL)
			   ? (char *) master_ptr - (char *) current_zone
			   : (gui_fatal ("unknown use `%d'", use), -1))));
#else		       
  switch (use)
    {
    case NREL:
      mm_set_block_fields_zone (block, EMPTY_STATE, use,
				size_correction, physical_size,
				(uint32) current_zone);

      break;
    case REL:
      {
	va_list ap;
	unsigned int state;
	
	va_start (ap, master_ptr);
	state = va_arg (ap, unsigned int); 
	mm_set_block_fields_offset (block, state, use,
				    size_correction, physical_size,
				    (char *) master_ptr -
				    (char *) current_zone);
      }
      break;
    default:
      gui_fatal ("unknown use `%d'", use);
      break;
    }
#endif

  if (olduse == FREE)
    ZONE_ZCB_FREE_X (current_zone) = CL (ZONE_ZCB_FREE (current_zone)
					 - PSIZE (block));
  else
    ZONE_ZCB_FREE_X (current_zone) = CL (ZONE_ZCB_FREE (current_zone)
					 + oldsize - asize);
}

void 
ROMlib_coalesce (block_header_t *block)
{
  block_header_t *t_block;
  int32 total_free;
  THz current_zone;
  
  current_zone = MR (TheZone);
  
  total_free = 0;
  for (t_block = block;
       t_block != ZONE_BK_LIM (current_zone) && USE (t_block) == FREE;
       t_block = BLOCK_NEXT (t_block))
    {
      total_free += PSIZE (t_block);
      if (PSIZE (t_block) < MIN_BLOCK_SIZE)
	HEAP_DEATH ();
    }
  
  BLOCK_SET_PSIZE (block, total_free);
  
  if (ZONE_ALLOC_PTR (current_zone) >= block
      && (char *) ZONE_ALLOC_PTR (current_zone) <= (char *) block + total_free)
    ZONE_ALLOC_PTR_X (current_zone) = (Ptr) RM (block);
}

/* Mark a block free.  If relocatable, the master must have already
   been updated appropriately */
void 
ROMlib_freeblock (block_header_t *block)
{
  THz current_zone;
  
  current_zone = MR (TheZone);
  ZONE_ZCB_FREE_X (current_zone) = CL (ZONE_ZCB_FREE (current_zone)
				       + PSIZE (block));
  
  mm_set_block_fields_offset (block, FREE_BLOCK_STATE, FREE, 0, PSIZE (block),
			      0);
  
  ROMlib_coalesce (block);
}

/* Move the relocatable block at OLDL to the empty block NEWL, growing
   its size to NEWSIZE */
void
ROMlib_moveblock (block_header_t *oldl, block_header_t *newl,
		  uint32 newsize)
{
  Handle master;
  THz current_zone;
  
  current_zone = MR (TheZone);
  
  master = BLOCK_TO_HANDLE (current_zone, oldl);
  
  if (USE (oldl) != REL || ROMlib_locked (oldl))
    gui_abort ();
  
  ROMlib_setupblock (newl, newsize, REL, master, BLOCK_STATE(oldl));
  BlockMove (BLOCK_DATA (oldl), BLOCK_DATA (newl), LSIZE (oldl));
  SETMASTER (master, BLOCK_DATA (newl));
  ROMlib_freeblock (oldl);
}

/* Move the relocatable block at BLOCK to some point after the block
   AFTER if at all possible.  Return 1 if successful. */
boolean_t
ROMlib_pushblock (block_header_t *block, block_header_t *after)
{
  block_header_t *t_block;
  THz current_zone;

  current_zone = MR (TheZone);
  for (t_block = after;
       t_block != ZONE_BK_LIM (current_zone) && t_block != block;
       t_block = BLOCK_NEXT (t_block))
    {
      if (PSIZE (t_block) < MIN_BLOCK_SIZE)
	HEAP_DEATH ();

      if (USE (t_block) == FREE)
	{
	  ROMlib_coalesce (t_block);
	  if (PSIZE (t_block) >= PSIZE (block))
	    {
	      ROMlib_moveblock (block, t_block, PSIZE (block) - SIZEC (block));
	      return TRUE;
	    }
	}
    }
  return FALSE;
}

boolean_t ROMlib_memnomove_p = 0;

/* Make space for a block of size SIZE starting at BLOCK.  If space
   exists return 1, else return 0.  */
boolean_t
ROMlib_makespace (block_header_t **block_out, uint32 size)
{
  block_header_t *b;
  uint32 total_size;
  block_header_t *lastblock, *old_block, *block, *bk_lim;
  THz current_zone;

  gui_assert (size > 0);

  current_zone = MR (TheZone);
  total_size = 0;
  old_block = block = *block_out;
  bk_lim = ZONE_BK_LIM (current_zone);
  for (b = block;
       (total_size < size && b != bk_lim
	&& ((USE (b) == FREE)
	    || ((USE (b) == REL)
		&& !ROMlib_locked (b)
		&& !ROMlib_memnomove_p)));
       b = BLOCK_NEXT (b))
    {
      if (PSIZE (b) < MIN_BLOCK_SIZE)
	HEAP_DEATH ();
      total_size += PSIZE (b);
      old_block = b;
    }

  if (total_size < size)
    {
      *block_out = old_block;
      return FALSE;
    }

  /* Sum the size of the remaining free blocks. */
  for (lastblock = b;
       USE (lastblock) == FREE && lastblock != bk_lim;
       lastblock = BLOCK_NEXT (lastblock))
    {
      if (PSIZE (lastblock) < MIN_BLOCK_SIZE)
	HEAP_DEATH ();
      total_size += PSIZE (lastblock);
    }

  for (b = block; b != lastblock; b = BLOCK_NEXT (b))
    {
      if (PSIZE (b) < MIN_BLOCK_SIZE)
	HEAP_DEATH ();
      if (USE (b) == FREE)
	continue;
      gui_assert (USE (b) == REL && !ROMlib_locked (b));
      if (!ROMlib_pushblock (b, lastblock))
	{
	  /* SetHandleSize will still recover through relalloc */
	  *block_out = b;
	  return FALSE;
	}
    }

  if (ZONE_ALLOC_PTR (current_zone) > block
      && ZONE_ALLOC_PTR (current_zone) <= b)
    ZONE_ALLOC_PTR_X (current_zone) = (Ptr) RM (block);

  /* Finally, coalesce the space into one big free block */
  gui_assert (USE (block) == FREE);
  gui_assert ((lastblock == (block_header_t *) ((char *) block + total_size)));

  SETPSIZE (block, total_size);
  return TRUE;
}

/* Tell if BLOCK is locked or not */
boolean_t
ROMlib_locked (block_header_t *block)
{
  Handle h;

  gui_assert (USE (block) == REL);

  h = BLOCK_TO_HANDLE (MR (TheZone), block);
  return (HANDLE_STATE (h, block) & LOCKBIT) || (h == MR (GZRootHnd));
}

/* Find the total amount of free space starting at block.  Compress them
   all. */
int32 
ROMlib_amtfree (block_header_t *block)
{
  int32 total;
  block_header_t *b;
  THz current_zone;

  current_zone = MR (TheZone);

  for (total = 0, b = block;
       USE (b) == FREE && b != ZONE_BK_LIM (current_zone);
       b = BLOCK_NEXT (b))
    {
      if (PSIZE (b) < MIN_BLOCK_SIZE)
	HEAP_DEATH ();
      total += PSIZE (b);
    }

  if (USE (block) == FREE)
    {
      SETPSIZE (block, total);
      if (ZONE_ALLOC_PTR (current_zone) > block
	  && ZONE_ALLOC_PTR (current_zone) <= b)
	ZONE_ALLOC_PTR_X (current_zone) = (Ptr) RM (block);
    }
  return total;
}


#if !defined (NDEBUG)
void 
checkallocptr (void)
{
  block_header_t *b;
  THz current_zone;

  current_zone = MR (TheZone);

  if (ZONE_ALLOC_PTR_X (current_zone) == CLC (0))
    return;
  for (b = ZONE_HEAP_DATA (current_zone);
       b != ZONE_BK_LIM (current_zone);
       b = BLOCK_NEXT (b))
    {
      if (PSIZE (b) < MIN_BLOCK_SIZE)
	HEAP_DEATH ();
      gui_assert (b < ZONE_BK_LIM (current_zone));
      if (b == ZONE_ALLOC_PTR (current_zone))
	return;
    }
  gui_abort ();
}
#endif /* !NDEBUG */


/* Find space for a relocatable block of size SIZE.  Return its
   address in newblk.  Size must include the header.  Return an error
   code.  Don't actually set the block up here.*/
OSErr
ROMlib_relalloc (Size size, block_header_t ** newblk)
{
  block_header_t *b;
  block_header_t *start;
  int32 biggest_block, old_biggest_block;
  THz current_zone;

  current_zone = MR (TheZone);

  /* If we get a negative size, we don't want to loop forever thinking
   * the request is easily satisfied.  This works around that problem.
   */
  if (size < 0)
    size = 0x7FFFFFFF;

  old_biggest_block = 0x7FFFFFFF;
 retry:
  start = ZONE_ALLOC_PTR (current_zone);
  /* We used to just check start for 0, but we have to worry about
     what *other* programs might do to allocPtr... check mman.c for
     more details.  */
  if (start < ZONE_HEAP_DATA (current_zone)
      || start > ZONE_BK_LIM (current_zone))
    start = ZONE_HEAP_DATA (current_zone);

  /* We don't need to account for the rounding or the 12 minimum here.
     Any free block which holds us is at least 12 and even. */

  /* Search from start to the end. */
  for (b = start;
       b != ZONE_BK_LIM (current_zone);
       b = BLOCK_NEXT (b))
    {
      if (PSIZE (b) < MIN_BLOCK_SIZE)
	HEAP_DEATH ();

      /* Is it free and big enough? */
      if (USE (b) == FREE)
	{
	  ROMlib_coalesce (b);
	  if (PSIZE (b) >= (uint32) size)
	    {
	      *newblk = b;
	      return noErr;
	    }
	}
    }

  /* Search from the beginning to start */
  for (b = ZONE_HEAP_DATA (current_zone); b < start; b = BLOCK_NEXT (b))
    {
      if (PSIZE (b) < MIN_BLOCK_SIZE)
	HEAP_DEATH ();
     
      /* Is it free and big enough? */
      if (USE (b) == FREE)
	{
	  ROMlib_coalesce (b);
	  if (PSIZE (b) >= (uint32) size)
	    {
	      *newblk = b;
	      return noErr;
	    }
	}
    }

  /* Try to get some memory */
  biggest_block = CompactMem (size);
  if (biggest_block >= size)
    goto retry;			/* Guaranteed to succeed. */

  /* Try a purge */
  PurgeMem (size);
  if (MemErr == CWC (noErr))
    goto retry;
  /* After a purge, we might be able to compact unlocked relocatable
     blocks and gain enough space, so redo the compact. */
  biggest_block = CompactMem (size);
  if (biggest_block >= size)
    goto retry;

  /* Finally, extend the heap if it's ApplZone */
  if (TheZone == ApplZone)
    {
      int32 newsize;

      newsize = size;
      /* Account for size restrictions */
      if (newsize < MIN_BLOCK_SIZE)
	newsize = MIN_BLOCK_SIZE;
      else
	newsize = (newsize + 3) & ~3;
      
      /* Check that there is room */

      /* As best as I can tell this extension code isn't really doing
	 as good a job as it could, since extending the heap to
	 ApplLimit and then doing a compact could get us enough
	 memory, when just doing the extension itself doesn't.  The
	 big question of course is what does the Mac do. --ctm */

/* #define SCAREY_NEW_OLD_CODE
 * When cotton rewrote the memory manager he made it so that ApplZone starts
 * out as large as it will ever grow and commented out this code.  The problem
 * is at least one program (a flight simulator named DogFight something or
 * other) depends on MaxMem returning a relatively small value that can be
 * grown later.  To make DogFight go more we need to enable this code and
 * then Juke InitApplZone to start ApplZone out a much smaller size.  However,
 * the program has other problems beyond that so it doesn't make sense to 
 * enable this code before we have time to do a lot of testing.
 */

#if defined (SCAREY_NEW_OLD_CODE)
      if ((uint32) MR (ApplLimit) - (uint32) HEAPEND >= newsize)
	{
	  /* Do the extension */
	  /* The new block */
	  b = ZONE_BK_LIM (current_zone);
	  SETPSIZE (b, newsize);
	  *newblk = b;

	  /* The new trailer */
	  ZONE_BK_LIM_X (current_zone) =
	    RM ((block_header_t *)
		((uint32) ZONE_BK_LIM (current_zone) + newsize));
	  b = ZONE_BK_LIM (current_zone);
	  SETZERO (b);
	  SETUSE (b, FREE);
	  SETPSIZE (b, 12);
	  SETSIZEC (b, 0);
	  SET_BLOCK_STATE (b, FREE_BLOCK_STATE);
	  
	  ZONE_ZCB_FREE_X (current_zone) = CL (ZONE_ZCB_FREE (current_zone)
					       + newsize);
	  HeapEnd = (Ptr) ZONE_BK_LIM (current_zone);
	  return noErr;
	}
#endif
    }

  /* Try growing the heap */
  if (ZONE_GZ_PROC_X (current_zone))
    {
      LONGINT retval;

      ZONE_ALLOC_PTR_X (current_zone) = CLC_NULL;
      ROMlib_hook (memory_gznumber);
      HOOKSAVEREGS ();
      retval = CToPascalCall (ZONE_GZ_PROC (current_zone), CTOP_BitNot, size);
      HOOKRESTOREREGS ();

      if (retval)
	ZONE_ALLOC_PTR_X (current_zone) = CLC_NULL;
      if (biggest_block != old_biggest_block)
	{
	  old_biggest_block = biggest_block;
	  goto retry;
	}
    }

  /* Give up. */
  GEN_MEM_ERR (memFullErr);
  return memFullErr;
}
