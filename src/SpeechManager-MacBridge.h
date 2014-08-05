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

#define __private_extern __attribute__((visibility("hidden")))

namespace MacBridge {
  __private_extern Executor::NumVersion SpeechManagerVersion (void);
  __private_extern int16 SpeechBusy (void);
  __private_extern int16 SpeechBusySystemWide(void);
  __private_extern Executor::OSErr CountVoices (int16 *numVoices);
  __private_extern Executor::OSErr DisposeSpeechChannel (Executor::SpeechChannel chan);
  __private_extern Executor::OSErr SpeakString (Executor::Str255 textToBeSpoken);
  
  __private_extern Executor::OSErr StopSpeech (Executor::SpeechChannel chan);
  __private_extern Executor::OSErr ContinueSpeech (Executor::SpeechChannel chan);
  
  __private_extern Executor::OSErr GetIndVoice (int16 index, Executor::VoiceSpec *voice);
  __private_extern Executor::OSErr NewSpeechChannel (Executor::VoiceSpec *voice, Executor::SpeechChannel *chan);
  __private_extern Executor::OSErr StopSpeechAt (Executor::SpeechChannel chan, int32 whereToStop);
  __private_extern Executor::OSErr PauseSpeechAt (Executor::SpeechChannel chan, int32 whereToPause);
  __private_extern Executor::OSErr SetSpeechRate(Executor::SpeechChannel chan, Executor::Fixed rate);
  __private_extern Executor::OSErr GetSpeechRate (Executor::SpeechChannel chan, Executor::Fixed *rate);
  __private_extern Executor::OSErr SetSpeechPitch (Executor::SpeechChannel chan, Executor::Fixed pitch);
  __private_extern Executor::OSErr GetSpeechPitch (Executor::SpeechChannel chan, Executor::Fixed *pitch);
  __private_extern Executor::OSErr UseDictionary (Executor::SpeechChannel chan, Executor::Handle dictionary);
  __private_extern Executor::OSErr MakeVoiceSpec (Executor::OSType creator, Executor::OSType id, Executor::VoiceSpec *voice);
  __private_extern Executor::OSErr GetVoiceDescription (
									  const Executor::VoiceSpec *voice,
									  Executor::VoiceDescription *info,
									  Executor::LONGINT infoLength
									  );
  __private_extern Executor::OSErr GetVoiceInfo (
							   const Executor::VoiceSpec *voice,
							   Executor::OSType selector,
							   void *voiceInfo
							   );
  __private_extern Executor::OSErr SpeakText (Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes);
  __private_extern Executor::OSErr SetSpeechInfo (
								Executor::SpeechChannel chan,
								Executor::OSType selector,
								const void *speechInfo
								);
  __private_extern Executor::OSErr GetSpeechInfo (
								Executor::SpeechChannel chan,
								Executor::OSType selector,
								void *speechInfo
								);
  __private_extern Executor::OSErr SpeakBuffer (
							  Executor::SpeechChannel chan,
							  const void *textBuf,
							  Executor::ULONGINT textBytes,
							  int32 controlFlags
							  );
  __private_extern Executor::OSErr TextToPhonemes (Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes, Executor::Handle phonemeBuf, Executor::LONGINT *phonemeBytes);

}

#endif /* defined(__CocoaExecutor__SpeechManager_MacBridge__) */
