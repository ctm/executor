//
//  SpeechManager-MacBridge.h
//  CocoaExecutor
//
//  Created by C.W. Betts on 8/4/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#ifndef __CocoaExecutor__SpeechManager_MacBridge__
#define __CocoaExecutor__SpeechManager_MacBridge__

#include "SpeechManager.h"

namespace MacBridge {
  PUBLIC Executor::NumVersion SpeechManagerVersion (void);
  PUBLIC int16 SpeechBusy (void);
  PUBLIC int16 SpeechBusySystemWide(void);
  PUBLIC Executor::OSErr CountVoices (int16 *numVoices);
  PUBLIC Executor::OSErr DisposeSpeechChannel (Executor::SpeechChannel chan);
  PUBLIC Executor::OSErr SpeakString (Executor::Str255 textToBeSpoken);
  
  PUBLIC Executor::OSErr StopSpeech (Executor::SpeechChannel chan);
  PUBLIC Executor::OSErr ContinueSpeech (Executor::SpeechChannel chan);
  
  PUBLIC Executor::OSErr GetIndVoice (int16 index, Executor::VoiceSpec *voice);
  PUBLIC Executor::OSErr NewSpeechChannel (Executor::VoiceSpec *voice, Executor::SpeechChannel *chan);
  PUBLIC Executor::OSErr StopSpeechAt (Executor::SpeechChannel chan, int32 whereToStop);
  PUBLIC Executor::OSErr PauseSpeechAt (Executor::SpeechChannel chan, int32 whereToPause);
  PUBLIC Executor::OSErr SetSpeechRate(Executor::SpeechChannel chan, Executor::Fixed rate);
  PUBLIC Executor::OSErr GetSpeechRate (Executor::SpeechChannel chan, Executor::Fixed *rate);
  PUBLIC Executor::OSErr SetSpeechPitch (Executor::SpeechChannel chan, Executor::Fixed pitch);
  PUBLIC Executor::OSErr GetSpeechPitch (Executor::SpeechChannel chan, Executor::Fixed *pitch);
  PUBLIC Executor::OSErr UseDictionary (Executor::SpeechChannel chan, Executor::Handle dictionary);
  PUBLIC Executor::OSErr MakeVoiceSpec (Executor::OSType creator, Executor::OSType id, Executor::VoiceSpec *voice);
  PUBLIC Executor::OSErr GetVoiceDescription (
									  const Executor::VoiceSpec *voice,
									  Executor::VoiceDescription *info,
									  Executor::LONGINT infoLength
									  );
  PUBLIC Executor::OSErr GetVoiceInfo (
							   const Executor::VoiceSpec *voice,
							   Executor::OSType selector,
							   void *voiceInfo
							   );
  PUBLIC Executor::OSErr SpeakText (Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes);
  PUBLIC Executor::OSErr SetSpeechInfo (
								Executor::SpeechChannel chan,
								Executor::OSType selector,
								const void *speechInfo
								);
  PUBLIC Executor::OSErr GetSpeechInfo (
								Executor::SpeechChannel chan,
								Executor::OSType selector,
								void *speechInfo
								);
  PUBLIC Executor::OSErr SpeakBuffer (
							  Executor::SpeechChannel chan,
							  const void *textBuf,
							  Executor::ULONGINT textBytes,
							  int32 controlFlags
							  );
  PUBLIC Executor::OSErr TextToPhonemes (Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes, Executor::Handle phonemeBuf, Executor::LONGINT *phonemeBytes);

}

#endif /* defined(__CocoaExecutor__SpeechManager_MacBridge__) */
