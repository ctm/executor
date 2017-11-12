/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_dump[] =
		"$Id: dump.c 88 2005-05-25 03:59:37Z ctm $";
#endif

#if !defined (NDEBUG)

/* dump.c; convenience functions for dumping various mac datastructures */

#if !defined (THINK_C)
/* executor */
#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"
#include "DialogMgr.h"
#include "ControlMgr.h"
#include "MenuMgr.h"
#include "FileMgr.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"
#include "rsys/ctl.h"
#include "rsys/itm.h"
#include "rsys/menu.h"
#include "rsys/dump.h"
#include "rsys/string.h"
#include "rsys/mman_private.h"

#include "rsys/print.h"

#define deref(x) STARH (x)

#define pmWindow(p) ((p)->pmWindow)
#define pmPrivate(p) ((p)->pmPrivate)
#define pmDevices(p) ((p)->pmDevices)
#define pmSeeds(p) ((p)->pmSeeds)

#define ciFlags(ci) ((ci)->ciFlags)
#define ciPrivate(ci) ((ci)->ciPrivate)

#else /* mac */

#define deref(x) (*(x))
#define Cx(x) (x)
#define CW(x) (x)
#define CWC(x) (x)
#define CL(x) (x)
#define CLC(x) (x)
#define theCPort ((CGrafPtr) thePort)
#define CGrafPort_p(port) (((char *)(port))[6] & 0xC0)

#define ROWBYTES_VALUE_BITS (0x3FFF)

#define RECT_HEIGHT(r) (CW ((r)->bottom) - CW ((r)->top))
#define RECT_WIDTH(r) (CW ((r)->right) - CW ((r)->left))

/* window accessors */
#define WINDOW_PORT(wp)			(&((WindowPeek) (wp))->port)
#define CWINDOW_PORT(wp)		(&((WindowPeek) (wp))->port)

/* big endian byte order */
#define WINDOW_KIND_X(wp)		(((WindowPeek) (wp))->windowKind)
#define WINDOW_VISIBLE_X(wp)		(((WindowPeek) (wp))->visible)
#define WINDOW_HILITED_X(wp)		(((WindowPeek) (wp))->hilited)
#define WINDOW_GO_AWAY_FLAG_X(wp)	(((WindowPeek) (wp))->goAwayFlag)
#define WINDOW_SPARE_FLAG_X(wp)		(((WindowPeek) (wp))->spareFlag)

#define WINDOW_STRUCT_REGION_X(wp)	(((WindowPeek) (wp))->strucRgn)
#define WINDOW_CONT_REGION_X(wp)	(((WindowPeek) (wp))->contRgn)
#define WINDOW_UPDATE_REGION_X(wp)	(((WindowPeek) (wp))->updateRgn)
#define WINDOW_DEF_PROC_X(wp)		(((WindowPeek) (wp))->windowDefProc)
#define WINDOW_DATA_X(wp)		(((WindowPeek) (wp))->dataHandle)
#define WINDOW_TITLE_X(wp)		(((WindowPeek) (wp))->titleHandle)
#define WINDOW_TITLE_WIDTH_X(wp)	(((WindowPeek) (wp))->titleWidth)
#define WINDOW_CONTROL_LIST_X(wp)	(((WindowPeek) (wp))->controlList)
#define WINDOW_NEXT_WINDOW_X(wp)	(((WindowPeek) (wp))->nextWindow)
#define WINDOW_PIC_X(wp)		(((WindowPeek) (wp))->windowPic)
#define WINDOW_REF_CON_X(wp)		(((WindowPeek) (wp))->refCon)

/* native byte order */
#define WINDOW_KIND(wp)			(Cx (WINDOW_KIND_X (wp)))
#define WINDOW_VISIBLE(wp)		(Cx (WINDOW_VISIBLE_X (wp)))
#define WINDOW_HILITED(wp)		(Cx (WINDOW_HILITED_X (wp)))
#define WINDOW_GO_AWAY_FLAG(wp)		(Cx (WINDOW_GO_AWAY_FLAG_X (wp)))
#define WINDOW_SPARE_FLAG(wp)		(Cx (WINDOW_SPARE_FLAG_X (wp)))

#define WINDOW_STRUCT_REGION(wp)	(Cx (WINDOW_STRUCT_REGION_X (wp)))
#define WINDOW_CONT_REGION(wp)		(Cx (WINDOW_CONT_REGION_X (wp)))
#define WINDOW_UPDATE_REGION(wp)	(Cx (WINDOW_UPDATE_REGION_X (wp)))
#define WINDOW_DEF_PROC(wp)		(Cx (WINDOW_DEF_PROC_X (wp)))
#define WINDOW_DATA(wp)			(Cx (WINDOW_DATA_X (wp)))
#define WINDOW_TITLE(wp)		(Cx (WINDOW_TITLE_X (wp)))
#define WINDOW_TITLE_WIDTH(wp)		(Cx (WINDOW_TITLE_WIDTH_X (wp)))
#define WINDOW_CONTROL_LIST(wp)		(Cx (WINDOW_CONTROL_LIST_X (wp)))
#define WINDOW_NEXT_WINDOW(wp)		(Cx (WINDOW_NEXT_WINDOW_X (wp)))
#define WINDOW_PIC(wp)			(Cx (WINDOW_PIC_X (wp)))
#define WINDOW_REF_CON(wp)		(Cx (WINDOW_REF_CON_X (wp)))

/* dialog accessors */
#define DIALOG_WINDOW(dialog)		((WindowPtr) &((DialogPeek) (dialog))->window)

#define DIALOG_ITEMS_X(dialog)		(((DialogPeek) (dialog))->items)
#define DIALOG_TEXTH_X(dialog)		(((DialogPeek) (dialog))->textH)
#define DIALOG_EDIT_FIELD_X(dialog)	(((DialogPeek) (dialog))->editField)
#define DIALOG_EDIT_OPEN_X(dialog)	(((DialogPeek) (dialog))->editOpen)
#define DIALOG_ADEF_ITEM_X(dialog)	(((DialogPeek) (dialog))->aDefItem)

#define DIALOG_ITEMS(dialog)		(CL (DIALOG_ITEMS_X (dialog)))
#define DIALOG_TEXTH(dialog)		(CL (DIALOG_TEXTH_X (dialog)))
#define DIALOG_EDIT_FIELD(dialog)	(CW (DIALOG_EDIT_FIELD_X (dialog)))
#define DIALOG_EDIT_OPEN(dialog)	(CW (DIALOG_EDIT_OPEN_X (dialog)))
#define DIALOG_ADEF_ITEM(dialog)	(CW (DIALOG_ADEF_ITEM_X (dialog)))

enum pixpat_pattern_types
{
  pixpat_type_orig = 0,
  pixpat_type_color = 1,
  pixpat_type_rgb = 2
};

#define pmWindow(p) (*(GrafPtr *) (&(p)->pmDataFields[0]))
#define pmPrivate(p) (*(short *) (&(p)->pmDataFields[2]))
#define pmDevices(p) (*(long *) (&(p)->pmDataFields[3]))
#define pmSeeds(p) (*(long *) (&(p)->pmDataFields[5]))

#define ciFlags(ci) (*(short *) (&(ci)->ciDataFields[0]))
#define ciPrivate(ci) (*(long *) (&(ci)->ciDataFields[1]))

