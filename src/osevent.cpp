/* Copyright 1986 - 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

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

#define NEVENT 20

static int nevent = 0;
static EvQEl evs[NEVENT], *freeelem = evs + NEVENT - 1;

#define ROMlib_curs LM(MouseLocation)

INTEGER Executor::ROMlib_mods = btnState;
static LONGINT autoticks;
static LONGINT lastdown = -1;

#if !defined(MACOSX_)
short ROMlib_pinned = false;
#else
short ROMlib_pinned = true;
#endif

static Ptr kchr_ptr;

void
Executor::invalidate_kchr_ptr(void)
{
    kchr_ptr = 0;
}

static INTEGER kchr_id = 0;

Ptr
Executor::ROMlib_kchr_ptr(void)
{
    if(!kchr_ptr)
    {
        TheZoneGuard guard(LM(SysZone));
        Handle kchr_hand;

        kchr_hand = GetResource(TICK("KCHR"), kchr_id);
        gui_assert(kchr_hand);
        LoadResource(kchr_hand);
        HLock(kchr_hand);
        kchr_ptr = STARH(kchr_hand);
    }
    return kchr_ptr;
}

bool
Executor::ROMlib_set_keyboard(const char *keyboardname)
{
    Handle new_h;

    TheZoneGuard guard(LM(SysZone));
    Str255 pkeyboardname;

    str255_from_c_string(pkeyboardname, keyboardname);
    new_h = GetNamedResource(TICK("KCHR"), pkeyboardname);
    if(new_h)
    {
        GUEST<INTEGER> tmpid;
        GetResInfo(new_h, &tmpid, 0, 0);
        kchr_id = CW(tmpid);
        LoadResource(new_h);
        if(kchr_ptr)
        {
            Handle kchr_hand;

            kchr_hand = RecoverHandle(kchr_ptr);
            HUnlock(kchr_hand);
        }
        HLock(new_h);
        kchr_ptr = STARH(new_h);
    }
    return !!new_h;
}

static bool map_right_to_left = true;

uint16_t
Executor::ROMlib_right_to_left_key_map(uint16_t what)
{
    uint16_t retval;

    retval = what;
    if(map_right_to_left)
        switch(what)
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

LONGINT
Executor::ROMlib_xlate(INTEGER virt, INTEGER modifiers, bool down_p)
{
    static uint16_t down_value[VIRT_MASK + 1];
    LONGINT retval;

    if(!down_p)
        retval = down_value[virt & VIRT_MASK];
    else
    {
        static LONGINT state;

        retval = KeyTrans(ROMlib_kchr_ptr(),
                          modifiers | (virt & VIRT_MASK), &state);
        down_value[virt & VIRT_MASK] = (retval >> 16
                                            ? retval >> 16
                                            : retval & 0xFFFF);
    }
    return retval;
}

void Executor::ROMlib_eventinit(bool graphics_valid_p) /* INTERNAL */
{
    static int beenhere = 0;
    EvQEl *p, *ep;

    if(!beenhere)
    {
        LM(MouseLocation).h = 0;
        LM(MouseLocation).v = 0;
        LM(MouseLocation2).h = 0;
        LM(MouseLocation2).v = 0;
        LM(ScrDmpEnb) = true;
        evs[0].qLink = 0; /* end of the line */
        beenhere = 1;
        for(p = evs + 1, ep = evs + NEVENT; p != ep; p++)
            p->qLink = RM((QElemPtr)(p - 1));
        LM(SysEvtMask) = CWC(~(1L << keyUp)); /* EVERYTHING except keyUp */
        if(graphics_valid_p)
        {
            Rect *main_gd_bounds;

            main_gd_bounds = &GD_BOUNDS(MR(LM(MainDevice)));
        }
    }
}

static void dropevent(EvQEl *);
static OSErrRET _PPostEvent(INTEGER evcode, LONGINT evmsg,
                            GUEST<EvQElPtr> *qelpp);
static BOOLEAN OSEventCommon(INTEGER evmask, EventRecord *eventp,
                             BOOLEAN dropit);

static void dropevent(EvQEl *qp)
{
    Dequeue((QElemPtr)qp, &LM(EventQueue));
    qp->qLink = RM((QElemPtr)freeelem);
    freeelem = qp;
    nevent--;
}

