/* Copyright 1990, 1992, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

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

void Executor::C_SndGetSysBeepState(GUEST<INTEGER> *statep)
{
    /* #warning SndGetSysBeepState not implemented */
    warning_sound_log(NULL_STRING);
}

OSErr Executor::C_SndSetSysBeepState(INTEGER state)
{
    /* #warning SndSetSysBeepState not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SndManagerStatus(INTEGER length, SMStatusPtr statusp)
{
    /* #warning SndManagerStatus not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

NumVersion Executor::C_SndSoundManagerVersion()
{
    NumVersion ret;

    warning_sound_log(NULL_STRING);
    switch(ROMlib_PretendSound)
    {
        case soundoff:
            ret = 0;
            break;
        case soundpretend:
        case soundon:
            ret = 0x03030303; /* FIXME; need to get this right */
            break;
        default:
            gui_abort();
            ret = 0;
    }

    return ret;
}

NumVersion Executor::C_MACEVersion()
{
    /* #warning MACEVersion not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? 0x2000000 : 0;
}

NumVersion Executor::C_SPBVersion()
{
    /* #warning SPBVersion not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? 0x1000000 : 0;
}

OSErr Executor::C_SndStartFilePlay(SndChannelPtr chanp, INTEGER refnum,
                                   INTEGER resnum, LONGINT buffersize,
                                   Ptr bufferp,
                                   AudioSelectionPtr theselectionp,
                                   ProcPtr completionp, BOOLEAN async)
{
    /* #warning SndStartFilePlay not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SndPauseFilePlay(SndChannelPtr chanp)
{
    /* #warning SndPauseFilePlay not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SndStopFilePlay(SndChannelPtr chanp, BOOLEAN async)
{
    /* #warning SndStopFilePlay not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

static struct
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

void Executor::clear_pending_sounds(void)
{
    call_back_info.headp = NULL;
    call_back_info.busy = false;
    if(sound_driver->HasSoundClearPending())
        SOUND_CLEAR_PENDING();
    allchans = nullptr;
}

/*
 * NOTE: we're not really playing anything here, although we could.
 */

static OSErr
start_playing(SndChannelPtr chanp, SndDoubleBufferHeaderPtr paramp,
              int which_buf)
{
    SndDoubleBackProcPtr pp;
    static bool task_inserted = false;

    pp = MR(paramp->dbhDoubleBack);
    if(pp)
    {
        SndDoubleBufferPtr dbp;

        dbp = MR(paramp->dbhBufferPtr[which_buf]);
        if(!(dbp->dbFlags & CLC(dbLastBuffer)))
        {
            LONGINT duration_in_mills;

            if(call_back_info.busy)
                warning_unexpected("busy");
            call_back_info.headp = paramp;
            call_back_info.chanp = chanp;
            call_back_info.current_buffer = which_buf;
            call_back_info.busy = true;
            if(!task_inserted)
            {
                call_back_info.task.tmAddr
                    = RM((ProcPtr)P_sound_timer_handler);
                InsTime((QElemPtr)&call_back_info.task);
                task_inserted = true;
            }
            duration_in_mills = (((long long)1000 * (1 << 16)
                                  * CL(dbp->dbNumFrames))
                                 / CL(paramp->dbhSampleRate));
            PrimeTime((QElemPtr)&call_back_info.task, duration_in_mills);
        }
        else
        {
            RmvTime((QElemPtr)&call_back_info.task);
            task_inserted = false;
        }
    }
    return noErr;
}

void Executor::C_sound_timer_handler()
{
    SndDoubleBufferPtr dbp;
    SndDoubleBackProcPtr pp;
    int current_buffer;

    if(call_back_info.headp)
    {
        current_buffer = call_back_info.current_buffer;
        pp = MR(call_back_info.headp->dbhDoubleBack);
        dbp = MR(call_back_info.headp->dbhBufferPtr[current_buffer]);
        call_back_info.busy = false;
        start_playing(call_back_info.chanp, call_back_info.headp,
                      current_buffer ^ 1);
        dbp->dbFlags.raw_and(CLC(~dbBufferReady));
        pp(call_back_info.chanp, dbp);
    }
}

OSErr Executor::C_SndPlayDoubleBuffer(SndChannelPtr chanp,
                                      SndDoubleBufferHeaderPtr paramp)
{
    OSErr retval;
    /* #warning SndPlayDoubleBuffer not implemented */
    switch(ROMlib_PretendSound)
    {
        case soundoff:
            retval = notEnoughHardware;
            break;

        case soundpretend:
            retval = start_playing(chanp, paramp, 0); /* always start with buffer 0 */
            break;

        case soundon:
            if(!paramp)
                warning_sound_log("paramp = NULL");
            else
                warning_sound_log("nc %d sz %d c %d p %d",
                                  CW(paramp->dbhNumChannels),
                                  CW(paramp->dbhSampleSize),
                                  CW(paramp->dbhCompressionID),
                                  CW(paramp->dbhPacketSize));
            SND_CHAN_DBHP(chanp) = paramp;
            SND_CHAN_CURRENT_DB(chanp) = 0;
            /*
      SND_CHAN_CURRENT_START (chanp) = SND_PROMOTE (SND_CHAN_TIME (chanp));
      */
            SND_CHAN_TIME(chanp) = 0;
            SND_CHAN_CURRENT_START(chanp) = 0;
            SND_CHAN_FLAGS_X(chanp).raw_or(CWC(CHAN_DBINPROG_FLAG));
            SOUND_GO();
            retval = noErr;
            break;

        default:
            gui_abort();
            retval = noErr; /* quiet gcc if necessary */
            break;
    }
    return retval;
}

void Executor::C_Comp3to1(Ptr inp, Ptr outp, LONGINT cnt, Ptr instatep,
                          Ptr outstatep, LONGINT numchannels,
                          LONGINT whichchannel)
{
    /* #warning Comp3to1 not implemented */
    warning_sound_log(NULL_STRING);
}

void Executor::C_Comp6to1(Ptr inp, Ptr outp, LONGINT cnt, Ptr instatep,
                          Ptr outstatep, LONGINT numchannels,
                          LONGINT whichchannel)
{
    /* #warning Comp6to1 not implemented */
    warning_sound_log(NULL_STRING);
}

void Executor::C_Exp1to3(Ptr inp, Ptr outp, LONGINT cnt, Ptr instatep,
                         Ptr outstatep, LONGINT numchannels,
                         LONGINT whichchannel)
{
    /* #warning Exp1to3 not implemented */
    warning_sound_log(NULL_STRING);
}

void Executor::C_Exp1to6(Ptr inp, Ptr outp, LONGINT cnt, Ptr instatep,
                         Ptr outstatep, LONGINT numchannels,
                         LONGINT whichchannel)
{
    /* #warning Exp1to6 not implemented */
    warning_sound_log(NULL_STRING);
}

OSErr Executor::C_SndRecord(ProcPtr filterp, Point corner, OSType quality,
                            GUEST<Handle> *sndhandlep)
{
    /* #warning SPBRecord not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SndRecordToFile(ProcPtr filterp, Point corner,
                                  OSType quality, INTEGER refnum)
{
    /* #warning SPBRecordToFile not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBOpenDevice(Str255 name, INTEGER permission,
                                GUEST<LONGINT> *inrefnump)
{
    /* #warning SPBOpenDevice not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBCloseDevice(LONGINT inrefnum)
{
    /* #warning SPBCloseDevice not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBRecord(SPBPtr inparamp, BOOLEAN async)
{
    /* #warning SPBRecord not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBRecordToFile(INTEGER refnum, SPBPtr inparamp,
                                  BOOLEAN async)
{
    /* #warning SPBRecordToFile not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBPauseRecording(LONGINT refnum)
{
    /* #warning SPBPauseRecording not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBResumeRecording(LONGINT refnum)
{
    /* #warning SPBResumeRecording not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBStopRecording(LONGINT refnum)
{
    /* #warning PPBStopRecording not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBGetRecordingStatus(LONGINT refnum,
                                        GUEST<INTEGER> *recordingstatus,
                                        GUEST<INTEGER> *meterlevel,
                                        GUEST<LONGINT> *totalsampstorecord,
                                        GUEST<LONGINT> *numsampsrecorded,
                                        GUEST<LONGINT> *totalmsecstorecord,
                                        GUEST<LONGINT> *numbermsecsrecorded)
{
    /* #warning SPBGetRecordingStatus not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBGetDeviceInfo(LONGINT refnum, OSType info, Ptr infop)
{
    /* #warning SPBGetDeviceInfo not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBSetDeviceInfo(LONGINT refnum, OSType info, Ptr infop)
{
    /* #warning SPBSetDeviceInfo not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SetupSndHeader(Handle sndhandle, INTEGER numchannels,
                                 Fixed rate, INTEGER size, OSType compresion,
                                 INTEGER basefreq, LONGINT numbytes,
                                 GUEST<INTEGER> *headerlenp)
{
    /* #warning SetupSndHeader not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SetupAIFFHeader(INTEGER refnum, INTEGER numchannels,
                                  Fixed samplerate, INTEGER samplesize,
                                  OSType compression, LONGINT numbytes,
                                  LONGINT numframes)
{
    /* #warning SetupAIFFHeader not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBSignInDevice(INTEGER refnum, Str255 name)
{
    /* #warning SPBSignInDevice not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBSignOutDevice(INTEGER refnum)
{
    /* #warning SPBSignOutDevice not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBGetIndexedDevice(INTEGER count, Str255 name,
                                      GUEST<Handle> *deviceiconhandlep)
{
    /* #warning SPBGetIndexedDevice not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBMillisecondsToBytes(LONGINT refnum, GUEST<LONGINT> *millip)
{
    /* #warning SPBMillisecondsToBytes not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

OSErr Executor::C_SPBBytesToMilliseconds(
    LONGINT refnum, GUEST<LONGINT> *bytecountp)
{
    /* #warning SPBBytesToMilliseconds not implemented */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

void Executor::C_FinaleUnknown1()
{
    /* Finale calls this */
    warning_sound_log(NULL_STRING);
}

OSErr Executor::C_FinaleUnknown2(ResType arg1, LONGINT arg2, Ptr arg3, Ptr arg4)
{
    /* Finale calls this */
    warning_sound_log(NULL_STRING);
    return ROMlib_PretendSound == soundpretend ? noErr : notEnoughHardware;
}

/* various self-running demos (made by Macromedia Director, I think)
   seem to call this */

LONGINT Executor::C_DirectorUnknown3()
{
    warning_sound_log(NULL_STRING);
    return 0;
}

INTEGER Executor::C_DirectorUnknown4(ResType arg1, INTEGER arg2, Ptr arg3,
                                     Ptr arg4)
{
    warning_sound_log(NULL_STRING);
    return paramErr;
}

/* Sound Manager 3.0 */

enum
{
    half_volume = 0x50
};

OSErr Executor::C_GetSysBeepVolume(GUEST<LONGINT> *levelp)
{
    OSErr retval;

    warning_sound_log(NULL_STRING);
    *levelp = CLC(half_volume);
    retval = noErr;
    return retval;
}

OSErr Executor::C_SetSysBeepVolume(LONGINT level)
{
    OSErr retval;

    warning_sound_log(NULL_STRING);
    retval = noErr;
    return retval;
}

OSErr Executor::C_GetDefaultOutputVolume(GUEST<LONGINT> *levelp)
{
    OSErr retval;

    warning_sound_log(NULL_STRING);
    *levelp = CLC(half_volume);
    retval = noErr;
    return retval;
}

OSErr Executor::C_SetDefaultOutputVolume(LONGINT level)
{
    OSErr retval;

    warning_sound_log(NULL_STRING);
    retval = noErr;
    return retval;
}

OSErr Executor::C_GetSoundHeaderOffset(Handle sndHandle, GUEST<LONGINT> *offset)
{
    OSErr retval;
    int num_commands;
    SndCommand *cmds;
    int i;

    warning_sound_log(NULL_STRING);

    num_commands = ROMlib_get_snd_cmds(sndHandle, &cmds);

    retval = badFormat;
    for(i = 0; i < num_commands; ++i)
    {
        if(cmds[i].cmd == CWC(bufferCmd | 0x8000) || cmds[i].cmd == CWC(soundCmd | 0x8000))
        {
            *offset = cmds[i].param2;
            retval = noErr;
            break;
        }
    }

    return retval;
}

UnsignedFixed Executor::C_UnsignedFixedMulDiv(
    UnsignedFixed value, UnsignedFixed multiplier, UnsignedFixed divisor)
{
    UnsignedFixed retval;

    warning_sound_log(NULL_STRING);
    warning_unimplemented(NULL_STRING);
    retval = 0;
    return retval;
}

OSErr Executor::C_GetCompressionInfo(INTEGER compressionID, OSType format,
                                     INTEGER numChannels, INTEGER sampleSize,
                                     CompressionInfoPtr cp)
{
    OSErr retval;

    warning_sound_log(NULL_STRING);
    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}

OSErr Executor::C_SetSoundPreference(OSType theType, Str255 name,
                                     Handle settings)
{
    OSErr retval;

    warning_sound_log(NULL_STRING);
    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}

OSErr Executor::C_GetSoundPreference(OSType theType, Str255 name,
                                     Handle settings)
{
    OSErr retval;

    warning_sound_log(NULL_STRING);
    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}

/* Sound Manager 3.1 */

OSErr Executor::C_SndGetInfo(SndChannelPtr chan, OSType selector, void *infoPtr)
{
    OSErr retval;

    warning_sound_log(NULL_STRING);
    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}

OSErr Executor::C_SndSetInfo(SndChannelPtr chan, OSType selector, void *infoPtr)
{
    OSErr retval;

    warning_sound_log(NULL_STRING);
    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}