#include "dump.h"
#endif /* mac */

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

using namespace Executor;

FILE *Executor::o_fp = NULL;

#ifdef MSDOS
/* useful for overriding when under dos */
FILE *stderr_fp = stderr;
FILE *stdout_fp = stdout;
#endif /* MSDOS */

static int indent = 0;

std::string field_name = "";

/* dump everything recursively; but no ctab tables... */
enum dump_flags
{
  dump_normal_flag = 1,
  dump_bitmap_data_flag = 2,
  dump_ctab_flag = 4,
  dump_xfields_flag = 8,
  dump_pixpat_fields_flag = 16
};

int Executor::dump_verbosity = 1;


void
dump_set_field (int field)
{
  dump_verbosity |= field;
}

void
dump_clear_field (int field)
{
  dump_verbosity &= ~field;
}

#define dump_field(dump_fn, dump_arg, field)	\
  do						\
    {						\
      field_name = field" ";			\
      dump_fn (dump_arg);			\
      field_name = "";				\
    } while (0)

#define iprintf(args)				\
  do						\
    {						\
      dump_spaces (indent);			\
      fprintf args;				\
    } while (0)

Rect big_rect = { CWC ((INTEGER)-32767), CWC ((INTEGER)-32767), CWC (32767), CWC (32767) };

void
Executor::dump_init (char *dst)
{
  if (dst)
    {
      o_fp = Ufopen (dst, "w");
      if (o_fp == NULL)
	exit (1);
    }
  else
    o_fp = stderr;
}

void
dump_finish (void)
{
  fflush (o_fp);
  if (!(o_fp == stderr))
    fclose (o_fp);
}

static void
dump_spaces (int nspaces)
{
  while (nspaces --)
    fputc (' ', o_fp);
  fflush (o_fp);
}

void
Executor::dump_ptr_real (Ptr x)
{
  if (x)
    iprintf ((o_fp, "%s%p[0x%lx]\n", field_name.c_str(), x,
             (long) GetPtrSize (x)));
  else
    iprintf ((o_fp, "%s%p\n", field_name.c_str(), x));
  fflush (o_fp);
}

void
Executor::dump_handle_real (Handle x)
{
  if (x)
    iprintf ((o_fp, "%s%p[0x%x] (%p)\n", field_name.c_str(), x,
	      GetHandleSize (x), deref (x)));
  else
    iprintf ((o_fp, "%s%p\n", field_name.c_str(), x));
  fflush (o_fp);
}

void
Executor::dump_rect (Rect *r)
{
  iprintf ((o_fp, "%s(Rect *%p) {\n", field_name.c_str(), r));  indent += 2;
  iprintf ((o_fp, "top 0x%x; left 0x%x;\n", CW (r->top), CW (r->left)));
  iprintf ((o_fp, "bottom 0x%x; right 0x%x; }\n", 
	    CW (r->bottom), CW (r->right)));  indent -= 2;
  fflush (o_fp);
}

void
Executor::dump_pattern (Pattern x)
{
  iprintf ((o_fp, "%s0x%02x%02x%02x%02x%02x%02x%02x%02x;\n", field_name.c_str(),
            x[0], x[1], x[2], x[3], 
            x[4], x[5], x[6], x[7]));
  fflush (o_fp);
}

void
Executor::dump_point (GUEST<Point> x)
{
  iprintf ((o_fp, "%s(Point) { v 0x%x; h 0x%x; }\n",
	    field_name.c_str(), CW (x.v), CW (x.h)));
  fflush (o_fp);
}

void
Executor::dump_bitmap_data (BitMap *x, int depth, Rect *rect)
{
  int rows, bytes_per_row;
  int row_bytes;
  int r, rb;
  char *addr;
  
  iprintf ((o_fp, "...%s data\n", field_name.c_str())); indent += 2;

  if (!rect)
    rect = &x->bounds;
  
  row_bytes = CW (x->rowBytes) & ROWBYTES_VALUE_BITS;
  addr = (char *)
    &MR (x->baseAddr)[(CW (rect->top) - CW (x->bounds.top)) * row_bytes
			   + ((CW (rect->left) - CW (x->bounds.left))
			      * depth) / 8];
  rows = RECT_HEIGHT (&x->bounds);
  bytes_per_row = (RECT_WIDTH (&x->bounds) * depth + 7) / 8;
  
  for (r = 0; r < rows; r ++)
    {
      iprintf ((o_fp, "%p: ", addr));
      for (rb = 0; rb < bytes_per_row; rb ++)
	{
	  char byte;
	  static const char x_digit[] =
	    {
	      '0', '1', '2', '3', '4', '5', '6', '7',
	      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	    };

	  byte = addr[rb];
	  fprintf (o_fp, "%c%c",
		   x_digit[(byte >> 4) & 15], x_digit[byte & 15]);
	}
      fprintf (o_fp, "\n");
      addr += row_bytes;
    }
  indent -= 2;
  fflush (o_fp);
}

void
Executor::dump_bits16 (GUEST<Bits16> data)
{
  if (dump_verbosity >= 3)
    {
      BitMap x;
      
      x.baseAddr = RM ((Ptr) data);
      x.rowBytes = CWC (2);
      x.bounds.top = x.bounds.left = 0;
      x.bounds.bottom = CWC (16);
      x.bounds.right = CWC (16);
      
      dump_bitmap_data (&x, 1, NULL);
    }
  else
    iprintf ((o_fp, "[%s field omitted]\n", field_name.c_str()));
  fflush (o_fp);
}

void
Executor::dump_bitmap (BitMap *x, Rect *rect)
{
  iprintf ((o_fp, "%s(BitMap *%p) {\n", field_name.c_str(), x));  indent += 2;
  iprintf ((o_fp, "baseAddr %p;\n", MR (x->baseAddr)));
  if (dump_verbosity >= 3)
    dump_bitmap_data (x, 1, rect);
  iprintf ((o_fp, "rowBytes 0x%hx;\n", (unsigned short) CW (x->rowBytes)));
  dump_field (dump_rect, &x->bounds, "bounds");
  indent -= 2; iprintf ((o_fp, "}\n")); 
  fflush (o_fp);
}

void
Executor::dump_bitmap_null_rect (BitMap *x)
{
  dump_bitmap (x, NULL);
}

void
Executor::dump_grafport (GrafPtr x)
{
  if (CGrafPort_p (x))
    dump_cgrafport_real ((CGrafPtr) x);
  else
    dump_grafport_real (x);
  fflush (o_fp);
}

void
Executor::dump_qdprocs (QDProcsPtr x)
{
  iprintf ((o_fp, "%s(QDProcsPtr *%p) {\n", field_name.c_str(), x));  indent += 2;
  if (x != NULL)
    {
      iprintf ((o_fp, "textProc    %p;\n", MR (x->textProc)));
      iprintf ((o_fp, "lineProc    %p;\n", MR (x->lineProc)));
      iprintf ((o_fp, "rectProc    %p;\n", MR (x->rectProc)));
      iprintf ((o_fp, "rRectProc   %p;\n", MR (x->rRectProc)));
      iprintf ((o_fp, "ovalProc    %p;\n", MR (x->ovalProc)));
      iprintf ((o_fp, "arcProc     %p;\n", MR (x->arcProc)));
      iprintf ((o_fp, "polyProc    %p;\n", MR (x->polyProc)));
      iprintf ((o_fp, "rgnProc     %p;\n", MR (x->rgnProc)));
      iprintf ((o_fp, "bitsProc    %p;\n", MR (x->bitsProc)));
      iprintf ((o_fp, "commentProc %p;\n", MR (x->commentProc)));
      iprintf ((o_fp, "txMeasProc  %p;\n", MR (x->txMeasProc)));
      iprintf ((o_fp, "getPicProc  %p;\n", MR (x->getPicProc)));
      iprintf ((o_fp, "putPicProc  %p;\n", MR (x->putPicProc)));
    }
  else
    iprintf ((o_fp, "<default grafprocs used>\n"));
  indent -= 2; iprintf ((o_fp, "}\n"));
  fflush (o_fp);
}

