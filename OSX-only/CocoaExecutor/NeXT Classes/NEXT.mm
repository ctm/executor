/* Copyright 1991, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_NEXT[] =
		    "$Id: NEXT.c 63 2004-12-24 18:19:43Z ctm $";
#endif


#include "rsys/common.h"

#include "rsys/notmac.h"
#include <Cocoa/Cocoa.h>

#include "QuickDraw.h"
#include "rsys/keyboards.h"
#include "OSEvent.h"
#include "rsys/osevent.h"
#include "rsys/prefs.h"
#include "rsys/flags.h"
#include "rsys/print.h"
#include "rsys/vdriver.h"
#include "rsys/stubify.h"
#include "rsys/arrowkeys.h"
#include "ourstuff.h"

#include "ToolboxEvent.h"
#include "ResourceMgr.h"

using namespace Executor;

PUBLIC keyboard_enum_t Executor::ROMlib_keyboard_type;

#define DIACREP1START	0x81
unsigned char diacrep1[] = {
  0xcb, 0xe7, 0xe5, 0xcc, 0x80, 0x81, 0x82,	/* 0x81 - 0x87 */
  0xe9, 0x83, 0xe6, 0xe8, 0xed, 0xea, 0xeb, 0xec,	/* 0x88 - 0x8F */
  0x00, 0x84, 0xf1, 0xee, 0xef, 0xcd, 0x85, 0xf4,	/* 0x90 - 0x97 */
  0xf2, 0xf3, 0x86,					/* 0x98 - 0x9A */
};

#define DIACREP2START	0xD5
unsigned char diacrep2[] = {
  0x88, 0x87, 0x89,	/* 0xD5 - 0xD7 */
  0x8b, 0x8a, 0x8c, 0x8d, 0x8f, 0x8e, 0x90, 0x91,	/* 0xD8 - 0xDF */
  0x93, 0xae, 0x92, 0xbb, 0x94, 0x95, 0x00, 0x96,	/* 0xE0 - 0xE7 */
  0x00, 0x00, 0x00, 0x00, 0x98, 0x97, 0x99, 0x9b,	/* 0xE8 - 0xEF */
  0x9a, 0xbe, 0x9d, 0x9c, 0x9e, 0x00, 0x9f, 0x00,	/* 0xF0 - 0xF7 */
  0x00, 0xbf, 0xcf, 0x00, 0x00, 0xd8,			/* 0xF8 - 0xFD */
};

/*
 * NOTE: this table is used in DOS land also
 */

