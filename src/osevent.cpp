/* Copyright 1986 - 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_osevent[] =
	    "$Id: osevent.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in OSEvent.h (DO NOT DELETE THIS LINE) */

/*
 * really should be divided into two sections, just like main.c (that
 * is to say just like main.c should be, it isn't yet)
 */

#include "rsys/common.h"

#include "QuickDraw.h"
#include "MemoryMgr.h"
#include "CQuickDraw.h"
#include "ToolboxEvent.h"
#include "OSEvent.h"
#include "EventMgr.h"
#include "OSUtil.h"
#include "ResourceMgr.h"
#include "ProcessMgr.h"
#include "AppleEvents.h"

#include "rsys/cquick.h"
#include "rsys/mman.h"
#include "rsys/arrowkeys.h"
#include "rsys/notmac.h"
#include "rsys/blockinterrupts.h"
#include "rsys/time.h"
#include "rsys/prefs.h"
#include "rsys/vdriver.h"
#include "rsys/host.h"
#include "rsys/next.h"
#include "rsys/segment.h"
#include "rsys/toolevent.h"
#include "rsys/osevent.h"
#include "rsys/dirtyrect.h"
#include "rsys/stdfile.h"
#include "rsys/system_error.h"

#include "rsys/string.h"
#include "rsys/keyboard.h"

#undef MACOSX_
#if defined(MACOSX_)
#include "contextswitch.h"
#endif

#include "DialogMgr.h"
#include "SegmentLdr.h"

using namespace Executor;

#define NEVENT	20

PRIVATE int nevent = 0;
PRIVATE EvQEl evs[NEVENT], *freeelem = evs+NEVENT-1;

#define ROMlib_curs	MouseLocation

PUBLIC INTEGER Executor::ROMlib_mods = btnState;
PRIVATE LONGINT autoticks;
PRIVATE LONGINT lastdown = -1;

#if !defined(MACOSX_)
PUBLIC short ROMlib_pinned = false;
#else
PUBLIC short ROMlib_pinned = true;
#endif

PRIVATE Ptr kchr_ptr;

PUBLIC void
Executor::invalidate_kchr_ptr (void)
{
  kchr_ptr = 0;
}

PRIVATE INTEGER kchr_id = 0;

PUBLIC Ptr
Executor::ROMlib_kchr_ptr (void)
{
  if (!kchr_ptr)
    {
        TheZoneGuard guard(SysZone);
  	   Handle kchr_hand;

	   kchr_hand = GetResource (TICK ("KCHR"), kchr_id);
	   gui_assert (kchr_hand);
	   LoadResource (kchr_hand);
	   HLock (kchr_hand);
	   kchr_ptr = STARH (kchr_hand);
    }
  return kchr_ptr;
}

PUBLIC bool
Executor::ROMlib_set_keyboard (const char *keyboardname)
{
  Handle new_h;

   TheZoneGuard guard(SysZone);
         Str255 pkeyboardname;

       str255_from_c_string (pkeyboardname, keyboardname);
       new_h = GetNamedResource (TICK ("KCHR"), pkeyboardname);
       if (new_h)
	 {
           GUEST<INTEGER> tmpid;
	   GetResInfo (new_h, &tmpid, 0, 0);
	   kchr_id = CW (tmpid);
	   LoadResource (new_h);
	   if (kchr_ptr)
	     {
	       Handle kchr_hand;

	       kchr_hand = RecoverHandle (kchr_ptr);
	       HUnlock (kchr_hand);
	     }
	   HLock (new_h);
	   kchr_ptr = STARH (new_h);
	 }
  return !!new_h;
}

PRIVATE bool map_right_to_left = true;

PUBLIC uint16
Executor::ROMlib_right_to_left_key_map (uint16 what)
{
  uint16 retval;

  retval = what;
  if (map_right_to_left)
    switch (what)
      {
      default:
	break;
      case MKV_RIGHTSHIFT:
	retval = MKV_LEFTSHIFT;
	break;
      case MKV_RIGHTOPTION:
	retval = MKV_LEFTOPTION;
	break;
      case MKV_RIGHTCNTL:
	retval = MKV_LEFTCNTL;
	break;
      }
  return retval;
}

/*
 * NOTE: we figure out the value for a down keystroke, then we just remember
 * what we figured out and return that value on the up.  This probably isn't
 * how the Mac does it, but it's probably close enough.  Largely this
 * routine is just a wrapper for KeyTrans now.  See IMV for an explanation
 * of what's going on here.
 */

