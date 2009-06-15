#if 0
/* Copyright 1996, 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_licensetext[] =
		"$Id: licensetext.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#include "rsys/licensetext.h"

license_text_page_t ROMlib_license[] =
{
  {
#if defined (RELEASE_DEMO)

#if !defined (WRAPPER_NAME) || WRAPPER_NAME != JWM
    "Executor Demo Information",

#if defined(LINUX)
    "Through January 31st 1999, Executor/Linux is available for $35 to "
    "full-time students and for $75 to everyone else.  Please read our web "
    "pages (<http://www.ardi.com/>) or the file "
    "/usr/doc/executor-aux-*/License_Fees for more information.\r\r"
#endif

#if defined(MSDOS) || defined(CYGWIN32)
    "Through January 31st 1999, Executor/DOS and Executor/Win32 are available "
    "for $65 to "
    "full-time students and for $150 to everyone else.  Please read our web "
    "pages (<http://www.ardi.com/>) or the file "
    "fees.txt for more information.\r\r"
#endif

    "Please read this entire message.  You will need to use the scroll-bar "
    "to see it all.\r\r"
  
    "This is a demo version of Executor.  This demo is available solely "
    "so you can evaluate whether or not Executor is worth purchasing.  "
    "This demo should expire at the end of the 30th day after you have "
    "installed it.  "
    "In this demo version, all pages printed will have "
    "\"Demo\" written on them.  This demo version of Executor will read "
    "Macintosh formatted floppies and hard drives but will not allow you to "
    "write to or format them.  The remaining difference between this demo "
    "version and the commercial version of Executor is that Command-Key "
    "equivalents will not work in this demo version.\r\r"

#else
    "JWM Executor Demo Information",

    "This demo version of Executor 2.0 is licensed only to run Japanese "
    "WordMage (JWM) and a few specific auxiliary programs:  ARDI's Browser, "
    "Documents created by DocMaker, JConv, User Info, "
    "Brian's Sound Tool, Giffer, Stuffit Expander, Compact Pro, Compact Pro "
    "User's Guide, Compact Pro Self-Extracting Archives, StuffIt Lite, "
    "StuffIt Lite Self-Extracting Archives, Desktop Textures Vol. 1, and "
    "TexEdit.  As such, this version of Executor can't "
    "be redistributed, or modified.  Using this version of Executor to run "
    "Macintosh programs other than the ones explicitly mentioned above is a "
    "violation of this license agreement.  "
    "This demo will only run for thirty minutes before it will force you to "
    "exit.  In addition, all pages printed from this demo version will have "
    "\"Demo\" written on them.  This demo version of Executor will read "
    "Macintosh formatted floppies and hard drives but will not allow you to "
    "write to or format them.\r\r"
#endif

#else
    "DO NOT REDISTRIBUTE",

#if defined(WRAPPER_NAME) && WRAPPER_NAME == JWM
    "This version of Executor 2.0 is licensed only to run Japanese "
    "WordMage (JWM) and a few specific auxiliary programs:  ARDI's Browser, "
    "Documents created by DocMaker, JConv, User Info, "
    "Brian's Sound Tool, Giffer, Stuffit Expander, Compact Pro, Compact Pro "
    "User's Guide, Compact Pro Self-Extracting Archives, StuffIt Lite, "
    "StuffIt Lite Self-Extracting Archives, Desktop Textures Vol. 1, and "
    "TexEdit.  As such, this version of Executor can't "
    "be redistributed, or modified.  Using this version of Executor to run "
    "Macintosh programs other than the ones explicitly mentioned above is a "
    "violation of this license agreement."
#elif defined (WRAPPER_NAME) && WRAPPER_NAME == TTS
    "This version of Executor 2.0 is licensed only to run software "
    "supplied by Teacher Technology Systems (TTS):  CalGen, "
    "Curriculum Manager, LogBook, RollBook and Test Generator.  "
    "As such, this version of Executor can't "
    "be redistributed, or modified.  Using this version of Executor to run "
    "programs other than the ones explicitly mentioned above is a "
    "violation of this license agreement."
#elif defined (WRAPPER_NAME) && WRAPPER_NAME == SUNPATH
    "This version of Executor 2.0 is licensed only to run sunPATH by Wide "
    "Screen Software.  "
    "As such, this version of Executor can't "
    "be redistributed, or modified.  Using this version of Executor to run "
    "programs other than sunPATH is a violation of this license agreement."
#else
    "This is the full featured version of Executor 2.0.  As such, it can't "
    "be redistributed.\r\r"
#endif

#endif

#if !defined (WRAPPER_NAME)
    "To get the most out of this software, please visit our web site "
    "frequently <http://www.ardi.com/>."
