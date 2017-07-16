//
//  SpeechManager-MacBridge.cpp
//  CocoaExecutor
//
//  Created by C.W. Betts on 8/4/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#include <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
#include "rsys/common.h"
#include "rsys/hfs.h"
#include "MemoryMgr.h"
#include "SpeechManager-MacBridge.h"

#include <vector>
#include <map>

#define VoiceCreatorIDKey @"VoiceSynthesizerNumericID"
#define VoiceIDKey @"VoiceNumericID"
#define VoiceNameKey @"VoiceName"

static NSSpeechSynthesizer *internalSynthesizer;
static NSArray *speechVoices;
static std::map<Executor::LONGINT, NSSpeechSynthesizer *> synthesizerMap;
static dispatch_once_t initSpeech = 0;

static dispatch_block_t initSpeechBlock= ^{
  synthesizerMap = std::map<Executor::LONGINT, NSSpeechSynthesizer *>();
  NSMutableArray *tmpVoices = [[NSMutableArray alloc] init];
  @autoreleasepool {
    for (NSString *aVoice in [NSSpeechSynthesizer availableVoices]) {
      NSDictionary *voiceDict = [NSSpeechSynthesizer attributesForVoice:aVoice];
      NSNumber *synthesizerID = voiceDict[@"VoiceSynthesizerNumericID"];
      NSNumber *voiceID = voiceDict[@"VoiceNumericID"];
      
      [tmpVoices addObject:@{VoiceNameKey: aVoice,
                             VoiceCreatorIDKey: synthesizerID,
                             VoiceIDKey: voiceID}];
    }
  }
  
  speechVoices = [tmpVoices copy];
  [tmpVoices release];
  internalSynthesizer = [[NSSpeechSynthesizer alloc] init];
};

#define BeginSpeech() dispatch_once(&initSpeech, initSpeechBlock)

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
  BeginSpeech();
  @autoreleasepool {
    SInt16 toRet = internalSynthesizer.speaking;
    
    return BigEndianValue(toRet);
  }
}

int16 MacBridge::SpeechBusySystemWide(void)
{
  @autoreleasepool {
    SInt16 toRet = [NSSpeechSynthesizer isAnyApplicationSpeaking];
    
    return BigEndianValue(toRet);
  }
}

Executor::OSErr MacBridge::CountVoices (int16 *numVoices)
{
  @autoreleasepool {
    if (!numVoices) {
      return CWC((Executor::OSErr)paramErr);
    }
    
    BeginSpeech();
    SInt16 voiceCount = [NSSpeechSynthesizer availableVoices].count;
    
    *numVoices = BigEndianValue(voiceCount);
    return CWC((Executor::OSErr)noErr);
  }
}

