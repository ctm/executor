#if !defined (_PPC_STUBS_H_)
#define _PPC_STUBS_H_

#include "StdFilePkg.h"
#include "HelpMgr.h"
#include "ListMgr.h"
#include "ScriptMgr.h"

typedef uint32 PointAsLong;

typedef struct
{
  INTEGER dig;
  ProcPtr fp;
}
sfpgetfile_aixtosysv4;

typedef struct
{
  INTEGER proc;
  INTEGER variant;
}
hmshowmenuballoon_aixtosysv4;

typedef struct
{
  INTEGER *theprocp;
  INTEGER *variantp;
  HMMessageRecord *helpmsgp;
  INTEGER *count;
}
hmgetindhelpmsg_aixtosysv4;

typedef struct
{
  INTEGER procid;
  LONGINT rc;
}
newcontrol_aixtosysv4;

typedef struct
{
  LONGINT l;
  Handle h;
}
newcdialog_aixtosysv4;

typedef struct
{
  LONGINT rc;
  Handle items;
}
newdialog_aixtosysv4;

typedef struct
{
  Fixed *lsb_array;
  Rect *bounds_array;
}
outlinemetrics_aixtosysv4;

typedef struct
{
  BOOLEAN scrollh;
  BOOLEAN scrollv;
}
lnew_aixtosysv4;

typedef struct
{
  Point numer;
  Point denom;
}
pixeltochar_aixtosysv4;

typedef struct
{
  Ptr activeList;
  ActivateYDProcPtr activateproc;
  Ptr yourdatap;
}
customputfile_aixtosysv4;

typedef struct
{
  ModalFilterYDProcPtr filterproc;
  Ptr activeList;
  ActivateYDProcPtr activateproc;
  UNIV Ptr yourdatap;  
}
customgetfile_aixtosysv4;

extern void SFPGetFile_AIX (PointAsLong pal, StringPtr prompt, ProcPtr filef,
			    INTEGER numt, SFTypeList tl, ProcPtr dh,
			    SFReply *rep, INTEGER dig, ProcPtr fp);

extern void SFPGetFile_SYSV4 (PointAsLong pal, StringPtr prompt, ProcPtr filef,
			      INTEGER numt, SFTypeList tl, ProcPtr dh,
			      SFReply *rep, const sfpgetfile_aixtosysv4 *pbp);

extern OSErr HMShowMenuBalloon_AIX (INTEGER item, INTEGER menuid,
				    LONGINT flags, LONGINT itemreserved,
				    PointAsLong pal, RectPtr alternaterectp,
				    Ptr tipproc, INTEGER proc,
				    INTEGER variant);

extern OSErr HMShowMenuBalloon_SYSV4 (INTEGER item, INTEGER menuid,
				      LONGINT flags, LONGINT itemreserved,
				      PointAsLong pal, RectPtr alternaterectp,
				      Ptr tipproc,
				      const
				      hmshowmenuballoon_aixtosysv4 *argp);

extern OSErr HMGetIndHelpMsg_AIX (ResType type, INTEGER resid, INTEGER msg,
				  INTEGER state, LONGINT *options,
				  PointAsLong pal, Rect *altrectp,
				  INTEGER *theprocp, INTEGER *variantp,
				  HMMessageRecord *helpmsgp, INTEGER *count);

extern OSErr HMGetIndHelpMsg_SYSV4 (ResType type, INTEGER resid, INTEGER msg,
				    INTEGER state, LONGINT *options,
				    PointAsLong pal, Rect *altrectp,
				    const hmgetindhelpmsg_aixtosysv4 *pbp);

extern ControlHandle NewControl_AIX (WindowPtr wst, Rect *r, StringPtr title,
				     BOOLEAN vis, INTEGER value, INTEGER min,
				     INTEGER max, INTEGER procid, LONGINT rc);

extern ControlHandle NewControl_SYSV4 (WindowPtr wst, Rect *r, StringPtr title,
				       BOOLEAN vis, INTEGER value, INTEGER min,
				       INTEGER max,
				       const newcontrol_aixtosysv4 *pbp);

extern CDialogPtr NewCDialog_AIX (Ptr p, Rect *rp, StringPtr sp, BOOLEAN b1,
				  INTEGER i, WindowPtr wp, BOOLEAN b2,
				  LONGINT l, Handle h);

