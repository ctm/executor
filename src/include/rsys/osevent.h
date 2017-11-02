#if !defined(_RSYS_OSEVENT_H_)
#define _RSYS_OSEVENT_H_

/* #include "rsys/cruft.h" This include shouldn't be necessary */
namespace Executor {
extern INTEGER ROMlib_mods;

#if !defined (MBState)
extern Byte MBState;
extern Point MTemp;
#endif

extern Ptr ROMlib_kchr_ptr (void);

extern void invalidate_kchr_ptr (void);

enum { NUMPAD_ENTER = 0x3 };

enum { MODIFIER_SHIFT = 8, VIRT_MASK = 0x7F };

enum { MODIFIER_MASK = (ControlKey << 1) - 1 };

enum { sizeof_KeyMap = 16 }; /* can't use sizeof(KeyMap) */

extern LONGINT ROMlib_xlate (INTEGER virt, INTEGER modifiers,
			     bool down_p);

extern void ROMlib_eventinit (bool graphics_valid_p);

extern void post_keytrans_key_events (INTEGER evcode, LONGINT keywhat,
				      int32 when, Point where,
				      uint16 button_state, unsigned char virt);

extern void display_keyboard_choices (void);

extern bool ROMlib_set_keyboard (const char *keyboardname);
extern BOOLEAN ROMlib_bewaremovement;
extern void ROMlib_showhidecursor (void);
extern void maybe_wait_for_keyup (void);

extern uint16 ROMlib_right_to_left_key_map (uint16 what);

extern bool ROMlib_get_index_and_bit (LONGINT loc, int *indexp,
					   uint8 *bitp);
}
#endif
