/* Copyright 1994 - 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_lowglobals_mem[] = "$Id: lowglobals-mem.c 85 2005-05-24 22:06:13Z ctm $";
#endif

#include "rsys/common.h"

#include <sys/types.h>
#include <sys/mman.h>

#include "rsys/memory_layout.h"
#include "rsys/assert.h"
#include "rsys/lowglobals.h"

#include <unistd.h>
#include <signal.h>

void
mmap_lowglobals (void)
{
  if (!force_big_offset)
    {
      caddr_t addr;

      addr = mmap ((caddr_t) PAGE_ZERO_START,
		   PAGE_ZERO_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE, -1, 0);
      gui_assert (addr == (caddr_t) PAGE_ZERO_START);
    }
}

#if !defined (powerpc)
PRIVATE caddr_t
round_up_to_page_size (unsigned long addr)
{
  caddr_t retval;
  size_t page_size;

  page_size = getpagesize ();
  retval = (caddr_t) ((addr + page_size - 1) / page_size * page_size);
  return retval;
}
#endif

static jmp_buf segv_return;

static void
segv_handler (int signum_ignored __attribute__((unused)))
{
  siglongjmp (segv_return, 1);
}

static bool
mmap_conflict (void *start, size_t length)
{
  bool retval;
  long page_size;

  retval = false;

  page_size = sysconf(_SC_PAGESIZE);

  if ((long) start % page_size != 0)
    {
      retval = true;
      warning_unexpected ("start = %p, page_size = %ld, "
			  "start %% page_size = %ld",
			  start, page_size, (long) start % page_size);
    }
  else if (length % page_size != 0)
    {
      retval = true;
      warning_unexpected ("length = %ld, page_size = %ld, "
			  "length %% page_size = %ld",
			  (long) length, page_size, (long) length % page_size);
    }
  else
    {  
      sig_t old_segv_handler;
      volatile int n_pages;
      volatile int n_failures;
      volatile char *volatile addr;
      char *stop;

      n_pages = 0;
      n_failures = 0;
      stop = (char *) start + length;

      old_segv_handler = signal (SIGSEGV, segv_handler);
      for (addr = start; addr < stop; addr += page_size)
	{
	  ++n_pages;
	  if (sigsetjmp (segv_return, 1) != 0)
	    ++n_failures;
	  else
	    *addr;
	}
      signal (SIGSEGV, old_segv_handler);
      retval = n_failures < n_pages;
      if (retval)
	warning_unexpected ("%d pages were already mapped",
			    n_pages - n_failures);
    }

  return retval;
}

void *
mmap_permanent_memory (unsigned long amount_wanted)
{
  caddr_t addr_got;
  caddr_t badness_start;

  /* Only do this if our text segment is up nice and high out of the way. */
  if (((unsigned long) mmap_permanent_memory & 0xFF000000L) == 0)
    return NULL;

  {
    extern void *_start;

    badness_start = (caddr_t) ((unsigned long) &_start
			       / (1024 * 1024) * (1024 * 1024));
  }

#if !defined (powerpc)  
  {
    caddr_t addr_wanted;

    addr_wanted = round_up_to_page_size (PAGE_ZERO_START + PAGE_ZERO_SIZE);

    if (addr_wanted + amount_wanted > badness_start)
      {
	warning_unexpected ("addr_wanted = %p, amount_wanted = 0x%lx, "
			    "badness_start = %p", addr_wanted, amount_wanted,
			    badness_start);
	return NULL;
      }

    if (mmap_conflict (addr_wanted, amount_wanted))
      addr_got = NULL;
    else
      {
	addr_got = mmap (addr_wanted, amount_wanted, PROT_READ | PROT_WRITE,
			 MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE, -1, 0);
	if (addr_got == (caddr_t) -1)
	  addr_got = NULL;
	else if (addr_got != addr_wanted)
	  warning_unexpected ("addr_wanted = %p, addr_got = %p",
			      addr_wanted, addr_got);
      }
  }
#else


#warning THIS CODE IS PROBABLY WRONG

  /*
   * I haven't tested a powerpc build in a while, but I just noticed that
   * we're trying to mmap from 0 and then we're returning addr_got.  I think
   * that when we return 0, the caller believes that we weren't able to
   * mmap the low globals.  As such, the code below PROBABLY DOESN'T DO
   * ANYTHING DIFFERENT THAN SIMPLY RETURNING NULL.
   */

  if (amount_wanted > badness_start)
    {
      warning_unexpected ("amount_wanted = 0x%x, badness_start = %p",
			  amount_wanted, badness_start);
      addr_got = NULL;
    }
  else
    addr_got = mmap (0, amount_wanted, PROT_READ | PROT_WRITE,
		     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (addr_got == (caddr_t) -1)
    addr_got = NULL;
#endif

  return addr_got;
}