PUBLIC LONGINT
Executor::ROMlib_xlate (INTEGER virt, INTEGER modifiers, bool down_p)
{
  static uint16 down_value[VIRT_MASK + 1];
  LONGINT retval;
  
  if (!down_p)
    retval = down_value[virt & VIRT_MASK];
  else
    {
      static LONGINT state;
      
      retval = KeyTrans (ROMlib_kchr_ptr (),
			 modifiers | (virt & VIRT_MASK), &state);
      down_value[virt & VIRT_MASK] = (retval >> 16
				      ? retval >> 16
				      : retval & 0xFFFF);
    }
  return retval;
}

char ROMlib_started;	/* flag used by Mac frontend */

A1(PUBLIC, void, ROMlib_eventinit, bool, graphics_valid_p)	/* INTERNAL */
{
    static int beenhere = 0;
    EvQEl *p, *ep;

    if (!beenhere)
      {
	MouseLocation.h = 0;
	MouseLocation.v = 0;
	MouseLocation2.h = 0;
	MouseLocation2.v = 0;
	ScrDmpEnb = true;
	evs[0].qLink = 0;	/* end of the line */
	beenhere = 1;
	for (p = evs+1, ep = evs+NEVENT; p != ep; p++)
	  p->qLink = RM((QElemPtr) (p-1));
	SysEvtMask = CWC(~(1L<< keyUp)); /* EVERYTHING except keyUp */
	ROMlib_started = 3;
	if (graphics_valid_p)
	  {
	    Rect *main_gd_bounds;
	    
	    main_gd_bounds = &GD_BOUNDS (MR (MainDevice));
#if defined (MSDOS)
	    init_dos_events (CW (main_gd_bounds->right),
			     CW (main_gd_bounds->bottom));
#elif defined (EVENT_SVGALIB)
	    if (!event_init (CW (main_gd_bounds->right),
			     CW (main_gd_bounds->bottom)))
	      {
		fprintf (stderr,
		  "Unable to initialize svgalib events.\n"
		  "Make sure that executor-svga is installed setuid root,\n"
		  "that libvga.config has your mouse properly identified\n"
		  "and that /dev/mouse is a symlink to the right place\n");
		exit (-1);
	      }
#endif
	  }
      }
}

namespace Executor {
  PRIVATE void dropevent(EvQEl*);
  PRIVATE OSErrRET _PPostEvent(INTEGER evcode,
							   LONGINT evmsg, GUEST<EvQElPtr> *qelpp);
  PRIVATE BOOLEAN OSEventCommon(INTEGER evmask, EventRecord *eventp,
								BOOLEAN dropit);
}

A1(PRIVATE, void, dropevent, EvQEl *, qp)
{
    Dequeue((QElemPtr) qp, &EventQueue);
    qp->qLink = RM((QElemPtr) freeelem);
    freeelem = qp;
    nevent--;
}

EvQEl *
Executor::geteventelem (void)
{
    EvQEl *retval = freeelem;

    if (nevent == NEVENT) {
	dropevent((EvQEl *) MR(EventQueue.qHead));
	retval = freeelem;
    }
    freeelem = (EvQEl *) MR(freeelem->qLink);
    nevent++;
    return retval;
}

PUBLIC bool
Executor::ROMlib_get_index_and_bit (LONGINT loc, int *indexp, uint8 *bitp)
{
  bool retval;

  if (loc < 0 || loc / 8 >= sizeof_KeyMap)
    retval = false;
  else
    {
      retval = true;
      *indexp = loc / 8;
      *bitp = (1 << (loc % 8));
    }
  return retval;
}

A2(PUBLIC, void, ROMlib_zapmap, LONGINT, loc, LONGINT, val)
{
  int i;
  uint8 bit;

  if (ROMlib_get_index_and_bit (loc, &i, &bit))
    {
      if (val)
	KeyMap[i] |= bit;
      else
	KeyMap[i] &= ~(bit);
    }
}

PRIVATE bool
key_down (uint8 loc)
{
  bool retval;
  int i;
  uint8 bit;

  if (!ROMlib_get_index_and_bit (loc, &i, &bit))
    retval = false;
  else
    retval = !!(KeyMap[i] & bit);
  return retval;
}

