/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/local_charset.h"

#include <ctype.h>

/*
 * Currently we support Executor under DOS, Windows and X, so
 * this code is set up with DOS and Windows with one set of values
 * and everyone else with the X values.  As we support more platforms
 * this can be more intelligently adjusted.
 */

#if defined(CYGWIN32) || defined(MSDOS) || defined(MACOSX_) || defined(WIN32)

/* No representation for:
   138, S caron
   154, s caron
   166, broken bar
   173, soft hypnen
   178, superscript 2
   179, superscript 3
   183, bullet (small)
   185, superscript 1
   188, one-fourth
   189, one-half
   190, three-fourths
   208, Icelandic Eth
   221, Y acute
   222, Icelandic Thorn
   240, Icelandic eth
   253, y acute
*/

enum
{
    LOCAL_CHAR_Adieresis = 196,
    LOCAL_CHAR_Aring = 197,
    LOCAL_CHAR_Ccedilla = 199,
    LOCAL_CHAR_Eacute = 201,
    LOCAL_CHAR_Ntilde = 209,
    LOCAL_CHAR_Odieresis = 214,
    LOCAL_CHAR_Udieresis = 220,
    LOCAL_CHAR_aacute = 225,
    LOCAL_CHAR_agrave = 224,
    LOCAL_CHAR_acircumflex = 226,
    LOCAL_CHAR_adieresis = 228,
    LOCAL_CHAR_atilde = 227,
    LOCAL_CHAR_aring = 229,
    LOCAL_CHAR_ccedilla = 231,
    LOCAL_CHAR_eacute = 233,
    LOCAL_CHAR_egrave = 232,
    LOCAL_CHAR_ecircumflex = 234,
    LOCAL_CHAR_edieresis = 235,
    LOCAL_CHAR_iacute = 237,
    LOCAL_CHAR_igrave = 236,
    LOCAL_CHAR_icircumflex = 238,
    LOCAL_CHAR_idieresis = 239,
    LOCAL_CHAR_ntilde = 241,
    LOCAL_CHAR_oacute = 243,
    LOCAL_CHAR_ograve = 242,
    LOCAL_CHAR_ocircumflex = 244,
    LOCAL_CHAR_odieresis = 246,
    LOCAL_CHAR_otilde = 245,
    LOCAL_CHAR_uacute = 250,
    LOCAL_CHAR_ugrave = 249,
    LOCAL_CHAR_ucircumflex = 251,
    LOCAL_CHAR_udieresis = 252,
    LOCAL_CHAR_dagger = 134,
    LOCAL_CHAR_degree = 176, /* also ring */
    LOCAL_CHAR_cent = 162,
    LOCAL_CHAR_sterling = 163,
    LOCAL_CHAR_section = 167,
    LOCAL_CHAR_bullet = 149,
    LOCAL_CHAR_paragraph = 182,
    LOCAL_CHAR_germandbls = 223,
    LOCAL_CHAR_registered = 174,
    LOCAL_CHAR_copyright = 169,
    LOCAL_CHAR_trademark = 0x00, /* <-- */
    LOCAL_CHAR_acute = 180,
    LOCAL_CHAR_dieresis = 168,
    LOCAL_CHAR_notequal = 0x00, /* <-- */
    LOCAL_CHAR_AE = 198,
    LOCAL_CHAR_Oslash = 216,
    LOCAL_CHAR_infinity = 0x00, /* <-- */
    LOCAL_CHAR_plusminus = 177,
    LOCAL_CHAR_lessequal = 0x00, /* <-- */
    LOCAL_CHAR_greaterequal = 0x00, /* <-- */
    LOCAL_CHAR_yen = 165,
    LOCAL_CHAR_mu = 181,
    LOCAL_CHAR_partialdiff = 0x00, /* <-- */
    LOCAL_CHAR_summation = 0x00, /* <-- */
    LOCAL_CHAR_product = 215, /* multiply symbol */
    LOCAL_CHAR_pi = 0x00, /* <-- */
    LOCAL_CHAR_integral = 0x00, /* <-- */
    LOCAL_CHAR_ordfeminine = 170,
    LOCAL_CHAR_ordmasculine = 186,
    LOCAL_CHAR_Omega = 0x00, /* <-- */
    LOCAL_CHAR_ae = 230,
    LOCAL_CHAR_oslash = 248,
    LOCAL_CHAR_questiondown = 191,
    LOCAL_CHAR_exclamdown = 161,
    LOCAL_CHAR_logicalnot = 172,
    LOCAL_CHAR_daggerdbl = 135,
    LOCAL_CHAR_florin = 131,
    LOCAL_CHAR_approxequal = 0x00, /* <-- */
    LOCAL_CHAR_Delta = 0x00, /* <-- */
    LOCAL_CHAR_guillemotleft = 171,
    LOCAL_CHAR_guillemotright = 187,
    LOCAL_CHAR_ellipsis = 133,
    LOCAL_CHAR_space = 160,
    LOCAL_CHAR_Agrave = 192,
    LOCAL_CHAR_Atilde = 195,
    LOCAL_CHAR_Otilde = 213,
    LOCAL_CHAR_OE = 140,
    LOCAL_CHAR_oe = 156,
    LOCAL_CHAR_endash = 150,
    LOCAL_CHAR_emdash = 151,
    LOCAL_CHAR_quotedblleft = 147,
    LOCAL_CHAR_quotedblright = 148,
    LOCAL_CHAR_quoteleft = 145,
    LOCAL_CHAR_quoteright = 146,
    LOCAL_CHAR_divide = 247,
    LOCAL_CHAR_lozenge = 0x00, /* <-- */
    LOCAL_CHAR_ydieresis = 255,
    LOCAL_CHAR_Ydieresis = 159,
    LOCAL_CHAR_fraction = 0x00, /* <-- */
    LOCAL_CHAR_currency = 164, /* intl. monetary symbol */
    LOCAL_CHAR_guilsinglleft = 139,
    LOCAL_CHAR_guilsinglright = 155,
    LOCAL_CHAR_fi = 0x00, /* <-- */
    LOCAL_CHAR_fl = 0x00, /* <-- */
    LOCAL_CHAR_periodcentered = 0x00, /* <-- */
    LOCAL_CHAR_quotesinglbase = 130,
    LOCAL_CHAR_quotedblbase = 132,
    LOCAL_CHAR_perthousand = 137,
    LOCAL_CHAR_Acircumflex = 194,
    LOCAL_CHAR_Ecircumflex = 202,
    LOCAL_CHAR_Aacute = 193,
    LOCAL_CHAR_Edieresis = 203,
    LOCAL_CHAR_Egrave = 200,
    LOCAL_CHAR_Iacute = 205,
    LOCAL_CHAR_Icircumflex = 206,
    LOCAL_CHAR_Idieresis = 207,
    LOCAL_CHAR_Igrave = 204,
    LOCAL_CHAR_Oacute = 211,
    LOCAL_CHAR_Ocircumflex = 212,
    LOCAL_CHAR_apple = 0x00, /* <-- */
    LOCAL_CHAR_Ograve = 210,
    LOCAL_CHAR_Uacute = 218,
    LOCAL_CHAR_Ucircumflex = 219,
    LOCAL_CHAR_Ugrave = 217,
    LOCAL_CHAR_dotlessi = 0x00, /* <-- */
    LOCAL_CHAR_circumflex = 136,
    LOCAL_CHAR_tilde = 152,
    LOCAL_CHAR_macron = 175,
    LOCAL_CHAR_breve = 0x00, /* <-- */
    LOCAL_CHAR_dotaccent = 0x00, /* <-- */
    LOCAL_CHAR_ring = 176,
    LOCAL_CHAR_cedilla = 184,
    LOCAL_CHAR_hungarumlaut = 0x00, /* <-- */
    LOCAL_CHAR_oganek = 0x00, /* <-- */
    LOCAL_CHAR_caron = 0x00, /* <-- */
};

