//
//  SpeechManager-MacBridge.cpp
//  CocoaExecutor
//
//  Created by C.W. Betts on 8/4/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#include <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
#define BOOL BOOL_EXEC
#undef YES
#undef NO
#define YES YES_EXEC
#define NO NO_EXEC
#include "rsys/common.h"
#include "rsys/hfs.h"
#include "MemoryMgr.h"
#include "rsys/byteswap.h"
#include "SpeechManager-MacBridge.h"
#undef BOOL
#undef YES
#undef NO
#define YES 1
#define NO 0

#include <vector>
#include <map>

#define VoiceCreatorIDKey @"VoiceSynthesizerNumericID"
#define VoiceIDKey @"VoiceNumericID"
#define VoiceNameKey @"VoiceName"

using namespace Executor;

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

int16_t MacBridge::SpeechBusy (void)
{
  BeginSpeech();
  @autoreleasepool {
    SInt16 toRet = internalSynthesizer.speaking;
    GuestWrapper<int16_t> toRet2 = GuestWrapper<int16_t>();
    toRet2 = toRet2.fromHost(toRet);
    
    return CW(toRet).get();
  }
}

int16_t MacBridge::SpeechBusySystemWide(void)
{
  @autoreleasepool {
    SInt16 toRet = [NSSpeechSynthesizer isAnyApplicationSpeaking];

    return CW(toRet).get();
  }
}

Executor::OSErr MacBridge::CountVoices (int16_t *numVoices)
{
  @autoreleasepool {
    if (!numVoices) {
      return ::paramErr;
    }
    
    BeginSpeech();
    SInt16 voiceCount = [NSSpeechSynthesizer availableVoices].count;
    
    *numVoices = CW(voiceCount).get();
    return Executor::noErr;
  }
}

Executor::OSErr MacBridge::DisposeSpeechChannel(Executor::SpeechChannel chan)
{
  //BeginSpeech();
  Executor::LONGINT ourDat = chan->data[0];
  NSSpeechSynthesizer *synth = synthesizerMap[ourDat];
  Executor::DisposPtr((Executor::Ptr)chan);
  [synth release];
  Executor::OSErr toRet = Executor::noErr;
  synthesizerMap.erase(ourDat);

  return CW(toRet).get();
}

Executor::OSErr MacBridge::SpeakString (Executor::Str255 textToBeSpoken)
{
  BeginSpeech();
  @autoreleasepool {
    NSString *ourStr = CFBridgingRelease(CFStringCreateWithPascalString(kCFAllocatorDefault, textToBeSpoken, kCFStringEncodingMacRoman));
    BOOL isBegin = [internalSynthesizer startSpeakingString:ourStr];
    Executor::OSErr toRet = isBegin ? Executor::noErr : -1;
    
    return CW(toRet).get();
  }
}

Executor::OSErr MacBridge::StopSpeech (Executor::SpeechChannel chan)
{
  BeginSpeech();
  NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
  @autoreleasepool {
    [synth stopSpeaking];
  }
  Executor::OSErr toRet = Executor::noErr;
  
  return CW(toRet).get();
}

Executor::OSErr MacBridge::ContinueSpeech (Executor::SpeechChannel chan)
{
  BeginSpeech();
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    [synth continueSpeaking];
    Executor::OSErr toRet = Executor::noErr;
    
    return CW(toRet).get();
  }
}

static inline void MacVoiceSpecToExecutorVoiceSpec(Executor::VoiceSpec &ExecutorVoice, ::VoiceSpec &MacVoice)
{
  ExecutorVoice.creator = CL(MacVoice.creator);
  ExecutorVoice.id = CL(MacVoice.id);
}

Executor::OSErr MacBridge::GetIndVoice (int16_t index, Executor::VoiceSpec *voice)
{
  @autoreleasepool {
    BeginSpeech();
    if (!voice) {
      return Executor::paramErr;
    }
    NSInteger voiceIndex = CW(index).get();
    NSArray *voices = [NSSpeechSynthesizer availableVoices];
    if (voiceIndex >= voices.count) {
      return CW((Executor::OSErr)(-244)).get();
    }
    NSDictionary *voiceDict = [NSSpeechSynthesizer attributesForVoice:voices[voiceIndex]];
    
    NSNumber *synthesizerID = voiceDict[@"VoiceSynthesizerNumericID"];
    NSNumber *voiceID = voiceDict[@"VoiceNumericID"];
    
    voice->creator = CL([synthesizerID unsignedIntValue]);
    voice->id = CL([voiceID unsignedIntValue]);
    
  }
  
  return Executor::noErr;
}