EvQEl *
Executor::geteventelem(void)
{
    EvQEl *retval = freeelem;

    if(nevent == NEVENT)
    {
        dropevent((EvQEl *)MR(LM(EventQueue).qHead));
        retval = freeelem;
    }
    freeelem = (EvQEl *)MR(freeelem->qLink);
    nevent++;
    return retval;
}

bool
Executor::ROMlib_get_index_and_bit(LONGINT loc, int *indexp, uint8 *bitp)
{
    bool retval;

    if(loc < 0 || loc / 8 >= sizeof_KeyMap)
        retval = false;
    else
    {
        retval = true;
        *indexp = loc / 8;
        *bitp = (1 << (loc % 8));
    }
    return retval;
}

void Executor::ROMlib_zapmap(LONGINT loc, LONGINT val)
{
    int i;
    uint8 bit;

    if(ROMlib_get_index_and_bit(loc, &i, &bit))
    {
        if(val)
            LM(KeyMap)[i] |= bit;
        else
            LM(KeyMap)[i] &= ~(bit);
    }
}

static bool
key_down(uint8 loc)
{
    bool retval;
    int i;
    uint8 bit;

    if(!ROMlib_get_index_and_bit(loc, &i, &bit))
        retval = false;
    else
        retval = !!(LM(KeyMap)[i] & bit);
    return retval;
}

OSErrRET Executor::PPostEvent(INTEGER evcode, LONGINT evmsg,
                              GUEST<EvQElPtr> *qelp) /* IMIV-85 */
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
    if(evcode == keyUp)
    {
        ROMlib_zapmap((evmsg >> 8) & 0xFF, 0);
        if(!(evmsg & 0xff))
        {
            if(qelp)
                *qelp = 0;
            return noErr;
        }
        lastdown = -1;
    }
    else if(evcode == keyDown)
    {
        ROMlib_zapmap((evmsg >> 8) & 0xFF, 1);
        if(!(evmsg & 0xff))
        {
            if(qelp)
                *qelp = 0;
            return noErr;
        }
        lastdown = evmsg;
        autoticks = tmpticks + Cx(LM(KeyThresh));

        if((evmsg & 0xff) == '2' && /* cmd-shift-2 */
           key_down(MKV_CLOVER) && (key_down(MKV_LEFTSHIFT) || key_down(MKV_RIGHTSHIFT)))

            dofloppymount();
    }

    if(!((1 << evcode) & Cx(LM(SysEvtMask))))
        /*-->*/ return evtNotEnb;
    qp = geteventelem();
    qp->evtQWhat = CW(evcode);
    qp->evtQMessage = CL(evmsg);
    qp->evtQWhen = CL(tmpticks);
    qp->evtQWhere = ROMlib_curs;
    qp->evtQModifiers = CW(ROMlib_mods);
    Enqueue((QElemPtr)qp, &LM(EventQueue));
    if(qelp)
        *qelp = RM(qp);
    return noErr;
}

static OSErrRET _PPostEvent(INTEGER evcode, LONGINT evmsg,
                            GUEST<EvQElPtr> *qelpp)
{
    OSErrRET ret;
    syn68k_addr_t proc;
    GUEST<EvQElPtr> retquelp;

    proc = ostraptable[0x2F];

#if 0 /* FIXME */
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

    if(qelpp)
        *qelpp = retquelp;
    return ret;
}

OSErrRET Executor::ROMlib_PPostEvent(INTEGER evcode, LONGINT evmsg,
                                     GUEST<EvQElPtr> *qelp, LONGINT when,
                                     Point where, INTEGER butmods)
{
    LM(MouseLocation2).h = ROMlib_curs.h = CW(where.h);
    LM(MouseLocation2).v = ROMlib_curs.v = CW(where.v);
    ROMlib_mods = butmods;

    return _PPostEvent(evcode, evmsg, qelp);
}

OSErrRET Executor::PostEvent(INTEGER evcode, LONGINT evmsg)
{
    return _PPostEvent(evcode, evmsg, (GUEST<EvQElPtr> *)0);
}

void Executor::FlushEvents(INTEGER evmask, INTEGER stopmask) /* II-69 */
{
    EvQEl *qp, *next;
    int x;
    virtual_int_state_t block;

    block = block_virtual_ints();
    for(qp = (EvQEl *)MR(LM(EventQueue).qHead);
        qp && !((x = 1 << Cx(qp->evtQWhat)) & stopmask); qp = next)
    {
        next = (EvQEl *)MR(qp->qLink); /* save before dropping event */
        if(x & evmask)
            dropevent(qp);
    }
    restore_virtual_ints(block);
    /* NOTE:  According to IMII-69 we should be leaving stuff in d0 */
}

