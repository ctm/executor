/* Copyright 1989 - 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in DeviceMgr.h (DO NOT DELETE THIS LINE) */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_device[] =
		"$Id: device.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "DeviceMgr.h"
#include "FileMgr.h"
#include "MemoryMgr.h"
#include "OSUtil.h"
#include "ResourceMgr.h"
#include "MenuMgr.h"
#include "ToolboxEvent.h"
#include "rsys/glue.h"
#include "rsys/mman.h"
#include "rsys/device.h"
#include "rsys/file.h"
#include "rsys/serial.h"

using namespace Executor;
using namespace ByteSwap;

/*
 * NOTE:  The device manager now executes "native code" and code read
 *	  in from resources.  The latter is "RAM" based, the former is
 *	  our version of "ROM" based.  We use the same bit that the Mac
 *	  uses to distinguish between the two, but our implementation of
 *	  ROM based is incompatible with theirs.  NOTE: even in our
 *	  incompatible ROM based routines, we still byte swap the pointers
 *	  that get us to the routines.
 */

A4(PUBLIC, OSErr, ROMlib_dispatch, ParmBlkPtr, p,		/* INTERNAL */
		    BOOLEAN, async, DriverRoutineType, routine, INTEGER, trapn)
{
	int devicen;
	ramdriverhand ramdh;
	DCtlHandle h;
	OSErr retval;
	typedef OSErr (*devfp_t)(ParmBlkPtr, DCtlPtr);
	devfp_t procp;
	
	devicen = -BigEndianValue(p->cntrlParam.ioCRefNum) - 1;
	if (devicen < 0 || devicen >= NDEVICES)
		retval = badUnitErr;
	else if (UTableBase == (DCtlHandlePtr) (long) CLC(0xFFFFFFFF) ||
			 (h = MR(MR(UTableBase)[devicen].p)) == 0 || (*h).p == 0)
		retval =  unitEmptyErr;
	else {
		HLock((Handle) h);
		p->ioParam.ioTrap = BigEndianValue(trapn);
		if (async)
			p->ioParam.ioTrap |= CWC(asyncTrpBit);
		else
			p->ioParam.ioTrap |= CWC(noQueueBit);
		if (!(HxX(h, dCtlFlags) & CWC(RAMBASEDBIT))) {
			switch (routine) {
				case Open:
					retval = (* (devfp_t) MR(HxP(h, dCtlDriver)->udrvrOpen))
					(p, STARH(h));
					break;
				case Prime:
					retval = (* (devfp_t) MR(HxP(h, dCtlDriver)->udrvrPrime))
					(p, STARH(h));
					break;
				case Ctl:
					retval = (* (devfp_t) MR(HxP(h, dCtlDriver)->udrvrCtl))
					(p, STARH(h));
					break;
				case Stat:
					retval = (* (devfp_t) MR(HxP(h, dCtlDriver)->udrvrStatus))
					(p, STARH(h));
					break;
				case Close:
					retval = (* (devfp_t) MR(HxP(h, dCtlDriver)->udrvrClose))
					(p, STARH(h));
					break;
				default:
					retval = fsDSIntErr;
					break;
			}
		} else {
			ramdh = (ramdriverhand) HxP(h, dCtlDriver);
			LoadResource((Handle) ramdh);
			HLock((Handle) ramdh);
			switch (routine) {
				case Open:
					procp = (devfp_t)(STARH(ramdh) + Hx(ramdh, drvrOpen));
					break;
				case Prime:
					procp = (devfp_t)(STARH(ramdh) + Hx(ramdh, drvrPrime));
					break;
				case Ctl:
					procp = (devfp_t)(STARH(ramdh) + Hx(ramdh, drvrCtl));
					break;
				case Stat:
					procp = (devfp_t)(STARH(ramdh) + Hx(ramdh, drvrStatus));
					break;
				case Close:
					procp = (devfp_t)(STARH(ramdh) + Hx(ramdh, drvrClose));
					break;
				default:
					procp = 0;
					break;
			}
			if (procp) {
				LONGINT saved1, saved2, saved3, saved4, saved5, saved6, saved7,
				savea2, savea3, savea4, savea5, savea6;
				
				savea2 = EM_A2;
				EM_A0 = (LONGINT) (long) US_TO_SYN68K(p);
				EM_A1 = (LONGINT) (long) US_TO_SYN68K(STARH(h));
				EM_A2 = (LONGINT) (long) US_TO_SYN68K(procp);	/* for compatibility with above */
				saved1 = EM_D1;
				saved2 = EM_D2;
				saved3 = EM_D3;
				saved4 = EM_D4;
				saved5 = EM_D5;
				saved6 = EM_D6;
				saved7 = EM_D7;
				savea3 = EM_A3;
				savea4 = EM_A4;
				savea5 = EM_A5;
				savea6 = EM_A6;
				CALL_EMULATOR((syn68k_addr_t) (long) US_TO_SYN68K (procp));
				EM_D1 = saved1;
				EM_D2 = saved2;
				EM_D3 = saved3;
				EM_D4 = saved4;
				EM_D5 = saved5;
				EM_D6 = saved6;
				EM_D7 = saved7;
				EM_A3 = savea3;
				EM_A4 = savea4;
				EM_A5 = savea5;
				EM_A6 = savea6;
				retval = EM_D0;
				EM_A2 = savea2;
			} else
				retval = fsDSIntErr;
			if (routine == Open)
				HxX(h, dCtlFlags) |= CWC(DRIVEROPENBIT);
			else if (routine == Close) {
				HxX(h, dCtlFlags) &= ~CWC(DRIVEROPENBIT);
				HUnlock((Handle) h);
				HUnlock((Handle) ramdh);
				MBarEnable = 0;
				/* NOTE: It's not clear whether we should zero out this
		   field or just check for DRIVEROPEN bit up above and never
		   send messages except open to non-open drivers.  */
				MR(UTableBase)[devicen].p = (DCtlHandle)CLC(0);
			}
			
			if (routine < Close)
				retval = BigEndianValue(p->ioParam.ioResult);	/* see II-193 */
		}
	}
	fs_err_hook (retval);
	return retval;
}

