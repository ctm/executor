//
//  SpeechManager.cpp
//  CocoaExecutor
//
//  Created by C.W. Betts on 8/4/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#include "rsys/common.h"
#include "SpeechManager.h"
#include "rsys/error.h"

#ifdef MACOSX
#include "SpeechManager-MacBridge.h"
#endif

using namespace Executor;

NumVersion Executor::C_SpeechManagerVersion()
{
#ifdef MACOSX
    return MacBridge::SpeechManagerVersion();
#else
    warning_unimplemented(NULL_STRING);
    return 0;
#endif
}

int16_t Executor::C_SpeechBusy()
{
#ifdef MACOSX
    return MacBridge::SpeechBusy();
#else
    warning_unimplemented(NULL_STRING);
    return 0;
#endif
}

int16_t Executor::C_SpeechBusySystemWide()
{
#ifdef MACOSX
    return MacBridge::SpeechBusySystemWide();
#else
    warning_unimplemented(NULL_STRING);
    return 0;
#endif
}

OSErr Executor::C_CountVoices(int16_t *numVoices)
{
#ifdef MACOSX
    return MacBridge::CountVoices(numVoices);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_DisposeSpeechChannel(SpeechChannel chan)
{
#ifdef MACOSX
    return MacBridge::DisposeSpeechChannel(chan);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_SpeakString(Str255 textToBeSpoken)
{
#ifdef MACOSX
    return MacBridge::SpeakString(textToBeSpoken);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_StopSpeech(SpeechChannel chan)
{
#ifdef MACOSX
    return MacBridge::StopSpeech(chan);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_ContinueSpeech(SpeechChannel chan)
{
#ifdef MACOSX
    return MacBridge::ContinueSpeech(chan);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_GetIndVoice(int16_t index, VoiceSpec *voice)
{
#ifdef MACOSX
    return MacBridge::GetIndVoice(index, voice);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_NewSpeechChannel(VoiceSpec *voice, SpeechChannel *chan)
{
#ifdef MACOSX
    return MacBridge::NewSpeechChannel(voice, chan);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_StopSpeechAt(SpeechChannel chan, int32_t whereToStop)
{
#ifdef MACOSX
    return MacBridge::StopSpeechAt(chan, whereToStop);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_PauseSpeechAt(SpeechChannel chan, int32_t whereToPause)
{
#ifdef MACOSX
    return MacBridge::PauseSpeechAt(chan, whereToPause);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_SetSpeechRate(SpeechChannel chan, Fixed rate)
{
#ifdef MACOSX
    return MacBridge::SetSpeechRate(chan, rate);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_GetSpeechRate(SpeechChannel chan, Fixed *rate)
{
#ifdef MACOSX
    return MacBridge::GetSpeechRate(chan, rate);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_SetSpeechPitch(SpeechChannel chan, Fixed pitch)
{
#ifdef MACOSX
    return MacBridge::SetSpeechPitch(chan, pitch);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_GetSpeechPitch(SpeechChannel chan, Fixed *pitch)
{
#ifdef MACOSX
    return MacBridge::GetSpeechPitch(chan, pitch);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_UseDictionary(SpeechChannel chan, Handle dictionary)
{
#ifdef MACOSX
    return MacBridge::UseDictionary(chan, dictionary);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_MakeVoiceSpec(OSType creator, OSType id, VoiceSpec *voice)
{
#ifdef MACOSX
    return MacBridge::MakeVoiceSpec(creator, id, voice);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_GetVoiceDescription(const VoiceSpec *voice,
                                      VoiceDescription *info,
                                      LONGINT infoLength)
{
#ifdef MACOSX
    return MacBridge::GetVoiceDescription(voice, info, infoLength);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_GetVoiceInfo(const VoiceSpec *voice, OSType selector,
                               void *voiceInfo)
{
#ifdef MACOSX
    return MacBridge::GetVoiceInfo(voice, selector, voiceInfo);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_SpeakText(SpeechChannel chan, const void *textBuf,
                            ULONGINT textBytes)
{
#ifdef MACOSX
    return MacBridge::SpeakText(chan, textBuf, textBytes);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_SetSpeechInfo(SpeechChannel chan, OSType selector,
                                const void *speechInfo)
{
#ifdef MACOSX
    return MacBridge::SetSpeechInfo(chan, selector, speechInfo);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_GetSpeechInfo(SpeechChannel chan, OSType selector,
                                void *speechInfo)
{
#ifdef MACOSX
    return MacBridge::GetSpeechInfo(chan, selector, speechInfo);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_SpeakBuffer(SpeechChannel chan, const void *textBuf,
                              ULONGINT textBytes, int32_t controlFlags)
{
#ifdef MACOSX
    return MacBridge::SpeakBuffer(chan, textBuf, textBytes, controlFlags);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

OSErr Executor::C_TextToPhonemes(SpeechChannel chan, const void *textBuf,
                                 ULONGINT textBytes, Handle phonemeBuf,
                                 GUEST<LONGINT> *phonemeBytes)
{
#ifdef MACOSX
    return MacBridge::TextToPhonemes(chan, textBuf, textBytes, phonemeBuf, phonemeBytes);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}
