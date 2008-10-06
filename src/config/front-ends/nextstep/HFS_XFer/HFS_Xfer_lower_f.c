#include "rsys/common.h"
#include "xfer.h"
     
func ap_funcs[] = {
     "\pAbout HFS xfer",about_HFSxfer
};

func file_funcs[] = {
     "\pCopy Disk", copydisk,
     "\pMove or Copy", transferfiles,
     "\pRename", renamefiles,
     "\pDelete", deletefiles,
     "\p(-", 0,
     "\pQuit/Q", quitfunc
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
LONGINT Firstcurdirstore, Secondcurdirstore;
INTEGER Firstsavedisk, Secondsavedisk, verifyfileoverwrite,
		     verifydiroverwrite, verifyfiledelete, verifydirdelete;
ListHandle firstlist, secondlist;

void framelists()
{
    Rect r;
    
    r = (*firstlist)->rView;
    InsetRect(&r, -1, -1);
    FrameRect(&r);
    r = (*secondlist)->rView;
    InsetRect(&r, -1, -1);
    FrameRect(&r);
}

void doerror(OSErr errno, char * s)
{
    Str255 s2;
    
    if (errno == wPrErr || errno == vLckdErr) {
        ParamText("\pThat volume is locked.", 0, 0, 0);
        StopAlert(ONEPARAMALERT, (ProcPtr) 0);
    } else {
        NumToString((LONGINT)errno, s2);
        ParamText(s2, s, 0, 0);
        CautionAlert(DOERRORALERT, (ProcPtr) 0);
    }
    framelists();
    LUpdate(thePort->visRgn, firstlist);
    LUpdate(thePort->visRgn, secondlist);
}

void filllist(ListHandle list, LONGINT curdir, INTEGER savedisk)
{
    Str255 s;
    char s2[256];
    CInfoPBRec pb;
    OSErr err;
    Cell cell;
    INTEGER numrows, len, done;
    
    err = noErr;
    SetPt(&cell, 0, 0);
    LDelRow(0, 0, list);
    LDoDraw(FALSE, list);
    numrows = 0;
    pb.hFileInfo.ioNamePtr  = s + 2;
    pb.hFileInfo.ioVRefNum  = savedisk;
    for (pb.hFileInfo.ioFDirIndex = 1; err != fnfErr;
 					     pb.hFileInfo.ioFDirIndex++) {
 	pb.hFileInfo.ioDirID = curdir;
 	err = xPBGetCatInfo(&pb, FALSE);
 	if (err == noErr) {
	    done = FALSE;
	    for (cell.v = 0 ; !done & (cell.v < numrows) ; cell.v++) {
	        len = 255;
	        LGetCell(s2, &len, cell, list);
	        s2[1] = len;
	        done = RelString(s + 2, s2 + 1, FALSE, FALSE) < 1;
	    }
	    if (done)
	        cell.v--;
	    len = s[2] + 2;
    	    if (pb.hFileInfo.ioFlAttrib & ISDIRMASK)
		s[1] = '>';
	    else
	    	s[1] = ' ';
	    s[2] = ' ';
 	    LAddRow(1, cell.v, list);
 	    LSetCell(s + 1, len, cell, list);
 	    numrows++;
 	} else if (err != fnfErr) {
 	    doerror(err, "\pPBGetCatInfo");
 	    break;
 	}
    }
    LDoDraw(TRUE, list);
    LUpdate(thePort->visRgn, list);
}
   
void cd(ListHandle list, direction way, INTEGER vrn, LONGINT  * dirid)
{
    Cell c;
    INTEGER len;
    Str255 s;
    CInfoPBRec hpb;
    OSErr err;
    
    hpb.hFileInfo.ioVRefNum = vrn;
    hpb.hFileInfo.ioDirID = *dirid;
    hpb.hFileInfo.ioNamePtr = s+1;
    switch(way) {
        case up:
	    hpb.hFileInfo.ioFDirIndex = -1;
	    err = xPBGetCatInfo(&hpb, FALSE);
	    if (err != noErr) {
 	        doerror(err, "\pPBGetCatInfo");
 	        return;
 	    }
	    if (hpb.hFileInfo.ioFlParID < 2)
/*-->*/         return;
	    *dirid = hpb.hFileInfo.ioFlParID;
            break;
        case down:
            SetPt(&c, 0, 0);
            if (!LGetSelect(TRUE, &c, list))
/*-->*/		return;
            len = 255;
	    LGetCell(s, &len, c, list);
	    s[1] = (unsigned char) len - 2;
	    hpb.hFileInfo.ioFDirIndex = 0;
	    err = xPBGetCatInfo(&hpb, FALSE);
	    if (err != noErr)
 	        doerror(err, "\pPBGetCatInfo");
    	    if (hpb.hFileInfo.ioFlAttrib & ISDIRMASK)
	        *dirid = hpb.dirInfo.ioDrDirID;
	    else
/*-->*/         return;
            break;
        default:
            break;
    }
    filllist(list, *dirid, vrn);
    framelists();
}

void about_HFSxfer()
{
    Alert(ABOUTALERT, (ProcPtr) 0);
}

void quitfunc()
{
    ExitToShell();
}

void unselect(ListHandle list)
{
    Cell c;
    
    c.h = 0;
    for (c.v = 0 ; c.v < (*list)->dataBounds.bottom ; c.v++)
        LSetSelect(FALSE, c, list);
}

void changedisk(ListHandle list, LONGINT *dirid, INTEGER  *disk,
					     DialogPtr dp, INTEGER itemno)
{    
    INTEGER current;
    ParamBlockRec pb;
    INTEGER vref0;
    OSErr err;
    Rect r;
    INTEGER type;
    Handle h;
    Str255 s;
 
    pb.volumeParam.ioVRefNum = *disk;
    pb.volumeParam.ioNamePtr = 0;
    pb.volumeParam.ioVolIndex = 0;
    err = xPBGetVInfo(&pb, FALSE);
    if (err != noErr)
        doerror(err, "\pPBGetVInfo");
    current = pb.volumeParam.ioVRefNum;
 
    pb.volumeParam.ioVolIndex = 1;
    err = xPBGetVInfo(&pb, FALSE);
    if (err != noErr)
        doerror(err, "\pPBGetVInfo");
    vref0 = pb.volumeParam.ioVRefNum;
    while (err == noErr && pb.volumeParam.ioVRefNum != current) {
 	++pb.volumeParam.ioVolIndex;
 	err = xPBGetVInfo(&pb, FALSE);
    }
    if (err != noErr) {
 	*disk = 0;
    } else {
 	++pb.volumeParam.ioVolIndex;
 	err = xPBGetVInfo(&pb, FALSE);
 	if (err == noErr)
 	    *disk = pb.volumeParam.ioVRefNum;
 	else
 	    *disk = vref0;
    }
    *dirid = 2;
    filllist(list, *dirid, *disk);
    pb.volumeParam.ioVolIndex = 0;
    pb.volumeParam.ioNamePtr = s;
    pb.volumeParam.ioVRefNum = *disk;
    err = xPBGetVInfo(&pb, FALSE);
    if (err != noErr) {
        doerror(err, "\pPBGetVInfo");
        return;
    }
    GetDItem(dp, itemno, &type, &h, &r);
    SetIText(h, s);
    framelists();
}

void eject(INTEGER vrn)
{
    OSErr err;
    ParamBlockRec pb;
    
    pb.volumeParam.ioCompletion = 0;
    pb.volumeParam.ioNamePtr = 0;
    pb.volumeParam.ioVRefNum = vrn;
    err = xPBEject(&pb);
    if (err != noErr)
        doerror(err, "\pPBEject");
    err = xPBUnmountVol(&pb);
    if (err != noErr)
        doerror(err, "\pPBUnmountVol");
}

void hilite(DialogPtr dp, INTEGER val)
{
    Rect r;
    INTEGER type;
    Handle h;
    Str255 s;

    GetDItem(dp, COPYBUTTON, &type, &h, &r);
    GetCTitle((ControlHandle)h, s);
    if (s[1] != 'C')
        HiliteControl((ControlHandle) h, val);
    GetDItem(dp, MOVEBUTTON, &type, &h, &r);
    HiliteControl((ControlHandle) h, val);
    GetDItem(dp, DELETEBUTTON, &type, &h, &r);
    HiliteControl((ControlHandle) h, val);
    GetDItem(dp, RENAMEBUTTON, &type, &h, &r);
    HiliteControl((ControlHandle) h, val);
}

pascal INTEGER myfilterproc(DialogPtr unuseddp, EventRecord *event, 
							    INTEGER *item)
{
    DialogPtr dp;
    Rect r;
    INTEGER type;
    Handle h;
    Str255 s;
    INTEGER toeject;
    EventRecord evt;
    Cell c;
    
    if (event->what != mouseDown)
/*-->*/ return FALSE;
    GetNextEvent(diskMask, &evt);
    DialogSelect(event, &dp, item);
    SetPt(&c, 0, 0);
    switch (*item) {
        case FIRSTLIST:
            unselect(secondlist);
            GetDItem(dp, COPYBUTTON, &type, &h, &r);
            GetCTitle((ControlHandle)h, s);
            if (s[1] == '<') {
                SetCTitle((ControlHandle)h, "\p>> Copy >>");
                GetDItem(dp, MOVEBUTTON, &type, &h, &r);
                SetCTitle((ControlHandle)h, "\p>> Move >>");
            }
/* FALL THROUGH */
        case FIRSTSCROLLBAR:
            GlobalToLocal(&event->where);
            if (LClick(event->where, event->modifiers, firstlist) &&
                  !(event->modifiers & (cmdKey | shiftKey | optionKey))) {
                cd(firstlist, down, Firstsavedisk, &Firstcurdirstore);
            }
            event->what = nullEvent;
            if (!LGetSelect(TRUE, &c, firstlist))
                hilite(dp, 255);
            else
                hilite(dp, 0);
            break;
        case SECONDLIST:
            unselect(firstlist);
            GetDItem(dp, COPYBUTTON, &type, &h, &r);
            GetCTitle((ControlHandle)h, s);
            if (s[1] == '>') {
                SetCTitle((ControlHandle)h, "\p<< Copy <<");
                GetDItem(dp, MOVEBUTTON, &type, &h, &r);
                SetCTitle((ControlHandle)h, "\p<< Move <<");
            }
/* FALL THROUGH */
        case SECONDSCROLLBAR:
            GlobalToLocal(&event->where);
            if (LClick(event->where, event->modifiers, secondlist) &&
                  !(event->modifiers & (cmdKey | shiftKey | optionKey))) {
                cd(secondlist, down, Secondsavedisk, &Secondcurdirstore);
            }
            event->what = nullEvent;
            if (!LGetSelect(TRUE, &c, secondlist))
                hilite(dp, 255);
            else
                hilite(dp, 0);
            break;
        case UPFIRSTDIR:
            cd(firstlist, up, Firstsavedisk, &Firstcurdirstore);
            if (!LGetSelect(TRUE, &c, secondlist))
                hilite(dp, 255);
            break;
        case UPSECONDDIR:
            cd(secondlist, up, Secondsavedisk, &Secondcurdirstore);
            if (!LGetSelect(TRUE, &c, firstlist))
                hilite(dp, 255);
            break;
        case FIRSTDRIVE:
            changedisk(firstlist, &Firstcurdirstore, &Firstsavedisk, dp,
            						    FIRSTVOLUME);
            if (!LGetSelect(TRUE, &c, secondlist))
                hilite(dp, 255);
            break;
        case SECONDDRIVE:
            changedisk(secondlist, &Secondcurdirstore, &Secondsavedisk, dp,
            						      SECONDVOLUME);
            if (!LGetSelect(TRUE, &c, firstlist))
                hilite(dp, 255);
            break;
        case DOWNFIRSTDIR:
            cd(firstlist, down, Firstsavedisk, &Firstcurdirstore);
            if (!LGetSelect(TRUE, &c, secondlist))
                hilite(dp, 255);
            break;
        case DOWNSECONDDIR:
            cd(secondlist, down, Secondsavedisk, &Secondcurdirstore);
            if (!LGetSelect(TRUE, &c, firstlist))
                hilite(dp, 255);
            break;
        case FIRSTEJECTBUTTON:
            toeject = Firstsavedisk;
            changedisk(firstlist, &Firstcurdirstore, &Firstsavedisk, dp,
            						    FIRSTVOLUME);
            eject(toeject);
            if (!LGetSelect(TRUE, &c, secondlist))
                hilite(dp, 255);
            break;
        case SECONDEJECTBUTTON:
            toeject = Secondsavedisk;
            changedisk(secondlist, &Secondcurdirstore, &Secondsavedisk, dp,
            						      SECONDVOLUME);
            eject(toeject);
            if (!LGetSelect(TRUE, &c, firstlist))
                hilite(dp, 255);
            break;
        default:
	    return TRUE;
            break;
    }
    return FALSE;
}

void setuplists(DialogPtr dp)
{
    Handle h;
    INTEGER type;
    Rect r, dbounds;
    Point csize;
    ParamBlockRec pb;
    Str255 s;
    OSErr err;

    SpaceExtra((Fixed)(CharWidth('>') - CharWidth(' ')) << 16);
    GetDItem(dp, FIRSTLIST, &type, &h, &r);
    SetRect(&dbounds, 0,0,1,0);
    SetPt(&csize, 0, 0);
    firstlist = LNew(&r, &dbounds, csize, 0, dp, FALSE, FALSE, FALSE, TRUE);
    filllist(firstlist, Firstcurdirstore, Firstsavedisk);
    pb.volumeParam.ioNamePtr = s;
    pb.volumeParam.ioVolIndex = 0;
    pb.volumeParam.ioVRefNum = Firstsavedisk;
    err = xPBGetVInfo(&pb, FALSE);
    if (err != noErr)
        doerror(err, "\pPBGetVInfo");
    GetDItem(dp, FIRSTVOLUME, &type, &h, &r);
    SetIText(h, s);
    GetDItem(dp, SECONDLIST, &type, &h, &r);
    secondlist = LNew(&r, &dbounds, csize, 0, dp, FALSE, FALSE, FALSE, TRUE);
    filllist(secondlist, Secondcurdirstore, Secondsavedisk);
    pb.volumeParam.ioVRefNum = Secondsavedisk;
    err = xPBGetVInfo(&pb, FALSE);
    if (err != noErr)
        doerror(err, "\pPBGetVInfo");
    GetDItem(dp, SECONDVOLUME, &type, &h, &r);
    SetIText(h, s);
    hilite(dp, 255);
    ShowWindow(dp);
    framelists();
    LUpdate(thePort->visRgn, firstlist);
    LUpdate(thePort->visRgn, secondlist);
}

void transferfiles()
{
    DialogPtr dp;
    int itemhit;
    GrafPtr saveport;
    
    GetPort(&saveport);
    dp = GetNewDialog(FILETRANSFERDIALOGID, (Ptr) 0, (Ptr) -1);
    SetPort(dp);
    setuplists(dp);
    itemhit = 0;
    while (itemhit != Cancel) {
        ModalDialog(myfilterproc, &itemhit);
        switch(itemhit) {
            case COPYBUTTON:
                dotransfer(dp, copy1file);
		filllist(firstlist, Firstcurdirstore, Firstsavedisk);
		filllist(secondlist, Secondcurdirstore, Secondsavedisk);
		framelists();
                break;
            case MOVEBUTTON:
                dotransfer(dp, move1file);
		filllist(firstlist, Firstcurdirstore, Firstsavedisk);
		filllist(secondlist, Secondcurdirstore, Secondsavedisk);
		framelists();
                break;
            default:
                break;
	}
    }
    SetPort(saveport);
    LDispose(firstlist);
    LDispose(secondlist);
    DisposDialog(dp);
}

void copydisk()
{
    DialogPtr dp;
    int itemhit;
    GrafPtr saveport;
    
    GetPort(&saveport);
    dp = GetNewDialog(COPYDISKDIALOGID, (Ptr) 0, (Ptr) -1);
    SetPort(dp);
    setuplists(dp);
    (*secondlist)->selFlags |= lOnlyOne;
    itemhit = 0;
    while (itemhit != Cancel) {
        ModalDialog(myfilterproc, &itemhit);
        switch(itemhit) {
            case COPYBUTTON:
                docopydisk();
		filllist(secondlist, Secondcurdirstore, Secondsavedisk);
		framelists();
                break;
            default:
                break;
	}
    }
    SetPort(saveport);
    LDispose(firstlist);
    LDispose(secondlist);
    DisposDialog(dp);
}

void renamefiles()
{
    DialogPtr dp;
    int itemhit;
    GrafPtr saveport;
    
    GetPort(&saveport);
    dp = GetNewDialog(RENAMEDIALOGID, (Ptr) 0, (Ptr) -1);
    SetPort(dp);
    setuplists(dp);
    (*firstlist)->selFlags |= lOnlyOne;
    itemhit = 0;
    while (itemhit != Cancel) {
        ModalDialog(myfilterproc, &itemhit);
        switch(itemhit) {
            case RENAMEBUTTON:
                dorename(dp);
		filllist(firstlist, Firstcurdirstore, Firstsavedisk);
		framelists();
                break;
            default:
                break;
	}
    }
    SetPort(saveport);
    LDispose(firstlist);
    LDispose(secondlist);
    DisposDialog(dp);
}

void deletefiles()
{
    DialogPtr dp;
    int itemhit;
    GrafPtr saveport;
    
    GetPort(&saveport);
    dp = GetNewDialog(DELETEDIALOGID, (Ptr) 0, (Ptr) -1);
    SetPort(dp);
    setuplists(dp);
    itemhit = 0;
    while (itemhit != Cancel) {
        ModalDialog(myfilterproc, &itemhit);
        switch(itemhit) {
            case DELETEBUTTON:
                dodelete();
		filllist(firstlist, Firstcurdirstore, Firstsavedisk);
		framelists();
                break;
            default:
                break;
	}
    }
    SetPort(saveport);
    LDispose(firstlist);
    LDispose(secondlist);
    DisposDialog(dp);
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


void init()
{
    int i;
    
    OpenResFile("\pHFS_Xfer");
    InitGraf(&thePort);
    InitWindows();
    InitFonts();
    InitMenus();
    InitCursor();
    InitDialogs((ProcPtr)0);
    TEInit();
    
    drvrmenu = NewMenu(1,"\p\023");
    for (i=0;i<NELEM(ap_funcs) ; i++)
	AppendMenu(drvrmenu,ap_funcs[i].name);
    AppendMenu(drvrmenu,"\p(-");
    AddResMenu(drvrmenu,'DRVR');
    InsertMenu(drvrmenu, 0);
    
    filemenu = NewMenu(2,"\pFile");
    for (i=0;i<NELEM(file_funcs) ; i++)
	AppendMenu(filemenu,file_funcs[i].name);
    InsertMenu(filemenu, 0);
    
    editmenu = NewMenu(3,"\pEdit");
    for (i=0;i<NELEM(edit_funcs) ; i++)
	AppendMenu(editmenu,edit_funcs[i].name);
    InsertMenu(editmenu, 0);
    
    optionmenu = NewMenu(4,"\pOptions");
    for (i=0;i<NELEM(optionvars) ; i++) {
	AppendMenu(optionmenu,optionvars[i].name);
	*optionvars[i].var = TRUE;
	CheckItem(optionmenu, i + 1, TRUE);
    }
    AppendMenu(optionmenu,"\p-;Verify None;Verify All");
    InsertMenu(optionmenu, 0);
    
    DrawMenuBar();
    
    Firstsavedisk = -SFSaveDisk;
    Firstcurdirstore = CurDirStore;
    Secondsavedisk = -SFSaveDisk;
    Secondcurdirstore = CurDirStore;
}

void doitem(choice)
long choice;
{
    Str255 apname;

    switch (HiWord(choice)) {
	case 1:
	    if (LoWord(choice) <= NELEM(ap_funcs)){
		(*ap_funcs[LoWord(choice) - 1].ptr)();
	    } else {
		GetItem(drvrmenu,LoWord(choice),&apname);
		OpenDeskAcc(apname);
	    }
	    break;
	case 2:
	    (*file_funcs[LoWord(choice) - 1].ptr)();
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

main()
{
    EventRecord evt;
    long mitem;
    WindowPtr winp;
    
    init();
    for (;;) {
        while (!GetNextEvent(everyEvent, &evt))
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
