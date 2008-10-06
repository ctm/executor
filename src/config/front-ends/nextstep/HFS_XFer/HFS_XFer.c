#include "rsys/common.h"
#include "xfer.h"

#if !defined(MAC)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <libc.h>
#include <signal.h>
#include "fs.h"
#include <assert.h>
#endif /* !defined(MAC) */
     
#define LONGTIME		216000		/* one hour */

func ap_funcs[] = {
     "\pAbout HFS_XFer...", about_HFS_XFer
};

funcinfo file_funcs[] = {
     "\pCopy Disk...", copydiskhook, 110, 30, "\pCopy Disk",
     "\pMove Files...", movefileshook, 28, 30, "\pMove File or Folder",
     "\pCopy Files...", copyfileshook, 28, 30, "\pCopy File or Folder",
     "\pRename...", renamefileshook, 28, 40, "\pRename File or Folder",
     "\pDelete...", deletefileshook, 28, 40, "\pDelete File or Folder",
     "\pNew Folder...", newdirhook, 28, 40, "\pCreate New Folder",
     "\p(-", 0, 0, 0, "",
     "\pQuit/Q", 0, 0, 0, ""
};
    
func edit_funcs[] = {
     "\p(Undo/Z", 0,
     "\p(Cut/X", 0,
     "\p(Copy/C", 0,
     "\p(Paste/V", 0
};

option optionvars[] = {
     "\pVerify overwriting files", &verifyfileoverwrite,
     "\pVerify overwriting directories", &verifydiroverwrite, 
     "\pVerify deleting files", &verifyfiledelete,
     "\pVerify deleting directories", &verifydirdelete
};

MenuHandle drvrmenu, filemenu, editmenu, optionmenu;
LONGINT destdir;
INTEGER destdisk, verifyfileoverwrite, verifydiroverwrite, 
			verifyfiledelete, verifydirdelete;
SFReply globalreply;
WindowPtr destdirwin;
TEHandle destdirname;

void doerror(OSErr errno, char * s)
{
    static errortable errormessages[] = {
	wPrErr,	"\pThat volume is locked.",
	vLckdErr,	"\pThat volume is locked.",
	permErr,	"\pPermission error writing to locked file.",
	ioErr,	"\pI/O Error."
    };

    INTEGER i;
    Str255 s2;

    for (i = 0 ; i < NELEM(errormessages) && errormessages[i].number != errno
									; i++)
	;
    if ( i < NELEM(errormessages) ) {
        ParamText((StringPtr) errormessages[i].message, 0, 0, 0);
        StopAlert(ONEPARAMALERT, (ProcPtr) 0);
    } else {
        NumToString((LONGINT)errno, s2);
        ParamText(s2, (StringPtr) s, 0, 0);
        CautionAlert(DOERRORALERT, (ProcPtr) 0);
    }
}

void about_HFS_XFer()
{
    Alert(ABOUTALERT, (ProcPtr) 0);
}

INTEGER quitfunc(void)
{
    ExitToShell();
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
        xPBGetCatInfo(&hpb, FALSE);
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
/*-->*/     return TRUE;
	} else if ((event->message & 0xFF) == '.' &&
						(event->modifiers & cmdKey)) {
	    *item = Cancel;
/*-->*/     return TRUE;
	}
	break;
    case updateEvt:
	if ((WindowPtr)event->message == destdirwin)
	    updatedestwin(-destdisk, destdir);
	break;
    }
    return FALSE;
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

    if (currentdisk != SFSaveDisk) {
	invaldestwin();
	currentdisk = SFSaveDisk;
    }
    switch(ep->what) {
    case keyDown:
	if ((ep->message & 0xFF) == '\r') {
	    *itemhit = getOpen;
/*-->*/     return TRUE;
	}
	break;
    case nullEvent:
	*itemhit = 100;
/*-->*/ return TRUE;
    case updateEvt:
	if ((WindowPtr)ep->message == destdirwin) {
	    updatedestwin(-SFSaveDisk, CurDirStore);
	    currentdisk = SFSaveDisk;
	}
	break;
    }
    return FALSE;
}

