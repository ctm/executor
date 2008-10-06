#if !defined (__SOUNDMGR__)
#define __SOUNDMGR__

/*
 * Copyright 1992 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: SoundMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "QuickDraw.h"

typedef struct {
    INTEGER cmd	PACKED;
    INTEGER param1	PACKED;
    LONGINT param2	PACKED;
} SndCommand;

#define stdQLength	128

enum
{
  stdSH = 0,		/* standard sound header */
  cmpSH = 0xFE,		/* compressed sound header */
  extSH = 0xFF,		/* extended sound header */
};

typedef struct _SndChannel {
  struct _SndChannel *nextChan	PACKED_P;
  Ptr firstMod	PACKED_P;
  ProcPtr callBack	PACKED_P;
  LONGINT userInfo	PACKED;
  LONGINT wait	PACKED;
  SndCommand cmdInProg	PACKED;
  INTEGER flags	PACKED;
  INTEGER qLength	PACKED;
  INTEGER qHead	PACKED;
  INTEGER qTail	PACKED;
  SndCommand queue[stdQLength]	PACKED;
} SndChannel, *SndChannelPtr;

#define SND_CHAN_FLAGS_X(c) (c->flags)
#define SND_CHAN_FLAGS(c) (CW (SND_CHAN_FLAGS_X (c)))

typedef struct { SndChannelPtr p PACKED_P; } HIDDEN_SndChannelPtr;

