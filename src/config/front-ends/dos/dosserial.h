#if !defined (_dosserial_h_)
#define _dosserial_h_

/*
 * Copyright 1997 by Abacus Research and Development, Inc.
 * All rights reserved.
 */

#include "Serial.h"

/*
 * This should really be divided into public and private .h files
 */

typedef enum { parity_none, parity_odd, parity_even } parity_t;

typedef struct
{
  uint8 line_data_ready:              1;
  uint8 line_overrun:                 1;
  uint8 line_parity_error:            1;
  uint8 line_framing_error:           1;
  uint8 line_break_received:          1;
  uint8 line_transmit_buffer_empty:   1;
  uint8 line_holding_and_shift_empty: 1;
  uint8 line_timeout_error:           1;
}
line_status_t;

typedef union
{
  uint8 ub;
  line_status_t status;
}
line_status_u;

typedef struct
{
  uint8 modem_delta_cts:            1;
  uint8 modem_delta_dsr:            1;
  uint8 modem_trailing_ring_detect: 1;
  uint8 modem_delta_dcd:            1;
  uint8 modem_cts:                  1;
  uint8 modem_dsr:                  1;
  uint8 modem_ring:                 1;
  uint8 modem_dcd:                  1;
}
modem_status_t;

typedef union
{
  uint8 ub;
  modem_status_t status;
}
modem_status_u;

/*
 * Modem control register as documented for a 16C1450/16C1550.
 * Beware -- some of these bits do different things on other serial chips.
 * Right now, we're only using these to control DTR.  See _The Undocumented PC_
 * pages 612 and 613 for more info.
 */

typedef struct
{
  uint8 modem_control_dtr:             1; /* NOTE: _TUPC_ p.613 claims that
					     this big is dsr, but p. 573 calls
					     this bit dtr, by implication */
  uint8 modem_control_rts:             1;
  uint8 modem_control_reset_uart:      1;
  uint8 modem_control_enable_irqs:     1;
  uint8 modem_control_loopback_enable: 1;
  uint8 modem_control_unused0:         1;
  uint8 modem_control_unused1:         1;
  uint8 modem_control_power_savings:   1;
}
modem_control_register_t;

typedef union
{
  uint8 ub;
  modem_control_register_t reg;
}
modem_control_register_u;

typedef uint8 bios_port_t;

typedef enum
{
  bios_serial_baud_110,
  bios_serial_baud_150,
  bios_serial_baud_300,
  bios_serial_baud_600,
  bios_serial_baud_1200,
  bios_serial_baud_2400,
  bios_serial_baud_4800,
  bios_serial_baud_9600,
}
bios_serial_baud_t;	       

typedef enum
{
  bios_serial_parity_none0,
  bios_serial_parity_odd,
  bios_serial_parity_none1,
  bios_serial_parity_even,
}
bios_serial_parity_t;

typedef enum
{
  bios_serial_stop_bits_1,
  bios_serial_stop_bits_more_than_1,
}
bios_serial_stop_bits_t;

typedef enum
{
  bios_serial_length_5,
  bios_serial_length_6,
  bios_serial_length_7,
  bios_serial_length_8,
}
bios_serial_length_t;

typedef struct
{
  bios_serial_length_t    bios_serial_init_data_length: 2;
  bios_serial_stop_bits_t bios_serial_init_stop_bits:   1;
  bios_serial_parity_t    bios_serial_init_parity:      2;
  bios_serial_baud_t      bios_serial_init_baud:        3;
}
bios_serial_init_t;

typedef union
{
  uint8 ub;
  bios_serial_init_t init;
}
bios_serial_init_u;

extern ssize_t serial_bios_read (LONGINT fd, void *buf, size_t count);

extern ssize_t serial_bios_write (LONGINT fd, void *buf, size_t count);

extern OSErr serial_bios_serset (LONGINT fd, INTEGER param);

extern OSErr serial_bios_serxhshake (LONGINT fd, SerShk *sershkp);

extern OSErr serial_bios_setbaud (LONGINT fd, INTEGER baud);

extern OSErr serial_bios_ctlbrk (LONGINT fd, INTEGER flag);

extern OSErr serial_bios_setflow (LONGINT fd, LONGINT flag);

extern OSErr serial_bios_setdtr (LONGINT fd);

extern OSErr serial_bios_clrdtr (LONGINT fd);

extern int serial_bios_fionread (LONGINT fd, LONGINT *np);

extern int set_modem_port_mapping_to_pc_port (int pc_port);

extern int set_printer_port_mapping_to_pc_port (int pc_port);

#endif /* !defined (_dosserial_h_) */
