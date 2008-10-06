/* Copyright 1989 - 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_prPrinting[] =
	    "$Id: prPrinting.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "QuickDraw.h"
#include "PrintMgr.h"
#include "MemoryMgr.h"
#include "FontMgr.h"
#include "ResourceMgr.h"

#include "rsys/pstuff.h"
#include "rsys/PSstrings.h"
#include "rsys/notmac.h"
#include "rsys/mman.h"
#include "rsys/uniquefile.h"
#include "rsys/print.h"
#include "rsys/string.h"
#include "rsys/cquick.h"
#include "rsys/dirtyrect.h"
#include "rsys/vdriver.h"
#include "rsys/file.h"
#include "rsys/cleanup.h"

#if defined (MSDOS) || defined (CYGWIN32)
#include <stdarg.h>

PUBLIC boolean_t deferred_printing_p = FALSE /* TRUE */;

#endif

PUBLIC int pageno = 0;	/* This isn't really the way to do it */
PUBLIC int ROMlib_passpostscript = TRUE;
PUBLIC int ROMlib_fontsubstitution = FALSE;

PUBLIC char *ROMlib_document_paper_sizes;
PUBLIC char *ROMlib_paper_size;
PUBLIC const char *ROMlib_paper_size_name;
PUBLIC const char *ROMlib_paper_size_name_terminator;
PUBLIC int ROMlib_rotation;
PUBLIC int ROMlib_translate_x;
PUBLIC int ROMlib_translate_y;
PUBLIC int ROMlib_resolution_x = 72;
PUBLIC int ROMlib_resolution_y = 72;

PUBLIC int ROMlib_paper_x;
PUBLIC int ROMlib_paper_y;

/* This boolean is here to prevent Energy Scheming from causing trouble.
   ES calls PrPageClose twice at the end.  This fix is sub-optimal, but
   probably won't hurt anything. */

PRIVATE boolean_t page_is_open = FALSE;

#include "rsys/nextprint.h"

#if defined(NEXTSTEP)
printstate_t printstate;
#endif

LONGINT pagewanted = 0;
PRIVATE int lastpagewanted = 0;

P4(PUBLIC pascal trap, void, donotPrArc, GrafVerb, verb, Rect *, r,
						INTEGER, starta, INTEGER, arca)
{
}

P4(PUBLIC pascal trap, void, PrArc, GrafVerb, verb, Rect *, r,
						INTEGER, starta, INTEGER, arca)
{
    if (pageno >= pagewanted && pageno <= lastpagewanted)
	NeXTPrArc(verb, (comRect *) r, starta, arca, (comGrafPtr) thePort);
}

P5(PUBLIC pascal trap, void, donotPrBits, BitMap *, srcbmp, Rect *, srcrp,
				 Rect *, dstrp, INTEGER, mode, RgnHandle, mask)
{
}

P5(PUBLIC pascal trap, void, PrBits, BitMap *, srcbmp, Rect *, srcrp,
				 Rect *, dstrp, INTEGER, mode, RgnHandle, mask)
{
    if (pageno >= pagewanted && pageno <= lastpagewanted)
	NeXTPrBits((comBitMap *) srcbmp, (comRect *) srcrp, (comRect *) dstrp,
			      mode, (comRgnHandle) mask, (comGrafPtr) thePort);
}

P1(PUBLIC pascal trap, void, donotPrLine, Point, p)
{
}

P1(PUBLIC pascal trap, void, PrLine, Point, p)
{
    if (pageno >= pagewanted && pageno <= lastpagewanted)
      {
	comPoint cp;
	
	cp.h = p.h;
	cp.v = p.v;
	NeXTPrLine(cp, (comGrafPtr) thePort);
      }
}

P2(PUBLIC pascal trap, void, donotPrOval, GrafVerb, v, Rect *, rp)
{
}

P2(PUBLIC pascal trap, void, PrOval, GrafVerb, v, Rect *, rp)
{
    if (pageno >= pagewanted && pageno <= lastpagewanted)
	NeXTPrOval(v, (comRect *) rp, (comGrafPtr) thePort);
}

P4(PUBLIC pascal trap, void, textasPS, INTEGER, n, Ptr, textbufp,
							Point, num, Point, den)
{
#if 1
    if (pageno >= pagewanted && pageno <= lastpagewanted)
      {
	NeXTsendps(n, (comPtr) textbufp);
#if !defined (CYGWIN32)
	NeXTsendps(1, (comPtr) "\r");
#else
	NeXTsendps(2, (comPtr) "\r\n");
#endif
      }
#endif
}


