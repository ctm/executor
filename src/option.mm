/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_option[] =
	    "$Id: option.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "rsys/flags.h"
#include "rsys/option.h"
#include "rsys/parsenum.h"
#include "rsys/notmac.h"

#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSString.h>

using namespace Executor;

struct opt_block
{
  char *interface;
  option_t *opts;
  int n_opts;
} *opt_blocks;
static int n_opt_blocks_max;
static int n_opt_blocks;

static int max_pre_notes;
static int n_pre_notes;
static char **pre_notes;

static char *wrap_buf;
static int wrap_buf_size;

static int help_buf_len, help_buf_len_max;
static char *help_buf;

void
Executor::opt_init (void)
{
  
}

void
Executor::opt_shutdown (void)
{
  if (wrap_buf) {
    free (wrap_buf);
    wrap_buf = nullptr;
  }
  
  if (help_buf) {
    free (help_buf);
    help_buf = nullptr;
  }
  
  if (pre_notes) {
    free (pre_notes);
    pre_notes = nullptr;
  }
  
  if (opt_blocks) {
    free (opt_blocks);
    opt_blocks = nullptr;
  }
}

void
strcpy_to_wrap_buf (char *text)
{
  int text_len;
  char *src, *dst;

  text_len = strlen (text) + 1;

  if (wrap_buf_size < text_len)
    {
      wrap_buf = (char*)realloc (wrap_buf, text_len * sizeof *wrap_buf);
      wrap_buf_size = text_len;
    }
  for (dst = wrap_buf, src = text; *src;)
    {
      if (*src != '\n')
	*dst ++ = *src ++;
      else
	src ++;
    }
  *dst = '\0';
}

static void
wrap (char *buf,
      int desired_len,
      int *out_len, int *next_len)
{
  int buf_len;

  buf_len = strlen (buf);
  
  if (buf_len <= desired_len)
    {
      *out_len = buf_len;
      *next_len = -1;
    }
  else
    {
      char *t;

      t = &buf[desired_len];
      while (!isspace (*t))
	t --;
      *next_len = (t - buf) + 1;

      while (isspace (*t))
	t --;

      *out_len = (t - buf) + 1;
    }
}

void
_safe_strncat (char *dst, const char *src, int len)
{
  char *t;
  int dst_len;

  dst_len = strlen (dst);
  t = &dst[dst_len];

  memcpy (t, src, len);

  dst[dst_len + len] = '\0';
}

void
send_to_help_buf (const char *text, int len, int append_newline_p)
{
  if (help_buf == NULL)
    {
      help_buf = (char*)malloc (2 * len);
      help_buf_len_max = 2 * len;

      *help_buf = '\0';
      help_buf_len = 1;
    }
  else if (help_buf_len + len > help_buf_len_max)
    {
      help_buf_len_max *= 2;
      help_buf = (char*)realloc (help_buf, help_buf_len_max * sizeof *help_buf);
    }
  
  _safe_strncat (help_buf, text, len);
  help_buf_len += len;
  if (append_newline_p)
    {
      strcat (help_buf, "\n");
      help_buf_len += len;
    }
}

void
_generate_help_message (void)
{
  int i, block_i;
  char *buf;

  for (i = 0; i < n_pre_notes; i ++)
    {
      int out_len, next_len;
      char *pre_note = pre_notes[i];
      
      strcpy_to_wrap_buf (pre_note);
      
      for (buf = wrap_buf;; buf += next_len)
	{	   
	  wrap (buf, 75, &out_len, &next_len);
	  send_to_help_buf (buf, out_len, TRUE);
	  if (next_len == -1)
	    break;
	}
    }
  /* newline to separate pre-notes and options */
  send_to_help_buf ("", 0, TRUE);
  for (block_i = 0; block_i < n_opt_blocks; block_i ++)
    {
      char *interface = opt_blocks[block_i].interface;
      option_t *opts = opt_blocks[block_i].opts;
      int n_opts = opt_blocks[block_i].n_opts;
      int opt_i;

      send_to_help_buf (interface, strlen (interface), FALSE);
      send_to_help_buf (":", 1, TRUE);
      
      for (opt_i = 0; opt_i < n_opts; opt_i ++)
	{
	  const char *spaces = "                    ";
	  int next_len, out_len;
	  
	  option_t *opt = &opts[opt_i];
	  int opt_text_len;
	  int same_line_p;

	  if (!opt->desc)
	    continue;
	  
	  opt_text_len = strlen (opt->text);
	  same_line_p = opt_text_len < 14;
	  
	  send_to_help_buf ("  -", 3, FALSE);
	  send_to_help_buf (opt->text, opt_text_len,
			    !same_line_p);
	  if (same_line_p)
	    send_to_help_buf (spaces,
			      20 - (3 + opt_text_len), FALSE);
	  else
	    send_to_help_buf (spaces, 20, FALSE);

	  strcpy_to_wrap_buf (opt->desc);
	  for (buf = wrap_buf;; buf += next_len)
	    {	   
	      wrap (buf, 55, &out_len, &next_len);
	      send_to_help_buf (buf, out_len, TRUE);
	      if (next_len != -1)
		send_to_help_buf (spaces, 20, FALSE);
	      else
		break;
	    }
	}
    }
}

