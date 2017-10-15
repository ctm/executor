#if !defined (_DIALOG_H_)
#define _DIALOG_H_

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: DialogMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "WindowMgr.h"
#include "TextEdit.h"

namespace Executor {
#define ctrlItem	4
#define btnCtrl		0
#define chkCtrl		1
#define radCtrl		2
#define resCtrl		3
#define statText	8
#define editText	16
#define iconItem	32
#define picItem		64
#define userItem	0
#define itemDisable	128

#define OK		1
#define Cancel		2

#define stopIcon	0
#define noteIcon	1
#define cautionIcon	2

struct DialogRecord : GuestStruct
{
   WindowRecord window;
   //PACKED_MEMBER(Handle, items);
   //PACKED_MEMBER(TEHandle, textH);
   GUEST<Handle> items;
   GUEST<TEHandle> textH;
   GUEST<INTEGER> editField;
   INTEGER editOpen;
   INTEGER aDefItem;
};
typedef DialogRecord *DialogPeek;

typedef CWindowPtr CDialogPtr;
typedef WindowPtr DialogPtr;
typedef HIDDEN_WindowPtr HIDDEN_DialogPtr;

/* dialog record accessors */
#define DIALOG_WINDOW(dialog)		((WindowPtr) &((DialogPeek) dialog)->window)

#define DIALOG_ITEMS_X(dialog)		(((DialogPeek) (dialog))->items)
#define DIALOG_TEXTH_X(dialog)		(((DialogPeek) (dialog))->textH)
#define DIALOG_EDIT_FIELD_X(dialog)	(((DialogPeek) (dialog))->editField)
#define DIALOG_EDIT_OPEN_X(dialog)	(((DialogPeek) (dialog))->editOpen)
#define DIALOG_ADEF_ITEM_X(dialog)	(((DialogPeek) (dialog))->aDefItem)

#define DIALOG_ITEMS(dialog)		(MR (DIALOG_ITEMS_X (dialog)))
#define DIALOG_TEXTH(dialog)		(MR (DIALOG_TEXTH_X (dialog)))
#define DIALOG_EDIT_FIELD(dialog)	(CW (DIALOG_EDIT_FIELD_X (dialog)))
#define DIALOG_EDIT_OPEN(dialog)	(CW (DIALOG_EDIT_OPEN_X (dialog)))
#define DIALOG_ADEF_ITEM(dialog)	(CW (DIALOG_ADEF_ITEM_X (dialog)))

struct DialogTemplate : GuestStruct
{
  Rect boundsRect;
  GUEST<INTEGER> procID;
  GUEST<BOOLEAN> visible;
  GUEST<BOOLEAN> filler1;
  GUEST<BOOLEAN> goAwayFlag;
  GUEST<BOOLEAN> filler2;
  GUEST<LONGINT> refCon;
  GUEST<INTEGER> itemsID;
  GUEST<Str255> title;
};

typedef DialogTemplate *DialogTPtr;
MAKE_HIDDEN(DialogTPtr);
typedef HIDDEN_DialogTPtr *DialogTHndl;

// ### Struct needs manual conversion to GUEST<...>
//   unsigned boldItm4: 1;
typedef struct PACKED {
  unsigned boldItm4: 1;
  unsigned boxDrwn4: 1;
  unsigned sound4: 2;
  unsigned boldItm3: 1;
  unsigned boxDrwn3: 1;
  unsigned sound3: 2;
  unsigned boldItm2: 1;
  unsigned boxDrwn2: 1;
  unsigned sound2: 2;
  unsigned boldItm1: 1;
  unsigned boxDrwn1: 1;
  unsigned sound1: 2;
} StageList;

struct AlertTemplate : GuestStruct {
    GUEST< Rect> boundsRect;
    GUEST< INTEGER> itemsID;
    GUEST< StageList> stages;
};
typedef AlertTemplate *AlertTPtr;
MAKE_HIDDEN(AlertTPtr);
typedef HIDDEN_AlertTPtr *AlertTHndl;

#define overlayDITL		0
#define appendDITLRight 	1
#define appendDITLBottom	2

typedef int16 DITLMethod;

#define TEdoFont 	1
#define TEdoFace 	2
#define TEdoSize 	4
#define TEdoColor 	8
#define TEdoAll 	15

#define TEaddSize 	16

#define doBColor 	8192
#define doMode 		16384
#define doFontName 	32768

#if 0
#if !defined (ResumeProc_H)
extern HIDDEN_ProcPtr 	ResumeProc_H;
extern HIDDEN_ProcPtr 	DABeeper_H;
extern HIDDEN_Handle 	DAStrings_H[4];
extern INTEGER 	ANumber;
extern INTEGER 	ACount;
extern INTEGER 	DlgFont;
#endif

#define ResumeProc	(ResumeProc_H.p)
#define DABeeper	(DABeeper_H.p)
#endif

extern pascal trap INTEGER C_Alert( INTEGER id, 
 ProcPtr fp );
extern pascal trap INTEGER C_StopAlert( INTEGER id, 
 ProcPtr fp );
extern pascal trap INTEGER C_NoteAlert( INTEGER id, 
 ProcPtr fp );
extern pascal trap INTEGER C_CautionAlert( INTEGER id, 
 ProcPtr fp );
extern pascal trap void C_CouldAlert( INTEGER id );
extern pascal trap void C_FreeAlert( INTEGER id );
extern pascal trap void C_CouldDialog( INTEGER id );
extern pascal trap void C_FreeDialog( INTEGER id );
extern pascal trap DialogPtr C_NewDialog( Ptr dst, 
 Rect *r, StringPtr tit, BOOLEAN vis, INTEGER procid, 
 WindowPtr behind, BOOLEAN gaflag, LONGINT rc, Handle items );
extern pascal trap DialogPtr C_GetNewDialog( INTEGER id, 
 Ptr dst, WindowPtr behind );
extern pascal trap void C_CloseDialog( DialogPtr dp );
extern pascal trap void C_DisposDialog( DialogPtr dp );
extern pascal BOOLEAN C_ROMlib_myfilt( DialogPeek dp, EventRecord *evt, 
 INTEGER *ith );

extern pascal trap void C_ModalDialog( ProcPtr fp, 
 INTEGER *item );
extern pascal trap BOOLEAN C_IsDialogEvent( 
 EventRecord *evt );
extern pascal trap void C_DrawDialog( DialogPtr dp );
extern pascal trap INTEGER C_FindDItem( DialogPtr dp, 
 Point pt );
extern pascal trap void C_UpdtDialog( DialogPtr dp, 
 RgnHandle rgn );
extern pascal trap BOOLEAN C_DialogSelect( 
 EventRecord *evt, HIDDEN_DialogPtr *dpp, INTEGER *item );
extern void DlgCut( DialogPtr dp );
extern void DlgCopy( DialogPtr dp );
extern void DlgPaste( DialogPtr dp );
extern void DlgDelete( DialogPtr dp );
extern pascal void C_ROMlib_mysound( INTEGER i );
extern pascal trap void C_ErrorSound( ProcPtr sp );
extern pascal trap void C_InitDialogs( ProcPtr rp );
extern void SetDAFont( INTEGER i ); 
extern pascal trap void C_ParamText( StringPtr p0, 
 StringPtr p1, StringPtr p2, StringPtr p3 );
extern pascal trap void C_GetDItem( DialogPtr dp, 
 INTEGER itemno, INTEGER *itype, HIDDEN_Handle *item, Rect *r );
extern pascal trap void C_SetDItem( DialogPtr dp, 
 INTEGER itemno, INTEGER itype, Handle item, Rect *r );
extern pascal trap void C_GetIText( Handle item, 
 StringPtr text );
extern pascal trap void C_SetIText( Handle item, 
 StringPtr text );
extern pascal trap void C_SelIText( DialogPtr dp, 
 INTEGER itemno, INTEGER start, INTEGER stop );
extern INTEGER GetAlrtStage( void  ); 
extern void ResetAlrtStage( void  ); 
extern pascal trap void C_HideDItem( DialogPtr dp, 
 INTEGER item );
extern pascal trap void C_ShowDItem( DialogPtr dp, 
 INTEGER item );

extern pascal trap CDialogPtr C_NewCDialog (Ptr, Rect *, StringPtr, BOOLEAN, INTEGER, WindowPtr, BOOLEAN, LONGINT, Handle);

extern pascal trap OSErr C_GetStdFilterProc (ProcPtr *proc);
extern pascal trap OSErr C_SetDialogDefaultItem (DialogPtr dialog,
						 int16 new_item);
extern pascal trap OSErr C_SetDialogCancelItem (DialogPtr dialog, 
						int16 new_item);
extern pascal trap OSErr C_SetDialogTracksCursor (DialogPtr dialog,
						  Boolean tracks);

extern void AppendDITL (DialogPtr, Handle, DITLMethod);
extern void ShortenDITL (DialogPtr, int16);
extern int16 CountDITL (DialogPtr);
}

#endif /* _DIALOG_H_ */
