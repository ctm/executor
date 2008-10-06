/* Copyright 1994, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_windib[] = "$Id: windib.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* DIB implementation of the framebuffer portion of win32 vdriver */

/* No includes -- designed to be included in windriver.c */

/* Local Definitions */
enum { NUM_COLORS = 256 };              /* Fix ourselves at an 8-bit palette */
enum { PAL_SIZE   = (NUM_COLORS*sizeof(RGBQUAD)) }; /* Size of 8-bit palette */

/* Local Variables */
static HANDLE      private_mem = NULL;
static BITMAPINFO *binfo = NULL;
static RGBQUAD    *saved_pal = NULL;
static HBITMAP     screen_bmp = NULL;
static char        sys_palette_p = 0;
static HPALETTE    our_palette;
static HPALETTE    sys_palette;


/* DIB vdriver implementation */

void
vdriver_opt_register (void)
{
}

/* The DIB driver can create windows of virtually any size */
vdriver_dib_mode_t vdriver_dib_modes =
{
  /* contiguous_range_p */ TRUE,
  /* num_sizes */ 2,
  {
    /* min */    { 512, 342 },  /* Where do these numbers come from? */
    /* max */ { VDRIVER_DEFAULT_SCREEN_WIDTH, VDRIVER_DEFAULT_SCREEN_HEIGHT },
  },
};


boolean_t
vdriver_init (int _max_width, int _max_height, int _max_bpp,
	      boolean_t fixed_p, int *argc, char *argv[])
{
  HDC hdc;

  /* Verify the video mode parameters */
  if ( ! _max_width )
    {
      _max_width = MIN(GetSystemMetrics(SM_CXFULLSCREEN),
		       MAX(VDRIVER_DEFAULT_SCREEN_WIDTH, flag_width));
    }
  if ( ! _max_height )
    {
      _max_height = MIN(GetSystemMetrics(SM_CYFULLSCREEN),
		      MAX(VDRIVER_DEFAULT_SCREEN_HEIGHT, flag_height));
    }
  if ( _max_bpp && (_max_bpp != 8) )
    return(FALSE);
  vdriver_max_bpp = 8;
  vdriver_log2_max_bpp = ROMlib_log2[vdriver_max_bpp];
  vdriver_dib_modes.size[1].width = _max_width;
  vdriver_dib_modes.size[1].height = _max_height;

  /* Create a chunk of memory that we can use for CreateDIBSection() */
  private_mem = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE,
				  0, _max_width*_max_height*vdriver_max_bpp/8,
				  NULL);
  if ( private_mem == NULL )
    {
      return(FALSE);
    }
  vdriver_fbuf = MapViewOfFile(private_mem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if ( vdriver_fbuf == NULL )
    {
      /* Fatal error -- couldn't create the memory chunk */
      return(FALSE);
    }

  /* Create a main window */
  {
    DWORD style;
    RECT bounds;

    /* Create a window with maximize minimize capability (WS_SYSMENU needed) */
#if defined(WIN32_FULLSCREEN)
    style = 
      (WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX);
#else
    style = 
      (WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX);
#endif
    bounds.top = 0;
    bounds.left = 0;
    bounds.right = _max_width;
    bounds.bottom = _max_height;
    AdjustWindowRect(&bounds, style, FALSE);
    Win_Window = CreateWindow(Win_AppName, Win_AppName, style, 
			      CW_USEDEFAULT, CW_USEDEFAULT,
			      bounds.right-bounds.left,
			      bounds.bottom-bounds.top,
			      NULL, NULL, Win_Instance, NULL);
  }
  ShowWindow(Win_Window, SW_SHOWDEFAULT);

  /* Find out if we have a palettized display.
   * If sys_palette_p is set, we do active manipulation of display palette
   *
   * We assume that if BITSPIXEL is 8, the display is palettized and has
   * NUM_COLORS colors.  This may not be true, but the RASTERCAPS test 
   * seems to fail on Windows 95.
   */
  hdc = GetDC(Win_Window);
  sys_palette_p = (GetDeviceCaps(hdc, BITSPIXEL) == 8);
  if ( sys_palette_p )
    {
      LOGPALETTE   *palette;
      PALETTEENTRY  entries[NUM_COLORS];

      /* Create an identity palette (to be modified later) */
      palette = alloca(sizeof(LOGPALETTE)+NUM_COLORS*sizeof(PALETTEENTRY));
      palette->palVersion = 0x300;
      palette->palNumEntries = NUM_COLORS;
      sys_palette = CreatePalette(palette);
      our_palette = CreatePalette(palette);
      GetSystemPaletteEntries(hdc, 0, NUM_COLORS, entries);
      SetPaletteEntries(sys_palette, 0, NUM_COLORS, entries);
      SetPaletteEntries(our_palette, 0, NUM_COLORS, entries);
      Win_Focus(1);
    }
  ReleaseDC(hdc, Win_Window);
  return(TRUE);
}

