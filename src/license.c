/* Copyright 1994 - 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_license[] =
		"$Id: license.c 137 2006-07-10 21:25:07Z ctm $";
#endif

#include "rsys/common.h"

#include "DialogMgr.h"
#include "OSUtil.h"
#include "SysErr.h"
#include "ResourceMgr.h"
#include "BinaryDecimal.h"
#include "SegmentLdr.h"
#include "MemoryMgr.h"

#include "rsys/arrowkeys.h"
#include "rsys/aboutpanel.h"
#include "rsys/license.h"
#include "rsys/keycode.h"
#include "rsys/pstuff.h"
#include "rsys/next.h"
#include "rsys/licensetext.h"
#include "rsys/segment.h"
#include "rsys/vdriver.h"
#include "rsys/custom.h"
#include "rsys/string.h"
#include "rsys/osevent.h"

which_license_t ROMlib_dolicense = no_license;

typedef enum
{
    REGISTER_OK_ITEM = 1,
    REGISTER_CANCEL_ITEM,
    REGISTER_NAME_ITEM,
    REGISTER_ORGANIZATION_ITEM,
    REGISTER_SN_ITEM,
    REGISTER_KEY_ITEM,
    REGISTER_MESSAGE_ITEM,
    REGISTER_NAME_LABEL,
    REGISTER_ORGANIZATION_LABEL,
    REGISTER_SN_LABEL,
    REGISTER_KEY_LABEL,
    REGISTER_COPYRIGHT_NOTICE,
    REGISTER_INSTRUCTIONS,
    REGISTER_THANK_YOU,
} register_items_t;

typedef enum
{
    LICENSE_NEXT_ITEM = 1,
    LICENSE_HEADING_ITEM,
    LICENSE_BODY_ITEM,
    LICENSE_OK_ITEM,
    LICENSE_PREVIOUS_ITEM,
} license_items_t;

typedef enum
{
    REGISTER_DIAL_ID= -4065,
} license_id_t;

typedef enum { dim, undim } operation_t;

PRIVATE void modify_item(operation_t op, DialogPtr dp, license_items_t item)
{
    HIDDEN_Handle h;
    INTEGER type;
    Rect r;
    BOOLEAN need_to_change;

    GetDItem(dp, item, &type, &h, &r);
    h.p = MR(h.p);
    need_to_change = FALSE;
    switch (op) {
    case dim:
	if (!(type & CWC(itemDisable))) {
	    need_to_change = TRUE;
	    type |= CWC(itemDisable);
	    HiliteControl((ControlHandle) h.p, 255);
	}
	break;
    case undim:
	if (type & CWC(itemDisable)) {
	    need_to_change = TRUE;
	    type &= ~CWC(itemDisable);
	    HiliteControl((ControlHandle) h.p, 0);
	}
	break;
    }
    if (need_to_change)
	SetDItem(dp, item, CW(type), h.p, &r);
}

PRIVATE void
right_justify (DialogPtr dp, INTEGER item)
{
  INTEGER type;
  HIDDEN_Handle h;
  Rect r;
  int len;
  enum { JUSTIFY_SLOP = 6 };

  GetDItem (dp, item, &type, &h, &r);
  h.p = MR (h.p);
  len = TextWidth (STARH (h.p), 0, GetHandleSize (h.p));
  r.left = CW (CW (r.right) - len - JUSTIFY_SLOP);
  SetDItem (dp, item, CW(type), h.p, &r);
  InvalRect (&r);
}

const char *updatetext =
	 "Number of CPUs: xxx\nLast free update: yyy.zzz\nExpires: www/vvv";

PRIVATE void updatemessage(DialogPtr dp, INTEGER valid_p,
			   decoded_info_t *infop)
{
    HIDDEN_Handle h;
    INTEGER type;
    Rect r;
    Str255 s;
    INTEGER pos;

    if (valid_p)
	modify_item(undim, dp, REGISTER_OK_ITEM);
/* TODO: Hilight ok button */
    else
	modify_item(dim, dp, REGISTER_OK_ITEM);

    GetDItem(dp, REGISTER_MESSAGE_ITEM, &type, &h, &r);
    h.p = MR(h.p);
    if (!infop)
      {
#if !defined (RELEASE_COMMERCIAL)
        pos = 1 + sprintf((char *) s + 1, "This is a demo version");
#else
	pos = 0;
#endif
	s[0] = pos;
    	SetIText(h.p, s);
      }
    else
      {
        pos = 1;
        pos += sprintf((char *) s+pos,
		       "Number of CPUs: %ld\r", (long) infop->n_cpu);
	if (!ROMlib_creatorsp)
	  {
	    pos += sprintf((char *) s+pos,
			   "Last free update: %d", infop->major_revision);
	    pos += sprintf((char *) s+pos,
			   ".%s\r", infop->updates_p ? "*" : "0");
	  }
	if (infop->expires_p)
	    pos += sprintf((char *) s+pos, "Expires: %d/%d", infop->last_month,
	    			 infop->last_year);
        s[0] = pos;
	SetIText(h.p, s);
      }
      InvalRect(&r);

    {
      StringPtr sp;
      Str255 str;

      GetDItem(dp, REGISTER_COPYRIGHT_NOTICE, &type, &h, &r);
      h.p = MR(h.p);
      if (!ROMlib_copyright_infop)
	sp = (StringPtr) "\073Copyright \251 1986-2006 "
	  "Abacus Research and Development, Inc.";
      else
	{
	  str255_from_c_string (str, (char *) ROMlib_copyright_infop->chars);
          sp = str;
	}
      SetIText(h.p, sp);
      InvalRect(&r);
    }
    if (ROMlib_thank_you_infop)
      {
	Str255 str;

	GetDItem(dp, REGISTER_THANK_YOU, &type, &h, &r);
	h.p = MR(h.p);
	str255_from_c_string (str, (char *) ROMlib_thank_you_infop->chars);
	SetIText(h.p, str);
	InvalRect(&r);
      }
    if (ROMlib_registration_instructionsp)
      {
	Str255 str;

	GetDItem(dp, REGISTER_INSTRUCTIONS, &type, &h, &r);
	h.p = MR(h.p);
	str255_from_c_string (str, 
			      (char *) ROMlib_registration_instructionsp->chars);
	SetIText(h.p, str);
	InvalRect(&r);
      }
    right_justify (dp, REGISTER_NAME_LABEL);
    right_justify (dp, REGISTER_ORGANIZATION_LABEL);
    right_justify (dp, REGISTER_SN_LABEL);
    right_justify (dp, REGISTER_KEY_LABEL);
}