P2(PUBLIC pascal trap, void, donotPrGetPic, Ptr, dp, INTEGER, bc)
{
    gui_abort();
}

P2(PUBLIC pascal trap, void, PrGetPic, Ptr, dp, INTEGER, bc)
{
    gui_abort();
    if (pageno >= pagewanted && pageno <= lastpagewanted)
	NeXTPrGetPic((comPtr) dp, bc, (comGrafPtr) thePort);
}

P2(PUBLIC pascal trap, void, donotPrPutPic, Ptr, sp, INTEGER, bc)
{
}

P2(PUBLIC pascal trap, void, PrPutPic, Ptr, sp, INTEGER, bc)
{
    if (pageno >= pagewanted && pageno <= lastpagewanted)
	NeXTPrPutPic((comPtr) sp, bc, (comGrafPtr) thePort);
}

P2(PUBLIC pascal trap, void, donotPrPoly, GrafVerb, verb, PolyHandle, ph)
{
}

P2(PUBLIC pascal trap, void, PrPoly, GrafVerb, verb, PolyHandle, ph)
{
    if (pageno >= pagewanted && pageno <= lastpagewanted)
	NeXTPrPoly(verb, (comPolyHandle) ph, (comGrafPtr) thePort);
}

P4(PUBLIC pascal trap, void, donotPrRRect, GrafVerb, verb, Rect *, r,
					       INTEGER, width, INTEGER, height)
{
}

P4(PUBLIC pascal trap, void, PrRRect, GrafVerb, verb, Rect *, r,
					       INTEGER, width, INTEGER, height)
{
    if (pageno >= pagewanted && pageno <= lastpagewanted)
	NeXTPrRRect(verb, (comRect *) r, width, height, (comGrafPtr) thePort);
}

P2(PUBLIC pascal trap, void, donotPrRect, GrafVerb, v, Rect *, rp)
{
}

P2(PUBLIC pascal trap, void, PrRect, GrafVerb, v, Rect *, rp)
{
    if (pageno >= pagewanted && pageno <= lastpagewanted)
	NeXTPrRect(v, (comRect *) rp, (comGrafPtr) thePort);
}

P2(PUBLIC pascal trap, void, donotPrRgn, GrafVerb, verb, RgnHandle, rgn)
{
}

P2(PUBLIC pascal trap, void, PrRgn, GrafVerb, verb, RgnHandle, rgn)
{
    if (pageno >= pagewanted && pageno <= lastpagewanted)
	NeXTPrRgn(verb, (comRgnHandle) rgn, (comGrafPtr) thePort);
}

P5(PUBLIC pascal trap, INTEGER, PrTxMeas, INTEGER, n, Ptr, p,
			      Point *, nump, Point *, denp, FontInfo *, finfop)
{
    StdTxMeas(n, p, nump, denp, finfop);
    return NeXTPrTxMeas(n, (comPtr) p, (comPoint *)nump, (comPoint *)denp,
				  (comFontInfo *)finfop, (comGrafPtr) thePort);
}

P4(PUBLIC pascal trap, void, donotPrText, INTEGER, n, Ptr, textbufp,
							Point, num, Point, den)
{
}

P4(PUBLIC pascal trap, void, PrText, INTEGER, n, Ptr, textbufp,
							Point, num, Point, den)
{
    if (pageno >= pagewanted && pageno <= lastpagewanted)
      {
	comPoint numc, denc;

	numc.h = num.h;
	numc.v = num.v;
	denc.h = den.h;
	denc.v = den.v;
	NeXTPrText(n, (comPtr) textbufp, numc, denc, (comGrafPtr) thePort);
      }
}

PRIVATE QDProcs prprocs;
PRIVATE QDProcs sendpsprocs;
PRIVATE boolean_t need_restore;

#if defined(BINCOMPAT)
#define GOOFYCAST
#else
#define GOOFYCAST	(Ptr)
#endif