void
Executor::dump_grafport_real (GrafPtr x)
{
  iprintf ((o_fp, "%s(GrafPort *%p) {\n", field_name.c_str(), x));  indent += 2;
  iprintf ((o_fp, "device %d;\n", CW (x->device)));
  dump_field (dump_bitmap_null_rect, &x->portBits, "portBits");
  dump_field (dump_rect, &x->portRect, "portRect");
  dump_field (dump_handle, MR (x->visRgn), "visRgn");
  dump_field (dump_handle, MR (x->clipRgn), "clipRgn");
  dump_field (dump_pattern, x->bkPat, "bkPat");
  dump_field (dump_pattern, x->fillPat, "fillPat");
  dump_field (dump_point, x->pnLoc, "pnLoc");
  dump_field (dump_point, x->pnSize, "pnSize");
  iprintf ((o_fp, "pnMode %d;\n", CW (x->pnMode)));
  dump_field (dump_pattern, x->pnPat, "pnPat");
  iprintf ((o_fp, "pnVis %d;\n", CW (x->pnVis)));
  iprintf ((o_fp, "txFont %d;\n", CW (x->txFont)));
  iprintf ((o_fp, "txFace %d;\n", x->txFace));
  iprintf ((o_fp, "txMode %d;\n", CW (x->txMode)));
  iprintf ((o_fp, "txSize %d;\n", CW (x->txSize)));
  iprintf ((o_fp, "spExtra %d;\n", CL (x->spExtra)));
  iprintf ((o_fp, "fgColor 0x%x;\n", CL (x->fgColor)));
  iprintf ((o_fp, "bkColor 0x%x;\n", CL (x->bkColor)));
  iprintf ((o_fp, "colrBit %d;\n", CW (x->colrBit)));
  iprintf ((o_fp, "patStretch %d;\n", CW (x->patStretch)));
  dump_field (dump_handle, MR (x->picSave), "picSave");
  dump_field (dump_handle, MR (x->rgnSave), "rgnSave");
  dump_field (dump_handle, MR (x->polySave), "polySave");
  dump_field (dump_qdprocs, MR (x->grafProcs), "grafProcs");
  indent -= 2; iprintf ((o_fp, "}\n"));
  fflush (o_fp);
}

GrafPtr
theport (void)
{
  return thePort;
}

void
Executor::dump_theport (void)
{
  dump_grafport (thePort);
}

void
Executor::dump_rgb_color (RGBColor *x)
{
  iprintf ((o_fp, "%s(RGBColor) { red 0x%lx; green 0x%lx, blue 0x%lx; }\n",
	    field_name.c_str(),
	    (long) CW (x->red), 
	    (long) CW (x->green), 
	    (long) CW (x->blue)));
  fflush (o_fp);
}

void
Executor::dump_ctab (CTabHandle ctab)
{
  CTabPtr x = deref (ctab);

  iprintf ((o_fp, "%s(ColorTable **%p) {\n", field_name.c_str(), ctab));  indent += 2;
  iprintf ((o_fp, "ctSeed 0x%x;\n", CL (x->ctSeed)));
  iprintf ((o_fp, "ctFlags 0x%x;\n", CW (x->ctFlags)));
  iprintf ((o_fp, "ctSize %d;\n", CW (x->ctSize)));
  if (dump_verbosity >= 2)
    {
      int i;

      iprintf ((o_fp, "ctTable\n"));
      for (i = 0; i <= CW (x->ctSize); i ++)
	{
	  iprintf ((o_fp, "%d:[0x%x] { 0x%lx, 0x%lx, 0x%lx }\n",
		    i, CW (x->ctTable[i].value), 
		    (long) CW (x->ctTable[i].rgb.red),
		    (long) CW (x->ctTable[i].rgb.green),
		    (long) CW (x->ctTable[i].rgb.blue)));
	}
      indent -= 2; iprintf ((o_fp, "}\n"));
    }
  else
    {
      iprintf ((o_fp, "[ctTable field omitted]; }\n")); indent -= 2;
    }
  fflush (o_fp);
}

void
Executor::dump_itab (ITabHandle itab)
{
  ITabPtr x = deref (itab);

  iprintf ((o_fp, "%s(ITab **%p) {\n", field_name.c_str(), itab));  indent += 2;
  iprintf ((o_fp, "iTabSeed 0x%x;\n", CL (x->iTabSeed)));
  iprintf ((o_fp, "iTabRes %d;\n", CW (x->iTabRes)));
  
  /* we always omit the inverse table... */
  iprintf ((o_fp, "[iTTable field omitted]; }\n"));  indent -= 2;
  fflush (o_fp);
}

void
Executor::dump_pixpat (PixPatHandle pixpat)
{
  PixPatPtr x = deref (pixpat);
  
  iprintf ((o_fp, "%s(PixPat **%p) {\n", field_name.c_str(), pixpat));  indent += 2;
  iprintf ((o_fp, "patType %s;\n",
	    x->patType == CWC (pixpat_old_style_pattern)
	    ? "old_style_pattern"
	    : (x->patType == CWC (pixpat_color_pattern)
	       ? "color_pattern"
	       : (x->patType == CWC (pixpat_rgb_pattern)
		  ? "rgb_pattern"
		  : "<unknown!>"))));

  if (x->patType != CWC (pixpat_type_orig))
    {
      if (dump_verbosity
	  && x->patMap)
	dump_field (dump_pixmap_null_rect, MR (x->patMap), "patMap");
      else
	dump_field (dump_handle, MR (x->patMap), "patMap");
      dump_field (dump_handle, MR (x->patData), "patData");
    }
  else
    {
      iprintf ((o_fp, "[pat{Map, Data} field omitted]; }\n"));
    }
  dump_field (dump_handle, MR (x->patXData), "patXData");
  iprintf ((o_fp, "patXValid %d;\n", CW (x->patXValid)));
  if (dump_verbosity
      && x->patXMap
      && !x->patXValid)
    dump_field (dump_pixmap_null_rect, (PixMapHandle) MR (x->patXMap),
		"patXMap");
  else
    dump_field (dump_handle, MR (x->patXMap), "patXMap");
  dump_field (dump_pattern, x->pat1Data, "pat1Data");
  indent -= 2; iprintf ((o_fp, "}\n"));
  fflush (o_fp);
}

