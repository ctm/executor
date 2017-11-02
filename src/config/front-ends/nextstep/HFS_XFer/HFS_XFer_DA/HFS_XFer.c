#include "rsys/common.h"
true

#if		defined(UNIX)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <libc.h>
#include <signal.h>
#include "fs.h"
#include <assert.h>
#endif	/* !defined(MAC) */

#include <DeviceMgr.h>
#include <DeskMgr.h>
#include	<SetUpA4.h>
	 
#define LONGTIME				216000			/* one hour */

func ap_funcs[1];

static init_ap_funcs( void )
{
	ap_funcs[0].name = "\pAbout HFS_XFer...";
    ap_funcs[0].ptr  = about_HFS_XFer;
}

funcinfo file_funcs[] = {
	 0, 0, 110, 30, 0,
	 0, 0,  28, 30, 0,
	 0, 0,  28, 30, 0,
	 0, 0,  28, 40, 0,
	 0, 0,  28, 40, 0,
	 0, 0,  28, 40, 0,
	 0, 0,   0,  0, 0,
	 0, 0,   0,  0, 0
};

static init_file_funcs( void )
{
    file_funcs[0].name = "\pCopy Disk...";
    file_funcs[0].dlgHook = copydiskhook;
    file_funcs[0].prompt = "\pCopy Disk";
    
    file_funcs[1].name = "\pMove Files...";
    file_funcs[1].dlgHook = movefileshook;
    file_funcs[1].prompt = "\pMove File or Folder";
    
    file_funcs[2].name = "\pCopy Files...";
    file_funcs[2].dlgHook = copyfileshook;
    file_funcs[2].prompt = "\pCopy File or Folder";
    
    file_funcs[3].name = "\pRename...";
    file_funcs[3].dlgHook = renamefileshook;
    file_funcs[3].prompt = "\pRename File or Folder";
    
    file_funcs[4].name = "\pDelete...";
    file_funcs[4].dlgHook = deletefileshook;
    file_funcs[4].prompt = "\pDelete File or Folder";
    
    file_funcs[5].name = "\pNew Folder...";
    file_funcs[5].dlgHook = newdirhook;
    file_funcs[5].prompt = "\pCreate New Folder";
    
    file_funcs[6].name = "\p(-";
    file_funcs[6].prompt = "";
    
    file_funcs[7].name = "\pQuit/Q";
    file_funcs[7].prompt = "";
}
	
func edit_funcs[4];

void init_edit_funcs( void )
{
	edit_funcs[0].name = "\p(Undo/Z";
	edit_funcs[1].name = "\p(Cut/X";
	edit_funcs[2].name = "\p(Copy/C";
	edit_funcs[3].name = "\p(Paste/V";
}

void init_all_tables( void )
{
    init_ap_funcs();
    init_file_funcs();
    init_edit_funcs();
}

MenuHandle drvrmenu, filemenu, editmenu, optionmenu;
LONGINT destdir;
INTEGER destdisk, verify_flags;
SFReply globalreply;
WindowPtr destdirwin;
TEHandle destdirname;

int abortflag = 0;

void doerror(OSErr errno, char * s)
{
	static errortable errormessages[] = {
		wPrErr, 		"\pThat volume is locked.",
		vLckdErr,		"\pThat volume is locked.",
		permErr,		"\pPermission error writing to locked file.",
		ioErr,			"\pI/O Error."
	};

	INTEGER i;
	Str255 s2;

	if (abortflag)
/*-->*/ return;

	for (i = 0 ; i < NELEM(errormessages) && errormessages[i].number != errno
																		; i++)
		;
	if ( i < NELEM(errormessages) ) {
		ParamText((StringPtr) errormessages[i].message, 0, 0, 0);
		abortflag = StopAlert(ONEPARAMABORTALERT, (ProcPtr) 0) == ABORTITEM;
	} else {
		NumToString((LONGINT)errno, s2);
		ParamText(s2, (StringPtr) s, 0, 0);
		abortflag = CautionAlert(DOERRORABORTALERT, (ProcPtr) 0) == ABORTITEM;
	}
}