P3(PUBLIC pascal trap, void, PrComment, INTEGER, kind, INTEGER, size,
								  Handle, hand)
{
    SignedByte state;
    INTEGER *ip, flippage, angle;
    Fixed *fp, yoffset, xoffset;

    if (pageno >= pagewanted && pageno <= lastpagewanted)
	{
	    switch (kind) {
	        case textbegin:
		    do_textbegin ((TTxtPicHdl) hand);
	            break;
	        case textend:
		    do_textend ();
	            break;
	        case textcenter:
		    do_textcenter ((TCenterRecHdl) hand);
	            break;
		case rotatebegin:
		    ip = (INTEGER *) STARH(hand);
		    flippage = CW(ip[0]);
		    angle = CW(ip[1]);
		    ROMlib_rotatebegin(flippage, angle);
		    break;
		case rotatecenter:
		    fp = (Fixed *) STARH(hand);
		    yoffset = CL(fp[0]);
		    xoffset = CL(fp[1]);
		    ROMlib_rotatecenter( Cx(thePort->pnLoc.v) +
					       (double) yoffset / (1L << 16),
					 Cx(thePort->pnLoc.h) +
					       (double) xoffset / (1L << 16) );
		    break;
		case rotateend:
		    ROMlib_rotateend();
		    break;
	        case psbeginnosave:
		  need_restore = FALSE;
		  /* FALL THROUGH */
		case postscriptbegin:
		    if (ROMlib_passpostscript)
		      {
			if (kind == postscriptbegin)
			  {
			    ROMlib_gsave();
			    need_restore = TRUE;
			  }
			thePort->grafProcs = RM(&sendpsprocs);
		    }
		    break;
		case postscriptend:
		    if (ROMlib_passpostscript)
		      {
			if (need_restore)
			  {
			    ROMlib_grestore();
			    need_restore = FALSE;
			  }
			thePort->grafProcs = RM(&prprocs);
		      }
		    break;
		case postscripttextis:
		    if (ROMlib_passpostscript)
			(PORT_GRAF_PROCS (thePort))->textProc
			  = RM(GOOFYCAST P_textasPS);
		    break;
		case postscripthandle:
		    if (pageno >= pagewanted && pageno <= lastpagewanted && ROMlib_passpostscript) {
			state = HGetState(hand);
			HLock(hand);
			NeXTsendps(size, (comPtr) STARH(hand));
			HSetState(hand, state);
		    }
		    break;
		case postscriptfile:
		default:
		  warning_unimplemented ("kind = %d", kind);
		    break;
	    }
	}
}

PRIVATE boolean_t printport_open_p = FALSE;

PRIVATE void
ourinit (TPPrPort port, BOOLEAN preserve_font)
{
  INTEGER saved_font;

  saved_font = port->gPort.txFont;

  printer_init ();
  update_printing_globals ();

    if (!printport_open_p) {
	OpenPort((GrafPtr) &printport);
	printport_open_p = TRUE;
    } else
	InitPort((GrafPtr) &printport);
    printport.txFont = CWC (-32768);	/* force reload */
    printport.pnLoc.h = CWC (-32768);
    printport.pnLoc.v = CWC (-32768);
    OpenPort(&port->gPort);
    sendpsprocs.textProc = RM(P_donotPrText);
    sendpsprocs.lineProc = RM(P_donotPrLine);
    sendpsprocs.rectProc = RM(P_donotPrRect);
    sendpsprocs.rRectProc = RM(P_donotPrRRect);
    sendpsprocs.ovalProc = RM(P_donotPrOval);
    sendpsprocs.arcProc = RM(P_donotPrArc);
    sendpsprocs.polyProc = RM(P_donotPrPoly);
    sendpsprocs.rgnProc = RM(P_donotPrRgn);
    sendpsprocs.bitsProc = RM(P_donotPrBits);
    sendpsprocs.commentProc = RM(P_PrComment);
    sendpsprocs.txMeasProc = RM(P_PrTxMeas);
#if 0
    sendpsprocs.getPicProc = RM(P_donotPrGetPic);
    sendpsprocs.putPicProc = RM(P_donotPrPutPic);
#else
    sendpsprocs.getPicProc = RM(P_StdGetPic);
    sendpsprocs.putPicProc = RM(P_StdPutPic);
#endif
    prprocs.textProc = RM(P_PrText);
    prprocs.lineProc = RM(P_PrLine);
    prprocs.rectProc = RM(P_PrRect);
    prprocs.rRectProc = RM(P_PrRRect);
    prprocs.ovalProc = RM(P_PrOval);
    prprocs.arcProc = RM(P_PrArc);
    prprocs.polyProc = RM(P_PrPoly);
    prprocs.rgnProc = RM(P_PrRgn);
    prprocs.bitsProc = RM(P_PrBits);
    prprocs.commentProc = RM(P_PrComment);
    prprocs.txMeasProc = RM(P_PrTxMeas);
#if 0
    prprocs.getPicProc = RM(P_PrGetPic);
    prprocs.putPicProc = RM(P_PrPutPic);
#else
    prprocs.getPicProc = RM(P_StdGetPic);
    prprocs.putPicProc = RM(P_StdPutPic);
#endif
    port->saveprocs = prprocs;
#if 1
    port->gPort.device = CWC(1);
#endif
    port->gPort.grafProcs = RM(&port->saveprocs);

    SetRect (&port->gPort.portBits.bounds,
	     -1 * ROMlib_resolution_x / 2,
	     -1 * ROMlib_resolution_y / 2,
	     ROMlib_paper_x - (ROMlib_resolution_x / 2),
	     ROMlib_paper_y - (ROMlib_resolution_y / 2));
    SetRect (&port->gPort.portRect, 0, 0,
	     ROMlib_paper_x - 72, ROMlib_paper_y - 72);
    HxX (MR (port->gPort.visRgn), rgnBBox) = port->gPort.portRect;

    if (preserve_font)
      port->gPort.txFont = saved_font;

    NeXTOpenPage();
}

