/* Copyright 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_openmany[] = "$Id: openmany.c 63 2004-12-24 18:19:43Z ctm $";
#endif


#include "rsys/common.h"
#include "openmany.h"
#include "rsys/notmac.h"
#include "rsys/vdriver.h"


msdos_open_many_result_t msdos_open_many_result = MSDOS_OM_NOT_TESTED;


/* This function is used to see if "enough" files can be open
 * simultaneously on this system, to quickly identify systems
 * where the user has something like FILES=8 (etc.) in their
 * config.sys.
 *
 * The idea is that you call this routine with a directory known
 * to have a bunch of files (e.g. the Executor configuration directory).
 * This routine will attempt to open `num_files' files with the
 * specified suffix simultaneously.  Three return values are possible
 * ("_OM_" is short for "OPEN_MANY"):
 *
 * MSDOS_OM_SUCCESS:        the requested number of files were opened
 *                            simultaneously.
 * MSDOS_OM_FAILURE:        we ran out of file descriptors trying to
 *                            open files.
 * MSDOS_OM_UNABLE_TO_TEST: we were unable to prove or disprove that
 *                            we have an adequate number of file
 *                            descriptors, perhaps because the
 *                            directory didn't exist or there weren't
 *                            enough files in it to do a full test.
 *
 * Note that some file descriptors may already be in use for other
 * purposes, so the maximum value for which this routine returns
 * MSDOS_OM_SUCCESS is probably not quite the same as the maximum
 * total file descriptors available.  The true maximum will almost
 * always be somewhat higher.
 */

static msdos_open_many_result_t
msdos_open_many_files (const char *directory, const char *suffix,
		       int num_files)
{
  DIR *dirp;
#if defined (USE_STRUCT_DIRECT)
  struct direct *dp;
#else
  struct dirent *dp;
#endif
  msdos_open_many_result_t result;

  dirp = Uopendir (directory);
  if (dirp == NULL)
    result = MSDOS_OM_UNABLE_TO_TEST;
  else
    {
      int *fd;
      int num_open, suffix_length;
      char path[MAXPATHLEN], *pathend;

      fd = alloca (num_files * sizeof fd[0]);
      suffix_length = strlen (suffix);

      /* Set up the basic path for the files. */
      if (directory[0] == '\0')
	path[0] = '\0';
      else
	sprintf (path, "%s/", directory);
      pathend = path + strlen (path);

      /* Try to open the specified number of files with the given suffix.
       * Some files may fail to open because we don't have permission,
       * or because they are directories, etc.  Since we're only seeing
       * how many files we can have open simultaneously, we don't
       * let such failures bother us.
       */
      result = MSDOS_OM_SUCCESS;  /* default */
      for (num_open = 0; num_open < num_files && result == MSDOS_OM_SUCCESS; )
	{
	  dp = readdir (dirp);

	  if (dp == NULL)
	    result = MSDOS_OM_UNABLE_TO_TEST;
	  else if (dp->d_namlen >= suffix_length
		   && !strcmp (dp->d_name + dp->d_namlen - suffix_length,
			       suffix))
	    {
	      strcpy (pathend, dp->d_name);
	      fd[num_open] = Uopen (path, O_RDONLY, 0 /* unused */);
	      if (fd[num_open] >= 0)
		++num_open;
	      else if (errno == EMFILE || errno == ENFILE)
		result = MSDOS_OM_FAILURE;
	    }
	}

      /* Close all the files we opened. */
      while (num_open > 0)
	close (fd[--num_open]);

      /* Close the directory. */
      closedir (dirp);
    }

  return result;
}


/* Returns FALSE if we know we don't have enough available file
 * descriptors, else TRUE.
 */
bool
msdos_test_max_files (void)
{
  char config_dir[MAXPATHLEN];

  if (ROMlib_startdir == NULL)
    {
      vdriver_shutdown ();
      fprintf (stderr, "startdir is NULL!\n");
      abort ();
    }

  /* We can't use ROMlib_ConfigurationFolder because that's not
   * guaranteed to be set up yet.  And it gets set up in the same routine
   * that tries to open a slew of .HFV's.
   */
  sprintf (config_dir, "%s/configur", ROMlib_startdir);

  /* The "6" here is arbitrary; it's merely meant to reflect a guess
   * about how many files might already be open.  If you make it too
   * small, then a user who actually has FILES=30 might still get
   * a complaint that his FILES= parameter is too small.
   */
  msdos_open_many_result = msdos_open_many_files (config_dir, ".ecf",
						  MSDOS_DESIRED_MIN_FILES - 6);

  if (msdos_open_many_result == MSDOS_OM_FAILURE)
    {
      vdriver_shutdown ();
      fputs ("\
Fatal error: FILES= parameter in CONFIG.SYS is too small.  We recommend
FILES=30 (or more).  To fix this, edit your CONFIG.SYS file (usually
C:\\CONFIG.SYS), and find the line that says something like:

FILES=20

(it may not be exactly 20) and change the number to 30 or more.  If the
FILES= value is already greater than 30 and you still get this message,
you may need to make it even larger.  You can use the \"-nofilescheck\"
command line switch to skip this check and proceed at your own risk.\n",
	     stderr);
      exit (-11);
    }

  return (msdos_open_many_result != MSDOS_OM_FAILURE);
}