void about_HFS_XFer()
{
	Alert(ABOUTALERT, (ProcPtr) 0);
}

INTEGER quitfunc(int rn)
{
	CloseDeskAcc(rn);
	return 0;
}

BOOLEAN caneject(DialogPtr dp)
{
	Rect r;
	INTEGER type;
	Handle h;
	
	GetDItem(dp, getEject, &type, &h, &r);
	return !(*(ControlHandle) h)->contrlHilite;
}

static int strlen(char *p)
{
    register int retval;
    
    for (retval = 0; *p++; ++retval)
        ;
    return retval;
}

void setcurdestname(INTEGER disk, LONGINT dir)
{
	CInfoPBRec hpb;
	Str255 s;
	INTEGER id;
	Ptr ptr;
	
	hpb.dirInfo.ioNamePtr = s;
	hpb.dirInfo.ioVRefNum = disk;
	hpb.dirInfo.ioFDirIndex = -1;
	hpb.dirInfo.ioDrDirID = dir;
	TESetText((Ptr) 0, 0, destdirname);
	do {
		xPBGetCatInfo(&hpb, false);
		/* insert name : */
		TEInsert((Ptr) s+1, (LONGINT) s[0], destdirname);
		TEInsert((Ptr) ":", 1, destdirname);
		TESetSelect(0, 0, destdirname);
		id = hpb.dirInfo.ioDrDirID;
		hpb.dirInfo.ioDrDirID = hpb.dirInfo.ioDrParID;
	} while (id != 2);
	TESetSelect(32767, 32767, destdirname);
	TESelView(destdirname);
/* TODO: change this to a drawtext */
	ptr = (Ptr)"Current Destination";
	TextFace(bold);
	MoveTo((destdirwin->portRect.right + destdirwin->portRect.left -
			TextWidth(ptr, 0, strlen((char *)ptr)))/2, 12);
	DrawText(ptr, 0, strlen((char *)ptr));
}

void updatedestwin(INTEGER disk, LONGINT dir)
{
	GrafPtr saveport;

	GetPort(&saveport);
	SetPort(destdirwin);
	BeginUpdate(destdirwin);
	EraseRect(&destdirwin->portRect);
	setcurdestname(disk, dir);
	TEUpdate(&destdirwin->portRect, destdirname);
	EndUpdate(destdirwin);
	SetPort(saveport);
}

pascal BOOLEAN visiblefilesonly(ParmBlkPtr p)
{
	BOOLEAN retval;

	retval = !!(p->fileParam.ioFlFndrInfo.fdFlags & fInvisible);
	return retval;
}

pascal BOOLEAN myfilterproc(DialogPtr dp, EventRecord *event, INTEGER *item)
{
	Rect r;
	INTEGER type;
	Handle h;
	Str255 s;

	SetUpA4();
	GetDItem(dp, TEXTITEM, &type, &h, &r);
	if(!(type & itemDisable)) {
		GetIText(h, s); 
		GetDItem(dp, ACTIONBUTTON, &type, &h, &r);
		if (s[0] == 0)
			HiliteControl( (ControlHandle)h, 255);
		else
			HiliteControl( (ControlHandle)h, 0);
	}
	GetDItem(dp, getOpen, &type, &h, &r);
	if (globalreply.fType != 0)
		HiliteControl( (ControlHandle)h, 0);
	else
		HiliteControl( (ControlHandle)h, 255);
	switch(event->what) {
	case keyDown:
		if ((event->message & 0xFF) == '\r' && globalreply.fType != 0) {
			if (((DialogPeek)dp)->editField == -1)
				*item = getOpen;
			else
				*item = ACTIONBUTTON;
			RestoreA4();
/*-->*/		return TRUE;
		} else if ((event->message & 0xFF) == '.' &&
												(event->modifiers & cmdKey)) {
			*item = Cancel;
			RestoreA4();
/*-->*/		return TRUE;
		}
		break;
	case updateEvt:
		if ((WindowPtr)event->message == destdirwin)
			updatedestwin(-destdisk, destdir);
		break;
	}
	RestoreA4();
	return false;
}

