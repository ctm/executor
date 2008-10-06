/* Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_win_print[] = "$Id: ptest.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

static HWND
main_window (void)
{
  HWND retval;

  retval = GetTopWindow (NULL);
  return retval;
}

/* ugly globals needed because the callback doesn't pass in a user
   supplied argument */

#define PRIVATE static

PRIVATE HDC global_hdc;
PRIVATE RECT global_src;
PRIVATE RECT global_dest;

#if !defined (NELEM)
#define NELEM(x) ((sizeof (x)) / sizeof ((x)[0]))
#endif

#define GSDLLAPI CALLBACK _export

enum
{
  GSDLL_STDIN=1,
  GSDLL_STDOUT,
  GSDLL_DEVICE,
  GSDLL_SYNC,
  GSDLL_PAGE,
  GSDLL_SIZE,
  GSDLL_POLL,
};

typedef int (*GSDLL_CALLBACK)(int, char *, unsigned long);


PRIVATE int GSDLLAPI (*gsdll_revision)(char **product, char **copyright, 
				       long *gs_revision,
				       long *gs_revisiondate);

PRIVATE int GSDLLAPI (*gsdll_init)(GSDLL_CALLBACK callback, HWND hwnd, 
				   int argc, const char *argv[]);

PRIVATE int GSDLLAPI (*gsdll_execute_begin)(void);

PRIVATE int GSDLLAPI (*gsdll_execute_cont)(const char *str, int len);

PRIVATE int GSDLLAPI (*gsdll_execute_end)(void);

PRIVATE int GSDLLAPI (*gsdll_exit)(void);

PRIVATE int GSDLLAPI (*gsdll_lock_device)(unsigned char *device, int flag);

PRIVATE HGLOBAL GSDLLAPI (*gsdll_copy_dib)(unsigned char *device);

PRIVATE HPALETTE GSDLLAPI (*gsdll_copy_palette)(unsigned char *device);

PRIVATE void GSDLLAPI (*gsdll_draw)(unsigned char *device, HDC hdc, 
				    LPRECT dest, LPRECT src);

PRIVATE int GSDLLAPI (*gsdll_get_bitmap_row)(unsigned char *device,
					     LPBITMAPINFOHEADER pbmih,
					     RGBQUAD *prgbquad, LPBYTE *ppbyte,
					     unsigned int row);

