/*
 * Here is the original comment... it's not too relevant anymore:
 *
 * BreakView.m, view to implement the "BreakApp" game.
 * Author: Ali Ozer
 * Written for 0.8 October 88. 
 * Modified for 0.9 March 89.
 * Modified for 1.0 July 89.
 * Removed use of Bitmap and threw away some classes May 90.
 * Final 2.0 fixes/enhancements Sept 90.
 *
 * BreakView implements an interactive custom view that allows the user
 * to play "BreakApp," a game similar to a popular arcade classic.
 *
 * BreakView's main control methods are based on the target-action
 * paradigm; thus you can include BreakView in an Interface-Builder based
 * application. Please refer to BreakView.h for a list of "public" methods
 * that you should provide links to in Interface Builder.
 *
 *  You may freely copy, distribute and reuse the code in this example.
 *  NeXT disclaims any warranty of any kind, expressed or implied,
 *  as to its fitness for any particular use.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_MacViewClass[] =
    "$Id: MacViewClass.m,v 2.7 1995/03/11 20:29:43 mat Exp mat $";
#endif

#include "rsys/common.h"

#import "next/MacViewClass.h"
#import "next/MacAppClass.h"
#import "next/ourstuff.h"

#import <math.h>
#import <dpsclient/wraps.h>	// PSxxx functions
#import <appkit/appkit.h>

#import <QuickDraw.h>
#import <CQuickDraw.h>
#import <rsys/cquick.h>
#include <regex.h>

#if defined(mc68000) && (!defined(NX_CURRENT_COMPILER_RELEASE) || \
	        (NX_CURRENT_COMPILER_RELEASE < NX_COMPILER_RELEASE_3_0))
#include <mach.h>
#else
#include <mach/mach.h>
#endif

#include "rsys/mactype.h"
#include <assert.h>
#include "rsys/nextprint.h"
#include "rsys/blockinterrupts.h"
#include "rsys/next.h"
#include "rsys/myabort.h"
#include "rsys/trapglue.h"
#include "rsys/syn68k_public.h"
#include "rsys/version.h"
#include "rsys/executor.h"

/*
 * TODO: move this into a common include file that OSUtil.h will pick up
 */


/* These are methods in this file and also Mac #defines. */
#undef mouseUp
#undef mouseDown
#undef keyDown
#undef keyUp

#define OSTrap		0
#define ToolTrap	1
extern long NGetTrapAddress( short n, short ttype ); 

@implementation MacViewClass

static DPSTimedEntry timer;
static id theonlyframe, theonlyscreenimage;/* UGLY UGLY UGLY */
static NXBitmapImageRep *theonlyscreenimagerep;
long orig_height, orig_width, curr_height, curr_width;
float mac_to_next_x = 1, mac_to_next_y = 1;
id realcursor, blankcursor, cursorimage;
NXBitmapImageRep *cursorrep;

typedef enum { MacToUNIX, UNIXToMac, MacRTFToUNIX, UNIXRTFToMac } convertdir_t;

typedef struct {
    long first;
    long second;
} pair_t;

unsigned char mactonext[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    26,
    27,
    28,
    29,
    30,
    31,
    32,
    33,
    34,
    35,
    36,
    37,
    38,
    39,			/* NEXTCHAR_QUOTESINGLE, */
    40,
    41,
    42,
    43,
    44,
    45,
    46,
    47,
    48,
    49,
    50,
    51,
    52,
    53,
    54,
    55,
    56,
    57,
    58,
    59,
    60,
    61,
    62,
    63,
    64,
    65,
    66,
    67,
    68,
    69,
    70,
    71,
    72,
    73,
    74,
    75,
    76,
    77,
    78,
    79,
    80,
    81,
    82,
    83,
    84,
    85,
    86,
    87,
    88,
    89,
    90,
    91,
    92,
    93,
    94,
    95,
    96,			/* NEXTCHAR_GRAVE, */
    97,
    98,
    99,
    100,
    101,
    102,
    103,
    104,
    105,
    106,
    107,
    108,
    109,
    110,
    111,
    112,
    113,
    114,
    115,
    116,
    117,
    118,
    119,
    120,
    121,
    122,
    123,
    124,
    125,
    126,
    127,
    NEXTCHAR_ADIERESIS,
    NEXTCHAR_ARING,
    NEXTCHAR_CCEDILLA,
    NEXTCHAR_EACUTE,
    NEXTCHAR_NTILDE,
    NEXTCHAR_ODIERESIS,
    NEXTCHAR_UDIERESIS,
    NEXTCHAR_aACUTE,
    NEXTCHAR_aGRAVE,
    NEXTCHAR_aCIRCUMFLEX,
    NEXTCHAR_aDIERESIS,
    NEXTCHAR_aTILDE,
    NEXTCHAR_aRING,
    NEXTCHAR_cCEDILLA,
    NEXTCHAR_eACUTE,
    NEXTCHAR_eGRAVE,
    NEXTCHAR_eCIRCUMFLEX,
    NEXTCHAR_eDIERESIS,
    NEXTCHAR_iACUTE,
    NEXTCHAR_iGRAVE,
    NEXTCHAR_iCIRCUMFLEX,
    NEXTCHAR_iDIERESIS,
    NEXTCHAR_nTILDE,
    NEXTCHAR_oACUTE,
    NEXTCHAR_oGRAVE,
    NEXTCHAR_oCIRCUMFLEX,
    NEXTCHAR_oDIERESIS,
    NEXTCHAR_oTILDE,
    NEXTCHAR_uACUTE,
    NEXTCHAR_uGRAVE,
    NEXTCHAR_uCIRCUMFLEX,
    NEXTCHAR_uDIERESIS,
    NEXTCHAR_DAGGER,
    NEXTCHAR_DEGREE,
    NEXTCHAR_CENT,
    NEXTCHAR_STERLING,
    NEXTCHAR_SECTION,
    NEXTCHAR_BULLET,
    NEXTCHAR_PARAGRAPH,
    NEXTCHAR_GERMANDBLS,
    NEXTCHAR_REGISTERED,
    NEXTCHAR_COPYRIGHT,
    NEXTCHAR_TRADEMARK,
    NEXTCHAR_ACUTE,
    NEXTCHAR_DIERESIS,
    NEXTCHAR_NOTEQUAL,
    NEXTCHAR_AE,
    NEXTCHAR_OSLASH,
    NEXTCHAR_INFINITY,
    NEXTCHAR_PLUSMINUS,
    NEXTCHAR_LESSEQUAL,
    NEXTCHAR_GREATEQUAL,
    NEXTCHAR_YEN,
    NEXTCHAR_MU,
    NEXTCHAR_PARTIALDIFF,
    NEXTCHAR_SUMMATION,
    NEXTCHAR_PRODUCT,
    NEXTCHAR_PI,
    NEXTCHAR_INTEGRAL,
    NEXTCHAR_ORDFEMININE,
    NEXTCHAR_ORDMASCULINE,
    NEXTCHAR_OMEGA,
    NEXTCHAR_ae,
    NEXTCHAR_oSLASH,
    NEXTCHAR_QUESTIONDOWN,
    NEXTCHAR_EXCLAMDOWN,
    NEXTCHAR_LOGICALNOT,
    NEXTCHAR_RADICAL,
    NEXTCHAR_FLORIN,
    NEXTCHAR_APPROXEQUAL,
    NEXTCHAR_DELTA,
    NEXTCHAR_GUILLEMOTLEFT,
    NEXTCHAR_GUILLEMOTRIGHT,
    NEXTCHAR_ELLIPSIS,
    NEXTCHAR_FIGSP,
    NEXTCHAR_AGRAVE,
    NEXTCHAR_ATILDE,
    NEXTCHAR_OTILDE,
    NEXTCHAR_OE,
    NEXTCHAR_oe,
    NEXTCHAR_ENDASH,
    NEXTCHAR_EMDASH,
    NEXTCHAR_QUOTEDBLLEFT,
    NEXTCHAR_QUOTEDBLRIGHT,
    NEXTCHAR_GRAVE,			/* NEXTCHAR_QUOTELEFT, */
    NEXTCHAR_QUOTESINGLE,		/* NEXTCHAR_QUOTERIGHT, */
    NEXTCHAR_DIVIDE,
    NEXTCHAR_LOZENGE,
    NEXTCHAR_yDIERESIS,
    NEXTCHAR_YDIERESIS,
    NEXTCHAR_FRACTION,
    NEXTCHAR_CURRENCY,
    NEXTCHAR_GUILSINGLLEFT,
    NEXTCHAR_GUILSINGLRIGHT,
    NEXTCHAR_FI,
    NEXTCHAR_FL,
    NEXTCHAR_DAGGERDBL,
    NEXTCHAR_PERIODCENTERED,
    NEXTCHAR_QUOTESINGLBASE,
    NEXTCHAR_QUOTEDBLBASE,
    NEXTCHAR_PERTHOUSAND,
    NEXTCHAR_ACIRCUMFLEX,
    NEXTCHAR_ECIRCUMFLEX,
    NEXTCHAR_AACUTE,
    NEXTCHAR_EDIERESIS,
    NEXTCHAR_EGRAVE,
    NEXTCHAR_IACUTE,
    NEXTCHAR_ICIRCUMFLEX,
    NEXTCHAR_IDIERESIS,
    NEXTCHAR_IGRAVE,
    NEXTCHAR_OACUTE,
    NEXTCHAR_OCIRCUMFLEX,
    NEXTCHAR_APPLE,
    NEXTCHAR_OGRAVE,
    NEXTCHAR_UACUTE,
    NEXTCHAR_UCIRCUMFLEX,
    NEXTCHAR_UGRAVE,
    NEXTCHAR_DOTLESSI,
    NEXTCHAR_CIRCUMFLEX,
    NEXTCHAR_TILDE,
    NEXTCHAR_MACRON,
    NEXTCHAR_BREVE,
    NEXTCHAR_DOTACCENT,
    NEXTCHAR_RING,
    NEXTCHAR_CEDILLA,
    NEXTCHAR_HUNGARUMLAUT,
    NEXTCHAR_OGONEK,
    NEXTCHAR_CARON
};

