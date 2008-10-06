/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_msdos[] = "$Id: msdos.c 119 2005-07-11 21:36:20Z ctm $";
#endif

#include "rsys/common.h"
#include "vga.h"
#include "rsys/os.h"
#include "rsys/mman.h"
#include "rsys/version.h"
#include "rsys/memsize.h"
#include "rsys/lockunlock.h"
#include "dosmem.h"
#include "dpmilock.h"
#include "openmany.h"
#include <crt0.h>
#include <dpmi.h>

/*
 * Sbrk is a greek tragedy -- everyone dies.
 *
 * DJGPP provides two sbrk methods, one keeps all memory together in one
 * hunk, but to do so it must move memory around.  There seems to be no
 * way to move memory around under Windows 3.x's DPMI implementation that
 * will not cause death during certain types of interrupts.  Hence, once
 * interrupts are enabled we can't move memory.  The other sbrk method uses
 * (potentially) discontinuous hunks of memory and due to the way it is
 * allocated, it can appear at any place in the address space, including
 * at location 0xF....... meaning that addresses can appear to be negative.
 * That would be really bad for Mac memory space to have anything in its
 * high nibble, so we use the non-gapped, but memory moving sbrk in the
 * beginning and then later switch to the potentially gapped, but non-moving
 * sbrk after we've gotten our big hunk.
 *
 * Very evil, indeed.
 *
 */

/* Specify up the flags djgpp uses to determine system features. */
int _crt0_startup_flags = _CRT0_FLAG_PRESERVE_UPPER_CASE | \
                          _CRT0_FLAG_NULLOK | _CRT0_FLAG_UNIX_SBRK;

/* Tell crt0 to give us at least a 500K stack. */
int _stklen = 500 * 1024;

/* Record how much DPMI memory we autodetected at initialization time. */
static long detected_dpmi_mem = -1;

void
switch_to_non_moving_sbrk (void)
{
  _crt0_startup_flags &= ~(_CRT0_FLAG_UNIX_SBRK | _CRT0_FLAG_NONMOVE_SBRK);
  _crt0_startup_flags |= _CRT0_FLAG_NONMOVE_SBRK;
}

/* Set to TRUE when we aren't able to get the real free memory info. */
static boolean_t dpmi_mem_is_rough_guess_p;

static void
guess_good_memory_settings (void)
{
  __dpmi_free_mem_info mem_info;
  unsigned long page_size;

  /* We set these up in case something bad happens in the DPMI
   * calls and they don't get initialized.  They should get
   * clobbered.
   */
  mem_info.total_number_of_free_pages = 1024 * 1024;
  page_size = 4096;

  if (__dpmi_get_free_memory_information (&mem_info) != -1
      && __dpmi_get_page_size (&page_size) != -1)
    {
      LONGINT new_appl_size;

      /* OS/2 Warp declines to tell us how many free pages there are
       * (as the DPMI spec allows it to do).  So if we can't get the
       * total free pages, we'll use a different heuristic.
       */
      if (mem_info.total_number_of_free_pages != -1)
	{
	  detected_dpmi_mem = mem_info.total_number_of_free_pages * page_size;
	  dpmi_mem_is_rough_guess_p = FALSE;
	}
      else
	{
	  uint32 phys_pages, linear_pages;
	  long pages;

	  /* Compute a decent guess for how much memory we should assume
	   * we have.  We use unsigned numbers here because DPMI can
	   * give us "0xFFFFFFFF" for missing values.
	   */
	  phys_pages = mem_info.total_number_of_physical_pages;
	  linear_pages = mem_info.free_linear_address_space_in_pages;
	  pages = MIN (phys_pages, linear_pages); /* heuristic */

	  if (pages >= 0)
	    detected_dpmi_mem = pages * page_size;
	  else
	    detected_dpmi_mem = 0;

	  dpmi_mem_is_rough_guess_p = TRUE;
	}

      if (detected_dpmi_mem > 512 * 1024 * 1024)
	{
	  /* Handle ridiculous numbers gracefully; something bad
	   * has happened, so don't choose a huge default.
	   */
	  detected_dpmi_mem = 0;
	}

      new_appl_size = detected_dpmi_mem / 2;
      if (new_appl_size > ROMlib_applzone_size)
	{
	  ROMlib_applzone_size = MIN (MAX_APPLZONE_SIZE, new_appl_size);
	}
    }
  else
    detected_dpmi_mem = -1;
}


/* Initializes OS-specific features.  Returns TRUE if successful, FALSE
 * on failure.
 */
