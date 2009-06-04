#if !defined(_RSYS_KEYBOARD_H_)
#define _RSYS_KEYBOARD_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: keyboard.h 63 2004-12-24 18:19:43Z ctm $
 */

typedef unsigned char raw_key_t;
typedef unsigned char virt_key_t;
typedef unsigned char modifier_table_number_t;

typedef struct
{
  unsigned char to_look_for	LPACKED;
  unsigned char replacement	LPACKED;
} completer_pair_t;

typedef struct
{
  INTEGER n_recs			PACKED;
  completer_pair_t completer_recs[0]	LPACKED; /* VARIABLE LENGTH */
} completer_t;

#define COMPLETER_N_RECS_X(p)		((p)->n_recs)
#define COMPLETER_COMPLETER_RECS_X(p)	((p)->completer_recs)

#define COMPLETER_N_RECS(p)		(CW (COMPLETER_N_RECS_X (p)))

typedef struct
{
  modifier_table_number_t table_number	LPACKED;
  virt_key_t virt_key			LPACKED;
  completer_t completer			LPACKED; /* VARIABLE LENGTH */
  unsigned char filler			LPACKED;
  unsigned char no_match		LPACKED;
} dead_key_rec_t;

#define DEAD_KEY_TABLE_NUMBER_X(p)	((p)->table_number)
#define DEAD_KEY_VIRT_KEY_X(p)		((p)->virt_key)
#define DEAD_KEY_COMPLETER_X(p)		((p)->completer)

#define DEAD_KEY_NO_MATCH_X(p)		(*((unsigned char *) \
					   &DEAD_KEY_COMPLETER_X (p) \
					   + sizeof(DEAD_KEY_COMPLETER_X (p)) \
					   + COMPLETER_N_RECS \
					   (&DEAD_KEY_COMPLETER_X (p)) \
					   * sizeof(completer_pair_t) + 1))

#define DEAD_KEY_TABLE_NUMBER(p)	(CB (DEAD_KEY_TABLE_NUMBER_X (p)))
#define DEAD_KEY_VIRT_KEY(p)		(CB (DEAD_KEY_VIRT_KEY_X (p)))
#define DEAD_KEY_NO_MATCH(p)		(CB (DEAD_KEY_NO_MATCH_X (p)))

typedef struct
{
  INTEGER version				PACKED;
  modifier_table_number_t modifier_table[256]	LPACKED;
  INTEGER n_tables				PACKED;
  unsigned char table[0][128]			LPACKED; /* VARIABLE LENGTH */
  INTEGER n_dead_key_recs			PACKED;
  dead_key_rec_t dead_key_recs[0]		LPACKED; /* VARIABLE LENGTH */
} kchr_str, *kchr_ptr_t;

typedef struct { kchr_ptr_t p PACKED_P; } HIDDEN_kchr_ptr, *kchr_hand;

#define KCHR_VERSION_X(p)		((p)->version)
#define KCHR_MODIFIER_TABLE_X(p)	((p)->modifier_table)
#define	KCHR_N_TABLES_X(p)		((p)->n_tables)
#define KCHR_TABLE_X(p)			((p)->table)

#define KCHR_N_DEAD_KEY_RECS_X(p)	(* (INTEGER *) (KCHR_TABLE_X (p) \
							+ KCHR_N_TABLES (p)))

#define KCHR_DEAD_KEY_RECS_X(p)		((dead_key_rec_t *) \
					 (&KCHR_N_DEAD_KEY_RECS_X (p) + 1))

#define KCHR_VERSION(p)			(CW (KCHR_VERSION_X (p)))
#define KCHR_N_TABLES(p)		(CW (KCHR_N_TABLES_X (p)))
#define KCHR_N_DEAD_KEY_RECS(p)		(CW (KCHR_N_DEAD_KEY_RECS_X (p)))

/* MKV prefix denotes a TRUE mac virtual key code (as opposed to the
   ones I made up) */
/* i pulled these defines from osevent.c; they should placed into a
   header file somewhere */

