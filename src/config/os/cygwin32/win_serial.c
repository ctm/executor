/*
 * Copyright 1997 by Abacus Research and Development, Inc.
 * All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_win_serial[] =
	    "$Id: win_serial.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"
#include "rsys/error.h"

#include "win_serial.h"

#include <windows.h>


enum { PARAMETER_ERROR = -2 };

enum { noErr = 0 };

/*
 * A useful macro for looking up the value of a key in a mapping table.
 * It uses the gcc-specific construct of ({ ... })
 */

#define LOOKUP_KEY(valp, thekey, map)		\
({						\
  int i;					\
  int retval;					\
						\
  retval = PARAMETER_ERROR;			\
  for (i = 0; i < (int) NELEM (map); ++i)	\
    if (map[i].key == thekey)			\
      {						\
	*valp = map[i].val;			\
	retval = 0;				\
	break;					\
      }						\
  retval;					\
})

static struct
{
  int val; /* 0 = COM1, 1 = COM2 ... */
  uint8 key;
} serial_port_map[] =
  {
    { 0, 0 },
    { 1, 1 },
  };

/*
 * xxx = 0 for modem, 1 for printer
 */

PRIVATE int
set_xxx_port_mapping_to_pc_port (int xxx, int pc_port)
{
  int retval;

  if (xxx >= 0 && xxx <= 1 && pc_port >= 1 && pc_port <= 4)
    {
      serial_port_map[xxx].val = pc_port - 1;
      retval = 0;
    }
  else
    retval = PARAMETER_ERROR;

  return retval;
}

PUBLIC int
set_modem_port_mapping_to_pc_port (int pc_port)
{
  int retval;

  retval = set_xxx_port_mapping_to_pc_port (0, pc_port);
  return retval;
}

PUBLIC int
set_printer_port_mapping_to_pc_port (int pc_port)
{
  int retval;

  retval = set_xxx_port_mapping_to_pc_port (1, pc_port);
  return retval;
}

/*
 * These four helper routines convert from the values we use on the outside
 * world to the values that the Win32 calls use.  If an inappropriate source
 * value is supplied, then they return PARAMETER_ERROR, otherwise they return
 * 0
 */

PRIVATE int
baud_to_win32_baud (DWORD *win32_baudp, uint32 baud)
{
  int retval;

  struct
  {
    DWORD val;
    uint32 key;
  } map[] =
    {
      {   300, baud300,  },
      {   600, baud600,  },
      {  1200, baud1200, },
      {  1800, baud1800, },
      {  2400, baud2400, },
      {  3600, baud3600, },
      {  4800, baud4800, },
      {  7200, baud7200, },
      {  9600, baud9600, },
      { 19200, baud19200 },
      { 57600, baud57600 },
    };
  retval = LOOKUP_KEY (win32_baudp, baud, map);
  return retval;
}

PRIVATE int
parity_to_win32_parity (BYTE *win32_parityp, parity_t parity)
{
  int retval;

  struct
    {
      BYTE val;
      parity_t key;
  } map[] =
    {
      { win32_serial_parity_no, noParity, },
      { win32_serial_parity_odd, oddParity, },
      { win32_serial_parity_even, evenParity, },
    };
  retval = LOOKUP_KEY (win32_parityp, parity, map);
  return retval;
}

PRIVATE int
stop_to_win32_stop (BYTE *win32_stopp, uint32 stop_bits)
{
  int retval;

  struct
  {
    BYTE val;
    uint32 key;
  } map[] = 
  {
    { win32_serial_stop_bits_1, 1, },
    { win32_serial_stop_bits_1_and_a_half, 2, },
    { win32_serial_stop_bits_2, 3, }
  };
  retval = LOOKUP_KEY (win32_stopp, stop_bits, map);
  return retval;
}

PRIVATE int
length_to_win32_length (BYTE *win32_lengthp, uint32 data_length)
{
  int retval;

  struct
  {
    BYTE val;
    uint32 key;
  } map[] =
  {
    { 5, data5, },
    { 6, data6, },
    { 7, data7, },
    { 8, data8, },
  };
  retval = LOOKUP_KEY (win32_lengthp, data_length, map);
  return retval;
}