boolean_t
os_init (void)
{
  /* Turn off expensive stat computations we don't need (you turn them
   * off by ORing in the appropriate bit).
   */
  _djstat_flags |= (_STAT_EXEC_EXT | _STAT_EXEC_MAGIC | _STAT_DIRSIZE);

  /* Juke stderr to go to stdout, so output can get captured to a file. */
  close (2);
  dup2 (1, 2);

  /* Set up conventional memory and releated information. */
  if (!init_dos_memory ())
    {
      puts ("Unable to allocate conventional memory.");
      return FALSE;
    }

  /* Lock down memory we might touch at interrupt time. */
  dpmi_lock_memory (0, 4096 * 2);  /* low globals + some crt0.s stuff */
  dpmi_lock_memory (&cpu_state, sizeof cpu_state);

  /* Make an educated guess for what memory sizes the user might want. */
  guess_good_memory_settings ();

  return TRUE;
}


void
msdos_print_info (void)
{
  printf ("This is %s, compiled %s.\n",
	  ROMlib_executor_full_name, ROMlib_executor_build_time);

  /* Print out CPU type. */
  if (arch_type == ARCH_TYPE_I386)
    printf ("CPU type is 80386.\n");
  else
    printf ("CPU type is 80486 or better.\n");

  /* Print out DPMI memory info. */
  if (detected_dpmi_mem <= 0)
    printf ("Unable to determine free DPMI memory.\n");
  else
    printf ("%ld bytes (%lu.%02lu MB) of potentially useful physical DPMI "
	    "memory detected%s.\n",
	    detected_dpmi_mem, detected_dpmi_mem / (1024U * 1024),
	    ((detected_dpmi_mem % (1024U * 1024)) * 100 / (1024 * 1024)),
	    dpmi_mem_is_rough_guess_p ? " (rough guess)" : "");

#define MB (1024 * 1024U)

  /* Print out actual memory size chosen. */
  printf ("Choosing %u.%02u MB for applzone, %u.%02u MB for syszone, "
	  "%u.%02u MB for stack\n",
	  ROMlib_applzone_size / MB,
	  (ROMlib_applzone_size % MB) * 100 / MB,
	  ROMlib_syszone_size / MB,
	  (ROMlib_syszone_size % MB) * 100 / MB,
	  ROMlib_stack_size / MB,
	  (ROMlib_stack_size % MB) * 100 / MB);

  /* Print out video driver info. */
  if (vesa_version)
    {
      printf ("VESA-compatible video driver detected, VBE version %d.%d\n",
	      vesa_version >> 8, vesa_version & 0xFF);
      if (vesa_version < 0x200)
	puts ("Executor works best with video drivers compatible with "
	      "VBE 2.0 or higher,\n"
              "such as SciTech Display Doctor (http://www.scitechsoft.com).");
    }
  else
    printf ("No VESA-compatible video driver detected; SVGA video modes "
	    "are unavailable.\n");

  switch (msdos_open_many_result)
    {
    case MSDOS_OM_UNABLE_TO_TEST:
      printf ("*** WARNING: Unable to verify that FILES= is big enough.\n");
      break;
    case MSDOS_OM_FAILURE:
      printf ("*** WARNING: FILES= is too small!  You need FILES=30 (or more) "
	      "in CONFIG.SYS\n");
      break;
    case MSDOS_OM_SUCCESS:
      printf ("FILES= in CONFIG.SYS appears to be large enough...good.\n");
      break;
    case MSDOS_OM_NOT_TESTED:
      /* Print nothing here; it's confusing our users. */
#if 0
      printf ("FILES= in CONFIG.SYS has not been checked.\n");
#endif
      break;
    default:
      printf ("Internal error checking FILES= !  result == %d.\n",
	      (int) msdos_open_many_result);
      break;
    }
}


/* Tries to allocate a memory block of the specified size from the
 * DPMI server.  The intent is to test if future memory allocations
 * are likely to succeed.  free (malloc (size)) would be a Really Bad
 * idea under djgpp because of how the malloc package works.  This
 * should hopefully work better.
 */
boolean_t
msdos_check_memory_remaining (unsigned long desired_bytes)
{
  __dpmi_meminfo meminfo;
  boolean_t success_p;

  meminfo.handle  = 0;	/* unused */
  meminfo.size    = desired_bytes;
  meminfo.address = 0;	/* unused */
  
  success_p = (__dpmi_allocate_memory (&meminfo) == 0);
  if (success_p)
    __dpmi_free_memory (meminfo.handle);

  return success_p;
}

#define paramErr		(-50)

PUBLIC int
ROMlib_lockunlockrange (int fd, uint32 begin, uint32 count, lockunlock_t op)
{
  warning_unimplemented (NULL_STRING);
  return paramErr;
}

PUBLIC int
ROMlib_launch_native_app (int n_filenames, char **filenames)
{
  warning_unimplemented (NULL_STRING);
  return paramErr;
}

PUBLIC boolean_t host_has_spfcommon (void)
{
  return false;
}

PUBLIC boolean_t
host_spfcommon (host_spf_reply_block *replyp, const char *prompt,
		const char *incoming_filename, void *fp, void *filef, int numt,
		void *tl, getorput_t getorput, sf_flavor_t flavor,
		void *activeList, void *activateproc, void *yourdatap)
{
  return false;
}
