//
//  SpeechManager.h
//  CocoaExecutor
//
//  Created by C.W. Betts on 8/4/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#ifndef __CocoaExecutor__SpeechManager__
#define __CocoaExecutor__SpeechManager__

#include "SoundMgr.h"
#include "FileMgr.h"

namespace Executor {
#pragma pack(push, 2)
  typedef struct VoiceSpec {
	OSType              creator;
	OSType              id;
  } VoiceSpec, *VoiceSpecPtr;

  
  typedef struct VoiceFileInfo {
	FSSpec	fileSpec;
	short	resID;
  } VoiceFileInfo;
  
  typedef struct SpeechStatusInfo {
	Boolean             outputBusy;
	Boolean             outputPaused;
	long                inputBytesLeft;
	short               phonemeCode;
  } SpeechStatusInfo;
  
  typedef struct VoiceDescription {
	int32              length;
	VoiceSpec           voice;
	int32              version;
	Str63               name;
	Str255              comment;
	int16              gender;
	int16              age;
	int16              script;
	int16              language;
	int16              region;
	int32              reserved[4];
  } VoiceDescription;

  typedef struct SpeechChannelRecord {
	LONGINT                data[1];
  } SpeechChannelRecord, *SpeechChannel;
  
  typedef struct PhonemeInfo {
	short               opcode;
	Str15               phStr;
	Str31               exampleStr;
	short               hiliteStart;
	short               hiliteEnd;
  } PhonemeInfo;
  
  typedef struct PhonemeDescriptor {
	short               phonemeCount;
	PhonemeInfo         thePhonemes[1];
  } PhonemeDescriptor;
  
  typedef struct SpeechXtndData {
	OSType              synthCreator;
	Byte                synthData[2];
  } SpeechXtndData;
  
  typedef struct DelimiterInfo {
	Byte                startDelimiter[2];
	Byte                endDelimiter[2];
  } DelimiterInfo;
  
  
#pragma pack(pop)
  
  PUBLIC pascal NumVersion C_SpeechManagerVersion (void);
  PUBLIC pascal int16 C_SpeechBusy (void);
  PUBLIC pascal int16 C_SpeechBusySystemWide(void);
  PUBLIC pascal OSErr C_CountVoices (int16 *numVoices);
  PUBLIC pascal OSErr C_DisposeSpeechChannel (SpeechChannel chan);
  PUBLIC pascal OSErr C_SpeakString (Str255 textToBeSpoken);
  
  PUBLIC pascal OSErr C_StopSpeech (SpeechChannel chan);
  PUBLIC pascal OSErr C_ContinueSpeech (SpeechChannel chan);
  
  PUBLIC pascal OSErr C_GetIndVoice (int16 index, VoiceSpec *voice);
  PUBLIC pascal OSErr C_NewSpeechChannel (VoiceSpec *voice, SpeechChannel *chan);
  PUBLIC pascal OSErr C_StopSpeechAt (SpeechChannel chan, int32 whereToStop);
  PUBLIC pascal OSErr C_PauseSpeechAt (SpeechChannel chan, int32 whereToPause);
  PUBLIC pascal OSErr C_SetSpeechRate(SpeechChannel chan, Fixed rate);
  PUBLIC pascal OSErr C_GetSpeechRate (SpeechChannel chan, Fixed *rate);
  PUBLIC pascal OSErr C_SetSpeechPitch (SpeechChannel chan, Fixed pitch);
  PUBLIC pascal OSErr C_GetSpeechPitch (SpeechChannel chan, Fixed *pitch);
  PUBLIC pascal OSErr C_UseDictionary (SpeechChannel chan, Handle dictionary);
  PUBLIC pascal OSErr C_MakeVoiceSpec (OSType creator, OSType id, VoiceSpec *voice);
  PUBLIC pascal OSErr C_GetVoiceDescription (const VoiceSpec *voice, VoiceDescription *info, LONGINT infoLength);
  PUBLIC pascal OSErr C_GetVoiceInfo (const VoiceSpec *voice, OSType selector, void *voiceInfo);
  PUBLIC pascal OSErr C_SpeakText (SpeechChannel chan, const void *textBuf, ULONGINT textBytes);
  PUBLIC pascal OSErr C_SetSpeechInfo (SpeechChannel chan, OSType selector, const void *speechInfo);
  PUBLIC pascal OSErr C_GetSpeechInfo (SpeechChannel chan, OSType selector, void *speechInfo);
  PUBLIC pascal OSErr C_SpeakBuffer (SpeechChannel chan, const void *textBuf, ULONGINT textBytes, int32 controlFlags);
  PUBLIC pascal OSErr C_TextToPhonemes (SpeechChannel chan, const void *textBuf, ULONGINT textBytes, Handle phonemeBuf, LONGINT *phonemeBytes);
}

#endif /* defined(__CocoaExecutor__SpeechManager__) */
