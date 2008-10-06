#include "xfer.h"

#include "alerts.proto.h"
#include "dircreate.proto.h"

OSErr
createdir (short volume, long dirid, StringPtr s)
{
  HParamBlockRec hpb;
  OSErr err;

  hpb.ioParam.ioVRefNum = volume;
  hpb.fileParam.ioDirID = dirid;
  hpb.ioParam.ioNamePtr = (StringPtr) s;
  err = PBDirCreate (&hpb, false);
  if (err == dupFNErr)
    {
      ParamText ((StringPtr) "\pThat name is already in use.", 0, 0, 0);
#ifdef THINK_C
      StopAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
#else
      StopAlert (FOUR_PARAM_ALERT, (ModalFilterUPP) 0);
#endif
    }
  else if (err != noErr)
    doerror (err, (StringPtr) "\pPBDirCreate");
  return err;
}