INTEGER dodelete(DialogPtr dp)
{
	Str255 sp;
	LONGINT fromdirid;
	
	getnameandfromdirid(&sp, &fromdirid);
	delete1file(-SFSaveDisk, fromdirid, sp);
	return 101;
}

INTEGER dorename(DialogPtr dp)
{
	renamefile(dp);
	return 101;
}

INTEGER docopy(DialogPtr dp)
{
	dotransfer(copy1file);
	return 0;
}

INTEGER domove(DialogPtr dp)
{
	dotransfer(move1file);
	return 101;
}

void invaldestwin()
{
	GrafPtr saveport;

	GetPort(&saveport);
	SetPort(destdirwin);
	InvalRect(&destdirwin->portRect);
	SetPort(saveport);
}

pascal Boolean dirsfilter(DialogPtr dp, EventRecord *ep, INTEGER *itemhit)
{
	static INTEGER currentdisk;

	SetUpA4();
	if (currentdisk != SFSaveDisk) {
		invaldestwin();
		currentdisk = SFSaveDisk;
	}
	switch(ep->what) {
	case keyDown:
		if ((ep->message & 0xFF) == '\r') {
			*itemhit = getOpen;
			RestoreA4();
/*-->*/		return TRUE;
		}
		break;
	case nullEvent:
		*itemhit = 100;
		RestoreA4();
/*-->*/ return TRUE;
	case updateEvt:
		if ((WindowPtr)ep->message == destdirwin) {
			updatedestwin(-SFSaveDisk, CurDirStore);
			currentdisk = SFSaveDisk;
		}
		break;
	}
	RestoreA4();
	return false;
}

pascal INTEGER DirDlgHook(INTEGER itemhit, DialogPtr dp)
{
	static INTEGER needupdate = TRUE;

	SetUpA4();
	if (needupdate) {
		invaldestwin();
		needupdate = false;
	}
	if (itemhit != 100)
		needupdate = TRUE;
	RestoreA4();
	if (itemhit == SELECTBUTTON)
		return getOpen;
	else
		return itemhit;
}

void getnewdest()
{
	SFReply reply;
	SFTypeList tl;
	LONGINT savedestdir, dir;
	INTEGER disk, savedestdisk;
	Point where;
	HParamBlockRec pb;

	disk = SFSaveDisk;
	dir = CurDirStore;
	savedestdisk = SFSaveDisk = destdisk;
	savedestdir = CurDirStore = destdir;
	
	tl[0] = 0x7B2A265E;
	tl[1] = 0x00000000;
	tl[2] = 0x00000000;
	tl[3] = 0x00000000;
	SetPt(&where, 82, 40);
	SFPGetFile(where, (StringPtr) "\pSelect Directory",
			(ProcPtr) visiblefilesonly, 1, tl, (ProcPtr) DirDlgHook, &reply,
											DIRSONLYDLG, (ProcPtr) dirsfilter);
	if (reply.good) {
		destdisk = SFSaveDisk;
		destdir = CurDirStore;
	} else {
		pb.volumeParam.ioVolIndex = 0;
		pb.volumeParam.ioNamePtr = 0;
		pb.volumeParam.ioCompletion = 0;
		pb.volumeParam.ioVRefNum = -savedestdisk;
		if (PBHGetVInfo(&pb, false)==noErr && pb.volumeParam.ioVDrvInfo != 0) {
			destdisk = savedestdisk;
			destdir = savedestdir;
			invaldestwin();
		} else {
			destdisk = SFSaveDisk;
			destdir = CurDirStore;
		}
	}
	SFSaveDisk = disk;
	CurDirStore = dir;
}

