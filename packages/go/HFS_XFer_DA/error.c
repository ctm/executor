#include "xfer.h"

#define OURERRORID	404
#define OURBADERRID 405

#include "error.proto.h"

PUBLIC void
errormessage (StringPtr msg, alerttype severity)
{
  short res;
#if 1
  DebugStr (msg);
  return;
#endif
  ParamText (msg, (StringPtr) "\p", (StringPtr) "\p", (StringPtr) "\p");
  switch (severity)
    {
    case NOTE:
      res = NoteAlert (OURERRORID, (ProcPtr) 0);
      break;
    case CAUTION:
      res = CautionAlert (OURERRORID, (ProcPtr) 0);
      break;
    case STOP:
      res = StopAlert (OURBADERRID, (ProcPtr) 0);
      break;
    default:
      res = StopAlert (OURBADERRID, (ProcPtr) 0);
      break;
    }
#if		defined(OUTDATEDCODE)
  if (res == Cancel)
    OurExit ();
#else
  ExitToShell ();
#endif
}
