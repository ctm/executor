%{
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned char uint8;

#include "rsys/custom.h"

typedef struct
{
  uint32 length;
  uint8 bytes[0];
}
byte_sequence_t;

static int yyparse (void);
static long yylex (void);
static void output (uint32 tag, const byte_sequence_t *bp);
static byte_sequence_t *bptr_from_file_contents (const char *filename);
static byte_sequence_t *bptr_from_uint32 (uint32 i);
static byte_sequence_t *bptr_from_bptr_and_uint32 (const byte_sequence_t *bp,
						   uint32 i);
static byte_sequence_t *bptr_from_bptr_and_string (const byte_sequence_t *bp,
						   const char *str);
static byte_sequence_t *bptr_from_string (const char *str);

%}

%start configuration

%union
{
  uint32 number;
  char *cptr;
  byte_sequence_t *bptr;
};

%token	STRING	/* ptr points to null-terminated string */
%token	HEXINT	/* number */
%token	INTEGER	/* number */
%token	CHAR4INT
%token  VARNAME

%type <cptr> STRING
%type <number> HEXINT INTEGER CHAR4INT VARNAME number
%type <bptr> filename numberlist bptr stringlist
%%

configuration: /* empty */
	| assignments
	;

assignment: VARNAME '=' bptr ';' { output ($1, $3); };

assignments: assignment
	| assignments assignment
	;

filename: STRING { $$ = bptr_from_file_contents ($1); free ($1); };

bptr: filename
	| '{' numberlist '}' { $$ = $2; }
	| '{' numberlist ',' '}' { $$ = $2; }
	| number { $$ = bptr_from_uint32 ($1); }
	| '{' stringlist '}' { $$ = bptr_from_bptr_and_string ($2, ""); }
	| '{' stringlist ',' '}' { $$ = bptr_from_bptr_and_string ($2, ""); }
        ; 

number:	HEXINT
	| INTEGER
	| '-' INTEGER { $$ = - $2; }
        | CHAR4INT
	;

numberlist: numberlist ',' number { $$ = bptr_from_bptr_and_uint32 ($1, $3); }
	| number { $$ = bptr_from_uint32 ($1); }
	;

stringlist: stringlist ',' STRING { $$ = bptr_from_bptr_and_string ($1, $3);
                                    free ($3); }
	| STRING { $$ = bptr_from_string ($1); free ($1); }
	;

%%
#include	<stdio.h>
#include	<ctype.h>

FILE *exec_fp;

struct namevaluestr
{
    char *name;
} reserved[] = 
{
  { CUSTOM_CREATORS },
  { CUSTOM_LICENSE },
  { CUSTOM_FIRST_SN },
  { CUSTOM_CHECKSUM },
  { CUSTOM_LAST_SN },
  { CUSTOM_SPLASH },
  { CUSTOM_ABOUT_BOX },
  { CUSTOM_COPYRIGHT_INFO },
  { CUSTOM_THANK_YOU_INFO },
  { CUSTOM_REGISTRATION_INSTRUCTIONS },
  { CUSTOM_MAGIC_VOLUMES },
  { CUSTOM_MUST_REGISTER },
  { CUSTOM_MAC_CDROM },
  { CUSTOM_DEMO_IDENTIFIER },
  { CUSTOM_DISABLE_COMMAND_KEYS },
  { CUSTOM_RESTART_STRING },
  { CUSTOM_MENU_ABOUT_STRING },
  { CUSTOM_VERSION_STRING },
  { CUSTOM_SUFFIX_MAPS },
  { CUSTOM_DEFAULT_APP },
  { CUSTOM_DEMO_DAYS },
};

static int
reserved_cmp (const void *p1, const void *p2)
{
  int retval;

  retval = strcmp (*(const char **) p1, *(const char **) p2);
  return retval;
}

typedef enum
{
  false,
  true,
}
boolean_t;

#if !defined (NELEM)
#define NELEM(x) (sizeof (x) / sizeof (x)[0])
#endif

#define	RELOAD	(-2)

static long
binfind(char *tofind, struct namevaluestr table[], short nentries)
{
  short low, mid, high;
  int cmpret;
  static char already_sorted = false;

  if (!already_sorted)
    {
      qsort (reserved, NELEM (reserved), sizeof reserved[0], reserved_cmp);
      already_sorted = true;
    }
  for (low = -1, high = nentries; high - low > 1;)
    {
      mid = (high + low) / 2;
      cmpret = strcmp(tofind, table[mid].name);
      if (cmpret < 0)
	high = mid;
      else if (cmpret == 0)
	{
	  yylval.number = *(uint32 *)table[mid].name;
/*-->*/	    return VARNAME;
	}
      else /* cmpret > 0 */
	low = mid;
    }
  return 0;
}

static long linecount = 1;

