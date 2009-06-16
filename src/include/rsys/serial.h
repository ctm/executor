#if !defined(_SERIAL_H_)
#define _SERIAL_H_

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: serial.h 63 2004-12-24 18:19:43Z ctm $
 */

typedef struct {
  Ptr p;
  INTEGER i;
} sersetbuf_t;

extern OSErr ROMlib_serialopen (ParmBlkPtr pbp, DCtlPtr dcp);
extern OSErr ROMlib_serialprime (ParmBlkPtr pbp, DCtlPtr dcp);
extern OSErr ROMlib_serialctl (ParmBlkPtr pbp, DCtlPtr dcp);
extern OSErr ROMlib_serialstatus (ParmBlkPtr pbp, DCtlPtr dcp);
extern OSErr ROMlib_serialclose (ParmBlkPtr pbp, DCtlPtr dcp);

#endif