#endif
  },

  {
    "License Description",

    "Executor is written and copyrighted by Abacus Research and Development, "
    "Inc. (ARDI).  Executor is not sold, but is provided under this "
    "Software License Agreement.  "

#if defined (RELEASE_DEMO)
    "This license allows for 30 days the licensee to "
    "use this demo version of Executor "
    "without any licensing fee.  After the 30 day evaluation period "
    "licensee no longer has any rights to use this software."
#else
    "This license allows the licensee the use of this "

#if defined (WRAPPER_NAME) && WRAPPER_NAME == JWM
    "JWM-specific version of Executor in conjunction with JWM and the "
    "auxiliary applications."
#elif defined (WRAPPER_NAME) && WRAPPER_NAME == TTS
    "TTS-specific version of Executor."
#elif defined (WRAPPER_NAME) && WRAPPER_NAME == SUNPATH
    "sunPATH-specific version of Executor."
#else
    "fully enabled version (commercial) of Executor after "
    "payment of the appropriate licensing fees to ARDI."
#endif

#endif

  },

  {
    "Duplication and Redistribution",

#if defined (RELEASE_DEMO)
    "Complete distributions of this demo version of Executor may be copied "
    "and redistributed as long as all copies are unmodified and contain all "
    "of the original files in their entirety."
#else
    "This "
#if defined (WRAPPER_NAME) && WRAPPER_NAME == JWM
    "JWM-specific version"
#elif defined (WRAPPER_NAME) && WRAPPER_NAME == TTS
    "TTS-specific version"
#elif defined (WRAPPER_NAME) && WRAPPER_NAME == SUNPATH
    "sunPATH-specific version"
#else
    "fully enabled version (commercial)"
#endif
    " of Executor may be copied only "
    "for backup purposes."
#endif
    "  Licensee may not "
    "modify or create derivative works based on Executor or any part thereof."
  },

#if defined (RELEASE_COMMERCIAL) && !defined (WRAPPER_NAME)
  {
    "NO WARRANTY",

    "Executor is licensed without a warranty.  ARDI disclaims all warranties "
    "relating to this software, whether expressed or implied, including but "
    "not limited to any implied warranties of merchantability and fitness for "
    "a particular purpose.  All such warranties are expressly and "
    "specifically disclaimed."
  },
#endif

  {
    "ARDI Disclaims Liability",

    "In no event will ARDI be liable for indirect, special, incidental, "
    "or consequential damages resulting from the use of Executor, even "
    "if advised of the possibility of such damages."
  },

#if defined (RELEASE_COMMERCIAL) && !defined (WRAPPER_NAME)
  {
    "Exclusions, Limitations",

    "If Licensee uses the licensed software in a "
    "state which does not allow the exclusion or limitation of implied "
    "warranties or limitation of liabilities for incidental or consequential "
    "damages, then the limitations on warranties and exclusions of "
    "damages agreed upon by the parties to this license agreement shall "
    "be interpreted, to the extent permitted by the applicable state's "
    "law, to fulfill the agreement of the parties set forth herein."
  },
#endif

  {
    "General Terms",

    "This license is the entire agreement between Licensee and ARDI. "
    "If any provision of this License shall be held to be unenforceable, "
    "such holding shall not affect the enforceability of the remaining "
    "provisions.  This license shall be governed by and construed under "
    "the laws of the State of New Mexico.  Licensee agrees to bring any "
    "proceeding concerning this license before a court of the State of "
    "New Mexico or a Federal court located in the State of New Mexico."
  },

  {
    "U.S. Government Restrictions",

    "This Software is licensed only with RESTRICTED RIGHTS.\r\r"

    "For civilian agencies:  The use, reproduction, or disclosure of "
    "this software is subject to restrictions as set forth in subparagraphs "
    "(a) through (d) of the Commercial Computer Software--Restricted "
    "Rights clause at 52.227-19 of the Federal Acquisition Regulations. "
    "For Department of Defense agencies:  The use, reproduction, or "
    "disclosure of this software by the Government is subject to "
    "restrictions as set forth in subparagraph (c)(1)(ii) of the Rights "
    "in Technical Data and Computer Software clause at DFARS 252.227-7013."
  },

#if defined(MSDOS)
  {
    "PROGRAMMERS:  Free source available for cwsdpmi and wmemu387",

    "The files: \"cwsdpmi.exe\" (a DPMI provider), and \"wmemu387.dxe\" (an "
    "FPU emulator) were both written outside "
    "of ARDI by people who want you to be able to get the source to these "
    "two files.  More information is provided in the docs subdirectory.\r\r"

    "Executor includes software developed by the University of California "
    "and its contributors."
  },
#endif

      { NULL, NULL },  /* array terminator */
};

#endif