INTEGER hookcommon(INTEGER item, DialogPtr dp, INTEGER (*pp)(DialogPtr dp))
{
	INTEGER retval;
	static INTEGER needtoupdatedestdisk;
	
	if (needtoupdatedestdisk) {
		needtoupdatedestdisk = false;
		destdisk = SFSaveDisk;
		destdir = CurDirStore;
		invaldestwin();
	}
	retval = 0;
	switch(item) {
	case -1:
		invaldestwin();
		break;
	case ACTIONBUTTON:
		retval = (*pp)(dp);
		break;
	case NEWDESTBUTTON:
		getnewdest();
		break;
	case getOpen:
		retval = 100;
		break;
	case getEject:
		if (destdisk == SFSaveDisk)
			needtoupdatedestdisk = TRUE;
		break;
	default:
		break;
	}
	return retval ? retval : item;
}

void hideitem(DialogPtr dp, INTEGER item)
{
	Handle h;
	Rect r;
	INTEGER type;
	
	GetDItem(dp, item, &type, &h, &r);
	if (type & ctrlItem) {
		HideControl((ControlHandle) h);
/*-->*/ return;
	}
	if (type == editText) {
		((DialogPeek)dp)->editField = -1;
		SizeWindow(dp, dp->portRect.right - dp->portRect.left, 220, TRUE);
	}
	OffsetRect(&r, 8000, 8000);
	type += itemDisable;
	SetDItem(dp, item, type, h, &r);
}

void setactionbutton(DialogPtr dp, Str255 title)
{
	Handle h;
	Rect r;
	INTEGER type;
	
	GetDItem(dp, ACTIONBUTTON, &type, &h, &r);
	SetCTitle((ControlHandle) h, title);
}

void myShowWindow(WindowPtr wp)
{
	BringToFront(wp);
	ShowWindow(wp);
}

pascal INTEGER copyfileshook(INTEGER item, DialogPtr dp)
{
	INTEGER retval;
	
	SetUpA4();
	abortflag = false;
	if (item == -1) {
		myShowWindow(destdirwin);
		hideitem(dp, TEXTITEM);
		setactionbutton(dp, (StringPtr)"\pCopy");
	}
	retval = hookcommon(item, dp, docopy);
	RestoreA4();
	return retval;
}

pascal INTEGER copydiskhook(INTEGER item, DialogPtr dp)
{
    INTEGER retval;
    
    SetUpA4();
    abortflag = false;
	if (item == -1) {
		myShowWindow(destdirwin);
		SetOrigin(240, 0);
		SizeWindow(dp, 250, dp->portRect.bottom - dp->portRect.top, TRUE);
		setactionbutton(dp, (StringPtr)"\pCopy");
		hideitem(dp, getOpen);
		hideitem(dp, TEXTITEM);
	}
	retval = hookcommon(item, dp, docopydisk);
	RestoreA4();
	return retval;
}

pascal INTEGER movefileshook(INTEGER item, DialogPtr dp)
{
	INTEGER retval;
	
    SetUpA4();
	abortflag = false;
	if (item == -1) {
		myShowWindow(destdirwin);
		setactionbutton(dp, (StringPtr)"\pMove");
		hideitem(dp, TEXTITEM);
	}
	retval = hookcommon(item, dp, domove);
	RestoreA4();
	return retval;
}

pascal INTEGER newdirhook(INTEGER item, DialogPtr dp)
{
	Handle h;
	INTEGER type;
	Rect r;
	INTEGER retval;

	SetUpA4();
	abortflag = false;
	if (item == -1) {
		setactionbutton(dp, (StringPtr)"\pCreate");
		GetDItem(dp, TEXTITEM, &type, &h, &r);
		SetIText(h, (StringPtr) "\pNewFolder");
		SelIText(dp, TEXTITEM, 0, 32767);
		hideitem(dp, NEWDESTBUTTON);
		hideitem(dp, DESTNAME);
	}
	retval = hookcommon(item, dp, donewdir);
	RestoreA4();
	return retval;
}

pascal INTEGER renamefileshook(INTEGER item, DialogPtr dp)
{
	Handle h;
	INTEGER type;
	Rect r;
	INTEGER retval;

	SetUpA4();
	abortflag = false;
	if (item == -1) {
		setactionbutton(dp, (StringPtr)"\pRename");
		GetDItem(dp, TEXTITEM, &type, &h, &r);
		SetIText(h, (StringPtr) "\p");
		hideitem(dp, NEWDESTBUTTON);
		hideitem(dp, DESTNAME);
	}
	retval = hookcommon(item, dp, dorename);
	RestoreA4();
	return retval;
}