#define KEYLENGTH 13
PRIVATE INTEGER checkkeychange(DialogPtr dp, EventRecord *ev, char ch)
{
    TEHandle teh;

    switch(ch)
      {
	case '\177': case '\010': case '\t':
	case ASCIILEFTARROW: case ASCIIRIGHTARROW: case ASCIIUPARROW:
	case ASCIIDOWNARROW:
/*--*/    return FALSE;
	  break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case 'a': case 'b': case 'c': case 'd': case 'e':
	case 'f': case 'g': case 'h': case 'i': case 'j':
	case 'k': case 'l': case 'm': case 'n': case 'o':
	case 'p': case 'q': case 'r': case 's': case 't':
	case 'u': case 'v': case 'w': case 'x': case 'y':
	case 'z':
	  break;
	default:
	  ParamText((StringPtr) "\077Authentication Keys contain only digits and lower-case letters.",
	 	     0, 0, 0);
	  CautionAlert(GENERIC_COMPLAINT_ID, (ProcPtr) 0);
	  return TRUE;
        break;
      }

    teh = MR(((DialogPeek) dp)->textH);
    if (Hx(teh, teLength) >= KEYLENGTH && Hx(teh, selEnd) == Hx(teh, selStart))
      {
/* FIXME: 13 is a constant in the text below. */
	  ParamText((StringPtr) "\041Keys are only 13 characters long.", 0, 0, 0);
	  CautionAlert(GENERIC_COMPLAINT_ID, (ProcPtr) 0);
	  return TRUE;
      }
    return FALSE;
}


#define SNLENGTH 10
PRIVATE INTEGER checksnchange(DialogPtr dp, EventRecord *ev, char ch)
{
    TEHandle teh;

    switch(ch)
      {
	case '\177': case '\010': case '\t':
	case ASCIILEFTARROW: case ASCIIRIGHTARROW: case ASCIIUPARROW:
	case ASCIIDOWNARROW:
/*--*/    return FALSE;
	  break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	  break;
	default:
	  ParamText((StringPtr) "\043Serial Numbers contain only digits.",
		    0, 0, 0);
	  CautionAlert(GENERIC_COMPLAINT_ID, (ProcPtr) 0);
	  return TRUE;
        break;
      }
    teh = MR(((DialogPeek) dp)->textH);
    if (Hx(teh, teLength) >= SNLENGTH && Hx(teh, selEnd) == Hx(teh, selStart))
      {
/* FIXME: 10 is a constant in the text below. */
	  ParamText((StringPtr) "\051Serial Numbers are at most "
		    "10 digits long.", 0, 0, 0);
	  CautionAlert(GENERIC_COMPLAINT_ID, (ProcPtr) 0);
	  return TRUE;
      }
    return FALSE;
}

