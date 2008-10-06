/*
 * Copyright 1997 by Abacus Research and Development, Inc.
 * All rights reserved.
 */

/*
 * NOTE: this is *NOT* the way we want to implement serial stuff; it's
 *       just a bunch of stubs to make serial.c compile
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_winserial[] =
	    "$Id: winserial.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "winserial.h"
#include "FileMgr.h"
#include "Serial.h"

PUBLIC ssize_t
serial_bios_read (LONGINT fd, void *buf, size_t count)
{
  ssize_t retval;

  retval = 0;
  return retval;
}

PUBLIC ssize_t
serial_bios_write (LONGINT fd, void *buf, size_t count)
{
  ssize_t retval;

  retval = 0;
  return retval;
}

PUBLIC OSErr
serial_bios_serset (LONGINT fd, INTEGER param)
{
  OSErr retval;

  retval = paramErr;
  return retval;
}

PUBLIC OSErr
serial_bios_serxhshake (LONGINT fd, SerShk *sershkp)
{
  OSErr retval;

  retval = paramErr;
  return retval;
}

PUBLIC OSErr
serial_bios_setbaud (LONGINT fd, INTEGER baud)
{
  OSErr retval;

  retval = paramErr;
  return retval;
}

PUBLIC OSErr
serial_bios_ctlbrk (LONGINT fd, INTEGER flag)
{
  OSErr retval;

  retval = paramErr;
  return retval;
}

PUBLIC OSErr
serial_bios_setflow (LONGINT fd, LONGINT flag)
{
  OSErr retval;

  retval = paramErr;
  return retval;
}

PUBLIC OSErr
serial_bios_setdtr (LONGINT fd)
{
  OSErr retval;

  retval = paramErr;
  return retval;
}

PUBLIC OSErr
serial_bios_clrdtr (LONGINT fd)
{
  OSErr retval;

  retval = paramErr;
  return retval;
}

PUBLIC int
serial_bios_fionread (LONGINT fd, LONGINT *np)
{
  int retval;

  retval = 0;
  return retval;
}

PUBLIC int
set_modem_port_mapping_to_pc_port (int pc_port)
{
  int retval;

  retval = 0;
  return retval;
}

PUBLIC int
set_printer_port_mapping_to_pc_port (int pc_port)
{
  int retval;

  retval = 0;
  return retval;
}
