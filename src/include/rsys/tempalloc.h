#if !defined (_tempalloc_h_)
#define _tempalloc_h_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: tempalloc.h 63 2004-12-24 18:19:43Z ctm $
 */

/* This header contains macros which are useful if you want to allocate
 * a potentially large amount of temporary storage.  Since we don't
 * have much stack space under some environments, this code will guarantee
 * you don't blow your stack.  It is safe to TEMP_ALLOC_FREE this memory
 * even if you never allocated it (in that case, nothing will happen).
 * This is handy when you may conditionally allocate new memory, and
 * you want to make sure any allocated memory gets freed at the end.
 *
 * Here's an example:
 *
 * void foo (void *x)
 * {
 *   TEMP_ALLOC_DECL (my_temp_storage);
 *   if (make_x_bigger)
 *     TEMP_ALLOC_ALLOCATE (x, my_temp_storage, 100000);
 *   do_stuff (x);
 *   TEMP_ALLOC_FREE (my_temp_storage);
 * }
 */

#if defined (MSDOS) || defined (CYGWIN32)

#include "MemoryMgr.h"
#include "rsys/mman.h"

typedef enum
{
  TEMP_ALLOC_NO_FREE,
  TEMP_ALLOC_FREE,
  TEMP_ALLOC_DISPOSHANDLE
} temp_alloc_status_t;


typedef struct
{
  temp_alloc_status_t status;
  union
    {
      Handle handle;
      void *ptr;
    } u;
} temp_alloc_data_t;


#define TEMP_ALLOC_DECL(name) \
  temp_alloc_data_t name = { TEMP_ALLOC_NO_FREE, { 0 } }

#define TEMP_ALLOC_ALLOCATE(ptr_var, name, size)			\
do {									\
  if ((size) <= 8192)  /* Satisfy small requests with alloca. */	\
    {									\
      (name).status = TEMP_ALLOC_NO_FREE;				\
      ptr_var = (name).u.ptr = (void *) alloca (size);			\
    }									\
  else									\
    {									\
      ZONE_SAVE_EXCURSION	/* Try SysZone first. */		\
	(SysZone, { (name).u.handle = NewHandle (size); });		\
      if (!(name).u.handle)						\
        ZONE_SAVE_EXCURSION	/* Then ApplZone. */			\
	  (ApplZone, { (name).u.handle = NewHandle (size); });		\
      if ((name).u.handle)						\
	{								\
	  (name).status = TEMP_ALLOC_DISPOSHANDLE;			\
	  HLock ((name).u.handle);					\
	  ptr_var = (void *) STARH ((name).u.handle);			\
	}								\
      else								\
	{								\
	  /* Use malloc. */						\
	  (name).status = TEMP_ALLOC_FREE;				\
	  ptr_var = (name).u.ptr = (void *) malloc (size);		\
	}								\
    }									\
} while (0)

#define TEMP_ALLOC_FREE(name)				\
do {							\
  if ((name).status == TEMP_ALLOC_FREE)			\
    free ((name).u.ptr);				\
  else if ((name).status == TEMP_ALLOC_DISPOSHANDLE)	\
    {							\
      HUnlock ((name).u.handle);			\
      DisposHandle ((name).u.handle);			\
    }							\
  (name).u.ptr = 0;					\
} while (0)

#else /* !MSDOS */

/* On other platforms, we can alloca extremely large amounts,
 * so there's no need for this complexity.
 */
#define TEMP_ALLOC_DECL(name)
#define TEMP_ALLOC_ALLOCATE(ptr_var, name, size) (ptr_var = (typeof(ptr_var))alloca (size))
#define TEMP_ALLOC_FREE(name)

#endif /* !MSDOS */

#endif /* !_tempalloc_h_ */
