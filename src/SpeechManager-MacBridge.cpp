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

using namespace ByteSwap;

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

  return BigEndianValue(toRet);
}

int16 MacBridge::SpeechBusySystemWide(void)
{
  SInt16 toRet = ::SpeechBusySystemWide();

  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::CountVoices (int16 *numVoices)
{
  if (!numVoices) {
    return CWC((Executor::OSErr)paramErr);
  }
  SInt16 voiceCount = 0;
  ::OSErr toRet = ::CountVoices(&voiceCount);
  
  *numVoices = BigEndianValue(voiceCount);
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::DisposeSpeechChannel (Executor::SpeechChannel chan)
{
  ::OSErr toRet = ::DisposeSpeechChannel((::SpeechChannel)chan);

  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::SpeakString (Executor::Str255 textToBeSpoken)
{
  ::OSErr toRet = ::SpeakString(textToBeSpoken);

  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::StopSpeech (Executor::SpeechChannel chan)
{
  ::OSErr toRet = ::StopSpeech((::SpeechChannel)chan);
  
  return BigEndianValue(toRet);

}

Executor::OSErr MacBridge::ContinueSpeech (Executor::SpeechChannel chan)
{
  ::OSErr toRet = ::StopSpeech((::SpeechChannel)chan);

  return BigEndianValue(toRet);
}

static inline void MacVoiceSpecToExecutorVoiceSpec(Executor::VoiceSpec* ExecutorVoice, ::VoiceSpec *MacVoice)
{
  ExecutorVoice->creator = BigEndianValue(MacVoice->creator);
  ExecutorVoice->id = BigEndianValue(MacVoice->id);
}

Executor::OSErr MacBridge::GetIndVoice (int16 index, Executor::VoiceSpec *voice)
{
  ::VoiceSpec macVoice = {0};
  if (!voice) {
    return CWC((Executor::OSErr)paramErr);
  }
  ::OSErr toRet = ::GetIndVoice(BigEndianValue(index), &macVoice);
  
  MacVoiceSpecToExecutorVoiceSpec(voice, &macVoice);
  
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::NewSpeechChannel (Executor::VoiceSpec *voice, Executor::SpeechChannel *chan)
{
  ::VoiceSpec macVoice = {.creator = (OSType)BigEndianValue(voice->creator),
  .id = (OSType)BigEndianValue(voice->id)};

  ::OSErr toRet = ::NewSpeechChannel(&macVoice, (::SpeechChannel*)chan);
  
  MacVoiceSpecToExecutorVoiceSpec(voice, &macVoice);
  
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::StopSpeechAt (Executor::SpeechChannel chan, int32 whereToStop)
{
  ::OSErr toRet = ::StopSpeechAt((::SpeechChannel)chan, BigEndianValue(whereToStop));
  
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::PauseSpeechAt (Executor::SpeechChannel chan, int32 whereToPause)
{
  ::OSErr toRet = ::PauseSpeechAt((::SpeechChannel)chan, BigEndianValue(whereToPause));
  
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::SetSpeechRate(Executor::SpeechChannel chan, Executor::Fixed rate)
{
  ::OSErr toRet = ::SetSpeechRate((::SpeechChannel)chan, BigEndianValue(rate));
  
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::GetSpeechRate (Executor::SpeechChannel chan, Executor::Fixed *rate)
{
  if (!rate) {
    return CWC((Executor::OSErr)paramErr);
  }
  ::OSErr toRet;
  ::Fixed ourFixed = BigEndianValue(*rate);
  toRet = ::GetSpeechRate((::SpeechChannel)chan, &ourFixed);
  *rate = BigEndianValue(ourFixed);
  
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::SetSpeechPitch (Executor::SpeechChannel chan, Executor::Fixed pitch)
{
  ::OSErr toRet = ::SetSpeechPitch((::SpeechChannel)chan, BigEndianValue(pitch));
  
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::GetSpeechPitch (Executor::SpeechChannel chan, Executor::Fixed *pitch)
{
  if (!pitch) {
    return CWC((Executor::OSErr)paramErr);
  }
  ::OSErr toRet;
  ::Fixed ourFixed = BigEndianValue(*pitch);
  toRet = ::GetSpeechRate((::SpeechChannel)chan, &ourFixed);
  *pitch = BigEndianValue(ourFixed);
  
  return BigEndianValue(toRet);
}

#undef NewHandle

Executor::OSErr MacBridge::UseDictionary (Executor::SpeechChannel chan, Executor::Handle dictionary)
{
  ::Size ExecSize = BigEndianValue(Executor::GetHandleSize(dictionary));
  ::Handle nativeHandle = ::NewHandle(ExecSize);
  memcpy(*nativeHandle, dictionary->p, ExecSize);
  
  ::OSErr toRet = ::UseDictionary((::SpeechChannel)chan, nativeHandle);
  
  ::DisposeHandle(nativeHandle);
  
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::MakeVoiceSpec (Executor::OSType creator, Executor::OSType id, Executor::VoiceSpec *voice)
{
  if (!voice) {
    return CWC((Executor::OSErr)paramErr);
  }
  ::VoiceSpec nativeSpec = {0};
  ::OSErr toRet = ::MakeVoiceSpec(BigEndianValue(creator), BigEndianValue(id), &nativeSpec);
  MacVoiceSpecToExecutorVoiceSpec(voice, &nativeSpec);
  
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::GetVoiceDescription (
											const Executor::VoiceSpec *voice,
											Executor::VoiceDescription *info,
											Executor::LONGINT infoLength)
{
  return noErr;

}

Executor::OSErr MacBridge::GetVoiceInfo (const Executor::VoiceSpec *voice, Executor::OSType selector, void *voiceInfo)
{
  // TODO: handle different data types
  ::OSErr toRet = ::GetVoiceInfo((const ::VoiceSpec*)voice, BigEndianValue(selector), voiceInfo);

  switch (selector) {
	case CLC(soVoiceFile):
	{
	  //TODO: error checking
	  unsigned char cLocation[PATH_MAX] = {0};
	  ::VoiceFileInfo *theFile = (::VoiceFileInfo*)voiceInfo;
	  //OS X's and Executor's FSSpecs WILL point to different files.
	  ::FSSpec tmpSpec = theFile->fileSpec;
	  ::FSRef tmpRef = {0};
	  ::CFURLRef tmpURL = NULL;
	  ::FSpMakeFSRef(&tmpSpec, &tmpRef);
	  tmpURL = ::CFURLCreateFromFSRef(kCFAllocatorDefault, &tmpRef);
	  ::CFStringRef fileName = CFURLCopyLastPathComponent(tmpURL);
	  ::CFURLGetFileSystemRepresentation(tmpURL, false, cLocation, sizeof(cLocation));
	  Executor::Str255 strName = {0};
	  CFStringGetPascalString(fileName, strName, sizeof(strName) - 1, kCFStringEncodingMacRoman);
	  Executor::HVCB *customPart = Executor::ROMlib_vcbbybiggestunixname((const char*)cLocation);
	  Executor::FSSpecPtr tmpSpecPtr = (Executor::FSSpecPtr)&theFile->fileSpec;
	  Executor::C_FSMakeFSSpec(customPart->vcbDrvNum, customPart->vcbDirIDM, strName, tmpSpecPtr);
	  
	  theFile->resID = BigEndianValue(theFile->resID);
	  ::CFRelease(tmpURL);
	  ::CFRelease(fileName);
	}
	  break;
	  
	  case CLC(soVoiceDescription):
	{
	  Executor::VoiceSpec exVSpec = {0};
	  ::VoiceDescription* voiDesc = (::VoiceDescription*)voiceInfo;
	  voiDesc->length = BigEndianValue(voiDesc->length);
	  MacVoiceSpecToExecutorVoiceSpec(&exVSpec, &voiDesc->voice);
	  memcpy(&voiDesc->voice, &exVSpec, sizeof(exVSpec));
	  voiDesc->version = BigEndianValue(voiDesc->version);
	  voiDesc->gender = BigEndianValue(voiDesc->gender);
	  voiDesc->age = BigEndianValue(voiDesc->age);
	  voiDesc->script = BigEndianValue(voiDesc->script);
	  voiDesc->language = BigEndianValue(voiDesc->language);
	  voiDesc->region = BigEndianValue(voiDesc->region);
	  //voiDesc->voice.creator
	  
	}
	  break;
	  
	default:
	  break;
  }
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::SpeakText (Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes)
{
  ::OSErr toRet = ::SpeakText((::SpeechChannel)chan, textBuf, BigEndianValue(textBytes));

  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::SetSpeechInfo (
									  Executor::SpeechChannel chan,
									  Executor::OSType selector,
									  const void *speechInfo
									  )
{
  // TODO: handle different data types
  ::OSErr toRet = ::SetSpeechInfo((::SpeechChannel)chan, BigEndianValue(selector), speechInfo);

  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::GetSpeechInfo (
									  Executor::SpeechChannel chan,
									  Executor::OSType selector,
									  void *speechInfo
									  )
{
  // TODO: handle different data types
  ::OSErr toRet = ::GetSpeechInfo((::SpeechChannel)chan, BigEndianValue(selector), speechInfo);

  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::SpeakBuffer (
									Executor::SpeechChannel chan,
									const void *textBuf,
									Executor::ULONGINT textBytes,
									int32 controlFlags
									)
{
  ::OSErr toRet = ::SpeakBuffer((::SpeechChannel)chan, textBuf, BigEndianValue(textBytes), BigEndianValue(controlFlags));

  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::TextToPhonemes (Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes, Executor::Handle phonemeBuf, Executor::LONGINT *phonemeBytes)
{
  ::Size ExecSize = BigEndianValue(Executor::GetHandleSize(phonemeBuf));
  ::Handle nativeHandle = ::NewHandle(ExecSize);
  long tempPhonemes = BigEndianValue(*phonemeBytes);
  Executor::LONGINT intPhonemes;
  memcpy(*nativeHandle, phonemeBuf->p, ExecSize);

  ::OSErr toRet = ::TextToPhonemes((::SpeechChannel)chan, textBuf, BigEndianValue(textBytes), nativeHandle, &tempPhonemes);
  intPhonemes = tempPhonemes;
  *phonemeBytes = BigEndianValue(intPhonemes);
  ::DisposeHandle(nativeHandle);

  return BigEndianValue(toRet);
}