boolean_t
vdriver_acceptable_mode_p (int width, int height, int bpp,
			   boolean_t grayscale_p, boolean_t exact_match_p)
{
  /* Verify the video mode parameters */
  if ( width && ((width < vdriver_dib_modes.size[0].width) ||
		 (width > vdriver_dib_modes.size[1].width)) )
    return(FALSE);
  if ( height && ((height < vdriver_dib_modes.size[0].height) ||
		  (height > vdriver_dib_modes.size[1].height)) )
    return(FALSE);
  if ( bpp && (bpp != 8) )
    return(FALSE);
  if ( grayscale_p != vdriver_grayscale_p )
    return(FALSE);
  return(TRUE);
}

boolean_t
vdriver_set_mode (int width, int height, int bpp, boolean_t grayscale_p)
{
  HDC hdc;
  uint8 *dummy_fbuf;

  if ( ! vdriver_acceptable_mode_p(width, height, bpp, grayscale_p, FALSE) )
    return(FALSE);

  /* Set up the bitmap info header with our parameters */
  if ( screen_bmp )
    DeleteObject(screen_bmp);
  if ( binfo )
    free(binfo);
  binfo = (BITMAPINFO *)malloc(sizeof(*binfo)+PAL_SIZE);
  if ( binfo == NULL )
    return(FALSE);
  binfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  if ( ! width )
    width = vdriver_dib_modes.size[1].width;
  binfo->bmiHeader.biWidth = width;
  /* Use a top-down bitmap for easy writes */
  if ( ! height )
    height = vdriver_dib_modes.size[1].height;
  binfo->bmiHeader.biHeight = -height;
  binfo->bmiHeader.biPlanes = 1;
  if ( ! bpp )
    bpp = 8;
  binfo->bmiHeader.biBitCount = bpp;
  binfo->bmiHeader.biCompression = BI_RGB;
  binfo->bmiHeader.biSizeImage = width*height*bpp/8;
  binfo->bmiHeader.biXPelsPerMeter = 0;
  binfo->bmiHeader.biYPelsPerMeter = 0;
  binfo->bmiHeader.biClrUsed = 0;
  binfo->bmiHeader.biClrImportant = 0;
  if ( ! saved_pal ) {
    saved_pal = (RGBQUAD *)malloc(PAL_SIZE);
    if ( saved_pal == NULL ) {
      free(binfo);
      binfo = NULL;
      return(FALSE);
    }
    memset(saved_pal, 0, PAL_SIZE);
  }
  memcpy(binfo->bmiColors, saved_pal, PAL_SIZE);

  /* Fill the vdriver globals */
  if ( vdriver_shadow_fbuf != NULL ) {
    free(vdriver_shadow_fbuf);
    vdriver_shadow_fbuf = NULL;
  }
  vdriver_width = binfo->bmiHeader.biWidth;
  vdriver_height = -binfo->bmiHeader.biHeight;
  vdriver_bpp = binfo->bmiHeader.biBitCount;
  vdriver_log2_bpp = ROMlib_log2[vdriver_bpp];
  vdriver_row_bytes = ((((vdriver_bpp*vdriver_width)/8)+3)&~3);

  /* Create a windows DIB for our display */
  hdc = GetDC(Win_Window);
  /* screen_bmp and dummy_fbuf are set to NULL if this fails */
  screen_bmp = CreateDIBSection(hdc, binfo, DIB_RGB_COLORS,
				(void **)(&dummy_fbuf), private_mem, 0);
  ReleaseDC(Win_Window, hdc);
  return(TRUE);
}


void
vdriver_set_colors (int first_color, int num_colors, const ColorSpec *colors)
{
  int i, c;
  PALETTEENTRY entries[NUM_COLORS];

  if ( ! saved_pal || ((first_color+num_colors) > NUM_COLORS) ) {
    /* Fatal error... */
    return;
  }

  /* Set the palette */
  for ( i=0; i<num_colors; ++i )
    {
      entries[i].peRed   = (CW (colors[i].rgb.red)   >> 8);
      entries[i].peGreen = (CW (colors[i].rgb.green) >> 8);
      entries[i].peBlue  = (CW (colors[i].rgb.blue)  >> 8);
      entries[i].peFlags = 0;
    }

  if ( sys_palette_p )
    SetPaletteEntries(our_palette, first_color, num_colors, entries);
      
  for ( i=0; i<num_colors; ++i )
    {
      c = first_color + i;
      saved_pal[c].rgbRed   = entries[i].peRed;
      saved_pal[c].rgbGreen = entries[i].peGreen;
      saved_pal[c].rgbBlue  = entries[i].peBlue;
      saved_pal[c].rgbReserved = 0;
    }

  /* Re-create the DIB with the new palette */
  vdriver_set_mode(vdriver_width, vdriver_height, vdriver_bpp,
                                             vdriver_grayscale_p);
  /* Redraw the screen with the new colors */
  vdriver_update_screen(0, 0, vdriver_height, vdriver_width, FALSE);
}

