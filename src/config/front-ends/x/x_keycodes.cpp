/* Copyright 1999 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#warning Insert is not giving us a 0x72

#include "rsys/common.h"
#include "rsys/keyboard.h"

#if defined(X)

unsigned char x_keycode_to_mac_virt[] = {
    0xFF, /* unused 0 */
    0xFF, /* unused 1 */
    0xFF, /* unused 2 */
    0xFF, /* unused 3 */
    0xFF, /* unused 4 */
    0xFF, /* unused 5 */
    0xFF, /* unused 6 */
    0xFF, /* unused 7 */
    0xFF, /* unused 8 */
    MKV_ESCAPE, /* <ESC>  =   9; */
    MKV_1, /* <AE01> =  10; */
    MKV_2, /* <AE02> =  11; */
    MKV_3, /* <AE03> =  12; */
    MKV_4, /* <AE04> =  13; */
    MKV_5, /* <AE05> =  14; */
    MKV_6, /* <AE06> =  15; */
    MKV_7, /* <AE07> =  16; */
    MKV_8, /* <AE08> =  17; */
    MKV_9, /* <AE09> =  18; */
    MKV_0, /* <AE10> =  19; */
    MKV_MINUS, /* <AE11> =  20; */
    MKV_EQUAL, /* <AE12> =  21; */
    MKV_BACKSPACE, /* <BKSP> =  22; */
    MKV_TAB, /* <TAB>  =  23; */
    MKV_q, /* <AD01> =  24; */
    MKV_w, /* <AD02> =  25; */
    MKV_e, /* <AD03> =  26; */
    MKV_r, /* <AD04> =  27; */
    MKV_t, /* <AD05> =  28; */
    MKV_y, /* <AD06> =  29; */
    MKV_u, /* <AD07> =  30; */
    MKV_i, /* <AD08> =  31; */
    MKV_o, /* <AD09> =  32; */
    MKV_p, /* <AD10> =  33; */
    MKV_LEFTBRACKET, /* <AD11> =  34; */
    MKV_RIGHTBRACKET, /* <AD12> =  35; */
    MKV_RETURN, /* <RTRN> =  36; */
    MKV_LEFTCNTL, /* <LCTL> =  37; */
    MKV_a, /* <AC01> =  38; */
    MKV_s, /* <AC02> =  39; */
    MKV_d, /* <AC03> =  40; */
    MKV_f, /* <AC04> =  41; */
    MKV_g, /* <AC05> =  42; */
    MKV_h, /* <AC06> =  43; */
    MKV_j, /* <AC07> =  44; */
    MKV_k, /* <AC08> =  45; */
    MKV_l, /* <AC09> =  46; */
    MKV_SEMI, /* <AC10> =  47; */
    MKV_TICK, /* <AC11> =  48; */
    MKV_BACKTICK, /* <TLDE> =  49; */
    MKV_LEFTSHIFT, /* <LFSH> =  50; */
    MKV_BACKSLASH, /* <BKSL> =  51; */
    MKV_z, /* <AB01> =  52; */
    MKV_x, /* <AB02> =  53; */
    MKV_c, /* <AB03> =  54; */
    MKV_v, /* <AB04> =  55; */
    MKV_b, /* <AB05> =  56; */
    MKV_n, /* <AB06> =  57; */
    MKV_m, /* <AB07> =  58; */
    MKV_COMMA, /* <AB08> =  59; */
    MKV_PERIOD, /* <AB09> =  60; */
    MKV_SLASH, /* <AB10> =  61; */
    MKV_RIGHTSHIFT, /* <RTSH> =  62; */
    MKV_NUMDIVIDE, /* <KPMU> =  63; */
    MKV_CLOVER, /* <LALT> =  64; */
    MKV_SPACE, /* <SPCE> =  65; */
    MKV_CAPS, /* <CAPS> =  66; */
    MKV_F1, /* <FK01> =  67; */
    MKV_F2, /* <FK02> =  68; */
    MKV_F3, /* <FK03> =  69; */
    MKV_F4, /* <FK04> =  70; */
    MKV_F5, /* <FK05> =  71; */
    MKV_F6, /* <FK06> =  72; */
    MKV_F7, /* <FK07> =  73; */
    MKV_F8, /* <FK08> =  74; */
    MKV_F9, /* <FK09> =  75; */
    MKV_F10, /* <FK10> =  76; */
    MKV_NUMCLEAR, /* <NMLK> =  77; */
    MKV_F14, /* <SCLK> =  78; */
    MKV_NUM7, /* <KP7>  =  79; */
    MKV_NUM8, /* <KP8>  =  80; */
    MKV_NUM9, /* <KP9>  =  81; */
    MKV_NUMMULTIPLY, /* <KPSU> =  82; */
    MKV_NUM4, /* <KP4>  =  83; */
    MKV_NUM5, /* <KP5>  =  84; */
    MKV_NUM6, /* <KP6>  =  85; */
    MKV_NUMPLUS, /* <KPAD> =  86; */
    MKV_NUM1, /* <KP1>  =  87; */
    MKV_NUM2, /* <KP2>  =  88; */
    MKV_NUM3, /* <KP3>  =  89; */
    MKV_NUM0, /* <KP0>  =  90; */
    MKV_NUMPOINT, /* <KPDL> =  91; */
    0xFF, /* unused 92 */
    0xFF, /* unused 93 */
    MKV_NUMMINUS, /* <LSGT> =  94; */
    MKV_F11, /* <FK11> =  95; */
    MKV_F12, /* <FK12> =  96; */
    MKV_HOME, /* unused 97 */
    MKV_UPARROW, /* unused 98 */
    MKV_PAGEUP, /* unused 99 */
    MKV_LEFTARROW, /* unused 100 */
    0xFF, /* unused 101 */
    MKV_RIGHTARROW, /* unused 102 */
    MKV_END, /* unused 103 */
    MKV_DOWNARROW, /* unused 104 */
    MKV_PAGEDOWN, /* unused 105 */
    MKV_HELP, /* unused 106 */
    MKV_DELFORWARD, /* unused 107 */
    MKV_NUMENTER, /* unused 108 */
    MKV_RIGHTCNTL, /* unused 109 */
    MKV_F15, /* unused 110 (pause/break)*/
    MKV_F13, /* unused 111 (print scrn/sysrq) */
    MKV_NUMEQUAL, /* unused 112 */
    MKV_LEFTOPTION, /* unused 113 (right alt) */
    0xFF, /* unused 114 */
    MKV_LEFTOPTION, /* unused 115 (left windows) */
    MKV_RIGHTOPTION, /* unused 116 (right windows) */
    MKV_RESET, /* unused 117 (windows menu selection thingy) */
    MKV_F15, /* <PAUS> = 118; */
    MKV_PAGEUP, /* <PGUP> = 119; */
    MKV_DOWNARROW, /* <DOWN> = 120; */
    MKV_F13, /* <PRSC> = 121; */
    MKV_RIGHTOPTION, /* <RALT> = 122; */
    MKV_RIGHTCNTL, /* <RCTL> = 123; */
    MKV_NUMENTER, /* <KPEN> = 124; */
    MKV_NUMDIVIDE, /* <KPDV> = 125; */
    0xFF, /* unused 126 */
    0xFF, /* unused 127 */
    MKV_UPARROW, /* <UP>   = 128; */
    MKV_DELFORWARD, /* <DELE> = 129; */
    MKV_END, /* <END>  = 130; */
    MKV_HELP, /* <INS>  = 131; */
    MKV_LEFTARROW, /* <LEFT> = 132; */
    MKV_RIGHTARROW, /* <RGHT> = 133; */
    MKV_PAGEDOWN, /* <PGDN> = 134; */
    MKV_HOME, /* <HOME> = 135; */
};
#endif
