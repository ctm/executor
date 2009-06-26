/*
 * Copyright 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: font.h 63 2004-12-24 18:19:43Z ctm $
 */

typedef struct PACKED {
  INTEGER size;
  INTEGER style;
  INTEGER fontresid;
} fatabentry;

typedef struct PACKED {
  unsigned short style;
  INTEGER table[1];	/* actually more */
} widentry_t;

typedef WidthTable *WPtr;
MAKE_HIDDEN(WPtr);
typedef HIDDEN_WPtr *WHandle;

typedef FamRec *FPtr;
MAKE_HIDDEN(FPtr);
typedef HIDDEN_FPtr *FHandle;

typedef WHandle *WHandlePtr;
MAKE_HIDDEN(WHandlePtr);
typedef HIDDEN_WHandlePtr *WHandleHandle;

#define FONTRESID(font, size)   (((font) << 7) | (size))

#define WIDTHPTR	((WPtr)    MR(WidthPtr))
#define WIDTHTABHANDLE	((WHandle) MR(WidthTabHandle))
#define WIDTHLISTHAND	((WHandleHandle) MR(WidthListHand))

extern Fixed font_width_expand (Fixed width, Fixed extra,
				Fixed hOutputInverse);

extern void ROMlib_shutdown_font_manager (void);