enum {
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

typedef struct {
    LONGINT offset	PACKED;
    LONGINT nsamples	PACKED;
    LONGINT rate	PACKED;
    LONGINT altbegin	PACKED;
    LONGINT altend	PACKED;
    INTEGER basenote	PACKED;
    unsigned char buf[1]	PACKED;
} soundbuffer_t;

typedef struct _SoundHeader {
  Ptr samplePtr		PACKED_P;
  LONGINT length	PACKED;
  Fixed sampleRate	PACKED;
  LONGINT loopStart	PACKED;
  LONGINT loopEnd	PACKED;
  Byte encode		PACKED;
  Byte baseFrequency	PACKED;
  Byte sampleArea[0]	PACKED;
} SoundHeader, *SoundHeaderPtr;

typedef struct _ExtSoundHeader {
  Ptr samplePtr		PACKED_P;
  LONGINT numChannels	PACKED;
  Fixed sampleRate	PACKED;
  LONGINT loopStart	PACKED;
  LONGINT loopEnd	PACKED;
  Byte encode		PACKED;
  Byte baseFrequency	PACKED;
  LONGINT numFrames	PACKED;
  Extended AIFFSampleRate PACKED;  /* ???  should be Extended80 */
  Ptr MarkerChunk	PACKED;
  Ptr instrumentChunks	PACKED;
  Ptr AESRecording	PACKED;
  INTEGER sampleSize	PACKED;
  INTEGER futureUse1	PACKED;
  LONGINT futureUse2	PACKED;
  LONGINT futureUse3	PACKED;
  LONGINT futureUse4	PACKED;
  Byte sampleArea[0]	PACKED;
} ExtSoundHeader, *ExtSoundHeaderPtr;

enum {
    noteSynth = 1,
    waveTableSynth = 3,
    sampledSynth = 5,
    MIDISynthIn = 7,
    MIDISynthOut = 9,
};

#define badChannel		(-205)
#define badFormat		(-206)
#define noHardware		(-200)
#define notEnoughHardware	(-201)
#define queueFull		(-203)
#define resProblem		(-204)

#if !defined (SoundActive)
extern Byte SoundActive;
#endif

enum {
    soundactiveoff    = 0,
    soundactive5      = 5,
    soundactiveinplay = 0x81,
    soundactivenone   = 0xFF,
};

typedef struct
{
  LONGINT dbNumFrames	PACKED;
  LONGINT dbFlags	PACKED;
  LONGINT dbUserInfo[2]	PACKED;
  Byte dbSoundData[0]	PACKED;
} SndDoubleBuffer, *SndDoubleBufferPtr;

enum {
  dbBufferReady = 1,
  dbLastBuffer = 4
};

typedef struct
{
  INTEGER dbhNumChannels	PACKED;
  INTEGER dbhSampleSize		PACKED;
  INTEGER dbhCompressionID	PACKED;
  INTEGER dbhPacketSize		PACKED;
  Fixed dbhSampleRate		PACKED;
  SndDoubleBufferPtr dbhBufferPtr[2] PACKED;
  ProcPtr dbhDoubleBack		PACKED;
} SndDoubleBufferHeader, *SndDoubleBufferHeaderPtr;

typedef struct _SCSTATUS {
  Fixed scStartTime		PACKED;
  Fixed scEndTime		PACKED;
  Fixed scCurrentTime		PACKED;
  Boolean scChannelBusy		PACKED;
  Boolean scChannelDisposed	PACKED;
  Boolean scChannelPaused	PACKED;
  Boolean scUnused		PACKED;
  LONGINT scChannelAttributes	PACKED;
  LONGINT scCPULoad		PACKED;
} SCStatus, *SCStatusPtr;

#if 1       /* stub definitions */
typedef void *SMStatusPtr;
typedef LONGINT NumVersion;
typedef void *AudioSelectionPtr;
typedef void *SPBPtr;
#endif

typedef unsigned long UnsignedFixed;
typedef Ptr CompressionInfoPtr;

extern trap void C_SndGetSysBeepState(INTEGER *statep);

extern trap OSErr C_SndSetSysBeepState(INTEGER state);


extern trap OSErr C_SndChannelStatus(SndChannelPtr chanp, INTEGER length,
							  SCStatusPtr statusp);

extern trap OSErr C_SndManagerStatus(INTEGER length, SMStatusPtr statusp);

extern trap NumVersion C_SndSoundManagerVersion( void );

extern trap NumVersion C_MACEVersion( void );

extern trap NumVersion C_SPBVersion( void );

extern trap OSErr C_SndStartFilePlay(SndChannelPtr chanp,
	INTEGER refnum, INTEGER resnum, LONGINT buffersize, Ptr bufferp,
	  AudioSelectionPtr theselectionp, ProcPtr completionp, BOOLEAN async);

extern trap OSErr C_SndPauseFilePlay(SndChannelPtr chanp);

extern trap OSErr C_SndStopFilePlay(SndChannelPtr chanp, BOOLEAN async);


extern trap OSErr C_SndPlayDoubleBuffer(SndChannelPtr chanp,
					      SndDoubleBufferHeaderPtr paramp);


extern trap void C_Comp3to1(Ptr inp, Ptr outp, LONGINT cnt,
       Ptr instatep, Ptr outstatep, LONGINT numchannels, LONGINT whichchannel);

extern trap void C_Comp6to1(Ptr inp, Ptr outp, LONGINT cnt,
       Ptr instatep, Ptr outstatep, LONGINT numchannels, LONGINT whichchannel);

extern trap void C_Exp1to3(Ptr inp, Ptr outp, LONGINT cnt, Ptr instatep,
		     Ptr outstatep, LONGINT numchannels, LONGINT whichchannel);

extern trap void C_Exp1to6(Ptr inp, Ptr outp, LONGINT cnt, Ptr instatep,
		     Ptr outstatep, LONGINT numchannels, LONGINT whichchannel);


extern trap OSErr C_SndRecord(ProcPtr filterp, Point corner,
					   OSType quality, Handle *sndhandlep);

extern trap OSErr C_SndRecordToFile(ProcPtr filterp, Point corner,
					       OSType quality, INTEGER refnum);


extern trap OSErr C_SPBOpenDevice(Str255 name, INTEGER permission,
							   LONGINT *inrefnump);

extern trap OSErr C_SPBCloseDevice(LONGINT inrefnum);

extern trap OSErr C_SPBRecord(SPBPtr inparamp, BOOLEAN async);

extern trap OSErr C_SPBRecordToFile(INTEGER refnum, SPBPtr inparamp,
							        BOOLEAN async);

extern trap OSErr C_SPBPauseRecording(LONGINT refnum);

extern trap OSErr C_SPBResumeRecording(LONGINT refnum);

extern trap OSErr C_SPBStopRecording(LONGINT refnum);

extern trap OSErr C_SPBGetRecordingStatus(LONGINT refnum,
	INTEGER *recordingstatus, INTEGER *meterlevel,
	LONGINT *totalsampstorecord, LONGINT *numsampsrecorded,
		    LONGINT *totalmsecstorecord, LONGINT *numbermsecsrecorded);


extern trap OSErr C_SPBGetDeviceInfo(LONGINT refnum, OSType info,
								    Ptr infop);

extern trap OSErr C_SPBSetDeviceInfo(LONGINT refnum, OSType info,
								    Ptr infop);


extern trap OSErr C_SetupSndHeader(Handle sndhandle, INTEGER numchannels,
	Fixed rate, INTEGER size, OSType compresion, INTEGER basefreq,
				        LONGINT numbytes, INTEGER *headerlenp);

extern trap OSErr C_SetupAIFFHeader(INTEGER refnum, INTEGER numchannels,
	Fixed samplerate, INTEGER samplesize, OSType compression,
					  LONGINT numbytes, LONGINT numframes);

extern trap OSErr C_SPBSignInDevice(INTEGER refnum, Str255 name);

extern trap OSErr C_SPBSignOutDevice(INTEGER refnum);

extern trap OSErr C_SPBGetIndexedDevice(INTEGER count, Str255 name,
						    Handle *deviceiconhandlep);

extern trap OSErr C_SPBMillisecondsToBytes(LONGINT refnum,
							      LONGINT *millip);

extern trap OSErr C_SPBBytesToMilliseconds(LONGINT refnum,
							  LONGINT *bytecountp);

extern trap OSErr C_GetSysBeepVolume (LONGINT *levelp);

extern trap OSErr C_SetSysBeepVolume (LONGINT level);

extern trap OSErr C_GetDefaultOutputVolume (LONGINT *levelp);

extern trap OSErr C_SetDefaultOutputVolume (LONGINT level);

extern trap OSErr C_GetSoundHeaderOffset (Handle sndHandle, LONGINT *offset);

extern trap UnsignedFixed C_UnsignedFixedMulDiv (UnsignedFixed value,
						 UnsignedFixed multiplier,
						 UnsignedFixed divisor);

extern trap OSErr C_GetCompressionInfo (INTEGER compressionID, OSType format,
					INTEGER numChannels,
					INTEGER sampleSize,
					CompressionInfoPtr cp);

extern trap OSErr C_SetSoundPreference (OSType theType, Str255 name,
					Handle settings);

extern trap OSErr C_GetSoundPreference (OSType theType, Str255 name,
					Handle settings);

extern trap OSErr C_SndGetInfo (SndChannelPtr chan, OSType selector,
				void * infoPtr);

extern trap OSErr C_SndSetInfo (SndChannelPtr chan, OSType selector,
				void *infoPtr);


/* DO NOT DELETE THIS LINE */
#if !defined (__STDC__)
extern void StartSound(); 
extern void StopSound(); 
extern BOOLEAN SoundDone(); 
extern void GetSoundVol(); 
extern void SetSoundVol(); 
extern pascal trap OSErr SndPlay(); 
extern pascal trap OSErr SndNewChannel(); 
extern pascal trap OSErr SndAddModifier(); 
extern pascal trap OSErr SndDoCommand(); 
extern pascal trap OSErr SndDoImmediate(); 
extern pascal trap OSErr SndControl(); 
extern pascal trap OSErr SndDisposeChannel(); 
#else /* __STDC__ */
extern void StartSound( Ptr srec, LONGINT nb, ProcPtr comp ); 
extern void StopSound( void  ); 
extern BOOLEAN SoundDone( void  ); 
extern void GetSoundVol( INTEGER *volp ); 
extern void SetSoundVol( INTEGER vol ); 
extern trap OSErr C_SndPlay( SndChannelPtr chanp, Handle sndh, 
 BOOLEAN async ); extern pascal trap OSErr P_SndPlay( SndChannelPtr chanp, Handle sndh, 
 BOOLEAN async ); 
extern trap OSErr C_SndNewChannel( HIDDEN_SndChannelPtr *chanpp, 
 INTEGER synth, LONGINT init, ProcPtr userroutinep ); extern pascal trap OSErr P_SndNewChannel( HIDDEN_SndChannelPtr *chanpp, 
 INTEGER synth, LONGINT init, ProcPtr userroutinep ); 
extern trap OSErr C_SndAddModifier( SndChannelPtr chanp, 
 ProcPtr mod, INTEGER id, LONGINT init ); extern pascal trap OSErr P_SndAddModifier( SndChannelPtr chanp, 
 ProcPtr mod, INTEGER id, LONGINT init ); 
extern trap OSErr C_SndDoCommand( SndChannelPtr chanp, 
 SndCommand *cmdp, BOOLEAN nowait ); extern pascal trap OSErr P_SndDoCommand( SndChannelPtr chanp, 
 SndCommand *cmdp, BOOLEAN nowait ); 
extern trap OSErr C_SndDoImmediate( SndChannelPtr chanp, 
 SndCommand *cmdp ); extern pascal trap OSErr P_SndDoImmediate( SndChannelPtr chanp, 
 SndCommand *cmdp ); 
extern trap OSErr C_SndControl( INTEGER id, SndCommand *cmdp ); extern pascal trap OSErr P_SndControl( INTEGER id, SndCommand *cmdp); 
extern trap OSErr C_SndDisposeChannel( SndChannelPtr chanp, 
 BOOLEAN quitnow ); extern pascal trap OSErr P_SndDisposeChannel( SndChannelPtr chanp, 
 BOOLEAN quitnow ); 
extern void C_FinaleUnknown1( void );
extern OSErr C_FinaleUnknown2( ResType, LONGINT, Ptr, Ptr);
extern long C_DirectorUnknown3 (void);
extern INTEGER C_DirectorUnknown4 (ResType, INTEGER, Ptr, Ptr);

#endif /* __STDC__ */
#endif /* __SOUNDMGR__ */
