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

namespace MacBridge
{
Executor::NumVersion SpeechManagerVersion(void);
int16_t SpeechBusy(void);
int16_t SpeechBusySystemWide(void);
Executor::OSErr CountVoices(int16_t *numVoices);
Executor::OSErr DisposeSpeechChannel(Executor::SpeechChannel chan);
Executor::OSErr SpeakString(Executor::Str255 textToBeSpoken);

Executor::OSErr StopSpeech(Executor::SpeechChannel chan);
Executor::OSErr ContinueSpeech(Executor::SpeechChannel chan);

Executor::OSErr GetIndVoice(int16_t index, Executor::VoiceSpec *voice);
Executor::OSErr NewSpeechChannel(Executor::VoiceSpec *voice, Executor::SpeechChannel *chan);
Executor::OSErr StopSpeechAt(Executor::SpeechChannel chan, int32_t whereToStop);
Executor::OSErr PauseSpeechAt(Executor::SpeechChannel chan, int32_t whereToPause);
Executor::OSErr SetSpeechRate(Executor::SpeechChannel chan, Executor::Fixed rate);
Executor::OSErr GetSpeechRate(Executor::SpeechChannel chan, Executor::Fixed *rate);
Executor::OSErr SetSpeechPitch(Executor::SpeechChannel chan, Executor::Fixed pitch);
Executor::OSErr GetSpeechPitch(Executor::SpeechChannel chan, Executor::Fixed *pitch);
Executor::OSErr UseDictionary(Executor::SpeechChannel chan, Executor::Handle dictionary);
Executor::OSErr MakeVoiceSpec(Executor::OSType creator, Executor::OSType id, Executor::VoiceSpec *voice);
Executor::OSErr GetVoiceDescription(
    const Executor::VoiceSpec *voice,
    Executor::VoiceDescription *info,
    Executor::LONGINT infoLength);
Executor::OSErr GetVoiceInfo(
    const Executor::VoiceSpec *voice,
    Executor::OSType selector,
    void *voiceInfo);
Executor::OSErr SpeakText(Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes);
Executor::OSErr SetSpeechInfo(
    Executor::SpeechChannel chan,
    Executor::OSType selector,
    const void *speechInfo);
Executor::OSErr GetSpeechInfo(
    Executor::SpeechChannel chan,
    Executor::OSType selector,
    void *speechInfo);
Executor::OSErr SpeakBuffer(
    Executor::SpeechChannel chan,
    const void *textBuf,
    Executor::ULONGINT textBytes,
    int32_t controlFlags);
Executor::OSErr TextToPhonemes(Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes, Executor::Handle phonemeBuf, Executor::GUEST<Executor::LONGINT> *phonemeBytes);
}

#endif /* defined(__CocoaExecutor__SpeechManager_MacBridge__) */
