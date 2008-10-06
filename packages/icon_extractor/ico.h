#if !defined (__ico_h__)
#define __ico_h__

#define PACKED /* __attribute__((packed)) */

typedef short int WORD;
typedef char BYTE;
typedef long DWORD;
typedef long LONG;

typedef struct _IconEntry
{
  BYTE Width;
  BYTE Height;
  BYTE NumColors;
  BYTE Reserved;
  WORD NumPlanes;
  WORD BitsPerPixel;
  DWORD DataSize;
  DWORD DataOffset;
}
PACKED ICONENTRY;

typedef struct tagBITMAPINFOHEADER
{
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
}
BITMAPINFOHEADER;

typedef struct tagPALETTEENTRY
{
  BYTE peRed;
  BYTE peGreen;
  BYTE peBlue;
  BYTE peFlags;
}
PALETTEENTRY;

typedef struct
{
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD;

typedef struct _IconData
{
  BITMAPINFOHEADER Header;
  RGBQUAD Palette[256];
  BYTE XorMap[32][32];
  BYTE AndMap[32][32/8];
}
PACKED ICONDATA;


typedef struct _IconFile
{
  WORD Reserved;
  WORD ResourceType;
  WORD IconCount;
  ICONENTRY IconDir;
  ICONDATA IconData;
}
PACKED ICONFILE;

#if defined(__i386__)

#define CBC(x) x
#define CWC(x) x
#define CLC(x) x

#else

#define CBC(x) (x)

#define CWC(x) (((unsigned short) (x) >> 8) | \
		((unsigned short) (x) << 8))


#define CLC(n) (((((long) n) & 0x000000FF) << 24) |			\
		((((long) n) & 0x0000FF00) <<  8) |			\
		((((long) n) & 0x00FF0000) >>  8) |			\
		(((unsigned long) (n) & 0xFF000000) >> 24))
#endif

#endif