pascal INTEGER deletefileshook(INTEGER item, DialogPtr dp)
{
	INTEGER retval;
	
	SetUpA4();
	abortflag = false;
	if (item == -1) {
		setactionbutton(dp, (StringPtr)"\pDelete");
		hideitem(dp, NEWDESTBUTTON);
		hideitem(dp, DESTNAME);
		hideitem(dp, TEXTITEM);
	}
	retval = hookcommon(item, dp, dodelete);
	RestoreA4();
	return retval;
}

Handle savembar, ourmbar;

void init( int rn, DialogPtr *ourwindowp )
{
	INTEGER i, itemtype;
	Handle itemhand;
	Rect itemrect;

	Rect r;
	DialogPtr startdp;

#if	0
	InitGraf((Ptr) &thePort);
	InitWindows();
	InitFonts();
	InitMenus();
	InitCursor();
	InitDialogs((ProcPtr)0);
	TEInit();
	
#endif
	savembar = GetMenuBar();
	ClearMenuBar();
	
	drvrmenu = NewMenu(DRVR(ourid, 0), (StringPtr) "\p\023");
	for (i=0;i<NELEM(ap_funcs) ; i++)
		AppendMenu(drvrmenu, (StringPtr) ap_funcs[i].name);
	InsertMenu(drvrmenu, 0);

	editmenu = NewMenu(DRVR(ourid, 2),(StringPtr) "\pEdit");
	for (i=0;i<NELEM(edit_funcs) ; i++)
		AppendMenu(editmenu, (StringPtr) edit_funcs[i].name);
	InsertMenu(editmenu, 0);
	ourmbar = GetMenuBar();
	MBarEnable = rn;

	DrawMenuBar();
	
	startdp = GetNewDialog(STARTALERT, (Ptr) 0, (WindowPtr) -1);
	((WindowPeek) startdp)->windowKind = rn;
	*ourwindowp = startdp;

	GetDItem(startdp, NEW_VOLUME_ITEM, &itemtype, &itemhand, &itemrect);
	HiliteControl((ControlHandle) itemhand, 255);
	
	verify_flags = 0;
	for (i = FIRST_VERIFY_ITEM; i <= LAST_VERIFY_ITEM; ++i) {
		GetDItem(startdp, i, &itemtype, &itemhand, &itemrect);
		SetCtlValue((ControlHandle) itemhand, 1);
		verify_flags |= ITEM_TO_BIT(i);
	}
	
	destdir = CurDirStore;
	destdisk = SFSaveDisk;
	destdirwin = GetNewWindow(DESTDIRWINID, (Ptr) 0, (WindowPtr) -1);
	SetPort(destdirwin);
	r = destdirwin->portRect;
	InsetRect(&r, 4, 8);
	OffsetRect(&r, 0, 8);
	destdirname = TENew(&r, &r);
	TEAutoView(TRUE, destdirname);
	(*destdirname)->txSize = 12;
}

void doitem(long choice, int rn)
{
	Str255 apname;
	funcinfo *infop;
	Point pt;

	switch (HiWord(choice) - DRVR(ourid, 0)) {
	case 0:
		if (LoWord(choice) <= NELEM(ap_funcs)){
			(*ap_funcs[LoWord(choice) - 1].ptr)();
		} else {
			GetItem(drvrmenu,LoWord(choice),(StringPtr) &apname);
			OpenDeskAcc(apname);
		}
		break;
	case 1:
		infop = file_funcs + LoWord(choice) - 1 ;
		SetPt(&pt, infop->h, infop->v);
		if (infop->dlgHook == 0)
			quitfunc(rn);
		else
			SFPGetFile(pt, (StringPtr) infop->prompt,
											(ProcPtr) visiblefilesonly, -1, 0,
										(ProcPtr) infop->dlgHook, &globalreply,
											XFERDLGID, (ProcPtr) myfilterproc);
		HideWindow(destdirwin);
		break;
	case 2:
		(*edit_funcs[LoWord(choice) - 1].ptr)();
		break;
	default:
		break;
	}
}