unsigned char Executor::next_virt_to_mac_virt[NVIRTMAPS][NKEYSTOMAP] =
{
  {
    0xFF,	/* unused 0x00 */
    0xFF,	/* unused 0x01 */
    0xFF,	/* unused 0x02 */
    0xFF,	/* unused 0x03 */
    0x1E,	/* ']' 0x04 */
    0x21,	/* '[' 0x05 */
    0x22,	/* 'i' 0x06 */
    0x1F,	/* 'o' 0x07 */
    0x23,	/* 'p' 0x08 */
    0x7B,	/* ',' 0x09	left arrow */
    0xFF,	/* unused 0x0a */
    0x52,	/* '0' 0x0b	numeric keypad */
    0x41,	/* '.' 0x0c	numeric keypad */
    0x4C,	/* '' 0x0d	enter numeric keypad */
    0xFF,	/* unused 0x0e */
    0x7D,	/* '/' 0x0f	down arrow */
    0x7C,	/* '.' 0x10	right arrow */
    0x53,	/* '1' 0x11	numeric keypad */
    0x56,	/* '4' 0x12	numeric keypad */
    0x58,	/* '6' 0x13	numeric keypad */
    0x55,	/* '3' 0x14	numeric keypad */
    0x45,	/* '+' 0x15	numeric keypad */
    0x7E,	/* '-' 0x16	up arrow */
    0x54,	/* '2' 0x17	numeric keypad */
    0x57,	/* '5' 0x18	numeric keypad */
    0xFF,	/* unused 0x19 */
    0xFF,	/* unused 0x1a */
    0x33,	/* '' 0x1b	backspace */
    0x18,	/* '=' 0x1c */
    0x1B,	/* '-' 0x1d */
    0x1C,	/* '8' 0x1e */
    0x19,	/* '9' 0x1f */
    0x1D,	/* '0' 0x20 */
    0x59,	/* '7' 0x21	numeric keypad */
    0x5B,	/* '8' 0x22	numeric keypad */
    0x5C,	/* '9' 0x23	numeric keypad */
    0x4E,	/* '-' 0x24	numeric keypad */
    0x43,	/* '*' 0x25	numeric keypad */
    0x32,	/* '`' 0x26	numeric keypad */	/*CTMHACK*/
    0x18,	/* '=' 0x27	numeric keypad */	/*CTMHACK*/
    0x2C,	/* '/' 0x28	numeric keypad */	/*CTMHACK*/
    0xFF,	/* unused 0x29 */
    0x24,	/* '
' 0x2a	carraige return */
    0x27,	/* ''' 0x2b */
    0x29,	/* ';' 0x2c */
    0x25,	/* 'l' 0x2d */
    0x2B,	/* ',' 0x2e */
    0x2F,	/* '.' 0x2f */
    0x2C,	/* '/' 0x30 */
    0x06,	/* 'z' 0x31 */
    0x07,	/* 'x' 0x32 */
    0x08,	/* 'c' 0x33 */
    0x09,	/* 'v' 0x34 */
    0x0B,	/* 'b' 0x35 */
    0x2E,	/* 'm' 0x36 */
    0x2D,	/* 'n' 0x37 */
    0x31,	/* ' ' 0x38 */
    0x00,	/* 'a' 0x39 */
    0x01,	/* 's' 0x3a */
    0x02,	/* 'd' 0x3b */
    0x03,	/* 'f' 0x3c */
    0x05,	/* 'g' 0x3d */
    0x28,	/* 'k' 0x3e */
    0x26,	/* 'j' 0x3f */
    0x04,	/* 'h' 0x40 */
    0x30,	/* '	' 0x41	tab */
    0x0C,	/* 'q' 0x42 */
    0x0D,	/* 'w' 0x43 */
    0x0E,	/* 'e' 0x44 */
    0x0F,	/* 'r' 0x45 */
    0x20,	/* 'u' 0x46 */
    0x10,	/* 'y' 0x47 */
    0x11,	/* 't' 0x48 */
    0x35,	/* '' 0x49	escape */
    0x12,	/* '1' 0x4a */
    0x13,	/* '2' 0x4b */
    0x14,	/* '3' 0x4c */
    0x15,	/* '4' 0x4d */
    0x1A,	/* '7' 0x4e */
    0x16,	/* '6' 0x4f */
    0x17,	/* '5' 0x50 */
    0xFF,	/* unused 0x51 */
    0xFF,	/* unused 0x52 */
    0xFF,	/* unused 0x53 */
    0xFF,	/* unused 0x54 */
    0xFF,	/* unused 0x55 */
    0xFF,	/* unused 0x56 */
    0xFF,	/* unused 0x57 */
    0xFF,	/* unused 0x58 */
    0xFF,	/* unused 0x59 */
    0xFF,	/* unused 0x5A */
    0xFF,	/* unused 0x5B */
    0xFF,	/* unused 0x5C */
    0xFF,	/* unused 0x5D */
    0xFF,	/* unused 0x5E */
    0xFF,	/* unused 0x5F */
    0xFF,	/* unused 0x60 */
    0xFF,	/* unused 0x61 */
    0xFF,	/* unused 0x62 */
    0xFF,	/* unused 0x63 */
    0xFF,	/* unused 0x64 */
    0xFF,	/* unused 0x65 */
    0xFF,	/* unused 0x66 */
    0xFF,	/* unused 0x67 */
  },
  {
    0xFF,	/* unused 0x00 */
    0x35,	/* ESC 0x01 */
    0x12,	/* '1' 0x02 */
    0x13,	/* '2' 0x03 */
    0x14,	/* '3' 0x04 */
    0x15,	/* '4' 0x05 */
    0x17,	/* '5' 0x06 */
    0x16,	/* '6' 0x07 */
    0x1A,	/* '7' 0x08 */
    0x1C,	/* '8' 0x09 */
    0x19,	/* '9' 0x0a */
    0x1D,	/* '0' 0x0b */
    0x1B,	/* '-' 0x0c */
    0x18,	/* '=' 0x0d */
    0x33,	/* BACKSPACE 0x0e */
    0x30,	/* TAB 0x0f */
    0x0C,	/* 'Q' 0x10 */
    0x0D,	/* 'W' 0x11 */
    0x0E,	/* 'E' 0x12 */
    0x0F,	/* 'R' 0x13 */
    0x11,	/* 'T' 0x14 */
    0x10,	/* 'Y' 0x15 */
    0x20,	/* 'U' 0x16 */
    0x22,	/* 'I' 0x17 */
    0x1F,	/* 'O' 0x18 */
    0x23,	/* 'P' 0x19 */
    0x21,	/* '[' 0x1a */
    0x1E,	/* ']' 0x1b */
    0x24,	/* ENTER 0x1c */
    0x3b,	/* CTRL */
    0x00,	/* 'A' 0x1e */
    0x01,	/* 'S' 0x1f */
    0x02,	/* 'D' 0x20 */
    0x03,	/* 'F' 0x21 */
    0x05,	/* 'G' 0x22 */
    0x04,	/* 'H' 0x23 */
    0x26,	/* 'J' 0x24 */
    0x28,	/* 'K' 0x25 */
    0x25,	/* 'L' 0x26 */
    0x29,	/* ';' 0x27 */
    0x27,	/* ''' 0x28 */
    0x32,	/* '`' 0x29 */
    0xFF,	/* unused 0x2a */
    0x2A,	/* '\' 0x2b */
    0x06,	/* 'Z' 0x2c */
    0x07,	/* 'X' 0x2d */
    0x08,	/* 'C' 0x2e */
    0x09,	/* 'V' 0x2f */
    0x0B,	/* 'B' 0x30 */
    0x2D,	/* 'N' 0x31 */
    0x2E,	/* 'M' 0x32 */
    0x2B,	/* ',' 0x33 */
    0x2F,	/* '.' 0x34 */
    0x2C,	/* '/' 0x35 */
    0xFF,	/* unused 0x36 */
    0x4B,	/* '*' 0x37 NUMERIC KEYPAD NOTE: '/' on Mac Keyboard*/
    0xFF,	/* unused 0x38 */
    0x31,	/* ' ' 0x39 */
    0xFF,	/* unused 0x3a */
    0xFF,	/* unused 0x3b */
    0xFF,	/* unused 0x3c */
    0xFF,	/* unused 0x3d */
    0xFF,	/* unused 0x3e */
    0xFF,	/* unused 0x3f */
    0xFF,	/* unused 0x40 */
    0xFF,	/* unused 0x41	*/
    0xFF,	/* unused 0x42 */
    0xFF,	/* unused 0x43 */
    0xFF,	/* unused 0x44 */
    0x47,	/* NUM-LOCK 0x45 NUMERIC KEYPAD NOTE: CLEAR on Mac Keyboard */
    0xFF,	/* unused 0x46 */
    0x59,	/* '7' 0x47 NUMERIC KEYPAD */
    0x5B,	/* '8' 0x48 NUMERIC KEYPAD */
    0x5C,	/* '9' 0x49 NUMERIC KEYPAD */
    0x43,	/* '-' 0x4a NUMERIC KEYPAD NOTE: '*' on Mac Keyboard */
    0x56,	/* '4' 0x4b NUMERIC KEYPAD */
    0x57,	/* '5' 0x4c NUMERIC KEYPAD */
    0x58,	/* '6' 0x4d NUMERIC KEYPAD */
    0x4E,	/* '+' 0x4e NUMERIC KEYPAD NOTE: '-' on Mac Keyboard */
    0x53,	/* '1' 0x4f NUMERIC KEYPAD */
    0x54,	/* '2' 0x50 NUMERIC KEYPAD */
    0x55,	/* '3' 0x51 NUMERIC KEYPAD */
    0x52,	/* '0' 0x52 NUMERIC KEYPAD */
    0x41,	/* '.' 0x53 NUMERIC KEYPAD */
    0xFF,	/* unused 0x54 */
    0xFF,	/* unused 0x55 */
    0xFF,	/* unused 0x56 */
    0xFF,	/* unused 0x57 */
    0xFF,	/* unused 0x58 */
    0xFF,	/* unused 0x59 */
    0xFF,	/* unused 0x5A */
    0xFF,	/* unused 0x5B */
    0xFF,	/* unused 0x5C */
    0xFF,	/* unused 0x5D */
    0xFF,	/* unused 0x5E */
    0xFF,	/* unused 0x5F */
    0xFF,	/* unused 0x60 */
    0xFF,	/* unused 0x61 */
    0x4C,	/* ENTER 0x62 NUMERIC KEYPAD */
    0x4B,	/* '/' 0x63 NUMERIC KEYPAD NOTE: '=' on Mac Keyboard */
    0x7E,	/* up arrow 0x64 NUMERIC KEYPAD */
    0x7D,	/* down arrow 0x65 NUMERIC KEYPAD */
    0x7B,	/* left arrow 0x66 NUMERIC KEYPAD */
    0x7C,	/* right arrow 0x67 NUMERIC KEYPAD */
  }
};

