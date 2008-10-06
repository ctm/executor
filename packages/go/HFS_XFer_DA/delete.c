#include "xfer.h"

#include "delete.proto.h"
#include "alerts.proto.h"

short verify_flags;

short
delete1file (short vrn, long dirid, Str255 s)
{
  HParamBlockRec hpb;
  OSErr err;
  Str255 s2;
  long subdirid;
  short save_verify_flags;
  ParamBlockRec pb;
  int bailcount;

  pb.volumeParam.ioVolIndex = 0;
  pb.volumeParam.ioVRefNum = vrn;
  pb.volumeParam.ioNamePtr = 0;
  CHECKR (PBGetVInfo, &pb);
  if (pb.volumeParam.ioVAtrb & VOLLOCKEDMASK)
    {
      doerror (vLckdErr, (StringPtr) 0);
      return false;
    }
  hpb.fileParam.ioVRefNum = vrn;
  hpb.fileParam.ioDirID = dirid;
  hpb.fileParam.ioFDirIndex = 0;
  hpb.fileParam.ioCompletion = 0;
  hpb.fileParam.ioNamePtr = s;
  CHECKR (PBGetCatInfo, (CInfoPBPtr) & hpb);
#if 0
/* Why shouldn't you be able to delete invisible files? */
  if (hpb.fileParam.ioFlFndrInfo.fdFlags & fInvisible)
/*-->*/ return false;
#endif /* 0 */
  if (hpb.fileParam.ioFlAttrib & ISDIRMASK)
    {
#ifdef THINK_C
      if ((verify_flags & VERIFY_DELETE_FOLDER) &&
	  (ask ((StringPtr) "\pdelete folder", s) == Cancel))
#else
      if ((verify_flags & VERIFY_DELETE_FOLDER) &&
	  (ask ("\pdelete folder", s) == cancel))
#endif
/*-->*/ return false;
      save_verify_flags = verify_flags;
      verify_flags &= ~(VERIFY_DELETE_FOLDER | VERIFY_DELETE_FILE);
      subdirid = hpb.fileParam.ioDirID;
      hpb.fileParam.ioNamePtr = s2;
      for (hpb.fileParam.ioFDirIndex = 1, bailcount = 0; err != fnfErr && ++bailcount != 3;)
	{
	  hpb.fileParam.ioDirID = subdirid;
	  err = PBGetCatInfo ((CInfoPBPtr) & hpb, false);
#if 1
	  if (err != noErr && err != fnfErr)
	    doerror (err, (StringPtr) 0);
#endif /* 0 */
	  if (err)
	    hpb.fileParam.ioFDirIndex++;
	  else
	    {
	      bailcount = 0;
	      delete1file (vrn, subdirid, s2);
	    }
	}
      verify_flags = save_verify_flags;
      hpb.fileParam.ioNamePtr = s;
    }
  else
    {
      if ((verify_flags & VERIFY_DELETE_FILE) && (ask ((StringPtr) "\pdelete file", s) == cancel))
/*-->*/ return false;
    }
  hpb.fileParam.ioDirID = dirid;
  CHECKR (PBHDelete, &hpb);
  return true;
}