void
dump_pixmap_ptr (PixMapPtr x, Rect *rect)
{
  iprintf ((o_fp, "%s(PixMap *%p) {\n", field_name.c_str(), x));  indent += 2;
  iprintf ((o_fp, "baseAddr %p;\n", MR (x->baseAddr)));
  if (dump_verbosity >= 3
      && x->baseAddr)
    {
      if (!rect)
	  rect = &x->bounds;
      dump_bitmap_data ((BitMap *) x, CW (x->pixelSize), rect);
    }
  iprintf ((o_fp, "rowBytes 0x%hx;\n", (unsigned short) CW (x->rowBytes)));
  dump_field (dump_rect, &x->bounds, "bounds");
  iprintf ((o_fp, "pmVersion 0x%x;\n", CW (x->pmVersion)));
  iprintf ((o_fp, "packType 0x%x;\n", CW (x->packType)));
  iprintf ((o_fp, "packSize 0x%x;\n", CL (x->packSize)));
  iprintf ((o_fp, "hRes 0x%x, vRes 0x%x;\n", 
	    CL (x->hRes), CL (x->vRes)));
  iprintf ((o_fp, "pixelType 0x%x;\n", CW (x->pixelType)));
  iprintf ((o_fp, "pixelSize %d;\n", CW (x->pixelSize)));
  iprintf ((o_fp, "cmpCount %d;\n", CW (x->cmpCount)));
  iprintf ((o_fp, "cmpSize %d;\n", CW (x->cmpSize)));
  iprintf ((o_fp, "planeBytes 0x%x;\n", CL (x->planeBytes)));
  if (dump_verbosity
      && x->pmTable)
    dump_field (dump_ctab, MR (x->pmTable), "pmTable");
  else
    dump_field (dump_handle, MR (x->pmTable), "pmTable");
  iprintf ((o_fp, "[Reserved field omitted]; }\n"));  indent -= 2;
  fflush (o_fp);
}

void
Executor::dump_pixmap_null_rect (PixMapHandle pixmap)
{
  dump_pixmap (pixmap, NULL);
}

void
Executor::dump_pixmap (PixMapHandle pixmap, Rect *rect)
{
  PixMapPtr x = deref (pixmap);
  
  iprintf ((o_fp, "%s(PixMap **%p) {\n", field_name.c_str(), pixmap));  indent += 2;
  iprintf ((o_fp, "baseAddr %p;\n", MR (x->baseAddr)));
  if (dump_verbosity >= 3
      && x->baseAddr)
    {
      if (!rect)
	  rect = &x->bounds;
      dump_bitmap_data ((BitMap *) x, CW (x->pixelSize), rect);
    }
  iprintf ((o_fp, "rowBytes 0x%hx;\n", (unsigned short) CW (x->rowBytes)));
  dump_field (dump_rect, &x->bounds, "bounds");
  iprintf ((o_fp, "pmVersion 0x%x;\n", CW (x->pmVersion)));
  iprintf ((o_fp, "packType 0x%x;\n", CW (x->packType)));
  iprintf ((o_fp, "packSize 0x%x;\n", CL (x->packSize)));
  iprintf ((o_fp, "hRes 0x%x, vRes 0x%x;\n", 
	    CL (x->hRes), CL (x->vRes)));
  iprintf ((o_fp, "pixelType 0x%x;\n", CW (x->pixelType)));
  iprintf ((o_fp, "pixelSize %d;\n", CW (x->pixelSize)));
  iprintf ((o_fp, "cmpCount %d;\n", CW (x->cmpCount)));
  iprintf ((o_fp, "cmpSize %d;\n", CW (x->cmpSize)));
  iprintf ((o_fp, "planeBytes 0x%x;\n", CL (x->planeBytes)));
  if (dump_verbosity
      && x->pmTable)
    dump_field (dump_ctab, MR (x->pmTable), "pmTable");
  else
    dump_field (dump_handle, MR (x->pmTable), "pmTable");
  iprintf ((o_fp, "[Reserved field omitted]; }\n"));  indent -= 2;
  fflush (o_fp);
}

void
Executor::dump_cqdprocs (CQDProcsPtr x)
{
  iprintf ((o_fp, "%s(CQDProcsPtr *%p) {\n", field_name.c_str(), x));  indent += 2;
  if (x != NULL)
    {
      iprintf ((o_fp, "textProc     %p;\n", MR (x->textProc)));
      iprintf ((o_fp, "lineProc     %p;\n", MR (x->lineProc)));
      iprintf ((o_fp, "rectProc     %p;\n", MR (x->rectProc)));
      iprintf ((o_fp, "rRectProc    %p;\n", MR (x->rRectProc)));
      iprintf ((o_fp, "ovalProc     %p;\n", MR (x->ovalProc)));
      iprintf ((o_fp, "arcProc      %p;\n", MR (x->arcProc)));
      iprintf ((o_fp, "polyProc     %p;\n", MR (x->polyProc)));
      iprintf ((o_fp, "rgnProc      %p;\n", MR (x->rgnProc)));
      iprintf ((o_fp, "bitsProc     %p;\n", MR (x->bitsProc)));
      iprintf ((o_fp, "commentProc  %p;\n", MR (x->commentProc)));
      iprintf ((o_fp, "txMeasProc   %p;\n", MR (x->txMeasProc)));
      iprintf ((o_fp, "getPicProc   %p;\n", MR (x->getPicProc)));
      iprintf ((o_fp, "putPicProc   %p;\n", MR (x->putPicProc)));
      iprintf ((o_fp, "newProc1Proc %p;\n", MR (x->newProc1Proc)));
      iprintf ((o_fp, "newProc2Proc %p;\n", MR (x->newProc2Proc)));
      iprintf ((o_fp, "newProc3Proc %p;\n", MR (x->newProc3Proc)));
      iprintf ((o_fp, "newProc4Proc %p;\n", MR (x->newProc4Proc)));
      iprintf ((o_fp, "newProc5Proc %p;\n", MR (x->newProc5Proc)));
      iprintf ((o_fp, "newProc6Proc %p;\n", MR (x->newProc6Proc)));
    }
  else
    iprintf ((o_fp, "<default grafprocs used>\n"));
  indent -= 2; iprintf ((o_fp, "}\n"));
  fflush (o_fp);
}