unsigned char nexttomac[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    26,
    27,
    28,
    29,
    30,
    31,
    32,
    33,
    34,
    35,
    36,
    37,
    38,
    39,				/* MACCHAR_QUOTERIGHT, */
    40,
    41,
    42,
    43,
    44,
    45,
    46,
    47,
    48,
    49,
    50,
    51,
    52,
    53,
    54,
    55,
    56,
    57,
    58,
    59,
    60,
    61,
    62,
    63,
    64,
    65,
    66,
    67,
    68,
    69,
    70,
    71,
    72,
    73,
    74,
    75,
    76,
    77,
    78,
    79,
    80,
    81,
    82,
    83,
    84,
    85,
    86,
    87,
    88,
    89,
    90,
    91,
    92,
    93,
    94,
    95,
    96,				/* MACCHAR_QUOTELEFT, */
    97,
    98,
    99,
    100,
    101,
    102,
    103,
    104,
    105,
    106,
    107,
    108,
    109,
    110,
    111,
    112,
    113,
    114,
    115,
    116,
    117,
    118,
    119,
    120,
    121,
    122,
    123,
    124,
    125,
    126,
    127,
    MACCHAR_FIGSP,
    MACCHAR_AGRAVE,
    MACCHAR_AACUTE,
    MACCHAR_ACIRCUMFLEX,
    MACCHAR_ATILDE,
    MACCHAR_ADIERESIS,
    MACCHAR_ARING,
    MACCHAR_CCEDILLA,
    MACCHAR_EGRAVE,
    MACCHAR_EACUTE,
    MACCHAR_ECIRCUMFLEX,
    MACCHAR_EDIERESIS,
    MACCHAR_IGRAVE,
    MACCHAR_IACUTE,
    MACCHAR_ICIRCUMFLEX,
    MACCHAR_IDIERESIS,
    MACCHAR_ETH,
    MACCHAR_NTILDE,
    MACCHAR_OGRAVE,
    MACCHAR_OACUTE,
    MACCHAR_OCIRCUMFLEX,
    MACCHAR_OTILDE,
    MACCHAR_ODIERESIS,
    MACCHAR_UGRAVE,
    MACCHAR_UACUTE,
    MACCHAR_UCIRCUMFLEX,
    MACCHAR_UDIERESIS,
    MACCHAR_YACUTE,
    MACCHAR_THORN,
    MACCHAR_MU,
    MACCHAR_MULTIPLY,
    MACCHAR_DIVIDE,
    MACCHAR_COPYRIGHT,
    MACCHAR_EXCLAMDOWN,
    MACCHAR_CENT,
    MACCHAR_STERLING,
    MACCHAR_FRACTION,
    MACCHAR_YEN,
    MACCHAR_FLORIN,
    MACCHAR_SECTION,
    MACCHAR_CURRENCY,
    MACCHAR_QUOTERIGHT,			/* MACCHAR_QUOTESINGLE, */
    MACCHAR_QUOTEDBLLEFT,
    MACCHAR_GUILLEMOTLEFT,
    MACCHAR_GUILSINGLLEFT,
    MACCHAR_GUILSINGLRIGHT,
    MACCHAR_FI,
    MACCHAR_FL,
    MACCHAR_REGISTERED,
    MACCHAR_ENDASH,
    MACCHAR_DAGGER,
    MACCHAR_DAGGERDBL,
    MACCHAR_PERIODCENTERED,
    MACCHAR_BROKENBAR,
    MACCHAR_PARAGRAPH,
    MACCHAR_BULLET,
    MACCHAR_QUOTESINGLBASE,
    MACCHAR_QUOTEDBLBASE,
    MACCHAR_QUOTEDBLRIGHT,
    MACCHAR_GUILLEMOTRIGHT,
    MACCHAR_ELLIPSIS,
    MACCHAR_PERTHOUSAND,
    MACCHAR_LOGICALNOT,
    MACCHAR_QUESTIONDOWN,
    MACCHAR_ONESUPERIOR,
    MACCHAR_QUOTELEFT,			/* MACCHAR_GRAVE, */
    MACCHAR_ACUTE,
    MACCHAR_CIRCUMFLEX,
    MACCHAR_TILDE,
    MACCHAR_MACRON,
    MACCHAR_BREVE,
    MACCHAR_DOTACCENT,
    MACCHAR_DIERESIS,
    MACCHAR_TWOSUPERIOR,
    MACCHAR_RING,
    MACCHAR_CEDILLA,
    MACCHAR_THREESUPERIOR,
    MACCHAR_HUNGARUMLAUT,
    MACCHAR_OGONEK,
    MACCHAR_CARON,
    MACCHAR_EMDASH,
    MACCHAR_PLUSMINUS,
    MACCHAR_ONEQUARTER,
    MACCHAR_ONEHALF,
    MACCHAR_THREEQUARTERS,
    MACCHAR_aGRAVE,
    MACCHAR_aACUTE,
    MACCHAR_aCIRCUMFLEX,
    MACCHAR_aTILDE,
    MACCHAR_aDIERESIS,
    MACCHAR_aRING,
    MACCHAR_cCEDILLA,
    MACCHAR_eGRAVE,
    MACCHAR_eACUTE,
    MACCHAR_eCIRCUMFLEX,
    MACCHAR_eDIERESIS,
    MACCHAR_iGRAVE,
    MACCHAR_AE,
    MACCHAR_iACUTE,
    MACCHAR_ORDFEMININE,
    MACCHAR_iCIRCUMFLEX,
    MACCHAR_iDIERESIS,
    MACCHAR_eTH,
    MACCHAR_nTILDE,
    MACCHAR_LSLASH,
    MACCHAR_OSLASH,
    MACCHAR_OE,
    MACCHAR_ORDMASCULINE,
    MACCHAR_oGRAVE,
    MACCHAR_oACUTE,
    MACCHAR_oCIRCUMFLEX,
    MACCHAR_oTILDE,
    MACCHAR_oDIERESIS,
    MACCHAR_ae,
    MACCHAR_uGRAVE,
    MACCHAR_uACUTE,
    MACCHAR_uCIRCUMFLEX,
    MACCHAR_DOTLESSI,
    MACCHAR_uDIERESIS,
    MACCHAR_yACUTE,
    MACCHAR_lSLASH,
    MACCHAR_oSLASH,
    MACCHAR_oe,
    MACCHAR_GERMANDBLS,
    MACCHAR_tHORN,
    MACCHAR_yDIERESIS,
    MACCHAR_NEXT254,
    MACCHAR_NEXT255
};


void converthex(char **h, unsigned char *table)
{
    unsigned int val, val2;
    char *p;

    p = *h;
    if (*p <= 'f' && *p >= 'a')
	val = *p - 'a' + 10;
    else if (*p <= 'F' && *p >= 'A')
	val = *p - 'A' + 10;
    else
	val = *p - '0';

    if (*(p + 1) <= 'f' && *(p + 1) >= 'a')
	val = val * 16 + *(p + 1) - 'a' + 10;
    else if (*(p + 1) <= 'F' && *(p + 1) >= 'A')
	val = val * 16 + *(p + 1) - 'A' + 10;
    else
	val = val * 16 + *(p + 1) - '0';

    val = table[val];
    val2 = val/16;
    if (val2 > 9)
	*p++ = val2 + 'a' - 10;
    else
	*p++ = val2 + '0';

    val2 = val & 0xF;
    if (val2 > 9)
	*p++ = val2 + 'a' - 10;
    else
	*p++ = val2 + '0';
}

void convertchars(char *data, long length, unsigned char *table)
{
    char *ep;
    int escaped;

    escaped = NO;
    ep = data + length;
    while (data < ep) {
	if (escaped && *data == '\'') {
	    *data++ = '\'';
	    converthex(&data, table);
	}
	if (*data == '\\')
	    escaped = YES;
	else
	    escaped = NO;
	*data++ = table[*(unsigned char *)data];
    }
}

static int unixrtfconvert(int length, char *ip, char *op)
{
    char *fromp, *ep;
    int retval;

    retval = length;
    fromp = ip;
    ep = ip + length;
    while (fromp < ep) {
	if (*fromp == '\n') {
	    *op++ = '\r';
	    fromp++;
	} else if (*fromp == '\\') {
	    *op++ = '\\';
	    fromp++;
	    if (fromp < ep && *fromp == '\n') {
#if !defined (WEDONTNEEDNOSTEENINGPAR)
		strcpy(op, "par \n");
		op += 5;
		fromp++;
		retval += 4;
#else /* WEDONTNEEDNOSTEENINGPAR */
		*op++ = '\r';
		fromp++;
#endif /*  WEDONTNEEDNOSTEENINGPAR */
	    } else if (fromp + 2 < ep && !strncmp(fromp, "ul0", 3)) {
		strcpy(op, "ulnone");
		op += 6;
		fromp += 3;
		retval += 3;
	    } else if (fromp + 6 < ep && !strncmp(fromp, "fonttbl", 6)) {
		strcpy(op, "fonttbl\\f99\\fa b;");
		op += 17;
		fromp += 7;
		retval += 10;
	    } else
		*op++ = *fromp++;
	} else
	    *op++ = *fromp++;
    }
    return retval;
}

static inline void copyandtranslate( char *cp, char **ipp, char **opp)
{
    *cp = *(*ipp)++;
    if (*cp == '\r')
	*cp = '\n';
    *(*opp)++ = *cp;
}

#define PREFIX	"\\endnhere"
#define NCHARPREFIX	(sizeof(PREFIX)-1)

#if 1
#define STICKIN				\
    "{\\fonttbl"			\
	"{\\f0\\fswiss Helvetica;}"	\
	"{\\f3\\fmodern Courier;}"	\
	"{\\f4\\fmodern Ohlfs;}"	\
	"{\\f20\\froman Times;}"	\
	"{\\f21\\fswiss Helvetica;}"	\
	"{\\f22\\fmodern Courier;}"	\
    "}\\f0"
#else /* 0 */
#define STICKIN				\
    "{\\fonttbl"			\
	"{\\f0\\fHelvetica;}"	\
	"{\\f3\\fCourier;}"	\
	"{\\f4\\fOhlfs;}"	\
	"{\\f20\\fTimes;}"	\
	"{\\f21\\fHelvetica;}"	\
	"{\\f22\\fCourier;}"	\
    "}\\f0"
#endif /* 0 */
#define NCHARSTICKIN	(sizeof(STICKIN)-1)

static int macrtfconvert(int length, char *ip, char *op)
{
    int retval, index;
    char c;
    SETUPA5;

    retval = length;
    index = 0;
    while (length > 0 && index < NCHARPREFIX) {
	copyandtranslate(&c, &ip, &op);
	if (PREFIX[index] == c)
	    ++index;
        else if (PREFIX[0] == c)
            index = 1;
        else
            index = 0;
	--length;
    }
#if 0
    bracecount = 1;
    while (length > 0 && bracecount > 0) {
	copyandtranslate(&c, &ip, &op);
	switch (c) {
	case '{':
	    ++bracecount;
	    break;
	case '}':
	    --bracecount;
	    break;
	case '\\':
	    if (--length >= 0)
		copyandtranslate(&c, &ip, &op);
	    break;
	}
	--length;
    }
#endif /* 0 */
    if (length > 0) {
#if 0
	retval += NCHARSTICKIN;
	bcopy(STICKIN, op, NCHARSTICKIN);
	op += NCHARSTICKIN;
#else /* 0 */
	retval += insertfonttbl(&op, (char) -1);
#endif /* 0 */
    }
    while (--length >= 0)
	copyandtranslate(&c, &ip, &op);
    RESTOREA5;
    return retval;
}