BOOLEAN Executor::ROMlib_bewaremovement;

int Executor::ROMlib_refresh = 0;

static BOOLEAN OSEventCommon(INTEGER evmask, EventRecord *eventp,
                             BOOLEAN dropit)
{
    EvQEl *qp;
    virtual_int_state_t block;
    BOOLEAN retval;
    static Point oldpoint = { -1, -1 };
    LONGINT ticks;

    /* We tend to call this routine from various ROMlib modal loops, so this
     * is a good place to check for timer interrupts, etc. */
    check_virtual_interrupt();

    if(send_application_open_aevt_p
       && application_accepts_open_app_aevt_p)
    {
        ProcessSerialNumber psn;
        OSErr err;

        GetCurrentProcess(&psn);

        {
            AppleEvent *aevt = (AppleEvent *)alloca(sizeof *aevt);
            AEAddressDesc *target = (AEAddressDesc *)alloca(sizeof *target);

            AEDescList *list = (AEDescList *)alloca(sizeof *list);
            int16_t count;
            GUEST<int16_t> count_s, dummy;

            err = AECreateDesc(typeProcessSerialNumber,
                               (Ptr)&psn, sizeof psn, target);

            CountAppFiles(&dummy, &count_s);
            count = CW(count_s);

            if(count)
            {
                int i;

                err = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments,
                                         target,
                                         /* dummy */ -1, /* dummy */ -1,
                                         aevt);
                err = AECreateList(NULL, 0, false, list);

                for(i = 1; i <= count; i++)
                {
                    FSSpec spec;
                    AppFile file;

                    GetAppFiles(i, &file);

#if 0
		  fprintf (stderr, "%d:`%s'\n",
			   i,
			   TEMP_C_STRING_FROM_STR255 (file.fName));
#endif

                    FSMakeFSSpec(CW(file.vRefNum), 0, file.fName, &spec);

                    AEPutPtr(list, i, typeFSS, (Ptr)&spec, sizeof spec);
                }

                AEPutKeyDesc(aevt, keyDirectObject, list);

                AESend(aevt,
                       /* dummy */ NULL,
                       kAENoReply,
                       /* dummy */ -1, /* dummy */ -1,
                       NULL, NULL);
            }
            else
            {
                err = AECreateAppleEvent(kCoreEventClass, kAEOpenApplication,
                                         target,
                                         /* dummy */ -1, /* dummy */ -1,
                                         aevt);

                AESend(aevt, /* dummy */ NULL,
                       kAENoReply, /* dummy */ -1,
                       /* dummy */ -1, NULL, NULL);
            }

            send_application_open_aevt_p = false;
        }
    }

    eventp->message = CLC(0);
    ROMlib_memnomove_p = false; /* this is an icky hack needed for Excel */
    ticks = TickCount();

#if defined(X)
    /* if we are running on a version of linux that doesn't support
       SIGIO this will handle events (although not asynchronously) */

    if(x_event_pending_p())
        post_pending_x_events(/* dummy */ -1, /* dummy */ NULL);

#endif /* X */

#if defined(SDL)
    /* if we are running SDL with the event thread disabled... */
    handle_sdl_events(/* dummy */ -1, /* dummy */ NULL);

#endif /* SDL */

#if defined(CYGWIN32) && !defined(SDL)
    /* Run the Win32 event loop, since currently we don't do this
       in a separate thread. */
    process_win32_events();

#endif /* CYGWIN32 */

#ifdef VDRIVER_PUMP_EVENTS
    /* why oh why are there special cases for all those platforms above?
       The following call was introduced for the SDL2 front-end,
       which is newer than all of the above. */
    vdriver_pump_events();