P3(PUBLIC, pascal INTEGER, ROMlib_licensefilt, DialogPeek, dp,
   EventRecord *, ev, INTEGER *, ith)
{
    char ch;
    TEHandle teh;

  switch (ev->what)
    {
      case CWC(keyDown):
      case CWC(autoKey):
	ch = CL(ev->message) & 0xFF;
	teh = MR(dp->textH);
	if (ev->modifiers & CW(cmdKey))
	  {
	    switch(ch) {
		case 'c':
		    TECopy(teh);
		    break;
		case 'v':
		    TEPaste(teh);
#if 0 /* TODO */
/* FIXME */
		    checkvalue(? ? ?);
#endif /* 0 */
		    break;
		case 'x':
		    TECut(teh);
		    break;
		default:
		    SysBeep(1);
		    break;
	    }
/*--*/	    return -1;
	  }
	else
	  {
	    if (ch == '\n' || ch == '\r' || ch == NUMPAD_ENTER)
	      {
		*ith = CWC(REGISTER_OK_ITEM);
/*-->*/		return -1;
    	      }
	    switch (CW(dp->editField) + 1)
	      {
	        case REGISTER_SN_ITEM:
		  return checksnchange((DialogPtr) dp, ev, ch);
	          break;
	        case REGISTER_KEY_ITEM:
		  return checkkeychange((DialogPtr) dp, ev, ch);
	          break;
	      }
	  }
      case CWC(updateEvt):
	ROMlib_circledefault((DialogPtr)dp);
        break;
    }
  return FALSE;
}

PRIVATE void get_dp_item_text(DialogPtr dp, INTEGER item, Str255 retstring)
{
  HIDDEN_Handle h;
  INTEGER type;
  Rect r;

  GetDItem(dp, item, &type, &h, &r);
  GetIText(MR(h.p), retstring);
}

PRIVATE int validcode(DialogPtr dp, decoded_info_t *info, Str255 userkey)
{
    LONGINT sn;
    Str255 s;

    get_dp_item_text(dp, REGISTER_SN_ITEM, s);
    StringToNum(s, &sn);
    if ( decode(userkey + 1, info) && (info->serial_number == sn))
        return TRUE;
    else
        return FALSE;
}

PRIVATE INTEGER validexit(DialogPtr dp, INTEGER ihit, INTEGER valid_p)
{
    Str255 s;

    if (ihit == CWC(REGISTER_CANCEL_ITEM))
/*-->*/ return TRUE;
    else if (ihit == CWC(REGISTER_OK_ITEM))
      {
        if (!valid_p)
/*-->*/   return FALSE;
	get_dp_item_text(dp, REGISTER_NAME_ITEM, s);
	if (s[0] < 1)
	  {
	    ParamText((StringPtr) "\027Please enter your name.", 0, 0, 0);
	    StopAlert(GENERIC_COMPLAINT_ID, (ProcPtr) 0);
/*-->*/     return FALSE;
	  }
        else
/*-->*/   return TRUE;
      }
    return FALSE;
}

PRIVATE void SetI_c_string(Handle h, const char *name)
{
  Str255 s;

  s[0] = strlen(name);
  memcpy(s+1, name, s[0]);
  SetIText(h, s);
}

PRIVATE void lockinfo(DialogPtr dp, LONGINT sn, const char *name,
		      const char *organization)
{
    HIDDEN_Handle h;
    INTEGER type;
    Rect r;
    Str255 s;

    GetDItem(dp, REGISTER_SN_ITEM, &type, &h, &r);
    h.p = MR(h.p);
    NumToString(sn, s);
    SetIText(h.p, s);
    SetDItem(dp, REGISTER_SN_ITEM, statText | itemDisable, h.p, &r);

    GetDItem(dp, REGISTER_NAME_ITEM, &type, &h, &r);
    h.p = MR(h.p);
    SetI_c_string(h.p, name);
    SetDItem(dp, REGISTER_NAME_ITEM, statText | itemDisable, h.p, &r);

    GetDItem(dp, REGISTER_ORGANIZATION_ITEM, &type, &h, &r);
    h.p = MR(h.p);
    SetI_c_string(h.p, organization);
    SetDItem(dp, REGISTER_ORGANIZATION_ITEM, statText | itemDisable, h.p, &r);
}

PRIVATE void clear_item_text(DialogPtr dp, INTEGER itemno)
{
    HIDDEN_Handle h;
    INTEGER type;
    Rect r;
    
    GetDItem(dp, itemno, &type, &h, &r);
    h.p = MR(h.p);
    SetIText(h.p, (StringPtr) "");
    SetDItem(dp, itemno, CW(type), h.p, &r);
}