static int convertreturns(char *datain, char *dataout, int length,
							      convertdir_t dir)
{
    char from, to;

    switch (dir) {
    case MacToUNIX:
	bcopy(datain, dataout, length);
	from = '\r';
	to   = '\n';
	break;
    case UNIXToMac:
	bcopy(datain, dataout, length);
	from = '\n';
	to   = '\r';
	break;
    case MacRTFToUNIX:
/*-->*/	return macrtfconvert(length, datain, dataout);
	break;
    case UNIXRTFToMac:
/*-->*/	return unixrtfconvert(length, datain, dataout);
	break;
    default:
	from = 0;
	to = 0;
	gui_abort();
    }
    while (--length >= 0)
	if (*dataout++ == from)
	    dataout[-1] = to;
    return 0;
}

short ROMlib_accelerated = 0;
extern id global_game;
extern id global_gameWindow;

static void accelerate(long top, long left, long bottom, long right)
{
    NXRect rect;
    NXPoint point;
    long offsetx, offsety;
    extern void ROMlib_blastframebuffer(long top, long left, long bottom,
		   long right, long offsetx, long offsety, long scaled_by_two);

    SETUPA5;
    [global_game getFrame:&rect];
    point = rect.origin;
    [global_game convertPoint:&point toView:nil];
    [global_gameWindow convertBaseToScreen:&point];
    offsetx = point.x;
    offsety = (SCREEN_MONO ? VIDEO_H : C16_VIDEO_H) - point.y - curr_height;

    ROMlib_blastframebuffer(top, left, bottom, right, offsetx, offsety,
							  (mac_to_next_x > 1));
    RESTOREA5;
}



long *ROMlib_realScreen;
long ROMlib_realScreenRowBytes;


static unsigned short color_map_table[256];
static unsigned char mono_map_table[256];
static long old_seed_x = CLC (-3);

unsigned char map_to_mono(const ColorSpec *tablep, int i)
{
    return (CW(tablep[i].rgb.red)   +
            CW(tablep[i].rgb.green) +
            CW(tablep[i].rgb.blue)  ) / 65535;
}

static void
init_color_map_table (const ColorSpec *table, int max_ctab_elt)
{
  int i;
  unsigned char trans[4];

  if (ROMlib_nextscreen == SCREEN_MONO) {
    trans[0] = map_to_mono(table, 0);
    trans[1] = map_to_mono(table, 1);
    trans[2] = map_to_mono(table, 2);
    trans[3] = map_to_mono(table, 3);
    for (i = 0; i <= 255; i++)
      {
	mono_map_table[i] = (trans[(i >> 6) & 3] << 6) |
			    (trans[(i >> 4) & 3] << 4) |
			    (trans[(i >> 2) & 3] << 2) |
			    (trans[(i >> 0) & 3] << 0);
      }
  } else {
    for (i = 0; i <= max_ctab_elt; i++)
      {
	color_map_table[i] = CW (((CW (table[i].rgb.red) >> 12) << 12)
				 | ((CW (table[i].rgb.green) >> 12) << 8)
				 | ((CW (table[i].rgb.blue) >> 12) << 4)
				 | 0x000F);
      }
  }
}

static int drawing_invalid_p = 0;

static void
update_screen_image_rep (int pixel_top, int pixel_left,
			 int pixel_width, int pixel_height)
{
  const unsigned char *in;
  unsigned short *out;
  unsigned char *out_bytep;
  unsigned in_add, out_add, out_add_byte;
  CTabHandle color_table;
  PixMapHandle pmap;

  int byte_left, byte_right, byte_width;
  int bpp;
  
  if (MainDevice == CLC (NULL) || drawing_invalid_p)
    return;
  
  pmap = GD_PMAP (CL (MainDevice));
  color_table = PIXMAP_TABLE (pmap);
  if (CTAB_SEED_X (color_table) != old_seed_x) {
    init_color_map_table (CTAB_TABLE (color_table),
			  CTAB_SIZE (color_table));
    old_seed_x = CTAB_SEED_X(color_table);
  }
  bpp = PIXMAP_PIXEL_SIZE (pmap);
  byte_left = (pixel_left * bpp) / 8;
  byte_right = ((pixel_left + pixel_width) * bpp + 7) / 8;
  byte_width = byte_right - byte_left;
  
  pixel_left = (byte_left * 8) / bpp;
  pixel_width = (byte_width * 8) / bpp;
  
  /* Grab the base of input */
  in = &((char *) ROMlib_realScreen)[ROMlib_realScreenRowBytes * pixel_top
				     + byte_left];
  in_add = ROMlib_realScreenRowBytes - byte_width;

  /* Transfer the stuff to the screen. */
  if (ROMlib_nextscreen == SCREEN_MONO) {
    /* Grab the base of output. */
    out_bytep = (unsigned char *) ((char *)[theonlyscreenimagerep data]
			    + ([theonlyscreenimagerep bytesPerRow] * pixel_top
			       + pixel_left * sizeof (unsigned short)));
    out_add_byte = ([theonlyscreenimagerep bytesPerRow] - byte_width);
    for (; pixel_height > 0; pixel_height--)
      {
	int w;

	for (w = byte_width; w > 0; w--)
	  {
	    unsigned char this_in;

	    this_in = *in++;
	    switch (bpp)
	      {
	      case 1:
		/* TODO */
		*out_bytep++ = mono_map_table[this_in];
	      case 2:
		*out_bytep++ = mono_map_table[this_in];
		break;
	      }
	  }

	in += in_add;
	out_bytep += out_add_byte;
      }
  } else {
    /* Grab the base of output. */
    out = (unsigned short *) ((char *)[theonlyscreenimagerep data]
			    + ([theonlyscreenimagerep bytesPerRow] * pixel_top
			       + pixel_left * sizeof (unsigned short)));
    out_add = ([theonlyscreenimagerep bytesPerRow]
	     - byte_width * sizeof (unsigned short));

    for (; pixel_height > 0; pixel_height--)
      {
	int w;

	for (w = byte_width; w > 0; w--)
	  {
	    unsigned char this_in;

	    this_in = *in++;
	    switch (bpp)
	      {
	      case 1:
		*out++ = color_map_table[(this_in >> 7) & 0x1];
		*out++ = color_map_table[(this_in >> 6) & 0x1];
		*out++ = color_map_table[(this_in >> 5) & 0x1];
		*out++ = color_map_table[(this_in >> 4) & 0x1];
		*out++ = color_map_table[(this_in >> 3) & 0x1];
		*out++ = color_map_table[(this_in >> 2) & 0x1];
		*out++ = color_map_table[(this_in >> 1) & 0x1];
		*out++ = color_map_table[this_in & 0x1];
		break;
	      case 2:
		*out++ = color_map_table[(this_in >> 6) & 0x3];
		*out++ = color_map_table[(this_in >> 4) & 0x3];
		*out++ = color_map_table[(this_in >> 2) & 0x3];
		*out++ = color_map_table[this_in & 0x3];
		break;
	      case 4:
		*out++ = color_map_table[(this_in >> 4) & 0xF];
		*out++ = color_map_table[this_in & 0xF];
		break;
	      case 8:
		*out++ = color_map_table[this_in];
		break;
	      }
	  }

	in += in_add;
	out = (unsigned short *) ((char *) out + out_add);
      }
  }
}

void putimageX(long top, long left, long bottom, long right)
{
    NXRect rect;
    NXPoint point;
    virtual_int_state_t block;

    point.x = left * mac_to_next_x;
    point.y = (orig_height - bottom) * mac_to_next_y;

    rect.origin      = point;
    rect.size.width  = (right - left) * mac_to_next_x;
    rect.size.height = (bottom - top) * mac_to_next_y;

    block = block_virtual_ints ();
#if 0
    [theonlyscreenimage recache];
    [theonlyscreenimage composite:NX_COPY fromRect:&rect toPoint:&point];

    /* WOULD BE NICE:  NXCopyBits(NXNullObject, &rect, &point); */
#else /* 0 */
    if (0 && ROMlib_accstate == acc_accelerated)
	accelerate(top, left, bottom, right);
    else
      {
	update_screen_image_rep (top, left, right - left, bottom - top);
	[theonlyframe display:&rect: 1: YES];
      }
#endif /* 0 */
    restore_virtual_ints (block);
}

/*
 * NOTE: the code below should be table driven.
 */

static NXAtom ARDIPICTPboardType = "PICT";

#define TEXT (('T'<<24)|('E'<<16)|('X'<<8)|'T')
#define EPS  (('E'<<24)|('P'<<16)|('S'<<8)|' ')
#define RTF  (('R'<<24)|('T'<<16)|('F'<<8)|' ')
#define TIFF (('T'<<24)|('I'<<16)|('F'<<8)|'F')
#define PICT (('P'<<24)|('I'<<16)|('C'<<8)|'T')

id ROMlib_pasteboard = 0;
int ROMlib_ourchangecount;

void PutScrapX(long type, long length, char *p)
{
    static int count = 0;
    static int textcount = 0, epscount = 0, rtfcount = 0, tiffcount = 0,
							  pictcount = 0;
    static char *textdata, *epsdata, *rtfdata, *tiffdata, *pictdata;
    static long textlength, epslength, rtflength, tifflength, pictlength;
    char doit;
    char const *types[5];
    int i;
    virtual_int_state_t block;
    long fonttblextra;

    SETUPA5;
    block = block_virtual_ints ();
    if (!ROMlib_pasteboard)
	ROMlib_pasteboard = [Pasteboard new];
    doit = NO;
    switch (type) {
    case TEXT:
	if (count == textcount)
	    ++count;
	textcount = count;
	if (textdata)
	    free(textdata);
	textdata = malloc(length);
	convertreturns(p, textdata, length, MacToUNIX);
	convertchars(textdata, length, mactonext);
	textlength = length;
	doit = YES;
	break;
    case EPS:
	if (count == epscount)
	    ++count;
	epscount = count;
	if (epsdata)
	    free(epsdata);
	epsdata = malloc(length);
	convertreturns(p, epsdata, length, MacToUNIX);
	epslength = length;
	doit = YES;
	break;
    case RTF:
	if (count == rtfcount)
	    ++count;
	rtfcount = count;
	if (rtfdata)
	    free(rtfdata);


	fonttblextra = insertfonttbl((char **) 0, (char) 0);

	rtfdata = malloc(length + fonttblextra);
	rtflength = convertreturns(p, rtfdata, length, MacRTFToUNIX);
	convertchars(rtfdata, rtflength, mactonext);
	doit = YES;
	break;
    case TIFF:
	if (count == tiffcount)
	    ++count;
	tiffcount = count;
	if (tiffdata)
	    free(tiffdata);
	tiffdata = malloc(length);
	bcopy(p, tiffdata, length);
	tifflength = length;
	doit = YES;
	break;
    case PICT:
	if (count == pictcount)
	    ++count;
	pictcount = count;
	if (pictdata)
	    free(pictdata);
	pictdata = malloc(length);
	bcopy(p, pictdata, length);
	pictlength = length;
	doit = YES;
	break;
    default:
	;
    }

    if (doit) {
	i = -1;
	if (rtfcount  == count)
	    types[++i] = NXRTFPboardType;
	if (epscount  == count)
	    types[++i] = NXPostScriptPboardType;
	if (textcount == count)
	    types[++i] = NXAsciiPboardType;
	if (tiffcount == count)
	    types[++i] = NXTIFFPboardType;
	if (pictcount == count)
	    types[++i] = ARDIPICTPboardType;
	[ROMlib_pasteboard declareTypes:types num:i+1 owner:0];
	if (rtfcount  == count)
	    [ROMlib_pasteboard writeType:NXRTFPboardType data:rtfdata
							     length:rtflength];
	if (epscount  == count)
	    [ROMlib_pasteboard writeType:NXPostScriptPboardType data:epsdata
							     length:epslength];
	if (textcount == count)
	    [ROMlib_pasteboard writeType:NXAsciiPboardType data:textdata
							    length:textlength];
	if (tiffcount == count)
	    [ROMlib_pasteboard writeType:NXTIFFPboardType data:tiffdata
							    length:tifflength];
	if (pictcount == count)
	    [ROMlib_pasteboard writeType:ARDIPICTPboardType data:pictdata
							    length:pictlength];
    }
    ROMlib_ourchangecount = [ROMlib_pasteboard changeCount];
    restore_virtual_ints (block);
    RESTOREA5;
}