#endif

    block = block_virtual_ints();
    for(qp = (EvQEl *)MR(LM(EventQueue).qHead); qp && !((1 << Cx(qp->evtQWhat)) & evmask);
        qp = (EvQEl *)MR(qp->qLink))
        ;
    if(qp)
    {
        *eventp = *(EventRecord *)(&qp->evtQWhat);
        if(dropit)
        {
            dropevent(qp);
        }
        retval = true;
    }
    else
    {
        eventp->when = CL(TickCount());

        {
#if defined(X) || defined(MACOSX_)
            if(!ROMlib_pinned)
            {
                LONGINT x, y;
                LONGINT newmods;

                querypointerX(&x, &y, &newmods);
                eventp->where.h = LM(MouseLocation2).h = ROMlib_curs.h = CW(x);
                eventp->where.v = LM(MouseLocation2).v = ROMlib_curs.v = CW(y);
            }
            else
#endif
                LM(MouseLocation2) = eventp->where = ROMlib_curs;
        }

        eventp->modifiers = CW(ROMlib_mods);
        if((evmask & autoKeyMask) && lastdown != -1 && ticks > autoticks)
        {
            autoticks = ticks + Cx(LM(KeyRepThresh));
            eventp->what = CWC(autoKey);
            eventp->message = CL(lastdown);
            retval = true;
        }
        else
        {
            eventp->what = CWC(nullEvent);
            retval = false;
        }
    }
    restore_virtual_ints(block);
    if(eventp->where.h.get() != oldpoint.h || eventp->where.v.get() != oldpoint.v)
    {
        oldpoint = eventp->where.get();
        if(ROMlib_bewaremovement)
        {
            ROMlib_showhidecursor();
            ROMlib_bewaremovement = false;
        }
    }
#if defined(MACOSX_)
    if(ROMlib_printtimeout < 0)
    { /* see MacViewClass.m */
        dirty_rect_update_screen();
        ROMlib_printtimeout = 1;
    }
    else
#endif
        if(ROMlib_when == WriteInOSEvent)
    {
        dirty_rect_update_screen();
    }
    else if(ROMlib_when == WriteAtEndOfTrap)
    {
        dirty_rect_update_screen();
    }

    return retval;
}

BOOLEANRET Executor::GetOSEvent(INTEGER evmask, EventRecord *eventp)
{
    return OSEventCommon(evmask, eventp, true);
}

BOOLEANRET Executor::OSEventAvail(INTEGER evmask, EventRecord *eventp)
{
    return OSEventCommon(evmask, eventp, false);
}

void Executor::SetEventMask(INTEGER evmask)
{
    LM(SysEvtMask) = CW(evmask);
}

QHdrPtr Executor::GetEvQHdr()
{
    return &LM(EventQueue);
}

void
Executor::post_keytrans_key_events(INTEGER evcode, LONGINT keywhat, int32_t when,
                                   Point where, uint16_t button_state, unsigned char virt)
{
    INTEGER first_key, second_key;

    first_key = keywhat >> 16;
    second_key = keywhat;

    if(first_key)
    {
        ROMlib_PPostEvent(evcode, (virt << 8) | first_key, 0, when, where,
                          button_state);
        if(second_key)
            ROMlib_PPostEvent(keyUp, (virt << 8) | first_key, 0, when, where,
                              button_state);
    }
    if(second_key || !first_key)
        ROMlib_PPostEvent(evcode, (virt << 8) | second_key, 0, when, where,
                          button_state);
}

static int
compare(const void *p1, const void *p2)
{
    int retval;

    retval = ROMlib_strcmp((const Byte *)p1, (const Byte *)p2);
    return retval;
}

void
Executor::display_keyboard_choices(void)
{
    INTEGER nres, i, nfound;
    unsigned char(*names)[256];

    vdriver_shutdown();
    printf("Available keyboard maps:\n");
    SetResLoad(false);
    nres = CountResources(TICK("KCHR"));
    names = (decltype(names))alloca(nres * sizeof(*names));
    nfound = 0;
    for(i = 1; i <= nres; ++i)
    {
        Handle h;

        h = GetIndResource(TICK("KCHR"), i);
        if(h)
        {
            GetResInfo(h, 0, 0, (StringPtr)names[nfound]);
            ++nfound;
        }
    }
    qsort(names, nfound, sizeof(names[0]), compare);

    for(i = 0; i < nfound; ++i)
        printf("%.*s\n", names[i][0], (char *)&names[i][1]);

    exit(0);
}

void
Executor::maybe_wait_for_keyup(void)
{
#if defined(SDL) && defined(CYGWIN32)
    /* Run SDL's event processor so that any pending events get
     sent to us instead of the Win32 print stuff.  Specifically
     we don't want to lose the key-up if someone hit <CR> to
     choose the default button in a dialog.  Losing the key-up
     can cause all sorts of trouble. */
    while(lastdown != -1)
        handle_sdl_events(0, NULL);
#endif
}