A1(PUBLIC, Executor::INTEGER, ROMlib_next_butmods_to_mac_butmods,
   NSEventModifierFlags, nextflags)
{
    INTEGER retval;
    
    retval = 0;
    if (nextflags & NSShiftKeyMask)
	retval |= shiftKey;
    if (nextflags & NSControlKeyMask)
	retval |= ControlKey;
    if (nextflags & NSAlternateKeyMask)
	retval |= optionKey;
    if (nextflags & NSCommandKeyMask)
	retval |= cmdKey;
    retval |= ROMlib_mods & btnState;
    return retval;
}

/*
 * NOTE:  We are probably not consistent with KeyTrans.
 */

namespace Executor {
  PRIVATE Executor::LONGINT next_keydata_to_mac_keydata(NSEvent*, boolean_t, unsigned char*);
}

A3(PRIVATE, Executor::LONGINT, next_keydata_to_mac_keydata, NSEvent *, neventp,
   boolean_t, down_p, unsigned char *, virtp)
{
  INTEGER macmodifiers;
  unsigned char charcode, keycode, rep;
  unsigned int next_flags;
  unichar thechar;

#define NEXT_DELETE	0x7F

  thechar = [[neventp characters] characterAtIndex:0];

  switch (thechar) {
    case NSUpArrowFunctionKey:
      thechar = ASCIIUPARROW;
      break;
    case NSDownArrowFunctionKey:
      thechar = ASCIIDOWNARROW;
      break;
    case NSLeftArrowFunctionKey:
      thechar = ASCIILEFTARROW;
      break;
    case NSRightArrowFunctionKey:
      thechar = ASCIIRIGHTARROW;
      break;
    default:
      break;
  }

  if (thechar == NEXT_DELETE)
	thechar = 8;	/* back space */

  rep = 0;
  if (thechar >= DIACREP1START && thechar < DIACREP1START + sizeof(diacrep1))
	rep = diacrep1[thechar - DIACREP1START];
  else if (thechar >= DIACREP2START && thechar < DIACREP2START + sizeof(diacrep2))
	rep = diacrep2[thechar - DIACREP2START];
  next_flags = [neventp modifierFlags];
  if (rep) {
	next_flags &= ~(NSShiftKeyMask | NSControlKeyMask | NSAlternateKeyMask |
                    NSCommandKeyMask);
	charcode = rep;
  } else {
	charcode = thechar;
  }
  keycode = ROMlib_keyboard_type ?
  next_virt_to_mac_virt[ROMlib_keyboard_type-1][[neventp keyCode]]
  :
  [neventp keyCode];
  macmodifiers = ROMlib_next_butmods_to_mac_butmods(next_flags);
  if ((macmodifiers & optionKey) && (macmodifiers & cmdKey))
    macmodifiers &= ~(cmdKey|optionKey);
  *virtp = keycode;
  return ROMlib_xlate(keycode & 0xff, macmodifiers, down_p);
}