#if !defined(NEXTSTEP)
#include <stdio.h>
#endif

PRIVATE boolean_t need_pclose;

#if defined (LINUX)
PRIVATE void (*old_pipe_signal)(int);
#endif

#if !defined (NEXTSTEP)
PRIVATE FILE *
open_ps_file (boolean_t *need_pclosep)
{
  FILE *retval;

  retval = NULL;
  *need_pclosep = FALSE;

#if defined (LINUX)
  if (strcmp (ROMlib_printer, "PostScript File") != 0)
    {
      value_t prog;

      prog = find_key ("Printer", ROMlib_printer);
      if (prog)
	retval = popen (prog, "w");
      if (retval)
	{
	  old_pipe_signal = signal (SIGPIPE, SIG_IGN);
	  *need_pclosep = TRUE;
	}
    }
#endif

#if defined (MSDOS) || defined (CYGWIN32)
  if (strcmp (ROMlib_printer, "Direct To Port") == 0)
    {
      value_t port;

      port = find_key ("Port", ROMlib_port);
      if (port)
	retval = fopen (port, "w");
    }
#endif

  /* 
   * Under DOS we don't print to a pipe, so we open ROMlib_spool_file
   * and then through that file to GhostScript.
   */

  if (!retval)
    {
#if !defined (MSDOS) && !defined (CYGWIN32)
      if (strcmp (ROMlib_printer, "PostScript File") == 0)
#endif
	{
	  if (!ROMlib_spool_file)
	    {
	      warning_unexpected (NULL_STRING);
	      retval = NULL;
	    }
	  else
	    {
	      retval = Ufopen (ROMlib_spool_file, "w");
#if !defined (MSDOS) && !defined (CYGWIN32)
	      free (ROMlib_spool_file);
	      ROMlib_spool_file = NULL;
#endif
	    }
	}
    }
  return retval;
}
#endif /* !NEXTSTEP */

PRIVATE boolean_t already_open = FALSE;

#if defined (QUESTIONABLE_FIX_FOR_LOGBOOK_THAT_BREAKS_PRINTING_UNDER_TESTGEN)
PRIVATE Byte save_FractEnable;
#endif

#if defined (CYGWIN32)
PRIVATE THPrint last_thprint;
PRIVATE uint32 job_dialog_count;
PRIVATE uint32 job_dialog_desired = 1;
#endif

PRIVATE void
call_job_dialog_if_needed (THPrint thprint)
{
#if defined (CYGWIN32)
  if (thprint != last_thprint || job_dialog_count < job_dialog_desired)
    {
      PrJobDialog (thprint);
      last_thprint = thprint;
      job_dialog_count = job_dialog_desired;
    }
  ++job_dialog_desired;
#endif
}

PUBLIC void
ROMlib_acknowledge_job_dialog (THPrint thprint)
{
#if defined (CYGWIN32)
  last_thprint = thprint;
  ++job_dialog_count;
#endif
}

