/* Copyright 1994, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */
#ifndef _windriver_h
#define _windriver_h

/* Header file for the private windows support functions */

/* Hack hack hack -- resolve conflicts between our headers and windows.h
   Do not use the following windows functions:
   AppendMenu InsertMenu DrawText FindWindow ReplaceText Polygon
*/
#undef AppendMenu
#undef InsertMenu
#undef DrawText
#undef FindWindow
#undef ReplaceText
/* Do not use the windows BOOLEAN or CHAR types */
#define BOOLEAN MAC_BOOLEAN
#define CHAR    MAC_CHAR
#define Polygon MAC_Polygon

#include "rsys/common.h"

#include "rsys/cquick.h"
#include "rsys/refresh.h"
#include "rsys/vdriver.h"
#include "rsys/flags.h"

/* Global Variables */
extern const char *Win_AppName;
extern HINSTANCE   Win_Instance;
extern HWND        Win_Window;
extern unsigned char *vdriver_shadow_fbuf;

/* Functions required for proper handling of windows event messages */
extern int  Win_RubberBand(int *width, int *height);
extern void Win_Focus(int on);
extern void Win_NewPal(void);
extern void Win_PAINT(void);

#endif /* _windriver_h */