A3(PUBLIC trap, OSErrRET, PPostEvent, INTEGER, evcode,		/* IMIV-85 */
				      LONGINT, evmsg, GUEST<EvQElPtr> *, qelp)
{
    EvQEl *qp;
    LONGINT tmpticks;

    /*
     * Here is where the portable autokey stuff should go
     * If it is a keyUp event, clear autoticks, if it is
     * a keyDown then set the appropriate bugger ...
     *
     * Now that the portable code is here, SCO stuff is out of date
     */

    /*
     * NOTE:  The code below isn't strictly correct, since IMI-260 says
     * you can have at most 2 non modifier keys down at one time.
     */

    tmpticks = TickCount();
    if (evcode == keyUp) {
	ROMlib_zapmap((evmsg >> 8) & 0xFF, 0);
	if (!(evmsg & 0xff))
	  {
	    if (qelp)
	      *qelp = 0;
	    return noErr;
	  }
	lastdown = -1;
    } else if (evcode == keyDown) {
	ROMlib_zapmap((evmsg >> 8) & 0xFF, 1);
	if (!(evmsg & 0xff))
	  {
	    if (qelp)
	      *qelp = 0;
	    return noErr;
	  }
	lastdown = evmsg;
	autoticks = tmpticks + Cx(KeyThresh);

	if ((evmsg & 0xff) == '2' && /* cmd-shift-2 */
	    key_down (MKV_CLOVER) &&
	    (key_down (MKV_LEFTSHIFT) || key_down (MKV_RIGHTSHIFT)))
	
	  dofloppymount();
    }

    if (!((1 << evcode)&Cx(SysEvtMask)))
/*-->*/	return evtNotEnb;
    qp = geteventelem();
    qp->evtQWhat      = CW(evcode);
    qp->evtQMessage   = CL(evmsg);
    qp->evtQWhen      = CL(tmpticks);
    qp->evtQWhere     = ROMlib_curs;
    qp->evtQModifiers = CW(ROMlib_mods);
    Enqueue((QElemPtr) qp, &EventQueue);
    if (qelp)
	*qelp = RM(qp);
    return noErr;
}

A3(PRIVATE, OSErrRET, _PPostEvent, INTEGER, evcode,
				      LONGINT, evmsg, GUEST<EvQElPtr> *, qelpp)
{
    OSErrRET ret;
    syn68k_addr_t proc;
    GUEST<EvQElPtr> retquelp;

    proc = ostraptable[0x2F];

#if 0	/* FIXME */
    if (proc == osstuff[0x2F].orig)
#endif
	ret = PPostEvent(evcode, evmsg, &retquelp);
#if 0
    else {
	EM_A0 = evcode;
	EM_D0 = evmsg;
	CALL_EMULATOR((syn68k_addr_t) proc);
	retquelp = EM_A0;
	ret = EM_D0;
    }
#endif

    if (qelpp)
	*qelpp = retquelp;
    return ret;
}

A6(PUBLIC, OSErrRET, ROMlib_PPostEvent, INTEGER, evcode, LONGINT, evmsg,
	GUEST<EvQElPtr> *, qelp, LONGINT, when, Point, where, INTEGER, butmods)
{
    MouseLocation2.h = ROMlib_curs.h = CW(where.h);
    MouseLocation2.v = ROMlib_curs.v = CW(where.v);
    ROMlib_mods = butmods;

    return _PPostEvent(evcode, evmsg, qelp);
}

A2(PUBLIC trap, OSErrRET, PostEvent, INTEGER, evcode, LONGINT, evmsg)
{
    return _PPostEvent(evcode, evmsg, (GUEST<EvQElPtr> *) 0);
}

A2(PUBLIC trap, void, FlushEvents, INTEGER, evmask,
					    INTEGER, stopmask)	/* II-69 */
{
    EvQEl *qp, *next;
    int x;
    virtual_int_state_t block;

    block = block_virtual_ints ();
    for (qp = (EvQEl *) MR(EventQueue.qHead);
	    qp && !((x=1<<Cx(qp->evtQWhat))&stopmask); qp = next) {
	next = (EvQEl *) MR(qp->qLink);	/* save before dropping event */
	if (x & evmask)
	    dropevent(qp);
    }
    restore_virtual_ints (block);
    /* NOTE:  According to IMII-69 we should be leaving stuff in d0 */
}

PUBLIC BOOLEAN Executor::ROMlib_bewaremovement;

PUBLIC int Executor::ROMlib_refresh = 0;