P3(PUBLIC pascal trap, TPPrPort, PrOpenDoc, THPrint, hPrint, TPPrPort, port,
								   Ptr, pIOBuf)
{
    call_job_dialog_if_needed (hPrint);
    if (port) {
	port->fOurPtr = CB(FALSE);
    } else {
	port = (TPPrPort) NewPtr((Size) sizeof(TPrPort));
	port->fOurPtr = CB(TRUE);
    }
    ourinit(port, FALSE);

#if !defined (NEXTSTEP)
    pagewanted = Hx(hPrint, prJob.iFstPage);
    lastpagewanted  = Hx(hPrint, prJob.iLstPage);
#else
    lastpagewanted = 9999;
#endif

#if !defined(NEXTSTEP)
    if (!already_open)
      {
#if defined (QUESTIONABLE_FIX_FOR_LOGBOOK_THAT_BREAKS_PRINTING_UNDER_TESTGEN)
	save_FractEnable = FractEnable;
	FractEnable = 0xff;
#endif

	if (!ROMlib_printfile)
	  ROMlib_printfile = open_ps_file(&need_pclose);

	if (ROMlib_printfile)
	  {
	    Handle h;
	    Ptr p;
	    int len;

	    h = GetResource (TICK ("PREC"), 103);
	    if (h)
	      {
		p = STARH (h);
		len = GetHandleSize (h);
	      }
	    else
	      {
		p = NULL;
		len = 0;
	      }
	    fprintf (ROMlib_printfile, ROMlib_doc_begin,
		     ROMlib_document_paper_sizes, ROMlib_paper_size_name,
		     ROMlib_paper_size_name_terminator,
		     ROMlib_paper_orientation);
	    fprintf (ROMlib_printfile, ROMlib_doc_prolog);
	    fprintf (ROMlib_printfile, ROMlib_doc_end_prolog,
		     ROMlib_paper_size, ROMlib_paper_size_name,
		     ROMlib_paper_size_name_terminator,
		     Hx(hPrint, prJob.iCopies), len, p);
	  }
	pageno = 0;
      }
#else
    printstate = seenOpenDoc;
    contextswitch(&romlib_sp, &nextstep_sp);
#endif
    already_open = TRUE;
    return port;
}

P2(PUBLIC pascal trap, void, PrOpenPage, TPPrPort, port, TPRect, pPageFrame)
{
  ++pageno;
  ourinit(port, TRUE);
  if (pageno >= pagewanted && pageno <= lastpagewanted)
    {
#if !defined(NEXTSTEP)
      if (ROMlib_printfile)
	fprintf(ROMlib_printfile, ROMlib_page_begin, pageno - pagewanted + 1,
		pageno - pagewanted + 1,
		ROMlib_paper_x, ROMlib_paper_y, ROMlib_paper_size,
		ROMlib_paper_size_name, ROMlib_paper_size_name_terminator,
		ROMlib_rotation, ROMlib_translate_x, ROMlib_translate_y,
		72.0 / ROMlib_resolution_x, -1 * 72.0 / ROMlib_resolution_y);
      ROMlib_suppressclip = 0;
#else
      printstate = seenOpenPage;
      contextswitch(&romlib_sp, &nextstep_sp);
#endif
    }
  page_is_open = TRUE;
}

P1(PUBLIC pascal trap, void, PrClosePage, TPPrPort, pPrPort)
{
  if (page_is_open)
    {
      if (pageno >= pagewanted && pageno <= lastpagewanted)
	{
#if !defined(NEXTSTEP)
	  if (ROMlib_printfile)
	    fprintf(ROMlib_printfile, ROMlib_page_end,
		    -55 + (ROMlib_rotation ? 30 : 0));
#else
	  printstate = seenClosePage;
	  contextswitch(&romlib_sp, &nextstep_sp);
#endif
	}
    }
  page_is_open = FALSE;
}

#if defined (MSDOS) || defined (CYGWIN32)

#if defined (MSDOS)
#include <process.h>
#include "dosevq.h"
#else
#include <process.h>

#if !defined P_WAIT
#define P_WAIT 1
#endif

#endif

#define BATCH_FILE_NAME "+\\print.bat"

PRIVATE void
backslash_string (char *p)
{
  for (; *p; ++p)
    if (*p == '/')
      *p = '\\';
}