short ourid;

int main( cntrlParam *pb, DCtlPtr dctlp, int n )
{
    static int beenhere = 0;
	EventRecord *evtp;
	long mitem;
	WindowPtr winp;
	WDPBRec wd;
	DialogPtr dp;
	INTEGER item, itemtype, val;
	Handle itemhand;
	Rect itemrect;
	typedef enum {
	    OpenSel,
	    PrimeSel,
	    ControlSel,
	    StatusSel,
	    CloseSel
	} think_sel_t;	/* See Think C p. 78 */
	
	ourid = -(dctlp->dCtlRefNum+1);
	RememberA4();
	init_all_tables();	/* has to be done with every call because
							the pointers to functions can move */
	switch (n) {
	case OpenSel:
	    if (!beenhere) {
	        beenhere = 1;
		    init(dctlp->dCtlRefNum, &dctlp->dCtlWindow);
		    dctlp->dCtlMenu = dctlp->dCtlRefNum;
		}
	    break;
	case PrimeSel:
	    break;
	case ControlSel:
		if (pb->csCode == accEvent) {
			evtp = *(EventRecord **)pb->csParam;
			if (evtp->what == activateEvt) {
				if (evtp->modifiers & activeFlag) {
					if (MBarEnable != dctlp->dCtlRefNum) {
						SetMenuBar(ourmbar);
						DrawMenuBar();
						MBarEnable = dctlp->dCtlRefNum;
					}
				} else {
					if (MBarEnable == dctlp->dCtlRefNum) {
						SetMenuBar(savembar);
						DrawMenuBar();
						MBarEnable = 0;
					}
				}
			}
			if (evtp->what != keyDown) {
			    if (DialogSelect(evtp, &dp, &item)) {
				    switch (item) {
					case NEW_VOLUME_ITEM:
						/* FALL THROUGH */
					case COPY_DISK_ITEM:
					case MOVE_FILE_ITEM:
					case COPY_FILE_ITEM:
					case RENAME_FILE_ITEM:
					case DELETE_FILE_ITEM:
					case NEW_FOLDER_ITEM:
						doitem(((1L + DRVR(ourid, 0)) << 16)|(item - COPY_DISK_ITEM+1),
													dctlp->dCtlRefNum);
					    break;
					
					case VERIFY_OVERWRITE_FILE_ITEM:
					case VERIFY_OVERWRITE_FOLDER_ITEM:
					case VERIFY_DELETE_FILE_ITEM:
					case VERIFY_DELETE_FOLDER_ITEM:
						GetDItem(dp, item, &itemtype, &itemhand, &itemrect);
						val = 1 ^ GetCtlValue((ControlHandle) itemhand);
						SetCtlValue((ControlHandle) itemhand, val);
						if (val)
						    verify_flags |= ITEM_TO_BIT(item);
						else
							verify_flags &= ~ITEM_TO_BIT(item);
					    break;
				    }
			    }
			} else {
				switch(evtp->what) {
				case mouseDown:
					break;
				case keyDown:
					if (evtp->modifiers & cmdKey) {
						mitem = MenuKey(evtp->message & 0xFF);
						doitem(mitem, dctlp->dCtlRefNum);
						HiliteMenu(0);
					}
					break;
				default:
					break;
				}
			}
		} else if (pb->csCode == accMenu) {
			mitem = *(long *)pb->csParam;
			doitem(mitem, dctlp->dCtlRefNum);
			HiliteMenu(0);
		}
	    break;
	case StatusSel:
	    break;
	case CloseSel:
		SetMenuBar(savembar);
		DrawMenuBar();
		MBarEnable = 0;
		DisposDialog(dctlp->dCtlWindow);
		DisposHandle(ourmbar);
		DisposHandle(savembar);
		dctlp->dCtlWindow = 0;
	    break;
	}
	return 0;
}
