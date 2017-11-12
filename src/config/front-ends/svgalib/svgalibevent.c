/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_svgalibevent[] = "$Id: svgalibevent.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#include "OSEvent.h"
#include "EventMgr.h"
#include "ToolboxEvent.h"
#include "rsys/m68kint.h"
#include "rsys/vgavdriver.h"
#include "rsys/blockinterrupts.h"
#include "rsys/adb.h"
#include <vgakeyboard.h>
#include <vgamouse.h>
#include <limits.h>
#include <sys/wait.h>
#include "rsys/keyboard.h"
#include "rsys/keyboards.h"
#include "rsys/osevent.h"
#include "rsys/keyboards.h"
#include "rsys/sigio_multiplex.h"

/* File descriptors for the keyboard and mouse. */
static int keyboard_fd = -1;
static int mouse_fd = -1;

/* Save the original sigio info for keyboard and mouse, for shutdown. */
static int orig_keyboard_sigio_flag;
static int orig_keyboard_sigio_owner;
static int orig_mouse_sigio_flag;
static int orig_mouse_sigio_owner;

/* Mouse range is [0, max_mouse_x], [0, max_mouse_y] inclusive. */
static int max_mouse_x, max_mouse_y;

static INTEGER
compute_keyboard_mods(void)
{
    static const struct
    {
        int scancode;
        INTEGER mask;
    } mask_keys[] = {
#define SCANCODE_LEFTSHIFT 42
#define SCANCODE_RIGHTSHIFT 54
#define SCANCODE_CAPSLOCK 58
        { SCANCODE_LEFTSHIFT, shiftKey },
        { SCANCODE_RIGHTSHIFT, shiftKey },
        { SCANCODE_CONTROL, ControlKey },
        { SCANCODE_LEFTCONTROL, ControlKey },
        { SCANCODE_RIGHTCONTROL, ControlKey },
        { SCANCODE_LEFTALT, cmdKey },
        { SCANCODE_RIGHTALT, optionKey },
        { SCANCODE_CAPSLOCK, alphaLock },
    };
    const char *key_states;
    INTEGER mask;
    int i;

    key_states = keyboard_getstate();
    for(i = NELEM(mask_keys) - 1, mask = 0; i >= 0; i--)
        if(key_states[mask_keys[i].scancode])
            mask |= mask_keys[i].mask;

    return mask;
}

static LONGINT
svgalib_keydata_to_mac_keydata(int scancode, INTEGER mods, bool down_p,
                               unsigned char *virtp)
{
    unsigned char keycode;
    INTEGER keywhat;

    keycode = ibm_virt_to_mac_virt[scancode];
    *virtp = keycode;
    keywhat = ROMlib_xlate(keycode, mods, down_p);
    return keywhat;
}

static void
keyboard_handler(int scancode, int pressed_p)
{
    INTEGER button_mods;
    LONGINT keywhat;
    LONGINT when;
    char *key_states;
    Point where;
    unsigned char virt;

    when = TickCount();

    /* We have to keep track of this ourselves if we don't
   * use their default keyboard handler.
   */
    key_states = keyboard_getstate();
    key_states[scancode] = pressed_p;

    button_mods = compute_keyboard_mods() | (ROMlib_mods & btnState);
    ROMlib_mods = button_mods;
    where.h = CW(MouseLocation.h);
    where.v = CW(MouseLocation.v);
    keywhat = svgalib_keydata_to_mac_keydata(scancode, button_mods, pressed_p,
                                             &virt);

    if(keywhat != -1)
        post_keytrans_key_events(pressed_p ? keyDown : keyUp, keywhat, when,
                                 where, button_mods, virt);
}

static void
mouse_handler(int button, int dx, int dy)
{
    static int old_button = 0;
    int x, y;

    /* We only care about whether *any* button is depressed or not. */
    button = (button != 0);

    /* Compute the new position and pin it. */
    x = CW(MouseLocation.h) + dx;
    y = CW(MouseLocation.v) + dy;
    if(x < 0)
        x = 0;
    else if(x > max_mouse_x)
        x = max_mouse_x;
    if(y < 0)
        y = 0;
    else if(y > max_mouse_y)
        y = max_mouse_y;

    /* Record it in the appropriate low global. */
    MouseLocation.h = CW(x);
    MouseLocation.v = CW(y);

    /* Post a button event if its status changed. */
    if(button != old_button)
    {
        INTEGER button_mods;
        Point where;
        LONGINT when;

        when = TickCount();
        where.h = x;
        where.v = y;
        button_mods = ROMlib_mods;

        if(button)
        {
            button_mods &= ~btnState;
            ROMlib_PPostEvent(mouseDown, 0, NULL, when, where, button_mods);
        }
        else
        {
            button_mods |= btnState;
            ROMlib_PPostEvent(mouseUp, 0, NULL, when, where, button_mods);
        }

        old_button = button;
    }

    adb_apeiron_hack(true, dx, dy);
}

