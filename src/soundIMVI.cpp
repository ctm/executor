/* Copyright 1990, 1992, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_soundIMVI[] =
	    "$Id: soundIMVI.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in SoundMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "MemoryMgr.h"
#include "ResourceMgr.h"
#include "SoundDvr.h"
#include "SoundMgr.h"
#include "TimeMgr.h"
#include "FileMgr.h"
#include "rsys/sounddriver.h"
#include "rsys/blockinterrupts.h"
#include "rsys/prefs.h"
#include "rsys/soundopts.h"

using namespace Executor;

P1(PUBLIC, pascal trap void, SndGetSysBeepState, GUEST<INTEGER> *, statep)
{
  /* #warning SndGetSysBeepState not implemented */
  warning_sound_log (NULL_STRING);
}

P1(PUBLIC, pascal trap OSErr, SndSetSysBeepState, INTEGER, state)
{
  /* #warning SndSetSysBeepState not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}


P2(PUBLIC, pascal trap OSErr, SndManagerStatus, INTEGER, length,
							  SMStatusPtr, statusp)
{
/* #warning SndManagerStatus not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}


P0(PUBLIC, pascal trap NumVersion, SndSoundManagerVersion)
{
  NumVersion ret;

  warning_sound_log (NULL_STRING);
  switch (ROMlib_PretendSound)
    {
    case soundoff:
      ret = 0;
      break;
    case soundpretend:
    case soundon:
      ret = 0x03030303;  /* FIXME; need to get this right */
      break;
    default:
      gui_abort ();
      ret = 0;
    }

  return ret;
}