using Executor::_NewPtr_flags;

Executor::OSErr MacBridge::NewSpeechChannel (Executor::VoiceSpec *voice, Executor::SpeechChannel *chan)
{
  static Executor::LONGINT speechChanData = 0;
  @autoreleasepool {
    BeginSpeech();
    NSString *voiceID = nil;
    ::OSType voiceIDFCC = CL(voice->id);
    ::OSType voiceCreatorFCC = CL(voice->creator);
    
    for (NSDictionary *aVoice in speechVoices) {
      
      //NSDictionary *voiceDict = [NSSpeechSynthesizer attributesForVoice:aVoice];
      NSNumber *synthesizerID = aVoice[VoiceCreatorIDKey];
      NSNumber *voiceID = aVoice[VoiceIDKey];
      
      if ([synthesizerID unsignedIntValue] == voiceCreatorFCC && [voiceID unsignedIntValue] == voiceIDFCC) {
        voiceID = aVoice[VoiceNameKey];
      }
    }
    
    if (voiceID == nil) {
      return (Executor::OSErr)(-244);
    }
    
    Executor::SpeechChannelRecord aChan;
    aChan.data[0] = ++speechChanData;
    
    *chan = (Executor::SpeechChannel)NewPtr(sizeof(Executor::SpeechChannelRecord));
    
    **chan = aChan;
    NSSpeechSynthesizer *NSsynth = [[NSSpeechSynthesizer alloc] initWithVoice:voiceID];
    synthesizerMap[aChan.data[0]] = NSsynth;
    
    return Executor::noErr;
  }
}

Executor::OSErr MacBridge::StopSpeechAt (Executor::SpeechChannel chan, int32_t whereToStop)
{
  NSSpeechBoundary boundary;
  NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
  switch (whereToStop) {
      //kImmediate
    case 0:
      boundary = NSSpeechImmediateBoundary;
      break;
      
      //kEndOfWord
    case 1:
      boundary = NSSpeechWordBoundary;
      break;
      
    case 2:
      boundary = NSSpeechSentenceBoundary;
      break;
      
    default:
      return Executor::paramErr;
      break;
  }
  
  @autoreleasepool {
    [synth stopSpeakingAtBoundary:boundary];
  }
  return Executor::noErr;
}

Executor::OSErr MacBridge::PauseSpeechAt (Executor::SpeechChannel chan, int32_t whereToPause)
{
  NSSpeechBoundary boundary;
  NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
  switch (whereToPause) {
      //kImmediate
    case 0:
      boundary = NSSpeechImmediateBoundary;
      break;
      
      //kEndOfWord
    case 1:
      boundary = NSSpeechWordBoundary;
      break;
      
    case 2:
      boundary = NSSpeechSentenceBoundary;
      break;
      
    default:
      return Executor::paramErr;
      break;
  }
  
  @autoreleasepool {
    [synth pauseSpeakingAtBoundary:boundary];
  }
  
  return Executor::noErr;
}

Executor::OSErr MacBridge::SetSpeechRate(Executor::SpeechChannel chan, Executor::Fixed rate)
{
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    
    
    synth.rate = FixedToFloat(CL(rate).get());
  }
  
  return Executor::noErr;
}

Executor::OSErr MacBridge::GetSpeechRate (Executor::SpeechChannel chan, Executor::Fixed *rate)
{
  if (!rate) {
    return Executor::paramErr;
  }
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    
    ::Fixed ourRate = FloatToFixed(synth.rate);
    
    *rate = CL(ourRate).get();
  }
  
  return Executor::noErr;
}

Executor::OSErr MacBridge::SetSpeechPitch (Executor::SpeechChannel chan, Executor::Fixed pitch)
{
  Executor::OSErr wasSuccess = Executor::noErr;
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    
    float ourFloatPitch = FixedToFloat(CL(pitch).get());
    
    wasSuccess = [synth setObject:@(ourFloatPitch) forProperty:NSSpeechPitchModProperty error:NULL] ? Executor::noErr : -1;
  }
  
  return CW(wasSuccess).get();
}

