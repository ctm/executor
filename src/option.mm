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
using namespace std;

typedef struct opt_block
{
  string interface;
  option_vec opts;
} opt_block;
typedef vector<opt_block> optBlocks;
static optBlocks opt_blocks;

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
  
  opt_blocks.clear();
}

static void
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

static inline void
strcpy_to_wrap_buf (string text)
{
  char *textP = (char *)alloca(text.length() + 10);
  strcpy(textP, text.c_str());
  strcpy_to_wrap_buf(textP);
}

static void
wrap (char *buf,
      int desired_len,
      int *out_len, int *next_len)
{
  int buf_len;

  buf_len = strlen (buf);
  
  if (buf_len <= desired_len) {
    *out_len = buf_len;
    *next_len = -1;
  } else {
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
send_to_help_buf (string text, int append_newline_p)
{
  send_to_help_buf(text.c_str(), text.length(), append_newline_p);
}

void
_generate_help_message (void)
{
  int i;
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
  for (optBlocks::iterator block_i = opt_blocks.begin(); block_i != opt_blocks.end(); block_i ++)
    {
      string interface = block_i->interface;
      option_vec *opts = &block_i->opts;

      send_to_help_buf (interface, FALSE);
      send_to_help_buf (":", 1, TRUE);
      
      for (option_vec::iterator opt = opts->begin(); opt != opts->end(); opt ++)
	{
	  const char *spaces = "                    ";
	  int next_len, out_len;
	  
	  int opt_text_len;
	  int same_line_p;

	  if (opt->desc == "")
	    continue;
	  
	  opt_text_len = opt->text.length();
	  same_line_p = opt_text_len < 14;
	  
	  send_to_help_buf ("  -", 3, FALSE);
	  send_to_help_buf (opt->text,
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

void Executor::opt_register_pre_note (string note)
{
  char *aPreNote = strdup(note.c_str());
  
  opt_register_pre_note(aPreNote);
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
Executor::opt_register (string new_interface,
	      option_vec new_opts)
{
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
  for (optBlocks::iterator blockIt = opt_blocks.begin(); blockIt != opt_blocks.end(); blockIt++)
  {
    string interface = blockIt->interface;
    option_vec *opts = &blockIt->opts;

    for (option_vec::iterator opt_i = opts->begin(); opt_i != opts->end(); opt_i ++)
      for (option_vec::iterator new_opt_i = new_opts.begin(); new_opt_i != new_opts.end(); new_opt_i ++)
      {
        if ( (opt_i->text ==
              new_opt_i->text))
        {
          /* conflicting options */
          fprintf (ERRMSG_STREAM, "\
                   %s: opt internal error: `%s' and `%s' both request option `%s'\n",
                   program_name,
                   interface.c_str(), new_interface.c_str(), opt_i->text.c_str());
          exit (-16);
        }
      }
  }
  
  block = &opt_blocks.back();
  
  block->interface = new_interface;
  block->opts = new_opts;
}

opt_database_t
Executor::opt_alloc_db (void)
{
  opt_database_t retval = opt_database_t() ;

  return retval;
}

PRIVATE opt_val_t *
opt_lookup_helper (opt_database_t &db, string &opt)
{
  opt_val_t *retval = nullptr;

  /* try to find this option in the database */
  for (opt_database_t::iterator i = db.begin() ; i != db.end(); i ++)
    {
      if (i->text == opt)
	{
	  retval = &*i;
	  break;
	}
    }
  return retval;
}

opt_val_t *
opt_lookup (opt_database_t &db, string &opt)
{
  opt_val_t *retval;

  retval = opt_lookup_helper (db, opt);

  if (!retval || retval->t_val == "")
    {
      NSUserDefaults *defaults;
      NSString *try1;

      defaults = [NSUserDefaults standardUserDefaults];
      try1 = [defaults stringForKey:@(opt.c_str())];

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
Executor::opt_put_val (opt_database_t &db, string &opt, string val,
	     priority_t pri, int temp_val_p)
{
  opt_val_t *opt_val = opt_lookup_helper (db, opt);

  if (!opt_val)
  {
    opt_val_t tmpVal;
    tmpVal.text = opt;
    tmpVal.val = "";
    tmpVal.t_val = "";
    db.push_back(tmpVal);
    opt_val = &db.back();
    
  }

  if (temp_val_p)
  {
    opt_val->t_val = val;
    opt_val->t_pri = pri;
  }
  else
  {
    opt_val->val = val;
    opt_val->pri = pri;
  }
}

void
Executor::opt_put_int_val (opt_database_t &db, string &opt, int valint,
		 priority_t pri, int temp_val_p)
{
  char *val, buf[256];
  
  sprintf (buf, "%d", valint);
  val = (char*)malloc (strlen (buf) + 1);
  strcpy (val, buf);
  
  opt_put_val (db, opt, val, pri, temp_val_p);
}

#define option_value(opt_val) ((opt_val)->t_val == "" ? "" : (opt_val)->val)

int
Executor::opt_val (opt_database_t &db, string opt, string *retval)
{
  opt_val_t *opt_val;
  string val = "";
  boolean_t found_p = FALSE;

  opt_val = opt_lookup (db, opt);
  if (opt_val)
    {
      val = option_value (opt_val);
      if (val != "")
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
Executor::opt_int_val (opt_database_t &db, string opt, int *retval,
	     boolean_t *parse_error_p)
{
  opt_val_t *opt_val;
  string val = "";

  opt_val = opt_lookup (db, opt);
  if (opt_val && (val = option_value (opt_val)) != "" && retval)
    {
      int32 v;
      if (!parse_number (val, &v, 1))
	{
	  if (parse_error_p)
	    {
	      fprintf (stderr, "Malformed numeric argument to -%s.\n", opt.c_str());
	      *parse_error_p = TRUE;
	    }
	  return FALSE;
	}
      *retval = v;
    }

  /* Do *NOT* touch *parse_error_p if there is no error. */

  return opt_val && val != "";
}

int
Executor::opt_parse (opt_database_t &db, option_vec opts,
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
	  /* find the option among the options */
      for (option_vec::iterator opt_i = opts.begin(); opt_i != opts.end(); opt_i ++)
	    {
	      option_t *opt = &*opt_i;

	      if (!strcmp (&arg[1],
			   opt->text.c_str()))
		{
          string optval = "";
		  
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
		      optval = &arg[1 + opt->text.length()];
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
				   program_name, opt->text.c_str());
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
				   program_name, opt->text.c_str());
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