A3(PRIVATE, BOOLEAN, OSEventCommon, INTEGER, evmask, EventRecord *, eventp,
							    BOOLEAN, dropit)
{
    EvQEl *qp;
    virtual_int_state_t block;
    BOOLEAN retval;
    static Point oldpoint = { -1, -1 };
    LONGINT ticks;

    /* We tend to call this routine from various ROMlib modal loops, so this
     * is a good place to check for timer interrupts, etc. */
    check_virtual_interrupt ();
    
    if (send_application_open_aevt_p
	&& application_accepts_open_app_aevt_p)
      {
	ProcessSerialNumber psn;
	OSErr err;
	
	GetCurrentProcess (&psn);
	
	{
	  AppleEvent *aevt = (AppleEvent *)alloca (sizeof *aevt);
	  AEAddressDesc *target = (AEAddressDesc *)alloca (sizeof *target);
	  
	  AEDescList *list = (AEDescList *)alloca (sizeof *list);
          int16 count;
          GUEST<int16> count_s, dummy;
	  
	  err = AECreateDesc (typeProcessSerialNumber,
			      (Ptr) &psn, sizeof psn, target);
	  
	  CountAppFiles (&dummy, &count_s);
	  count = CW (count_s);
	  
	  if (count)
	    {
	      int i;
	      
	      err = AECreateAppleEvent (kCoreEventClass, kAEOpenDocuments,
					target,
					/* dummy */ -1, /* dummy */ -1,
					aevt);
	      err = AECreateList (NULL, 0, false, list);
	      
	      for (i = 1; i <= count; i ++)
		{
		  FSSpec spec;
		  AppFile file;
		  
		  GetAppFiles (i, &file);
		  
#if 0
		  fprintf (stderr, "%d:`%s'\n",
			   i,
			   TEMP_C_STRING_FROM_STR255 (file.fName));
#endif
		  
		  FSMakeFSSpec (CW (file.vRefNum), 0, file.fName, &spec);
		  
		  AEPutPtr (list, i, typeFSS, (Ptr) &spec, sizeof spec);
		}
	      
	      AEPutKeyDesc (aevt, keyDirectObject, list);
	      
	      AESend (aevt,
		      /* dummy */ NULL,
		      kAENoReply,
		      /* dummy */ -1, /* dummy */ -1,
		      NULL, NULL);
	    }
	  else
	    {
	      err = AECreateAppleEvent (kCoreEventClass, kAEOpenApplication,
					target,
					/* dummy */ -1, /* dummy */ -1,
					aevt);
	      
	      AESend (aevt, /* dummy */ NULL,
		      kAENoReply, /* dummy */ -1,
		      /* dummy */ -1, NULL, NULL);
	    }
	  
	  send_application_open_aevt_p = false;
	}
      }
    
    eventp->message = CLC(0);
    ROMlib_memnomove_p = false;	/* this is an icky hack needed for Excel */
    ticks = TickCount();

#if defined (X)
    /* if we are running on a version of linux that doesn't support
       SIGIO this will handle events (although not asynchronously) */
    
    if (x_event_pending_p ())
      post_pending_x_events (/* dummy */ -1, /* dummy */ NULL);

#endif /* X */

#if defined (SDL)
    /* if we are running SDL with the event thread disabled... */
    handle_sdl_events (/* dummy */ -1, /* dummy */ NULL);

#endif /* SDL */

#if defined(CYGWIN32) && !defined(SDL)
    /* Run the Win32 event loop, since currently we don't do this
       in a separate thread. */
    process_win32_events();

#endif /* CYGWIN32 */

    block = block_virtual_ints ();
    for (qp = (EvQEl *) MR(EventQueue.qHead); qp && !((1<<Cx(qp->evtQWhat))&evmask) ;
						    qp = (EvQEl *) MR(qp->qLink))
	;
    if (qp) {
	*eventp = *(EventRecord *)(&qp->evtQWhat);
	if (dropit) {
	    dropevent(qp);
	}
	retval = true;
    } else {
	eventp->when      = CL(TickCount());

	{
#if defined(X) || defined(MACOSX_)
	    if (!ROMlib_pinned) {
	    	LONGINT x, y;
		LONGINT newmods;

		querypointerX(&x, &y, &newmods);
		eventp->where.h   = MouseLocation2.h = ROMlib_curs.h = CW(x);
		eventp->where.v   = MouseLocation2.v = ROMlib_curs.v = CW(y);
	    } else
#endif
		MouseLocation2 = eventp->where = ROMlib_curs;
	}

#if defined (MSDOS) || defined (EVENT_SVGALIB)
	{
	  LONGINT x, y;

	  querypointerX (&x, &y, NULL);
	  eventp->where.h = MouseLocation2.h = ROMlib_curs.h = CW (x);
	  eventp->where.v = MouseLocation2.v = ROMlib_curs.v = CW (y);
	}
#endif

	eventp->modifiers = CW(ROMlib_mods);
	if ((evmask & autoKeyMask) && lastdown != -1 && ticks > autoticks) {
	    autoticks = ticks + Cx(KeyRepThresh);
	    eventp->what = CWC(autoKey);
	    eventp->message = CL(lastdown);
	    retval = true;
	} else {
	    eventp->what = CWC(nullEvent);
	    retval = false;
	}
    }
    restore_virtual_ints (block);
    if (eventp->where.h.get() != oldpoint.h || eventp->where.v.get() != oldpoint.v) {
	oldpoint = eventp->where.get();
	if (ROMlib_bewaremovement) {
	    ROMlib_showhidecursor();
	    ROMlib_bewaremovement = false;
	}
    }
#if defined(MACOSX_)
    if (ROMlib_printtimeout < 0) {	/* see MacViewClass.m */
	dirty_rect_update_screen ();
	ROMlib_printtimeout = 1;
    } else
#endif
    if (ROMlib_when == WriteInOSEvent) {
	dirty_rect_update_screen ();
    } else if (ROMlib_when == WriteAtEndOfTrap) {
	dirty_rect_update_screen ();
    }

    return retval;
}