void
vdriver_get_colors (int first_color, int num_colors, ColorSpec *colors)
{
  gui_fatal ("`!vdriver_fixed_clut_p' and `vdriver_get_colors ()' called");
}

int
vdriver_update_screen_rects (int num_rects, const vdriver_rect_t *r,
                                                      boolean_t cursor_p)
{
  HDC hdc, mdc;
  int i;
  int top, left, width, height;

  hdc = GetDC(Win_Window);
  if ( sys_palette_p )
    {
      SelectPalette(hdc, our_palette, FALSE);
      RealizePalette(hdc);
    }
  mdc = CreateCompatibleDC(hdc);
  SelectObject(mdc, screen_bmp);
  for ( i=0; i<num_rects; ++i ) {
    top    = r[i].top;
    left   = r[i].left;
    width  = r[i].right - r[i].left;
    height = r[i].bottom - r[i].top;
    BitBlt(hdc, left, top, width, height, mdc, left, top, SRCCOPY);
  }
  DeleteDC(mdc);
  ReleaseDC(Win_Window, hdc);
  return(0);
}

int
vdriver_update_screen (int top, int left, int bottom, int right,
		       boolean_t cursor_p)
{
  vdriver_rect_t r;
  
  if (top < 0)
    top = 0;
  if (left < 0)
    left = 0;
  
  if (bottom > vdriver_height)
    bottom = vdriver_height;
  if (right > vdriver_width)
    right = vdriver_width;
  
  r.top    = top;
  r.left   = left;
  r.bottom = bottom;
  r.right  = right;
  
  return vdriver_update_screen_rects (1, &r, cursor_p);
}

void
vdriver_flush_display (void)
{
}

void
vdriver_shutdown (void)
{
  /* Free up the various data structures */
  if ( binfo ) {
    free(binfo);
    binfo = NULL;
  }
  if ( screen_bmp ) {
    DeleteObject(screen_bmp);
    screen_bmp = NULL;
  }
  if ( vdriver_fbuf )
    {
      UnmapViewOfFile(vdriver_fbuf);
      vdriver_fbuf = NULL;
    }
  if ( private_mem )
    {
      CloseHandle(private_mem);
      private_mem = NULL;
    }
  if ( vdriver_shadow_fbuf ) {
    free(vdriver_shadow_fbuf);
    vdriver_shadow_fbuf = NULL;
  }
  if ( sys_palette_p )
    {
      Win_Focus(0);
      DeleteObject(our_palette);
      DeleteObject(sys_palette);
    }
}

/* Called in response to WM_PALETTECHANGED messages
 * -- We just repaint (BltBlt) our window with the new colormap.
 */
void
Win_NewPal(void)
{
  InvalidateRect(Win_Window, NULL, 0);
}

/* Called in response to WM_ACTIVATE messages
 */
void
Win_Focus(int on)
{
  HDC hdc;

  /* Right now, the only thing we care about is palette manipulation */
  if ( ! sys_palette_p )
    return;

  hdc = GetDC(Win_Window);
  if ( on )
    {
      PALETTEENTRY entries[NUM_COLORS];

      /* Save the current system palette */
      GetSystemPaletteEntries(hdc, 0, NUM_COLORS, entries);
      SetPaletteEntries(sys_palette, 0, NUM_COLORS, entries);

      /* Set our palette as the logical palette */
#ifdef PRIVATE_CMAP
      /* This is more trouble than it's worth.  It fries system colors */
      SetSystemPaletteUse(hdc, SYSPAL_NOSTATIC);
#endif
      SelectPalette(hdc, our_palette, FALSE);
      RealizePalette(hdc);
      InvalidateRect(Win_Window, NULL, 0);
    }
  else
    {
      /* Set the system palette and reset colors */
      SelectPalette(hdc, sys_palette, FALSE);
      RealizePalette(hdc);
#ifdef PRIVATE_CMAP
      SetSystemPaletteUse(hdc, SYSPAL_STATIC);
#endif
    }
  ReleaseDC(hdc, Win_Window);
}

/* Called in response to WM_PAINT messages
   -- We need to update some portion of our screen.
 */
void Win_PAINT(void)
{
  PAINTSTRUCT ps;
  HDC hdc, mdc;

  hdc = BeginPaint(Win_Window, &ps);
  if ( sys_palette_p )
    {
      SelectPalette(hdc, our_palette, FALSE);
      RealizePalette(hdc);
    }
  mdc = CreateCompatibleDC(hdc);
  SelectObject(mdc, screen_bmp);
  BitBlt(hdc, 0, 0, vdriver_width, vdriver_height, mdc, 0, 0, SRCCOPY);
  DeleteDC(mdc);
  EndPaint(Win_Window, &ps);
}