PRIVATE void
invoke_print_batch_file (const char *filename, ini_key_t printer, ini_key_t port)
{
  int spawn_result;
  char *batch_file;
  char *backslashed_filename;
  char *backslashed_start_dir;

  batch_file = copystr (BATCH_FILE_NAME);
  backslashed_filename = alloca (strlen (filename) + 1);
  strcpy (backslashed_filename, filename);
  backslash_string (backslashed_filename);

  backslashed_start_dir = alloca (strlen (ROMlib_startdir) + 1);
  strcpy (backslashed_start_dir, ROMlib_startdir);
  backslash_string (backslashed_start_dir);

  if (deferred_printing_p)
    {
      add_to_cleanup ("cd %s\n%s %s %s %s %s\n", backslashed_start_dir,
		      batch_file, backslashed_start_dir, backslashed_filename,
		      printer, port);
    }
  else
    {
      spawn_result = spawnl (P_WAIT, batch_file, BATCH_FILE_NAME,
			     backslashed_start_dir, backslashed_filename,
			     printer, port, 0);

      warning_trace_info ("spawn(\"%s\", \"%s\", \"%s\", \"%s\", \"%s\") = %d",
			  batch_file, backslashed_start_dir,
			  backslashed_filename,
			  printer, port, spawn_result);
    }
  free (batch_file);

#if defined (MSDOS)
  if (!deferred_printing_p)
    {
      need_hacky_screen_update = TRUE;
      dosevq_reinit_mouse ();
    }
#endif
}
#endif

P1(PUBLIC pascal trap, void, PrCloseDoc, TPPrPort, port)
{
#if !defined(NEXTSTEP)
  if (ROMlib_printfile)
    {
      fprintf(ROMlib_printfile, ROMlib_doc_end, pageno - pagewanted + 1,
	      ROMlib_paper_x, ROMlib_paper_y);
      fflush (ROMlib_printfile);
    }
  
  if (need_pclose)
    {
#if !defined (CYGWIN32)
      pclose(ROMlib_printfile);
#else
      ; /* CYGWIN32 has no pclose */
#endif
#if defined (LINUX)
      signal (SIGPIPE, old_pipe_signal);
#endif
    }
  else
    if (ROMlib_printfile)
      {
	fclose(ROMlib_printfile);
#if defined (MSDOS) || defined (CYGWIN32)
#if defined (CYGWIN32)
	warning_trace_info ("ROMlib_printer = %s, WIN32_TOKEN = %s",
			    ROMlib_printer, WIN32_TOKEN);
	if (strcmp (ROMlib_printer, WIN32_TOKEN) == 0)
	  {
	    print_file (ROMlib_wp, ROMlib_spool_file, NULL);
	  }
	else
#endif
	{
	  if (strcmp (ROMlib_printer, "PostScript File") != 0 &&
	      strcmp (ROMlib_printer, "Direct To Port") != 0)
	    {
	      value_t printer, port;

	      printer = find_key ("Printer", ROMlib_printer);
	      port = find_key ("Port", ROMlib_port);
	      invoke_print_batch_file (ROMlib_spool_file, printer, port);
	    }
	}
#endif
      }
  ROMlib_printfile = 0;
#else
  printstate = __idle;

#if 0 /* DO NOT call the dirty rect stuff ... we're still redirected away
	 from the screen */
  dirty_rect_accrue (0, 0, vdriver_height, vdriver_width);
  dirty_rect_update_screen ();
#endif

  free (ROMlib_spool_file); /* so we'll do unique name again later */
  ROMlib_spool_file = 0;

#endif
  if (port->fOurPtr)
    DisposPtr((Ptr) port);
#if defined (QUESTIONABLE_FIX_FOR_LOGBOOK_THAT_BREAKS_PRINTING_UNDER_TESTGEN)
  FractEnable = save_FractEnable;
#endif
  already_open = FALSE;
  
  if (printport_open_p)
    {
      ClosePort ((GrafPtr) &printport);
      printport_open_p = FALSE;
    }

}

P5(PUBLIC pascal trap, void, PrPicFile, THPrint, hPrint, TPPrPort, pPrPort,
	    Ptr, pIOBuf, Ptr, pDevBuf, TPrStatus *, prStatus)	/* TODO */
{
  warning_unimplemented (NULL_STRING);
}

PUBLIC void
print_reinit (void)
{
  printport_open_p = FALSE;
}
