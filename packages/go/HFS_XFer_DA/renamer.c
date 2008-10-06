#include "xfer.h"

#include "renamer.proto.h"
#include "transferer.proto.h"
#include "alerts.proto.h"
#include "delete.proto.h"
#include "dircreate.proto.h"

OSErr
renamefile (DialogPtr dp)
{
  Str255 s, s2;
  INTEGER type;
  HParamBlockRec hpb;
  CInfoPBRec cpb;
  OSErr err;
  Rect r;
  Handle h;
  INTEGER save_verify_flags;
  LONGINT fromdirid;

  getnameandfromdirid (&s, &fromdirid);
  hpb.fileParam.ioVRefNum = -SFSaveDisk;
  hpb.fileParam.ioDirID = fromdirid;
  hpb.fileParam.ioCompletion = 0;
  hpb.fileParam.ioNamePtr = s;
  GetDItem (dp, TEXTITEM, &type, &h, &r);
  GetIText (h, s2);
  hpb.ioParam.ioMisc = (LONGORPTR) s2;
  err = PBHRename (&hpb, false);
  if (err == dupFNErr)
    {
      cpb.hFileInfo.ioFDirIndex = 0;
      cpb.hFileInfo.ioNamePtr = s2;
      cpb.hFileInfo.ioVRefNum = -SFSaveDisk;
      cpb.hFileInfo.ioDirID = fromdirid;

      PBGetCatInfo (&cpb, false);
      if (cpb.hFileInfo.ioFlAttrib & ISDIRMASK)
	{
	  if (!(verify_flags & VERIFY_OVERWRITE_FOLDER) ||
	      ask ("\poverwrite directory", s2) == OK)
	    {
	      save_verify_flags = verify_flags;
	      verify_flags &= ~(VERIFY_DELETE_FILE | VERIFY_DELETE_FOLDER);
	      delete1file (-SFSaveDisk, fromdirid, s2);
	      verify_flags = save_verify_flags;
	      err = PBHRename (&hpb, false);
	    }
	}
      else
	{
	  if (!(verify_flags & VERIFY_OVERWRITE_FILE) ||
	      ask ("\poverwrite file", s2) == OK)
	    {
	      save_verify_flags = verify_flags;
	      verify_flags &= ~(VERIFY_DELETE_FILE | VERIFY_DELETE_FOLDER);
	      delete1file (-SFSaveDisk, fromdirid, s2);
	      verify_flags = save_verify_flags;
	      err = PBHRename (&hpb, false);
	    }
	}
    }
  else if (err != noErr)
    doerror (err, "\pPBHRename");
  return err;
}

INTEGER
donewdir (DialogPtr dp)
{
  Str255 s;
  INTEGER type;
  Rect r;
  Handle h;

  GetDItem (dp, TEXTITEM, &type, &h, &r);
  GetIText (h, s);
  createdir (-SFSaveDisk, CurDirStore, s);
  return 101;
}
