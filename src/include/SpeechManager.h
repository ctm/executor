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

#define MODULE_NAME SpeechManager
#include <rsys/api-module.h>

namespace Executor
{
typedef struct VoiceSpec
{
    GUEST_STRUCT;
    GUEST<OSType> creator;
    GUEST<OSType> id;
} VoiceSpec, *VoiceSpecPtr;

typedef struct VoiceFileInfo
{
    GUEST_STRUCT;
    FSSpec fileSpec;
    GUEST<uint16_t> resID;
} VoiceFileInfo;

typedef struct SpeechStatusInfo
{
    GUEST_STRUCT;
    GUEST<Boolean> outputBusy;
    GUEST<Boolean> outputPaused;
    GUEST<int32_t> inputBytesLeft;
    GUEST<int16_t> phonemeCode;
} SpeechStatusInfo;

typedef struct VoiceDescription
{
    GUEST_STRUCT;
    GUEST<int32_t> length;
    GUEST<VoiceSpec> voice;
    GUEST<int32_t> version;
    GUEST<Str63> name;
    GUEST<Str255> comment;
    GUEST<int16_t> gender;
    GUEST<int16_t> age;
    GUEST<int16_t> script;
    GUEST<int16_t> language;
    GUEST<int16_t> region;
    GUEST<int32_t> reserved[4];
} VoiceDescription;

typedef struct SpeechChannelRecord
{
    GUEST_STRUCT;
    LONGINT data[1];
} SpeechChannelRecord, *SpeechChannel;

typedef struct PhonemeInfo
{
    GUEST_STRUCT;
    GUEST<int16_t> opcode;
    GUEST<Str15> phStr;
    GUEST<Str31> exampleStr;
    GUEST<int16_t> hiliteStart;
    GUEST<int16_t> hiliteEnd;
} PhonemeInfo;

typedef struct PhonemeDescriptor
{
    GUEST_STRUCT;
    GUEST<int16_t> phonemeCount;
    GUEST<PhonemeInfo> thePhonemes[1];
} PhonemeDescriptor;

typedef struct SpeechXtndData
{
    GUEST_STRUCT;
    GUEST<OSType> synthCreator;
    GUEST<Byte> synthData[2];
} SpeechXtndData;

typedef struct DelimiterInfo
{
    GUEST_STRUCT;
    GUEST<Byte> startDelimiter[2];
    GUEST<Byte> endDelimiter[2];
} DelimiterInfo;

NumVersion C_SpeechManagerVersion(void);
PASCAL_SUBTRAP(SpeechManagerVersion, 0xA800, 0x0000000C, SoundDispatch);
int16_t C_SpeechBusy(void);
PASCAL_SUBTRAP(SpeechBusy, 0xA800, 0x003C000C, SoundDispatch);
int16_t C_SpeechBusySystemWide(void);
PASCAL_SUBTRAP(SpeechBusySystemWide, 0xA800, 0x0040000C, SoundDispatch);
OSErr C_CountVoices(int16_t *numVoices);
PASCAL_SUBTRAP(CountVoices, 0xA800, 0x0108000C, SoundDispatch);
OSErr C_DisposeSpeechChannel(SpeechChannel chan);
PASCAL_SUBTRAP(DisposeSpeechChannel, 0xA800, 0x021C000C, SoundDispatch);
OSErr C_SpeakString(Str255 textToBeSpoken);
PASCAL_SUBTRAP(SpeakString, 0xA800, 0x0220000C, SoundDispatch);

OSErr C_StopSpeech(SpeechChannel chan);
PASCAL_SUBTRAP(StopSpeech, 0xA800, 0x022C000C, SoundDispatch);
OSErr C_ContinueSpeech(SpeechChannel chan);
PASCAL_SUBTRAP(ContinueSpeech, 0xA800, 0x0238000C, SoundDispatch);

OSErr C_GetIndVoice(int16_t index, VoiceSpec *voice);
PASCAL_SUBTRAP(GetIndVoice, 0xA800, 0x030C000C, SoundDispatch);
OSErr C_NewSpeechChannel(VoiceSpec *voice, SpeechChannel *chan);
PASCAL_SUBTRAP(NewSpeechChannel, 0xA800, 0x0418000C, SoundDispatch);
OSErr C_StopSpeechAt(SpeechChannel chan, int32_t whereToStop);
PASCAL_SUBTRAP(StopSpeechAt, 0xA800, 0x0430000C, SoundDispatch);
OSErr C_PauseSpeechAt(SpeechChannel chan, int32_t whereToPause);
PASCAL_SUBTRAP(PauseSpeechAt, 0xA800, 0x0434000C, SoundDispatch);
OSErr C_SetSpeechRate(SpeechChannel chan, Fixed rate);
PASCAL_SUBTRAP(SetSpeechRate, 0xA800, 0x0444000C, SoundDispatch);
OSErr C_GetSpeechRate(SpeechChannel chan, Fixed *rate);
PASCAL_SUBTRAP(GetSpeechRate, 0xA800, 0x0448000C, SoundDispatch);
OSErr C_SetSpeechPitch(SpeechChannel chan, Fixed pitch);
PASCAL_SUBTRAP(SetSpeechPitch, 0xA800, 0x044C000C, SoundDispatch);
OSErr C_GetSpeechPitch(SpeechChannel chan, Fixed *pitch);
PASCAL_SUBTRAP(GetSpeechPitch, 0xA800, 0x0450000C, SoundDispatch);
OSErr C_UseDictionary(SpeechChannel chan, Handle dictionary);
PASCAL_SUBTRAP(UseDictionary, 0xA800, 0x0460000C, SoundDispatch);
OSErr C_MakeVoiceSpec(OSType creator, OSType id, VoiceSpec *voice);
PASCAL_SUBTRAP(MakeVoiceSpec, 0xA800, 0x0604000C, SoundDispatch);
OSErr C_GetVoiceDescription(const VoiceSpec *voice, VoiceDescription *info, LONGINT infoLength);
PASCAL_SUBTRAP(GetVoiceDescription, 0xA800, 0x0610000C, SoundDispatch);
OSErr C_GetVoiceInfo(const VoiceSpec *voice, OSType selector, void *voiceInfo);
PASCAL_SUBTRAP(GetVoiceInfo, 0xA800, 0x0614000C, SoundDispatch);
OSErr C_SpeakText(SpeechChannel chan, const void *textBuf, ULONGINT textBytes);
PASCAL_SUBTRAP(SpeakText, 0xA800, 0x0624000C, SoundDispatch);
OSErr C_SetSpeechInfo(SpeechChannel chan, OSType selector, const void *speechInfo);
PASCAL_SUBTRAP(SetSpeechInfo, 0xA800, 0x0654000C, SoundDispatch);
OSErr C_GetSpeechInfo(SpeechChannel chan, OSType selector, void *speechInfo);
PASCAL_SUBTRAP(GetSpeechInfo, 0xA800, 0x0658000C, SoundDispatch);
OSErr C_SpeakBuffer(SpeechChannel chan, const void *textBuf, ULONGINT textBytes, int32_t controlFlags);
PASCAL_SUBTRAP(SpeakBuffer, 0xA800, 0x0828000C, SoundDispatch);
OSErr C_TextToPhonemes(SpeechChannel chan, const void *textBuf, ULONGINT textBytes, Handle phonemeBuf, GUEST<LONGINT> *phonemeBytes);
PASCAL_SUBTRAP(TextToPhonemes, 0xA800, 0x0A5C000C, SoundDispatch);
}

#endif /* defined(__CocoaExecutor__SpeechManager__) */