void event_shutdown(void)
{
    if(keyboard_fd >= 0 || mouse_fd >= 0)
    {
        /* no more sigio */
        signal(SIGIO, SIG_IGN);

        if(keyboard_fd >= 0)
        {
            fcntl(keyboard_fd, F_SETOWN, orig_keyboard_sigio_owner);
            fcntl(keyboard_fd, F_SETFL, orig_keyboard_sigio_flag);
            keyboard_close();
            keyboard_fd = -1;
        }

        if(mouse_fd >= 0)
        {
            fcntl(keyboard_fd, F_SETOWN, orig_mouse_sigio_owner);
            fcntl(keyboard_fd, F_SETFL, orig_mouse_sigio_flag);
            mouse_close();
            mouse_fd = -1;
        }
    }
}

static void
mouse_moved(int x, int y)
{
    virtual_int_state_t old_virt;

    /* Actually redraw the cursor. */
    old_virt = block_virtual_ints();
    vga_update_cursor(x, y);
    restore_virtual_ints(old_virt);
}

bool event_post_all_pending(void)
{
    int evt, found_keyboard_event, found_mouse_event;
    int swapped_old_x, swapped_old_y;

    /* Grab all keyboard events. */
    found_keyboard_event = 0;
    do
    {
        evt = keyboard_update();
        found_keyboard_event |= evt;
    } while(evt);

    /* Grab all mouse events. */
    found_mouse_event = 0;
    swapped_old_x = MouseLocation.h;
    swapped_old_y = MouseLocation.v;
    do
    {
        evt = mouse_update();
        found_mouse_event |= evt;
    } while(evt);

    /* If the mouse moved, redraw it. */
    if(swapped_old_x != MouseLocation.h || swapped_old_y != MouseLocation.v)
        mouse_moved(CW(MouseLocation.h), CW(MouseLocation.v));

    return (found_keyboard_event || found_mouse_event);
}

static syn68k_addr_t
post_pending_events(syn68k_addr_t interrupt_addr, void *unused)
{
    event_post_all_pending();
    return MAGIC_RTE_ADDRESS;
}

static void
sigio_handler(int signo)
{
    /* request syncint */
    cpu_state.interrupt_pending[M68K_EVENT_PRIORITY] = 1;
    cpu_state.interrupt_status_changed = INTERRUPT_STATUS_CHANGED;
}

static void
install_phoney_mouse_polling(void)
{
    struct sigaction action;
    struct itimerval itimer;

    action.sa_handler = sigio_handler;
    action.sa_flags = 0;
    action.sa_restorer = 0;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGVTALRM);
    sigaction(SIGVTALRM, &action, 0);

    itimer.it_value.tv_sec = 0;
    itimer.it_value.tv_usec = 1 /* 1000000 / 25 */;
    itimer.it_interval = itimer.it_value;
#if 0
  setitimer (ITIMER_VIRTUAL, &itimer, 0);
#endif
}

bool event_init(int screen_width, int screen_height)
{
    syn68k_addr_t event_callback;

    /* Set up keyboard. */
    keyboard_fd = keyboard_init_return_fd();
    if(keyboard_fd < 0)
        return false;

    /* Set up mouse. */
    mouse_fd = mouse_init_return_fd("", vga_getmousetype(),
                                    MOUSE_DEFAULTSAMPLERATE);
    if(mouse_fd < 0)
    {
        event_shutdown(); /* De-install keyboard handler. */
        return false;
    }

    /* Install default handlers. */
    keyboard_seteventhandler(keyboard_handler);
    mouse_seteventhandler(mouse_handler);

    /* Note the bounds on the mouse position. */
    max_mouse_x = screen_width - 1;
    max_mouse_y = screen_height - 1;

    /* Don't constrain the mouse's position. */
    mouse_setxrange(INT_MIN, INT_MAX);
    mouse_setyrange(INT_MIN, INT_MAX);
    mouse_setwrap(MOUSE_WRAP);
    mouse_setposition(0, 0);

    /* Set up keyboard for sigio. */
    sigio_multiplex_install_handler(keyboard_fd, sigio_handler);
    fcntl(keyboard_fd, F_GETOWN, &orig_keyboard_sigio_owner);
    fcntl(keyboard_fd, F_SETOWN, getpid());
    orig_keyboard_sigio_flag = fcntl(keyboard_fd, F_GETFL, 0);
    fcntl(keyboard_fd, F_SETFL, orig_keyboard_sigio_flag | FASYNC);

    /* Set up mouse for sigio. */
    sigio_multiplex_install_handler(mouse_fd, sigio_handler);
    fcntl(mouse_fd, F_GETOWN, &orig_mouse_sigio_owner);
    fcntl(mouse_fd, F_SETOWN, getpid());
    orig_mouse_sigio_flag = fcntl(mouse_fd, F_GETFL, 0);
    fcntl(mouse_fd, F_SETFL, orig_mouse_sigio_flag | FASYNC);
    sigio_multiplex_install_handler(mouse_fd, sigio_handler);

    /* hook into syn68k synchronous interrupts */
    event_callback = callback_install(post_pending_events, NULL);
    *(syn68k_addr_t *)SYN68K_TO_US(M68K_EVENT_VECTOR * 4) = CL(event_callback);

    install_phoney_mouse_polling();

    atexit(event_shutdown);
    return true;
}

/* Grabs the current mouse position. */
void querypointerX(LONGINT *xp, LONGINT *yp, LONGINT *notused)
{
    *xp = CW(MouseLocation.h);
    *yp = CW(MouseLocation.v);
}
