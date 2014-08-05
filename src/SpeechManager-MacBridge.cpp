//
//  SpeechManager-MacBridge.cpp
//  CocoaExecutor
//
//  Created by C.W. Betts on 8/4/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#include <Carbon/Carbon.h>
#include "rsys/common.h"
#include "rsys/hfs.h"
#include "MemoryMgr.h"
#include "SpeechManager-MacBridge.h"

Executor::NumVersion MacBridge::SpeechManagerVersion (void)
{
#if 0
  //The NumVersion struct on OS X is Endian-safe, so we can do this.
#pragma pack(push, 2)
  union ourNumVers {
	::NumVersion MacVers;
	Executor::NumVersion ExecutorVers;
  } ourNumVersn;
#pragma pack(pop)
  
  ourNumVersn.MacVers = ::SpeechManagerVersion();
  return ourNumVersn.ExecutorVers;
#else
  ::NumVersion theVers = ::SpeechManagerVersion();
  return theVers.majorRev << 24 | theVers.minorAndBugRev << 16 | theVers.stage << 8 | theVers.nonRelRev;
#endif
}

int16 MacBridge::SpeechBusy (void)
{
  SInt16 toRet = ::SpeechBusy();
  return CW(toRet);
}

int16 MacBridge::SpeechBusySystemWide(void)
{
  SInt16 toRet = ::SpeechBusySystemWide();
  return CW(toRet);
}

Executor::OSErr MacBridge::CountVoices (int16 *numVoices)
{
  if (!numVoices) {
    return CWC((Executor::OSErr)paramErr);
  }
  SInt16 voiceCount = 0;
  ::OSErr toRet = ::CountVoices(&voiceCount);
  
  *numVoices = CW(voiceCount);
  return CW(toRet);
  return noErr;

}

Executor::OSErr MacBridge::DisposeSpeechChannel (Executor::SpeechChannel chan)
{
  ::OSErr toRet = ::DisposeSpeechChannel((::SpeechChannel)chan);

  return CW(toRet);
}

Executor::OSErr MacBridge::SpeakString (Executor::Str255 textToBeSpoken)
{
  ::OSErr toRet = ::SpeakString(textToBeSpoken);

  return CW(toRet);
}

Executor::OSErr MacBridge::StopSpeech (Executor::SpeechChannel chan)
{
  ::OSErr toRet = ::StopSpeech((::SpeechChannel)chan);
  
  return CW(toRet);

}

Executor::OSErr MacBridge::ContinueSpeech (Executor::SpeechChannel chan)
{
  ::OSErr toRet = ::StopSpeech((::SpeechChannel)chan);

  return CW(toRet);
}

static inline void MacVoiceSpecToExecutorVoiceSpec(Executor::VoiceSpec* ExecutorVoice, ::VoiceSpec *MacVoice)
{
  ExecutorVoice->creator = CL(MacVoice->creator);
  ExecutorVoice->id = CL(MacVoice->id);
}

Executor::OSErr MacBridge::GetIndVoice (int16 index, Executor::VoiceSpec *voice)
{
  ::VoiceSpec macVoice = {0};
  if (!voice) {
    return CWC((Executor::OSErr)paramErr);
  }
  ::OSErr toRet = ::GetIndVoice(CW(index), &macVoice);
  
  MacVoiceSpecToExecutorVoiceSpec(voice, &macVoice);
  
  return CW(toRet);
}

Executor::OSErr MacBridge::NewSpeechChannel (Executor::VoiceSpec *voice, Executor::SpeechChannel *chan)
{
  return noErr;

}

Executor::OSErr MacBridge::StopSpeechAt (Executor::SpeechChannel chan, int32 whereToStop)
{
  ::OSErr toRet = ::StopSpeechAt((::SpeechChannel)chan, CL(whereToStop));
  
  return CW(toRet);
}

Executor::OSErr MacBridge::PauseSpeechAt (Executor::SpeechChannel chan, int32 whereToPause)
{
  ::OSErr toRet = ::PauseSpeechAt((::SpeechChannel)chan, CL(whereToPause));
  
  return CW(toRet);
}

Executor::OSErr MacBridge::SetSpeechRate(Executor::SpeechChannel chan, Executor::Fixed rate)
{
  ::OSErr toRet = ::SetSpeechRate((::SpeechChannel)chan, CL(rate));
  
  return CW(toRet);
}

PUBLIC Executor::OSErr MacBridge::GetSpeechRate (Executor::SpeechChannel chan, Executor::Fixed *rate)
{
  if (!rate) {
    return CWC((Executor::OSErr)paramErr);
  }
  ::OSErr toRet;
  ::Fixed ourFixed = CL(*rate);
  toRet = ::GetSpeechRate((::SpeechChannel)chan, &ourFixed);
  *rate = CL(ourFixed);
  
  return CW(toRet);
}