/*
 * NOTE:  ROMlib_ReallocHandle is just calls ReallocHandle
 *        but ROMlib_ReallocHandle has normal calling conventions.
 *	  ReallocHandle is really a macro that we can't include in
 *	  this pristine, ROMlib ignorant, file.
 */

extern short MemErr;
#define noErr 0

long GetScrapX(long type, char **h)
{
    const NXAtom *types;
    char *data;
    int length;
    NXAtom tofind;
    long retval;
    int temp;
    extern long ROMlib_ZeroScrap( void );
    virtual_int_state_t block;

    SETUPA5;
    block = block_virtual_ints ();
    if (!ROMlib_pasteboard)
	ROMlib_pasteboard = [Pasteboard new];
    retval = -1;
    switch (type) {
    case TEXT:
	tofind = NXAsciiPboardType;
	break;
    case RTF:
	tofind = NXRTFPboardType;
	break;
    case EPS:
	tofind = NXPostScriptPboardType;
	break;
    case TIFF:
	tofind = NXTIFFPboardType;
	break;
    case PICT:
	tofind = ARDIPICTPboardType;
	break;
    default:
	tofind = 0;
	break;
    }
    if (tofind &&
	    (temp = [ROMlib_pasteboard changeCount]) > ROMlib_ourchangecount) {
	types = [ROMlib_pasteboard types];
	while (*types && strcmp(*types, tofind) != 0)
	    ++types;
	if (*types &&
	       [ROMlib_pasteboard readType:*types data:&data length:&length]) {
	    if (tofind == NXRTFPboardType) {
		ROMlib_ReallocHandle(h, length * 2);
		if (MemErr != noErr) {
		    retval = -1;
/*-->*/		    goto DONE;
		}
		retval = convertreturns(data, CL(*h), length, UNIXRTFToMac);
		convertchars(CL(*h), retval, nexttomac);
		ROMlib_ReallocHandle(h, retval);
		if (MemErr != noErr)
		    retval = -1;
/*-->*/ 	goto DONE;
	    } else {
		ROMlib_ReallocHandle(h, length);
		if (MemErr != noErr) {
		    retval = -1;
/*-->*/		    goto DONE;
		}
		if (tofind != NXTIFFPboardType && tofind != ARDIPICTPboardType)
		    convertreturns(data, CL(*h), length, UNIXToMac);
		else
		    bcopy(data, CL(*h), length);
		if (tofind == NXAsciiPboardType)
		    convertchars(CL(*h), length, nexttomac);
	    }
	    vm_deallocate(task_self(), (vm_address_t) data, length);
	    retval = length;
	} else {
	    ROMlib_ZeroScrap();
	}
    }
DONE:
    restore_virtual_ints (block);
    RESTOREA5;
    return retval;
}

long *ROMlib_shadowScreen;
long ROMlib_width_slop;