/* PBOpen, PBClose, PBRead and PBWrite are part of the file manager */

A2(PUBLIC, OSErr, PBControl, ParmBlkPtr, pbp, BOOLEAN, a)	/* IMII-186 */
{
  OSErr err;

  err = ROMlib_dispatch(pbp, a, Ctl, 0);
  fs_err_hook (err);
  return err;
}

A2(PUBLIC, OSErr, PBStatus, ParmBlkPtr, pbp, BOOLEAN, a)	/* IMII-186 */
{
  OSErr err;

  err = ROMlib_dispatch(pbp, a, Stat, 0);
  fs_err_hook (err);
  return err;
}

A2(PUBLIC, OSErr, PBKillIO, ParmBlkPtr, pbp, BOOLEAN, a)	/* IMII-187 */
{
  OSErr err;

  pbp->cntrlParam.csCode = CWC(killCode);
  err = ROMlib_dispatch(pbp, a, Ctl, 0);
  fs_err_hook (err);
  return err;
}

/*
 * OpenDriver is defined below ROMlib_driveropen.
 * CloseDriver is defined below ROMlib_driverclose.
 */

/* FSRead, FSWrite are part of the file manager */

A3(PUBLIC, OSErr, Control, INTEGER, rn, INTEGER, code,
						    Ptr, param) /* IMII-179 */
{
    ParamBlockRec pb;
    OSErr err;

    pb.cntrlParam.ioVRefNum = 0;
    pb.cntrlParam.ioCRefNum = BigEndianValue(rn);
    pb.cntrlParam.csCode = BigEndianValue(code);
    if (param)
	BlockMove(param, (Ptr) pb.cntrlParam.csParam,
					 (Size) sizeof(pb.cntrlParam.csParam));
    err = PBControl(&pb, FALSE);
    fs_err_hook (err);
    return err;
}

A3(PUBLIC, OSErr, Status, INTEGER, rn, INTEGER, code, Ptr, param) /* IMII-179 */
{
    ParamBlockRec pb;
    OSErr retval;

    pb.cntrlParam.ioVRefNum = 0;
    pb.cntrlParam.ioCRefNum = BigEndianValue(rn);
    pb.cntrlParam.csCode = BigEndianValue(code);
    retval = PBStatus(&pb, FALSE);
    if (param)
	BlockMove((Ptr) pb.cntrlParam.csParam, param,
					 (Size) sizeof(pb.cntrlParam.csParam));
    fs_err_hook (retval);
    return retval;
}