char *
Executor::opt_help_message (void)
{
  if (help_buf == NULL)
    _generate_help_message ();
  return help_buf;
}

void
Executor::opt_register_pre_note (char *note)
{
  if (!pre_notes)
    {
      n_pre_notes = 0;
      max_pre_notes = 4;
      pre_notes = (char**)malloc (max_pre_notes * sizeof *pre_notes);
    }
  if (n_pre_notes == max_pre_notes)
    {
      max_pre_notes *= 2;
      pre_notes = (char**)realloc (pre_notes, max_pre_notes * sizeof *pre_notes);
    }
  pre_notes[n_pre_notes ++] = note;
}


#if !defined (MSDOS)
# define ERRMSG_STREAM stderr
#else
# define ERRMSG_STREAM stdout  /* stderr is annoying to DOS users. */
#endif


void
Executor::opt_register (char *new_interface,
	      option_t *new_opts, int n_new_opts)
{
  int block_i;
  struct opt_block *block;

  if (help_buf)
    {
      /* internal error, must register all options before generating
	 help message */
      fprintf (ERRMSG_STREAM, "\
%s: internal options error: opt register after help message generation.\n",
	       program_name);
      exit (-16);
    }
  
  /* check for conflicting options */
  for (block_i = 0; block_i < n_opt_blocks; block_i ++)
    {
      char *interface = opt_blocks[block_i].interface;
      option_t *opts = opt_blocks[block_i].opts;
      int n_opts = opt_blocks[block_i].n_opts;
      int opt_i, new_opt_i;
      
      for (opt_i = 0; opt_i < n_opts; opt_i ++)
	for (new_opt_i = 0; new_opt_i < n_new_opts; new_opt_i ++)
	  {
	    if (!strcmp (opts[opt_i].text,
			 new_opts[new_opt_i].text))
	      {
		/* conflicting options */
		fprintf (ERRMSG_STREAM, "\
%s: opt internal error: `%s' and `%s' both request option `%s'\n",
			 program_name,
			 interface, new_interface, opts[opt_i].text);
		exit (-16);
	      }
	  }
    }
  
  if (!opt_blocks)
    {
      n_opt_blocks = 0;
      n_opt_blocks_max = 4;
      opt_blocks = (struct opt_block*)malloc (n_opt_blocks_max * sizeof *opt_blocks);
    }
  else if (n_opt_blocks == n_opt_blocks_max)
    {
      n_opt_blocks_max *= 2;
      opt_blocks = (struct opt_block*)realloc (opt_blocks, n_opt_blocks_max * sizeof *opt_blocks);
    }
  
  block = &opt_blocks[n_opt_blocks ++];
  
  block->interface = new_interface;
  block->opts = new_opts;
  block->n_opts = n_new_opts;
}

opt_database_t *
Executor::opt_alloc_db (void)
{
  opt_database_t *retval = (opt_database_t*)malloc(sizeof *retval);

  retval->opt_vals = NULL;
  retval->n_opt_vals = 0;
  /* default maximum */
  retval->max_opt_vals = 4;
  
  return retval;
}

PRIVATE opt_val_t *
opt_lookup_helper (opt_database_t *db, char *opt)
{
  opt_val_t *retval;
  int i;  

  retval = 0;

  /* try to find this option in the database */
  for (i = 0; i < db->n_opt_vals; i ++)
    {
      if (strcmp (db->opt_vals[i].text, opt) == 0)
	{
	  retval = &db->opt_vals[i];
	  break;
	}
    }
  return retval;
}

