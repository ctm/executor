/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_PSstrings[] =
		"$Id: PSstrings.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/PSstrings.h"

using namespace Executor;

const char Executor::ROMlib_doc_begin[] =
"%%!PS-Adobe-3.0\n"
"%%%%Creator: Executor\n"
"%%%%DocumentFonts: (atend)\n"
"%%%%Pages: (atend)\n"
"%%%%BoundingBox: (atend)\n"
"%s%s%s" /* "%%DocumentPaperSizes: Letter\n" */
"%%%%Orientation: %s\n" /* "Portrait" */
"%%%%EndComments\n"
"%%%%BeginProlog\n"
"\n";

const char Executor::ROMlib_doc_prolog[] =
"/MacEncoding StandardEncoding 256 array copy def\n"
"\n"
"128\n"
"[\n"
"    /Adieresis		/Aring		/Ccedilla	/Eacute\n"
"    /Ntilde		/Odieresis	/Udieresis	/aacute\n"
"    /agrave		/acircumflex	/adieresis	/atilde\n"
"    /aring		/ccedilla	/eacute		/egrave\n"
"    /ecircumflex	/edieresis	/iacute		/igrave\n"
"    /icircumflex	/idieresis	/ntilde		/oacute\n"
"    /ograve		/ocircumflex	/odieresis	/otilde\n"
"    /uacute		/ugrave		/ucircumflex	/udieresis\n"
"    /dagger		/degree		/cent		/sterling\n"
"    /section		/bullet		/paragraph	/germandbls\n"
"    /registered		/copyright	/trademark	/acute\n"
"    /dieresis		/notequal	/AE		/Oslash\n"
"    /infinity		/plusminus	/lessequal	/greaterequal\n"
"    /yen		/mu		/partialdiff	/summation\n"
"    /product		/pi		/integral	/ordfeminine\n"
"    /ordmasculine	/Omega		/ae		/oslash\n"
"    /questiondown	/exclamdown	/logicalnot	/daggerdbl\n"
"    /florin		/approxequal	/Delta		/guillemotleft\n"
"    /guillemotright	/ellipsis	/space		/Agrave\n"
"    /Atilde		/Otilde		/OE		/oe\n"
"    /endash		/emdash		/quotedblleft	/quotedblright\n"
"    /quoteleft		/quoteright	/divide		/lozenge\n"
"    /ydieresis		/Ydieresis	/fraction	/currency\n"
"    /guilsinglleft	/guilsinglright	/fi		/fl\n"
"    /daggerdbl		/periodcentered	/quotesinglbase	/quotedblbase\n"
"    /perthousand	/Acircumflex	/Ecircumflex	/Aacute\n"
"    /Edieresis		/Egrave		/Iacute		/Icircumflex\n"
"    /Idieresis		/Igrave		/Oacute		/Ocircumflex\n"
"    /apple		/Ograve		/Uacute		/Ucircumflex\n"
"    /Ugrave		/dotlessi	/circumflex	/tilde\n"
"    /macron		/breve		/dotaccent	/ring\n"
"    /cedilla		/hungarumlaut	/oganek		/caron\n"
"]\n"
"    { MacEncoding 2 index 2 index put pop 1 add }\n"
"forall\n"
"pop\n"
"\n"
"/MacEncoding MacEncoding readonly def\n"
"\n"
"/_MACfstr 128 string dup 0 (_MAC) putinterval def   \n"
"\n"
"/findfont2\n"
"{\n"
"    /currentshared where\n"
"        {pop currentshared}\n"
"	{false}\n"
"    ifelse\n"
"	{//findfont exec}\n"
"	{\n"
"	    dup _MACfstr 4 124 getinterval cvs length 4 add _MACfstr 0 3 -1 roll\n"
"	    getinterval cvn exch FontDirectory 2 index known \n"
"		{pop FontDirectory exch get}\n"
"		{\n"
"		    //findfont exec dup /Encoding get\n"
"			dup\n"
"			StandardEncoding eq\n"
"			exch\n"
"			/NextStepEncoding where\n"
"			    { /NextStepEncoding get eq }\n"
"			    { pop false }\n"
"			ifelse\n"
"			or\n"
"			{\n"
"			    dup length dict exch\n"
"			    {\n"
"				1 index /FID ne\n"
"				    {2 index 3 1 roll put}\n"
"				    {pop pop}\n"
"				ifelse\n"
"			    } forall\n"
"			    dup /Encoding MacEncoding put definefont\n"
"			}\n"
"			{exch pop}\n"
"		    ifelse\n"
"		}\n"
"	    ifelse\n"
"	}\n"
"    ifelse\n"
"} bind def\n"
"\n"
"/selectfont {\n"
"    exch findfont2\n"
"    exch dup type /arraytype eq\n"
"        {makefont}\n"
"	{scalefont}\n"
"    ifelse\n"
"    setfont\n"
"} bind def\n"

