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
#define putDlgID (-3999)

#define putSave 1
#define putCancel 2
#define putEject 5
#define putDrive 6
#define putName 7

#define getDlgID (-4000)

#define getOpen 1
#define getCancel 3
#define getEject 5
#define getDrive 6
#define getNmList 7
#define getScroll 8

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
extern pascal trap void P_SFPPutFile(Point p, StringPtr prompt,
                                     StringPtr name, ProcPtr dh, SFReply *rep, INTEGER dig, ProcPtr fp);

extern pascal trap void C_SFPutFile(Point p, StringPtr prompt, StringPtr name,
                                    ProcPtr dh, SFReply *rep);
extern pascal trap void P_SFPutFile(Point p, StringPtr prompt, StringPtr name,
                                    ProcPtr dh, SFReply *rep);

extern pascal trap void C_SFPGetFile(Point p, StringPtr prompt, ProcPtr filef,
                                     INTEGER numt, GUEST<SFTypeList> tl, ProcPtr dh, SFReply *rep,
                                     INTEGER dig, ProcPtr fp);

extern pascal trap void C_SFGetFile(Point p, StringPtr prompt, ProcPtr filef,
                                    INTEGER numt, GUEST<SFTypeList> tl, ProcPtr dh, SFReply *rep);

extern pascal trap void C_StandardGetFile(ProcPtr filef, INTEGER numt,
                                          GUEST<SFTypeList> tl,
                                          StandardFileReply *replyp);

extern pascal trap void C_StandardPutFile(Str255 prompt, Str255 defaultname,
                                          StandardFileReply *replyp);

extern pascal trap void C_CustomPutFile(Str255 prompt, Str255 defaultName,
                                        StandardFileReply *replyp,
                                        INTEGER dlgid, Point where,
                                        DlgHookYDProcPtr dlghook,
                                        ModalFilterYDProcPtr filterproc,
                                        Ptr activeList,
                                        ActivateYDProcPtr activateproc,
                                        UNIV Ptr yourdatap);

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

}

#endif /* __STDFILE__ */
