#if !defined(_RSYS_KEYBOARDS_H_)
#define _RSYS_KEYBOARDS_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: keyboards.h 63 2004-12-24 18:19:43Z ctm $
 */

/*
 * Don't change the order of these without reworking the
 * next_virt_to_mac_virt table in NEXT.c
 */

typedef enum {
    adb_keyboard,
    default_keyboard,
    pc_keyboard
} keyboard_enum_t;

#define NVIRTMAPS	(pc_keyboard)
#define NKEYSTOMAP	(0x68)

extern keyboard_enum_t ROMlib_keyboard_type;
extern keyboard_enum_t ROMlib_get_keyboard_type( void );

extern unsigned char next_virt_to_mac_virt[NVIRTMAPS][NKEYSTOMAP];

/* We need these to map various keycodes to NeXT keycodes so we can share
 * some ROMlib lookup tables to map PC keycodes->Mac keycodes.
 */
#define NSFIP_LEFT_ARROW_KEYCODE  0x66
#define NSFIP_RIGHT_ARROW_KEYCODE 0x67
#define NSFIP_UP_ARROW_KEYCODE    0x64
#define NSFIP_DOWN_ARROW_KEYCODE  0x65

#if defined (SDL) && defined (CYGWIN32)
extern void ROMlib_set_use_scancodes (boolean_t val);
#endif

#endif