/* For the next definition, each comment represents the stack, with
   the top of stack on the left (the opposite of the way the PostScript
   red book shows the stack) after each of the operations on that line
   have been completed. */

"/__char_spaces_width_show {\n"
"  dup\n"           	/* string string desiredwidth nspaces nchar */
"  stringwidth pop\n"	/* realwidth string desiredwidth nspaces nchar */
"  exch 4 1 roll\n"	/* realwidth desiredwidth nspaces string nchar */
"  sub\n"		/* difference nspaces string nchar */
"  dup\n"               /* difference difference nspaces string nchar */
"  3 -1 roll div\n"	/* spaceadjust difference string nchar */
"  dup 0 lt\n" 
"  {\n"
"    pop\n"             /* difference string nchar */
"    3 -1 roll\n"	/* nchar difference string */
"    div\n" 		/* charadjustx string */
"    0 0 32 4 -1 roll 0\n"	/* 0 charadjustx 32 0 0 string */
"  }\n"
"  {\n"
"    2 -1 roll pop\n"    /* spaceadjustx string nchar */
"    3 -1 roll pop\n"    /* spaceadjustx string */
"    0 32 0 0\n"	/* 0 0 32 0 spaceadjustx string */
"  }\n"
"  ifelse\n"
"  6 -1 roll\n"		/* string 0 charadjustx 32 0 spaceadjustx */
"  awidthshow\n"
"}\n"
"bind def\n"
"\n"
"%% Dilemma: If we don't undef NextStepEncoding then NeXT generated EPS\n"
"%%          files will not have the correct encoding vector when printed\n"
"%%          under Executor.  BUT if we undef NextStepEncoding then we\n"
"%%          can't use their implementation of findfont.  Foo.\n"
"%% /languagelevel where\n"
"%% {\n"
"%%     pop\n"
"%%     languagelevel 2 ge\n"
"%%     {\n"
"%%         currentdict /NextStepEncoding undef\n"
"%%     } if\n"
"%% } if\n";

const char Executor::ROMlib_doc_end_prolog[] =
"gsave\n"
"-1 -9 translate\n"
" /__NXbasematrix matrix currentmatrix def\n"
"grestore\n"
"%%%%EndProlog\n"
"%%%%BeginSetup\n"
"%s%s%s" /* "%%PaperSize: Letter\n" */
"/#copies %d def\n"

"%.*s\n" /* for resource ('PREC', 103) */

"%%%%EndSetup\n";

const char Executor::ROMlib_doc_end[] =
"%%%%Trailer\n"
"%%%%Pages: %d\n"
"%%%%BoundingBox:0 0 %d %d\n";

const char Executor::ROMlib_page_begin[] =
"%%%%Page: %d %d\n"
"%%%%PageBoundingBox: 0 0 %d %d\n"
"%%%%PageFonts: (atend)\n"
"%%%%BeginPageSetup\n"
"%s%s%s" /* "%%PaperSize: Letter\n" */
"/__NXsheetsavetoken save def\n"
"%d rotate\n"
"%d %d translate\n"
/* "0 0 translate\n" */
/* "0 792 translate\n" */
"%f %f scale\n"
"/pse {} def\n"
"/psb {} def\n"
"/execuserobject {\n"
"  dup UserObjects length ge\n"
"    { pop }\n"
"    { UserObjects exch get exec }\n"
"  ifelse\n"
"} def\n"
"/currentmouse { 0 0 } def\n"
"/printobject { pop pop } def\n"
"gsave\n"
"-1 -9 translate\n"
" /__NXbasematrix matrix currentmatrix def\n"
"grestore\n"
"0 0 translate\n"
"%%%%EndPageSetup\n"
"%% The following is a lie that is necessary because Word 5\n"
"%% doesn't wrap EPS files properly and we can't tell where\n"
"%% the PostScript we're printing comes from.\n"
"%%%%BeginDocument: IWishWeDidntHaveToDoThis\n"
"gsave\n"
;

const char Executor::ROMlib_page_end[] =
"grestore\n"
"%%%%EndDocument\n"

"/showpage systemdict begin { showpage } bind end def\n"
"showpage\n"
"__NXsheetsavetoken restore\n"
"%%%%PageTrailer\n"
;