pascal INTEGER DirDlgHook(INTEGER itemhit, DialogPtr dp)
{
    static INTEGER needupdate = TRUE;

    if (needupdate) {
	invaldestwin();
	needupdate = FALSE;
    }
    if (itemhit != 100)
	needupdate = TRUE;
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
	if (PBHGetVInfo(&pb, FALSE)==noErr && pb.volumeParam.ioVDrvInfo != 0) {
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
	needtoupdatedestdisk = FALSE;
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

pascal INTEGER copyfileshook(INTEGER item, DialogPtr dp)
{
    if (item == -1) {
	ShowWindow(destdirwin);
        hideitem(dp, TEXTITEM);
        setactionbutton(dp, (StringPtr)"\pCopy");
    }
    return hookcommon(item, dp, docopy);
}

pascal INTEGER copydiskhook(INTEGER item, DialogPtr dp)
{
    if (item == -1) {
	ShowWindow(destdirwin);
        SetOrigin(240, 0);
        SizeWindow(dp, 250, dp->portRect.bottom - dp->portRect.top, TRUE);
        setactionbutton(dp, (StringPtr)"\pCopy");
        hideitem(dp, getOpen);
        hideitem(dp, TEXTITEM);
    }
    return hookcommon(item, dp, docopydisk);
}

pascal INTEGER movefileshook(INTEGER item, DialogPtr dp)
{
    if (item == -1) {
	ShowWindow(destdirwin);
        setactionbutton(dp, (StringPtr)"\pMove");
        hideitem(dp, TEXTITEM);
    }
    return hookcommon(item, dp, domove);
}

pascal INTEGER newdirhook(INTEGER item, DialogPtr dp)
{
    Handle h;
    INTEGER type;
    Rect r;

    if (item == -1) {
        setactionbutton(dp, (StringPtr)"\pCreate");
	GetDItem(dp, TEXTITEM, &type, &h, &r);
	SetIText(h, (StringPtr) "\pNewFolder");
	SelIText(dp, TEXTITEM, 0, 32767);
        hideitem(dp, NEWDESTBUTTON);
        hideitem(dp, DESTNAME);
    }
    return hookcommon(item, dp, donewdir);
}

pascal INTEGER renamefileshook(INTEGER item, DialogPtr dp)
{
    Handle h;
    INTEGER type;
    Rect r;

    if (item == -1) {
        setactionbutton(dp, (StringPtr)"\pRename");
	GetDItem(dp, TEXTITEM, &type, &h, &r);
	SetIText(h, (StringPtr) "\p");
        hideitem(dp, NEWDESTBUTTON);
        hideitem(dp, DESTNAME);
    }
    return hookcommon(item, dp, dorename);
}

pascal INTEGER deletefileshook(INTEGER item, DialogPtr dp)
{
    if (item == -1) {
        setactionbutton(dp, (StringPtr)"\pDelete");
        hideitem(dp, NEWDESTBUTTON);
        hideitem(dp, DESTNAME);
        hideitem(dp, TEXTITEM);
    }
    return hookcommon(item, dp, dodelete);
}

void changeoption(INTEGER i)
{
    INTEGER val;

    val = !*optionvars[i - 1].var;
    CheckItem(optionmenu, i, val);
    *optionvars[i - 1].var = val;
}

void setalloptions(INTEGER what)
{
    INTEGER i;
    
    for (i = 1 ; i <= NELEM(optionvars) ; i++) {
        CheckItem(optionmenu, i, what);
        *optionvars[i - 1].var = what;
    }
}


void init( void )
{
    INTEGER i;
    Rect r;
    
    InitGraf((Ptr) &thePort);
    InitWindows();
    InitFonts();
    InitMenus();
    InitCursor();
    InitDialogs((ProcPtr)0);
    TEInit();
    
    drvrmenu = NewMenu(1,(StringPtr) "\p\023");
    for (i=0;i<NELEM(ap_funcs) ; i++)
	AppendMenu(drvrmenu, (StringPtr) ap_funcs[i].name);
    AppendMenu(drvrmenu,(StringPtr) "\p(-");
    AddResMenu(drvrmenu,'DRVR');
    InsertMenu(drvrmenu, 0);
    
    filemenu = NewMenu(2,(StringPtr) "\pFile");
    for (i=0;i<NELEM(file_funcs) ; i++)
	AppendMenu(filemenu, (StringPtr) file_funcs[i].name);
    InsertMenu(filemenu, 0);
    
    editmenu = NewMenu(3,(StringPtr) "\pEdit");
    for (i=0;i<NELEM(edit_funcs) ; i++)
	AppendMenu(editmenu, (StringPtr) edit_funcs[i].name);
    InsertMenu(editmenu, 0);
    
    optionmenu = NewMenu(4,(StringPtr) "\pOptions");
    for (i=0;i<NELEM(optionvars) ; i++) {
	AppendMenu(optionmenu, (StringPtr) optionvars[i].name);
	*optionvars[i].var = TRUE;
	CheckItem(optionmenu, i + 1, TRUE);
    }
    AppendMenu(optionmenu,(StringPtr) "\p-;Verify None;Verify All");
    InsertMenu(optionmenu, 0);
    
    DrawMenuBar();
    
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

void doitem(choice)
long choice;
{
    Str255 apname;
    funcinfo *infop;
    Point pt;

    switch (HiWord(choice)) {
    case 1:
	if (LoWord(choice) <= NELEM(ap_funcs)){
	    (*ap_funcs[LoWord(choice) - 1].ptr)();
	} else {
	    GetItem(drvrmenu,LoWord(choice),(StringPtr) &apname);
	    OpenDeskAcc(apname);
	}
	break;
    case 2:
	infop = file_funcs + LoWord(choice) - 1 ;
	SetPt(&pt, infop->h, infop->v);
	if (infop->dlgHook == 0)
	    quitfunc();
	else
	    SFPGetFile(pt, (StringPtr) infop->prompt,
					    (ProcPtr) visiblefilesonly, -1, 0,
					(ProcPtr) infop->dlgHook, &globalreply,
					    XFERDLGID, (ProcPtr) myfilterproc);
	HideWindow(destdirwin);
	break;
    case 3:
	(*edit_funcs[LoWord(choice) - 1].ptr)();
	break;
    case 4:
	if (LoWord(choice) <= NELEM(optionvars)){
	    changeoption(LoWord(choice));
	} else {
	    setalloptions(LoWord(choice) - NELEM(optionvars) - 2);
	}
	break;
    default:
	break;
    }
}

long sock;
long pipefd[2];

void main( void )
{
    EventRecord evt;
    long mitem;
    WindowPtr winp;
    WDPBRec wd;
    
    wd.ioWDDirID = CurDirStore;
    wd.ioVRefNum = -SFSaveDisk;
    wd.ioNamePtr = 0;
    PBHSetVol(&wd, FALSE);
    OpenResFile((StringPtr) "\pHFS_XFer");	/* DON'T TAKE THIS OUT!
						   or put calls to init()
						   before it! */
    init();
#if defined(UNIX)
    ROMlib_WriteWhen(WriteInOSEvent);
#endif
    for (;;) {
        while (!WaitNextEvent(everyEvent, &evt, LONGTIME, 0))
            ;
        switch(evt.what) {
	case mouseDown:
	    if (FindWindow(evt.where, &winp) == inMenuBar) {
		mitem = MenuSelect(evt.where);
		doitem(mitem);
		HiliteMenu(0);
	    }
	    break;
	case keyDown:
	    if (evt.modifiers & cmdKey) {
		mitem = MenuKey(evt.message & 0xFF);
		doitem(mitem);
		HiliteMenu(0);
	    }
	    break;
	default:
	    break;
        }
    }
}
