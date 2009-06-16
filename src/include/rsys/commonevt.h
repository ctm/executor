#if !defined (_COMMONEVT_H_)
#define _COMMONEVT_H_

/*
 * commonevents are Xevents with the X constants changed to constants that
 * we have control of.  This means that only one source module actually has
 * to include X include files.  This is necessary since both QuickDraw and
 * X have their own idea of what the type "Cursor" should be.
 */

typedef struct {
    enum { commonkeypress,      commonkeyrelease, commonbuttonpress,
	   commonbuttonrelease, commonexpose,     commonenternotify,
	   commonleavenotify,   commonmotionnotify  } type;
  INTEGER button;
  INTEGER x;
  INTEGER y;
  LONGINT state;
} commonevent;

#define commonshiftmask		(1 << 0)
#define commonlockmask		(1 << 1)
#define commoncontrolmask	(1 << 2)
#define commonmod1mask		(1 << 3)
#define commonbutton2mask	(1 << 4)
#define commonbutton1mask	(1 << 5)

#define commonbutton1	1
#define commonbutton2	2
#define commonbutton3	3

#endif /* !_COMMONEVT_H_ */
