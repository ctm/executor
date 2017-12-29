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

#if 0
#if !defined(DSAlertTab_H)
extern GUEST<Ptr> DSAlertTab_H;
extern Rect DSAlertRect;
extern Byte 	WWExist;
extern Byte 	QDExist;
#endif

enum
{
    DSAlertTab = (DSAlertTab_H.p),
};
#endif

extern char syserr_msg[];

extern pascal void C_SysError(short errorcode);
}

#endif /* !_SYSERR_H_ */
