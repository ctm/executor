#if !defined (_ITM_H_)
#define _ITM_H_

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: itm.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "rsys/pstuff.h"
namespace Executor {
struct itmstr { GUEST_STRUCT;
    GUEST< Handle> itmhand;
    GUEST< Rect> itmr;
    GUEST< unsigned char> itmtype;
    GUEST< unsigned char> itmlen;
};

typedef itmstr *itmp;

typedef GUEST<itmp> *itmh;

#define DIALOG_RES_HAS_POSITION_P(dlogh)	\
  ((  20					\
    + ((HxX (dlogh, dlglen) + 2) & ~1)		\
    + 2) == GetHandleSize ((Handle) (dlogh)))
#define DIALOG_RES_POSITION_X(dlogh)			\
  (*(int16 *) ((char *) &HxX (dlogh, dlglen)		\
	       + ((HxX (dlogh, dlglen) + 2) & ~1)))

#define DIALOG_RES_POSITION(dlog)			\
  CW (DIALOG_RES_POSITION_X (dlog))


#define ALERT_RES_HAS_POSITION_P(alerth)	\
  ((sizeof (altstr) + 2) == GetHandleSize ((Handle) (alerth)))

#define ALERT_RES_POSITION_X(alerth)		\
  (*(int16 *) ((char *) &HxX (alerth, altstag) + 2))
#define ALERT_RES_POSITION(alerth)		\
  CW (ALERT_RES_POSITION_X (alerth))

#define noAutoCenter				(0)
#define alertPositionParentWindow		(0xB00A)
#define alertPositionMainScreen			(0x300A)
#define alertPositionParentWindowScreen		(0x700A)
#define dialogPositionParentWindow		(0xA80A)
#define dialogPositionMainScreen		(0x280A)
#define dialogPositionParentWindowScreen	(0x680A)

extern void dialog_compute_rect (Rect *dialog_rect, Rect *dst_rect,
				 int position);

struct altstr { GUEST_STRUCT;
    GUEST< Rect> altr;
    GUEST< INTEGER> altiid;
    GUEST< INTEGER> altstag;
};

typedef altstr *altp;

typedef GUEST<altp> *alth;

struct dlogstr { GUEST_STRUCT;
    GUEST< Rect> dlgr;
    GUEST< INTEGER> dlgprocid;
    GUEST< char> dlgvis;
    GUEST< char> dlgfil1;
    GUEST< char> dlggaflag;
    GUEST< char> dlgfil2;
    GUEST< LONGINT> dlgrc;
    GUEST< INTEGER> dlgditl;
    GUEST< char> dlglen;
};
typedef dlogstr *dlogp;

typedef GUEST<dlogp> *dlogh;

typedef struct item_style_info { GUEST_STRUCT;
    GUEST< int16> font;
    GUEST< Style> face;
    GUEST< unsigned char> filler;
    GUEST< int16> size;
    GUEST< RGBColor> foreground;
    GUEST< RGBColor> background;
    GUEST< int16> mode;
} item_style_info_t;

typedef struct item_color_info { GUEST_STRUCT;
    GUEST< int16> data;
    GUEST< int16> offset;
} item_color_info_t;

extern itmp ROMlib_dpnotoip (DialogPeek dp, INTEGER itemno, SignedByte *flags);
extern void ROMlib_dpntoteh (DialogPeek dp, INTEGER no);
extern void C_ROMlib_mysound (INTEGER n);

extern void ROMlib_drawiptext (DialogPtr dp, itmp ip, int item_no);
extern void dialog_create_item (DialogPeek dp, itmp dst, itmp src,
				int item_no, Point base_pt);
extern boolean_t get_item_style_info (DialogPtr dp, int item_no,
				      uint16 *flags_return,
				      item_style_info_t *style_info);

extern void dialog_draw_item (DialogPtr dp, itmp itemp, int itemno);

#define ITEM_RECT(itemp)	\
  ((itemp)->itmr)

#define ITEM_H_X(itemp)		\
  ((itemp)->itmhand)
#define ITEM_H(itemp)		(MR (ITEM_H_X (itemp)))

#define ITEM_TYPE(itemp)	\
  ((itemp)->itmtype)

#define ITEM_LEN(itemp)		\
  ((sizeof *(itemp) + (itemp)->itmlen + 1) & ~1)
#define ITEM_DATA(itemp)	\
  ((int16 *) ((itemp) + 1))

#define BUMPIP(ip)					\
  ((void)						\
   ({							\
     (ip) = (itmp) ((char *) (ip) + ITEM_LEN (ip));	\
   }))

typedef pascal void (*soundprocp) (INTEGER sound);

extern void BEEPER (INTEGER n);
#define BEEP(n)	BEEPER(n)
}
#endif /* _ITM_H_ */