#define SANE_DEBUGGING
#if defined (SANE_DEBUGGING)
static int sane_debugging_on = 0; /* Leave this off and let the person doing the
			      debugging turn it on if he/she wants.  If this
			      is set to non-zero, it breaks code.  Not a
			      very nice thing to do. */
#endif /* SANE_DEBUGGING */

A0 (PUBLIC, void, sendsuspendevent)
{
  Point p;

  if (printstate == __idle
      && (size_info.size_flags & SZacceptSuspendResumeEvents)
#if defined (SANE_DEBUGGING)
      && !sane_debugging_on
#endif /* SANE_DEBUGGING */
      )
      {
	p.h = CW(MouseLocation.h);
	p.v = CW(MouseLocation.v);
	ROMlib_PPostEvent(osEvt, SUSPENDRESUMEBITS|SUSPEND|CONVERTCLIPBOARD,
			  (HIDDEN_EvQElPtr *) 0, TickCount(), p, ROMlib_mods);
      }
}

A1 (PUBLIC, void, sendresumeevent, LONGINT, cvtclip)
{
  LONGINT what;
  Point p;

  if (printstate == __idle
#if defined (BINCOMPAT)
      && (size_info.size_flags & SZacceptSuspendResumeEvents)
#endif /* BINCOMPAT */
#if defined (SANE_DEBUGGING)
      && !sane_debugging_on
#endif /* SANE_DEBUGGING */
      )
    {
      what = SUSPENDRESUMEBITS | RESUME;
      if (cvtclip)
	what |= CONVERTCLIPBOARD;
      p.h = CW(MouseLocation.h);
      p.v = CW(MouseLocation.v);
      ROMlib_PPostEvent(osEvt, what, (HIDDEN_EvQElPtr *) 0, TickCount(),
			p, ROMlib_mods);
    }
}