- (void) resetscreensize
{
    extern short ROMlib_constrained;
    long nbytes;
    int fwidth;
    NXSize size;

    ROMlib_constrained = YES;
    [screenimage removeRepresentation:screenimagerep];
    mac_to_next_x = curr_width / orig_width;
    mac_to_next_y = curr_height / orig_height;
    theonlyscreenimagerep =
    screenimagerep = [NXBitmapImageRep allocFromZone:[self zone]];

#if 0
    if ((orig_width % 16) && ((orig_width % 16) < 9)) {	/* need padding } */
	fwidth = (orig_width+15) / 16 * 16;
#else
    if (orig_width % 2) {
      fwidth = orig_width + 1;
#endif
	ROMlib_width_slop = fwidth - orig_width;
	[screenimage getSize:&size];
	size.width  += ROMlib_width_slop * size.width / orig_width;
	[screenimage setSize:&size];
    } else
	fwidth = orig_width;

    ROMlib_checkscreen();
    if (ROMlib_nextscreen == SCREEN_MONO) {
      [screenimagerep initData:0 pixelsWide:fwidth pixelsHigh:orig_height
		 bitsPerSample:2 samplesPerPixel:1 hasAlpha:NO isPlanar:NO
		 colorSpace:NX_OneIsWhiteColorSpace bytesPerRow:0
		 bitsPerPixel:2];
      ROMlib_realScreenRowBytes = (fwidth+7) / 8 * 2;
    } else {
      [screenimagerep initData:0 pixelsWide:fwidth pixelsHigh:orig_height
		 bitsPerSample:4 samplesPerPixel:3 hasAlpha:NO isPlanar:NO
		 colorSpace:NX_RGBColorSpace bytesPerRow:0
		 bitsPerPixel:16];
      ROMlib_realScreenRowBytes = fwidth;  /* 1 byte per pixel */
    }

    nbytes = [screenimagerep bytesPerRow] * orig_height;
    memset ([screenimagerep data], ~0, nbytes);
    free(ROMlib_shadowScreen);
    free(ROMlib_realScreen);
    ROMlib_shadowScreen = (long *) malloc(ROMlib_realScreenRowBytes * orig_height);
    ROMlib_realScreen = (long *) malloc(ROMlib_realScreenRowBytes * orig_height);
    memset (ROMlib_realScreen, 0, ROMlib_realScreenRowBytes * orig_height);
    drawing_invalid_p = 0;

    [screenimage useRepresentation:screenimagerep];
    host_reset_graphics();
}

void ROMlib_resetscreensize( void )
{
    virtual_int_state_t block;

    block = block_virtual_ints ();
    [theonlyframe resetscreensize];
    restore_virtual_ints (block);
}

extern id global_menu;

extern id global_sptext;
extern id global_starsptext;
extern id global_pctext;
extern id global_starpctext;
extern id global_d0text;
extern id global_pstext;
extern id global_sigtext;
extern id global_debtext;
extern id global_debtable;
extern id global_deathwindow;
extern id global_commenttext;
extern id global_deathMenuCell;

void querypointerX(long *xp, long *yp, long *notused)
{
    NXPoint p;
    virtual_int_state_t block;

    block = block_virtual_ints ();
    [global_gameWindow getMouseLocation:&p];
    [theonlyframe convertPoint:&p fromView:nil];
    *xp = p.x * (1 / mac_to_next_x);
    *yp = (curr_height - p.y) * (1 / mac_to_next_y);
    restore_virtual_ints (block);
}

void showcursorX(long show)
{
    virtual_int_state_t block;

    block = block_virtual_ints ();
    if (show)
	[realcursor set];
    else
	[blankcursor set];
    restore_virtual_ints (block);
}

#undef gray  /* hack! */
void setcursorX(short *data, short *mask, long hotx, long hoty)
{
    extern char CrsrVis;
    char *datap;
    short i;
    short mymask[16], mydata[16], gray;
    NXPoint p;
    static beenhere = NO;
    virtual_int_state_t block;

    block = block_virtual_ints ();
    if (!beenhere) {
	[global_gameWindow disableCursorRects];
	beenhere = YES;
    }
    gray = 0x5555;
    for (i = 0; i < 16; ++i) {
	mymask[i] = ~(data[i] | mask[i]);
	gray = ~gray;
	mydata[i] = data[i] & ~(gray & (~mask[i] & data[i]));
    }
	
    datap = (char *) [cursorrep data];
    bcopy(mydata, datap   , 32);
    bcopy(mymask, datap+32, 32);
    [cursorimage recache];
    if (CrsrVis)
	[blankcursor set];

    [realcursor setImage:cursorimage];
    p.x = hotx;
    p.y = hoty;
    [realcursor setHotSpot:&p];

    if (CrsrVis)
	[realcursor set];
    restore_virtual_ints (block);
}

/*
 * ROMlib_printtimeout is specifically for Excel, when you try to
 * print an empty page.  Excel starts the printing process and then
 * writes an message to the screen, informing you that you have a
 * blank worksheet.  This doesn't in with our printing paradigm, so
 * ROMlib_printtimeout is a hack to solve this one instance of the
 * general class of printing problems.
 *
 * ROMlib_printtimeout is positive when we're counting context
 *	switches, looking for an OpenPage.
 *
 * ROMlib_printtimeout is zero if we've timed out, but haven't
 *	yet finished printing from the NeXT's perspective.
 *
 * ROMlib_printtimeout is negative when we've finished printing.
 *	This will result in the screen being updated once we
 *	drop back into the Mac universe.
 */

long ROMlib_printtimeout = 10000;	/* any positive number will do */

- step
{
    extern char *nextstep_sp, *romlib_sp;
    NXEvent dummyEvent;
    
    [self lockFocus];
    
    do {
	contextswitch(&nextstep_sp, &romlib_sp);
    } while (printstate != seenOpenDoc && printstate != seenPageSetUp &&
		   [NXApp peekNextEvent:NX_ALLEVENTS into:&dummyEvent] == NULL);

    [self unlockFocus];
    {
	SETUPA5;
	if (printstate == seenOpenDoc) {
	    [self printPSCode:self];
	    if (ROMlib_printtimeout == 0)
		ROMlib_printtimeout = -1;
	} else if (printstate == seenPageSetUp) {
	    [NXApp runPageLayout:self];
	    printstate = __idle;
	}
	RESTOREA5;
    }

    return self;
}

static void timerswitch(DPSTimedEntry notused, double timenow, void *data)
{
    static double oldnow;

    SETUPA5;
    if (oldnow == 0.0 || timenow - oldnow > 5*50) {
	protectus(0, 0);
	oldnow = timenow;
    }
    /* might be useful to look at timenow and update various low memory
       globals */
    [(id)data step];
    RESTOREA5;
}

- initFrame:(const NXRect *)frm
{
    NXSize size;
    short *maskp;
    char *datap;
    int bytecount;
    short i;
    long nbytes;

    orig_height = frm->size.height;
    orig_width  = frm->size.width;
    curr_height = orig_height;
    curr_width  = orig_width;

    [super initFrame:frm];
    

    [self setClipping:NO];
    [self setOpaque:YES];
    [self allocateGState];	// For faster lock/unlockFocus
    
/* make the screen */

    theonlyscreenimagerep =
    screenimagerep = [NXBitmapImageRep allocFromZone:[self zone]];

    ROMlib_checkscreen();
    if (ROMlib_nextscreen == SCREEN_MONO) {
      [screenimagerep initData:0 pixelsWide:orig_width pixelsHigh:orig_height
		 bitsPerSample:2 samplesPerPixel:1 hasAlpha:NO isPlanar:NO
		 colorSpace:NX_OneIsWhiteColorSpace bytesPerRow:0
		 bitsPerPixel:2];
      ROMlib_realScreenRowBytes = (orig_width + 7) / 8 * 2;
    } else {
      [screenimagerep initData:0 pixelsWide:orig_width pixelsHigh:orig_height
		 bitsPerSample:4 samplesPerPixel:3 hasAlpha:NO isPlanar:NO
		 colorSpace:NX_RGBColorSpace bytesPerRow:0
		 bitsPerPixel:16];
      ROMlib_realScreenRowBytes = (orig_width + 1) & ~1;  /* 1 byte per pixel */
    }
    nbytes = [screenimagerep bytesPerRow] * orig_height;
    memset ([screenimagerep data], ~0, nbytes);

    theonlyscreenimage =
    screenimage = [[NXImage allocFromZone:[self zone]] initSize:&frm->size];
    [screenimage setDataRetained:YES];
    [screenimage setScalable:YES];	/* may be faster to set NO until
					   someone resizes */
    [screenimage useRepresentation:screenimagerep];

    ROMlib_shadowScreen = (long *) malloc(ROMlib_realScreenRowBytes * orig_height);
    ROMlib_realScreen = (long *) malloc(ROMlib_realScreenRowBytes * orig_height);
    memset (ROMlib_realScreen, 0, ROMlib_realScreenRowBytes * orig_height);

    drawing_invalid_p = 0;

/* make the cursor */

    cursorrep = [NXBitmapImageRep allocFromZone:[self zone]];
    [cursorrep initData:0 pixelsWide:16 pixelsHigh:16
		 bitsPerSample:1 samplesPerPixel:2 hasAlpha:YES isPlanar:YES
		 colorSpace:NX_OneIsBlackColorSpace bytesPerRow:0
		 bitsPerPixel:0];
    bzero([cursorrep data], [cursorrep bytesPerRow] * 16 * 2);

    size.width = size.height = 16;
    cursorimage = [[NXImage allocFromZone:[self zone]] initSize:&size];
    [cursorimage setDataRetained:YES];
    [cursorimage setScalable:YES];	/* may be faster to set NO until
					   someone resizes */
    [cursorimage useRepresentation:cursorrep];
    realcursor = [NXCursor allocFromZone:[self zone]];
    [realcursor initFromImage:cursorimage];

/* make the blank cursor (for when it's hidden) */

    blankcursorrep = [NXBitmapImageRep allocFromZone:[self zone]];
    [blankcursorrep initData:0 pixelsWide:16 pixelsHigh:16
		 bitsPerSample:1 samplesPerPixel:2 hasAlpha:YES isPlanar:YES
		 colorSpace:NX_OneIsBlackColorSpace bytesPerRow:0
		 bitsPerPixel:0];
    
    bzero(datap = (char *) [blankcursorrep data],
				bytecount = [blankcursorrep bytesPerRow] * 16);
    maskp = (short *) (datap + bytecount);
    for (i = 16; --i >= 0;)
	*maskp++ = 0xFFFF;

    size.width = size.height = 16;
    blankcursorimage = [[NXImage allocFromZone:[self zone]] initSize:&size];
    [blankcursorimage setDataRetained:YES];
    [blankcursorimage setScalable:YES];	/* may be faster to set NO until
					   someone resizes */
    [blankcursorimage useRepresentation:blankcursorrep];
    blankcursor = [NXCursor allocFromZone:[self zone]];
    [blankcursor initFromImage:blankcursorimage];

    timer = DPSAddTimedEntry(0, &timerswitch, self, NX_BASETHRESHOLD);

    return theonlyframe = self;
}

void ROMlib_nextscreeninfo(short *rowbytep, short *bottomp, short *rightp,
								  char **addrp)
{
    virtual_int_state_t block;

    block = block_virtual_ints ();
    *rowbytep = ROMlib_realScreenRowBytes;
    *bottomp = [theonlyscreenimagerep pixelsHigh];
    *rightp = orig_width;
    *addrp = (char *) ROMlib_realScreen /*[theonlyscreenimagerep data]*/;
    restore_virtual_ints (block);
}

void ROMlib_SetTitle(char *newtitle)
{
    virtual_int_state_t block;

    block = block_virtual_ints ();
    [global_gameWindow setTitle:newtitle];
    restore_virtual_ints (block);
}

/*
 * NOTE: ROMlib_SetSize should only be called before the application is
 *	 truly initialized (i.e. before *it* calls InitGraf).
 *
 *	 Right now ParseConfigFile() is the only thing that calls
 *       ROMlib_SetSize.
 */

void ROMlib_SetSize(pair_t *pairp, pair_t *p2)
{
    virtual_int_state_t block;

    block = block_virtual_ints ();
    curr_width  = pairp->first;
    curr_height = pairp->second;
    orig_width  = p2->first;
    orig_height = p2->second;
    ROMlib_resetscreensize();
    [global_gameWindow sizeWindow: curr_width : curr_height];
    restore_virtual_ints (block);
}

void ROMlib_SetLocation(pair_t *pairp)
{
    NXSize size;
    virtual_int_state_t block;

    block = block_virtual_ints ();
    [NXApp getScreenSize:&size];
    [global_gameWindow moveTopLeftTo:pairp->first:size.height - pairp->second];
    restore_virtual_ints (block);
}

void ROMlib_HideScreen(void)
{
    virtual_int_state_t block;

    block = block_virtual_ints ();
    [global_gameWindow orderOut:0];
    restore_virtual_ints (block);
}

void ROMlib_ShowScreen(void)
{
    virtual_int_state_t block;

    block = block_virtual_ints ();
    [global_gameWindow makeKeyAndOrderFront:0];
    restore_virtual_ints (block);
}

typedef enum { hexfield, unknownfield, signalfield, decimalfield} field_t;

static void deathfield(id theid, field_t fieldtype, long value, const char *pre)
{
    char buf[80], *next;
    char *signames[] = {
	"HUP",  "INT",    "QUIT", "ILL",   "TRAP", "IOT",  "EMT",  "FPE",
	"KILL", "BUS",    "SEGV", "SYS",   "PIPE", "ALRM", "TERM", "URG",
	"STOP", "TSTP",   "CONT", "CHLD",  "TTIN", "TTOU", "IO",   "XCPU",
	"XFSZ", "VTALRM", "PROF", "WINCH", "LOST", "USR1", "USR2",
    };

    sprintf(buf, "%s = ", pre);
    next = buf + strlen(buf);
    switch (fieldtype) {
	case hexfield:
	    sprintf(next, "0x%08lX", value);
	    break;
	case signalfield:
	    if (value >= 1 && value <= 31) {
		sprintf(next, "SIG%s", signames[value-1]);
		break;
	    }
	    /* else FALL THROUGH */
	case unknownfield:
	    sprintf(next, "**********");
	    break;
	case decimalfield:
	    sprintf(next, "%ld", value);
	    break;
    }
    [theid setStringValue:buf];
}

static jmp_buf jbuf;

static void badaddr( void )
{
    longjmp(jbuf, 1);
}

/*
 * fetchlongat has the nasty side effect of clobbering signal vectors,
 * but since it's only called as part of the deathwish, it doesn't matter.
 */

static int fetchlongat(long pc, long *contentsp)
{
    int retval;

    signal(SIGBUS,  (void *) badaddr);
    signal(SIGSEGV, (void *) badaddr);
    sigsetmask(sigsetmask(-1L) & ~(sigmask(SIGBUS)|sigmask(SIGSEGV)));
    if (setjmp(jbuf)) {
	retval = 0;	/* will only get here via longjmp */
    } else {
	*contentsp = *(long *)pc;	/* could cause buserror or segv */
	retval = 1;
    }
    return retval;
}

typedef struct {
    long when;
    unsigned short trapn;
    short filler;
} trapsortentry_t;

#if !defined(NITEMS)
#define NITEMS(x)	(sizeof((x)) / sizeof((x)[0]))
#endif

#if defined(BINCOMPAT)
static void deathtraps( void )
{
    trapsortentry_t lasttraps[24];
    char buf[512];	/* more than necessary */
    short i, j, ninthere, ntomove;
    extern long debugtable[1<<12];
    long newwhen;

    ninthere = 0;
    for (i = 0; i < NITEMS(lasttraps); ++i) {
	lasttraps[i].when = -1;
	lasttraps[i].trapn = 0;
    }
    for (i = 0; i < NITEMS(debugtable); ++i) {
	newwhen = debugtable[i];
	for (j = 0; j < ninthere; ++j)
	    if (newwhen > lasttraps[j].when)
/*-->*/		break;
	ntomove = ninthere - j - 1;
	if (ntomove > 0)
	    bcopy((char *) &lasttraps[j], (char *) &lasttraps[j+1],
					       sizeof(lasttraps[0]) * ntomove);
	if (j < NITEMS(lasttraps)) {
	    lasttraps[j].when = newwhen;
	    lasttraps[j].trapn = 0xA000 + i;
	    if (ninthere < NITEMS(lasttraps))
		++ninthere;
	}
    }
    sprintf(buf, "%04X, %04X, %04X\n"
		 "%04X, %04X, %04X\n"
		 "%04X, %04X, %04X\n"
		 "%04X, %04X, %04X\n"
		 "%04X, %04X, %04X\n"
		 "%04X, %04X, %04X\n"
		 "%04X, %04X, %04X\n"
		 "%04X, %04X, %04X",
		 lasttraps[ 0].trapn, lasttraps[ 1].trapn, lasttraps[ 2].trapn,
		 lasttraps[ 3].trapn, lasttraps[ 4].trapn, lasttraps[ 5].trapn,
		 lasttraps[ 6].trapn, lasttraps[ 7].trapn, lasttraps[ 8].trapn,
		 lasttraps[ 9].trapn, lasttraps[10].trapn, lasttraps[11].trapn,
		 lasttraps[12].trapn, lasttraps[13].trapn, lasttraps[14].trapn,
		 lasttraps[15].trapn, lasttraps[16].trapn, lasttraps[17].trapn,
		 lasttraps[18].trapn, lasttraps[19].trapn, lasttraps[20].trapn,
		 lasttraps[21].trapn, lasttraps[22].trapn, lasttraps[23].trapn);
    [global_debtable setStringValue:buf];
}
#endif defined(BINCOMPAT)

#if !defined(NELEM)
#define NELEM(x)	(sizeof((x)) / sizeof((x)[0]))
#endif

typedef enum { appnotstarted, appisrunning, appdied } appstate_t;

static appstate_t appstate = appnotstarted;

void ROMlib_startapp( void )
{
    id abortcell;

    abortcell = [global_menu findCellWithTag:11];
    [abortcell setTitleNoCopy:"Abort..."];
    appstate = appisrunning;
}

#if defined(BINCOMPAT)
static long addrof( unsigned short trapno )
{
    return NGetTrapAddress(mostrecenttrap,
				 mostrecenttrap & TOOLBIT ? ToolTrap : OSTrap);
}
#endif

/*
 * TODO: probably it make sense to context switch before doing any
 *	 NeXT stuff... we could set a flag and then context switch and then
 *	 look at the flag...
 */

static long sp, pc, starsp, starpc, d0, psl, sig;
static field_t starspfield, starpcfield;

void ROMlib_death_by_signal( long wsignal, long code, struct sigcontext *scp )
{
    extern char *nextstep_sp, *romlib_sp;
    char buf[512];
#if defined(BINCOMPAT)
    extern unsigned short mostrecenttrap;
    extern char _UNKNOWN;
#endif
    struct itimerval toset;
    id abortcell;

    SETUPA5;
    toset.it_value.tv_sec     = 0;
    toset.it_value.tv_usec    = 0;
    toset.it_interval.tv_sec  = 0;
    toset.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &toset, (struct itimerval *) 0);

    DPSRemoveTimedEntry(timer);


#if defined(mc68000)
    if (fetchlongat(scp->sc_sp, &starsp))
	starspfield = hexfield;
    else
	starspfield = unknownfield;

    if (fetchlongat(scp->sc_pc, &starpc))
	starpcfield = hexfield;
    else
	starpcfield = unknownfield;

    sp  = scp->sc_sp;
    pc  = scp->sc_pc;
    d0  = scp->sc_d0;
    psl = scp->sc_ps;
#elif defined(i386)	|| defined(i486)
    if (fetchlongat(scp->sc_esp, &starsp))
	starspfield = hexfield;
    else
	starspfield = unknownfield;

    if (fetchlongat(scp->sc_eip, &starpc))
	starpcfield = hexfield;
    else
	starpcfield = unknownfield;

    sp  = scp->sc_esp;
    pc  = scp->sc_eip;
    d0  = scp->sc_eax;
    psl = scp->sc_eflags;
#else
    This will not compile
#endif
    sig = wsignal;

    deathfield(global_sptext,	   hexfield,	  sp,	       " SP");
    deathfield(global_starsptext,  starspfield,	  starsp,      "*SP");
    deathfield(global_pctext,	   hexfield,	  pc,	       " PC");
    deathfield(global_starpctext,  starpcfield,	  starpc,      "*PC");
    deathfield(global_d0text,	   hexfield,	  d0,          " D0");
    deathfield(global_pstext,	   hexfield,	  psl,         "PSL");
    deathfield(global_sigtext,	   signalfield,	  wsignal,     "SIG");
#if defined(BINCOMPAT)
    deathfield(global_debtext,	   decimalfield,  debugnumber, "deb");
    deathtraps();
#endif
    switch (wsignal) {
	case SIGHUP:
	case SIGINT:
	case SIGQUIT:
	case SIGKILL:
	case SIGTERM:
	case SIGSTOP:
	case SIGTSTP:
	case SIGCONT:
	case SIGCHLD:
	    strcpy(buf,
		"The death was generated externally from Executor."
		"  Either"
		" you killed Executor, or someone using your account or root"
		" did so.");
	    break;

	case SIGILL:
#if defined(BINCOMPAT) && defined(mc68000)
	    if (*(short *) scp->sc_pc == 0x4AFC) {
		if (scp->sc_pc == (long) &_UNKNOWN) {
		    sprintf(buf, "The program running attempted to execute an"
				 " A-line [0x%x] trap for which ARDI does not"
				 " have documentation.", mostrecenttrap);
		} else {
		    if (addrof(mostrecenttrap) == addrof(0xA89F))
			sprintf(buf, "The program running attempted to execute"
			     " an A-line trap [0x%x] that we do not currently"
			     " support"
			     ".", mostrecenttrap);
		    else
			sprintf(buf, "An internal error has been detected.");
		}
		break;
	    } /* else FALL THROUGH */
#endif

	case SIGTRAP:
	case SIGIOT:
	case SIGEMT:
	case SIGFPE:
	    strcpy(buf,
		"This death is surprising.  The fault was not caused"
		" by a non-32 bit clean program, but it may have been caused"
		" by MC68040 cache problems."
		);
	    break;

	case SIGBUS:
	case SIGSEGV:
	    strcpy(buf,
		"Several things could have caused this death.  Non-32 bit"
		" clean applications are the most common cause of this."
		);
	    break;

	case SIGSYS:
	case SIGPIPE:
	case SIGALRM:
	case SIGURG :
	case SIGTTIN:
	case SIGTTOU:
	case SIGIO:
	case SIGVTALRM:
	case SIGPROF:
	case SIGWINCH:
	case SIGLOST:
	default:
	    strcpy(buf,
		"Executor received a very surprising signal.  If this death"
		" is reproducible, you should notify ARDI so it can be fixed.");
	    break;

	case SIGXCPU:
	    strcpy(buf, "You have a CPU limit, and Executor exceeded it.");
	    break;

	case SIGXFSZ:
	    strcpy(buf,
		      "You have a File size limit, and Executor exceeded it.");
	    break;
    }
    [global_commenttext setStringValue:buf];
    [global_deathwindow orderFront:0];
    abortcell = [global_menu findCellWithTag:11];
    [abortcell setTitleNoCopy:"Abort"];
    [global_deathMenuCell setEnabled:YES];
    appstate = appdied;
    RESTOREA5;
#if defined(mc68000)
    if (((unsigned long )scp->sc_sp & 0xFFFF0000) ==	/* not the best test */
	 ((unsigned long) romlib_sp & 0xFFFF0000))
#endif
	contextswitch(&romlib_sp, &nextstep_sp);
}