extern CDialogPtr NewCDialog_SYSV4 (Ptr p, Rect *rp, StringPtr sp, BOOLEAN b1,
				    INTEGER i, WindowPtr wp, BOOLEAN b2,
				    const newcdialog_aixtosysv4 *pbp);

extern DialogPtr NewDialog_AIX (Ptr dst,  Rect *r, StringPtr tit, BOOLEAN vis,
				INTEGER procid, WindowPtr behind,
				BOOLEAN gaflag, LONGINT rc, Handle items);

extern DialogPtr NewDialog_SYSV4 (Ptr dst,  Rect *r, StringPtr tit,
				  BOOLEAN vis, INTEGER procid,
				  WindowPtr behind, BOOLEAN gaflag,
				  const newdialog_aixtosysv4 *pbp);

extern OSErr OutlineMetrics_AIX (int16 byte_count, Ptr text,
				 PointAsLong numerAL, PointAsLong denomAL,
				 int16 *y_max, int16 *y_min, Fixed *aw_array,
				 Fixed *lsb_array, Rect *bounds_array);

extern OSErr OutlineMetrics_SYSV4 (int16 byte_count, Ptr text,
				   PointAsLong numerAL, PointAsLong denomAL,
				   int16 *y_max, int16 *y_min, Fixed *aw_array,
				   const outlinemetrics_aixtosysv4 *pbp);

extern ListHandle LNew_AIX (Rect *rview, Rect *bounds, PointAsLong pal,
			    INTEGER proc, WindowPtr wind, BOOLEAN draw,
			    BOOLEAN grow, BOOLEAN scrollh, BOOLEAN scrollv);

extern ListHandle LNew_SYSV4 (Rect *rview, Rect *bounds, PointAsLong pal,
			      INTEGER proc, WindowPtr wind, BOOLEAN draw,
			      BOOLEAN grow,
			      const lnew_aixtosysv4 *pbp);

extern INTEGER PixelToChar_AIX (Ptr textBuf, LONGINT textLen, Fixed slop,
				Fixed pixelWidth, BOOLEAN *leadingEdgep,
				Fixed *widthRemainingp,
				JustStyleCode styleRunPosition,
				PointAsLong numerAL, PointAsLong denomAL);

extern INTEGER PixelToChar_SYSV4 (Ptr textBuf, LONGINT textLen, Fixed slop,
				  Fixed pixelWidth, BOOLEAN *leadingEdgep,
				  Fixed *widthRemainingp,
				  JustStyleCode styleRunPosition,
				  const pixeltochar_aixtosysv4 *pbp);

extern void CustomPutFile_AIX (Str255 prompt, Str255 defaultName,
			       StandardFileReply *replyp, INTEGER dlgid,
			       PointAsLong pal, DlgHookYDProcPtr dlghook,
			       ModalFilterYDProcPtr filterproc,
			       Ptr activeList,
			       ActivateYDProcPtr activateproc,
			       UNIV Ptr yourdatap);

extern void CustomPutFile_SYSV4 (Str255 prompt, Str255 defaultName,
				 StandardFileReply *replyp, INTEGER dlgid,
				 PointAsLong pal, DlgHookYDProcPtr dlghook,
				 ModalFilterYDProcPtr filterproc,
				 const customputfile_aixtosysv4 *pbp);

extern void CustomGetFile_AIX (FileFilterYDProcPtr filefilter,
			       INTEGER numtypes, SFTypeList typelist,
			       StandardFileReply *replyp, INTEGER dlgid,
			       PointAsLong pal, DlgHookYDProcPtr dlghook,
			       ModalFilterYDProcPtr filterproc,
			       Ptr activeList,
			       ActivateYDProcPtr activateproc,
			       UNIV Ptr yourdatap);

extern void CustomGetFile_SYSV4 (FileFilterYDProcPtr filefilter,
				 INTEGER numtypes, SFTypeList typelist,
				 StandardFileReply *replyp, INTEGER dlgid,
				 PointAsLong pal, DlgHookYDProcPtr dlghook,
				 const customgetfile_aixtosysv4 *pbp);

#endif