A1(PUBLIC, OSErr, KillIO, INTEGER, rn)	/* IMII-179 */
{
    ParamBlockRec pb;
    OSErr err;

    pb.cntrlParam.ioCRefNum = BigEndianValue(rn);
    err = PBKillIO(&pb, FALSE);
    fs_err_hook (err);
    return err;
}

A1(PUBLIC, DCtlHandle, GetDCtlEntry, INTEGER, rn)
{
    int devicen;

    devicen = -rn - 1;
    return (devicen < 0 || devicen >= NDEVICES) ?
	       0
	   :
	       MR(MR(UTableBase)[devicen].p);
}

/*
 * ROMlib_driveropen will be called by PBOpen if it encounters a name
 * beginning with * a period.
 */

PUBLIC driverinfo *ROMlib_otherdrivers = 0;	/* for extensibility */

PRIVATE driverinfo knowndrivers[] = {
#if defined (LINUX) || defined (MACOSX_) || defined (MSDOS) || defined (CYGWIN32)
  { (OSErr(*)())ROMlib_serialopen, (OSErr(*)())ROMlib_serialprime, (OSErr(*)())ROMlib_serialctl,
    (OSErr(*)())ROMlib_serialstatus, (OSErr(*)())ROMlib_serialclose, (StringPtr) "\p.AIn",  -6, },

  { (OSErr(*)())ROMlib_serialopen, (OSErr(*)())ROMlib_serialprime, (OSErr(*)())ROMlib_serialctl,
    (OSErr(*)())ROMlib_serialstatus, (OSErr(*)())ROMlib_serialclose, (StringPtr) "\p.AOut", -7, },

  { (OSErr(*)())ROMlib_serialopen, (OSErr(*)())ROMlib_serialprime, (OSErr(*)())ROMlib_serialctl,
    (OSErr(*)())ROMlib_serialstatus, (OSErr(*)())ROMlib_serialclose, (StringPtr) "\p.BIn",  -8, },

  { (OSErr(*)())ROMlib_serialopen, (OSErr(*)())ROMlib_serialprime, (OSErr(*)())ROMlib_serialctl,
    (OSErr(*)())ROMlib_serialstatus, (OSErr(*)())ROMlib_serialclose, (StringPtr) "\p.BOut", -9, },
#endif
};

