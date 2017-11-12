#if !defined(_RSYS_CRC_H_)
#define _RSYS_CRC_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: crc.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor
{
extern short getthecrc(ResType type, long id);
extern unsigned short ROMlib_crcccitt(unsigned char *data, long length);
}
#endif /* _RSYS_CRC_H_ */