Executor::OSErr MacBridge::GetSpeechPitch (Executor::SpeechChannel chan, Executor::Fixed *pitch)
{
  if (!pitch) {
    return Executor::paramErr;
  }
  ::OSErr toRet = Executor::noErr;
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    
    NSNumber *ourNum = [synth objectForProperty:NSSpeechPitchModProperty error:nil];
    if (ourNum == NULL) {
      toRet = -1;
    } else {
      Executor::Fixed ourPitch = FloatToFixed(ourNum.floatValue);
      *pitch = CL(ourPitch).get();
    }
  }
  
  return CW(toRet).get();
}

#undef NewHandle

Executor::OSErr MacBridge::UseDictionary (Executor::SpeechChannel chan, Executor::Handle dictionary)
{
#if 0
  @autoreleasepool {
    ::Size ExecSize = BigEndianValue(Executor::GetHandleSize(dictionary));
    NSData *aData = [NSData dataWithBytes:*dictionary length:ExecSize];
    NSSpeechSynthesizer *aSynth = synthesizerMap[chan];
  }
#endif
  warning_unimplemented (NULL_STRING);
  return CWC(Executor::paramErr).get();
}

Executor::OSErr MacBridge::MakeVoiceSpec (Executor::OSType creator, Executor::OSType identifier, Executor::VoiceSpec *voice)
{
  if (!voice) {
    return Executor::paramErr;
  }
  ::VoiceSpec nativeSpec = {0};
  ::OSErr toRet = ::MakeVoiceSpec(CL(creator).get(), CL(identifier).get(), &nativeSpec);
  MacVoiceSpecToExecutorVoiceSpec(*voice, nativeSpec);
  
  return CW(toRet).get();
}

Executor::OSErr MacBridge::GetVoiceDescription (
											const Executor::VoiceSpec *voice,
											Executor::VoiceDescription *info,
											Executor::LONGINT infoLength)
{
  if (CL(infoLength).get() != 362) {
    return Executor::paramErr;
  }
  if (voice == NULL || info == NULL) {
    return Executor::paramErr;
  }
  
  @autoreleasepool {
    info->length = CLC(362);
    //NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    NSString *voiceID = nil;
    ::OSType voiceIDFCC = CL(voice->id);
    ::OSType voiceCreatorFCC = CL(voice->creator);
    
    for (NSDictionary *aVoice in speechVoices) {
      
      //NSDictionary *voiceDict = [NSSpeechSynthesizer attributesForVoice:aVoice];
      NSNumber *synthesizerID = aVoice[VoiceCreatorIDKey];
      NSNumber *voiceID = aVoice[VoiceIDKey];
      
      if ([synthesizerID unsignedIntValue] == voiceCreatorFCC && [voiceID unsignedIntValue] == voiceIDFCC) {
        voiceID = aVoice[VoiceNameKey];
      }
    }
    if (voiceID == nil) {
      return (Executor::OSErr)(-244);
    }
    
    NSDictionary *aVoiceInfo = [NSSpeechSynthesizer attributesForVoice:voiceID];
    NSString *name = aVoiceInfo[NSVoiceName];
    CFStringGetPascalString((CFStringRef)name, info->name, 63, kCFStringEncodingMacRoman);
    info->age = CW([aVoiceInfo[NSVoiceAge] shortValue]);
    info->voice = *voice;
    info->version = CLC(1); //TODO: get real version!
    {
      NSString *gender = aVoiceInfo[NSVoiceGender];
      if ([gender isEqualToString:NSVoiceGenderMale]) {
        info->gender = CWC((int16_t)1); //kMale
      } else if ([gender isEqualToString:NSVoiceGenderFemale]) {
        info->gender = CWC((int16_t)2); //kFemale
      } else {
        info->gender = CWC((int16_t)0);
      }
    }
    
    NSString *comment = aVoiceInfo[NSVoiceDemoText];
    CFStringGetPascalString((CFStringRef)comment, info->comment, 255, kCFStringEncodingMacRoman);
    
    LangCode aLang = 0;
    RegionCode aRegion = 0;
    ::OSStatus ourStat = ::LocaleStringToLangAndRegionCodes([aVoiceInfo[NSVoiceLocaleIdentifier] UTF8String], &aLang, &aRegion);
    if (ourStat != ::noErr) {
      ourStat = ::LocaleStringToLangAndRegionCodes("en-US", &aLang, &aRegion);
      if (ourStat != ::noErr) {
        return -244;
      }
    }
    info->language = CW(aLang);
    info->region = CW(aRegion);
    info->script = CWC((int16_t)NSMacOSRomanStringEncoding);
    memset(info->reserved, 0, sizeof(info->reserved));
  }
  
  return Executor::noErr;
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
  ::OSErr toRet = ::noErr;
  //::OSErr toRet = ::GetVoiceInfo((const ::VoiceSpec*)voice, BigEndianValue(selector), voiceInfo);
  
  switch (CLC(selector).get()) {
    case soVoiceFile:
      //TODO: implement?
      warning_unimplemented (NULL_STRING);
	  toRet = Executor::fnfErr;
      break;
      
    case soVoiceDescription:
      return MacBridge::GetVoiceDescription(voice, (Executor::VoiceDescription*)voiceInfo, 362);
      break;
      
    default:
      break;
  }
  return CW(toRet).get();
}

