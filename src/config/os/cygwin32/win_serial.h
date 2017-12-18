#if !defined(_win_serial_h_)
#define _win_serial_h_

/*
 * Copyright 1997 by Abacus Research and Development, Inc.
 * All rights reserved.
 */

#include "Serial.h"

/*
 * This should really be divided into public and private .h files
 */

typedef enum { parity_none,
               parity_odd,
               parity_even } parity_t;

typedef enum {
    win32_serial_parity_no,
    win32_serial_parity_odd,
    win32_serial_parity_even,
    win32_serial_parity_mark,
    win32_serial_parity_space,
} win32_serial_parity_t;

typedef enum {
    win32_serial_stop_bits_1,
    win32_serial_stop_bits_1_and_a_half,
    win32_serial_stop_bits_2,
} win32_serial_stop_bits_t;

extern uint32_t serial_bios_read(LONGINT fd, void *buf, size_t count);

extern uint32_t serial_bios_write(LONGINT fd, void *buf, size_t count);

extern int32_t serial_bios_serset(LONGINT fd, INTEGER param);

extern int32_t serial_bios_serxhshake(LONGINT fd, SerShk *sershkp);

extern int32_t serial_bios_setbaud(LONGINT fd, INTEGER baud);

extern int32_t serial_bios_ctlbrk(LONGINT fd, INTEGER flag);

extern int32_t serial_bios_setflow(LONGINT fd, LONGINT flag);

extern int32_t serial_bios_setdtr(LONGINT fd);

extern int32_t serial_bios_clrdtr(LONGINT fd);

extern int serial_bios_fionread(LONGINT fd, LONGINT *np);

extern int set_modem_port_mapping_to_pc_port(int pc_port);

extern int set_printer_port_mapping_to_pc_port(int pc_port);

#endif /* !defined (_win_serial_h_) */
