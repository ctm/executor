/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_prRecords[] =
	"$Id: prRecords.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in PrintMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "PrintMgr.h"
#include "ResourceMgr.h"

#include "rsys/nextprint.h"
#include "rsys/pstuff.h"
#include "rsys/print.h"

using namespace Executor;
using namespace ByteSwap;

PRIVATE void
set_wDev (THPrint hPrint)
{
  HxX(hPrint, prStl.wDev)   = CWC(0x307);
}

PUBLIC void
Executor::ROMlib_set_default_resolution (THPrint hPrint, INTEGER vres, INTEGER hres)
{
  printer_init ();
  update_printing_globals ();

  HxX(hPrint, prInfo.iVRes) = BigEndianValue (vres);
  HxX(hPrint, prInfo.iHRes) = BigEndianValue (hres);
  HxX(hPrint, prInfo.rPage.top)    = CWC (0);
  HxX(hPrint, prInfo.rPage.left)   = CWC (0);
  HxX(hPrint, prInfo.rPage.bottom) = BigEndianValue ((ROMlib_paper_y - 72) * vres / 72);
  HxX(hPrint, prInfo.rPage.right)  = BigEndianValue ((ROMlib_paper_x - 72) * hres / 72);

  HxX(hPrint, rPaper.top)    = BigEndianValue ((INTEGER) (-0.5 * vres));
  HxX(hPrint, rPaper.bottom) = BigEndianValue ((INTEGER)
				   ((ROMlib_paper_y - 36) * vres / 72));
  HxX(hPrint, rPaper.left)   = BigEndianValue ((INTEGER) (-0.5 * hres));
  HxX(hPrint, rPaper.right)  = BigEndianValue ((INTEGER)
				   ((ROMlib_paper_x - 36) * hres / 72));

  ROMlib_resolution_x = hres;
  ROMlib_resolution_y = vres;
}

P1(PUBLIC pascal trap, void, PrintDefault, THPrint, hPrint)
{
    /* TODO:  fill this information in from the currently open
       printer resource file.  I don't know where it's kept so
       I've filled in the values by hand to be what I suspect the
       LaserWriter we're using wants */

    memset((char *) STARH(hPrint), 0, sizeof(TPrint));
    HxX(hPrint, iPrVersion) = BigEndianValue (ROMlib_PrDrvrVers);

    ROMlib_set_default_resolution (hPrint, 72, 72);
    
    HxX(hPrint, prInfo.iDev) = 0;

    set_wDev (hPrint);
    HxX(hPrint, prStl.iPageV) = CWC(1320); /* These were switched a while back */
    HxX(hPrint, prStl.iPageH) = CWC(1020); /* but I think it was a mistake */
    HxX(hPrint, prStl.bPort)  = 0;
    HxX(hPrint, prStl.feed)   = 2;

    HxX(hPrint, prInfoPT.iDev)         = 0;
    HxX(hPrint, prInfoPT.iVRes)        = CWC(72);
    HxX(hPrint, prInfoPT.iHRes)        = CWC(72);
    HxX(hPrint, prInfoPT.rPage)	     = HxX(hPrint, prInfo.rPage);

    HxX(hPrint, prXInfo.iRowBytes) = BigEndianValue((Hx(hPrint, prXInfo.iBandH) + 7) / 8);
    /* TODO: what about the rest of prXInfo? (is zero for now) */

    HxX(hPrint, prJob.iFstPage)  =  CWC(1);
    HxX(hPrint, prJob.iLstPage)  =  CWC(9999);
    HxX(hPrint, prJob.iCopies)   =  CWC(1);
    HxX(hPrint, prJob.bJDocLoop) =  2; /* used to be 1, but then File Maker
					  Pro 2.1 would call PrOpenDoc
					  and PrCloseDoc once for each page */
    HxX(hPrint, prJob.fFromUsr)  =  1;
}

P1(PUBLIC pascal trap, BOOLEAN, PrValidate, THPrint, hPrint) /* IMII-158 */
{
    /* TODO: figure out what are problem areas for us and adjust
	     accordingly */

  set_wDev (hPrint);

  if (!HxX (hPrint, prInfo.iVRes) || !HxX (hPrint, prInfo.iHRes))
    PrintDefault (hPrint);

  {
    int first, last;

    first = Hx (hPrint, prJob.iFstPage);
    last  = Hx (hPrint, prJob.iLstPage);

    if (first < 1 || first > last)
      {
	HxX(hPrint, prJob.iFstPage)  =  CWC(1);
	HxX(hPrint, prJob.iLstPage)  =  CWC(9999);
      }
  }
  {
    int copies;

    copies = Hx (hPrint, prJob.iCopies);
    
    if (copies < 1 || copies > 99)
      HxX(hPrint, prJob.iCopies)   =  CWC(1);
  }

  HxX(hPrint, prJob.bJDocLoop) =  2;
  return FALSE;
}

P1(PUBLIC pascal trap, BOOLEAN, PrStlDialog, THPrint, hPrint)
{
    BOOLEAN retval;

    retval = C_PrDlgMain(hPrint, (ProcPtr) P_PrStlInit);
    return retval;
}

P1(PUBLIC pascal trap, BOOLEAN, PrJobDialog, THPrint, hPrint)
{
  ROMlib_acknowledge_job_dialog (hPrint);
  return C_PrDlgMain(hPrint, (ProcPtr) P_PrJobInit);
}

P2(PUBLIC pascal trap, void, PrJobMerge, THPrint, hPrintSrc,
					    THPrint, hPrintDst)	/* TODO */
{
  warning_unimplemented (NULL_STRING);
}