#else

#include <X11/keysym.h>

/* No representation for:
#define XK_brokenbar           0x0a6
#define XK_twosuperior         0x0b2
#define XK_threesuperior       0x0b3
#define XK_onesuperior         0x0b9
#define XK_onequarter          0x0bc
#define XK_onehalf             0x0bd
#define XK_threequarters       0x0be
#define XK_ETH                 0x0d0
#define XK_Yacute              0x0dd
#define XK_THORN               0x0de
#define XK_eth                 0x0f0
#define XK_yacute              0x0fd
#define XK_thorn               0x0fe
*/

enum
{
    LOCAL_CHAR_Adieresis = XK_Adiaeresis,
    LOCAL_CHAR_Aring = XK_Aring,
    LOCAL_CHAR_Ccedilla = XK_Ccedilla,
    LOCAL_CHAR_Eacute = XK_Eacute,
    LOCAL_CHAR_Ntilde = XK_Ntilde,
    LOCAL_CHAR_Odieresis = XK_Odiaeresis,
    LOCAL_CHAR_Udieresis = XK_Udiaeresis,
    LOCAL_CHAR_aacute = XK_aacute,
    LOCAL_CHAR_agrave = XK_agrave,
    LOCAL_CHAR_acircumflex = XK_acircumflex,
    LOCAL_CHAR_adieresis = XK_adiaeresis,
    LOCAL_CHAR_atilde = XK_atilde,
    LOCAL_CHAR_aring = XK_aring,
    LOCAL_CHAR_ccedilla = XK_ccedilla,
    LOCAL_CHAR_eacute = XK_eacute,
    LOCAL_CHAR_egrave = XK_egrave,
    LOCAL_CHAR_ecircumflex = XK_ecircumflex,
    LOCAL_CHAR_edieresis = XK_ediaeresis,
    LOCAL_CHAR_iacute = XK_iacute,
    LOCAL_CHAR_igrave = XK_igrave,
    LOCAL_CHAR_icircumflex = XK_icircumflex,
    LOCAL_CHAR_idieresis = XK_idiaeresis,
    LOCAL_CHAR_ntilde = XK_ntilde,
    LOCAL_CHAR_oacute = XK_oacute,
    LOCAL_CHAR_ograve = XK_ograve,
    LOCAL_CHAR_ocircumflex = XK_ocircumflex,
    LOCAL_CHAR_odieresis = XK_odiaeresis,
    LOCAL_CHAR_otilde = XK_otilde,
    LOCAL_CHAR_uacute = XK_uacute,
    LOCAL_CHAR_ugrave = XK_ugrave,
    LOCAL_CHAR_ucircumflex = XK_ucircumflex,
    LOCAL_CHAR_udieresis = XK_udiaeresis,
    LOCAL_CHAR_dagger = 0x00, /* <-- */
    LOCAL_CHAR_degree = XK_degree,
    LOCAL_CHAR_cent = XK_cent,
    LOCAL_CHAR_sterling = XK_sterling,
    LOCAL_CHAR_section = XK_section,
    LOCAL_CHAR_bullet = 0x00, /* <-- */
    LOCAL_CHAR_paragraph = XK_paragraph,
    LOCAL_CHAR_germandbls = XK_ssharp,
    LOCAL_CHAR_registered = XK_registered,
    LOCAL_CHAR_copyright = XK_copyright,
    LOCAL_CHAR_trademark = 0x00, /* <-- */
    LOCAL_CHAR_acute = XK_acute,
    LOCAL_CHAR_dieresis = XK_diaeresis,
    LOCAL_CHAR_notequal = 0x00, /* <-- */
    LOCAL_CHAR_AE = XK_AE,
    LOCAL_CHAR_Oslash = XK_Ooblique,
    LOCAL_CHAR_infinity = 0x00, /* <-- */
    LOCAL_CHAR_plusminus = XK_plusminus,
    LOCAL_CHAR_lessequal = 0x00, /* <-- */
    LOCAL_CHAR_greaterequal = 0x00, /* <-- */
    LOCAL_CHAR_yen = XK_yen,
    LOCAL_CHAR_mu = XK_mu,
    LOCAL_CHAR_partialdiff = 0x00, /* <-- */
    LOCAL_CHAR_summation = 0x00, /* <-- */
    LOCAL_CHAR_product = XK_multiply,
    LOCAL_CHAR_pi = 0x00, /* <-- */
    LOCAL_CHAR_integral = 0x00, /* <-- */
    LOCAL_CHAR_ordfeminine = XK_ordfeminine,
    LOCAL_CHAR_ordmasculine = XK_masculine,
    LOCAL_CHAR_Omega = 0x00, /* <-- */
    LOCAL_CHAR_ae = XK_ae,
    LOCAL_CHAR_oslash = XK_oslash,
    LOCAL_CHAR_questiondown = XK_questiondown,
    LOCAL_CHAR_exclamdown = XK_exclamdown,
    LOCAL_CHAR_logicalnot = XK_notsign,
    LOCAL_CHAR_daggerdbl = 0x00, /* <-- */
    LOCAL_CHAR_florin = 0x00, /* <-- */
    LOCAL_CHAR_approxequal = 0x00, /* <-- */
    LOCAL_CHAR_Delta = 0x00, /* <-- */
    LOCAL_CHAR_guillemotleft = XK_guillemotleft,
    LOCAL_CHAR_guillemotright = XK_guillemotright,
    LOCAL_CHAR_ellipsis = 0x00, /* <-- */
    LOCAL_CHAR_space = XK_nobreakspace,
    LOCAL_CHAR_Agrave = XK_Agrave,
    LOCAL_CHAR_Atilde = XK_Atilde,
    LOCAL_CHAR_Otilde = XK_Otilde,
    LOCAL_CHAR_OE = 0x00, /* <-- */
    LOCAL_CHAR_oe = 0x00, /* <-- */
    LOCAL_CHAR_endash = XK_hyphen,
    LOCAL_CHAR_emdash = 0x00, /* <-- */
    LOCAL_CHAR_quotedblleft = 0x00, /* <-- */
    LOCAL_CHAR_quotedblright = 0x00, /* <-- */
    LOCAL_CHAR_quoteleft = 0x00, /* <-- */
    LOCAL_CHAR_quoteright = 0x00, /* <-- */
    LOCAL_CHAR_divide = XK_division,
    LOCAL_CHAR_lozenge = 0x00, /* <-- */
    LOCAL_CHAR_ydieresis = XK_ydiaeresis,
    LOCAL_CHAR_Ydieresis = XK_ydiaeresis, /* no XK_Ydiaeresis */
    LOCAL_CHAR_fraction = 0x00, /* <-- */
    LOCAL_CHAR_currency = XK_currency,
    LOCAL_CHAR_guilsinglleft = 0x00, /* <-- */
    LOCAL_CHAR_guilsinglright = 0x00, /* <-- */
    LOCAL_CHAR_fi = 0x00, /* <-- */
    LOCAL_CHAR_fl = 0x00, /* <-- */
    LOCAL_CHAR_periodcentered = XK_periodcentered,
    LOCAL_CHAR_quotesinglbase = 0x00, /* <-- */
    LOCAL_CHAR_quotedblbase = 0x00, /* <-- */
    LOCAL_CHAR_perthousand = 0x00, /* <-- */
    LOCAL_CHAR_Acircumflex = XK_Acircumflex,
    LOCAL_CHAR_Ecircumflex = XK_Ecircumflex,
    LOCAL_CHAR_Aacute = XK_Aacute,
    LOCAL_CHAR_Edieresis = XK_Ediaeresis,
    LOCAL_CHAR_Egrave = XK_Egrave,
    LOCAL_CHAR_Iacute = XK_Iacute,
    LOCAL_CHAR_Icircumflex = XK_Icircumflex,
    LOCAL_CHAR_Idieresis = XK_Idiaeresis,
    LOCAL_CHAR_Igrave = XK_Igrave,
    LOCAL_CHAR_Oacute = XK_Oacute,
    LOCAL_CHAR_Ocircumflex = XK_Ocircumflex,
    LOCAL_CHAR_apple = 0x00, /* <-- */
    LOCAL_CHAR_Ograve = XK_Ograve,
    LOCAL_CHAR_Uacute = XK_Uacute,
    LOCAL_CHAR_Ucircumflex = XK_Ucircumflex,
    LOCAL_CHAR_Ugrave = XK_Ugrave,
    LOCAL_CHAR_dotlessi = 0x00, /* <-- */
    LOCAL_CHAR_circumflex = 0x00, /* <-- */
    LOCAL_CHAR_tilde = 0x00, /* <-- */
    LOCAL_CHAR_macron = XK_macron,
    LOCAL_CHAR_breve = 0x00, /* <-- */
    LOCAL_CHAR_dotaccent = 0x00, /* <-- */
    LOCAL_CHAR_ring = 0x00, /* <-- */
    LOCAL_CHAR_cedilla = XK_cedilla,
    LOCAL_CHAR_hungarumlaut = 0x00, /* <-- */
    LOCAL_CHAR_oganek = 0x00, /* <-- */
    LOCAL_CHAR_caron = 0x00, /* <-- */
};