void
Executor::dump_cgrafport_real (CGrafPtr x)
{
  iprintf ((o_fp, "%s(CGrafPort *%p) {\n", field_name.c_str(), x));  indent += 2;
  iprintf ((o_fp, "device 0x%x;\n", CW (x->device)));
  if (dump_verbosity
      && x->portPixMap)
    dump_field (dump_pixmap_null_rect, MR (x->portPixMap), "portPixMap");
  else
    dump_field (dump_handle, MR (x->portPixMap), "portPixMap");
  iprintf ((o_fp, "portVersion 0x%x;\n", CW (x->portVersion)));
  dump_field (dump_handle, MR (x->grafVars), "grafVars");
  iprintf ((o_fp, "chExtra %d;\n", CW (x->chExtra)));
  iprintf ((o_fp, "pnLocHFrac 0x%x;\n", CW (x->pnLocHFrac)));
  dump_field (dump_rect, &x->portRect, "portRect");
  dump_field (dump_handle, MR (x->visRgn), "visRgn");
  dump_field (dump_handle, MR (x->clipRgn), "clipRgn");
  if (dump_verbosity
      && x->bkPixPat)
    dump_field (dump_pixpat, MR (x->bkPixPat), "bkPixPat");
  else
    dump_field (dump_handle, MR (x->bkPixPat), "bkPixPat");
  dump_field (dump_rgb_color, &x->rgbFgColor, "rgbFgColor");
  dump_field (dump_rgb_color, &x->rgbBkColor, "rgbBkColor");
  dump_field (dump_point, x->pnLoc, "pnLoc");
  dump_field (dump_point, x->pnSize, "pnSize");
  iprintf ((o_fp, "pnMode %d;\n", CW (x->pnMode)));
  if (dump_verbosity
      && x->pnPixPat)
    dump_field (dump_pixpat, MR (x->pnPixPat), "pnPixPat");
  else
    dump_field (dump_handle, MR (x->pnPixPat), "pnPixPat");
  if (dump_verbosity
      && x->fillPixPat)
    dump_field (dump_pixpat, MR (x->fillPixPat), "fillPixPat");
  else
    dump_field (dump_handle, MR (x->fillPixPat), "fillPixPat");
  iprintf ((o_fp, "pnVis %d;\n", CW (x->pnVis)));
  iprintf ((o_fp, "txFont %d;\n", CW (x->txFont)));
  iprintf ((o_fp, "txFace %d;\n", x->txFace));
  iprintf ((o_fp, "txMode %d;\n", CW (x->txMode)));
  iprintf ((o_fp, "txSize %d;\n", CW (x->txSize)));
  iprintf ((o_fp, "spExtra %d;\n", CL (x->spExtra)));
  iprintf ((o_fp, "fgColor 0x%x;\n", CL (x->fgColor)));
  iprintf ((o_fp, "bkColor 0x%x;\n", CL (x->bkColor)));
  iprintf ((o_fp, "colrBit %d;\n", CW (x->colrBit)));
  iprintf ((o_fp, "patStretch %x;\n", CW (x->patStretch)));
  dump_field (dump_handle, MR (x->picSave), "picSave");
  dump_field (dump_handle, MR (x->rgnSave), "rgnSave");
  dump_field (dump_handle, MR (x->polySave), "polySave");
  dump_field (dump_cqdprocs, MR (x->grafProcs), "grafProcs");
  indent -= 2; iprintf ((o_fp, "}\n"));
  fflush (o_fp);
}

void
Executor::dump_gdevice (GDHandle gdev)
{
  GDPtr x = deref (gdev);
  SProcHndl proc;
  
  iprintf ((o_fp, "%s(GDevice **%p) {\n", field_name.c_str(), gdev));  indent += 2;
  iprintf ((o_fp, "gdID 0x%x;\n", CW (x->gdID)));
  iprintf ((o_fp, "gdType 0x%x;\n", CW (x->gdType)));
  if (dump_verbosity
      && x->gdITable)
    dump_field (dump_itab, MR (x->gdITable), "gdITable");
  else
    dump_field (dump_handle, MR (x->gdITable), "gdITable");
  iprintf ((o_fp, "gdResPref 0x%x;\n", CW (x->gdResPref)));
#if 0
  dump_field (dump_handle, CL (x->gdSearchProc), "gdSearchProc");
#else
  iprintf ((o_fp, "gdSearchProc %p;\n", MR (x->gdSearchProc)));
  for (proc = MR (x->gdSearchProc); proc; proc = HxP (proc, nxtSrch))
    iprintf ((o_fp, "  proc [%p, %p]; %p\n",
	      proc, HxP (proc, nxtSrch), HxP (proc, srchProc)));
#endif
  dump_field (dump_handle, MR (x->gdCompProc), "gdCompProc");
  iprintf ((o_fp, "gdFlags 0x%hx;\n", (unsigned short) CW (x->gdFlags)));
  if (dump_verbosity
      && x->gdPMap)
    dump_field (dump_pixmap_null_rect, MR (x->gdPMap), "gdPMap");
  else
    dump_field (dump_handle, MR (x->gdPMap), "gdPMap");
  iprintf ((o_fp, "gdRefCon 0x%x;\n", CL (x->gdRefCon)));
  if (dump_verbosity
      && x->gdNextGD)
    dump_field (dump_gdevice, (GDHandle) MR (x->gdNextGD), "gdNextGD");
  else
    dump_field (dump_handle, MR (x->gdNextGD), "gdNextGD");
  dump_field (dump_rect, &x->gdRect, "gdRect");
  iprintf ((o_fp, "gdMode 0x%x;\n", CL (x->gdMode)));
  iprintf ((o_fp, "[CC, Reserved fields omitted]; }\n")); indent -= 2;
  fflush (o_fp);
}

void
Executor::dump_thegdevice (void)
{
  dump_gdevice (MR (TheGDevice));
}

void
Executor::dump_maindevice (void)
{
  dump_gdevice (MR (MainDevice));
}

void
Executor::dump_string (unsigned char *s)
{
  /* pascal string */
  unsigned char t[256];
  int len;

  len = *s;
  strncpy ((char *) t, (char *) &s[1], len);
  t[len] = '\0';
  iprintf ((o_fp, "%s %p \"%s\";\n",
	    field_name.c_str(), s, t));
}

void
Executor::dump_palette (PaletteHandle palette)
{
  PalettePtr x = deref (palette);

  iprintf ((o_fp, "%s(PaletteHandle **%p) {\n", field_name.c_str(), palette));
  indent += 2;
  iprintf ((o_fp, "pmEntries 0x%x;\n", CW (x->pmEntries)));
  if (pmWindow (x)
      && dump_verbosity >= 2
      && 0)
    dump_grafport ((GrafPtr) MR (pmWindow (x)));
  else
    dump_field (dump_handle, MR (pmWindow (x)), "pmWindow");
  iprintf ((o_fp, "pmPrivate 0x%x\n", CW (pmPrivate (x))));
  iprintf ((o_fp, "pmDevices 0x%lx\n", (long) CL (pmDevices (x))));
  iprintf ((o_fp, "pmSeeds 0x%lx\n", (long) MR (pmSeeds (x))));
  if (dump_verbosity >= 2)
    {
      int i;
      
      iprintf ((o_fp, "pmInfo\n"));
      for (i = 0; i < CW (x->pmEntries); i ++)
	{
	  iprintf ((o_fp, "%3x { rgb { 0x%lx, 0x%lx, 0x%lx }\n",
		    i,
		    (long) CW (x->pmInfo[i].ciRGB.red),
		    (long) CW (x->pmInfo[i].ciRGB.green),
		    (long) CW (x->pmInfo[i].ciRGB.blue)));
	  iprintf ((o_fp, "      usage 0x%x; tolerance 0x%x;\n",
		    CW (x->pmInfo[i].ciUsage),
		    CW (x->pmInfo[i].ciTolerance)));
	  iprintf ((o_fp, "      flags 0x%x; private 0x%lx; };\n",
		    CW (ciFlags (&x->pmInfo[i])),
		    (unsigned long) CL (ciPrivate (&x->pmInfo[i]))));
	}
      indent -= 2; iprintf ((o_fp, "}\n"));
    }
  else
    {
      indent -= 2; iprintf ((o_fp, "[pmInfo field omitted]; }\n"));
    }
  fflush (o_fp);
}

