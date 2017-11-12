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

#ifdef MACOSX_
#include "SpeechManager-MacBridge.h"
#endif

using namespace Executor;

P0(PUBLIC pascal, NumVersion, SpeechManagerVersion)
{
#ifdef MACOSX_
    return MacBridge::SpeechManagerVersion();
#else
    warning_unimplemented(NULL_STRING);
    return 0;
#endif
}

P0(PUBLIC pascal, int16, SpeechBusy)
{
#ifdef MACOSX_
    return MacBridge::SpeechBusy();
#else
    warning_unimplemented(NULL_STRING);
    return 0;
#endif
}

P0(PUBLIC pascal, int16, SpeechBusySystemWide)
{
#ifdef MACOSX_
    return MacBridge::SpeechBusySystemWide();
#else
    warning_unimplemented(NULL_STRING);
    return 0;
#endif
}

P1(PUBLIC pascal, OSErr, CountVoices, int16 *, numVoices)
{
#ifdef MACOSX_
    return MacBridge::CountVoices(numVoices);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P1(PUBLIC pascal, OSErr, DisposeSpeechChannel, SpeechChannel, chan)
{
#ifdef MACOSX_
    return MacBridge::DisposeSpeechChannel(chan);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P1(PUBLIC pascal, OSErr, SpeakString, Str255, textToBeSpoken)
{
#ifdef MACOSX_
    return MacBridge::SpeakString(textToBeSpoken);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P1(PUBLIC pascal, OSErr, StopSpeech, SpeechChannel, chan)
{
#ifdef MACOSX_
    return MacBridge::StopSpeech(chan);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P1(PUBLIC pascal, OSErr, ContinueSpeech, SpeechChannel, chan)
{
#ifdef MACOSX_
    return MacBridge::ContinueSpeech(chan);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P2(PUBLIC pascal, OSErr, GetIndVoice, int16, index, VoiceSpec *, voice)
{
#ifdef MACOSX_
    return MacBridge::GetIndVoice(index, voice);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P2(PUBLIC pascal, OSErr, NewSpeechChannel, VoiceSpec *, voice, SpeechChannel *, chan)
{
#ifdef MACOSX_
    return MacBridge::NewSpeechChannel(voice, chan);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P2(PUBLIC pascal, OSErr, StopSpeechAt, SpeechChannel, chan, int32, whereToStop)
{
#ifdef MACOSX_
    return MacBridge::StopSpeechAt(chan, whereToStop);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P2(PUBLIC pascal, OSErr, PauseSpeechAt, SpeechChannel, chan, int32, whereToPause)
{
#ifdef MACOSX_
    return MacBridge::PauseSpeechAt(chan, whereToPause);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P2(PUBLIC pascal, OSErr, SetSpeechRate, SpeechChannel, chan, Fixed, rate)
{
#ifdef MACOSX_
    return MacBridge::SetSpeechRate(chan, rate);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P2(PUBLIC pascal, OSErr, GetSpeechRate, SpeechChannel, chan, Fixed *, rate)
{
#ifdef MACOSX_
    return MacBridge::GetSpeechRate(chan, rate);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P2(PUBLIC pascal, OSErr, SetSpeechPitch, SpeechChannel, chan, Fixed, pitch)
{
#ifdef MACOSX_
    return MacBridge::SetSpeechPitch(chan, pitch);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P2(PUBLIC pascal, OSErr, GetSpeechPitch, SpeechChannel, chan, Fixed *, pitch)
{
#ifdef MACOSX_
    return MacBridge::GetSpeechPitch(chan, pitch);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P2(PUBLIC pascal, OSErr, UseDictionary, SpeechChannel, chan, Handle, dictionary)
{
#ifdef MACOSX_
    return MacBridge::UseDictionary(chan, dictionary);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P3(PUBLIC pascal, OSErr, MakeVoiceSpec, OSType, creator, OSType, id, VoiceSpec *, voice)
{
#ifdef MACOSX_
    return MacBridge::MakeVoiceSpec(creator, id, voice);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P3(PUBLIC pascal, OSErr, GetVoiceDescription, const VoiceSpec *, voice, VoiceDescription *, info, LONGINT, infoLength)
{
#ifdef MACOSX_
    return MacBridge::GetVoiceDescription(voice, info, infoLength);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P3(PUBLIC pascal, OSErr, GetVoiceInfo, const VoiceSpec *, voice, OSType, selector, void *, voiceInfo)
{
#ifdef MACOSX_
    return MacBridge::GetVoiceInfo(voice, selector, voiceInfo);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P3(PUBLIC pascal, OSErr, SpeakText, SpeechChannel, chan, const void *, textBuf, ULONGINT, textBytes)
{
#ifdef MACOSX_
    return MacBridge::SpeakText(chan, textBuf, textBytes);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P3(PUBLIC pascal, OSErr, SetSpeechInfo, SpeechChannel, chan, OSType, selector, const void *, speechInfo)
{
#ifdef MACOSX_
    return MacBridge::SetSpeechInfo(chan, selector, speechInfo);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P3(PUBLIC pascal, OSErr, GetSpeechInfo, SpeechChannel, chan, OSType, selector, void *, speechInfo)
{
#ifdef MACOSX_
    return MacBridge::GetSpeechInfo(chan, selector, speechInfo);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P4(PUBLIC pascal, OSErr, SpeakBuffer, SpeechChannel, chan, const void *, textBuf, ULONGINT, textBytes, int32, controlFlags)
{
#ifdef MACOSX_
    return MacBridge::SpeakBuffer(chan, textBuf, textBytes, controlFlags);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}

P5(PUBLIC pascal, OSErr, TextToPhonemes, SpeechChannel, chan, const void *, textBuf, ULONGINT, textBytes, Handle, phonemeBuf, GUEST<LONGINT> *, phonemeBytes)
{
#ifdef MACOSX_
    return MacBridge::TextToPhonemes(chan, textBuf, textBytes, phonemeBuf, phonemeBytes);
#else
    warning_unimplemented(NULL_STRING);
    return paramErr;
#endif
}
