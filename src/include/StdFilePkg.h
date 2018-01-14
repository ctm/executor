#if !defined(__STDFILE__)
#define __STDFILE__

#include "DialogMgr.h"
#include "FileMgr.h"

/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
enum
{
    putDlgID = (-3999),
};

enum
{
    putSave = 1,
    putCancel = 2,
    putEject = 5,
    putDrive = 6,
    putName = 7,
};

enum
{
    getDlgID = (-4000),
};

enum
{
    getOpen = 1,
    getCancel = 3,
    getEject = 5,
    getDrive = 6,
    getNmList = 7,
    getScroll = 8,
};

struct SFReply
{
    GUEST_STRUCT;
    GUEST<BOOLEAN> good;
    GUEST<BOOLEAN> copy;
    GUEST<OSType> fType;
    GUEST<INTEGER> vRefNum;
    GUEST<INTEGER> version;
    GUEST<Str63> fName;
};

typedef OSType SFTypeList[4];

const LowMemGlobal<INTEGER> SFSaveDisk { 0x214 }; // StdFilePkg IMIV-72 (true);
const LowMemGlobal<LONGINT> CurDirStore { 0x398 }; // StdFilePkg IMIV-72 (true);

struct StandardFileReply
{
    GUEST_STRUCT;
    GUEST<BOOLEAN> sfGood;
    GUEST<BOOLEAN> sfReplacing;
    GUEST<OSType> sfType;
    GUEST<FSSpec> sfFile;
    GUEST<ScriptCode> sfScript;
    GUEST<INTEGER> sfFlags;
    GUEST<BOOLEAN> sfIsFolder;
    GUEST<BOOLEAN> sfIsVolume;
    GUEST<LONGINT> sfReserved1;
    GUEST<INTEGER> sfReserved2;
};

typedef UPP<INTEGER(INTEGER item, DialogPtr theDialog)> DlgHookProcPtr;
typedef UPP<Boolean(CInfoPBPtr pb)> FileFilterProcPtr;
typedef UPP<INTEGER(INTEGER item, DialogPtr theDialog, void *yourDataPtr)> DlgHookYDProcPtr;
/* ModalFilterYDProcPtr moved to Dialogs.h */
typedef UPP<Boolean(CInfoPBPtr pb, void *yourDataPtr)> FileFilterYDProcPtr;
typedef UPP<void(DialogPtr theDialog, INTEGER itemNo, Boolean activating, void *yourDataPtr)> ActivateYDProcPtr;

enum
{
    sfItemOpenButton = 1,
    sfItemCancelButton = 2,
    sfItemBalloonHelp = 3,
    sfItemVolumeUser = 4,
    sfItemEjectButton = 5,
    sfItemDesktopButton = 6,
    sfItemFileListUser = 7,
    sfItemPopUpMenuUser = 8,
    sfItemDividerLinePict = 9,

    sfItemFileNameTextEdit = 10, /* Put only */
    sfItemPromptStaticText = 11,
    sfItemNewFolderUser = 12,
};

enum
{
    sfPutDialogID = -6043,
    sfGetDialogID = -6042
};

extern void C_ROMlib_filebox(DialogPeek dp, INTEGER which);
PASCAL_FUNCTION(ROMlib_filebox);

extern void C_SFPPutFile(Point p, StringPtr prompt,
                                     StringPtr name, DlgHookProcPtr dh, SFReply *rep, INTEGER dig, ModalFilterProcPtr fp);
PASCAL_SUBTRAP(SFPPutFile, 0xA9EA, Pack3);

extern void C_SFPutFile(Point p, StringPtr prompt, StringPtr name,
                                    DlgHookProcPtr dh, SFReply *rep);
PASCAL_SUBTRAP(SFPutFile, 0xA9EA, Pack3);

extern void C_SFPGetFile(Point p, StringPtr prompt, FileFilterProcPtr filef,
                                     INTEGER numt, GUEST<SFTypeList> tl, DlgHookProcPtr dh, SFReply *rep,
                                     INTEGER dig, ModalFilterProcPtr fp);
PASCAL_SUBTRAP(SFPGetFile, 0xA9EA, Pack3);

extern void C_SFGetFile(Point p, StringPtr prompt, FileFilterProcPtr filef,
                                    INTEGER numt, GUEST<SFTypeList> tl, DlgHookProcPtr dh, SFReply *rep);
PASCAL_SUBTRAP(SFGetFile, 0xA9EA, Pack3);

extern void C_StandardGetFile(FileFilterProcPtr filef, INTEGER numt,
                                          GUEST<SFTypeList> tl,
                                          StandardFileReply *replyp);
PASCAL_SUBTRAP(StandardGetFile, 0xA9EA, Pack3);

extern void C_StandardPutFile(Str255 prompt, Str255 defaultname,
                                          StandardFileReply *replyp);
PASCAL_SUBTRAP(StandardPutFile, 0xA9EA, Pack3);

extern void C_CustomPutFile(Str255 prompt, Str255 defaultName,
                                        StandardFileReply *replyp,
                                        INTEGER dlgid, Point where,
                                        DlgHookYDProcPtr dlghook,
                                        ModalFilterYDProcPtr filterproc,
                                        Ptr activeList,
                                        ActivateYDProcPtr activateproc,
                                        void *yourdatap);
PASCAL_SUBTRAP(CustomPutFile, 0xA9EA, Pack3);

extern void C_CustomGetFile(FileFilterYDProcPtr filefilter,
                                        INTEGER numtypes,
                                        GUEST<SFTypeList> typelist,
                                        StandardFileReply *replyp,
                                        INTEGER dlgid, Point where,
                                        DlgHookYDProcPtr dlghook,
                                        ModalFilterYDProcPtr filterproc,
                                        Ptr activeList,
                                        ActivateYDProcPtr activateproc,
                                        void *yourdatap);
PASCAL_SUBTRAP(CustomGetFile, 0xA9EA, Pack3);
}

#endif /* __STDFILE__ */