#define MKV_ESCAPE	0x35
#define MKV_F1	0x7a
#define MKV_F2	0x78
#define MKV_F3	0x63
#define MKV_F4	0x76
#define MKV_F5	0x60
#define MKV_F6	0x61
#define MKV_F7	0x62
#define MKV_F8	0x64
#define MKV_F9	0x65
#define MKV_F10	0x6d
#define MKV_F11	0x67
#define MKV_F12	0x6f
#define MKV_F13	0x69
#define MKV_F14	0x6b
#define MKV_F15	0x71

#define MKV_BACKTICK	0x32
#define MKV_1	0x12
#define MKV_2	0x13
#define MKV_3	0x14
#define MKV_4	0x15
#define MKV_5	0x17
#define MKV_6	0x16
#define MKV_7	0x1a
#define MKV_8	0x1c
#define MKV_9	0x19
#define MKV_0	0x1d
#define MKV_MINUS	0x1b
#define MKV_EQUAL	0x18
#define MKV_BACKSPACE	0x33

#define MKV_TAB	0x30
#define MKV_q	0x0c
#define MKV_w	0x0d
#define MKV_e	0x0e
#define MKV_r	0x0f
#define MKV_t	0x11
#define MKV_y	0x10
#define MKV_u	0x20
#define MKV_i	0x22
#define MKV_o	0x1f
#define MKV_p	0x23
#define MKV_LEFTBRACKET	0x21
#define MKV_RIGHTBRACKET	0x1e
#define MKV_BACKSLASH	0x2a

#define MKV_CAPS	0x39
#define MKV_a	0x00
#define MKV_s	0x01
#define MKV_d	0x02
#define MKV_f	0x03
#define MKV_g	0x05
#define MKV_h	0x04
#define MKV_j	0x26
#define MKV_k	0x28
#define MKV_l	0x25
#define MKV_SEMI	0x29
#define MKV_TICK	0x27
#define MKV_RETURN	0x24

#define MKV_LEFTSHIFT	0x38
#define MKV_z	0x06
#define MKV_x	0x07
#define MKV_c	0x08
#define MKV_v	0x09
#define MKV_b	0x0b
#define MKV_n	0x2d
#define MKV_m	0x2e
#define MKV_COMMA	0x2b
#define MKV_PERIOD	0x2f
#define MKV_SLASH	0x2c
#define MKV_RIGHTSHIFT	0x3c

#define MKV_LEFTCNTL	0x3b
#define MKV_LEFTOPTION	0x3a
#define MKV_CLOVER	0x37
#define MKV_SPACE	0x31
#define MKV_RIGHTOPTION	0x3d
#define MKV_RIGHTCNTL	0x3e

#define MKV_HELP	0x72
#define MKV_HOME	0x73
#define MKV_PAGEUP	0x74
#define MKV_DELFORWARD	0x75
#define MKV_END		0x77
#define MKV_PAGEDOWN	0x79

#define MKV_UPARROW	0x7e
#define MKV_LEFTARROW	0x7b
#define MKV_RIGHTARROW	0x7c
#define MKV_DOWNARROW	0x7d

#define MKV_NUMCLEAR	0x47
#define MKV_NUMEQUAL	0x51
#define MKV_NUMDIVIDE	0x4b
#define MKV_NUMMULTIPLY	0x43
#define MKV_NUM7	0x59
#define MKV_NUM8	0x5b
#define MKV_NUM9	0x5c
#define MKV_NUMMINUS	0x4e
#define MKV_NUM4	0x56
#define MKV_NUM5	0x57
#define MKV_NUM6	0x58
#define MKV_NUMPLUS	0x45
#define MKV_NUM1	0x53
#define MKV_NUM2	0x54
#define MKV_NUM3	0x55
#define MKV_NUMENTER	0x4c
#define MKV_NUM0	0x52
#define MKV_NUMPOINT	0x41

#define MKV_PRINT_SCREEN	0x69
#define MKV_SCROLL_LOCK	0x6b
#define MKV_PAUSE	0x71

#define MKV_RESET 0x7f

extern unsigned char ibm_virt_to_mac_virt[];

#endif /* !defined(_RSYS_KEYBOARD_H_) */