A0(PUBLIC, void, sendcopy)
{
    Point p;

    p.h = CW(MouseLocation.h);
    p.v = CW(MouseLocation.v);
    ROMlib_PPostEvent(keyDown, 0x0863,	/* 0x63 == 'c' */
			        (HIDDEN_EvQElPtr *) 0, TickCount(), p, cmdKey|btnState);
    ROMlib_PPostEvent(keyUp, 0x0863,
			        (HIDDEN_EvQElPtr *) 0, TickCount(), p, cmdKey|btnState);
}

A0(PUBLIC, void, sendpaste)
{
    Point p;

    p.h = CW(MouseLocation.h);
    p.v = CW(MouseLocation.v);
    ROMlib_PPostEvent(keyDown, 0x0976,	/* 0x76 == 'v' */
				(HIDDEN_EvQElPtr *) 0, TickCount(), p, cmdKey|btnState);
    ROMlib_PPostEvent(keyUp, 0x0976,
				(HIDDEN_EvQElPtr *) 0, TickCount(), p, cmdKey|btnState);
}

PRIVATE void pinmouse(INTEGER *hp, INTEGER *vp, BOOLEAN swap)
{
    if (swap) {
	*hp = CW(*hp);
	*vp = CW(*vp);
    }

    if (*hp < 0)
	*hp = 0;
    else if (*hp > vdriver_width - 1)
	*hp = vdriver_width - 1;

    if (*vp < 0)
	*vp = 0;
    else if (*vp > vdriver_height - 1)
	*vp = vdriver_height - 1;

    if (swap) {
	*hp = CW(*hp);
	*vp = CW(*vp);
    }
}

A1(PUBLIC, void, ROMlib_updatemouselocation, NSEvent *, neventp) /* INTERNAL */
{
    MouseLocation.h = CW ([neventp locationInWindow].x);
    MouseLocation.v = CW (vdriver_height - [neventp locationInWindow].y);
    pinmouse(&MouseLocation.h, &MouseLocation.v, TRUE);
}

PRIVATE void
getwhere (Executor::Point *wherep, NSEvent *event)
{
  switch ([event type])
    {
    case NSLeftMouseDown:
    case NSRightMouseDown:
    case NSLeftMouseUp:
    case NSRightMouseUp:
      wherep->h = [event locationInWindow].x;
      wherep->v = vdriver_height - [event locationInWindow].y;
      break;
    default:
      {
	LONGINT x, y;
	querypointerX (&x, &y, NULL);
	wherep->h = x;
	wherep->v = y;
      }
    }
    pinmouse(&wherep->h, &wherep->v, FALSE);
}

