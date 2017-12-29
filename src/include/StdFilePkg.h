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

#if !defined(SFSaveDisk)
extern INTEGER SFSaveDisk;
extern LONGINT CurDirStore;
#endif

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

typedef ProcPtr FileFilterYDProcPtr;
typedef ProcPtr DlgHookYDProcPtr;
typedef ProcPtr ModalFilterYDProcPtr;
typedef ProcPtr ActivateYDProcPtr;

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

#if !defined(UNIV)
#define UNIV
#endif

extern pascal void C_ROMlib_filebox(DialogPeek dp, INTEGER which);
extern pascal void P_ROMlib_filebox(DialogPeek dp, INTEGER which);

extern pascal trap void C_SFPPutFile(Point p, StringPtr prompt,
                                     StringPtr name, ProcPtr dh, SFReply *rep, INTEGER dig, ProcPtr fp);
PASCAL_FUNCTION(SFPPutFile);

extern pascal trap void C_SFPutFile(Point p, StringPtr prompt, StringPtr name,
                                    ProcPtr dh, SFReply *rep);
PASCAL_FUNCTION(SFPutFile);

extern pascal trap void C_SFPGetFile(Point p, StringPtr prompt, ProcPtr filef,
                                     INTEGER numt, GUEST<SFTypeList> tl, ProcPtr dh, SFReply *rep,
                                     INTEGER dig, ProcPtr fp);
PASCAL_FUNCTION(SFPGetFile);

extern pascal trap void C_SFGetFile(Point p, StringPtr prompt, ProcPtr filef,
                                    INTEGER numt, GUEST<SFTypeList> tl, ProcPtr dh, SFReply *rep);
PASCAL_FUNCTION(SFGetFile);

extern pascal trap void C_StandardGetFile(ProcPtr filef, INTEGER numt,
                                          GUEST<SFTypeList> tl,
                                          StandardFileReply *replyp);
PASCAL_FUNCTION(StandardGetFile);

extern pascal trap void C_StandardPutFile(Str255 prompt, Str255 defaultname,
                                          StandardFileReply *replyp);
PASCAL_FUNCTION(StandardPutFile);

extern pascal trap void C_CustomPutFile(Str255 prompt, Str255 defaultName,
                                        StandardFileReply *replyp,
                                        INTEGER dlgid, Point where,
                                        DlgHookYDProcPtr dlghook,
                                        ModalFilterYDProcPtr filterproc,
                                        Ptr activeList,
                                        ActivateYDProcPtr activateproc,
                                        UNIV Ptr yourdatap);
PASCAL_FUNCTION(CustomPutFile);

extern pascal trap void C_CustomGetFile(FileFilterYDProcPtr filefilter,
                                        INTEGER numtypes,
                                        GUEST<SFTypeList> typelist,
                                        StandardFileReply *replyp,
                                        INTEGER dlgid, Point where,
                                        DlgHookYDProcPtr dlghook,
                                        ModalFilterYDProcPtr filterproc,
                                        Ptr activeList,
                                        ActivateYDProcPtr activateproc,
                                        UNIV Ptr yourdatap);
PASCAL_FUNCTION(CustomGetFile);

}

#endif /* __STDFILE__ */