// The following allows MacViewClass to grab the mousedown event that activates
// the window. By default, the View's acceptsFirstMouse returns NO.

- (BOOL)acceptsFirstMouse
{
#if 1
    return YES;
#else
    return NO;
#endif
}

- (BOOL) acceptsFirstResponder
{
    return YES;
}

- sizeTo:(NXCoord) width :(NXCoord)height
{
    NXRect trackrect;
    NXSize size;

    size.width = width;
    size.height = height;
    curr_height = height;
    curr_width  = width;
    mac_to_next_x = width / orig_width;
    mac_to_next_y = height / orig_height;
    [super sizeTo:width :height];
    size.width += ROMlib_width_slop * size.width / orig_width;
    [screenimage setSize:&size];
    trackrect.origin.x = 0;
    trackrect.origin.y = 0;
    trackrect.size = size;
    [self convertRect:&trackrect toView:nil];
    [[self window] setTrackingRect:&trackrect inside:YES owner:self tag:1
							     left:NO right:NO];

    /* HACK! */
    drawing_invalid_p = 1;

    return self;
}

- mouseDown:(NXEvent *)eventp
{
    SETUPA5;
    [self convertPoint:&eventp->location fromView:nil];
    postnextevent(eventp);
    RESTOREA5;

    return self;
}

- mouseMoved:(NXEvent *)eventp
{
    SETUPA5;
    [self convertPoint:&eventp->location fromView:nil];
    ROMlib_updatemouselocation(eventp);
    RESTOREA5;
    return self;
}

- mouseDragged:(NXEvent *)eventp
{
    SETUPA5;
    [self convertPoint:&eventp->location fromView:nil];
    ROMlib_updatemouselocation(eventp);
    RESTOREA5;
    return self;
}

- mouseUp:(NXEvent *)eventp
{
    SETUPA5;
    [self convertPoint:&eventp->location fromView:nil];
    postnextevent(eventp);
    RESTOREA5;

    return self;
}

- mouseEntered:(NXEvent *)eventp
{
    extern char CrsrVis;

    SETUPA5;
    if (CrsrVis)
	[realcursor set];
    else
	[blankcursor set];
    [self convertPoint:&eventp->location fromView:nil];
    postnextevent(eventp);
    RESTOREA5;
    return self;
}

- mouseExited:(NXEvent *)eventp
{
    SETUPA5;
    [NXArrow set];
    [self convertPoint:&eventp->location fromView:nil];
    postnextevent(eventp);
    RESTOREA5;
    return self;
}

- keyDown:(NXEvent *)eventp
{
    SETUPA5;
    [self convertPoint:&eventp->location fromView:nil];
    postnextevent(eventp);
    RESTOREA5;
    return self;
}

static struct {
    long mask;		    char key;
} maskkeys[] = {
    { NX_ALPHASHIFTMASK,	0x39, },
    { NX_SHIFTMASK,		0x38, },
    { NX_CONTROLMASK,		0x3B, },
    { NX_ALTERNATEMASK,		0x3A, },
    { NX_COMMANDMASK,		0x37, },
};

- flagsChanged:(NXEvent *)eventp
{
    int i;
    extern void ROMlib_zapmap( long key, long value );
    extern short ROMlib_mods;
    extern short ROMlib_next_butmods_to_mac_butmods(long flags);

    SETUPA5;
    for (i = NELEM(maskkeys); --i >= 0;) {
	ROMlib_zapmap(maskkeys[i].key, !!(eventp->flags & maskkeys[i].mask));
    }
    /* If shift is down, assume caps lock is not.  The NeXT doesn't give us
     * a separate bit for just caps lock, which we need for Solarian.  This
     * will give us a decent (but imperfect) approximation...
     */
    if (eventp->flags & NX_SHIFTMASK)
	ROMlib_zapmap(0x39, 0);
    ROMlib_mods = ROMlib_next_butmods_to_mac_butmods(eventp->flags);
    RESTOREA5;
    return self;
}