A1(PUBLIC, void, postnextevent, NSEvent *, neventp)	/* INTERNAL */
{
    LONGINT when;
    Point where;
    INTEGER butmods;
    LONGINT keywhat;
    unsigned char virt;

    when    = TickCount();
    butmods = ROMlib_next_butmods_to_mac_butmods([neventp modifierFlags]);
    getwhere (&where, neventp);

    switch ([neventp type]) {
    case NSLeftMouseDown:
    case NSRightMouseDown:
	butmods &= ~btnState;
	ROMlib_PPostEvent(mouseDown, 0, (HIDDEN_EvQElPtr *) 0, when, where, butmods);
	break;
    case NSLeftMouseUp:
    case NSRightMouseUp:
	butmods |= btnState;
	ROMlib_PPostEvent(mouseUp, 0, (HIDDEN_EvQElPtr *) 0, when, where, butmods);
	break;
    case NSKeyDown:
#warning some sort of special translation needs to be done here

/* 
 * Actually, the above '\\' code was special because of the location of
 * the backslash key on the original NeXT keyboards.  We used to recognize
 * it and change things around, because some programs would get surprised
 * to find that backslash was a shifted key.
 *
 * The best solution is probably to have a way of sucking host keyboard
 * mappings into the virtual Mac on startup time *and* also giving people
 * the option of using their own modified KMAP and KCHR resources.  The
 * former will be goode enough for the vast majority of users, but the latter
 * will still conceivably need to be done for non-portable apps.
 */

	    keywhat = next_keydata_to_mac_keydata(neventp, TRUE, &virt);
	post_keytrans_key_events (keyDown, keywhat, when, where, butmods,
				  virt);
/*
 * NOTE: this sorry hack is here because I don't know how to receive key
 *	 up messages when the control key is down.
 */
	if (butmods & cmdKey)
	  {
	    keywhat = next_keydata_to_mac_keydata(neventp,  FALSE, &virt);
	    post_keytrans_key_events (keyUp, keywhat, when, where, butmods,
				      virt);
	  }
	break;
    case NSKeyUp:
#warning some sort of special translation needs to be done here
	    keywhat = next_keydata_to_mac_keydata(neventp, FALSE, &virt);
	post_keytrans_key_events (keyUp, keywhat, when, where, butmods, virt);
	break;
    default:
        break;
    }
}

namespace Executor {
  static LONGINT addtoop(char*, INTEGER, char**, char);
}

A4(PRIVATE, LONGINT, addtoop, char *, string, INTEGER, size, char **, op,
								    char, doit)
{
    if (!doit)
/*-->*/ return size;
    memmove(*op, string, size);
    *op += size;
    return size;
}

A2(PUBLIC, LONGINT, insertfonttbl, char **, op, char, doit)
{
    char numstr[12];
    Str255 str;
    INTEGER i, n, nres, shift;
    ResType t, restype;
    Handle h;
    LONGINT retval;

    retval = 0;

#define TABLESTART      "{\\fonttbl"
    retval += addtoop(TABLESTART, sizeof(TABLESTART) - 1, op, doit);
    restype = TICK("FONT");
    shift = TRUE;
    do {
	nres = CountResources(restype);
	for (n = 1; n <= nres; n++) {
	    h = GetIndResource(restype, n);
	    GetResInfo(h, &i, &t, str);
	    if (shift)
		i >>= 7;
	    if (str[0] && str[1] != '.' && str[1] != '%') {
		retval += addtoop("{\\f", 3, op, doit);
		sprintf(numstr, "%d", (LONGINT) i);
		retval += addtoop(numstr, strlen(numstr), op, doit);
		retval += addtoop("\\fxxx ", 6, op, doit);
		str[str[0] + 1] = 0;
		ROMlib_trytomatch((char *)str + 1, 0);
		retval += addtoop((char *) str + 1,
					    strlen((char *)str + 1), op, doit);
		/* TODO: get the name from str */
		retval += addtoop(";}", 2, op, doit);
	    }
	}
    } while (restype == TICK("FONT") && (restype = TICK("FOND")) &&
							     !(shift = FALSE));
#define TABLEEND        "}\\f0"
    retval += addtoop(TABLEEND, sizeof(TABLEEND) - 1, op, doit);
    return retval;
}