#define GET_REMAINING_DEFINE(func, test)                                \
static int								\
get_remaining_ ## func (char **tokenp, int c)				\
{									\
  int retval;								\
  char *buf, *old_buf, *p;						\
  int bufsize, old_bufsize;						\
  int chars_left;							\
									\
  bufsize = 32;								\
  old_bufsize = 0;							\
  old_buf = NULL; /* to get rid of compiler warnings */			\
  do									\
    {									\
      buf = alloca (bufsize);						\
      memcpy (buf, old_buf, old_bufsize);				\
      chars_left = bufsize - old_bufsize;				\
      p = buf + old_bufsize;						\
      while (c != EOF && chars_left > 0 && (test))			\
	{								\
	  *p++ = c;							\
	  --chars_left;							\
	  c = getchar ();						\
	}								\
      old_buf = buf;							\
      old_bufsize = bufsize;						\
      bufsize *= 2;							\
    }									\
  while (!chars_left);							\
  *p = 0;								\
  *tokenp = strdup (buf);						\
  retval = c;								\
  return retval;							\
}

GET_REMAINING_DEFINE(token, (isalnum (c) || c == '_'))
GET_REMAINING_DEFINE(string, ((c == '\\' ? ((c = getchar ()),1) : 0) || \
			      c != '"'))

#define warning_unexpected(fmt, args...) fprintf (stderr, fmt "\n" , ## args)

static void
cleanup_linefeeds (char *p)
{
  char *orig_p;

  orig_p = p;

  while (*p)
    {
      if (*p == '\n')
	{
	  if (p > orig_p && p[-1] == '\r')
	    memmove (p, p+1, strlen (p));
	  else
	    {
	      *p = '\r';
	      orig_p = p+1;
	    }
	}
      ++p;
    }
}

static long
yylex (void)
{
  static short c = RELOAD;
  long retval;
  int i;

  if (c == RELOAD)
    c = getchar();

  while (true)
    {
      switch (c)
	{
	case '"':
	  c = getchar (); /* consume leading " */
	  c = get_remaining_string (&yylval.cptr, c);
	  cleanup_linefeeds (yylval.cptr);
	  c = getchar (); /* consume trailing " */
	  return STRING;
	  break;

	case '0':
	  if ((c = getchar()) == 'x')
	    {
	      yylval.number = 0;
	      while (isalnum (c = getchar ()))
		{
		  if (yylval.number & 0xF0000000)
		    warning_unexpected ("hex constant too long parsing "
					"configuration file (line %ld)",
					linecount);
		  if (c >= '0' && c <= '9')
		    yylval.number = (yylval.number << 4) + c - '0';
		  else if (c >= 'a' && c <= 'f')
		    yylval.number = (yylval.number << 4) + c - 'a' + 10;
		  else if (c >= 'A' && c <= 'F')
		    yylval.number = (yylval.number << 4) + c - 'A' + 10;
		  else
		    warning_unexpected ("bad character in hex constant "
					"parsing configuration file "
					"(line %ld)", linecount);
		}
	      return HEXINT;
	      break;
	    }
	  /* We just ate a 0 that should be part of a decimal integer
	     (we don't support octal).  It is a leading zero, so we can
	     forget about it and fall through */
	  ungetc(c, stdin);
	  c = '0';
	  /* FALL THROUGH */
	case '1': case '2': case '3':
	case '4': case '5': case '6':
	case '7': case '8': case '9':
	  yylval.number = c - '0';
	  while (isdigit(c = getchar ()))
	    {
	      yylval.number = yylval.number * 10 + c - '0';
	      if (yylval.number < 0)
		warning_unexpected ("integer overflow parsing configuration "
				    "file (line %ld)", linecount);
	    }
	  return INTEGER;
	  break;
	case ' ':
	case '\r':
	case '\n':
	case '\t':
	case '\f':
	  do
	    if (c == '\n')
	      linecount++;
	  while (isspace(c = getchar ()));
	  break;
	case '/':
	  if ((c = getchar ()) == '/')
	    {
	      while ((c = getchar ()) != '\n')
		;
	      linecount++;
	      c = getchar ();
	    }
	  else
	    warning_unexpected ("single '/' parsing configuration file "
				"(line %ld)", linecount);
	  break;
	case '\'':
	  yylval.number = 0;
	  for (i = 0 ; i < 4 ; i++)
	    {
	      c = getchar ();
	      yylval.number = (unsigned long)yylval.number * 256 + c;
	    }
	  if ((c = getchar ()) != '\'')
	    warning_unexpected ("missing \' parsing configuration file (line "
				"%ld)", linecount);
	  c = getchar ();
	  return CHAR4INT;
	  break;
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
	case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
	case 's': case 't': case 'u': case 'v': case 'w': case 'x':
	case 'y': case 'z':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	case 'Y': case 'Z':
	  {
	    char *token;

	    c = get_remaining_token (&token, c);
	    retval = binfind(token, reserved, NELEM (reserved));
	    if (!retval)
	      warning_unexpected ("unknown reserved word (\"%s\") parsing "
				  "configuration file (line %ld)", token,
				  linecount);
	    free (token);
	    return retval;
	  }
	  break;
	case '{':
	case '}':
	case '(':
	case ')':
	case '=':
	case ':':
	case ',':
	case ';':
	case '-':
	case '.':
	  retval = c;
	  c = getchar ();
	  return retval;
	  break;
	case EOF:
	  c = RELOAD;	/* for the next time around */
	  linecount = 1;
	  return EOF;
	  break;
	default:
	  warning_unexpected ("unknown character parsing configuration file "
			      "(line %ld)", linecount);
	}
    }
}