- (BOOL) performKeyEquivalent:(NXEvent *)eventp
{
    if (appstate == appisrunning) {
	SETUPA5;
	[self convertPoint:&eventp->location fromView:nil];
	postnextevent(eventp);
	RESTOREA5;
	return YES;
    } else {
	if (debugnumber && (eventp->data.key.charCode == 'M')) {
	    [mailPanel makeKeyAndOrderFront:0];
	    return YES;
	} else
	    return [super performKeyEquivalent:eventp];
    }
}

- keyUp:(NXEvent *)eventp
{
    SETUPA5;
    [self convertPoint:&eventp->location fromView:nil];
    postnextevent(eventp);
    RESTOREA5;
    return self;
}

- drawSelf:(NXRect *)rects :(int)rectCount 
{
    NXRect r, *rp;
    extern char *nextstep_sp, *romlib_sp;
    int i;
    long top, left, bottom, right;
    SETUPA5;

    switch (printstate) {
    case __idle:
	if (0 && ROMlib_accstate == acc_accelerated) {
	    i = rectCount;
	    rp = rects;
	    if (i > 1) {
		--i;	/* skip first rectangle, since it's the bounding */
		++rp;	/* rectangle */
	    }
	    while (--i >= 0) {
		top    = rp->origin.y;
		left   = rp->origin.x;
		bottom = top  + rp->size.height;
		right  = left + rp->size.width;
		accelerate(top, left, bottom, right);
	    }
	} else {
	    r.origin.x    = 0;
	    r.origin.y    = 0;
	    r.size.width  = curr_width +
				   ROMlib_width_slop * curr_width / orig_width;
	    r.size.height = curr_height;

	    update_screen_image_rep (0, 0, curr_width, curr_height);

	    [theonlyscreenimagerep drawIn:&r];
	}
	break;
    case seenOpenPage:
	RESTOREA5;
	do
	    contextswitch(&nextstep_sp, &romlib_sp);
	while (printstate != __idle && printstate != seenClosePage);
	goto avoidrestorea5;
	break;
    default:
	/* seenClosePage may get us in here, but we can't do anything
		without sploding (I think) */
	break;
    }

    RESTOREA5;
avoidrestorea5:
    return self;
}

- validRequestorForSendType:(NXAtom)typeSent andReturnType:(NXAtom)typeReturned
{
    return self;
}

- readSelectionFromPasteboard:pboard
{
    extern char *nextstep_sp, *romlib_sp;
    id saveROMlib_pasteboard;

    {
	SETUPA5;
	sendsuspendevent();
	RESTOREA5;
    }
    contextswitch(&nextstep_sp, &romlib_sp);
    contextswitch(&nextstep_sp, &romlib_sp);
    contextswitch(&nextstep_sp, &romlib_sp);
    saveROMlib_pasteboard = ROMlib_pasteboard;
    ROMlib_pasteboard = pboard;
    {
	SETUPA5;
	sendresumeevent(YES);
	RESTOREA5;
    }
    contextswitch(&nextstep_sp, &romlib_sp);
    contextswitch(&nextstep_sp, &romlib_sp);
    contextswitch(&nextstep_sp, &romlib_sp);
    ROMlib_pasteboard = saveROMlib_pasteboard;
    {
	SETUPA5;
	sendpaste();
	RESTOREA5;
    }
    contextswitch(&nextstep_sp, &romlib_sp);
    contextswitch(&nextstep_sp, &romlib_sp);
    contextswitch(&nextstep_sp, &romlib_sp);
    return self;
}

- (BOOL)writeSelectionToPasteboard:pboard types:(NXAtom *)types
{
    extern char *nextstep_sp, *romlib_sp;
    id saveROMlib_pasteboard;
    
    {
	SETUPA5;
	sendcopy();
	RESTOREA5;
    }
    contextswitch(&nextstep_sp, &romlib_sp);
    contextswitch(&nextstep_sp, &romlib_sp);
    contextswitch(&nextstep_sp, &romlib_sp);
    saveROMlib_pasteboard = ROMlib_pasteboard;
    ROMlib_pasteboard = pboard;
    {
	SETUPA5;
	sendsuspendevent();
	RESTOREA5;
    }
    contextswitch(&nextstep_sp, &romlib_sp);
    contextswitch(&nextstep_sp, &romlib_sp);
    contextswitch(&nextstep_sp, &romlib_sp);
    ROMlib_pasteboard = saveROMlib_pasteboard;
    {
	SETUPA5;
	sendresumeevent(NO);
	RESTOREA5;
    }
    return YES;
}

- (BOOL)getRect:(NXRect *)theRect forPage:(int)page
{
    extern char *nextstep_sp, *romlib_sp;
    extern long pagewanted;

    pagewanted = page;
    *theRect = *[[NXApp printInfo] paperRect];
    ROMlib_printtimeout = 10000;
    while (printstate != __idle && printstate != seenOpenPage &&
						    --ROMlib_printtimeout != 0)
	contextswitch(&nextstep_sp, &romlib_sp);
    if (ROMlib_printtimeout == 0)
	printstate = __idle;
    return printstate == __idle ? NO : YES;
}

- (BOOL)knowsPagesFirst:(int *)firstPageNum last:(int *)lastPageNum
{
    return YES;
}

char ROMlib_needtorestore;
extern char ROMlib_suppressclip;

- beginPageSetupRect:(const NXRect  * )aRect 
	   placement:(const NXPoint *) location
{
    id retval;

    retval = [super beginPageSetupRect:aRect placement:location];
    ROMlib_needtorestore = 0;
    ROMlib_suppressclip = 0;

    return retval;
}

- addToPageSetup
{
    float scale;

    scale = [[NXApp printInfo] scalingFactor];
    PStranslate(0, [[NXApp printInfo] paperRect]->size.height);
    PSscale(1 * scale, -1 * scale);
/*
 * NOTE: we should probably check to see whether pse and psb are
 *	 defined before blowing them away, but then again, just
 *	 'cause they're defined doesn't mean they're what we want.
 */
    DPSPrintf(DPSGetCurrentContext(), "/pse {} def\n"
				      "/psb {} def\n"
				      "/execuserobject {\n"
				      "  dup UserObjects length ge\n"
				      "    { pop }\n"
				      "    { UserObjects exch get exec }\n"
				      "  ifelse\n"
				      "} def\n"
				      "/currentmouse { 0 0 } def\n"
				      "/printobject { pop pop } def\n"
				      );
    return [super addToPageSetup];
}

/*
 * NOTE: %% is needed to get just one %
 */

- endPageSetup
{
    id retval;

    retval = [super endPageSetup];

    DPSPrintf(DPSGetCurrentContext(),
		"%% The following is a lie that is necessary because Word 5\n"
	        "%% doesn't wrap EPS files properly and we can't tell where\n"
	        "%% the PostScript we're printing comes from.\n"
		"%%%%BeginDocument: IWishWeDidntHaveToDoThis\n");
    return retval;
}

- endPage
{
    if (ROMlib_needtorestore)
	PSgrestore();
    DPSPrintf(DPSGetCurrentContext(),
		"%%%%EndDocument\n"
	        "/showpage systemdict begin { showpage } bind end def\n");
    return [super endPage];
}

- endPSOutput
{
    extern long pagewanted;

    [super endPSOutput];
    pagewanted = 1024 * 1024;	/* i.e. real big */
    return self;
}


- pause:sender
{
    id pausecell;
    static char oldtitle[80];

    pausecell = [global_menu findCellWithTag:10];
    if ([pausecell title][0] == 'P') {
	DPSRemoveTimedEntry(timer);
	strncpy(oldtitle, [global_gameWindow title], sizeof(oldtitle));
	oldtitle[sizeof(oldtitle)-1] = 0;
	[global_gameWindow setTitle:"Executor is PAUSED"];
	[pausecell setTitleNoCopy:"Continue"];
    } else {
	[global_gameWindow setTitle:oldtitle];
	[pausecell setTitleNoCopy:"Pause"];
	timer = DPSAddTimedEntry(0, &timerswitch, self, NX_BASETHRESHOLD);
    }
    return self;
}

- abort:sender
{
    if (appstate != appisrunning ||
	NXRunAlertPanel("Are You Sure",
	"Hitting the \"Abort Anyway\" button will cause Executor "
	"to stop immediately, without giving the currently running "
	"program a chance to quit gracefully.  This can result in "
	"corrupted files.", "Cancel", "Abort Anyway", (const char *) 0) ==
							     NX_ALERTALTERNATE)
	[NXApp terminate:sender];
    return self;
}

- endPrologue
{
    extern int pageno;

    pageno = 0;
    DPSPrintf(DPSGetCurrentContext(),
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
"    /questiondown	/exclamdown	/logicalnot	/radical\n"
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
"/findfont\n"
"{\n"
"    /currentshared where\n"
"        {pop currentshared}\n"
"	{FALSE}\n"
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
"			    { FALSE }\n"
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
"    exch findfont\n"
"    exch dup type /arraytype eq\n"
"        {makefont}\n"
"	{scalefont}\n"
"    ifelse\n"
"    setfont\n"
"} bind def\n"
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
"%% } if\n");
    return [super endPrologue];
}

/* Beginning of stuff that should probably be in its own file */

#include <mach/mach.h>

typedef struct {
    const char *name;
    int subtype;
} subtype_entry_t;

static subtype_entry_t vax_cpu[] = {
    "VAX780",		1,
    "VAX785",		2,
    "VAX750",		3,
    "VAX730",		4,
    "UVAXI",		5,
    "UVAXII",		6,
    "VAX8200",		7,
    "VAX8500",		8,
    "VAX8600",		9,
    "VAX8650",		10,
    "VAX8800",		11,
    "UVAXIII",		12,
    (const char *) 0,	0,
};


static subtype_entry_t romp_cpu[] = {
    "RT_PC",		1,
    "RT_APC",		2,
    "RT_135",		3,
    (const char *) 0,	0,
};

static subtype_entry_t ns32032_cpu[] = {
    "MMAX_DPC",		1,
    "SQT	",	2,
    "MMAX_APC_FPU",	3,
    "MMAX_APC_FPA",	4,
    "MMAX_XPC",		5,
    (const char *) 0,	0,
};

static subtype_entry_t i386_cpu[] = {
    "386",		3,
    "486",		4,
    "486SX",		4 + 128,
    "586",		5,
    "586SX",		5 + 128,
    (const char *) 0,	0,
};

static subtype_entry_t mips_cpu[] = {
    "MIPS_R2300",	1,
    "MIPS_R2600",	2,
    "MIPS_R2800",	3,
    "MIPS_R2000a",	4,
    (const char *) 0,	0,
};

static subtype_entry_t mc680x0_cpu[] = {
    "MC68030",		1,
    "MC68040",		2,
    "MC68030_ONLY",	3,
    (const char *) 0,	0,
};