Executor::OSErr MacBridge::SpeakText (Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes)
{
  Executor::OSErr theErr = ::noErr;
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    
    NSString *ourStr = [[NSString alloc] initWithBytes:textBuf length:CL(textBytes).get() encoding:NSMacOSRomanStringEncoding];
    
    if (![synth startSpeakingString:ourStr]) {
      theErr = -1;
    }
  }
  return CW(theErr).get();
}

Executor::OSErr MacBridge::SetSpeechInfo (
									  Executor::SpeechChannel chan,
									  Executor::OSType selector,
									  const void *speechInfo
									  )
{
  // TODO: handle different data types
  //::OSErr toRet = ::SetSpeechInfo((::SpeechChannel)chan, BigEndianValue(selector), speechInfo);
  ::OSErr toRet = ::noErr;
  
  warning_unimplemented (NULL_STRING);
  return toRet;
}

Executor::OSErr MacBridge::GetSpeechInfo (
                                          Executor::SpeechChannel chan,
                                          Executor::OSType selector,
                                          void *speechInfo)
{
  // TODO: handle different data types
  //::OSErr toRet = ::GetSpeechInfo((::SpeechChannel)chan, BigEndianValue(selector), speechInfo);
  ::OSErr toRet = ::noErr;

  warning_unimplemented (NULL_STRING);
  return CW(toRet).get();
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
									int32_t controlFlags
									)
{
  ::OSErr toRet = ::noErr;
  @autoreleasepool {
    //TODO: handle flags
    Executor::ULONGINT textSize = CL(textBytes).get();
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    NSString *aStr = [[NSString alloc] initWithBytes:textBuf length:textSize encoding:NSMacOSRomanStringEncoding];
    if (![synth startSpeakingString:aStr]) {
      toRet = -1;
    }
  }
  
	return CW(toRet).get();
}

Executor::OSErr MacBridge::TextToPhonemes (Executor::SpeechChannel chan, const void *textBuf, Executor::ULONGINT textBytes, Executor::Handle phonemeBuf, GUEST<Executor::LONGINT> *phonemeBytes)
{
  Executor::ULONGINT ExecSize = CL(Executor::GetHandleSize(phonemeBuf)).get();
  Executor::ULONGINT textSize = CL(textBytes).get();
  ::OSErr toRet = ::noErr;
  @autoreleasepool {
    NSSpeechSynthesizer *synth = synthesizerMap[chan->data[0]];
    NSString *aStr = [[NSString alloc] initWithBytes:textBuf length:textSize encoding:NSMacOSRomanStringEncoding];
    NSString *phonemes = [synth phonemesFromText:aStr];
    [aStr release];
    Executor::LONGINT lengthInMacRoman = (Executor::LONGINT)[phonemes lengthOfBytesUsingEncoding:NSMacOSRomanStringEncoding];
    
    if (lengthInMacRoman == 0) {
      return -224;
    }
    if (lengthInMacRoman > ExecSize) {
      Executor::SetHandleSize(phonemeBuf, CL(lengthInMacRoman).get());
    }
    strcpy((char*)MR(*phonemeBuf), [phonemes cStringUsingEncoding:NSMacOSRomanStringEncoding]);
    *phonemeBytes = CL(lengthInMacRoman);
  }
  
	return CW(toRet).get();
}
