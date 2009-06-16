%{
#include "rsys/common.h"
#include "rsys/options.h"
#include "rsys/prefs.h"
#include "rsys/crc.h"
#include "rsys/parse.h"
#include "rsys/parseopt.h"
#include "rsys/gestalt.h"
#include "rsys/launch.h"
#include "rsys/print.h"
#include "rsys/version.h"

/* Copyright 1992 - 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_parse[] =
	    "$Id: parse.y,v 2.26 2000/02/12 05:31:08 ctm Exp $";
#endif

static long yylex( void );
static void freestring( char *string );
char validblock = 1;
char *ROMlib_win32_token = 0;
char *ROMlib_WindowName = 0;
char *ROMlib_DongleFamily = 0;
char *ROMlib_Comments = 0;
char *ROMlib_new_printer_name = 0;
char *ROMlib_new_label = 0;
static char *ROMlib_mac_cdrom;
pair_t ROMlib_ScreenSize = { INITIALPAIRVALUE, INITIALPAIRVALUE };
pair_t ROMlib_MacSize = { INITIALPAIRVALUE, INITIALPAIRVALUE };
pair_t ROMlib_ScreenLocation = { INITIALPAIRVALUE, INITIALPAIRVALUE };
unsigned short crcval;
int ROMlib_AppleChar = 0;
int ROMlib_right_button_modifier = 0;

void yyerror(const char *str);
%}

%start configuration

%union	{ long number; char *cptr; char **cptrptr; pair_t *pptr; int *iptr;
	  unsigned long rtype;};

%token	STRINGCONSTANT	/* ptr points to null-terminated string */
%token	HEXCONSTANT	/* number */
%token	INTEGERCONSTANT	/* number */
%token	CHAR4CONSTANT	
%token	OPTIONSCONSTANT	/* number */
%token	WINDOWNAME	/* ptr points to null-terminated string */
%token	DONGLEFAMILY	/* ptr points to null-terminated string */
%token	SYSTEMVERSION
%token	COMMENTS
%token	RDELAY
%token	SCREENSIZE
%token	MACSIZE
%token	SCREENLOCATION
%token	REFRESHNUMBER
%token	CRC
%token	OPTIONS
%token	CACHEPROBLEMS
%token	BITSNUMBER
%token  APPLECHAR
%token  PRINTERNAME
%token  PRINTERTYPELABEL
%token  WIN32TOKEN
%token  MACCDROM
%token  RIGHT_BUTTON_MODIFIER
%token  GESTALT
%token  VERSION
%token	PRVERSION

%type <cptr> STRINGCONSTANT
%type <number> HEXCONSTANT INTEGERCONSTANT OPTIONSCONSTANT
%type <rtype> CHAR4CONSTANT
%type <cptrptr> stringvar
%type <iptr>	optionsvar
%type <iptr>	numervar
%type <number> number optionvalues
%type <pptr> pairvar
%type <number> blockbegin	/* holds previous validblock */
%type <number> gestalt_entry

%%

configuration: /* empty */
	| assignmentsandblocks
	;

assignmentsandblocks: assignment
	| block
	| assignmentsandblocks assignment
	| assignmentsandblocks block
	;

assignment:	stringassignment
	| arrayassignment
	| optionsassignment
	| numerassignment
	| sysversassignment
	| prversassignment
	;

block:	blockbegin '{' assignments '}' ';'
	{
	    validblock = $1;
	}
	;

assignments: assignment
	| assignments assignment
	;

blockbegin:	CRC '(' CHAR4CONSTANT ',' INTEGERCONSTANT ')' ':' HEXCONSTANT
		{
		    $$ = validblock;
		    crcval = getthecrc($3, $5);
		    validblock = $8 == crcval;
		    if (ROMlib_options & ROMLIB_DEBUG_BIT)
			printf("CRC('%c%c%c%c', %d) = 0x%04lx\n", (int) $3>>24,
		 (int) $3>>16, (int) $3>>8, (int) $3, (int) $5, (long) crcval);
		}
		| VERSION '(' HEXCONSTANT ')' ':' HEXCONSTANT
		{
		    $$ = validblock;
		    validblock = (ROMlib_version_long & $3) == $6;
		}
		;

stringassignment:	stringvar '=' STRINGCONSTANT ';'
			{
			    if (validblock)
			      {
				if (*$1)
				  free (*$1);
				*$1 = strdup ($3);
				freestring ($3);
			      }
			    else
				freestring($3);
			}
	;

sysversassignment:	SYSTEMVERSION '=' number ';'
			{ 
			  if (validblock)
			    ROMlib_set_system_version
			      (CREATE_SYSTEM_VERSION ($3, 0, 0));
			}
			|
			SYSTEMVERSION '=' number '.' number ';'
			{
			  if (validblock) 
			    ROMlib_set_system_version
			      (CREATE_SYSTEM_VERSION ($3, $5, 0));
			}
			|
			SYSTEMVERSION '=' number '.' number '.' number ';'
			{
			  if (validblock)
			    ROMlib_set_system_version
			      (CREATE_SYSTEM_VERSION ($3, $5, $7));
			}
	;


prversassignment:	PRVERSION '=' number ';'
			{ 
			  if (validblock)
			    ROMlib_PrDrvrVers = $3 * 10;
			}
			|
			PRVERSION '=' number '.' number ';' 
			{
			  if (validblock)
			    ROMlib_PrDrvrVers = $3 * 10 + $5;
			}
	;


numerassignment:	numervar '=' number ';'
			{
			    if (validblock)
				*$1 = $3;
			}
	;

stringvar:	WINDOWNAME
	{ $$ = &ROMlib_WindowName; }
	| COMMENTS
	{ $$ = &ROMlib_Comments; }
	| DONGLEFAMILY
	{ $$ = &ROMlib_DongleFamily; }
	| PRINTERNAME
	{ $$ = &ROMlib_new_printer_name; }
	| PRINTERTYPELABEL
	{ $$ = &ROMlib_new_label; }
	| WIN32TOKEN
	{ $$ = &ROMlib_win32_token; }
	| MACCDROM
	{ $$ = &ROMlib_mac_cdrom;
          warning_unexpected ("MacCDROM no longer supported "
			      "in configuration files"); }
	;

numervar:	RDELAY
	{ $$ = &ROMlib_delay; }
	| REFRESHNUMBER
	{ $$ = &ROMlib_refresh; }
	| BITSNUMBER
	{ $$ = &ROMlib_desired_bpp; }
	| APPLECHAR
	{ $$ = &ROMlib_AppleChar; }
        | RIGHT_BUTTON_MODIFIER
	{ $$ = &ROMlib_right_button_modifier; }
	;

optionvalues: optionvalues ',' OPTIONSCONSTANT 
	{ $$ = $1 | $3; }
	| OPTIONSCONSTANT
	{ $$ = $1; }
	;

optionsvar:	OPTIONS
	{ 
	  if (validblock)
	    {
	      $$ = &ROMlib_options;
	      ROMlib_options = 0;
	    }
	}
	;

optionsassignment:	optionsvar '=' '{' optionvalues '}' ';'
	{ if (validblock)
	      *$1 = $4;
	}
	| optionsvar '=' '{' '}' ';'
	{
	  if (validblock)
	    *$1 = 0;
	}
	| optionsvar '=' '{' optionvalues ',' '}' ';'
	{
	  if (validblock)
	    *$1 = $4;
	}
	;

arrayassignment:	pairassignment
	| unboundedassignment
        | gestaltassignment
	;

pairassignment:	pairvar '=' '{' number ',' number '}' ';'
	{
	  if (validblock)
	    {
	      $1->first = $4;
	      $1->second = $6;
	    }
	}
	;

gestaltassignment:	gestalt_entry '=' '{' number ',' number '}' ';'
	{ if (validblock)
	    ROMlib_add_to_gestalt_list ($1, $4, $6);
	}
	;

number:	HEXCONSTANT
	{ $$ = $1; }
	| INTEGERCONSTANT
	{ $$ = $1; }
	| '-' number
	{ $$ = - $2; }
	;

gestalt_entry:	GESTALT '(' CHAR4CONSTANT ')'
	{ $$ = $3; }
	| GESTALT '(' HEXCONSTANT ')'
	{ $$ = $3; }
	;

pairvar:	SCREENSIZE
	{ $$ = &ROMlib_ScreenSize; }
	| SCREENLOCATION
	{ $$ = &ROMlib_ScreenLocation; }
	| MACSIZE
	{ $$ = &ROMlib_MacSize; }
	;

unboundedassignment:	unboundedvar '=' '{' numberlist '}' ';'
	| unboundedvar '=' '{' numberlist ',' '}' ';'
	| unboundedvar '=' '{' '}' ';'
	;

unboundedvar:	CACHEPROBLEMS
	;

/* NOTE: numberlist is expected only to be used for CacheProblems
	 so the reduction directly calls a cache specific routine */
numberlist: numberlist ',' number
	{ if (validblock)
	    /* addtocache($3); */{};
        }
	| number
	{ if (validblock)
	    /* addtocache($1); */{};
	}
	;

%%
#include	<stdio.h>
#include	<ctype.h>

struct namevaluestr {
    char *name;
    long value;
    long lval;
} reserved[] = 
{	/* NOTE: these are sorted so we can use a binary search */
  { "Accelerated",	OPTIONSCONSTANT,	ROMLIB_ACCELERATED_BIT },
  { "AppleChar",	APPLECHAR,              0 },
  { "BitsPerPixel",	BITSNUMBER,		0 },
  { "BlitAtTrapEnd",	OPTIONSCONSTANT,	ROMLIB_BLIT_TRAP_BIT },
  { "BlitInOSEvent",	OPTIONSCONSTANT,	ROMLIB_BLIT_OS_BIT },
  { "BlitOften",	OPTIONSCONSTANT,	ROMLIB_BLIT_OFTEN_BIT },
  { "CRC",		CRC,			0 },
  { "CacheProblems",	CACHEPROBLEMS,		0 },
  { "CloseIsQuit",	OPTIONSCONSTANT,	ROMLIB_CLOSE_IS_QUIT_BIT },
  { "Comments",		COMMENTS,		0 },
  { "Debug",		OPTIONSCONSTANT,	ROMLIB_DEBUG_BIT },
  { "Delay",		RDELAY,			0 },
  { "DirectDiskAccess",	OPTIONSCONSTANT,	ROMLIB_DIRECTDISKACCESS_BIT },
  { "DirtyVariant",	OPTIONSCONSTANT,	ROMLIB_DIRTY_VARIANT_BIT },
  { "DisposHandleHack", OPTIONSCONSTANT,        ROMLIB_DISPOSHANDLE_HACK_BIT },
  { "DongleFamily",	DONGLEFAMILY,		0 },
  { "FlushOften",	OPTIONSCONSTANT,	ROMLIB_FLUSHOFTEN_BIT },
  { "Gestalt",          GESTALT,                0 },
  { "MacCDROM",         MACCDROM,               0 },
  { "MacSize",		MACSIZE,		0 },
  { "NewLineToCR",	OPTIONSCONSTANT,	ROMLIB_NEWLINETOCR_BIT },
  { "NoClock",		OPTIONSCONSTANT,	ROMLIB_NOCLOCK_BIT },
  { "NoLower",		OPTIONSCONSTANT,	ROMLIB_NOLOWER_BIT },
  { "NoPrefPanel",	OPTIONSCONSTANT,	ROMLIB_NOPREFS_BIT },
  { "NoSuspend",	OPTIONSCONSTANT,	ROMLIB_NOSUSPEND_BIT },
  { "NoWarn32",		OPTIONSCONSTANT,	ROMLIB_NOWARN32_BIT },
  { "Options",		OPTIONS,		0 },
  { "PassPostscript",	OPTIONSCONSTANT,	ROMLIB_PASSPOSTSCRIPT_BIT },
  { "PretendAlias",	OPTIONSCONSTANT,	ROMLIB_PRETEND_ALIAS_BIT },
  { "PretendEdition",	OPTIONSCONSTANT,	ROMLIB_PRETEND_EDITION_BIT },
  { "PretendHelp",	OPTIONSCONSTANT,	ROMLIB_PRETEND_HELP_BIT },
  { "PretendScript",	OPTIONSCONSTANT,	ROMLIB_PRETEND_SCRIPT_BIT },
  { "PretendSound",	OPTIONSCONSTANT,	ROMLIB_PRETENDSOUND_BIT },
  { "PrinterName",      PRINTERNAME,            0, },
  { "PrinterTypeLabel", PRINTERTYPELABEL,       0, },
  { "PrintingHack",	OPTIONSCONSTANT,	ROMLIB_PRINTING_HACK_BIT },
  { "PrVers",		PRVERSION,		0, },
  { "RectScreen",	OPTIONSCONSTANT,	ROMLIB_RECT_SCREEN_BIT },
  { "Refresh",		OPTIONSCONSTANT,	ROMLIB_REFRESH_BIT },
  { "RefreshNumber",	REFRESHNUMBER,		0 },
  { "RightButtonModifier", RIGHT_BUTTON_MODIFIER, 0 },
  { "ScreenLocation",	SCREENLOCATION,		0 },
  { "ScreenSize",	SCREENSIZE,		0 },
  { "SoundOff",		OPTIONSCONSTANT,	ROMLIB_SOUNDOFF_BIT },
  { "SoundOn",		OPTIONSCONSTANT,	ROMLIB_SOUNDON_BIT },
  { "StripAddress",	OPTIONSCONSTANT,	ROMLIB_STRIPADDRESSHACK_BIT },
  { "SystemVersion",	SYSTEMVERSION,          0 },
  { "TextDisableHack",	OPTIONSCONSTANT,	ROMLIB_TEXT_DISABLE_BIT },
  { "Version",		VERSION,		0 },
  { "Win32Token",       WIN32TOKEN,             0 },
  { "WindowName",	WINDOWNAME,		0 },
};

#define	RELOAD	(-2)
#define	STRINGSPACE	1024

long binfind(char *tofind, struct namevaluestr table[], short nentries)
{
  short low, mid, high;
  int cmpret;

    for (low = -1, high = nentries; high - low > 1;) {
	mid = (high + low) / 2;
	cmpret = strcmp(tofind, table[mid].name);
	if (cmpret < 0)
	    high = mid;
	else if (cmpret == 0) {
	    yylval.number = table[mid].lval;
/*-->*/	    return table[mid].value;
	} else /* cmpret > 0 */
	    low = mid;
    }
    return 0;
}

static char charspace[STRINGSPACE];
static char *freechar = charspace, *endchar = charspace + STRINGSPACE;

/*
 * NOTE: call freestring only if you know no strings have been assigned
 *       after the one you are freeing.
 */

static void freestring( char *string )
{
    freechar = string;
}

static long linecount = 1;

static int
cmp (const void *p1, const void *p2)
{
  int retval;
  const char *str1;
  const char *str2;

  str1 = ((struct namevaluestr *)p1)->name;
  str2 = ((struct namevaluestr *)p2)->name;
  retval = strcmp (str1, str2);
  return retval;
}

static long yylex( void ) {
    static short c = RELOAD;
    char *tempp;
    long retval;
    int i;

    if (c == RELOAD)
	c = getc(configfile);

    for (;;) {
	switch (c) {

/* NOTE: if you mess with the '"' code, make sure to adjust clean()
   in toolevent.c.  */

	case '"':
	    yylval.cptr = freechar;
	    while (freechar < endchar && ((c = getc(configfile)) != '"'))
		*freechar++ = c;
	    if (freechar == endchar)
		warning_unexpected ("out of string space parsing configuration file (line %ld)", linecount);
	    else
		*freechar++ = 0;
	    c = getc(configfile);
	    return STRINGCONSTANT;
	    break;

	case '0':
	    if ((c = getc(configfile)) == 'x') {
		yylval.number = 0;
		while (isalnum(c = getc(configfile))) {
		    if (yylval.number & 0xF0000000)
			warning_unexpected ("hex constant too long parsing configuration file (line %ld)", linecount);
		    if (c >= '0' && c <= '9')
			yylval.number = (yylval.number << 4) + c - '0';
		    else if (c >= 'a' && c <= 'f')
			yylval.number = (yylval.number << 4) + c - 'a' + 10;
		    else if (c >= 'A' && c <= 'F')
			yylval.number = (yylval.number << 4) + c - 'A' + 10;
		    else
			warning_unexpected ("bad character in hex constant parsing configuration file (line %ld)", linecount);
		}
		return HEXCONSTANT;
		break;
	    }
	    /* We just ate a 0 that should be part of a decimal integer
	       (we don't support octal).  It is a leading zero, so we can
	       forget about it and fall through */
	    ungetc(c, configfile);
	    c = '0';
	    /* FALL THROUGH */
	case '1': case '2': case '3':
	case '4': case '5': case '6':
	case '7': case '8': case '9':
	    yylval.number = c - '0';
	    while (isdigit(c = getc(configfile))) {
		yylval.number = yylval.number * 10 + c - '0';
		if (yylval.number < 0)
		    warning_unexpected ("integer overflow parsing configuration file (line %ld)", linecount);
	    }
	    return INTEGERCONSTANT;
	    break;
	case ' ':
	case '\r':
	case '\n':
	case '\t':
	case '\f':
	    do
		if (c == '\n')
		    linecount++;
	    while (isspace(c = getc(configfile)));
	    break;
	case '/':
	    if ((c = getc(configfile)) == '/') {
		while ((c = getc(configfile)) != '\n')
		    ;
		linecount++;
		c = getc(configfile);
	    } else
		warning_unexpected ("single '/' parsing configuration file (line %ld)", linecount);
	    break;
	case '\'':
	    yylval.rtype = 0;
	    for (i = 0 ; i < 4 ; i++) {
		c = getc(configfile);
		yylval.rtype = (unsigned long)yylval.rtype * 256 + c;
	    }
	    if ((c = getc(configfile)) != '\'')
		warning_unexpected ("missing \' parsing configuration file (line %ld)", linecount);
	    c = getc(configfile);
	    return CHAR4CONSTANT;
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
	    tempp = freechar;
	    if (tempp < endchar)
		do
		    *tempp++ = c;
		while (tempp < endchar && isalnum(c = getc(configfile)));
	    if (tempp == endchar)
		warning_unexpected ("out of string space while looking up keyword parsing configuration file (line %ld)", linecount);
	    else
		*tempp++ = 0;
	    {
	      static boolean_t sorted = 0;

	      if (!sorted)
		{
		  qsort (reserved, NELEM (reserved), sizeof reserved[0], cmp);
		  sorted = 1;
		}
	    }
	    retval = binfind(freechar, reserved, NELEM (reserved));
	    if (!retval)
		warning_unexpected ("unknown reserved word parsing configuration file (line %ld)", linecount);
	    return retval;
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
	    c = getc(configfile);
	    return retval;
	    break;
	case EOF:
	    c = RELOAD;	/* for the next time around */
	    linecount = 1;
	    validblock = 1;
	    return EOF;
	    break;
	default:
	    warning_unexpected ("unknown character parsing configuration file (line %ld)", linecount);
	}
    }
}

void yyerror(const char *str)
{
    warning_unexpected ("configuration file parse error (line %ld): %s", linecount, str);
}