Executor::OSErr MacBridge::DisposeSpeechChannel(Executor::SpeechChannel chan)
{
  //BeginSpeech();
  Executor::LONGINT ourDat = chan->data[0];
  NSSpeechSynthesizer *synth = synthesizerMap[ourDat];
  Executor::DisposPtr((Executor::Ptr)chan);
  [synth release];
  ::OSErr toRet = noErr;
  synthesizerMap.erase(ourDat);

  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::SpeakString (Executor::Str255 textToBeSpoken)
{
  BeginSpeech();
  @autoreleasepool {
    NSString *ourStr = CFBridgingRelease(CFStringCreateWithPascalString(kCFAllocatorDefault, textToBeSpoken, kCFStringEncodingMacRoman));
    BOOL isBegin = [internalSynthesizer startSpeakingString:ourStr];
    ::OSErr toRet = isBegin ? noErr : -1;
    
    return BigEndianValue(toRet);
  }
}

Executor::OSErr MacBridge::StopSpeech (Executor::SpeechChannel chan)
{
  BeginSpeech();
  NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
  @autoreleasepool {
    [synth stopSpeaking];
  }
  ::OSErr toRet = noErr;
  
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::ContinueSpeech (Executor::SpeechChannel chan)
{
  BeginSpeech();
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    [synth continueSpeaking];
    ::OSErr toRet = noErr;
    
    return BigEndianValue(toRet);
  }
}

static inline void MacVoiceSpecToExecutorVoiceSpec(Executor::VoiceSpec &ExecutorVoice, ::VoiceSpec &MacVoice)
{
  ExecutorVoice.creator = BigEndianValue(MacVoice.creator);
  ExecutorVoice.id = BigEndianValue(MacVoice.id);
}

Executor::OSErr MacBridge::GetIndVoice (int16 index, Executor::VoiceSpec *voice)
{
  @autoreleasepool {
    BeginSpeech();
    if (!voice) {
      return CWC((Executor::OSErr)paramErr);
    }
    NSInteger voiceIndex = BigEndianValue(index);
    NSArray *voices = [NSSpeechSynthesizer availableVoices];
    if (voiceIndex >= voices.count) {
      return CWC((Executor::OSErr)(-244));
    }
    NSDictionary *voiceDict = [NSSpeechSynthesizer attributesForVoice:voices[voiceIndex]];
    
    NSNumber *synthesizerID = voiceDict[@"VoiceSynthesizerNumericID"];
    NSNumber *voiceID = voiceDict[@"VoiceNumericID"];
    
    voice->creator = BigEndianValue([synthesizerID unsignedIntValue]);
    voice->id = BigEndianValue([voiceID unsignedIntValue]);
    
  }
  
  return CWC((Executor::OSErr)(noErr));
}

using Executor::_NewPtr_flags;

Executor::OSErr MacBridge::NewSpeechChannel (Executor::VoiceSpec *voice, Executor::SpeechChannel *chan)
{
  static Executor::LONGINT speechChanData = 0;
  @autoreleasepool {
    BeginSpeech();
    NSString *voiceID = nil;
    ::OSType voiceIDFCC = BigEndianValue(voice->id);
    ::OSType voiceCreatorFCC = BigEndianValue(voice->creator);
    
    for (NSDictionary *aVoice in speechVoices) {
      
      //NSDictionary *voiceDict = [NSSpeechSynthesizer attributesForVoice:aVoice];
      NSNumber *synthesizerID = aVoice[VoiceCreatorIDKey];
      NSNumber *voiceID = aVoice[VoiceIDKey];
      
      if ([synthesizerID unsignedIntValue] == voiceCreatorFCC && [voiceID unsignedIntValue] == voiceIDFCC) {
        voiceID = aVoice[VoiceNameKey];
      }
    }
    
    if (voiceID == nil) {
      return CWC((Executor::OSErr)(-244));
    }
    
    Executor::SpeechChannelRecord aChan;
    aChan.data[0] = ++speechChanData;
    
    *chan = (Executor::SpeechChannel)NewPtr(sizeof(Executor::SpeechChannelRecord));
    
    **chan = aChan;
    NSSpeechSynthesizer *NSsynth = [[NSSpeechSynthesizer alloc] initWithVoice:voiceID];
    synthesizerMap[aChan.data[0]] = NSsynth;
    
    return CWC((Executor::OSErr)noErr);
  }
}

Executor::OSErr MacBridge::StopSpeechAt (Executor::SpeechChannel chan, int32 whereToStop)
{
  NSSpeechBoundary boundary;
  NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
  switch (whereToStop) {
      //kImmediate
    case CLC(0):
      boundary = NSSpeechImmediateBoundary;
      break;
      
      //kEndOfWord
    case CLC(1):
      boundary = NSSpeechWordBoundary;
      break;
      
    case CLC(2):
      boundary = NSSpeechSentenceBoundary;
      break;
      
    default:
      return CWC((Executor::OSErr)paramErr);
      break;
  }
  
  @autoreleasepool {
    [synth stopSpeakingAtBoundary:boundary];
  }
  return CWC((Executor::OSErr)noErr);
}

Executor::OSErr MacBridge::PauseSpeechAt (Executor::SpeechChannel chan, int32 whereToPause)
{
  NSSpeechBoundary boundary;
  NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
  switch (whereToPause) {
      //kImmediate
    case CLC(0):
      boundary = NSSpeechImmediateBoundary;
      break;
      
      //kEndOfWord
    case CLC(1):
      boundary = NSSpeechWordBoundary;
      break;
      
    case CLC(2):
      boundary = NSSpeechSentenceBoundary;
      break;
      
    default:
      return CWC((Executor::OSErr)paramErr);
      break;
  }
  
  @autoreleasepool {
    [synth pauseSpeakingAtBoundary:boundary];
  }
  
  return CWC((Executor::OSErr)noErr);
}

Executor::OSErr MacBridge::SetSpeechRate(Executor::SpeechChannel chan, Executor::Fixed rate)
{
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    
    ::Fixed ourRate = BigEndianValue(rate);
    
    synth.rate = FixedToFloat(ourRate);
  }
  
  return CWC((Executor::OSErr)noErr);
}

