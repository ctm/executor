#if !defined(__SOUNDMGR__)
#define __SOUNDMGR__

/*
 * Copyright 1992 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "QuickDraw.h"

namespace Executor
{
struct SndCommand
{
    GUEST_STRUCT;
    GUEST<INTEGER> cmd;
    GUEST<INTEGER> param1;
    GUEST<LONGINT> param2;
};

enum
{
    stdQLength = 128,
};

enum
{
    stdSH = 0, /* standard sound header */
    cmpSH = 0xFE, /* compressed sound header */
    extSH = 0xFF, /* extended sound header */
};

typedef struct SndChannel
{
    GUEST_STRUCT;
    GUEST<SndChannel *> nextChan;
    GUEST<Ptr> firstMod;
    GUEST<ProcPtr> callBack;
    GUEST<LONGINT> userInfo;
    GUEST<LONGINT> wait;
    GUEST<SndCommand> cmdInProg;
    GUEST<INTEGER> flags;
    GUEST<INTEGER> qLength;
    GUEST<INTEGER> qHead;
    GUEST<INTEGER> qTail;
    GUEST<SndCommand[stdQLength]> queue;
} * SndChannelPtr;

#define SND_CHAN_FLAGS_X(c) (c->flags)
#define SND_CHAN_FLAGS(c) (CW(SND_CHAN_FLAGS_X(c)))

enum
{
    nullCmd,
    initCmd,
    freeCmd,
    quietCmd,
    flushCmd,
    waitCmd = 10,
    pauseCmd,
    resumeCmd,
    callBackCmd,
    syncCmd,
    emptyCmd,
    tickleCmd = 20,
    requestNextCmd,
    howOftenCmd,
    wakeUpCmd,
    availableCmd,
    noteCmd = 40,
    restCmd,
    freqCmd,
    ampCmd,
    timbreCmd,
    waveTableCmd = 60,
    phaseCmd,
    soundCmd = 80,
    bufferCmd,
    rateCmd,
    midiDataCmd = 100,
};

struct soundbuffer_t
{
    GUEST_STRUCT;
    GUEST<LONGINT> offset;
    GUEST<LONGINT> nsamples;
    GUEST<LONGINT> rate;
    GUEST<LONGINT> altbegin;
    GUEST<LONGINT> altend;
    GUEST<INTEGER> basenote;
    GUEST<unsigned char[1]> buf;
};

typedef struct _SoundHeader
{
    GUEST_STRUCT;
    GUEST<Ptr> samplePtr;
    GUEST<LONGINT> length;
    GUEST<Fixed> sampleRate;
    GUEST<LONGINT> loopStart;
    GUEST<LONGINT> loopEnd;
    GUEST<Byte> encode;
    GUEST<Byte> baseFrequency;
    GUEST<Byte> sampleArea[0];
} SoundHeader, *SoundHeaderPtr;

typedef struct _ExtSoundHeader
{
    GUEST_STRUCT;
    GUEST<Ptr> samplePtr;
    GUEST<LONGINT> numChannels;
    GUEST<Fixed> sampleRate;
    GUEST<LONGINT> loopStart;
    GUEST<LONGINT> loopEnd;
    GUEST<Byte> encode;
    GUEST<Byte> baseFrequency;
    GUEST<LONGINT> numFrames;
    GUEST<Extended> AIFFSampleRate; /* ???  should be Extended80 */
    GUEST<Ptr> MarkerChunk;
    GUEST<Ptr> instrumentChunks;
    GUEST<Ptr> AESRecording;
    GUEST<INTEGER> sampleSize;
    GUEST<INTEGER> futureUse1;
    GUEST<LONGINT> futureUse2;
    GUEST<LONGINT> futureUse3;
    GUEST<LONGINT> futureUse4;
    GUEST<Byte> sampleArea[0];
} ExtSoundHeader, *ExtSoundHeaderPtr;

enum
{
    noteSynth = 1,
    waveTableSynth = 3,
    sampledSynth = 5,
    MIDISynthIn = 7,
    MIDISynthOut = 9,
};

enum
{
    badChannel = (-205),
    badFormat = (-206),
    noHardware = (-200),
    notEnoughHardware = (-201),
    queueFull = (-203),
    resProblem = (-204),
};