#define GETPROCADDRESS(lib, func)			\
do							\
{							\
  if (lib)						\
    {							\
      func = (void *) GetProcAddress (lib, #func);	\
      if (!func)					\
	{						\
	  FreeLibrary (lib);				\
	  lib = NULL;					\
	}						\
    }							\
}							\
while (0)

PRIVATE void
loadgs (void)
{
  HINSTANCE lib;

  lib = LoadLibrary ("gsdll32");
  GETPROCADDRESS (lib, gsdll_revision);
  GETPROCADDRESS (lib, gsdll_init);
  GETPROCADDRESS (lib, gsdll_execute_begin);
  GETPROCADDRESS (lib, gsdll_execute_cont);
  GETPROCADDRESS (lib, gsdll_execute_end);
  GETPROCADDRESS (lib, gsdll_exit);
  GETPROCADDRESS (lib, gsdll_lock_device);
  GETPROCADDRESS (lib, gsdll_copy_dib);
  GETPROCADDRESS (lib, gsdll_copy_palette);
  GETPROCADDRESS (lib, gsdll_draw);
  GETPROCADDRESS (lib, gsdll_get_bitmap_row);
}

PRIVATE int
gsdll_callback(int message, char *str, unsigned long count)
{
  if (message != GSDLL_POLL)
    printf ("message = %d\n", message);
  switch (message)
    {
    case GSDLL_PAGE:
      StartPage (global_hdc);
      gsdll_lock_device (str, 1);
#if 0
      /* NOTE: this works on FAX modem, but not on laserjet */
      gsdll_draw (str, global_hdc, &global_dest, &global_src);
#else
      {
	int n;
	struct
	{
	  BITMAPINFOHEADER h PACKED;
	  RGBQUAD colors[2] PACKED;
	}
	bmi;
	LPBYTE bytep;

	bmi.colors[0].rgbRed   = 0;
	bmi.colors[0].rgbGreen = 0;
	bmi.colors[0].rgbBlue  = 0;
	bmi.colors[1].rgbRed   = ~0;
	bmi.colors[1].rgbGreen = ~0;
	bmi.colors[1].rgbBlue  = ~0;
	gsdll_get_bitmap_row (str, &bmi.h, NULL, &bytep, 0);

#if !defined (DIB_PAL_INDICES)
#define DIB_PAL_INDICES 2
#endif

#if 1
	n = SetDIBitsToDevice (global_hdc, global_dest.left, global_dest.top,
			       global_src.right - global_src.left,
			       global_src.bottom - global_src.top,
			       global_src.left, global_src.top,
			       0,
			       global_src.bottom - global_src.top,
			       bytep, 
			       (LPBITMAPINFO) &bmi,
			       DIB_RGB_COLORS);
#else
	n = SetDIBitsToDevice (global_hdc, 0, 0,
			       global_src.right - global_src.left,
			       global_src.bottom - global_src.top,
			       0, 0,
			       0,
			       global_src.bottom - global_src.top,
			       bytep, 
			       (LPBITMAPINFO) &bmi,
			       DIB_RGB_COLORS);
#endif
      }
#endif
      gsdll_lock_device (str, 0);
      EndPage (global_hdc);
      break;
    case GSDLL_STDOUT:
      if (str != NULL)
	fwrite (str, 1, count, stdout);
      break;
    case GSDLL_DEVICE:
      if (count == 0)
	{
	  EndDoc (global_hdc);
	}
      break;
    default:
      /* ignore */
      break;
    }
  fflush (stdout);
  return 0;
}


int PASCAL
WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
  PRINTDLG pd;
  int retval;

  freopen ("stdout.txt", "w", stdout);
  setbuf (stdout, 0);
  freopen ("stderr.txt", "w", stderr);
  setbuf (stderr, 0);

  loadgs ();

  memset (&pd, 0, sizeof pd);
  pd.nCopies = 1;
  pd.hwndOwner = main_window ();
  pd.lStructSize = sizeof pd;
  pd.Flags = (PD_HIDEPRINTTOFILE    |
	      PD_NOPAGENUMS         |
	      PD_NOSELECTION        |
	      PD_RETURNDC           |
	      PD_USEDEVMODECOPIES);
  retval = PrintDlg (&pd);
  if (!retval)
    {
      fprintf (stderr, "PrintDlg failed\n");
    }
  else
    {
      static const char *argv[] =
      {
	"executor",
	"-IC:\\GS;C:\\GS\\FONTS",
	"-q",
	"-dNOPAUSE",
	"-sDEVICE=mswindll",
	"-dBitsPerPixel=1",
	NULL, /* -g%dx%d (papersize) */
	NULL, /* -r%dx%d (resolution)*/
	"calibrate.ps", /* %s (filename) */
	"-c quit",
	NULL,
      };
      char *papersize;
      char *resolution;

      global_hdc = pd.hDC;
      global_src.top = 0;
      global_src.left = 0;
      global_src.bottom = GetDeviceCaps (pd.hDC, VERTRES);
      global_src.right = GetDeviceCaps (pd.hDC, HORZRES);
      global_dest = global_src;

      printf ("top = %ld, left = %ld, bottom = %ld, right = %ld\n",
	      global_src.top, global_src.left,
	      global_src.bottom, global_src.right);

      {
	DOCINFO di;
	int job_identifier;

	memset (&di, 0, sizeof di);
	di.cbSize = sizeof di;
	di.lpszDocName = "Test Document";
	job_identifier = StartDoc (global_hdc, &di);
	if (job_identifier <= 0)
	  {
	    retval = 1;
	    MessageBox (NULL, "Couldn't StartDoc", "Print Failure", MB_OK);
	  }
      }

      papersize = alloca (128);
      sprintf (papersize, "-g%ldx%ld", global_src.right, global_src.bottom);

      resolution = alloca (128);
      sprintf (resolution, "-r%dx%d",
	       GetDeviceCaps (pd.hDC, LOGPIXELSX),
	       GetDeviceCaps (pd.hDC, LOGPIXELSY));

      argv[6] = papersize;
      argv[7] = resolution;


      {
	int i;

	for (i = 0; i <= 9; ++i)
	  printf ("argv[%d] = '%s'\n", i, argv[i]);
      }
      
      if (gsdll_init (gsdll_callback, NULL, NELEM (argv)-1, argv) != 0)
	{
	  fprintf (stderr, "gsdll_init failed\n");
	}
      else
	{
	  int code;
	  
	  gsdll_execute_begin ();
	  code = gsdll_execute_cont ("", 0);
	  printf ("code = %d\n", code);
	  if (code > -100)
	    gsdll_execute_end ();
	  gsdll_exit ();
	  retval = TRUE;
	  EndDoc (global_hdc);
	}
    }
  return 0;
}