Executor::OSErr MacBridge::GetSpeechRate (Executor::SpeechChannel chan, Executor::Fixed *rate)
{
  if (!rate) {
    return CWC((Executor::OSErr)paramErr);
  }
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    
    ::Fixed ourRate = FloatToFixed(synth.rate);
    
    *rate = BigEndianValue(ourRate);
  }
  
  return CWC((Executor::OSErr)noErr);
}

Executor::OSErr MacBridge::SetSpeechPitch (Executor::SpeechChannel chan, Executor::Fixed pitch)
{
  ::OSErr wasSuccess = noErr;
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    
    ::Fixed ourRate = BigEndianValue(pitch);
    float ourFloatPitch = FixedToFloat(ourRate);
    
    wasSuccess = [synth setObject:@(ourFloatPitch) forProperty:NSSpeechPitchModProperty error:NULL] ? noErr : -1;
  }
  
  return BigEndianValue(wasSuccess);
}

Executor::OSErr MacBridge::GetSpeechPitch (Executor::SpeechChannel chan, Executor::Fixed *pitch)
{
  if (!pitch) {
    return CWC((Executor::OSErr)paramErr);
  }
  ::OSErr toRet = noErr;
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    
    NSNumber *ourNum = [synth objectForProperty:NSSpeechPitchModProperty error:nil];
    if (ourNum == NULL) {
      toRet = -1;
    } else {
      ::Fixed ourPitch = FloatToFixed(ourNum.floatValue);
      *pitch = BigEndianValue(ourPitch);
    }
  }
  
  return BigEndianValue(toRet);
}

#undef NewHandle

Executor::OSErr MacBridge::UseDictionary (Executor::SpeechChannel chan, Executor::Handle dictionary)
{
#if 0
  @autoreleasepool {
    ::Size ExecSize = BigEndianValue(Executor::GetHandleSize(dictionary));
    NSData *aData = [NSData dataWithBytes:dictionary->p length:ExecSize];
    NSSpeechSynthesizer *aSynth = synthesizerMap[chan];
  }
#endif
  warning_unimplemented (NULL_STRING);
  return CWC((Executor::OSErr)paramErr);
}