void
yyerror(char *str)
{
  warning_unexpected ("configuration file parse error (line %ld): %s",
		      linecount, str);
}

static void
output (uint32 tag, const byte_sequence_t *bp)
{
  if (fwrite (&tag, sizeof tag, 1, exec_fp)               != 1 ||
      fwrite (&bp->length, sizeof bp->length, 1, exec_fp) != 1 ||
      fwrite (bp->bytes, bp->length, 1, exec_fp)          != 1)
    warning_unexpected ("Trouble writing\n");
  free ((byte_sequence_t *) bp);
}

static byte_sequence_t *
bptr_from_file_contents (const char *filename)
{
  byte_sequence_t *retval;
  FILE *fp;

  retval = 0;
  fp = fopen (filename, "r");
  if (!fp)
    warning_unexpected ("Couldn't open \"%s\"\n", filename);
  else
    {
      struct stat sbuf;

      if (fstat (fileno (fp), &sbuf) != 0)
	warning_unexpected ("Couldn't fstat \"%s\"\n", filename);
      else
	{
	  retval = malloc (sizeof *retval + sbuf.st_size);
	  if (retval)
	    {
	      int n;

	      retval->length = sbuf.st_size;
	      n = fread (retval->bytes, retval->length, 1, fp);
	      if (n != 1)
		warning_unexpected ("Trouble reading\n");
	    }
	}
      fclose (fp);
    }
  return retval;
}

static byte_sequence_t *
bptr_from_uint32 (uint32 i)
{
  byte_sequence_t *retval;

  retval = malloc (sizeof *retval + sizeof i);
  if (retval)
    {
      retval->length = sizeof i;
      memcpy (retval->bytes, &i, sizeof i);
    }
  return retval;
}

static byte_sequence_t *
bptr_from_bptr_and_uint32 (const byte_sequence_t *bp, uint32 i)
{
  byte_sequence_t *retval;

  retval = malloc (sizeof *retval + bp->length + sizeof i);
  if (retval)
    {
      retval->length = bp->length + sizeof i;
      memcpy (retval->bytes, bp->bytes, bp->length);
      memcpy (retval->bytes + bp->length, &i, sizeof i);
    }
  free ((byte_sequence_t *) bp);
  return retval;
}

static byte_sequence_t *
bptr_from_bptr_and_string (const byte_sequence_t *bp, const char *str)
{
  byte_sequence_t *retval;
  int len;

  len = strlen (str) + 1;

  retval = malloc (sizeof *retval + bp->length + len);
  if (retval)
    {
      retval->length = bp->length + len;
      memcpy (retval->bytes, bp->bytes, bp->length);
      memcpy (retval->bytes + bp->length, str, len);
    }
  free ((byte_sequence_t *) bp);
  return retval;
}

static byte_sequence_t *
bptr_from_string (const char *str)
{
  byte_sequence_t *retval;
  int len;

  len = strlen (str) + 1;

  retval = malloc (sizeof *retval + len);
  if (retval)
    {
      retval->length = len;
      memcpy (retval->bytes, str, len);
    }
  return retval;
}

static boolean_t
position_after_magic (FILE *fp, uint64 magic)
{
  uint64 test;
  long offset;
  boolean_t found;
  boolean_t retval;
  
  for (offset = 0, found = false;
       (!found && fseek (fp, offset, SEEK_SET) == 0 &&
	fread (&test, sizeof test, 1, fp) == 1);
       ++offset)
    {
      if (test == magic)
	found = true;
    }
  if (!found)
    retval = false;
  else
    {
      test = 0;
      retval = (fseek (fp, offset-1, SEEK_SET) == 0 &&
		fwrite (&test, sizeof test, 1, fp) == 1);
    }
  return retval;
}

int
main (int argc, const char *argv[])
{
  int retval;

  if (argc != 3)
    {
      fprintf (stderr, "Usage: customize executor.exe config_file\n");
      exit (1);
    }
  
  exec_fp = fopen (argv[1], "r+b");
  if (!exec_fp)
    {
      fprintf (stderr, "Couldn't open \"%s\" for writing\n", argv[1]);
      exit (1);
    }
  if (!position_after_magic (exec_fp, CUSTOM_MAGIC))
    {
      fprintf (stderr, "Couldn't find magic\n");
      exit (1);
    }
  if (freopen (argv[2], "r", stdin) == NULL)
    {
      fprintf (stderr, "Couldn't open \"%s\" for reading\n", argv[2]);
      exit (1);
    }
  retval = yyparse ();
  fclose (exec_fp);
  return retval;
}