#endif

PRIVATE unsigned char
    local_high_bit_characters[128]
    = {
        LOCAL_CHAR_Adieresis,
        LOCAL_CHAR_Aring,
        LOCAL_CHAR_Ccedilla,
        LOCAL_CHAR_Eacute,
        LOCAL_CHAR_Ntilde,
        LOCAL_CHAR_Odieresis,
        LOCAL_CHAR_Udieresis,
        LOCAL_CHAR_aacute,
        LOCAL_CHAR_agrave,
        LOCAL_CHAR_acircumflex,
        LOCAL_CHAR_adieresis,
        LOCAL_CHAR_atilde,
        LOCAL_CHAR_aring,
        LOCAL_CHAR_ccedilla,
        LOCAL_CHAR_eacute,
        LOCAL_CHAR_egrave,
        LOCAL_CHAR_ecircumflex,
        LOCAL_CHAR_edieresis,
        LOCAL_CHAR_iacute,
        LOCAL_CHAR_igrave,
        LOCAL_CHAR_icircumflex,
        LOCAL_CHAR_idieresis,
        LOCAL_CHAR_ntilde,
        LOCAL_CHAR_oacute,
        LOCAL_CHAR_ograve,
        LOCAL_CHAR_ocircumflex,
        LOCAL_CHAR_odieresis,
        LOCAL_CHAR_otilde,
        LOCAL_CHAR_uacute,
        LOCAL_CHAR_ugrave,
        LOCAL_CHAR_ucircumflex,
        LOCAL_CHAR_udieresis,
        LOCAL_CHAR_dagger,
        LOCAL_CHAR_degree,
        LOCAL_CHAR_cent,
        LOCAL_CHAR_sterling,
        LOCAL_CHAR_section,
        LOCAL_CHAR_bullet,
        LOCAL_CHAR_paragraph,
        LOCAL_CHAR_germandbls,
        LOCAL_CHAR_registered,
        LOCAL_CHAR_copyright,
        LOCAL_CHAR_trademark,
        LOCAL_CHAR_acute,
        LOCAL_CHAR_dieresis,
        LOCAL_CHAR_notequal,
        LOCAL_CHAR_AE,
        LOCAL_CHAR_Oslash,
        LOCAL_CHAR_infinity,
        LOCAL_CHAR_plusminus,
        LOCAL_CHAR_lessequal,
        LOCAL_CHAR_greaterequal,
        LOCAL_CHAR_yen,
        LOCAL_CHAR_mu,
        LOCAL_CHAR_partialdiff,
        LOCAL_CHAR_summation,
        LOCAL_CHAR_product,
        LOCAL_CHAR_pi,
        LOCAL_CHAR_integral,
        LOCAL_CHAR_ordfeminine,
        LOCAL_CHAR_ordmasculine,
        LOCAL_CHAR_Omega,
        LOCAL_CHAR_ae,
        LOCAL_CHAR_oslash,
        LOCAL_CHAR_questiondown,
        LOCAL_CHAR_exclamdown,
        LOCAL_CHAR_logicalnot,
        LOCAL_CHAR_daggerdbl,
        LOCAL_CHAR_florin,
        LOCAL_CHAR_approxequal,
        LOCAL_CHAR_Delta,
        LOCAL_CHAR_guillemotleft,
        LOCAL_CHAR_guillemotright,
        LOCAL_CHAR_ellipsis,
        LOCAL_CHAR_space,
        LOCAL_CHAR_Agrave,
        LOCAL_CHAR_Atilde,
        LOCAL_CHAR_Otilde,
        LOCAL_CHAR_OE,
        LOCAL_CHAR_oe,
        LOCAL_CHAR_endash,
        LOCAL_CHAR_emdash,
        LOCAL_CHAR_quotedblleft,
        LOCAL_CHAR_quotedblright,
        LOCAL_CHAR_quoteleft,
        LOCAL_CHAR_quoteright,
        LOCAL_CHAR_divide,
        LOCAL_CHAR_lozenge,
        LOCAL_CHAR_ydieresis,
        LOCAL_CHAR_Ydieresis,
        LOCAL_CHAR_fraction,
        LOCAL_CHAR_currency,
        LOCAL_CHAR_guilsinglleft,
        LOCAL_CHAR_guilsinglright,
        LOCAL_CHAR_fi,
        LOCAL_CHAR_fl,
        LOCAL_CHAR_daggerdbl,
        LOCAL_CHAR_periodcentered,
        LOCAL_CHAR_quotesinglbase,
        LOCAL_CHAR_quotedblbase,
        LOCAL_CHAR_perthousand,
        LOCAL_CHAR_Acircumflex,
        LOCAL_CHAR_Ecircumflex,
        LOCAL_CHAR_Aacute,
        LOCAL_CHAR_Edieresis,
        LOCAL_CHAR_Egrave,
        LOCAL_CHAR_Iacute,
        LOCAL_CHAR_Icircumflex,
        LOCAL_CHAR_Idieresis,
        LOCAL_CHAR_Igrave,
        LOCAL_CHAR_Oacute,
        LOCAL_CHAR_Ocircumflex,
        LOCAL_CHAR_apple,
        LOCAL_CHAR_Ograve,
        LOCAL_CHAR_Uacute,
        LOCAL_CHAR_Ucircumflex,
        LOCAL_CHAR_Ugrave,
        LOCAL_CHAR_dotlessi,
        LOCAL_CHAR_circumflex,
        LOCAL_CHAR_tilde,
        LOCAL_CHAR_macron,
        LOCAL_CHAR_breve,
        LOCAL_CHAR_dotaccent,
        LOCAL_CHAR_ring,
        LOCAL_CHAR_cedilla,
        LOCAL_CHAR_hungarumlaut,
        LOCAL_CHAR_oganek,
        LOCAL_CHAR_caron,
      };

static unsigned char
hexval(unsigned char c)
{
    unsigned char retval;

    if((c >= '0') && (c <= '9'))
        retval = c - '0';
    else if((c >= 'A') && (c <= 'F'))
        retval = 10 + (c - 'A');
    else if((c >= 'a') && (c <= 'f'))
        retval = 10 + (c - 'a');
    else
        retval = 0;

    return retval;
}

PUBLIC void
Executor::ROMlib_localize_string(char *p)
{
    unsigned char c;
    unsigned char *ip, *op;

    for(ip = (unsigned char *)p, op = (unsigned char *)p; (c = *ip++);)
    {
        if(!(c & 0x80))
        {
            if(c == '%' && isxdigit(ip[0]) && isxdigit(ip[1]))
            {
                *op++ = (hexval(ip[0]) << 4) | hexval(ip[1]);
                ip += 2;
            }
            else
                *op++ = c;
        }
        else
        {
            unsigned char replacement;

            replacement = local_high_bit_characters[c & 0x7f];
            if(replacement)
                *op++ = replacement;
        }
    }
    *op++ = 0;
}