/*
 * These four macros convert from the values we use on the outside world
 * to the values that the WIN32 calls use.  If an inappropriate source value
 * is supplied, they automatically force the calling function to return
 *  PARAMETER_ERROR
 */ 

#define BAUD_TO_WIN32_BAUD(win32_baudp, baud)			\
do								\
{								\
  if (baud_to_win32_baud (win32_baudp, baud) != 0)		\
    return PARAMETER_ERROR;					\
} while (0)

#define PARITY_TO_WIN32_PARITY(win32_parityp, parity)		\
do								\
{								\
  if (parity_to_win32_parity (win32_parityp, parity) != 0)	\
    return PARAMETER_ERROR;					\
} while (0)

#define STOP_TO_WIN32_STOP(win32_stopp, stop_bits)		\
do								\
{								\
  if (stop_to_win32_stop (win32_stopp, stop_bits) != 0)		\
    return PARAMETER_ERROR;					\
} while (0)

#define LENGTH_TO_WIN32_LENGTH(win32_lengthp, data_length)	\
do								\
{								\
  if (length_to_win32_length (win32_lengthp, data_length) != 0)	\
    return PARAMETER_ERROR;					\
} while (0)

enum { MAX_N_PORTS = 4 };

/*
 * It would be nicer if we opened and closed ports as part of the serial
 * open and close routines, but right now we're mimicing the old BIOS
 * paradigm of just using ports without opening them.
 */

/* port in this case is 0 for modem, 1 for printer */

#define COM_TEMPLATE "COM1"

PRIVATE HANDLE
port_to_handle (uint8 port)
{
  HANDLE retval;
  int com_minus_1;
  static boolean_t been_here;
  static HANDLE com_hands[MAX_N_PORTS];

  if (!been_here)
    {
      int i;

      for (i = 0; i < (int) NELEM (com_hands); ++i)
	com_hands[i] = INVALID_HANDLE_VALUE;
      been_here = TRUE;
    }

  if (LOOKUP_KEY (&com_minus_1, port, serial_port_map) != 0)
    retval = INVALID_HANDLE_VALUE;
  else
    {
      if (com_minus_1 < 0 || com_minus_1 >= MAX_N_PORTS)
	retval = INVALID_HANDLE_VALUE;
      else
	{
	  retval = com_hands[com_minus_1];
	  if (retval == INVALID_HANDLE_VALUE)
	    {
	      char *com_filename;

	      com_filename = alloca (sizeof COM_TEMPLATE);
	      memcpy (com_filename, COM_TEMPLATE, sizeof COM_TEMPLATE);
	      com_filename[sizeof COM_TEMPLATE - 2] += com_minus_1;
	      retval = com_hands[com_minus_1]
		= CreateFile (com_filename, GENERIC_READ | GENERIC_WRITE,
			      0, NULL, OPEN_EXISTING, 0, NULL);
	    }
	}
    }
  return retval;
}

/*
 * These are the serial port accessing routines that the BIOS directly
 * provides.  We build all other routines on top of these (at least until
 * we write our own drivers).
 */

PRIVATE int
dos_serial_bios_init_port (uint8 port,
			   uint32 baud,
			   uint32 parity,
			   uint32 stop_bits,
			   uint32 data_length)
{
  int retval;
  HANDLE h;

  h = port_to_handle (port);
  if (h == INVALID_HANDLE_VALUE)
    retval = -1;
  else
    {
      DCB dcb;

      dcb.DCBlength = sizeof dcb;
      if (!GetCommState (h, &dcb))
	retval = -1;
      else
	{
	  DWORD win32_baud;
	  BYTE win32_parity;
	  BYTE win32_stop;
	  BYTE win32_length;

	  BAUD_TO_WIN32_BAUD (&win32_baud, baud);
	  PARITY_TO_WIN32_PARITY (&win32_parity, parity);
	  STOP_TO_WIN32_STOP (&win32_stop, stop_bits);
	  LENGTH_TO_WIN32_LENGTH (&win32_length, data_length);
	  
	  dcb.BaudRate = win32_baud;
	  dcb.fBinary = TRUE;
	  dcb.fParity = TRUE;
	  dcb.ByteSize = win32_length;
	  dcb.Parity = win32_parity;
	  dcb.StopBits = win32_stop;
	  retval = SetCommState (h, &dcb) ? 0 : -1;
	}
    }
  return retval;
}

