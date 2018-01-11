#if !defined(_SYSERR_H_)
#define _SYSERR_H_

/*
 * Copyright 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
typedef enum { EXIST_YES = 0,
               EXIST_NO = 0xFF } exist_enum_t;

const LowMemGlobal<Ptr> DSAlertTab { 0x2BA }; // SysErr IMII-359 (true);
const LowMemGlobal<Rect> DSAlertRect { 0x3F8 }; // SysErr IMII-362 (true);
const LowMemGlobal<Byte> WWExist { 0x8F2 }; // SysError SysEqu.a (true);
const LowMemGlobal<Byte> QDExist { 0x8F3 }; // SysError SysEqu.a (true);

extern char syserr_msg[];

extern void C_SysError(short errorcode);
}

#endif /* !_SYSERR_H_ */