void
Executor::dump_ccrsr (CCrsrHandle ccrsr)
{
  CCrsrPtr x = deref (ccrsr);

  iprintf ((o_fp, "%s(CCrsrHandle **%p) {\n", field_name.c_str(), ccrsr));
  indent += 2;
  iprintf ((o_fp, "crsrType 0x%hx;\n", CW (x->crsrType)));
  if (x->crsrMap
      && dump_verbosity >= 1)
    dump_field (dump_pixmap_null_rect, MR (x->crsrMap), "crsrMap");
  else
    dump_field (dump_handle, MR (x->crsrMap), "crsrMap");
  dump_field (dump_handle, MR (x->crsrData), "crsrData");
  if (dump_verbosity >= 3
      && x->crsrXData
      && x->crsrXValid != CW(0))
    {
      BitMap bm;
      int depth;
      /* dump the expanded pixel data */

      depth = CW (x->crsrXValid);
      bm.baseAddr = RM (deref (MR (x->crsrXData)));
      bm.rowBytes = CW (2 * depth);
      bm.bounds.top = bm.bounds.left = CWC (0);
      bm.bounds.bottom = CWC (16);
      bm.bounds.right = CWC (16);


      field_name = "crsrXData";
      dump_bitmap_data (&bm, depth, NULL);
      field_name = "";
    }
  else
    dump_field (dump_handle, MR (x->crsrXData), "crsrXData");
  iprintf ((o_fp, "crsrXValid %d;\n", CW (x->crsrXValid)));
  dump_field (dump_handle, MR (x->crsrXHandle), "crsrXHandle");
  dump_field (dump_bits16, x->crsr1Data, "crsr1Data");
  dump_field (dump_bits16, x->crsrMask, "crsrMask");
  dump_field (dump_point, x->crsrHotSpot, "crsrHotSpot");
  iprintf ((o_fp, "crsrXTable %x\n", CL (x->crsrXTable)));
  iprintf ((o_fp, "crsrID 0x%x; }\n", CL (x->crsrID)));  indent -= 2;
  fflush (o_fp);
}

void
Executor::dump_wmgrport (void)
{
  dump_grafport (MR (WMgrPort));
}

void
Executor::dump_wmgrcport (void)
{
  dump_grafport ((GrafPtr) MR (WMgrCPort));
}

void
Executor::dump_string_handle (StringHandle sh)
{
  /* pascal string */
  unsigned char *s, t[256];
  int size, len;

  s = deref (sh);
  size = GetHandleSize ((Handle) sh);
  len = *s;
  strncpy ((char *) t, (char *) &s[1], len);
  t[len] = '\0';
  iprintf ((o_fp, "%s %p[%d] \"%s\";\n",
	    field_name.c_str(), sh, size, t));
}

void
Executor::dump_window_peek (WindowPeek w)
{
  iprintf ((o_fp, "%s(WindowPeek %p) {\n", field_name.c_str(), w));
  indent += 2;
  
  dump_field (dump_grafport, WINDOW_PORT (w), "port");
  iprintf ((o_fp, "windowKind %lx;\n", (long) WINDOW_KIND (w)));
  iprintf ((o_fp, "visible %x;\n", WINDOW_VISIBLE (w)));
  iprintf ((o_fp, "hilited %x;\n", WINDOW_HILITED (w)));
  iprintf ((o_fp, "goAwayFlag %x;\n", WINDOW_GO_AWAY_FLAG (w)));
  iprintf ((o_fp, "spareFlag %x;\n", WINDOW_SPARE_FLAG (w)));

  dump_field (dump_handle, WINDOW_STRUCT_REGION (w), "strucRgn");
  dump_field (dump_handle, WINDOW_CONT_REGION (w), "contRgn");
  dump_field (dump_handle, WINDOW_UPDATE_REGION (w), "updateRgn");

  dump_field (dump_handle, WINDOW_DEF_PROC (w), "windowDefProc");
  dump_field (dump_handle, WINDOW_DATA (w), "dataHandle");
  dump_field (dump_string_handle, WINDOW_TITLE (w), "titleHandle");

  iprintf ((o_fp, "titleWidth %lx;\n", (long) WINDOW_TITLE_WIDTH (w)));

  dump_field (dump_handle, WINDOW_CONTROL_LIST (w), "controlList");
  dump_field (dump_ptr, WINDOW_NEXT_WINDOW (w), "nextWindow");
  dump_field (dump_handle, WINDOW_PIC (w), "picHandle");
  
  iprintf ((o_fp, "refCon %lx; }\n", (long) WINDOW_REF_CON (w)));
  indent -= 2;

  fflush (o_fp);
}

void
Executor::dump_dialog_peek (DialogPeek d)
{
  iprintf ((o_fp, "%s(DialogPeek %p) {\n", field_name.c_str(), d));
  indent += 2;
  
  dump_field (dump_window_peek, (WindowPeek) DIALOG_WINDOW (d), "window");
  dump_field (dump_handle, DIALOG_ITEMS (d), "items");
  dump_field (dump_handle, DIALOG_TEXTH (d), "textH");
  iprintf ((o_fp, "editField 0x%x;\n", DIALOG_EDIT_FIELD (d)));
  iprintf ((o_fp, "editOpen 0x%x;\n", DIALOG_EDIT_OPEN (d)));
  iprintf ((o_fp, "aDefItem 0x%x; }\n", DIALOG_ADEF_ITEM (d)));
  indent -= 2;
 
  fflush (o_fp);
}

void
dump_dialog_items (DialogPeek dp)
{
  GUEST<int16> *item_data;
  int n_items;
  itmp items;
  int i;
 
  item_data = (GUEST<int16> *) STARH (DIALOG_ITEMS (dp));
  n_items = CW (*item_data);
  items = (itmp) &item_data[1];
  fprintf (o_fp, "%d items:\n", n_items);
  for (i = 0; i < n_items; i ++)
    {
      itmp item;
       
      item = items;
      fprintf (o_fp, "item %d; type %d, hand %p, (%d, %d, %d, %d)\n",
 	       i, (int) (item->itmtype),
 	       MR (item->itmhand),
 	       CW (item->itmr.top), CW (item->itmr.left), 
 	       CW (item->itmr.bottom), CW (item->itmr.right));
      BUMPIP (items);
    }
}

void
Executor::dump_aux_win_list (void)
{
  AuxWinHandle t;

  for (t = MR (AuxWinHead); t; t = MR (deref (t)->awNext))
    {
      dump_aux_win (t);
    }
}

void
Executor::dump_aux_win (AuxWinHandle awh)
{
  AuxWinPtr aw = deref (awh);
  
  iprintf ((o_fp, "%s(AuxWinHandle **%p) {\n", field_name.c_str(), awh));
  indent += 2;
  iprintf ((o_fp, "awNext %p;\n", MR (aw->awNext)));
  iprintf ((o_fp, "awOwner %p;\n", MR (aw->awOwner)));
  dump_field (dump_ctab, MR (aw->awCTable), "awCTable");
  dump_field (dump_handle, MR (aw->dialogCItem), "dialogCItem");
  iprintf ((o_fp, "awFlags 0x%lx;\n", (long) CL (aw->awFlags)));
  iprintf ((o_fp, "awReserved 0x%lx;\n", (long) MR (aw->awReserved)));
  iprintf ((o_fp, "awRefCon 0x%lx; }\n", (long) CL (aw->awRefCon)));
  indent -= 2;

  fflush (o_fp);
}

