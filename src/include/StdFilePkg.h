#if !defined (__STDFILE__)
#define __STDFILE__

#include "DialogMgr.h"
#include "FileMgr.h"

/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: StdFilePkg.h 63 2004-12-24 18:19:43Z ctm $
 */

#define putDlgID	(-3999)

#define putSave		1
#define putCancel	2
#define putEject	5
#define putDrive	6
#define putName		7

#define getDlgID	(-4000)

#define getOpen		1
#define getCancel	3
#define getEject	5
#define getDrive	6
#define getNmList	7
#define getScroll	8

typedef struct PACKED {
  BOOLEAN good;
  BOOLEAN copy;
  OSType fType;
  INTEGER vRefNum;
  INTEGER version;
  Str63 fName;
} SFReply;

typedef OSType SFTypeList[4];

#if !defined (SFSaveDisk)
extern INTEGER 	SFSaveDisk;
extern LONGINT 	CurDirStore;
#endif

typedef struct PACKED
{
  BOOLEAN sfGood;
  BOOLEAN sfReplacing;
  OSType sfType;
  FSSpec sfFile;
  ScriptCode sfScript;
  INTEGER sfFlags;
  BOOLEAN sfIsFolder;
  BOOLEAN sfIsVolume;
  LONGINT sfReserved1;
  INTEGER sfReserved2;
} StandardFileReply;

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

enum { sfPutDialogID = -6043, sfGetDialogID = -6042 };

#if !defined (UNIV)
#define UNIV
#endif

#if !defined (__STDC__)

extern pascal void ROMlib_filebox();
extern void SFPPutFile();
extern void SFPutFile();
extern void SFPGetFile();
extern void SFGetFile();

#else /* __STDC__ */

extern pascal void C_ROMlib_filebox( DialogPeek dp, INTEGER which );
extern pascal void P_ROMlib_filebox( DialogPeek dp, INTEGER which );

extern pascal trap void C_SFPPutFile( Point p, StringPtr prompt,
	   StringPtr name, ProcPtr dh, SFReply *rep, INTEGER dig, ProcPtr fp );
extern pascal trap void P_SFPPutFile( Point p, StringPtr prompt,
	   StringPtr name, ProcPtr dh, SFReply *rep, INTEGER dig, ProcPtr fp );

extern pascal trap void C_SFPutFile( Point p, StringPtr prompt, StringPtr name,
						    ProcPtr dh, SFReply *rep );
extern pascal trap void P_SFPutFile( Point p, StringPtr prompt, StringPtr name,
						    ProcPtr dh, SFReply *rep );

extern pascal trap void C_SFPGetFile( Point p, StringPtr prompt, ProcPtr filef,
		         INTEGER numt, SFTypeList tl, ProcPtr dh, SFReply *rep,
						     INTEGER dig, ProcPtr fp );
extern pascal trap void P_SFPGetFile( Point p, StringPtr prompt, ProcPtr filef,
		         INTEGER numt, SFTypeList tl, ProcPtr dh, SFReply *rep,
						     INTEGER dig, ProcPtr fp );

extern pascal trap void C_SFGetFile( Point p, StringPtr prompt, ProcPtr filef,
		       INTEGER numt, SFTypeList tl, ProcPtr dh, SFReply *rep );
extern pascal trap void P_SFGetFile( Point p, StringPtr prompt, ProcPtr filef,
		       INTEGER numt, SFTypeList tl, ProcPtr dh, SFReply *rep );

extern pascal trap void C_StandardGetFile (ProcPtr filef, INTEGER numt,
					   SFTypeList tl,
					   StandardFileReply *replyp);

extern pascal trap void C_StandardPutFile (Str255 prompt, Str255 defaultname,
					   StandardFileReply *replyp);

extern pascal trap void C_CustomPutFile (Str255 prompt, Str255 defaultName,
					 StandardFileReply *replyp,
					 INTEGER dlgid, Point where,
					 DlgHookYDProcPtr dlghook,
					 ModalFilterYDProcPtr filterproc,
					 Ptr activeList,
					 ActivateYDProcPtr activateproc,
					 UNIV Ptr yourdatap);

extern pascal trap void C_CustomGetFile (FileFilterYDProcPtr filefilter,
					 INTEGER numtypes,
					 SFTypeList typelist,
					 StandardFileReply *replyp,
					 INTEGER dlgid, Point where,
					 DlgHookYDProcPtr dlghook,
					 ModalFilterYDProcPtr filterproc,
					 Ptr activeList,
					 ActivateYDProcPtr activateproc,
					 UNIV Ptr yourdatap);

#endif /* __STDC__ */


#endif /* __STDFILE__ */
