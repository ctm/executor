/*
 * Copyright 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: font.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
struct fatabentry { GUEST_STRUCT;
    GUEST< INTEGER> size;
    GUEST< INTEGER> style;
    GUEST< INTEGER> fontresid;
};

struct widentry_t { GUEST_STRUCT;
    GUEST< unsigned short> style;
    GUEST< INTEGER[1]> table;    /* actually more */
};

typedef WidthTable *WPtr;

typedef GUEST<WPtr> *WHandle;

typedef FamRec *FPtr;

typedef GUEST<FPtr> *FHandle;

typedef GUEST<WHandle> *WHandlePtr;

typedef GUEST<WHandlePtr> *WHandleHandle;

#define FONTRESID(font, size)   (((font) << 7) | (size))

#define WIDTHPTR	((WPtr)    MR(WidthPtr))
#define WIDTHTABHANDLE	((WHandle) MR(WidthTabHandle))
#define WIDTHLISTHAND	((WHandleHandle) MR(WidthListHand))

extern Fixed font_width_expand (Fixed width, Fixed extra,
				Fixed hOutputInverse);

extern void ROMlib_shutdown_font_manager (void);
}
