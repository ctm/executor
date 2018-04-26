#if !defined(_TOOLEVENT_H_)
#define _TOOLEVENT_H_

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
struct keymap
{
    GUEST_STRUCT;
    GUEST<INTEGER> version;
    GUEST<INTEGER[256]> tableno;
    GUEST<INTEGER> ntables;
    GUEST<Byte[1][128]> table; /* who knows how many */
};

extern void dofloppymount(void);
extern BOOLEAN ROMlib_beepedonce;
extern void ROMlib_send_quit(void);
}

extern "C" int ROMlib_right_button_modifier;

#endif /* !_TOOLEVENT_H_ */