P0(PUBLIC, pascal trap NumVersion, MACEVersion)
{
  /* #warning MACEVersion not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? 0x2000000 : 0;
}

P0(PUBLIC, pascal trap NumVersion, SPBVersion)
{
  /* #warning SPBVersion not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? 0x1000000 : 0;
}

P8(PUBLIC, pascal trap OSErr, SndStartFilePlay, SndChannelPtr, chanp,
	    INTEGER, refnum, INTEGER, resnum, LONGINT, buffersize, Ptr, bufferp,
	    AudioSelectionPtr, theselectionp, ProcPtr, completionp,
							        BOOLEAN, async)
{
  /* #warning SndStartFilePlay not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P1(PUBLIC, pascal trap OSErr, SndPauseFilePlay, SndChannelPtr, chanp)
{
  /* #warning SndPauseFilePlay not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P2(PUBLIC, pascal trap OSErr, SndStopFilePlay, SndChannelPtr, chanp,
								BOOLEAN, async)
{
  /* #warning SndStopFilePlay not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

PRIVATE struct
{
  SndDoubleBufferHeaderPtr headp;
  SndChannelPtr chanp;
  int current_buffer;
  TMTask task;
  BOOLEAN busy;
} call_back_info;


/*
 * clear_pending_sounds may be called before we ever initialize sound, so
 * we have to check to make sure the sound_clear_pending function has
 * been initialized.
 */

void
Executor::clear_pending_sounds (void)
{
  call_back_info.headp = NULL;
  call_back_info.busy = FALSE;
  if (sound_driver->HasSoundClearPending())
    SOUND_CLEAR_PENDING ();
  allchans = nullptr;
}


/*
 * NOTE: we're not really playing anything here, although we could.
 */

PRIVATE OSErr
start_playing (SndChannelPtr chanp, SndDoubleBufferHeaderPtr paramp,
	       int which_buf)
{
  ProcPtr pp;
  static bool task_inserted = FALSE;

  pp = MR (paramp->dbhDoubleBack);
  if (pp)
    {
      SndDoubleBufferPtr dbp;
	  
      dbp = MR (paramp->dbhBufferPtr[which_buf]);
      if (!(dbp->dbFlags & CLC (dbLastBuffer)))
	{
	  LONGINT duration_in_mills;

	  if (call_back_info.busy)
	    warning_unexpected ("busy");
	  call_back_info.headp = paramp;
	  call_back_info.chanp = chanp;
	  call_back_info.current_buffer = which_buf;
	  call_back_info.busy = TRUE;
	  if (!task_inserted)
	    {
	      call_back_info.task.tmAddr
		= RM ((ProcPtr) P_sound_timer_handler);
	      InsTime ((QElemPtr) &call_back_info.task);
	      task_inserted = TRUE;
	    }
	  duration_in_mills = (((long long) 1000 * (1 << 16)
				* CL (dbp->dbNumFrames))
			       / CL (paramp->dbhSampleRate));
	  PrimeTime ((QElemPtr) &call_back_info.task, duration_in_mills);
	}
      else
	{
	  RmvTime ((QElemPtr) &call_back_info.task);
	  task_inserted = FALSE;
	}
    }
  return noErr;
}

A0 (PUBLIC, void, C_sound_timer_handler)
{
  SndDoubleBufferPtr dbp;
  ProcPtr pp;
  int current_buffer;

  if (call_back_info.headp)
    {
      current_buffer = call_back_info.current_buffer;
      pp = MR (call_back_info.headp->dbhDoubleBack);
      dbp = MR (call_back_info.headp->dbhBufferPtr[current_buffer]);
      call_back_info.busy = FALSE;
      start_playing (call_back_info.chanp, call_back_info.headp,
		     current_buffer ^ 1);
      dbp->dbFlags.raw_and( CLC (~dbBufferReady) );
      CToPascalCall((void*)pp, CTOP_SetCTitle, call_back_info.chanp, dbp);
    }
}

P2(PUBLIC, pascal trap OSErr, SndPlayDoubleBuffer, SndChannelPtr, chanp,
   SndDoubleBufferHeaderPtr, paramp)
{
  OSErr retval;
  /* #warning SndPlayDoubleBuffer not implemented */
  switch (ROMlib_PretendSound)
    {
    case soundoff:
      retval = notEnoughHardware;
      break;

    case soundpretend:
      retval = start_playing (chanp, paramp, 0); /* always start with buffer 0 */
      break;

    case soundon:
      if (!paramp)
	warning_sound_log ("paramp = NULL");
      else
	warning_sound_log ("nc %d sz %d c %d p %d",
			   CW (paramp->dbhNumChannels),
			   CW (paramp->dbhSampleSize),
			   CW (paramp->dbhCompressionID),
			   CW (paramp->dbhPacketSize));
      SND_CHAN_DBHP (chanp) = paramp;
      SND_CHAN_CURRENT_DB (chanp) = 0;
      /*
      SND_CHAN_CURRENT_START (chanp) = SND_PROMOTE (SND_CHAN_TIME (chanp));
      */
      SND_CHAN_TIME (chanp) = 0;
      SND_CHAN_CURRENT_START (chanp) = 0;
      SND_CHAN_FLAGS_X (chanp).raw_or( CWC (CHAN_DBINPROG_FLAG) );
      SOUND_GO ();
      retval = noErr;
      break;

    default:
      gui_abort ();
      retval = noErr;  /* quiet gcc if necessary */
      break;
    }
  return retval;
}

P7(PUBLIC, pascal trap void, Comp3to1, Ptr, inp, Ptr, outp, LONGINT, cnt,
		Ptr, instatep, Ptr, outstatep, LONGINT, numchannels,
							 LONGINT, whichchannel)
{
/* #warning Comp3to1 not implemented */
  warning_sound_log (NULL_STRING);
}

P7(PUBLIC, pascal trap void, Comp6to1, Ptr, inp, Ptr, outp, LONGINT, cnt,
		Ptr, instatep, Ptr, outstatep, LONGINT, numchannels,
							 LONGINT, whichchannel)
{
/* #warning Comp6to1 not implemented */
  warning_sound_log (NULL_STRING);
}

P7(PUBLIC, pascal trap void, Exp1to3, Ptr, inp, Ptr, outp, LONGINT, cnt,
		Ptr, instatep, Ptr, outstatep, LONGINT, numchannels,
							 LONGINT, whichchannel)
{
/* #warning Exp1to3 not implemented */
  warning_sound_log (NULL_STRING);
}

P7(PUBLIC, pascal trap void, Exp1to6, Ptr, inp, Ptr, outp, LONGINT, cnt,
		Ptr, instatep, Ptr, outstatep, LONGINT, numchannels,
							 LONGINT, whichchannel)
{
/* #warning Exp1to6 not implemented */
  warning_sound_log (NULL_STRING);
}


P4(PUBLIC, pascal trap OSErr, SndRecord, ProcPtr, filterp, Point, corner,
					 OSType, quality, GUEST<Handle> *, sndhandlep)
{
/* #warning SPBRecord not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P4(PUBLIC, pascal trap OSErr, SndRecordToFile, ProcPtr, filterp,
			       Point, corner, OSType, quality, INTEGER, refnum)
{
/* #warning SPBRecordToFile not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}


P3(PUBLIC, pascal trap OSErr, SPBOpenDevice, Str255, name, INTEGER, permission,
							  GUEST<LONGINT> *, inrefnump)
{
/* #warning SPBOpenDevice not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P1(PUBLIC, pascal trap OSErr, SPBCloseDevice, LONGINT, inrefnum)
{
/* #warning SPBCloseDevice not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}


P2(PUBLIC, pascal trap OSErr, SPBRecord, SPBPtr, inparamp, BOOLEAN, async)
{
/* #warning SPBRecord not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P3(PUBLIC, pascal trap OSErr, SPBRecordToFile, INTEGER, refnum,
					      SPBPtr, inparamp, BOOLEAN, async)
{
/* #warning SPBRecordToFile not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P1(PUBLIC, pascal trap OSErr, SPBPauseRecording, LONGINT, refnum)
{
/* #warning SPBPauseRecording not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P1(PUBLIC, pascal trap OSErr, SPBResumeRecording, LONGINT, refnum)
{
/* #warning SPBResumeRecording not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P1(PUBLIC, pascal trap OSErr, SPBStopRecording, LONGINT, refnum)
{
/* #warning PPBStopRecording not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P7(PUBLIC, pascal trap OSErr, SPBGetRecordingStatus, LONGINT, refnum,
        GUEST<INTEGER> *, recordingstatus, GUEST<INTEGER> *, meterlevel, GUEST<LONGINT> *,
	    totalsampstorecord, GUEST<LONGINT> *, numsampsrecorded, GUEST<LONGINT> *,
			    totalmsecstorecord, GUEST<LONGINT> *, numbermsecsrecorded)
{
/* #warning SPBGetRecordingStatus not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}


P3(PUBLIC, pascal trap OSErr, SPBGetDeviceInfo, LONGINT, refnum, OSType, info,
								    Ptr, infop)
{
/* #warning SPBGetDeviceInfo not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P3(PUBLIC, pascal trap OSErr, SPBSetDeviceInfo, LONGINT, refnum, OSType,
							      info, Ptr, infop)
{
/* #warning SPBSetDeviceInfo not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}


P8(PUBLIC, pascal trap OSErr, SetupSndHeader, Handle, sndhandle, INTEGER,
    numchannels, Fixed, rate, INTEGER, size, OSType, compresion, INTEGER,
			    basefreq, LONGINT, numbytes, GUEST<INTEGER> *, headerlenp)
{
/* #warning SetupSndHeader not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P7(PUBLIC, pascal trap OSErr, SetupAIFFHeader, INTEGER, refnum, INTEGER,
	numchannels, Fixed, samplerate, INTEGER, samplesize,
		    OSType, compression, LONGINT, numbytes, LONGINT, numframes)
{
/* #warning SetupAIFFHeader not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}


P2(PUBLIC, pascal trap OSErr, SPBSignInDevice, INTEGER, refnum, Str255, name)
{
/* #warning SPBSignInDevice not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P1(PUBLIC, pascal trap OSErr, SPBSignOutDevice, INTEGER, refnum)
{
/* #warning SPBSignOutDevice not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P3(PUBLIC, pascal trap OSErr, SPBGetIndexedDevice, INTEGER, count,
				     Str255, name, GUEST<Handle> *, deviceiconhandlep)
{
/* #warning SPBGetIndexedDevice not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}


P2(PUBLIC, pascal trap OSErr, SPBMillisecondsToBytes, LONGINT, refnum,
        GUEST<LONGINT> *, millip)
{
/* #warning SPBMillisecondsToBytes not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P2(PUBLIC, pascal trap OSErr, SPBBytesToMilliseconds, LONGINT, refnum,
        GUEST<LONGINT> *, bytecountp)
{
/* #warning SPBBytesToMilliseconds not implemented */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

P0(PUBLIC, pascal trap void, FinaleUnknown1)
{
  /* Finale calls this */
  warning_sound_log (NULL_STRING);
}


P4(PUBLIC, pascal trap OSErr, FinaleUnknown2, ResType, arg1, LONGINT, arg2,
   Ptr, arg3, Ptr, arg4)
{
  /* Finale calls this */
  warning_sound_log (NULL_STRING);
  return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

/* various self-running demos (made by Macromedia Director, I think)
   seem to call this */

P0(PUBLIC, pascal trap long, DirectorUnknown3)
{
  warning_sound_log (NULL_STRING);
  return 0;
}


P4(PUBLIC, pascal trap INTEGER, DirectorUnknown4, ResType, arg1, INTEGER, arg2,
   Ptr, arg3, Ptr, arg4)
{
  warning_sound_log (NULL_STRING);
  return paramErr;
}


/* Sound Manager 3.0 */

enum { half_volume = 0x50 };

P1(PUBLIC, pascal trap OSErr, GetSysBeepVolume, GUEST<LONGINT> *, levelp)
{
  OSErr retval;

  warning_sound_log (NULL_STRING);
  *levelp = CLC(half_volume);
  retval = noErr;
  return retval;
}

P1(PUBLIC, pascal trap OSErr, SetSysBeepVolume, LONGINT, level)
{
  OSErr retval;

  warning_sound_log (NULL_STRING);
  retval = noErr;
  return retval;
}

P1(PUBLIC, pascal trap OSErr, GetDefaultOutputVolume, GUEST<LONGINT> *,levelp)
{
  OSErr retval;

  warning_sound_log (NULL_STRING);
  *levelp = CLC(half_volume);
  retval = noErr;
  return retval;
}

P1(PUBLIC, pascal trap OSErr, SetDefaultOutputVolume, LONGINT, level)
{
  OSErr retval;

  warning_sound_log (NULL_STRING);
  retval = noErr;
  return retval;
}

P2(PUBLIC, pascal trap OSErr, GetSoundHeaderOffset, Handle, sndHandle,
   GUEST<LONGINT> *,offset)
{
  OSErr retval;
  int num_commands;
  SndCommand *cmds;
  int i;

  warning_sound_log (NULL_STRING);

  num_commands = ROMlib_get_snd_cmds (sndHandle, &cmds);

  retval = badFormat;
  for (i = 0; i < num_commands; ++i)
    {
      if (cmds[i].cmd == CWC(bufferCmd | 0x8000) ||
	  cmds[i].cmd == CWC(soundCmd | 0x8000))
	{
	  *offset = cmds[i].param2;
	  retval = noErr;
	  break;
	}
    }
  
  return retval;
}

P3(PUBLIC, pascal trap UnsignedFixed, UnsignedFixedMulDiv, UnsignedFixed, value,
   UnsignedFixed, multiplier, UnsignedFixed, divisor)
{
  UnsignedFixed retval;

  warning_sound_log (NULL_STRING);
  warning_unimplemented (NULL_STRING);
  retval = 0;
  return retval;
}

P5(PUBLIC, pascal trap OSErr, GetCompressionInfo, INTEGER, compressionID,
   OSType, format, INTEGER, numChannels, INTEGER, sampleSize,
   CompressionInfoPtr, cp)
{
  OSErr retval;

  warning_sound_log (NULL_STRING);
  warning_unimplemented (NULL_STRING);
  retval = paramErr;
  return retval;
}

P3(PUBLIC, pascal trap OSErr, SetSoundPreference, OSType, theType, Str255, name,
   Handle, settings)
{
  OSErr retval;

  warning_sound_log (NULL_STRING);
  warning_unimplemented (NULL_STRING);
  retval = paramErr;
  return retval;
}

P3(PUBLIC, pascal trap OSErr, GetSoundPreference, OSType, theType, Str255, name,
   Handle, settings)
{
  OSErr retval;

  warning_sound_log (NULL_STRING);
  warning_unimplemented (NULL_STRING);
  retval = paramErr;
  return retval;
}

/* Sound Manager 3.1 */

P3(PUBLIC, pascal trap OSErr, SndGetInfo, SndChannelPtr, chan, OSType, selector,
   void *, infoPtr)
{
  OSErr retval;

  warning_sound_log (NULL_STRING);
  warning_unimplemented (NULL_STRING);
  retval = paramErr;
  return retval;
}

P3(PUBLIC, pascal trap OSErr, SndSetInfo, SndChannelPtr, chan, OSType, selector,
   void *, infoPtr)
{
  OSErr retval;

  warning_sound_log (NULL_STRING);
  warning_unimplemented (NULL_STRING);
  retval = paramErr;
  return retval;
}