opt_val_t *
opt_lookup (opt_database_t *db, char *opt)
{
  opt_val_t *retval;

  retval = opt_lookup_helper (db, opt);

  if (!retval || !retval->t_val)
    {
      NSUserDefaults *defaults;
      NSString *try1;

      defaults = [NSUserDefaults standardUserDefaults];
      try1 = [defaults stringForKey:@(opt)];

      if (try1) {
	  if (retval) {
	      char *str;
	      int len;

	      len = [try1 lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
	      str = (char*)malloc (len+1);
	      [try1 getCString:str maxLength:len encoding:NSUTF8StringEncoding];
	      retval->val = str;
	    }
	  else
	    {
	      opt_put_val (db, opt, [try1 UTF8String], pri_dwrite, FALSE);
	      retval = opt_lookup_helper (db, opt);
	    }
	}
    }

  return retval;
}

void
Executor::opt_put_val (opt_database_t *db, char *opt, const char *val,
	     priority_t pri, int temp_val_p)
{
  opt_val_t *opt_val;

  opt_val = opt_lookup_helper (db, opt);
  if (!opt_val)
    {
      if (db->opt_vals)
	{
	  /* this option is not yet in the database, add it */
	  if (db->n_opt_vals == db->max_opt_vals)
	    {
	      /* allocate some new ones */
	      db->max_opt_vals *= 2;
	      db->opt_vals = (opt_val_t*)realloc (db->opt_vals,
				      (sizeof *(db->opt_vals)
				       * db->max_opt_vals));
	    }
	}
      else
	{
	  db->opt_vals = (opt_val_t*)malloc (sizeof *(db->opt_vals) * db->max_opt_vals);
	}
      opt_val = &db->opt_vals[db->n_opt_vals ++];
      
      opt_val->text = opt;
      opt_val->val = NULL;
      opt_val->t_val = NULL;
    }

  if (temp_val_p)
    {
      opt_val->t_val = (char*)val;
      opt_val->t_pri = pri;
    }
  else
    {
      opt_val->val = val;
      opt_val->pri = pri;
    }
}

void
Executor::opt_put_int_val (opt_database_t *db, char *opt, int valint,
		 priority_t pri, int temp_val_p)
{
  char *val, buf[256];
  
  sprintf (buf, "%d", valint);
  val = (char*)malloc (strlen (buf) + 1);
  strcpy (val, buf);
  
  opt_put_val (db, opt, val, pri, temp_val_p);
}

#define option_value(opt_val) ((opt_val)->t_val ?: (opt_val)->val)

int
Executor::opt_val (opt_database_t *db, char *opt, const char **retval)
{
  opt_val_t *opt_val;
  const char *val = NULL;
  boolean_t found_p = FALSE;

  opt_val = opt_lookup (db, opt);
  if (opt_val)
    {
      val = option_value (opt_val);
      if (val)
	{
	  if (retval)
	    *retval = val;
	  found_p = TRUE;
	}
    }
  return found_p;
}


/* Parses an integer value and returns it in *retval.  If
 * parse_error_p is non-NULL, sets parse_error_p to TRUE and prints an
 * error message in case of a parse error, else leaves it untouched.
 * Returns TRUE if a value was found.
 */
int
Executor::opt_int_val (opt_database_t *db, char *opt, int *retval,
	     boolean_t *parse_error_p)
{
  opt_val_t *opt_val;
  const char *val = NULL;

  opt_val = opt_lookup (db, opt);
  if (opt_val && (val = option_value (opt_val)) && retval)
    {
      int32 v;
      if (!parse_number (val, &v, 1))
	{
	  if (parse_error_p)
	    {
	      fprintf (stderr, "Malformed numeric argument to -%s.\n", opt);
	      *parse_error_p = TRUE;
	    }
	  return FALSE;
	}
      *retval = v;
    }

  /* Do *NOT* touch *parse_error_p if there is no error. */

  return opt_val && val;
}

int
Executor::opt_parse (opt_database_t *db, option_t *opts, int n_opts,
	   int *argc, char *argv[])
{
  int parse_error_p = FALSE;
  int i;
  int argc_left;

  /* we are sure to copy the null termination because we start with
     `i = 1' */
  argc_left = *argc;

  /* skip over the program name */
  for (i = 1; i < *argc; i ++, argc_left --)
    {
      char *arg;
      int arg_i;

      arg_i = i;
      arg = argv[i];
      /* option */
      if (*arg == '-')
	{
	  int opt_i;
	  
	  /* find the option among the options */
	  for (opt_i = 0; opt_i < n_opts; opt_i ++)
	    {
	      option_t *opt = &opts[opt_i];

	      if (!strcmp (&arg[1],
			   opt->text))
		{
		  const char *optval = NULL;
		  
		  /* found the option */
		  switch (opt->kind)
		    {
		    case opt_no_arg:
		      optval = "1";
		      break;
		    case opt_optional:
		      if ((i + 1) < *argc && *argv[i + 1] != '-')
			{
			  optval = argv[++ i];
			  argc_left --;
			}
		      else
			optval = opt->opt_val;
		      break;
		    case opt_sticky:
		      optval = &arg[1 + strlen (opt->text)];
		      break;
		    case opt_sep:
		      if ((i + 1) < *argc)
			{
			  optval = argv[++ i];
			  argc_left --;
			}
		      else
			{
			  fprintf (ERRMSG_STREAM, "\
%s: option `-%s' requires argument\n",
				   program_name, opt->text);
			  parse_error_p = TRUE;
			}
		      break;
		    case opt_ignore:
		      goto next_arg;
		    case opt_sep_ignore:
		      if ((i + 1) < *argc)
			{
			  i ++;
			  argc_left --;
			}
		      else
			{
			  fprintf (ERRMSG_STREAM, "\
%s: option `-%s' requires argument\n",
				   program_name, opt->text);
			  parse_error_p = TRUE;
			}
		      goto next_arg;
		    }
		  *argc -= (i + 1) - arg_i;
		  memmove (&argv[arg_i], &argv[i + 1],
			   (argc_left-1) * sizeof *argv);
		  i = arg_i - 1;
		  opt_put_val (db, opt->text, optval, pri_command_line,
			       FALSE);
		}
	    }
	}
    next_arg:;
    }
  return parse_error_p;
}