static subtype_entry_t hppa_cpu[] = {
    "HPPA_825",		1,
    "HPPA_835",		2,
    "HPPA_840",		3,
    "HPPA_850",		4,
    "HPPA_855",		5,
    (const char *) 0,	0,
};

static subtype_entry_t arm_cpu[] = {
    "ARM_A500_ARCH",	1,
    "ARM_A500",		2,
    "ARM_A440",		3,
    "ARM_M4",		4,
    "ARM_A680",		5,
    (const char *) 0,	0,
};

static subtype_entry_t mc88000_cpu[] = {
    "MMAX_JPC",		1,
    "MC88100",		1,
    "MC88110",		2,
    (const char *) 0,	0,
};

static subtype_entry_t mc98000_cpu[] = {
    "MC98601",		1,
    (const char *) 0,	0,
};

static subtype_entry_t i860_cpu[] = {
    "I860_860",		1,
    (const char *) 0,	0,
};

static subtype_entry_t i860_little_cpu[] = {
    "I860_LITTLE",	1,
    (const char *) 0,	0,
};


static subtype_entry_t rs6000_cpu[] = {
    "RS6000",		1,
    (const char *) 0,	0,
};

static subtype_entry_t sparc_cpu[] = {
    "SUN4_260",		1,
    "SUN4_110",		2,
    (const char *) 0,	0,
};

static subtype_entry_t unknown_cpu[] = {
    (const char *) 0,	0,
};

#define SUBTABLE(name, table) \
	#name, table ## _cpu

typedef struct {
    const char *name;
    const subtype_entry_t *subtable;
} cpu_entry_t;

static cpu_entry_t cpu_info[] = {
    SUBTABLE(unknown, unknown),		/* ((cpu_type_t) 0) */
    SUBTABLE(VAX, vax),			/* ((cpu_type_t) 1) */
    SUBTABLE(RT, romp),			/* ((cpu_type_t) 2) */
    SUBTABLE(unknown, unknown),		/* ((cpu_type_t) 3) */
    SUBTABLE(NS32032, ns32032),		/* ((cpu_type_t) 4) */
    SUBTABLE(NS32332, ns32032),      	/* ((cpu_type_t) 5) */
    SUBTABLE(M68K, mc680x0),		/* ((cpu_type_t) 6) */
    SUBTABLE(PC, i386),			/* ((cpu_type_t) 7) */
    SUBTABLE(MIPS, mips),		/* ((cpu_type_t) 8) */
    SUBTABLE(NS32532, ns32032),      	/* ((cpu_type_t) 9) */
    SUBTABLE(unknown, unknown),		/* ((cpu_type_t) 10) */
    SUBTABLE(SNAKE, hppa),         	/* ((cpu_type_t) 11) */
    SUBTABLE(ARM, arm),			/* ((cpu_type_t) 12) */
    SUBTABLE(M88K, mc88000),		/* ((cpu_type_t) 13) */
    SUBTABLE(SPARC, sparc),		/* ((cpu_type_t) 14) */
    SUBTABLE(I860, i860),		/* ((cpu_type_t) 15) big-endian */
    SUBTABLE(I860_LITTLE, i860_little),	/* ((cpu_type_t) 16) little-endian */
    SUBTABLE(RS6000, rs6000),		/* ((cpu_type_t) 17) */
    SUBTABLE(POWERPC, mc98000),		/* ((cpu_type_t) 18) */
};

#if !defined(NELEM)
#define NELEM(x)	(sizeof((x)) / sizeof((x)[0]))
#endif /* !defined(NELEM) */

void getinfo(char const **cpupp, char const **subpp, char const **ospp)
{
    struct host_basic_info basic_info;
    unsigned int count;
    const subtype_entry_t *subp;
    static kernel_version_t kvers;

    count = HOST_BASIC_INFO_COUNT;
    if (host_info(host_self(), HOST_BASIC_INFO, (host_info_t) &basic_info,
						     &count) == KERN_SUCCESS &&
				       basic_info.cpu_type < NELEM(cpu_info)) {
	*cpupp = cpu_info[basic_info.cpu_type].name;
	for (subp = cpu_info[basic_info.cpu_type].subtable; subp->name &&
			       subp->subtype != basic_info.cpu_subtype; ++subp)
	    ;
	if (subp->name)
	    *subpp = subp->name;
	else
	    *subpp = "unknown";
    } else {
	*cpupp = "unknown";
	*subpp = "unknown";
    }
    if (host_kernel_version(host_self(), kvers) == KERN_SUCCESS)
	*ospp  = kvers;
    else
	*ospp = "unknown";
}

/* End of stuff that should probably be in its own file */

#define T(a, b, c, d)	((a << 24) |		\
			 (b << 16) |		\
			 (c <<  8) |		\
			 (d <<  0))

const char *getcategory(const char *severity, const char *class,
							  const char *synopsis)
{
    const char *retval;

    switch (ROMlib_creator) {
    case T('A', 'T', 'r', 'n'):
	retval = "atrain";
	break;
    case T('C', 'T', 'I', 'M'):
	retval = "carmen";
	break;
    case T('C', 'Y', 'Q', 'S'):
	retval = "crystal-quest";
	break;
    case T('X', 'C', 'E', 'L'):
	retval = "excel";
	break;
    case T('P', 's', 'y', 'g'):
	retval = "lemmings";
	break;
    case T('M', 'I', 'T', '1'):
	retval = "macintax";
	break;
    case T('S', 'S', 'L', 'A'):
	retval = "macmoney";
	break;
    case T('M', 'Y', 'M', 'C'):
	retval = "mym";
	break;
    case T('P', 'O', '\xC4', 'P'):
	retval = "pop";
	break;
    case T('I', 'N', 'T', 'U'):
	retval = "quicken";
	break;
    case T('R', 'I', 'S', 'K'):
	retval = "risk";
	break;
    case T('S', 'A', 'N', 'T'):
	retval = "simant";
	break;
    case T('S', 'I', 'T', '!'):
	retval = "stuffit-lite";
	break;
    case T('M', 'S', 'W', 'D'):
	retval = "word";
	break;
    }
    if (!retval) {
	if (strcmp(severity, "System Crash/Hang") == 0)
	    return "yipes";
	else if (debugnumber < 10000)
	    return "red";
	else if (strcmp(class, "Documentation") == 0)
	    return "docs";
	else if (recmp(".*[Pp][Rr][Ii][Nn][Tt].*", (char *) synopsis) == 0)
	    return "printing";
	else if (recmp(".*[Xx][Ff][Ee][Rr].*", (char *) synopsis) == 0)
	    return "hfs_xfer";
	else
	    retval = "executor";
    }
    return retval;
}

#define OPENCMD	"/usr/ucb/Mail -s BugReport %s"
#define FIELDCHARS "**********"
		 /* 0x12345678 */

- mailARDI: sender
{
    FILE *mailfp;
    char *opencmd, *notetext;
    const char *cputype, *cpusubtype, *osversion;
    char starspchars[sizeof(FIELDCHARS)], starpcchars[sizeof(FIELDCHARS)];
    extern long debugtable[1L << 12];
    int i;
    id textid;
    const char *severity, *class, *synopsis, *category;
    int closeretval;

    opencmd = alloca(strlen(OPENCMD) +
				    strlen([addressForm stringValueAt:0]));
    sprintf(opencmd, OPENCMD, [addressForm stringValueAt:0]);
    mailfp = popen(opencmd, "w");
    if (!mailfp)
	NXRunAlertPanel("No Mail?", "Could not open mail connection.",
		     (const char *) 0, (const char *) 0, (const char *) 0);
    else {
	textid = [noteBox docView];
	notetext = alloca([textid textLength]);
	[textid getSubstring:notetext start:0 length:[textid textLength]+1];
	getinfo(&cputype, &cpusubtype, &osversion);
	severity = [[[severityPopUp itemList] selectedCell] title];
	if (!severity)
	    severity = [[[severityPopUp itemList] cellAt:0 :0] title];
	class = [[[classPopUp itemList] selectedCell] title];
	if (!class)
	    class = [[[classPopUp itemList] cellAt:0 :0] title];
	synopsis = [synopsisForm stringValueAt:0];
	category = getcategory(severity, class, synopsis);
	fprintf(mailfp,
			">Release: Executor %s %s\n"
			">Software: %s\n"
			">Version: %s\n"
			">Synopsis: %s\n"
			">Severity: %s\n"
			">Class: %s\n"
			">Category: %s\n"
			">Environment:\n"
			"    CPU_TYPE: %s\n"
			"    CPU_SUB_TYPE: %s\n"
			"    OS: %s\n"
			">Description:\n"
			"    %s\n",
			ROMlib_executor_version,
			ROMlib_executor_build_time,
			[softwareForm stringValueAt:0],
			[softwareForm stringValueAt:1],
			synopsis,
			severity,
			class,
			category,
			cputype,
			cpusubtype,
			osversion,
			notetext);

	if (appstate == appdied) {
	    if (starpcfield == hexfield)
		sprintf(starpcchars, "0x%08lx", starpc);
	    else
		sprintf(starpcchars, FIELDCHARS);
	    if (starspfield == hexfield)
		sprintf(starspchars, "0x%08lx", starsp);
	    else
		sprintf(starspchars, FIELDCHARS);
	    fprintf(mailfp, ">Death Certificate:\n"
			    "    SP 0x%08lx\n"
			    "    *SP %s\n"
			    "     PC 0x%08lx\n"
			    "    *PC %s\n"
			    "     D0 0x%08lx\n"
			    "    PSL 0x%08lx\n"
			    "    SIG %d\n"
			    "    DEB %d\n",
			    sp,
			    starspchars,
			    pc,
			    starpcchars,
			    d0,
			    psl,
			    sig,
			    debugnumber);
	    for (i = 0; i < NELEM(debugtable); i++)
		if (debugtable[i])
		    fprintf(mailfp, "    0xA%03X: %d\n", i, debugtable[i]);
	}
	if ((closeretval = pclose(mailfp)) != 0) {
	    NXRunAlertPanel("Problem Report NOT Sent",
			"%s exited with an error return of %d.",
			  (const char *) 0, (const char *) 0, (const char *) 0,
							 opencmd, closeretval);
	} else
	    NXRunAlertPanel("Sent", "Your Problem Report has been sent.",
		     (const char *) 0, (const char *) 0, (const char *) 0);
	[textid selectAll:sender];
	[textid replaceSel:""];
	[synopsisForm setStringValue:"" at:0];
    }
    [mailPanel orderOut:sender];
    return sender;
}

- (void) installinfo:(const char *)name :(const char *) version
{
    [softwareForm setStringValue:name    at:0];
    [softwareForm setStringValue:version at:1];
}

void ROMlib_installinfo(const char *name, const char *version)
{
    [global_game installinfo:name :version];
}

@end