PRIVATE void get_dp_item_c_string(DialogPtr dp, INTEGER item, char *stringp,
				  int string_length)
{
  Str255 temp_text;

  get_dp_item_text(dp, item, temp_text);
  if (temp_text[0] >= string_length)
    temp_text[0] = string_length-1;

  /* FIXME - use a subroutine to convert pascal string to C string */
  strncpy(stringp, (char *) temp_text + 1, temp_text[0]);
  stringp[temp_text[0]] = 0;
}

PRIVATE void doregistration( void )
{
  DialogPtr dp;
  INTEGER ihit;
  Str255 userkey;
  decoded_info_t info, default_info, *default_infop;
  INTEGER valid_p;
  INTEGER start_sel;
  
  dp = GetNewDialog(REGISTER_DIAL_ID, (Ptr) 0, (WindowPtr) -1);

  default_infop = 0;
  if (ROMlib_info.serialnumber)
    {
      clear_item_text(dp, REGISTER_KEY_ITEM);
      start_sel = REGISTER_KEY_ITEM;
      lockinfo(dp, ROMlib_info.serialnumber, ROMlib_info.name,
	       ROMlib_info.organization);
      if ( decode((unsigned char *) ROMlib_info.key, &default_info)
	  && (default_info.serial_number == ROMlib_info.serialnumber))
	default_infop = &default_info;
    }
  else
   {
      clear_item_text(dp, REGISTER_NAME_ITEM);
      clear_item_text(dp, REGISTER_ORGANIZATION_ITEM);
      clear_item_text(dp, REGISTER_SN_ITEM);
      clear_item_text(dp, REGISTER_KEY_ITEM);
      start_sel = REGISTER_NAME_ITEM;
    }
  SelIText(dp, start_sel, 0, 32767);

  valid_p = FALSE;
  modify_item(dim, dp, REGISTER_OK_ITEM);
  ROMlib_circledefault(dp);
  ShowWindow(dp);
  SetPort(dp);

  updatemessage(dp, FALSE, default_infop);
  do
    {
      ModalDialog((ProcPtr) P_ROMlib_licensefilt, &ihit);
      get_dp_item_text(dp, REGISTER_KEY_ITEM, userkey);
/* these next two lines won't be needed once the filterproc is added */
      if (userkey[0] == 255)
	userkey[0] = 254;
      userkey[userkey[0] + 1] = 0;
      if (valid_key_format(userkey + 1) && validcode(dp, &info, userkey))
	{
	  if (!valid_p)
	    {
	      updatemessage(dp, TRUE, &info);
	      valid_p = TRUE;
	    }
	}
      else
	{
	  if (valid_p)
	    {
	      updatemessage(dp, FALSE, default_infop);
	      valid_p = FALSE;
	    }
	}
    }
  while (!validexit(dp, ihit, valid_p));
  if (ihit != CWC(REGISTER_OK_ITEM))
    {
      ROMlib_exit = 1;
      C_ExitToShell ();
    }
  else
    {
      char temp_name[81];
      char temp_org[81];
      char temp_key[14]; /* FIXME hardcoded numbers */


#if 0
#if !defined(NEXTSTEP)
      ParamText("\070Executor will now exit and be registered when restarted.",
		0, 0, 0);
#else
      ParamText("\072Executor will now exit, then you must run it once as root.",
		0, 0, 0);
#endif
#endif
      if (!ROMlib_restart_stringp)
	ParamText((StringPtr) "\070Executor will now exit and "
		  "be registered when restarted.",
		  0, 0, 0);
      else
	{
	  Str255 str;

	  str255_from_c_string (str, (char *) ROMlib_restart_stringp->chars);
	  ParamText(str, 0, 0, 0);
	}

      NoteAlert(GENERIC_COMPLAINT_ID, (ProcPtr) 0);
      get_dp_item_c_string(dp, REGISTER_NAME_ITEM,
			   temp_name, sizeof(temp_name));

      get_dp_item_c_string(dp, REGISTER_ORGANIZATION_ITEM,
			   temp_org,  sizeof(temp_org));


      get_dp_item_c_string(dp, REGISTER_KEY_ITEM,
			   temp_key,  sizeof(temp_key));

      ROMlib_info.serialnumber = info.serial_number;
      ROMlib_writenameorgkey(temp_name, temp_org, temp_key);
    }
  DisposDialog(dp);
}

PUBLIC void dolicense( void )
{
  if (ROMlib_dolicense != no_license)
    {
      if (WWExist != EXIST_YES)
	{
	  SysBeep (5);
	  vdriver_shutdown ();
	  fprintf (stderr, "Error: could not display registration panel!\n");
	  ROMlib_exit = 1;
	  C_ExitToShell ();
	}
      else if (ROMlib_dolicense == register_only)
	{
	  doregistration ();
	  ROMlib_exit = 1;
	}
    }
}
