/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_linux[] = "$Id: linux.c 119 2005-07-11 21:36:20Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/os.h"
#include "rsys/memsize.h"
#include "rsys/mman.h"
#include "linux_except.h"
#include "Gestalt.h"
#include "rsys/gestalt.h"

#include "rsys/lockunlock.h"

static unsigned long
physical_memory (void)
{
  FILE *fp;
  unsigned long mem;
  
  mem = 0;
  fp = fopen ("/proc/meminfo", "r");
  if (fp)
    {
      char buf[256];

      while (fgets (buf, sizeof buf - 1, fp))
	if (!strncmp (buf, "Mem:", 4) && sscanf (buf + 4, "%lu", &mem))
	  break;

      fclose (fp);
    }

  replace_physgestalt_selector (gestaltPhysicalRAMSize, mem);
  return mem;
}


static void
guess_good_memory_settings (void)
{
  unsigned long new_appl_size;

  new_appl_size = physical_memory () / 4;

#if defined (powerpc)

  /* This hack prevents Photoshop 5.5 demo from complaining that we don't
     have enough memory when we run on a 64 MB Linux machine.  Our division
     by four is a bit naive above, so there's really no harm, other than
     ugliness, to this hack.  */

  {
    enum { PHOTOSHOP_55_PREFERRED_SIZE = 16584 * 1024 };

    if (new_appl_size < PHOTOSHOP_55_PREFERRED_SIZE &&
	new_appl_size >= PHOTOSHOP_55_PREFERRED_SIZE * 8 / 10)
      new_appl_size = PHOTOSHOP_55_PREFERRED_SIZE;
  }

#endif

  if (new_appl_size > ROMlib_applzone_size)
    ROMlib_applzone_size = MIN (MAX_APPLZONE_SIZE, new_appl_size);
}


boolean_t
os_init (void)
{
  guess_good_memory_settings ();
#if defined (SDL)
  install_exception_handler ();
#endif
  return TRUE;
}

PUBLIC int
ROMlib_lockunlockrange (int fd, uint32 begin, uint32 count, lockunlock_t op)
{
  int retval;
  struct flock flock;

  warning_trace_info ("fd = %d, begin = %d, count = %d, op = %d",
		      fd, begin, count, op);
  retval = noErr;
  switch (op)
    {
    case lock:
      flock.l_type = F_WRLCK;
      break;
    case unlock:
      flock.l_type = F_UNLCK;
      break;
    default:
      warning_unexpected ("op = %d", op);
      retval = paramErr;
      break;
    }
    
  if (retval == noErr)
    {
      boolean_t success;

      flock.l_whence = SEEK_SET;
      flock.l_start = begin;
      flock.l_len = count;

      success = fcntl (fd, F_SETLK, &flock) != -1;
      if (success)
	retval = noErr;
      else
	{
	  switch (errno)
	    {
	    case EAGAIN:
	    case EACCES:
	      retval = fLckdErr;
	      break;
#if 0
	    case ERROR_NOT_LOCKED:
	      retval = afpRangeNotLocked;
	      break;
#endif
#if 0
	    case ERROR_LOCK_FAILED:
	      retval = afpRangeOverlap;
	      break;
#endif
	    default:
	      warning_unexpected ("errno = %d", errno);
	      retval = noErr;
	      break;
	    }
	}
    }
  return retval;
}

PUBLIC int
ROMlib_launch_native_app (int n_filenames, char **filenames)
{
  char **v;

  v = alloca (sizeof *v * (n_filenames + 1));
  memcpy (v, filenames, n_filenames * sizeof *v);
  v[n_filenames] = 0;
  if (fork () == 0)
    execv (filenames[0], v);

  return 0;
}

#if !(defined (__GNUC__) && defined (__GNUC_MINOR__) && ((__GNUC__ == 2 && __GNUC_MINOR__ == 7 && defined (__i386__)) || defined (RELEASE_INTERNAL) || defined (powerpc) || defined (__alpha)))
#warning THIS IS NOT A PRODUCTION BUILD
#endif

/*
 * There is a very bad problem associated with the use of the db
 * shared libraries under Linux.  Specifically, the calling convention
 * for functions which return structs that are larger than 32 bits
 * somehow got changed when some of the Linux distributions switched
 * from gcc to egcs.  Both compilers put an extra pointer on the stack
 * before calling the routine that returns the large struct, but gcc
 * expects the caller to pop that extra pointer, where egcs expects
 * the called to pop it.  This means that if you compile the caller
 * with gcc and call a shared library that was called with egcs, the
 * stack pointer will be off by 4 bytes after the function returns and
 * the stack is adjusted.  That can be a catastrophe if further code
 * expects the stack to be correct after adjustments.  On the other
 * hand, if we make the questionable call and then do nothing else,
 * the "leave" instruction will restore the stack pointer by using the
 * frame pointer and we'll never be bothered by the extra pop.
 *
 * So, as a workaround, we can wrap the routines, then check the
 * assembly code that the compiler produces to make sure that it's
 * tolerant of the error, then call the wrappers.  That makes the
 * wrapper routines look like voodoo code that was written by a
 * superstitious programmer, but the code (or some other workaround)
 * is absolutely necessary because we want to have one Executor
 * executable that can run with both the shared libraries from Red Hat
 * 5.2 as well as the new ones in SuSE 6.0 (and the new ones that will
 * probably be in Red Hat 6.0).
 *
 * These wrappers absolutely have to be compiled with enough
 * optimization so that the stack isn't adjusted before the leave
 * instruction.  If it is adjusted, then any interrupt that occurs
 * between the adjustment and the transfering of the data from the
 * temporary stack space to the address passed will cause corruption.
 *
 * If you do not understand the above, or if you disagree with it,
 * please contact Cliff before changing the following code.  Axing the
 * code alone and then testing the result is not sufficient, unless
 * you're sure that your test involves *both* db shared libaries.
 *
 */

PUBLIC void
_dbm_fetch (datum *datump, DBM *db, datum datum)
{
  *datump = dbm_fetch (db, datum);
}

PUBLIC void
_dbm_firstkey (datum *datump, DBM *db)
{
  *datump = dbm_firstkey (db);
}

PUBLIC void
_dbm_nextkey (datum *datump, DBM *db)
{
  *datump = dbm_nextkey (db);
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
