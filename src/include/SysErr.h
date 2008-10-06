#if !defined (_SYSERR_H_)
#define _SYSERR_H_

/*
 * Copyright 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: SysErr.h 63 2004-12-24 18:19:43Z ctm $
 */

typedef enum { EXIST_YES = 0, EXIST_NO = 0xFF } exist_enum_t;

#if !defined (DSAlertTab_H)
extern HIDDEN_Ptr DSAlertTab_H;
extern Rect DSAlertRect;
extern Byte 	WWExist;
extern Byte 	QDExist;
#endif

#define DSAlertTab (DSAlertTab_H.p)

extern char syserr_msg[];

extern pascal void C_SysError (short errorcode);

#endif /* !_SYSERR_H_ */