#if !defined(SoundActive)
extern Byte SoundActive;
#endif

enum
{
    soundactiveoff = 0,
    soundactive5 = 5,
    soundactiveinplay = 0x81,
    soundactivenone = 0xFF,
};

typedef struct SndDoubleBuffer
{
    GUEST_STRUCT;
    GUEST<LONGINT> dbNumFrames;
    GUEST<LONGINT> dbFlags;
    GUEST<LONGINT[2]> dbUserInfo;
    GUEST<Byte> dbSoundData[0];
} * SndDoubleBufferPtr;

enum
{
    dbBufferReady = 1,
    dbLastBuffer = 4
};

typedef struct SndDoubleBufferHeader
{
    GUEST_STRUCT;
    GUEST<INTEGER> dbhNumChannels;
    GUEST<INTEGER> dbhSampleSize;
    GUEST<INTEGER> dbhCompressionID;
    GUEST<INTEGER> dbhPacketSize;
    GUEST<Fixed> dbhSampleRate;
    GUEST<SndDoubleBufferPtr[2]> dbhBufferPtr;
    GUEST<ProcPtr> dbhDoubleBack;
} * SndDoubleBufferHeaderPtr;

typedef struct _SCSTATUS
{
    GUEST_STRUCT;
    GUEST<Fixed> scStartTime;
    GUEST<Fixed> scEndTime;
    GUEST<Fixed> scCurrentTime;
    GUEST<Boolean> scChannelBusy;
    GUEST<Boolean> scChannelDisposed;
    GUEST<Boolean> scChannelPaused;
    GUEST<Boolean> scUnused;
    GUEST<LONGINT> scChannelAttributes;
    GUEST<LONGINT> scCPULoad;
} SCStatus, *SCStatusPtr;

#if 1 /* stub definitions */
typedef void *SMStatusPtr;
typedef LONGINT NumVersion;
typedef void *AudioSelectionPtr;
typedef void *SPBPtr;
#endif

typedef uint32_t UnsignedFixed;
typedef Ptr CompressionInfoPtr;

extern trap void C_SndGetSysBeepState(GUEST<INTEGER> *statep);
PASCAL_FUNCTION(SndGetSysBeepState);

extern trap OSErr C_SndSetSysBeepState(INTEGER state);
PASCAL_FUNCTION(SndSetSysBeepState);

extern trap OSErr C_SndChannelStatus(SndChannelPtr chanp, INTEGER length,
                                     SCStatusPtr statusp);
PASCAL_FUNCTION(SndChannelStatus);

extern trap OSErr C_SndManagerStatus(INTEGER length, SMStatusPtr statusp);
PASCAL_FUNCTION(SndManagerStatus);

extern trap NumVersion C_SndSoundManagerVersion(void);
PASCAL_FUNCTION(SndSoundManagerVersion);

extern trap NumVersion C_MACEVersion(void);
PASCAL_FUNCTION(MACEVersion);

extern trap NumVersion C_SPBVersion(void);
PASCAL_FUNCTION(SPBVersion);

extern trap OSErr C_SndStartFilePlay(SndChannelPtr chanp,
                                     INTEGER refnum, INTEGER resnum, LONGINT buffersize, Ptr bufferp,
                                     AudioSelectionPtr theselectionp, ProcPtr completionp, BOOLEAN async);
PASCAL_FUNCTION(SndStartFilePlay);

extern trap OSErr C_SndPauseFilePlay(SndChannelPtr chanp);
PASCAL_FUNCTION(SndPauseFilePlay);

extern trap OSErr C_SndStopFilePlay(SndChannelPtr chanp, BOOLEAN async);
PASCAL_FUNCTION(SndStopFilePlay);

extern trap OSErr C_SndPlayDoubleBuffer(SndChannelPtr chanp,
                                        SndDoubleBufferHeaderPtr paramp);
PASCAL_FUNCTION(SndPlayDoubleBuffer);

extern trap void C_Comp3to1(Ptr inp, Ptr outp, LONGINT cnt,
                            Ptr instatep, Ptr outstatep, LONGINT numchannels, LONGINT whichchannel);
PASCAL_FUNCTION(Comp3to1);

extern trap void C_Comp6to1(Ptr inp, Ptr outp, LONGINT cnt,
                            Ptr instatep, Ptr outstatep, LONGINT numchannels, LONGINT whichchannel);
