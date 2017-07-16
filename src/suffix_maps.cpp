/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_suffix_maps[] =
	    "$Id: suffix_maps.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#include "rsys/suffix_maps.h"
#include "rsys/file.h"

#include <ctype.h>

using namespace Executor;

/*
 * This file contains the routines needed to create mappings from file
 * suffixes to (Creator, Type) pairs.  When a file is created with a suffix
 * that is in the list, the normal % file won't be created.  When the file's
 * creator and type is queried, the creator and type from the list will be
 * read.
 */

typedef struct suffix_entry
{
  char *suffix;
  uint32 creator;
  uint32 type;
  const char *application;
  struct suffix_entry *next;
}
suffix_entry_t;

PRIVATE suffix_entry_t *suffix_head;

PRIVATE boolean_t
str_to_hex (const char *str, uint32 *valp)
{
  boolean_t retval;
  int len;
  
  len = strlen (str);
  retval = (str[0] == '0' &&
	    (str[1] == 'x' || str[1] == 'X') &&
	    len >= 3 &&
	    len <= 10);

  if (retval)
    {
      uint32 val;

      val = 0;
      for (str += 2; retval && *str; ++str)
	if (isxdigit (*str))
	  val = (val << 4) | ROMlib_fromhex (*str);
	else
	  retval = FALSE;
      *valp = val;
    }
  return retval;
}

PUBLIC void
ROMlib_add_suffix_quad (const char *suffixp, const char *creator_hexp,
			const char *type_hexp, const char *applicationp)
{
  uint32 creator;
  uint32 type;

  if (str_to_hex (creator_hexp, &creator) &&
      str_to_hex (type_hexp, &type))
    {
      suffix_entry_t *new1 = (suffix_entry_t *)malloc (sizeof *new1);
      if (new1) {
	  new1->suffix = strdup (suffixp);
	  if (!new1->suffix)
	    free (new1);
	  else
	    {
	      new1->creator = creator;
	      new1->type = type;
	      new1->application = strdup (applicationp);
	      new1->next = suffix_head;
	      suffix_head = new1;
	    }
	}
    }
}

PRIVATE suffix_entry_t **
find_suffixpp (const char *suffix)
{
  suffix_entry_t **retval;

  retval = &suffix_head;

  while (*retval && strcasecmp ((*retval)->suffix, suffix) != 0)
    retval = &(*retval)->next;

  return retval;
}

/*
 * Best match is (creator, 'APPL')
 * next best is (creator, type)
 * next best is (creator, X)
 * worst is (X, type)
 */

PUBLIC const char *
ROMlib_find_best_creator_type_match (uint32 creator, uint32 type)
{
  enum { no_match, type_match, creator_match, both_match, appl_match } match;
  suffix_entry_t **pp;
  const char *retval;

  retval = NULL;
  for (match = no_match, pp = &suffix_head;
       match != appl_match && *pp;
       pp = &(*pp)->next)
    {
      if (*(*pp)->application)
	{
	  if ((*pp)->creator == creator)
	    {
	      if ((*pp)->type == TICK("APPL"))
		{
		  retval = (*pp)->application;
		  match = appl_match;
		}
	      else if (match < both_match && (*pp)->type == type)
		{
		  retval = (*pp)->application;
		  match = both_match;
		}
	      else if (match < creator_match)
		{
		  retval = (*pp)->application;
		  match = creator_match;
		}
	    }
	  else if ((*pp)->type == type && match < type_match) 
	    {
	      retval = (*pp)->application;
	      match = type_match;
	    }
	}
    }

  return retval;
}

PRIVATE boolean_t
filename_suffix_match (const char *suffix, int len, const char *filename)
{
  boolean_t retval;
  int suffix_len;

  suffix_len = strlen (suffix);
  retval = (len >= suffix_len && 
	    strncasecmp (suffix, filename + len - suffix_len, suffix_len)
	    == 0);
  return retval;
}

PRIVATE suffix_entry_t **
find_filenamepp (int len, const char *filename)
{
  suffix_entry_t **retval;

  retval = &suffix_head;

  while (*retval && !filename_suffix_match ((*retval)->suffix, len, filename))
    retval = &(*retval)->next;

  return retval;
}

PUBLIC boolean_t
ROMlib_creator_and_type_from_suffix (const char *suffix, uint32 *creatorp,
				     uint32 *typep)
{
  boolean_t retval;
  suffix_entry_t **pp;

  pp = find_suffixpp (suffix);
  if (!*pp)
    retval = FALSE;
  else
    {
      retval = TRUE;
      if (creatorp)
	*creatorp = (*pp)->creator;
      if (typep)
	*typep = (*pp)->type;
    }
  return retval;
}

PUBLIC boolean_t
ROMlib_creator_and_type_from_filename (int len, const char *filename,
				       uint32 *creatorp, uint32 *typep)
{
  boolean_t retval;
  suffix_entry_t **pp;

  pp = find_filenamepp (len, filename);
  if (!*pp)
    retval = FALSE;
  else
    {
      retval = TRUE;
      if (creatorp)
	*creatorp = (*pp)->creator;
      if (typep)
	*typep = (*pp)->type;
    }
  return retval;
}

PRIVATE void
release_entry (suffix_entry_t *to_release)
{
  free (to_release->suffix);
  free (to_release);
}

PUBLIC boolean_t
ROMlib_delete_suffix (const char *suffix)
{
  boolean_t retval;
  suffix_entry_t **pp;

  pp = find_suffixpp (suffix);
  if (!*pp)
    retval = FALSE;
  else
    {
      suffix_entry_t *to_release;

      retval = TRUE;
      to_release = *pp;
      *pp = (*pp)->next;
      release_entry (to_release);
    }
  return retval;
}
