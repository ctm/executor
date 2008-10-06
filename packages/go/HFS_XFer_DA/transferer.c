#include "xfer.h"

#define BUFSIZE (8L * 1024)

#include "sharedtransfer.proto.h"
#include "transferer.proto.h"
#include "HFS_Xfer.proto.h"
#include "alerts.proto.h"

static char buf[BUFSIZE];

void
getnameandfromdirid (Str255 * sp, LONGINT * fromdirid)
{
  CInfoPBRec cpb;

  if (globalreply.fName[0] == 0)
    {
      cpb.hFileInfo.ioFDirIndex = -1;
      cpb.hFileInfo.ioDirID = globalreply.fType;
      cpb.hFileInfo.ioVRefNum = -SFSaveDisk;
      cpb.hFileInfo.ioNamePtr = *sp;
      PBGetCatInfo (&cpb, false);
      if (sp[0] != 0)
	*fromdirid = cpb.hFileInfo.ioFlParID;
      else
	*fromdirid = CurDirStore;
    }
  else
    {
      *fromdirid = CurDirStore;
      BlockMove (globalreply.fName, *sp, globalreply.fName[0] + 1);
    }
}

void
dotransfer (INTEGER (*fp) (short, short, long,
			   long, Str255, char))
{
  long fromdirid;
  Str255 sp;

  getnameandfromdirid (&sp, &fromdirid);

  commontrans (fromdirid, destdir, -SFSaveDisk, -destdisk, sp, fp);
/* ignore return value of commontrans */
}

INTEGER
docopydisk (DialogPtr dp)
{
  Str255 s;
  ParamBlockRec pb;
  OSErr err;

  if (!caneject (dp))
    {
      ParamText ((StringPtr) "\pOnly floppies may be copied with this option.", 0, 0, 0);
      StopAlert (ONEPARAMALERT, (ProcPtr) 0);
      return false;
    }
  if (SFSaveDisk == destdisk)
    {
      warnaboutincest ();
      return false;
    }
  pb.volumeParam.ioCompletion = 0;
  pb.volumeParam.ioNamePtr = s;
  pb.volumeParam.ioVRefNum = -SFSaveDisk;
  pb.volumeParam.ioVolIndex = 0;
  CHECKR (PBGetVInfo, &pb);

  return commontrans (1L, destdir, -SFSaveDisk, -destdisk, s, copy1file);
}
