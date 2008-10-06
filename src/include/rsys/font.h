/*
 * Copyright 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: font.h 63 2004-12-24 18:19:43Z ctm $
 */

typedef struct {
    INTEGER size	PACKED;
    INTEGER style	PACKED;
    INTEGER fontresid	PACKED;
} fatabentry;

typedef WidthTable *WPtr;
typedef struct { WPtr p PACKED_P; } HIDDEN_WPtr;
typedef HIDDEN_WPtr *WHandle;

typedef FamRec *FPtr;
typedef struct { FPtr p PACKED_P; } HIDDEN_FPtr;
typedef HIDDEN_FPtr *FHandle;

typedef WHandle *WHandlePtr;
typedef struct { WHandlePtr p PACKED_P; } HIDDEN_WHandlePtr;
typedef HIDDEN_WHandlePtr *WHandleHandle;

#define FONTRESID(font, size)   (((font) << 7) | (size))

#define WIDTHPTR	((WPtr)    MR(WidthPtr))
#define WIDTHTABHANDLE	((WHandle) MR(WidthTabHandle))
#define WIDTHLISTHAND	((WHandleHandle) MR(WidthListHand))

extern Fixed font_width_expand (Fixed width, Fixed extra,
				Fixed hOutputInverse);

extern void ROMlib_shutdown_font_manager (void);