Executor::OSErr MacBridge::MakeVoiceSpec (Executor::OSType creator, Executor::OSType identifier, Executor::VoiceSpec *voice)
{
  if (!voice) {
    return CWC((Executor::OSErr)paramErr);
  }
  ::VoiceSpec nativeSpec = {0};
  ::OSErr toRet = ::MakeVoiceSpec(BigEndianValue(creator), BigEndianValue(identifier), &nativeSpec);
  MacVoiceSpecToExecutorVoiceSpec(*voice, nativeSpec);
  
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::GetVoiceDescription (
											const Executor::VoiceSpec *voice,
											Executor::VoiceDescription *info,
											Executor::LONGINT infoLength)
{
  if (BigEndianValue(infoLength) != 362) {
    return CWC((Executor::OSErr)paramErr);
  }
  if (voice == NULL || info == NULL) {
    return CWC((Executor::OSErr)paramErr);
  }
  
  @autoreleasepool {
    info->length = BigEndianValue((int32)362);
    //NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    NSString *voiceID = nil;
    ::OSType voiceIDFCC = BigEndianValue(voice->id);
    ::OSType voiceCreatorFCC = BigEndianValue(voice->creator);
    
    for (NSDictionary *aVoice in speechVoices) {
      
      //NSDictionary *voiceDict = [NSSpeechSynthesizer attributesForVoice:aVoice];
      NSNumber *synthesizerID = aVoice[VoiceCreatorIDKey];
      NSNumber *voiceID = aVoice[VoiceIDKey];
      
      if ([synthesizerID unsignedIntValue] == voiceCreatorFCC && [voiceID unsignedIntValue] == voiceIDFCC) {
        voiceID = aVoice[VoiceNameKey];
      }
    }
    if (voiceID == nil) {
      return CWC((Executor::OSErr)(-244));
    }
    
    NSDictionary *aVoiceInfo = [NSSpeechSynthesizer attributesForVoice:voiceID];
    NSString *name = aVoiceInfo[NSVoiceName];
    CFStringGetPascalString((CFStringRef)name, info->name, 63, kCFStringEncodingMacRoman);
    info->age = BigEndianValue([aVoiceInfo[NSVoiceAge] shortValue]);
    info->voice = *voice;
    info->version = 1; //TODO: get real version!
    {
      NSString *gender = aVoiceInfo[NSVoiceGender];
      if ([gender isEqualToString:NSVoiceGenderMale]) {
        info->gender = CWC((int16)1); //kMale
      } else if ([gender isEqualToString:NSVoiceGenderFemale]) {
        info->gender = CWC((int16)2); //kFemale
      } else {
        info->gender = CWC((int16)0);
      }
    }
    
    NSString *comment = aVoiceInfo[NSVoiceDemoText];
    CFStringGetPascalString((CFStringRef)comment, info->comment, 255, kCFStringEncodingMacRoman);
    
    LangCode aLang = 0;
    RegionCode aRegion = 0;
    ::OSStatus ourStat = ::LocaleStringToLangAndRegionCodes([aVoiceInfo[NSVoiceLocaleIdentifier] UTF8String], &aLang, &aRegion);
    if (ourStat != noErr) {
      ourStat = ::LocaleStringToLangAndRegionCodes("en-US", &aLang, &aRegion);
      if (ourStat != noErr) {
        return CWC((Executor::OSErr)(-244));
      }
    }
    info->language = BigEndianValue(aLang);
    info->region = BigEndianValue(aRegion);
    info->script = CWC((int16)NSMacOSRomanStringEncoding);
    memset(info->reserved, 0, sizeof(info->reserved));
  }
  
  return noErr;
}

Executor::OSErr MacBridge::GetVoiceInfo (const Executor::VoiceSpec *voice, Executor::OSType selector, void *voiceInfo)
{
#if 0
  @autoreleasepool {
    //NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    NSString *voiceID = nil;
    ::OSType voiceIDFCC = BigEndianValue(voice->id);
    ::OSType voiceCreatorFCC = BigEndianValue(voice->creator);
    
    for (NSDictionary *aVoice in speechVoices) {
      
      //NSDictionary *voiceDict = [NSSpeechSynthesizer attributesForVoice:aVoice];
      NSNumber *synthesizerID = aVoice[VoiceCreatorIDKey];
      NSNumber *voiceID = aVoice[VoiceIDKey];
      
      if ([synthesizerID unsignedIntValue] == voiceCreatorFCC && [voiceID unsignedIntValue] == voiceIDFCC) {
        voiceID = aVoice[VoiceNameKey];
      }
    }
    if (voiceID == nil) {
      return CWC((Executor::OSErr)(-244));
    }
    
    NSDictionary *aVoiceInfo = [NSSpeechSynthesizer attributesForVoice:voiceID];
  }
#endif
  // TODO: handle different data types
  ::OSErr toRet = noErr;
  //::OSErr toRet = ::GetVoiceInfo((const ::VoiceSpec*)voice, BigEndianValue(selector), voiceInfo);
  
  switch (selector) {
    case CLC(soVoiceFile):
      //TODO: implement?
      warning_unimplemented (NULL_STRING);
      return CWC((Executor::OSErr)fnfErr);
      break;
      
    case CLC(soVoiceDescription):
      return MacBridge::GetVoiceDescription(voice, (Executor::VoiceDescription*)voiceInfo, CWC((Executor::LONGINT)362));
      break;
      
    default:
      break;
  }
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::SpeakText (Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes)
{
  Executor::OSErr theErr = noErr;
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    
    NSString *ourStr = [[NSString alloc] initWithBytes:textBuf length:BigEndianValue(textBytes) encoding:NSMacOSRomanStringEncoding];
    
    if (![synth startSpeakingString:ourStr]) {
      theErr = -1;
    }
  }
  return BigEndianValue(theErr);
}

Executor::OSErr MacBridge::SetSpeechInfo (
									  Executor::SpeechChannel chan,
									  Executor::OSType selector,
									  const void *speechInfo
									  )
{
  // TODO: handle different data types
  //::OSErr toRet = ::SetSpeechInfo((::SpeechChannel)chan, BigEndianValue(selector), speechInfo);
  ::OSErr toRet = noErr;
  
  warning_unimplemented (NULL_STRING);
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::GetSpeechInfo (
                                          Executor::SpeechChannel chan,
                                          Executor::OSType selector,
                                          void *speechInfo)
{
  // TODO: handle different data types
  //::OSErr toRet = ::GetSpeechInfo((::SpeechChannel)chan, BigEndianValue(selector), speechInfo);
  ::OSErr toRet = noErr;

  warning_unimplemented (NULL_STRING);
  return BigEndianValue(toRet);
}

namespace MacBridge {
  typedef NS_OPTIONS(SInt32, SpeechFlags) {
    kNoEndingProsody = 1,
    kNoSpeechInterrupt = 2,
    kPreflightThenPause = 4
  };
}

Executor::OSErr MacBridge::SpeakBuffer (
									Executor::SpeechChannel chan,
									const void *textBuf,
									Executor::ULONGINT textBytes,
									int32 controlFlags
									)
{
  ::OSErr toRet = noErr;
  @autoreleasepool {
    //TODO: handle flags
    ::Size textSize = BigEndianValue(textBytes);
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    NSString *aStr = [[NSString alloc] initWithBytes:textBuf length:textSize encoding:NSMacOSRomanStringEncoding];
    if (![synth startSpeakingString:aStr]) {
      toRet = -1;
    }
  }
  
  return BigEndianValue(toRet);
}

Executor::OSErr MacBridge::TextToPhonemes (Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes, Executor::Handle phonemeBuf, Executor::LONGINT *phonemeBytes)
{
  ::Size ExecSize = BigEndianValue(Executor::GetHandleSize(phonemeBuf));
  ::Size textSize = BigEndianValue(textBytes);
  ::OSErr toRet = noErr;
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    NSString *aStr = [[NSString alloc] initWithBytes:textBuf length:textSize encoding:NSMacOSRomanStringEncoding];
    NSString *phonemes = [synth phonemesFromText:aStr];
    [aStr release];
    Executor::LONGINT lengthInMacRoman = [phonemes lengthOfBytesUsingEncoding:NSMacOSRomanStringEncoding];
    
    if (lengthInMacRoman == 0) {
      return CWC((Executor::OSErr)-224);
    }
    if (lengthInMacRoman > ExecSize) {
      Executor::SetHandleSize(phonemeBuf, BigEndianValue(lengthInMacRoman));
    }
    strcpy((char*)phonemeBuf->p, [phonemes cStringUsingEncoding:NSMacOSRomanStringEncoding]);
    *phonemeBytes = BigEndianValue(lengthInMacRoman);
  }
  
  return BigEndianValue(toRet);
}