void
Executor::dump_window_list (WindowPeek w)
{
  WindowPeek t_w;
  WindowPeek front_w;

  front_w = (WindowPeek) FrontWindow ();
  for (t_w = MR (WindowList);
       t_w;
       t_w = WINDOW_NEXT_WINDOW (t_w))
    {
      fprintf (o_fp, "%p", t_w);
      if (t_w == w)
	fprintf (o_fp, " arg");
      if (t_w == front_w)
	fprintf (o_fp, " front");
      fprintf (o_fp, "\n");
      dump_string_handle (WINDOW_TITLE (t_w));
    }
  fflush (o_fp);
}

void
Executor::dump_rgn (RgnHandle rgn)
{
  RgnPtr x;
  
  iprintf ((o_fp, "%s(RgnHandle **%p) {\n", field_name.c_str(), rgn)); indent += 2;
  x = deref (rgn);
  iprintf ((o_fp, "rgnSize %d\n", CW (x->rgnSize)));
  dump_field (dump_rect, &x->rgnBBox, "rgnBBox");
  iprintf ((o_fp, "[special data omitted]; }\n")); indent -= 2;
  fflush (o_fp);
}

void
dump_menu_info (MenuHandle x)
{
  dump_field (dump_handle, x, "MenuHandle");
  iprintf ((o_fp, "%s(MenuHandle **%p) {\n", field_name.c_str(), x));
  indent += 2;
  iprintf ((o_fp, "menuID %d\n", MI_ID (x)));
  iprintf ((o_fp, "menuWidth %d\n", MI_WIDTH (x)));
  iprintf ((o_fp, "menuHeight %d\n", MI_HEIGHT (x)));
  dump_field (dump_handle, MI_PROC (x), "menuProc");
  iprintf ((o_fp, "enableFlags %x\n", MI_ENABLE_FLAGS (x)));
  dump_field (dump_string, MI_DATA (x), "menuTitle");

  indent += 2;
  {
    unsigned char *p;

    p = MI_DATA (x) + ((unsigned char *)MI_DATA (x))[0] + 1;
    while (*p)
      {
	dump_field (dump_string, p, "item name");
	p += p[0] + 1;
	iprintf ((o_fp, "icon = 0x%02x, keyeq = 0x%02x, marker = 0x%02x,"
		 " style = 0x%02x\n", p[0], p[1], p[2], p[3]));
	p += 4;
      }
    indent -= 2;
    iprintf ((o_fp, "total chars = %ld\n", p - (unsigned char *) STARH(x)));
  }
  indent -= 2;
}

void
dump_control_list (WindowPeek w)
{
  ControlHandle x;

  for (x = WINDOW_CONTROL_LIST (w); x; x = CTL_NEXT_CONTROL (x))
    {
      char buf[256];
      int len;

      len = *CTL_TITLE (x);
      strncpy (buf, (char *) &CTL_TITLE (x)[1], len);
      buf[len] = '\0';
      
      fprintf (o_fp, "%p %s\n", x, buf);
    }
}

void
dump_control (ControlHandle x)
{
  iprintf ((o_fp, "%s(ControlHandle **%p) {\n", field_name.c_str(), x));
  indent += 2;

  dump_field (dump_handle, CTL_NEXT_CONTROL (x), "nextControl");
  dump_field (dump_ptr, CTL_OWNER (x), "contrlOwner");
  dump_field (dump_rect, &CTL_RECT (x), "contrlRect");
  iprintf ((o_fp, "contrlVis %d\n", (int) CTL_VIS (x)));
  iprintf ((o_fp, "contrlHilite %d\n", (int) CTL_HILITE (x)));
  
  iprintf ((o_fp, "contrlValue %d\n", (int) CTL_VALUE (x)));
  iprintf ((o_fp, "contrlMin %d\n", (int) CTL_MIN (x)));
  iprintf ((o_fp, "contrlMax %d\n", (int) CTL_MAX (x)));
  
  dump_field (dump_handle, CTL_DEFPROC (x), "contrlDefProc");
  dump_field (dump_ptr, CTL_DATA (x), "contrlData");
  
  iprintf ((o_fp, "contrlAction %p\n", CTL_ACTION (x)));
  iprintf ((o_fp, "contrlRfCon %d\n", (int) CTL_REF_CON (x)));
  dump_field (dump_string, CTL_TITLE (x), "contrlTitle");
  indent -= 2;
  iprintf ((o_fp, "}\n"));
  

}

void
dump_memlocs( uint32 to_look_for, int size, const void *start_addr,
	     const void *end_addr )
{
  const uint8 *ucp;
  const uint16 *usp;
  const uint32 *ulp;

  switch (size)
    {
    case 1:
      for (ucp = (const uint8*)start_addr; ucp < (uint8 *) end_addr; ++ucp)
	if (*ucp == (uint8) to_look_for)
	  iprintf ((o_fp, "%p\n", ucp));
      break;
    case 2:
      for (usp = (const uint16*)start_addr; usp < (uint16 *) end_addr; ++usp)
	if (*usp == (uint16) to_look_for)
	  iprintf ((o_fp, "%p\n", usp));
      break;
    case 4:
      for (ulp = (const uint32*)start_addr; ulp < (uint32 *) end_addr;
	   ulp = (const uint32 *) ((char *)ulp + 2))
	if (*ulp == to_look_for)
	  iprintf ((o_fp, "%p\n", ulp));
      break;
    default:
      iprintf ((o_fp, "Bad Size\n"));
      break;
    }
}