A2(PUBLIC trap, BOOLEANRET, GetOSEvent, INTEGER, evmask, EventRecord *, eventp)
{
    return OSEventCommon(evmask, eventp, true);
}

A2(PUBLIC trap, BOOLEANRET, OSEventAvail, INTEGER, evmask,
							 EventRecord *, eventp)
{
    return OSEventCommon(evmask, eventp, false);
}

A1(PUBLIC trap, void, SetEventMask, INTEGER, evmask)
{
    SysEvtMask = CW(evmask);
}

A0(PUBLIC, QHdrPtr, GetEvQHdr)
{
    return &EventQueue;
}

PUBLIC void
Executor::post_keytrans_key_events (INTEGER evcode, LONGINT keywhat, int32 when,
			  Point where, uint16 button_state, unsigned char virt)
{
  INTEGER first_key, second_key;

  first_key = keywhat >> 16;
  second_key = keywhat;

  if (first_key)
    {
      ROMlib_PPostEvent (evcode, (virt << 8) | first_key, 0, when, where,
			 button_state);
      if (second_key)
	ROMlib_PPostEvent (keyUp, (virt << 8) | first_key, 0, when, where,
			   button_state);
    }
  if (second_key || !first_key)
    ROMlib_PPostEvent (evcode, (virt << 8) | second_key, 0, when, where,
		       button_state);
}

PRIVATE int
compare (const void *p1, const void *p2)
{
  int retval;

  retval = ROMlib_strcmp ((const Byte*)p1, (const Byte*)p2);
  return retval;
}

PUBLIC void
Executor::display_keyboard_choices (void)
{
  INTEGER nres, i, nfound;
  unsigned char (*names)[256];

  vdriver_shutdown ();
  printf ("Available keyboard maps:\n");
  SetResLoad (false);
  nres = CountResources (TICK ("KCHR"));
  names = (decltype(names))alloca (nres * sizeof (*names));
  nfound = 0;
  for (i = 1; i <= nres; ++i)
    {
      Handle h;
      
      h = GetIndResource (TICK ("KCHR"), i);
      if (h)
	{
	  GetResInfo (h, 0, 0, (StringPtr) names[nfound]);
	  ++nfound;
	}
    }
  qsort (names, nfound, sizeof (names[0]), compare);

  for (i = 0; i < nfound; ++i)
    printf ("%.*s\n", names[i][0], (char *) &names[i][1]);

  exit (0);
}

PUBLIC void
Executor::maybe_wait_for_keyup (void)
{
#if defined (SDL) && defined (CYGWIN32)
  /* Run SDL's event processor so that any pending events get
     sent to us instead of the Win32 print stuff.  Specifically
     we don't want to lose the key-up if someone hit <CR> to
     choose the default button in a dialog.  Losing the key-up
     can cause all sorts of trouble. */
  while (lastdown != -1)
    handle_sdl_events (0, NULL);
#endif
}