PASCAL_FUNCTION(Comp6to1);

extern trap void C_Exp1to3(Ptr inp, Ptr outp, LONGINT cnt, Ptr instatep,
                           Ptr outstatep, LONGINT numchannels, LONGINT whichchannel);
PASCAL_FUNCTION(Exp1to3);

extern trap void C_Exp1to6(Ptr inp, Ptr outp, LONGINT cnt, Ptr instatep,
                           Ptr outstatep, LONGINT numchannels, LONGINT whichchannel);
PASCAL_FUNCTION(Exp1to6);

extern trap OSErr C_SndRecord(ProcPtr filterp, Point corner,
                              OSType quality, GUEST<Handle> *sndhandlep);
PASCAL_FUNCTION(SndRecord);

extern trap OSErr C_SndRecordToFile(ProcPtr filterp, Point corner,
                                    OSType quality, INTEGER refnum);
PASCAL_FUNCTION(SndRecordToFile);

extern trap OSErr C_SPBOpenDevice(Str255 name, INTEGER permission,
                                  GUEST<LONGINT> *inrefnump);
PASCAL_FUNCTION(SPBOpenDevice);

extern trap OSErr C_SPBCloseDevice(LONGINT inrefnum);
PASCAL_FUNCTION(SPBCloseDevice);

extern trap OSErr C_SPBRecord(SPBPtr inparamp, BOOLEAN async);
PASCAL_FUNCTION(SPBRecord);

extern trap OSErr C_SPBRecordToFile(INTEGER refnum, SPBPtr inparamp,
                                    BOOLEAN async);
PASCAL_FUNCTION(SPBRecordToFile);

extern trap OSErr C_SPBPauseRecording(LONGINT refnum);
PASCAL_FUNCTION(SPBPauseRecording);

extern trap OSErr C_SPBResumeRecording(LONGINT refnum);
PASCAL_FUNCTION(SPBResumeRecording);

extern trap OSErr C_SPBStopRecording(LONGINT refnum);
PASCAL_FUNCTION(SPBStopRecording);

extern trap OSErr C_SPBGetRecordingStatus(LONGINT refnum,
                                          GUEST<INTEGER> *recordingstatus, GUEST<INTEGER> *meterlevel,
                                          GUEST<LONGINT> *totalsampstorecord, GUEST<LONGINT> *numsampsrecorded,
                                          GUEST<LONGINT> *totalmsecstorecord, GUEST<LONGINT> *numbermsecsrecorded);
PASCAL_FUNCTION(SPBGetRecordingStatus);

extern trap OSErr C_SPBGetDeviceInfo(LONGINT refnum, OSType info,
                                     Ptr infop);
PASCAL_FUNCTION(SPBGetDeviceInfo);

extern trap OSErr C_SPBSetDeviceInfo(LONGINT refnum, OSType info,
                                     Ptr infop);
PASCAL_FUNCTION(SPBSetDeviceInfo);

extern trap OSErr C_SetupSndHeader(Handle sndhandle, INTEGER numchannels,
                                   Fixed rate, INTEGER size, OSType compresion, INTEGER basefreq,
                                   LONGINT numbytes, GUEST<INTEGER> *headerlenp);
PASCAL_FUNCTION(SetupSndHeader);

extern trap OSErr C_SetupAIFFHeader(INTEGER refnum, INTEGER numchannels,
                                    Fixed samplerate, INTEGER samplesize, OSType compression,
                                    LONGINT numbytes, LONGINT numframes);
PASCAL_FUNCTION(SetupAIFFHeader);

extern trap OSErr C_SPBSignInDevice(INTEGER refnum, Str255 name);
PASCAL_FUNCTION(SPBSignInDevice);

extern trap OSErr C_SPBSignOutDevice(INTEGER refnum);
PASCAL_FUNCTION(SPBSignOutDevice);

extern trap OSErr C_SPBGetIndexedDevice(INTEGER count, Str255 name,
                                        GUEST<Handle> *deviceiconhandlep);
PASCAL_FUNCTION(SPBGetIndexedDevice);

extern trap OSErr C_SPBMillisecondsToBytes(LONGINT refnum,
                                           GUEST<LONGINT> *millip);