Executor::OSErr MacBridge::SetSpeechPitch (Executor::SpeechChannel chan, Executor::Fixed pitch)
{
  ::OSErr toRet = ::SetSpeechPitch((::SpeechChannel)chan, CL(pitch));
  
  return CW(toRet);
}

Executor::OSErr MacBridge::GetSpeechPitch (Executor::SpeechChannel chan, Executor::Fixed *pitch)
{
  if (!pitch) {
    return CWC((Executor::OSErr)paramErr);
  }
  ::OSErr toRet;
  ::Fixed ourFixed = CL(*pitch);
  toRet = ::GetSpeechRate((::SpeechChannel)chan, &ourFixed);
  *pitch = CL(ourFixed);
  
  return CW(toRet);
}

#undef NewHandle

Executor::OSErr MacBridge::UseDictionary (Executor::SpeechChannel chan, Executor::Handle dictionary)
{
  ::Size ExecSize = CL(Executor::GetHandleSize(dictionary));
  ::Handle nativeHandle = ::NewHandle(ExecSize);
  memcpy(*nativeHandle, dictionary->p, ExecSize);
  
  ::OSErr toRet = ::UseDictionary((::SpeechChannel)chan, nativeHandle);
  
  ::DisposeHandle(nativeHandle);
  
  return CW(toRet);
}

Executor::OSErr MacBridge::MakeVoiceSpec (Executor::OSType creator, Executor::OSType id, Executor::VoiceSpec *voice)
{
  if (!voice) {
    return CW((Executor::OSErr)paramErr);
  }
  ::VoiceSpec nativeSpec = {0};
  ::OSErr toRet = ::MakeVoiceSpec(CL(creator), CL(id), &nativeSpec);
  MacVoiceSpecToExecutorVoiceSpec(voice, &nativeSpec);
  
  return CW(toRet);
}

Executor::OSErr MacBridge::GetVoiceDescription (
											const Executor::VoiceSpec *voice,
											Executor::VoiceDescription *info,
											Executor::LONGINT infoLength
											)
{
  return noErr;

}

Executor::OSErr MacBridge::GetVoiceInfo (const Executor::VoiceSpec *voice, Executor::OSType selector, void *voiceInfo)
{
  // TODO: handle different data types
  ::OSErr toRet = ::GetVoiceInfo((const ::VoiceSpec*)voice, CL(selector), voiceInfo);

  return CW(toRet);
}

Executor::OSErr MacBridge::SpeakText (Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes)
{
  ::OSErr toRet = ::SpeakText((::SpeechChannel)chan, textBuf, CL(textBytes));

  return CW(toRet);
}

Executor::OSErr MacBridge::SetSpeechInfo (
									  Executor::SpeechChannel chan,
									  Executor::OSType selector,
									  const void *speechInfo
									  )
{
  // TODO: handle different data types
  ::OSErr toRet = ::SetSpeechInfo((::SpeechChannel)chan, CL(selector), speechInfo);

  return CW(toRet);
}

Executor::OSErr MacBridge::GetSpeechInfo (
									  Executor::SpeechChannel chan,
									  Executor::OSType selector,
									  void *speechInfo
									  )
{
  // TODO: handle different data types
  ::OSErr toRet = ::GetSpeechInfo((::SpeechChannel)chan, CL(selector), speechInfo);

  return CW(toRet);
}

Executor::OSErr MacBridge::SpeakBuffer (
									Executor::SpeechChannel chan,
									const void *textBuf,
									Executor::ULONGINT textBytes,
									int32 controlFlags
									)
{
  ::OSErr toRet = ::SpeakBuffer((::SpeechChannel)chan, textBuf, CL(textBytes), CL(controlFlags));

  return CW(toRet);
}

Executor::OSErr MacBridge::TextToPhonemes (Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes, Executor::Handle phonemeBuf, Executor::LONGINT *phonemeBytes)
{
  ::Size ExecSize = CL(Executor::GetHandleSize(phonemeBuf));
  ::Handle nativeHandle = ::NewHandle(ExecSize);
  long tempPhonemes = CL(*phonemeBytes);
  Executor::LONGINT intPhonemes;
  memcpy(*nativeHandle, phonemeBuf->p, ExecSize);

  ::OSErr toRet = ::TextToPhonemes((::SpeechChannel)chan, textBuf, CL(textBytes), nativeHandle, &tempPhonemes);
  intPhonemes = tempPhonemes;
  *phonemeBytes = CL(intPhonemes);
  ::DisposeHandle(nativeHandle);

  return CW(toRet);
}