void
dump_te (TEHandle te)
{
  int i;
  
  iprintf ((o_fp, "%s(TEHandle **%p) {\n", field_name.c_str(), te));
  indent += 2;
  
  dump_field (dump_rect, &TE_DEST_RECT (te), "destRect");
  dump_field (dump_rect, &TE_VIEW_RECT (te), "viewRect");
  dump_field (dump_rect, &TE_SEL_RECT (te), "selRect");
  
  iprintf ((o_fp, "lineHeight %d\n", TE_LINE_HEIGHT (te)));
  iprintf ((o_fp, "fontAscent %d\n", TE_FONT_ASCENT (te)));
  
  iprintf ((o_fp, "selStart %d\n", TE_SEL_START (te)));
  iprintf ((o_fp, "selEnd %d\n", TE_SEL_END (te)));
  iprintf ((o_fp, "caretState %d\n", TE_CARET_STATE (te)));
  
  iprintf ((o_fp, "just %d\n", TE_JUST (te)));
  iprintf ((o_fp, "teLength %d\n", TE_LENGTH (te)));
  
  iprintf ((o_fp, "txFont %d\n", TE_TX_FONT (te)));
  iprintf ((o_fp, "txFace %d\n", TE_TX_FACE (te)));
  iprintf ((o_fp, "txMode %d\n", TE_TX_MODE (te)));
  iprintf ((o_fp, "txSize %d\n", TE_TX_SIZE (te)));
  
  iprintf ((o_fp, "nLines %d\n", TE_N_LINES (te)));

  {
    char buf[40000];
    int16 length;
    
    length = TE_LENGTH (te);
    memcpy (buf, STARH (TE_HTEXT (te)), length);
    buf[length] = '\0';

    fprintf (o_fp, "`%s'\n", buf);
  }

  {
    int16 n_lines;
    GUEST<int16> *line_starts;
    
    n_lines = TE_N_LINES (te);
    line_starts = TE_LINE_STARTS (te);
    
    for (i = 0; i <= n_lines; i ++)
      iprintf ((o_fp, "lineStart[%d]: %d\n", i, CW (line_starts[i])));
  }

  {
    STHandle style_table;
    StyleRun *runs;
    int16 n_lines, n_runs, n_styles;
    TEStyleHandle te_style;
    LHHandle lh_table;
    LHElement *lh;
    
    n_lines = TE_N_LINES (te);
    te_style = TE_GET_STYLE (te);
    lh_table = TE_STYLE_LH_TABLE (te_style);
    n_runs = TE_STYLE_N_RUNS (te_style);
    lh = STARH (lh_table);

    n_styles = TE_STYLE_N_STYLES (te_style);
    
    iprintf ((o_fp, "(TEStyleHandle **%p) {\n", te_style));
    indent += 2;

    iprintf ((o_fp, "nRuns %d\n", TE_STYLE_N_RUNS (te_style)));
    iprintf ((o_fp, "nStyles %d\n", n_styles));
    
    for (i = 0; i <= n_lines; i ++)
      iprintf ((o_fp, "lhTab[%d]: lhHeight %d, lhAscent %d\n",
		i, LH_HEIGHT (&lh[i]), LH_ASCENT (&lh[i])));
    
    runs = TE_STYLE_RUNS (te_style);
    for (i = 0; i <= n_runs; i ++)
      iprintf ((o_fp, "runs[%d]: startChar %d, styleIndex %d\n", i,
		STYLE_RUN_START_CHAR (&runs[i]),
		STYLE_RUN_STYLE_INDEX (&runs[i])));

    style_table = TE_STYLE_STYLE_TABLE (te_style);
    for (i = 0; i < n_styles; i ++)
      {
	STElement *style;

	style = ST_ELT (style_table, i);
	iprintf ((o_fp, "style[%d] stCount %d, stHieght %d, stAscent %d\n",
		  i, ST_ELT_COUNT (style),
		  ST_ELT_HEIGHT (style), ST_ELT_ASCENT (style)));
      }
    
    indent -= 2;
    iprintf ((o_fp, "}\n"));
  }
  indent -= 2;
  iprintf ((o_fp, "}\n"));
}

void
dump_scrap (StScrpHandle scrap)
{
  
}

class MapSaveGuard
{
    GUEST<INTEGER> saveMap;
public:
    MapSaveGuard(GUEST<INTEGER> map)
        : saveMap(CurMap)
    {
        CurMap = map;
    }
    ~MapSaveGuard()
    {
        CurMap = saveMap;
    }
};

static INTEGER
CountResourcesRN (LONGINT type, INTEGER rn)
{
  MapSaveGuard(CW (rn));
  return Count1Resources (type);
}

static Handle
GetIndResourceRN (LONGINT type, INTEGER i, INTEGER rn)
{
  MapSaveGuard(CW (rn));
  return Get1IndResource (type, i);
}

static void
AddResourceRN (Handle h, LONGINT type, INTEGER id, Str255 name, INTEGER rn)
{
  MapSaveGuard(CW (rn));
  AddResource (h, type, id, name);
}

static OSErr
copy_resources (INTEGER new_rn, INTEGER old_rn, LONGINT type)
{
  INTEGER num_res;
  BOOLEAN save_res_load;
  INTEGER i;
	  
  save_res_load = ResLoad;
  ResLoad = true;
  num_res = CountResourcesRN (type, old_rn);
  for (i = 1; i <= num_res; ++i)
    {
      Handle h;
      GUEST<INTEGER> id;
      Str255 name;
      GUEST<LONGINT> ignored;

      h = GetIndResourceRN (type, i, old_rn);
      GetResInfo (h, &id, &ignored, name);
      DetachResource (h);
      AddResourceRN (h, type, CW (id), name, new_rn);
    }
  ResLoad = save_res_load;
  return noErr;
}

/* NOTE: dump_code_resources detachs the current (CODE '0'), so you will
         not be able to continue after calling this function */

/* EXAMPLE: dump_code_resources ("/:tmp:filename"); */

OSErr
dump_code_resources (const char *filename)
{
  Str255 pfilename;
  OSErr retval;

  str255_from_c_string (pfilename, filename);
  CreateResFile (pfilename);
  retval = ResError ();
  if (retval == noErr)
    {
      Handle h;
      INTEGER old_rn, new_rn;

      h = GetResource (TICK("CODE"), 0);
      retval = ResError ();
      if (retval == noErr)
	{
	  old_rn = HomeResFile (h);
	  DetachResource (h);
	  retval = ResError ();
	  if (retval == noErr)
	    {
	      new_rn = OpenRFPerm (pfilename, 0, fsRdWrPerm);
	      retval = ResError ();
	      if (retval == noErr)
		retval = copy_resources (new_rn, old_rn, TICK("CODE"));
	      if (new_rn)
		{
		  CloseResFile (new_rn);
		  if (retval == noErr)
		    retval = ResError ();
		}
	    }
	}
    }
  return retval;
}

#if defined (SYN68K)
/* This is just a linker hack to make sure we pull in the syn68k .o file
 * containing the m68kaddr function.
 */

#include <syn68k_public.h>

void (*m68kaddr_linker_hack)(const uint16 *) = m68kaddr;
#endif

PRIVATE const char *
just_name (unsigned char just)
{
  const char *names[] =
  {
    "none", "left", "center", "right", "full", "INVALID"
  };
  const char *retval;

  retval = just < NELEM (names) ? names[just] : names[NELEM(names-1)];
  return retval;
}

PRIVATE const char *
flop_name (unsigned char flop)
{
  const char *names[] =
  {
    "none", "horizontal", "vertical", "INVALID"
  };
  const char *retval;

  retval = flop < NELEM (names) ? names[flop] : names[NELEM(names-1)];
  return retval;
}

PUBLIC void
dump_textbegin (TTxtPicHdl h)
{
  iprintf ((o_fp, "just = %d(%s)\n", TEXTPIC_JUST (h),
	    just_name (TEXTPIC_JUST (h))));
  iprintf ((o_fp, "flop = %d(%s)\n", TEXTPIC_FLOP (h),
	    flop_name (TEXTPIC_FLOP (h))));
  iprintf ((o_fp, "angle = %d\n", TEXTPIC_ANGLE (h)));
  iprintf ((o_fp, "line = %d\n", TEXTPIC_LINE (h)));
  iprintf ((o_fp, "comment = %d\n", TEXTPIC_COMMENT (h)));
  if (GetHandleSize ((Handle) h) >= 10)
    iprintf ((o_fp, "angle_fixed = 0x%08x\n", TEXTPIC_ANGLE_FIXED (h)));
}

PUBLIC void
dump_textcenter (TCenterRecHdl h)
{
  iprintf ((o_fp, "y = 0x%08x\n", TEXTCENTER_Y (h)));
  iprintf ((o_fp, "x = 0x%08x\n", TEXTCENTER_X (h)));
}

PUBLIC void
dump_zone_stats (void)
{
  iprintf ((o_fp, "applzone free = %d\n", ZONE_ZCB_FREE (MR (ApplZone))));
  iprintf ((o_fp, " syszone free = %d\n", ZONE_ZCB_FREE (MR (SysZone))));
}

#endif  /* !NDEBUG */