typedef WINBOOL (WINAPI *xferfuncp_t) (HANDLE, PVOID, DWORD, PDWORD, LPOVERLAPPED);

PUBLIC uint32
serial_bios_xfer (LONGINT fd, void *buf, size_t count, xferfuncp_t funcp)
{
  HANDLE h;
  uint32 retval;

  h = port_to_handle (fd);
  if (h == INVALID_HANDLE_VALUE)
    retval = 0;
  else
    {
      DWORD nread;
      
      if (funcp (h, buf, count, &nread, NULL))
	retval = nread;
      else
	retval = 0;
    }
  return retval;
}

/*
 * Here are the public routines.  They're made to slip comfortably into
 * the existing "serial.c" code (which is quite a hack)
 *
 * The first implementation of these routines is incredibly naive.  One thing
 * that we can do if the first cut isn't sufficient is to always pay attention
 * to the line status, and whenever there's a character that can be read,
 * read it and squirrel it away for future use.  That can be done in all of
 * these routines, even the write routine.
 */

PUBLIC uint32
serial_bios_read (LONGINT fd, void *buf, size_t count)
{
  uint32 retval;

  retval = serial_bios_xfer (fd, buf, count, ReadFile);
  return retval;
}

PUBLIC uint32
serial_bios_write (LONGINT fd, void *buf, size_t count)
{
  uint32 retval;

  retval = serial_bios_xfer (fd, buf, count, (void *) WriteFile);
  return retval;
}

PUBLIC int32
serial_bios_serset (LONGINT fd, INTEGER param)
{
  int32 retval;
  uint32 mac_baud, mac_data_length, mac_parity, mac_stop_bits;

  /* TODO */
  mac_baud = param & 0x3FF;
  mac_data_length = param & 0xc00;
  mac_parity = param & 0x3000;
  mac_stop_bits = ((param >> 14) & 0x3);

  dos_serial_bios_init_port (fd, mac_baud, mac_parity, mac_stop_bits,
			     mac_data_length);

  retval = noErr;
  return retval;
}

PUBLIC int32
serial_bios_serxhshake (LONGINT fd, SerShk *sershkp)
{
  int32 retval;

  warning_unimplemented (NULL_STRING);
  retval = noErr;
  return retval;
}

/*
 * In order to explicitly set the baud without setting anything else, we'll
 * need to keep track of the last settings we've made for each port.  That's
 * not hard to do, but we just don't do it yet.
 */

PUBLIC int32
serial_bios_setbaud (LONGINT fd, INTEGER baud)
{
  int32 retval;

  warning_unimplemented (NULL_STRING);
  retval = noErr;
  return retval;
}

PUBLIC int32
serial_bios_ctlbrk (LONGINT fd, INTEGER flag)
{
  int32 retval;

  warning_unimplemented (NULL_STRING);
  retval = noErr;
  return retval;
}

PUBLIC int32
serial_bios_setflow (LONGINT fd, LONGINT flag)
{
  int32 retval;

  warning_unimplemented (NULL_STRING);
  retval = noErr;
  return retval;
}

PUBLIC int32
serial_bios_setdtr (LONGINT fd)
{
  int32 retval;

  warning_unimplemented (NULL_STRING);
  retval = noErr;
  return retval;
}

PUBLIC int32
serial_bios_clrdtr (LONGINT fd)
{
  int32 retval;

  warning_unimplemented (NULL_STRING);
  retval = noErr;
  return retval;
}

PUBLIC int
serial_bios_fionread (LONGINT fd, LONGINT *np)
{
  HANDLE h;
  int retval;

  h = port_to_handle (fd);
  if (h == INVALID_HANDLE_VALUE)
    retval = -1;
  else
    {
      DWORD err;
      COMSTAT comstat;

      if (!ClearCommError (h, &err, &comstat))
	retval = -1;
      else
	{
	  *np = comstat.cbInQue;
	  retval = 0;
	}
    }
  return retval;
}