A2(PUBLIC, OSErr, ROMlib_driveropen, ParmBlkPtr, pbp,		/* INTERNAL */
								   BOOLEAN, a)
{
  driverinfo *dip, *edip;
  OSErr err;
  INTEGER devicen;
  umacdriverptr up;
  DCtlHandle h;
  ramdriverhand ramdh;
  ResType typ;
  BOOLEAN alreadyopen;
  
  ZONE_SAVE_EXCURSION
  (SysZone,
   {
     err = noErr;
	 
     if ((ramdh =
          (ramdriverhand) GetNamedResource(TICK("DRVR"),
                                           MR(pbp->ioParam.ioNamePtr)))) {
            LoadResource((Handle) ramdh);
            GetResInfo((Handle) ramdh, &devicen, &typ, (StringPtr) 0);
            BigEndianInPlace(devicen);
            h = MR(MR(UTableBase)[devicen].p);
            alreadyopen = h && (HxX(h, dCtlFlags) & CWC(DRIVEROPENBIT));
            if (!h && !(h = MR(MR(UTableBase)[devicen].p =
                               RM((DCtlHandle) NewHandle(sizeof(DCtlEntry))))))
              err = MemError();
            else if (!alreadyopen) {
              memset((char *) STARH(h), 0, sizeof(DCtlEntry));
              HxX(h, dCtlDriver)   = (umacdriverptr) RM(ramdh);
              HxX(h, dCtlFlags)    = HxX(ramdh, drvrFlags) | CWC(RAMBASEDBIT);
              HxX(h, dCtlRefNum)   = BigEndianValue(- (devicen + 1));
              HxX(h, dCtlDelay)    = HxX(ramdh, drvrDelay);
              HxX(h, dCtlEMask)    = HxX(ramdh, drvrEMask);
              HxX(h, dCtlMenu)     = HxX(ramdh, drvrMenu);
              if (HxX(h, dCtlFlags) & CWC(NEEDTIMEBIT))
                HxX(h, dCtlCurTicks) = BigEndianValue(TickCount() + Hx(h, dCtlDelay));
              else
                HxX(h, dCtlCurTicks) = CLC(0x7FFFFFFF);
              /*
               * NOTE: this code doesn't check to see if something is already open.
               *	 TODO:  fix this
               */
              pbp->cntrlParam.ioCRefNum = HxX(h, dCtlRefNum);
              err = ROMlib_dispatch(pbp, a, Open, 0);
            } else
            {
              pbp->cntrlParam.ioCRefNum = HxX(h, dCtlRefNum);
              err = noErr;
            }
          } else {
            
            dip = 0;
            if (ROMlib_otherdrivers) {
              for (dip = ROMlib_otherdrivers; dip->open &&
                   !EqualString(dip->name, MR(pbp->ioParam.ioNamePtr), FALSE, TRUE);
                   dip++)
                ;
              if (!dip->open)
                dip = 0;
            }
            if (!dip) {
              for (dip = knowndrivers, edip = dip + NELEM(knowndrivers);
                   dip != edip &&
                   !EqualString(dip->name, MR(pbp->ioParam.ioNamePtr), FALSE, TRUE);
                   dip++)
                ;
              if (dip == edip)
                dip = 0;
            }
            if (dip) {
              devicen = -dip->refnum -1;
              if (devicen < 0 || devicen >= NDEVICES)
                err = badUnitErr;
              else if (MR(UTableBase)[devicen].p)
                err = noErr;	/* note:  if we choose to support desk */
              /*	  accessories, we will have to */
              /*	  check to see if this is one and */
              /*	  call the open routine if it is */
              else {
                if (!(h = MR(MR(UTableBase)[devicen].p =
                             RM((DCtlHandle) NewHandle(sizeof(DCtlEntry))))))
                  err = MemError();
                else {
                  memset((char *) STARH(h), 0, sizeof(DCtlEntry));
                  up = (umacdriverptr) NewPtr(sizeof(umacdriver));
                  if (!(HxX(h, dCtlDriver) = RM(up)))
                    err = MemError();
                  else {
                    up->udrvrOpen   = (ProcPtr) RM(dip->open);
                    up->udrvrPrime  = (ProcPtr) RM(dip->prime);
                    up->udrvrCtl    = (ProcPtr) RM(dip->ctl);
                    up->udrvrStatus = (ProcPtr) RM(dip->status);
                    up->udrvrClose  = (ProcPtr) RM(dip->close);
                    str255assign(up->udrvrName, dip->name);
                    err = noErr;
                  }
                }
              }
              if (err == noErr) {
                pbp->cntrlParam.ioCRefNum = BigEndianValue(dip->refnum);
                err = ROMlib_dispatch(pbp, a, Open, 0);
              }
            } else
              err = dInstErr;
          }
   });
  fs_err_hook (err);
  return err;
}

A2(PUBLIC, OSErr, OpenDriver, StringPtr, name, INTEGER *, rnp)	/* IMII-178 */
{
    ParamBlockRec pb;
    OSErr retval;

    pb.ioParam.ioRefNum = CWC (0); /* so we can't get garbage for ioRefNum. */
    pb.ioParam.ioPermssn = fsCurPerm;
    pb.ioParam.ioNamePtr = RM(name);
    retval = ROMlib_driveropen(&pb, FALSE);
    *rnp = pb.ioParam.ioRefNum;
    fs_err_hook (retval);
    return retval;
}

A1(PUBLIC, OSErr, CloseDriver, INTEGER, rn)   /* IMII-178 */
{
    ParamBlockRec pb;
    OSErr err;

    pb.cntrlParam.ioCRefNum = BigEndianValue(rn);
    err = PBClose(&pb, FALSE);
    fs_err_hook (err);
    return err;
}
