/* Copyright 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_checkpoint[] =
	    "$Id: checkpoint.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#if defined (STANDALONE_TEST)

#warning do not check this in

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned char bool;
typedef unsigned int uint32;
#define PUBLIC
#define PRIVATE static
PRIVATE uint32 ROMlib_macdrives;
PRIVATE uint32 ROMlib_dosdrives;
enum { false, true };

#endif

#include "rsys/common.h"
#include "rsys/stdfile.h"
#include "rsys/option.h"
#include "rsys/checkpoint.h"

#if defined (MSDOS) || defined (CYGWIN32)

#if defined (CYGWIN32)
#include "winfs.h"
#endif

PRIVATE bool checkpointing_p = true;

PUBLIC checkpoint_t *checkpointp; /* globals are bad, but we use one in
				     this case because checkpointing is
				     effectively a global issue currently
				     and we don't want to modify the various
				     routines that we checkpoint to take
				     an additional argument */

PRIVATE uint32
extract_bad_drives_from_string (const char *opt, const char *str)
{
  int len;
  char *p;
  uint32 good_drives;
  uint32 retval;

  while (*str != ' ')
    ++str;
  ++str;
  len = strspn (str, "abcdefghijklmnopqrstuvwxyz-");
  p = alloca (len+1);
  memcpy (p, str, len);
  p[len] = 0;
  good_drives = parse_drive_opt (opt, p);
  retval = ~good_drives & ((1 << 26)-1);
  return retval;
}

PUBLIC checkpoint_t *
checkpoint_init (void)
{
  checkpoint_t *retval;
  FILE *fp;

  retval = malloc (sizeof *retval);
  memset (retval, 0, sizeof *retval);
  fp = executor_dir_fopen (CHECKPOINT_FILE, "r");
  if (fp)
    {
      char buf[1024], *p;
      int nread;
      
      nread = fread (buf, 1, sizeof buf-1, fp);
      fclose (fp);
      buf[sizeof buf -1] = 0;
      if (strstr (buf, "-nosound"))
	retval->sound_fails = true;
      if (strstr (buf, "-skipaspi"))
	retval->aspi_fails = true;
      p = strstr (buf, "-macdrives ");
      if (p)
	retval->bad_macdrives
	  = extract_bad_drives_from_string ("macdrives", p);
      p = strstr (buf, "-dosdrives ");
      if (p)
	retval->bad_dosdrives
	  = extract_bad_drives_from_string ("dosdrives", p);
    }
  return retval;
}

PRIVATE void
write_drive_string (FILE *fp, uint32 drive)
{
  drive &= (1 << 26)-1; /* only represent A-Z */
  if (!drive)
    fprintf (fp, "\"\"\n");
  else
    {
      int drive_bit;

      drive_bit = 0;
      while (drive)
	{
	  while ((drive & (1 << drive_bit)) == 0)
	    drive_bit++;
	  if ((drive & (1 << (drive_bit+1))) == 0)
	    {
	      fprintf (fp, "%c", 'a' + drive_bit);
	      drive &= ~(1 << drive_bit);
	    }
	  else
	    {
	      if ((drive & (1 << (drive_bit+2))) == 0)
		{
		  fprintf (fp, "%c%c", 'a' + drive_bit, 'a' + drive_bit + 1);
		  drive &= ~((1 << drive_bit) | (1 << (drive_bit+1)));
		}
	      else
		{
		  int last_good_bit;
		  uint32 mask;

		  last_good_bit = drive_bit+2;
		  mask = (1 << drive_bit) | (1 << (drive_bit+1))
		    | (1 << (drive_bit+2));
		  while ((drive & (1 << (last_good_bit+1))) != 0)
		    {
		      ++last_good_bit;
		      mask |= (1 << last_good_bit);
		    }
		  fprintf (fp, "%c-%c", 'a' + drive_bit, 'a' + last_good_bit);
		  drive &= ~mask;
		}
	    }
	}
      fprintf (fp, "\n");
    }
}

#define CHECKPOINT_PREAMBLE \
"# This file was created because Executor found a problem while it was\n" \
"# probing your system.  The lines in this file that don't start with\n" \
"# a '#' are commands that Executor uses to bypass the failing subsystems.\n" \
"#\n" \
"# DO NOT ALTER THIS FILE unless you know what you're doing and even then\n" \
"# make sure you make a backup, first.\n"

PRIVATE void
write_checkpoint_file (checkpoint_t *cp)
{
  FILE *fp;

  if (!(cp->sound_fails
	|| cp->aspi_fails
	|| cp->bad_macdrives
	|| cp->bad_dosdrives))
    executor_dir_remove (CHECKPOINT_FILE);
  else
    {
      fp = executor_dir_fopen (CHECKPOINT_FILE, "w");
      if (fp)
	{
	  fprintf (fp, CHECKPOINT_PREAMBLE);
	  if (cp->sound_fails)
	    fprintf (fp, "-nosound\n");
	  if (cp->aspi_fails)
	    fprintf (fp, "-skipaspi\n");
	  if (cp->bad_macdrives)
	    {
	      uint32 macdrives;

	      macdrives = ROMlib_macdrives ? ROMlib_macdrives : ~0;
	      macdrives &= ~cp->bad_macdrives;
	      fprintf (fp, "-macdrives ");
	      write_drive_string (fp, macdrives);
	    }
	  if (cp->bad_dosdrives)
	    {
	      uint32 dosdrives;

	      dosdrives = ROMlib_dosdrives ? ROMlib_dosdrives : ~0;
	      dosdrives &= ~cp->bad_dosdrives;
	      fprintf (fp, "-dosdrives ");
	      write_drive_string (fp, dosdrives);
	    }
	  fclose (fp);
	  sync ();
	}
    }
}

PUBLIC void
checkpoint_sound (checkpoint_t *cp, checkpoint_option option)
{
  if (checkpointing_p && cp)
    {
      cp->sound_fails = option == begin;
      write_checkpoint_file (cp);
    }
}

PUBLIC void
checkpoint_aspi (checkpoint_t *cp, checkpoint_option option)
{
  if (checkpointing_p && cp)
    {
      cp->aspi_fails = option == begin;
      write_checkpoint_file (cp);
    }
}

PUBLIC void
checkpoint_macdrive (checkpoint_t *cp, checkpoint_option option, uint32 drive)
{
  if (checkpointing_p && cp)
    {
      if (option == begin)
	cp->bad_macdrives |= drive;
      else
	cp->bad_macdrives &= ~drive;
      write_checkpoint_file (cp);
    }
}

PUBLIC void
checkpoint_dosdrives (checkpoint_t *cp, checkpoint_option option, uint32 drive)
{
  if (checkpointing_p && cp)
    {
      if (option == begin)
	cp->bad_dosdrives |= drive;
      else
	cp->bad_dosdrives &= ~drive;
      write_checkpoint_file (cp);
    }
}

PUBLIC void
disable_checkpointing (void)
{
  checkpointing_p = false;
}

#if defined (STANDALONE_TEST)
PUBLIC int
main (void)
{
  checkpoint_t *cp;

  cp = checkpoint_init ();
  return 0;
}
#endif

#endif