PASCAL_FUNCTION(SPBMillisecondsToBytes);

extern trap OSErr C_SPBBytesToMilliseconds(LONGINT refnum,
                                           GUEST<LONGINT> *bytecountp);
PASCAL_FUNCTION(SPBBytesToMilliseconds);

extern trap OSErr C_GetSysBeepVolume(GUEST<LONGINT> *levelp);
PASCAL_FUNCTION(GetSysBeepVolume);

extern trap OSErr C_SetSysBeepVolume(LONGINT level);
PASCAL_FUNCTION(SetSysBeepVolume);

extern trap OSErr C_GetDefaultOutputVolume(GUEST<LONGINT> *levelp);
PASCAL_FUNCTION(GetDefaultOutputVolume);

extern trap OSErr C_SetDefaultOutputVolume(LONGINT level);
PASCAL_FUNCTION(SetDefaultOutputVolume);

extern trap OSErr C_GetSoundHeaderOffset(Handle sndHandle, GUEST<LONGINT> *offset);
PASCAL_FUNCTION(GetSoundHeaderOffset);

extern trap UnsignedFixed C_UnsignedFixedMulDiv(UnsignedFixed value,
                                                UnsignedFixed multiplier,
                                                UnsignedFixed divisor);
PASCAL_FUNCTION(UnsignedFixedMulDiv);

extern trap OSErr C_GetCompressionInfo(INTEGER compressionID, OSType format,
                                       INTEGER numChannels,
                                       INTEGER sampleSize,
                                       CompressionInfoPtr cp);
PASCAL_FUNCTION(GetCompressionInfo);

extern trap OSErr C_SetSoundPreference(OSType theType, Str255 name,
                                       Handle settings);
PASCAL_FUNCTION(SetSoundPreference);

extern trap OSErr C_GetSoundPreference(OSType theType, Str255 name,
                                       Handle settings);
PASCAL_FUNCTION(GetSoundPreference);

extern trap OSErr C_SndGetInfo(SndChannelPtr chan, OSType selector,
                               void *infoPtr);
PASCAL_FUNCTION(SndGetInfo);

extern trap OSErr C_SndSetInfo(SndChannelPtr chan, OSType selector,
                               void *infoPtr);
PASCAL_FUNCTION(SndSetInfo);

extern void StartSound(Ptr srec, LONGINT nb, ProcPtr comp);
extern void StopSound(void);
extern BOOLEAN SoundDone(void);
extern void GetSoundVol(INTEGER *volp);
extern void SetSoundVol(INTEGER vol);
extern trap OSErr C_SndPlay(SndChannelPtr chanp, Handle sndh,
                            BOOLEAN async);
PASCAL_TRAP(SndPlay, 0xA805);
extern trap OSErr C_SndNewChannel(GUEST<SndChannelPtr> *chanpp,
                                  INTEGER synth, LONGINT init, ProcPtr userroutinep);
PASCAL_TRAP(SndNewChannel, 0xA807);
extern trap OSErr C_SndAddModifier(SndChannelPtr chanp,
                                   ProcPtr mod, INTEGER id, LONGINT init);
PASCAL_TRAP(SndAddModifier, 0xA802);
extern trap OSErr C_SndDoCommand(SndChannelPtr chanp,
                                 SndCommand *cmdp, BOOLEAN nowait);
PASCAL_TRAP(SndDoCommand, 0xA803);
extern trap OSErr C_SndDoImmediate(SndChannelPtr chanp,
                                   SndCommand *cmdp);
PASCAL_TRAP(SndDoImmediate, 0xA804);
extern trap OSErr C_SndControl(INTEGER id, SndCommand *cmdp);
PASCAL_TRAP(SndControl, 0xA806);

extern trap OSErr C_SndDisposeChannel(SndChannelPtr chanp,
                                      BOOLEAN quitnow);
PASCAL_TRAP(SndDisposeChannel, 0xA801);
extern void C_FinaleUnknown1(void);
extern OSErr C_FinaleUnknown2(ResType, LONGINT, Ptr, Ptr);
extern long C_DirectorUnknown3(void);
extern INTEGER C_DirectorUnknown4(ResType, INTEGER, Ptr, Ptr);
}
#endif /* __SOUNDMGR__ */
