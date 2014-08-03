#if !defined(_TOOLEVENT_H_)
#define _TOOLEVENT_H_

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: toolevent.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
typedef struct PACKED {
    INTEGER version;
    INTEGER tableno[256];
    INTEGER ntables;
    Byte table[1][128];	/* who knows how many */
} keymap;

extern void dofloppymount (void);
extern BOOLEAN ROMlib_beepedonce;
extern void ROMlib_send_quit (void);
}

extern "C" int ROMlib_right_button_modifier;

#endif /* !_TOOLEVENT_H_ */
